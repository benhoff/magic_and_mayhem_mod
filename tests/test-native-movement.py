#!/usr/bin/env python3
"""Compare host models to isolated original x86 code; dependency redirects vary by component."""
import argparse
import hashlib
from pathlib import Path
import os
import subprocess
import tempfile

REPO = Path(__file__).resolve().parent.parent
EXPECTED = "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168"

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--component", choices=("movement", "cell", "occupancy", "validity", "cell-validity", "record-query", "cell-support", "boundary", "clearance", "scalar"), default="movement")
    component = parser.parse_args().component
    executable = REPO / "working/game-nocd/Chaos.exe"
    data = executable.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXPECTED:
        raise ValueError("Unsupported executable hash; refusing native reference")
    parent = REPO / "working/tests"
    parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=f"native-{component}-", dir=parent) as temp:
        root = Path(temp)
        snapshot = root / "reference.exe"
        snapshot.write_bytes(data)
        model = REPO / "reconstruction/pathfinding"
        binary = root / "compare"
        subprocess.run([os.environ.get("CXX", "g++"), "-m32", "-std=c++17", "-Wall", "-Wextra",
                        "-Werror", "-g", "-DMNM_NATIVE_REFERENCE", "-I", str(model),
                        *[str(model / name) for name in ("route_request.cpp", "route_search.cpp",
                            "route_neighbors.cpp", "route_creature_acceptance.cpp", "route_movement.cpp", "route_cell.cpp", "route_occupancy.cpp", "route_validity.cpp", "route_cell_validity.cpp", "route_record_query.cpp", "route_cell_support.cpp", "route_boundary.cpp", "route_clearance.cpp", "route_scalar.cpp")],
                        str(REPO / f"tests/route-{component}-test.cpp"), "-o", str(binary)], check=True)
        subprocess.run([str(binary), str(snapshot)], check=True, timeout=30)
        if component == "clearance":
            for option in ("--cycle-any", "--cycle-tall"):
                try:
                    subprocess.run([str(binary), str(snapshot), option], check=True, timeout=1)
                except subprocess.TimeoutExpired:
                    print(f"Confirmed original cyclic scan: {option} (isolated one-second timeout).")
                else:
                    raise AssertionError("Expected original oversized empty scan to keep cycling")
        if component == "boundary":
            subprocess.run([str(binary), str(snapshot), "--full-chain"], check=True, timeout=30)
    if hashlib.sha256(executable.read_bytes()).hexdigest() != EXPECTED:
        raise ValueError("Input changed during native reference test")
    if component == "scalar":
        print("Original x86 scalar, table lookup, modifiers and acceleration chain match both output DWORDs.")
    elif component == "clearance":
        print("Both original x86 clearance predicates and the complete movement-helper chain match model results.")
    elif component == "cell-support":
        print("Original x86 cell support, aggregation, validity, and movement chains match model results and preserve fixture bytes.")
    elif component == "cell-validity":
        print("Original x86 cell validity, footprint, and movement chains match model results and preserve fixture bytes.")
    elif component == "occupancy":
        print("Original x86 occupancy routine and integrated footprint match model results and preserve map bytes.")
    else:
        print(f"Original x86 {component} routine matches model results and lower-helper call order.")

if __name__ == "__main__":
    main()
