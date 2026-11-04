"""th08's build hook: the standalone exe's function and data order."""
import csv
from pathlib import Path

ORDER = "build/exe_order.txt"
ROOT = Path(__file__).resolve().parents[1]


def exe_objects(objects: list[str], feats: list[str]) -> list[str]:
    """Reconstruct game code and readonly COMDAT placement.

    This uses /ORDER as a layout reconstruction aid, without claiming
    that the original build used it. Folded aliases must all be listed:
    LINK orders the ICF representative, which can be a different symbol
    from the function whose original address is known.
    """
    import coff
    import project

    defined: set[str] = set()
    constants: set[str] = set()
    for path in objects:
        obj = coff.Object(path)
        defined.update(
            symbol.name
            for symbol in obj.externals()
            if symbol.section > 0
            and obj.sections[symbol.section - 1].flags & coff.CODE
        )
        constants.update(
            symbol.name
            for symbol in obj.externals()
            if symbol.section > 0
            and obj.sections[symbol.section - 1].name == ".rdata"
            and obj.sections[symbol.section - 1].flags & 0x1000  # COMDAT
        )
    with (ROOT / "config/symbols.csv").open() as handle:
        addresses: dict[str, int] = {
            symbol: int(row["address"], 16)
            for row in csv.DictReader(handle)
            if row["size"] and 0x402000 <= int(row["address"], 16) < project.GAME_END
            for symbol in row["symbol"].split()
            if symbol in defined
        }
    with (ROOT / "config/exe_constants.csv").open() as handle:
        addresses.update(
            (row["symbol"], int(row["address"], 16))
            for row in csv.DictReader(handle)
            if row["symbol"] in constants
        )
    # LINK supplies the leading underscore for C/stdcall names itself.
    names = [
        symbol.removeprefix("_")
        for symbol in sorted(addresses, key=lambda symbol: (addresses[symbol], symbol))
    ]
    (ROOT / ORDER).write_text("\n".join(names) + "\n")
    return objects
