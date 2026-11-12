#!/usr/bin/env python3
"""Compare complete local screen acquisition callers and controlled lifecycle composition."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import resource
import shlex
import subprocess
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import evidence_digest, read_json, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168"
IDS = [
    "RS.main-screen-acquisition",
    "RS.world-screen-acquisition",
    "RS.report-screen-acquisition",
]
RANGES = [
    (0x4A6F70, 0x4A7024),
    (0x4A7110, 0x4A756D),
    (0x46A460, 0x46A92E),
    (0x473D70, 0x4745C8),
    (0x474A50, 0x474B03),
    (0x4A7030, 0x4A7107),
    (0x473C10, 0x473D01),
    (0x46AB30, 0x46AE8C),
    (0x4CACB0, 0x4CACF1),
    (0x4A6A60, 0x4A6AFD),
    (0x4A6B00, 0x4A6B67),
    (0x4CAB50, 0x4CAB69),
    (0x4CAB70, 0x4CABCC),
    (0x4748A0, 0x47493A),
]


def local_flow(pe, start, end):
    decoder = Decoder()
    todo, rows = [start], {}
    try:
        while todo:
            at = todo.pop()
            if at in rows:
                continue
            if not start <= at < end:
                raise ValueError("Local edge escapes " + hex(at))
            row = decoder.decode(pe.read(at, min(15, end - at)), at)
            rows[at] = row
            mnemonic, operand = row["mnemonic"], row["operand"]
            if mnemonic.startswith("ret"):
                continue
            if mnemonic.startswith("j"):
                if not operand.startswith("0x"):
                    raise ValueError("Unresolved local branch")
                todo.append(int(operand, 16))
            if mnemonic != "jmp":
                todo.append(at + row["size"])
        occupied = {
            a
            for row in rows.values()
            for a in range(row["address"], row["address"] + row["size"])
        }
        if occupied != set(range(start, end)) or len(occupied) != sum(
            r["size"] for r in rows.values()
        ):
            raise ValueError("Local instruction census mismatch " + hex(start))
        return dict(
            entry=start,
            end=end,
            sha256=hashlib.sha256(pe.read(start, end - start)).hexdigest(),
            instructions=sorted(rows.values(), key=lambda r: r["address"]),
            closed_local_cfg=True,
        )
    finally:
        decoder.close()


def main():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--report", required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if (
        out.exists()
        or not out.is_relative_to(ROOT / "working/tests")
        or (ROOT / args.report).exists()
    ):
        parser.error("Require new owned output and immutable report")
    out.mkdir(parents=True)
    (out / "tmp").mkdir()
    (out / ".retain").write_text(
        "Original screen ownership comparison logs and dependency provenance.\n"
    )
    ids = IDS
    ranges = RANGES
    cpp = "tests/screen-acquisition-reference.cpp"
    reg = read_json(ROOT / "research/runtime/coverage/register.json", root=ROOT)
    behaviors = {b["id"]: b for b in reg["behaviors"]}
    builds = {b["id"]: b for b in reg["builds"]}
    sources = {
        name: evidence_digest(ROOT / name, root=ROOT)
        for name in sorted(set().union(*(required_sources(behaviors[i]) for i in ids)))
    }
    report = dict(
        schema=1,
        success=False,
        original_sha256=BUILD,
        sources=sources,
        claims=[
            dict(behavior=i, contract_sha256=behavior_contract(behaviors[i], builds))
            for i in ids
        ],
        scope=behaviors[ids[0]]["scope"],
        commands=[],
        live_replacement=False,
        provider_allocator_and_crt_endpoints_authored=True,
    )

    def run(label, command):
        print(label, flush=True)
        result = subprocess.run(
            list(map(str, command)),
            cwd=ROOT,
            capture_output=True,
            timeout=900,
            env=os.environ
            | dict(
                TMPDIR=os.environ.get("TMPDIR", str(out / "tmp")),
                ASAN_OPTIONS="detect_leaks=1:halt_on_error=1",
                UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1",
            ),
        )
        payload = result.stdout + b"\n" + result.stderr
        (out / (label + ".log")).write_bytes(payload)
        write_report(
            ROOT,
            str(
                Path(args.report).parent
                / (Path(args.report).stem + "-" + label + ".log")
            ),
            payload,
        )
        report["commands"].append(
            dict(
                label=label,
                exit=result.returncode,
                sha256=hashlib.sha256(payload).hexdigest(),
            )
        )
        if result.returncode:
            if label == "original":
                diagnostic = subprocess.run(
                    [
                        "gdb",
                        "-q",
                        "-batch",
                        "-ex",
                        "set debuginfod enabled off",
                        "-ex",
                        "run",
                        "-ex",
                        "bt",
                        "-ex",
                        "p profile",
                        "-ex",
                        "p mutation",
                        "-ex",
                        "p nextAllocation",
                        "-ex",
                        "p calls.size()",
                        "-ex",
                        "x/2wx 0x634754",
                        "-ex",
                        "x/4wx *(unsigned int*)0x634754",
                        "-ex",
                        "x/8wx $esp+16",
                        "-ex",
                        "info registers",
                        "--args",
                        out / label,
                        ROOT / "original/Arcane_Nocd/Chaos.exe",
                    ],
                    capture_output=True,
                    timeout=90,
                )
                write_report(
                    ROOT,
                    str(
                        Path(args.report).parent / (Path(args.report).stem + "-gdb.log")
                    ),
                    diagnostic.stdout + diagnostic.stderr,
                )
            raise ValueError(
                label
                + " exit "
                + str(result.returncode)
                + ": "
                + payload.decode(errors="replace")[-3500:]
            )
        return result.stdout.decode()

    try:
        run("manifest-before", [ROOT / "tools/original-manifest.sh", "verify"])
        image = ROOT / "original/Arcane_Nocd/Chaos.exe"
        pe = PE32(image.read_bytes())
        if pe.sha256 != BUILD:
            raise ValueError("Unsupported original executable")
        report["flow"] = [local_flow(pe, a, b) for a, b in ranges]
        report["original_entry_anchors"] = {
            hex(a): pe.read(a, 16).hex() for a, b in ranges
        }
        for label, flags in [
            ("original", ["-m32", "-O2", "-DMNM_ORIGINAL_REFERENCE"]),
            (
                "sanitized",
                [
                    "-m32",
                    "-O1",
                    "-DMNM_ORIGINAL_REFERENCE",
                    "-fsanitize=address,undefined",
                    "-fno-omit-frame-pointer",
                ],
            ),
            (
                "native64",
                ["-O1", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"],
            ),
        ]:
            run(
                label + "-build",
                [
                    "g++",
                    "-std=c++17",
                    "-mstackrealign",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-fno-pie",
                    "-no-pie",
                    "-g",
                    *flags,
                    "-MMD",
                    "-MF",
                    out / (label + ".d"),
                    ROOT / cpp,
                    "-o",
                    out / label,
                ],
            )
            for token in shlex.split(
                (out / (label + ".d")).read_text().replace("\\\n", " ").split(":", 1)[1]
            ):
                path = Path(token).resolve()
                if (
                    path.is_relative_to(ROOT)
                    and path.relative_to(ROOT).as_posix() not in sources
                ):
                    raise ValueError("Undeclared compiler source " + token)
            report[label] = json.loads(
                run(label, [out / label, *([image] if label != "native64" else [])])
            )
        if (
            report["original"] != report["sanitized"]
            or report["original"] != report["native64"]
        ):
            raise ValueError("Cross-configuration mismatch")
        report["success"] = True
    except (OSError, ValueError, AssertionError, subprocess.SubprocessError) as exc:
        report["error"] = str(exc)
    finally:
        try:
            run("manifest-after", [ROOT / "tools/original-manifest.sh", "verify"])
            report["original_manifest_verified_before_after"] = True
        except (OSError, ValueError, subprocess.SubprocessError) as exc:
            report.update(success=False, manifest_error=str(exc))
        report["sources_stable"] = all(
            evidence_digest(ROOT / n, root=ROOT) == h for n, h in sources.items()
        )
        report["success"] &= report["sources_stable"]
        digest = write_report(
            ROOT, args.report, (json.dumps(report, indent=2) + "\n").encode()
        )
        print(
            json.dumps(
                dict(
                    success=report["success"], sha256=digest, error=report.get("error")
                )
            ),
            flush=True,
        )
    return int(not report["success"])


if __name__ == "__main__":
    raise SystemExit(main())
