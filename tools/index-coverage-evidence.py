#!/usr/bin/env python3
"""Generate and pin provenance-only dependencies for affected-claim checks."""

import argparse
from pathlib import Path
from coverage_scoped import write_index

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root", type=Path, default=Path(__file__).resolve().parents[1]
    )
    args = parser.parse_args()
    print(write_index(args.root))
