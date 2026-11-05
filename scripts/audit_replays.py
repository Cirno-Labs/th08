"""Revalidate saved replay runs and compare their complete, unchanged traces.

    python3 scripts/audit_replays.py --labels original original+ standalone

Uses the current parser/checks, verifies stored artifact hashes and exit
status, and writes build/replay/audit.json. It never changes raw run
summaries or traces, so a corrected validator can be audited without
replaying the games or discarding earlier assessments.
"""
from __future__ import annotations

import argparse
import datetime
import hashlib
import json
from pathlib import Path
import sys
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools/replay"))
import replay


def audit_run(case: str, label: str, rpy: Path) -> tuple[dict[str, Any], replay.Trace | None]:
    directory = ROOT / "build/replay" / case / label
    failures: list[str] = []
    try:
        summary = json.loads((directory / "run.json").read_text())
        artifacts = {
            "replay": rpy,
            "exe": ROOT / summary["exe_path"],
            "patched": directory / "game/th08.exe",
            "tracer": directory / "game/th08trace.dll",
            "trace": directory / "trace.bin",
        }
        for name, path in artifacts.items():
            if replay.sha1(path) != summary.get(name + "_sha1"):
                failures.append(f"{name} artifact differs from its recorded SHA-1")
        if summary["exit_code"] != 0:
            failures.append(f"exit code {summary['exit_code']}")
        if summary["stuck"]:
            failures.append(summary["stuck"])
        trace = replay.read_trace(artifacts["trace"])
        validation = replay.validate(replay.hook().Replay(rpy), trace)
        failures.extend(validation["failures"])
        result = dict(label=label, exe_sha1=summary["exe_sha1"], trace_sha1=summary["trace_sha1"],
                      seconds=summary["seconds"], **validation)
        result.update(passed=not failures, failures=failures)
        return result, trace
    except (OSError, ValueError, KeyError) as exc:
        return dict(label=label, passed=False, failures=[str(exc)]), None


def audit(paths: list[Path], labels: list[str]) -> dict[str, Any]:
    cases = []
    for rpy in paths:
        case = rpy.stem
        runs = []
        comparisons = []
        base: replay.Trace | None = None
        for i, label in enumerate(labels):
            result, trace = audit_run(case, label, rpy)
            runs.append(result)
            if i == 0:
                base = trace
            else:
                comparison = (replay.compare(base, trace) if base is not None and trace is not None else
                              dict(identical=False, compared=0, difference="a trace is missing"))
                comparison.update(a=labels[0], b=label)
                comparisons.append(comparison)
        passed = all(run["passed"] for run in runs) and all(c["identical"] for c in comparisons)
        display = rpy.relative_to(ROOT) if rpy.is_relative_to(ROOT) else rpy
        cases.append(dict(replay=str(display), passed=passed, runs=runs, comparisons=comparisons))
        print(f"{'PASS' if passed else 'FAIL'} {case}", flush=True)
        for run in runs:
            for failure in run["failures"]:
                print(f"    {run['label']}: {failure}", flush=True)
        for comparison in comparisons:
            if not comparison["identical"]:
                print(f"    DIFFERENT: {comparison}", flush=True)
    hashes = {str(path.relative_to(ROOT)): hashlib.sha1(path.read_bytes()).hexdigest()
              for path in (Path(__file__), ROOT / "scripts/replay_hook.py", ROOT / "tools/replay/replay.py")}
    return dict(audited_at=datetime.datetime.now(datetime.timezone.utc).isoformat(), validator_sha1=hashes,
                passed=all(case["passed"] for case in cases),
                covered=sum(case["passed"] for case in cases),
                trace_matches=sum(all(c["identical"] for c in case["comparisons"]) for case in cases),
                covered_calcs=sum(case["runs"][0].get("playing_calcs", 0) for case in cases if case["passed"]),
                total=len(cases), cases=cases)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("rpy", nargs="*", type=Path)
    parser.add_argument("--labels", nargs="+", default=["original", "original+", "standalone"])
    parser.add_argument("--out", type=Path, default=ROOT / "build/replay/audit.json")
    args = parser.parse_args()
    if len(args.labels) < 2 or len(set(args.labels)) != len(args.labels):
        parser.error("provide at least two distinct run-directory labels")
    for label in args.labels:
        if Path(label).name != label or label in (".", ".."):
            parser.error("labels must be directory names")
    paths = ([path.resolve() for path in args.rpy] if args.rpy else
             [ROOT / "replays" / entry["file"] for entry in
              json.loads((ROOT / "replays/manifest.json").read_text())["replays"]])
    if not paths:
        parser.error("no replays to audit")
    result = audit(paths, args.labels)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=1) + "\n")
    print(f"{result['covered']} of {result['total']} complete replay cases passed and matched")
    sys.exit(0 if result["passed"] else 1)


if __name__ == "__main__":
    main()
