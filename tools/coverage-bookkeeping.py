#!/usr/bin/env python3
"""Prepare, record, query and export coverage reviews through local SQLite."""
import argparse
import json
from pathlib import Path
import sqlite3
import subprocess
import sys

from coverage_bookkeeping import Workspace, prepare, record


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--db', type=Path, help='Alternate SQLite path within this worktree\'s working/ directory')
    commands = parser.add_subparsers(dest='command', required=True)
    commands.add_parser('sync', help='Losslessly import Git-tracked receipt history')
    prep = commands.add_parser('prepare', help='Discover changed files, affected contracts and pending gaps')
    prep.add_argument('--base', default='HEAD')
    prep.add_argument('--path', action='append', default=[], help='Exact changed file; repeat to scope a review')
    prep.add_argument('--behavior', action='append', default=[], help='Exact changed behavior; repeat as needed')
    prep.add_argument('--index-review-reason', help='Acknowledge reviewed source links and refresh/pin the census automatically')
    rec = commands.add_parser('record', help='Recheck a draft, record supplied reasons and export automatically')
    rec.add_argument('draft', nargs='?', help='Prepared draft ID; defaults to the latest draft in this workspace')
    rec.add_argument('--reason', required=True)
    rec.add_argument('--validation-reason', required=True)
    rec.add_argument('--validation', choices=['pending', 'recorded', 'not_required'], default='pending')
    rec.add_argument('--evidence', action='append', default=[])
    rec.add_argument('--stage', action='append', default=[], choices=['implementation', 'comparison', 'integration', 'replacement'])
    rec.add_argument('--no-export', action='store_true', help='Keep accepted receipt locally until export')
    commands.add_parser('export', help='Append accepted local receipts to the existing text journal')
    query = commands.add_parser('find', help='Query receipt history by exact path, behavior and/or validation status')
    query.add_argument('--path')
    query.add_argument('--behavior')
    query.add_argument('--validation', choices=['pending', 'recorded', 'not_required'])
    query.add_argument('--limit', type=int, default=25)
    args = parser.parse_args()
    workspace = None
    try:
        workspace = Workspace(args.root, args.db)
        if args.command == 'prepare':
            result = prepare(workspace, args.base, args.path, args.behavior, args.index_review_reason)
        elif args.command == 'record':
            result = record(workspace, args.draft, args.reason, args.validation_reason,
                            args.validation, args.evidence, args.stage, not args.no_export)
        else:
            with workspace.locked():
                if args.command == 'export':
                    result = {'exported': workspace.export()}
                else:
                    journal = workspace.sync()
                    if args.command == 'find':
                        if args.limit < 1:
                            raise ValueError('--limit must be positive')
                        result = {'reviews': workspace.find(path=args.path, behavior=args.behavior,
                                                           status=args.validation, limit=args.limit)}
                    else:
                        result = {'database': str(workspace.path), 'imported': len(journal['reviews']),
                                  'awaiting_export': len(workspace.pending(journal))}
        print(json.dumps(result, indent=2))
        return 0
    except (ValueError, KeyError, TypeError, OSError, sqlite3.Error, subprocess.CalledProcessError) as exc:
        print(json.dumps({'errors': [str(exc)]}, indent=2), file=sys.stderr)
        return 1
    finally:
        if workspace:
            workspace.close()


if __name__ == '__main__':
    raise SystemExit(main())
