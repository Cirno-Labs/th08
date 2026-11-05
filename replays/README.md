# Replay corpus

These 59 player recordings were downloaded from the
[Imperishable Night world record table](https://maribelhearn.com/wr)
on 2026-11-05 for regression testing. They cover all twelve shot types
on Easy, Normal, Hard, Lunatic and Extra, except Remilia Hard, whose
link served an empty file; the manifest lists it under `skipped`.

`manifest.json` retains each recording's source URL, credited player,
date, shot, difficulty, score and SHA-1. The recordings belong to their
players; inclusion here documents test provenance, not authorship.

Refresh with `python3 tools/replay/replay.py fetch`. Run with `just replay`.
See [the harness documentation](../docs/replay.md) for validation and
coverage. Downloading or parsing a recording alone does not count as a
successful parity check.
