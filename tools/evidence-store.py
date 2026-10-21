#!/usr/bin/env python3
"""Import, verify, freeze, and restore immutable SQLite evidence archives."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

from coverage_evidence import (CONFIG, Store, clean, freeze, hydrate, install_filter,
                               pack_git, opened_store, verify_push)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    sub = parser.add_subparsers(dest='action', required=True)
    p = sub.add_parser('pack-git')
    p.add_argument('--database', type=Path, required=True)
    p = sub.add_parser('verify')
    p.add_argument('--database', type=Path, required=True)
    p = sub.add_parser('freeze')
    p.add_argument('--database', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--tag', required=True)
    sub.add_parser('hydrate')
    sub.add_parser('install-filter')
    sub.add_parser('verify-push')
    p = sub.add_parser('publish')
    p.add_argument('--database', type=Path)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--tag', required=True)
    p = sub.add_parser('clean')
    p.add_argument('path')
    args = parser.parse_args()
    root = args.root.resolve()
    if args.action == 'clean':
        root = Path(subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], text=True).strip())
        clean(root, args.path, sys.stdin.buffer, sys.stdout.buffer)
        return
    if args.action == 'verify-push':
        print(json.dumps({'verified_pointers': verify_push(root, sys.stdin.read())}))
        return
    if args.action == 'pack-git':
        result = pack_git(root, args.database)
    elif args.action == 'verify':
        with opened_store(args.database, readonly=True) as store:
            result = {'objects_verified': store.verify()}
    elif args.action == 'freeze':
        result = freeze(args.database, args.output, args.tag)
    elif args.action == 'publish':
        from evidence_publish import transport, await_publication
        from coverage_bookkeeping import atomic_replace
        database = args.database or root/'working/coverage/evidence-pending.sqlite3'
        manifest = freeze(database, args.output, args.tag)
        assets = [args.output/p['name'] for p in manifest['archives'][0]['parts']]
        assets.append(args.output/CONFIG)
        result = transport(root, args.output/'upload', assets, args.tag)
        verified = await_publication(root, manifest)
        path = root/CONFIG
        previous = path.read_bytes() if path.exists() else None
        current = json.loads(previous) if previous else {'schema': 1, 'archives': []}
        if not any(a['sha256'] == manifest['archives'][0]['sha256'] for a in current['archives']):
            current['archives'].extend(manifest['archives'])
        atomic_replace(path, (json.dumps(current, indent=2)+'\n').encode(), previous)
        result.update(verified_database=str(verified), manifest_updated=True,
                      next='Prepare/record the exact coverage review, then stage the manifest with its pointers.')
    elif args.action == 'install-filter':
        install_filter(root, __file__)
        result = {'installed': True}
    else:
        install_filter(root, __file__)
        result = {'restored': hydrate(root)}
        # Refresh the stat cache through the clean filter after hydration.
        subprocess.run(['git', '-C', str(root), 'update-index', '--refresh'], stdout=subprocess.DEVNULL)
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, KeyError) as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
