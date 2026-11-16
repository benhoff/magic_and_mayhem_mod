#!/usr/bin/env python3
"""Review finite evidence retirement; explicit pruning never runs in a hook."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from evidence_lifecycle import integrated, prepare_retirement, export_retirement, prune_store


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    commands = parser.add_subparsers(dest='action', required=True)
    clock = commands.add_parser('integrated', help='Observe master inclusion; start the review clock now')
    clock.add_argument('--commit', action='append', required=True)
    plan = commands.add_parser('prepare', help='Verify original report bytes and prepare a SQLite retirement plan')
    plan.add_argument('--evidence', action='append', required=True)
    plan.add_argument('--commit', required=True)
    for key in ('owner', 'reason', 'consumer-review', 'result', 'reproduction'):
        plan.add_argument('--' + key, required=True)
    export = commands.add_parser('export', help='Export authored declarations for normal commit/integration review')
    export.add_argument('--plan', required=True)
    prune = commands.add_parser('prune-store', help='Preview/explicitly prune expired payloads from a stopped plain pending store')
    prune.add_argument('--database', type=Path, required=True)
    prune.add_argument('--consumer-review', required=True)
    prune.add_argument('--pin-review', default='')
    prune.add_argument('--execute', action='store_true')
    args = parser.parse_args()
    if args.action == 'integrated':
        result = integrated(args.root, args.commit)
    elif args.action == 'prepare':
        result = prepare_retirement(args.root, args.evidence, args.commit, owner=args.owner,
            reason=args.reason, consumer_review=args.consumer_review, result=args.result,
            reproduction=args.reproduction)
    elif args.action == 'export':
        result = export_retirement(args.root, args.plan)
    else:
        result = prune_store(args.root, args.database, consumer_review=args.consumer_review,
                             pin_review=args.pin_review, execute=args.execute)
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
