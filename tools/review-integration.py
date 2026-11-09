#!/usr/bin/env python3
"""Dry-first metadata reconciliation and exact committed-history review."""
import argparse
import json
from pathlib import Path
import sys
import subprocess
from integration_review import reconcile, review_history


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    sub = parser.add_subparsers(dest='command', required=True)
    merge = sub.add_parser('metadata')
    history = sub.add_parser('history')
    for command in (merge, history):
        command.add_argument('--execute', action='store_true')
        command.add_argument('--reason', default='')
        command.add_argument('--report')
    history.add_argument('--base', required=True)
    history.add_argument('--tip', action='append', required=True)
    history.add_argument('--db', type=Path)
    history.add_argument('--behavior', action='append', default=[])
    history.add_argument('--pending-gap', action='append', default=[])
    args = parser.parse_args()
    if args.command == 'metadata':
        value = reconcile(args.root, execute=args.execute, reason=args.reason, report=args.report)
    else:
        value = review_history(args.root, args.base, args.tip, record=args.execute,
                               reason=args.reason, database=args.db, report=args.report,
                               behaviors=args.behavior, pending_gaps=args.pending_gap)
    print(json.dumps(value, indent=2))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(2)
