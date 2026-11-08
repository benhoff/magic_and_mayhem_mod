#!/usr/bin/env python3
"""Explicit worktree compaction and lossless legacy snapshot union/recovery."""
import argparse
import json
from pathlib import Path
from evidence_objects import configured
import worktree_compaction
import snapshot_consolidation


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    sub = parser.add_subparsers(dest='command', required=True)
    p = sub.add_parser('worktree')
    p.add_argument('path', type=Path)
    p.add_argument('--scratch', type=Path, required=True)
    p.add_argument('--directory', choices=['research', 'tests'], action='append')
    p = sub.add_parser('restore-worktree')
    p.add_argument('--manifest', required=True)
    p.add_argument('--size', type=int, required=True)
    p.add_argument('--output', type=Path, required=True)
    p = sub.add_parser('snapshots')
    p.add_argument('--source', type=Path, action='append', required=True)
    p.add_argument('--destination', type=Path, action='append', required=True)
    p.add_argument('--scratch', type=Path, required=True)
    p.add_argument('--retire', action='store_true')
    p.add_argument('--resume', action='store_true')
    p.add_argument('--primary-minimum-free-gib', type=int, default=100)
    p.add_argument('--replica-minimum-free-gib', type=int, default=90)
    p = sub.add_parser('restore-catalog')
    p.add_argument('--union', type=Path, required=True)
    p.add_argument('--source', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = parser.parse_args()
    if a.command == 'restore-catalog':
        result = dict(path=str(snapshot_consolidation.restore_catalog(a.union, a.source, a.output)))
    elif a.command == 'snapshots':
        if min(a.primary_minimum_free_gib, a.replica_minimum_free_gib) < 0:
            parser.error('Storage floors must be nonnegative')
        class Recorder:
            # Private catalog construction/restoration needs no shared writer
            # lease. Acquire that lease only when publishing custody roots.
            def root(self, *args):
                with configured(a.root) as pair:
                    return pair.root(*args)
        result = snapshot_consolidation.consolidate(a.source, a.destination, Recorder(), a.scratch,
            floors=(a.primary_minimum_free_gib * 2**30, a.replica_minimum_free_gib * 2**30), retire=a.retire, resume=a.resume)
        result = dict(result, sources=[{k: p[k] for k in ('source', 'container_sha256', 'byte_exact_container_restore')} for p in result['sources']])
    else:
        with configured(a.root) as pair:
            if a.command == 'worktree':
                result = worktree_compaction.compact(a.path, a.root, pair, scratch=a.scratch, prefixes=a.directory or ('research', 'tests'))
            elif a.command == 'restore-worktree':
                result = dict(path=str(worktree_compaction.restore(pair, dict(sha256=a.manifest, size=a.size), a.output)))
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
