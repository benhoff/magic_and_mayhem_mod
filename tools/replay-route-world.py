#!/usr/bin/env python3
"""Verify a frozen world capture, build the host search, and print its replay JSON."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
import struct
import tempfile
import sys
REPO = Path(__file__).resolve().parent.parent

def compare_reference(report, reference):
    comparison = {"status": "no_reference"}
    if reference is not None:
        expected = bytes.fromhex(reference["snapshot_from_search"])
        actual = bytes.fromhex(report["snapshot_hex"])
        if len(expected) != 0x20c or len(actual) != 0x20c:
            raise ValueError("Reference route snapshot size mismatch")
        differences = [i for i, (a, b) in enumerate(zip(actual, expected)) if a != b]
        fields = [name for name, actual, expected in [
            ("budget", report["budget_remaining"], reference["budget_after"]),
            ("flag", report["flag"], reference["flag_after_search"])] if actual != expected]
        if "context_after_hex" in reference:
            context = bytes.fromhex(reference["context_after_hex"])
            if len(context) != 0x269:
                raise ValueError("Reference context size mismatch")
            for name, offset, fmt in [("best_heuristic",0x211,"i"),("best_node",0x215,"I"),
                                      ("best_priority",0x265,"i")]:
                if report[name] != struct.unpack_from("<" + fmt,context,offset)[0]:
                    fields.append(name)
        comparison = {"status": "mismatch" if differences or fields else "match",
                      "snapshot_byte_offsets": differences, "fields": fields}
    return comparison

def build_replay():
    build = REPO / "working/build/pathfinding"
    subprocess.run(["cmake", "-S", str(REPO / "reconstruction/pathfinding"), "-B", str(build)],
                   check=True, stdout=sys.stderr)
    subprocess.run(["cmake", "--build", str(build), "--target", "route-replay", "-j", "4"],
                   check=True, stdout=sys.stderr)
    return build / "route-replay"


def load_sequence(directory, world):
    calls = [json.loads(line) for line in (directory / "search-calls.jsonl").read_text().splitlines()]
    if not calls:
        raise ValueError("No completed common-search calls")
    calls.sort(key=lambda c: c["sequence"])
    seen = set()
    snapshots = set()
    image_base = calls[0]["image_base"]
    rows = []
    for call in calls:
        number = call["sequence"]
        if not isinstance(number, int) or number <= 0 or number in seen:
            raise ValueError("Invalid or duplicate sequence number")
        seen.add(number)
        if call["image_base"] != image_base:
            raise ValueError("Sequence crosses loaded images; split captures by launch")
        search = call["search"]
        token, flag = search["context"], search["flag_before"]
        if not isinstance(token,int) or not 0 < token <= 0xffffffff or not isinstance(flag,int) or not 0 <= flag <= 255:
            raise ValueError("Invalid context/initialization byte")
        name = search["world_snapshot"]
        if Path(name).name != name or name in snapshots:
            raise ValueError("Unsafe or duplicate sequence snapshot name")
        snapshots.add(name)
        path = directory / name
        metadata = world.verify_snapshot(path)
        if metadata.get("context") != token or metadata.get("flag_before") != flag or metadata["image_base"] != image_base:
            raise ValueError("World metadata does not match search call")
        header = world.HEADER.unpack_from(path.read_bytes())
        expected = [search["object"], search["budget_before"], search["unknown_argument"], *search["coordinates"]]
        actual = [header[1], header[7], header[8], *header[9:12]]
        if actual != [value & 0xffffffff for value in expected]:
            raise ValueError("World header does not match search arguments")
        for key, expected_flag in [("context_before_hex",flag),("context_after_hex",call["flag_after_search"])]:
            if key in call:
                context = bytes.fromhex(call[key])
                if len(context) != 0x269 or context[0x20c] != expected_flag:
                    raise ValueError("Context bytes disagree with observed flag")
        # std::quoted consumes escaped backslashes/quotes, not JSON escapes.
        absolute = str(path.resolve())
        if any(c in absolute for c in "\n\r\0"):
            raise ValueError("Invalid snapshot pathname")
        quoted = '"' + absolute.replace('\\','\\\\').replace('"','\\"') + '"'
        rows.append(f"{token} {flag} {quoted}\n")
    return calls, rows


def compare_sequence(reports, calls):
    if len(reports) != len(calls):
        raise ValueError("Replay output count differs from captured sequence")
    tainted = set()
    for report, call in zip(reports,calls):
        token = call["search"]["context"]
        if report["context"] != token:
            raise ValueError("Replay context differs from captured call")
        report["sequence"] = call["sequence"]
        report["caller"] = call["return_address"]
        if call["search"]["flag_before"]:
            tainted.discard(token)
        if report["status"] != "replayed":
            report["native_comparison"] = {"status":"inconclusive","reason":report["reason"]}
            tainted.add(token)
        else:
            comparison = compare_reference(report,call)
            if token in tainted:
                comparison["observed_status"] = comparison["status"]
                comparison["status"] = "inconclusive"
                comparison["reason"] = "prior_mismatch"
            report["native_comparison"] = comparison
            if comparison["status"] == "mismatch":
                tainted.add(token)
    statuses = [r["native_comparison"]["status"] for r in reports]
    return {"calls": reports, "matched":statuses.count("match"),
            "mismatched":statuses.count("mismatch"), "inconclusive":statuses.count("inconclusive")}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("snapshot", type=Path, nargs="?")
    parser.add_argument("--sequence", type=Path, help="replay completed search-calls.jsonl and worlds in this directory")
    parser.add_argument("--calls", type=Path, help="completed wrapper trace calls (default: calls.jsonl beside snapshot)")
    args = parser.parse_args()
    if bool(args.snapshot) == bool(args.sequence) or (args.sequence and args.calls):
        parser.error("provide one snapshot or --sequence DIRECTORY; --calls applies only to a snapshot")
    spec = importlib.util.spec_from_file_location("world", REPO / "tools/route-world-snapshot.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    if args.sequence:
        calls, rows = load_sequence(args.sequence,module)
        binary = build_replay()
        with tempfile.TemporaryDirectory(prefix="sequence-replay-", dir=binary.parent) as temporary:
            index = Path(temporary) / "index.txt"
            index.write_text("".join(rows))
            result = subprocess.run([str(binary), "--sequence", str(index)],check=True,
                                    timeout=60,capture_output=True,text=True)
        report = compare_sequence([json.loads(line) for line in result.stdout.splitlines()],calls)
        (args.sequence / "sequence-replay.json").write_text(json.dumps(report,indent=2) + "\n")
        print(json.dumps(report,indent=2))
        if report["mismatched"]:
            raise SystemExit(1)
        if report["inconclusive"]:
            raise SystemExit(2)
        return
    manifest = module.verify_snapshot(args.snapshot)
    if manifest.get("format") != "MNMWLD01":
        raise ValueError("Unsupported world manifest")
    if not manifest.get("flag_before",1):
        raise ValueError("Continuation snapshot requires --sequence with preceding fresh calls")
    binary = build_replay()
    result = subprocess.run([str(binary), str(args.snapshot.resolve())],
                            check=True, timeout=60, capture_output=True, text=True)
    report = json.loads(result.stdout)
    calls = args.calls or args.snapshot.parent / "calls.jsonl"
    reference = None
    if calls.exists():
        for line in calls.read_text().splitlines():
            call = json.loads(line)
            if call.get("search", {}).get("world_snapshot") == args.snapshot.name:
                if reference is not None:
                    raise ValueError("Multiple reference calls name the same snapshot")
                reference = call
    report["native_comparison"] = compare_reference(report, reference)
    args.snapshot.with_suffix(".replay.json").write_text(json.dumps(report,indent=2) + "\n")
    print(json.dumps(report,indent=2))
    if report["native_comparison"]["status"] == "mismatch":
        raise SystemExit(1)

if __name__ == "__main__":
    main()
