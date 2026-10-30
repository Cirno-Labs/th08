"""th08's build hook (build.hook in project.toml): the standalone exe's
DOS stub.

The original's headers take 0x1e00 bytes: after the section table they
hold the .comment sections (compiler banners) of D3DX8's Intel-compiled
objects. Those come from a DirectX 8.1 build we don't have, and the
linker leaves ours out, so its headers would take 0x400 bytes and every
section would start 0x1000 bytes early. A DOS stub padded to 0x1a00
bytes (/STUB in exe.ldflags) takes up that room: the sections then
start at the original's addresses and file offsets. The stub's first
0x80 bytes are the linker's usual DOS header and program.
"""
import os, struct

STUB = "build/stub.bin"
SIZE = 0x1A00

# The usual 16-bit program: prints "This program cannot be run in DOS
# mode." and exits.
PROGRAM = bytes.fromhex(
    "0e1fba0e00b409cd21b8014ccd21546869732070726f6772616d2063616e6e6f"
    "742062652072756e20696e20444f53206d6f64652e0d0d0a2400000000000000")


def stub():
    header = bytearray(0x40)
    # MZ, 0x90 bytes in the last page, 3 pages, 4 paragraphs of header,
    # 0xffff paragraphs at most, sp 0xb8, relocations at 0x40.
    struct.pack_into("<2sHHHHHHHHHHHH", header, 0, b"MZ", 0x90, 3, 0, 4, 0, 0xFFFF, 0, 0xB8, 0, 0, 0, 0x40)
    return bytes(header + PROGRAM).ljust(SIZE, b"\0")


def exe_flags(feats):
    data = stub()
    os.makedirs(os.path.dirname(STUB), exist_ok=True)
    if not os.path.exists(STUB) or open(STUB, "rb").read() != data:
        with open(STUB, "wb") as f:
            f.write(data)
    return []
