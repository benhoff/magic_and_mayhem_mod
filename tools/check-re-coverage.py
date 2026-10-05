#!/usr/bin/env python3
"""Gate coverage changes against a reviewed snapshot or an actual Git revision."""
import argparse
from contextlib import contextmanager, ExitStack
import io
import json
from pathlib import Path
import subprocess
import tarfile
import tempfile

from coverage_gate import BASELINE, REGISTER, REVIEWS, check, changes, draft, load_json, snapshot

ROOT = Path(__file__).resolve().parents[1]


def git(root, *args):
    result = subprocess.run(['git', '-C', str(root), *args], check=True, capture_output=True)
    return result.stdout


def git_snapshot(root, revision):
    """Read a base tree without switching branches or executing its code."""
    commit = git(root, 'rev-parse', '--verify', '--end-of-options', revision + '^{commit}').decode().strip()
    archive = git(root, 'archive', '--format=tar', commit)
    with tempfile.TemporaryDirectory(prefix='coverage-git-base-') as temporary:
        target = Path(temporary)
        with tarfile.open(fileobj=io.BytesIO(archive)) as tree:
            for member in tree:
                path = Path(member.name)
                if path.is_absolute() or '..' in path.parts:
                    raise ValueError('Unsafe path in Git archive')
                if path.parts[0] in {'original', 'working', '.git'}:
                    continue
                if member.issym() or member.islnk():
                    raise ValueError(f'Git base contains unsupported symlink: {member.name}')
                if member.isfile():
                    output = target/path
                    output.parent.mkdir(parents=True, exist_ok=True)
                    output.write_bytes(tree.extractfile(member).read())
        if not (target/BASELINE).is_file():
            raise ValueError('Base revision predates the gate baseline; use --bootstrap only for initial adoption')
        value, _ = snapshot(target)
    return value, commit


@contextmanager
def staged_tree(root):
    """Copy only index entries, without changing the index or reading game media."""
    entries = git(root, 'ls-files', '--stage', '-z').split(b'\0')
    paths = []
    for entry in filter(None, entries):
        metadata, name = entry.split(b'\t', 1)
        mode, _, stage = metadata.split()
        if stage != b'0':
            raise ValueError('Resolve index conflicts before checking coverage')
        path = Path(name.decode('utf-8', errors='surrogateescape'))
        if path.is_absolute() or '..' in path.parts:
            raise ValueError('Unsafe path in Git index')
        if path.parts[0] in {'original', 'working', '.git'}:
            continue
        if mode not in {b'100644', b'100755'}:
            raise ValueError(f'Unsupported staged file mode: {path}')
        paths.append(name)
    with tempfile.TemporaryDirectory(prefix='coverage-git-index-') as temporary:
        target = Path(temporary)
        subprocess.run(['git', '-C', str(root), 'checkout-index', '--prefix=' + temporary + '/',
                        '-z', '--stdin'], input=b'\0'.join(paths) + b'\0' if paths else b'',
                       check=True, capture_output=True)
        yield target


def write_new(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('x') as out:
        json.dump(value, out, indent=2, sort_keys=True)
        out.write('\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group()
    source.add_argument('--base', help='Actual Git base commit; same-change baseline edits cannot reset the comparison')
    source.add_argument('--baseline', type=Path, help='Reviewed local baseline (default: coverage/gate-baseline.json)')
    source.add_argument('--freeze-baseline', type=Path, help='Create a reviewed accounting snapshot; refuses overwrite')
    parser.add_argument('--bootstrap', action='store_true', help='Permit initial adoption when --base predates the baseline')
    parser.add_argument('--staged', action='store_true', help='Check the Git index instead of working files (pre-commit)')
    parser.add_argument('--reviews', type=Path)
    parser.add_argument('--json', type=Path, help='Write a new full gate report')
    parser.add_argument('--draft-review', type=Path, help='Write a hash-bound draft requiring reasons before acceptance')
    args = parser.parse_args()
    with ExitStack() as stack:
        return run(args, stack)


def run(args, stack):
    try:
        for path in (args.freeze_baseline, args.json, args.draft_review):
            if path and path.exists():
                raise ValueError(f'Refusing to overwrite: {path}')
        if args.bootstrap and not args.base:
            raise ValueError('--bootstrap requires --base')
        if args.staged and (args.freeze_baseline or args.baseline or args.reviews):
            raise ValueError('--staged uses only staged baseline/reviews and cannot freeze a baseline')
        current_root = stack.enter_context(staged_tree(ROOT)) if args.staged else ROOT
        current, audit_report = snapshot(current_root)
        if args.freeze_baseline:
            if args.draft_review or args.json:
                raise ValueError('--freeze-baseline cannot be combined with report/draft output')
            write_new(args.freeze_baseline, current)
            print(json.dumps({'baseline': str(args.freeze_baseline), 'sources': len(current['sources']), 'retained_gaps': len(current['gaps'])}, indent=2))
            return 0
        if args.base:
            if args.bootstrap:
                commit = git(ROOT, 'rev-parse', '--verify', '--end-of-options', args.base + '^{commit}').decode().strip()
                exists = subprocess.run(['git', '-C', str(ROOT), 'cat-file', '-e', commit + ':' + BASELINE], capture_output=True).returncode == 0
                if exists:
                    raise ValueError('--bootstrap is forbidden once the base has a gate baseline')
                before = load_json(current_root, BASELINE)
                base_label = commit + ' (initial adoption)'
            else:
                before, base_label = git_snapshot(ROOT, args.base)
        else:
            path = args.baseline or current_root/BASELINE
            before = json.loads(path.read_text())
            base_label = str(path)
        reviews = json.loads((args.reviews or current_root/REVIEWS).read_text())
        report = check(before, current, reviews, audit_report)
        report['base'] = base_label
        if args.json:
            write_new(args.json, report)
        if args.draft_review:
            write_new(args.draft_review, draft(changes(before, current)))
        print(json.dumps({'errors': report['errors'], 'warnings': report['warnings'], 'counts': report['counts'],
                          'base': base_label, 'draft': str(args.draft_review) if args.draft_review else None}, indent=2))
        return 0 if args.draft_review else int(bool(report['errors']))
    except (ValueError, KeyError, TypeError, OSError, subprocess.CalledProcessError, tarfile.TarError) as exc:
        print(json.dumps({'errors': [str(exc)]}, indent=2))
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
