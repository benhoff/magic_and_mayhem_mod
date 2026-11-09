#!/usr/bin/env python3
"""Inventory or preserve reviewed working data. Defaults to a non-destructive inventory."""
import argparse
import json
from pathlib import Path
import sqlite3
import subprocess
import sys

from storage_cleanup import apply_plan, inventory, make_plan, read_database, summary, write_database


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    commands = parser.add_subparsers(dest='command')
    inv = commands.add_parser('inventory', help='Read-only review of working storage; never deletes')
    inv.add_argument('--include-worktrees', action='store_true')
    inv.add_argument('--output', type=Path, help='Optional new SQLite inventory')
    inv.add_argument('--max-entries', type=int, default=100000)
    plan = commands.add_parser('plan', help='Hash exact, explicitly reviewed completed execution directories')
    plan.add_argument('--select', action='append', required=True, type=Path)
    plan.add_argument('--completion-reason', required=True)
    plan.add_argument('--dependency-review', required=True)
    plan.add_argument('--output', required=True, type=Path)
    plan.add_argument('--max-entries', type=int, default=100000)
    show = commands.add_parser('show', help='Read a SQLite inventory or immutable plan')
    show.add_argument('database', type=Path)
    apply = commands.add_parser('apply', help='Revalidate a plan; remains dry unless --execute is supplied')
    apply.add_argument('database', type=Path)
    apply.add_argument('--plan-sha256')
    apply.add_argument('--execute', action='store_true')
    apply.add_argument('--scratch', type=Path, help='Existing restore scratch directory, preferably on tmpfs')
    args = parser.parse_args(argv)
    try:
        if getattr(args, 'max_entries', 1) <= 0:
            raise ValueError('--max-entries must be positive')
        if args.command in (None, 'inventory'):
            data = inventory(args.root, include_worktrees=getattr(args, 'include_worktrees', False),
                             max_entries=getattr(args, 'max_entries', 100000))
            output = getattr(args, 'output', None)
            value = dict(summary(data), rows=data['rows'])
            if output:
                value.update(database=str(output.absolute()), sha256=write_database(output, data))
        elif args.command == 'plan':
            data = make_plan(args.root, args.select, args.completion_reason, args.dependency_review,
                             max_entries=args.max_entries)
            sha = write_database(args.output, data)
            value = dict(summary(data), database=str(args.output.absolute()), plan_sha256=sha,
                         dry_run=True, selected=[r['path'] for r in data['rows']])
        elif args.command == 'show':
            sha, data = read_database(args.database)
            value = dict(sha256=sha, data=data)
        else:
            value = apply_plan(args.database, expected_sha=args.plan_sha256, execute=args.execute, scratch=args.scratch)
        print(json.dumps(value, indent=2))
        return 0
    except (OSError, ValueError, KeyError, sqlite3.Error, subprocess.SubprocessError) as error:
        print(json.dumps(dict(success=False, error=str(error))), file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
