"""Check th08's game functions and writable globals against their original addresses.

Run after `just exe`: python3 scripts/check_exe_layout.py
Local static addresses are recovered from linked DIR32 operands in the
functions that reference them, using the corresponding COFF relocations.
"""

from __future__ import annotations

import csv
import os
import struct
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import build
import coff
import compare_exe


def local_addresses(
    image: coff.Image,
    mapped: dict[str, tuple[int, str]],
    names: set[str],
) -> dict[str, set[int]]:
    """Recover named function statics from their linked address operands."""
    found: dict[str, set[int]] = defaultdict(set)
    for _, path in build.sources(build.EXE, build.OBJ_EXE):
        obj = coff.Object(path)
        for function in obj.externals():
            if function.section <= 0 or function.name not in mapped:
                continue
            section = obj.sections[function.section - 1]
            if not section.flags & coff.CODE:
                continue
            following = [
                symbol.value
                for symbol in obj.definitions()
                if symbol.section == function.section and symbol.value > function.value
            ]
            end = min(following, default=len(section.data))
            address = mapped[function.name][0]
            for offset, index, kind in section.relocs:
                if kind != coff.DIR32 or not function.value <= offset < end:
                    continue
                target = obj.symbols[index]
                if target is None or target.section <= 0 or "@?" not in target.name:
                    continue
                name = target.name.removeprefix("?").split("@", 1)[0]
                if name not in names or obj.sections[target.section - 1].flags & coff.CODE:
                    continue
                addend = struct.unpack_from("<I", section.data, offset)[0]
                linked = struct.unpack(
                    "<I", image.read(address + offset - function.value, 4)
                )[0]
                found[name].add((linked - addend) & 0xFFFFFFFF)
    return found


def main() -> int:
    os.chdir(ROOT)
    image = coff.Image(ROOT / build.EXE)
    mapped: dict[str, tuple[int, str]] = compare_exe.read_map(ROOT / build.EXE_MAP)
    with (ROOT / "config/symbols.csv").open() as handle:
        rows: list[dict[str, str]] = list(csv.DictReader(handle))
    writable = [row for row in rows if 0x4C6000 <= int(row["address"], 16) < 0x18DA000]
    functions = [
        row for row in rows
        if 0x402000 <= int(row["address"], 16) < build.project.GAME_END
        and row["size"] and row["symbol"]
    ]
    with (ROOT / "config/exe_constants.csv").open() as handle:
        selected = [
            {"address": row["address"], "name": row["symbol"], "symbol": row["symbol"]}
            for row in csv.DictReader(handle)
        ]
    checked = functions + writable + selected
    local = local_addresses(image, mapped, {row["name"] for row in checked})
    failures: list[str] = []
    resolved = 0
    for row in checked:
        expected = int(row["address"], 16)
        addresses = {mapped[symbol][0] for symbol in row["symbol"].split() if symbol in mapped}
        if not addresses:
            addresses = local.get(row["name"], set())
            resolved += bool(addresses)
        if addresses != {expected}:
            actual = ", ".join(f"{address:#x}" for address in sorted(addresses)) or "unresolved"
            failures.append(f"{row['name']}: expected {expected:#x}, got {actual}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print(f"All {len(functions)} named game functions are at their original addresses.")
    print(f"All {len(writable)} named writable globals are at their original addresses.")
    print(f"All {len(selected)} readonly COMDAT symbols are at their original addresses.")
    print(f"Resolved {resolved} function statics through linked COFF relocations.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
