"""Recover the readonly COMDAT order after building the vanilla exe.

    python3 scripts/recover_exe_constants.py [OUTPUT]

OUTPUT defaults to config/exe_constants.csv. Addresses come from original
instruction operands and initialized tables, following our COFF relocations.
Unreferenced string literals require a unique byte match in original .rdata.
No executable bytes are written or copied into the build inputs.
"""
from __future__ import annotations

from collections import defaultdict
import csv
import os
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import build
import coff
import compare_exe


def recover_object(
    obj: coff.Object,
    original: compare_exe.PE,
    known: dict[str, int],
    logical: dict[str, int],
) -> tuple[dict[str, set[int]], dict[str, bytes]]:
    locations: dict[tuple[int, int], set[int]] = defaultdict(set)
    symbols: list[coff.Symbol] = [
        symbol for symbol in obj.symbols if symbol is not None and symbol.section > 0
    ]
    for symbol in symbols:
        address = known.get(symbol.name)
        if address is None and "@?" in symbol.name:
            address = logical.get(symbol.name.removeprefix("?").split("@", 1)[0])
        if address is not None:
            locations[(symbol.section, symbol.value)].add(address)
    while True:
        previous = sum(len(values) for values in locations.values())
        for section_index, section in enumerate(obj.sections, 1):
            if section.flags & 0x80:  # uninitialized data
                continue
            anchors = [
                (value, addresses.copy())
                for (index, value), addresses in locations.items()
                if index == section_index
            ]
            for value, addresses in anchors:
                following = [
                    symbol.value for symbol in symbols
                    if symbol.section == section_index and symbol.value > value
                    and (not section.flags & coff.CODE or symbol.storage == coff.CLASS_EXTERNAL)
                ]
                end = min(following, default=len(section.data))
                for offset, index, kind in section.relocs:
                    if kind != coff.DIR32 or not value <= offset < end:
                        continue
                    target = obj.symbols[index]
                    if target is None or target.section <= 0:
                        continue
                    if obj.sections[target.section - 1].flags & coff.CODE:
                        continue
                    addend = struct.unpack_from("<I", section.data, offset)[0]
                    for address in addresses:
                        data = original.read(address + offset - value, 4)
                        if len(data) != 4:
                            continue
                        expected = (struct.unpack("<I", data)[0] - addend) & 0xFFFFFFFF
                        if original.section_of(expected) == ".rdata":
                            locations[(target.section, target.value)].add(expected)
        if sum(len(values) for values in locations.values()) == previous:
            break
    addresses_by_name: dict[str, set[int]] = {}
    contents: dict[str, bytes] = {}
    for symbol in obj.externals():
        if symbol.section <= 0:
            continue
        section = obj.sections[symbol.section - 1]
        if section.name == ".rdata" and section.flags & 0x1000:  # COMDAT
            addresses_by_name[symbol.name] = locations.get((symbol.section, symbol.value), set())
            contents[symbol.name] = section.data[symbol.value:]
    return addresses_by_name, contents


def main() -> int:
    os.chdir(ROOT)
    original = compare_exe.PE(build.ORIGINAL)
    with (ROOT / "config/symbols.csv").open() as handle:
        rows: list[dict[str, str]] = list(csv.DictReader(handle))
    known: dict[str, int] = build.read_symbols()
    logical = {
        row["name"]: int(row["address"], 16)
        for row in rows if row["name"].startswith("g_")
    }
    recovered: dict[str, set[int]] = defaultdict(set)
    contents: dict[str, bytes] = {}
    for _, path in build.sources(build.EXE, build.OBJ_EXE):
        addresses, data = recover_object(coff.Object(path), original, known, logical)
        for symbol, values in addresses.items():
            recovered[symbol].update(values)
        contents.update(data)
    start, size, _, _ = original.sections[".rdata"]
    rdata = original.read(start, size)
    for symbol, addresses in recovered.items():
        if addresses or not symbol.startswith("??_C@"):
            continue
        data = contents[symbol]
        first = rdata.find(data)
        if data and first >= 0 and rdata.find(data, first + 1) < 0:
            addresses.add(start + first)
    unresolved = {symbol: sorted(values) for symbol, values in recovered.items() if len(values) != 1}
    if unresolved:
        for symbol, values in unresolved.items():
            print(f"{symbol}: {[hex(value) for value in values]}", file=sys.stderr)
        return 1
    ordered = sorted(
        ((next(iter(values)), symbol) for symbol, values in recovered.items()),
        key=lambda item: (item[0], item[1]),
    )
    output = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "config/exe_constants.csv"
    with output.open("w", newline="") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(["address", "symbol"])
        writer.writerows((f"0x{address:08x}", symbol) for address, symbol in ordered)
    print(f"Recovered {len(ordered)} unambiguous readonly COMDAT addresses into {output}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
