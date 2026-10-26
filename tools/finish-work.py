#!/usr/bin/env python3
"""Review, back up, stage and commit one owned batch without publication."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

from coverage_bookkeeping import Workspace, finish


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--db', type=Path)
    parser.add_argument('--base', default='HEAD')
    parser.add_argument('--reason', required=True)
    parser.add_argument('--validation-reason', required=True)
    parser.add_argument('--index-review-reason')
    parser.add_argument('--record-current-promotions', action='store_true',
                        help='Record only declared promoted stages with current complete-source execution proof')
    parser.add_argument('--message', required=True)
    parser.add_argument('--timings', action='store_true', help='Print finish phase durations and return timings_seconds')
    args = parser.parse_args()
    workspace = Workspace(args.root, args.db)
    try:
        result = finish(workspace, args.base, args.reason, args.validation_reason,
                        args.message, args.index_review_reason, args.record_current_promotions, args.timings)
        print(json.dumps(result, indent=2))
    finally:
        workspace.close()


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
