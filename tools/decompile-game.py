#!/usr/bin/env python3
"""Export no-CD route assembly and, when available, Ghidra pseudocode."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import platform
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parent.parent
EXPECTED_HASH = "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168"
FUNCTIONS = {
    "route_request": (0x512800, 0x512895),
    "route_search": (0x54B800, 0x54BE48),
    "expand_neighbors": (0x4EBAE0, 0x4EC779),
    "heuristic": (0x4EC780, 0x4EC8A7),
    "wrap_x": (0x40E290, 0x40E2B8),
    "wrap_y": (0x40E8A0, 0x40E8C8),
    "copy_z": (0x40EEB0, 0x40EEBB),
}


def find_headless(explicit: Path | None) -> Path | None:
    if explicit is not None:
        candidates = [explicit / "support/analyzeHeadless", explicit]
    else:
        candidates = []
        if os.environ.get("GHIDRA_HOME"):
            candidates.append(Path(os.environ["GHIDRA_HOME"]) / "support/analyzeHeadless")
        found = shutil.which("analyzeHeadless")
        if found:
            candidates.append(Path(found))
        # Prefer a workspace-local native build prepared on an earlier run.
        candidates.extend(sorted((REPO / "working/toolchain").glob(
            "ghidra*/support/analyzeHeadless")))
        for launcher in ("ghidra", "ghidraRun"):
            found = shutil.which(launcher)
            if found:
                candidates.append(Path(found).resolve().parent / "support/analyzeHeadless")
        for root in (REPO / "working/toolchain", Path.home() / "Downloads", Path("/opt")):
            candidates.extend(sorted(root.glob("ghidra*/support/analyzeHeadless")))
    return next((p.resolve() for p in candidates if p.is_file() and os.access(p, os.X_OK)), None)


def prepare_native(headless: Path) -> Path:
    """Build the bundled decompiler locally if this ARM64 host lacks one."""
    if platform.system() != "Linux" or platform.machine() not in ("aarch64", "arm64"):
        return headless
    installation = headless.parent.parent
    module = Path("Ghidra/Features/Decompiler")
    if (installation / module / "os/linux_arm_64/decompile").is_file():
        return headless
    source = installation / module / "src/decompile/cpp"
    if not (source / "Makefile").is_file() or not shutil.which("make") or not shutil.which("g++"):
        raise ValueError("ARM64 decompiler missing; bundled C++ source, make and g++ are required")
    parent = REPO / "working/toolchain"
    parent.mkdir(parents=True, exist_ok=True)
    if installation.is_relative_to(parent.resolve()):
        destination = installation
    else:
        destination = Path(tempfile.mkdtemp(prefix=installation.name + "-arm64-", dir=parent))
        # Fresh destination only. The user's installed Ghidra remains untouched.
        shutil.copytree(installation, destination, dirs_exist_ok=True)
    print(f"Building workspace-local ARM64 decompiler: {destination}", flush=True)
    cpp = destination / module / "src/decompile/cpp"
    with (destination / "native-build.log").open("a") as log:
        result = subprocess.run(["make", "-j2", "ARCH_TYPE=", "ghidra_opt"], cwd=cpp,
                                stdout=log, stderr=subprocess.STDOUT)
    if result.returncode != 0:
        raise ValueError(f"Native build failed; inspect {destination / 'native-build.log'}")
    native = destination / module / "os/linux_arm_64"
    native.mkdir(parents=True, exist_ok=True)
    shutil.copy2(cpp / "ghidra_opt", native / "decompile")
    return destination / "support/analyzeHeadless"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", nargs="?", type=Path,
                        default=REPO / "working/game-nocd/Chaos.exe")
    parser.add_argument("--ghidra", type=Path, help="Ghidra directory or analyzeHeadless path")
    parser.add_argument("--all", action="store_true", help="decompile all discovered functions")
    parser.add_argument("--annotated", action="store_true", help="apply reproducible route markup before export")
    parser.add_argument("--disassembly-only", action="store_true", help="use objdump only")
    args = parser.parse_args()
    executable = args.executable.resolve()
    digest = hashlib.sha256(executable.read_bytes()).hexdigest()
    if digest != EXPECTED_HASH:
        raise ValueError(f"Unknown executable SHA-256 {digest}; refusing build-specific addresses")
    objdump = shutil.which("objdump")
    if not objdump:
        raise ValueError("objdump is required")
    headless = find_headless(args.ghidra)
    parent = REPO / "working/decompiled"
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix="nocd-", dir=parent))
    metadata = {"input": str(executable), "sha256": digest,
                "status": "disassembly_only", "functions": FUNCTIONS,
                "annotated": args.annotated,
                "ghidra": str(headless) if headless else None}
    manifest = output / "manifest.json"

    def save_manifest() -> None:
        manifest.write_text(json.dumps(metadata, indent=2) + "\n")

    save_manifest()
    print(f"Evidence directory: {output}", flush=True)
    for name, (start, end) in FUNCTIONS.items():
        result = subprocess.run([objdump, "-d", "-Mintel", f"--start-address={start}",
                                 f"--stop-address={end}", str(executable)],
                                capture_output=True, text=True, check=True)
        (output / f"{name}.asm").write_text(result.stdout)
    if args.disassembly_only:
        print("Disassembly saved; no automatic decompilation requested.")
        return 0
    if not headless:
        print("Error: Ghidra is not installed/found. Disassembly was saved, not decompiled.\n"
              "Extract an official Ghidra release into working/toolchain, then rerun.\n"
              "Alternatively set GHIDRA_HOME or use --ghidra DIRECTORY.\n"
              "On ARM64, Ghidra's native decompiler must support this host; see\n"
              "research/runtime/route-decompilation.md for setup and current findings.",
              file=sys.stderr)
        return 1
    project = output / "project"
    project.mkdir()
    headless = prepare_native(headless)
    metadata["ghidra"] = str(headless)
    command = [str(headless), str(project), "MagicMayhem", "-import", str(executable),
               "-scriptPath", str(REPO / "tools/ghidra")]
    if args.annotated:
        command += ["-postScript", "AnnotateRouteMilestone.java"]
    command += ["-postScript",
               "ExportGameDecompilation.java", str(output), "all" if args.all else "routes",
               "-max-cpu", "2", "-analysisTimeoutPerFile", "600"]
    metadata["command"] = command
    metadata["status"] = "ghidra_running"
    save_manifest()
    # Keep launch support settings, compilation caches and logs inside this run.
    environment = os.environ.copy()
    environment["XDG_CONFIG_HOME"] = str(output / "config")
    environment["XDG_CACHE_HOME"] = str(output / "cache")
    environment["GHIDRA_HEADLESS_JAVA_OPTIONS"] = (
        environment.get("GHIDRA_HEADLESS_JAVA_OPTIONS", "") +
        f" -Dapplication.cachedir={output / 'cache'}" +
        f" -Dapplication.tempdir={output / 'tmp'}")
    with (output / "ghidra.log").open("w") as log:
        result = subprocess.run(command, env=environment, stdout=log, stderr=subprocess.STDOUT)
    report = output / "export-summary.json"
    summary = json.loads(report.read_text()) if report.exists() else None
    annotations_applied = (args.annotated and
        "Applied hash-checked route milestone names, signatures and partial types." in
        (output / "ghidra.log").read_text())
    metadata["annotations_applied"] = annotations_applied
    complete = (result.returncode == 0 and summary is not None
                and summary["succeeded"] > 0 and summary["failed"] == 0
                and (not args.annotated or annotations_applied))
    metadata["status"] = "decompiled" if complete else "ghidra_failed_or_partial"
    metadata["export_summary"] = summary
    metadata["exit_status"] = result.returncode
    metadata["input_unchanged"] = hashlib.sha256(executable.read_bytes()).hexdigest() == digest
    save_manifest()
    if not complete or not metadata["input_unchanged"]:
        print(f"Error: decompilation incomplete; inspect {output / 'ghidra.log'}", file=sys.stderr)
        return 1
    print(f"Exported {summary['succeeded']} functions; see {output / 'functions.tsv'}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Error: {error}", file=sys.stderr)
        raise SystemExit(1)
