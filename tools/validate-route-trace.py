#!/usr/bin/env python3
"""Replay captured wrapper calls through the C++ model with measured search output."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parent.parent
HASH = "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168"


def validate_call(record: dict, replay: Path) -> list[str]:
    snapshot = bytes.fromhex(record["snapshot_from_search"])
    if len(snapshot) != 0x20c or len(bytes.fromhex(record["return"]["snapshot"])) != 0x20c:
        raise ValueError("Snapshot length is not 0x20c")
    dimensions = record["dimensions"]
    if len(dimensions) != 2 or any(not 0 < value <= 0x7fffffff for value in dimensions):
        raise ValueError("Dimensions outside positive signed model domain")
    arguments = record["arguments"]
    if len(arguments) != 4 or any(not 0 <= value <= 0xffffffff for value in arguments):
        raise ValueError("Expected four 32-bit argument words")
    inputs = arguments + [record["configured_budget"], *dimensions,
                          record["flag_after_search"], record["budget_after"]]
    inputs += list(struct.unpack("<131I", snapshot))
    result = subprocess.run([str(replay)], input=" ".join(f"{value & 0xffffffff:x}" for value in inputs),
                            text=True, capture_output=True, check=True, timeout=10)
    words = [int(value, 16) for value in result.stdout.split()]
    if len(words) != 143:
        raise ValueError("Unexpected replay output length")
    observed = record["search"]
    returned = record["return"]
    expected = [*observed["coordinates"], observed["unknown_argument"], observed["budget_before"],
        observed["flag_before"], returned["flag"], record["budget_after"], returned["eax"],
        returned["route_present"], returned["unknown_d03"], returned["configured_budget"]]
    expected += list(struct.unpack("<131I", bytes.fromhex(returned["snapshot"])))
    mismatches = []
    labels = ["search.x", "search.y", "search.z", "search.unknown_argument", "search.budget_before",
              "search.flag_before", "return.flag", "budget_after", "return.eax", "route_present",
              "unknown_d03", "configured_budget_after"]
    for index, (actual, wanted) in enumerate(zip(words, expected)):
        if actual != wanted & 0xffffffff:
            label = labels[index] if index < 12 else f"snapshot DWORD {index - 12}"
            mismatches.append(f"{label}: model={actual:x}, game={wanted & 0xffffffff:x}")
    if observed["object"] != record["object"]:
        mismatches.append("search object does not match entry ECX")
    if observed["context"] != record["image_base"] + 0x290148:
        mismatches.append("search context does not match relocated scratch address")
    if observed["budget_pointer"] != record["entry_sp"] - 4:
        mismatches.append("budget pointer is not the expected wrapper stack local")
    return mismatches


def build_replay(destination: Path) -> Path:
    compiler = os.environ.get("CXX", "g++")
    output = destination / "route-trace-replay"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-I", str(REPO / "reconstruction/pathfinding"),
        str(REPO / "reconstruction/pathfinding/route_request.cpp"),
        str(REPO / "reconstruction/pathfinding/replay_trace.cpp"), "-o", str(output)], check=True)
    return output


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("evidence", nargs="?", type=Path, help="default: newest run with calls.jsonl")
    args = parser.parse_args()
    evidence = args.evidence
    if evidence is None:
        candidates = list((REPO / "working/experiments/route-trace").glob("run-*/calls.jsonl"))
        if not candidates:
            raise ValueError("No captured route trace found; run ./tools/trace-route-experiment.py")
        evidence = max(candidates, key=lambda path: path.stat().st_mtime_ns).parent
    manifest = json.loads((evidence / "manifest.json").read_text())
    if manifest["sha256"] != HASH:
        raise ValueError("Trace executable hash is not the supported no-CD build")
    # Fresh validation subdirectory preserves all prior reports.
    output = Path(tempfile.mkdtemp(prefix="validation-", dir=evidence))
    replay = build_replay(output)
    records = [json.loads(line) for line in (evidence / "calls.jsonl").read_text().splitlines() if line.strip()]
    failures = []
    for index, record in enumerate(records):
        differences = validate_call(record, replay)
        if differences:
            failures.append({"call": index + 1, "differences": differences})
    summary = {"calls_checked": len(records), "failures": failures,
        "origin": manifest.get("origin", "unknown"),
        "matched": bool(records) and not failures, "scope": "wrapper only; measured search output supplied",
        "trace_sha256": hashlib.sha256((evidence / "calls.jsonl").read_bytes()).hexdigest(),
        "model_sha256": {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in
            [*(REPO / "reconstruction/pathfinding").glob("route_request.*"),
             REPO / "reconstruction/pathfinding/replay_trace.cpp"]},
        "capture_complete": manifest.get("status") == "captured"}
    (output / "report.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(f"Checked {len(records)} captured calls; {len(failures)} mismatching calls. Report: {output / 'report.json'}")
    return 0 if summary["matched"] else 2


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f"Error: {error}", file=sys.stderr)
        raise SystemExit(1)
