"""Extract the four demo replays from the user's th08.dat, into build/.

    python3 scripts/replay_fixtures.py
    just replay build/replay/fixtures/*.rpy

No game assets are added to the repository. These are single-stage runs,
not substitutes for the full-route corpus downloaded by replay.py fetch.
"""
from __future__ import annotations

from pathlib import Path
import struct

try:
    from .replay_hook import decompress
except ImportError:
    from replay_hook import decompress

ROOT = Path(__file__).resolve().parents[1]

# g_CryptParams: a file kind's stored letter, key, step, block size and
# limit (src/unsorted.cpp).
CRYPT_PARAMS = [(0x5d, 0x1b, 0x37, 64, 10240), (0x74, 0x51, 0xe9, 64, 12288),
                (0x71, 0xc1, 0x51, 5120, 8192), (0x8a, 0x03, 0x19, 5120, 30720),
                (0x95, 0xab, 0xcd, 512, 4096), (0xb7, 0x12, 0x34, 1024, 10240),
                (0x9d, 0x35, 0x97, 128, 10240), (0xaa, 0x99, 0x37, 1024, 4096)]
CRYPT_MAGIC = bytes([0x85 - 0x20, 0xa4 - 0x40, 0xda - 0x60])  # "edz"


def decrypt(data: bytes, key: int, step: int, block: int, limit: int) -> bytes:
    """DecryptData (src/unsorted.cpp): each block's bytes land on every
    other byte from its end backwards, then in the gaps, xored with a
    growing key; a short tail and the bytes past `limit` are copied."""
    size = len(data)
    tail = size % block if size % block < block // 4 else 0
    tail += size & 1
    size -= tail
    out = bytearray(len(data))
    p = w = 0
    while size > 0 and limit > 0:
        block = min(block, size)
        for start in (w + block - 1, w + block - 2):
            for i in range(start, w - 1, -2):
                out[i] = data[p] ^ key
                p += 1
                key = (key + step) & 255
        w += block
        size -= block
        limit -= block
    size += tail
    out[w:w + size] = data[p:p + size]
    return bytes(out)


def decrypt_file(data: bytes) -> bytes:
    """DecryptFileData: a file starting with "edz" and its kind's letter
    is the rest, decrypted with the kind's parameters."""
    if data[:3] != CRYPT_MAGIC:
        return data
    for j, (kind, key, step, block, limit) in enumerate(CRYPT_PARAMS):
        if data[3] == (kind - j * 16 - 0x10) & 255:
            return decrypt(data[4:], key, step, block, limit)
    return data


def extract(archive: Path, destination: Path) -> list[Path]:
    """DatArchive::ReadTable and Decode, for the .rpy entries."""
    data = archive.read_bytes()
    if len(data) < 16 or data[:4] != b"PBGZ":
        raise ValueError(f"{archive}: not a PBGZ archive")
    count, table_offset, size = struct.unpack_from("<III", decrypt(data[4:16], 0x1b, 0x37, 12, 0x400))
    count -= 123456
    table_offset -= 345678
    size -= 567891
    if count <= 0 or not 16 <= table_offset < len(data):
        raise ValueError(f"{archive}: invalid archive header")
    table = decompress(decrypt(data[table_offset:], 0x3e, 0x9b, 0x80, 0x400), size)
    entries: list[tuple[str, int, int]] = []
    at = 0
    for _ in range(count):
        end = table.index(0, at)
        name = table[at:end].decode("shift_jis")
        offset, length, _ = struct.unpack_from("<III", table, end + 1)
        entries.append((name, offset, length))
        at = end + 13
    paths = []
    for i, (name, offset, length) in enumerate(entries):
        if not name.lower().endswith(".rpy"):
            continue
        end_offset = entries[i + 1][1] if i + 1 < len(entries) else table_offset
        if not 16 <= offset < end_offset <= table_offset:
            raise ValueError(f"invalid entry {name}")
        path = destination / name.replace("\\", "/").rsplit("/", 1)[-1]
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(decrypt_file(decompress(data[offset:end_offset], length)))
        paths.append(path)
    return paths


if __name__ == "__main__":
    for path in extract(ROOT / "orig/th08_100d/th08.dat", ROOT / "build/replay/fixtures"):
        print(path.relative_to(ROOT))
