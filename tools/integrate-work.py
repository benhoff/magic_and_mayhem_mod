#!/usr/bin/env python3
"""Preview, or explicitly prepare/check/promote ready tips under one admission."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from integration_batch import preview, execute


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--tip', action='append', required=True)
    parser.add_argument('--execute', action='store_true')
    parser.add_argument('--branch')
    parser.add_argument('--report')
    parser.add_argument('--db', type=Path)
    parser.add_argument('--reason', default='')
    parser.add_argument('--validation-reason', default='')
    parser.add_argument('--index-review-reason', default='')
    parser.add_argument('--history-reason', default='')
    parser.add_argument('--behavior', action='append', default=[])
    parser.add_argument('--expected-master')
    parser.add_argument('--message', default='')
    args = parser.parse_args()
    if not args.execute:
        print(json.dumps(preview(args.root, args.tip), indent=2))
        return
    if not all((args.branch, args.report, args.db)):
        parser.error('--execute needs --branch, --report and --db')
    value = execute(args.root, args.tip, branch=args.branch, report=args.report,
                    database=args.db, reason=args.reason, validation_reason=args.validation_reason,
                    index_reason=args.index_review_reason, message=args.message,
                    history_reason=args.history_reason, behaviors=args.behavior,
                    expected_master=args.expected_master)
    print(json.dumps(value, indent=2))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
