#!/usr/bin/env python3
"""Gate coverage changes against a reviewed snapshot or an actual Git revision."""
import argparse
from contextlib import contextmanager, ExitStack
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile
import time

from coverage_gate import BASELINE, REGISTER, REVIEWS, check, changes, draft, load_json, snapshot
from coverage_merge_history import inherited_history
from coverage_reviews import load_journal, read_journal
from coverage_evidence import shared_cache
from evidence_local import local_evidence

ROOT = Path(__file__).resolve().parents[1]


def git(root, *args):
    result = subprocess.run(['git', '-C', str(root), *args], check=True, capture_output=True)
    return result.stdout


def git_snapshot(root, revision):
    """Read a base tree without switching branches or executing its code."""
    commit = git(root, 'rev-parse', '--verify', '--end-of-options', revision + '^{commit}').decode().strip()
    with shared_cache(root), tempfile.TemporaryDirectory(prefix='coverage-git-base-') as temporary:
        target = Path(temporary)
        # Stream the archive instead of buffering several GiB before extraction.
        # Keep stderr off a pipe so an export failure cannot deadlock the reader.
        command = ['git', '-C', str(root), 'archive', '--format=tar', commit]
        with tempfile.TemporaryFile() as errors:
            export = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors)
            try:
                with tarfile.open(fileobj=export.stdout, mode='r|') as tree:
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
                            with output.open('wb') as destination:
                                # File members can also be large; bound the copy.
                                shutil.copyfileobj(tree.extractfile(member), destination, 1024 * 1024)
                export.stdout.close()
                if export.wait():
                    errors.seek(0)
                    raise subprocess.CalledProcessError(export.returncode, command, stderr=errors.read())
            finally:
                export.stdout.close()
                if export.poll() is None:
                    export.terminate()
                export.wait()
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
    with shared_cache(root), tempfile.TemporaryDirectory(prefix='coverage-git-index-') as temporary:
        target = Path(temporary)
        # The evidence smudge direction is identity. Avoid one filter process
        # per report (and retain exact staged pointer bytes for this check).
        subprocess.run(['git', '-C', str(root),
                        '-c', 'filter.mm-evidence.process=',
                        '-c', 'filter.mm-evidence.smudge=',
                        '-c', 'filter.mm-evidence.required=false',
                        'checkout-index', '--prefix=' + temporary + '/',
                        '-z', '--stdin'], input=b'\0'.join(paths) + b'\0' if paths else b'',
                       check=True, capture_output=True)
        yield target


def write_new(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('x') as out:
        json.dump(value, out, indent=2, sort_keys=True)
        out.write('\n')


def main():
    global ROOT
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT,
                        help='Repository/worktree checked using this current tooling')
    source = parser.add_mutually_exclusive_group()
    source.add_argument('--base', help='Actual Git base commit; same-change baseline edits cannot reset the comparison')
    source.add_argument('--baseline', type=Path, help='Reviewed local baseline (default: coverage/gate-baseline.json)')
    source.add_argument('--freeze-baseline', type=Path, help='Create a reviewed accounting snapshot; refuses overwrite')
    parser.add_argument('--bootstrap', action='store_true', help='Permit initial adoption when --base predates the baseline')
    parser.add_argument('--staged', action='store_true', help='Check the Git index instead of working files (pre-commit)')
    parser.add_argument('--reviews', type=Path)
    parser.add_argument('--json', type=Path, help='Write a new full gate report')
    parser.add_argument('--draft-review', type=Path, help='Write a hash-bound draft requiring reasons before acceptance')
    parser.add_argument('--timings', action='store_true', help='Print phase durations to stderr and include them in the JSON report')
    args = parser.parse_args()
    ROOT = args.root.resolve()
    with local_evidence(ROOT), ExitStack() as stack:
        return run(args, stack)


def run(args, stack):
    timings = {}

    def measure(name, operation):
        started = time.perf_counter()
        try:
            return operation()
        finally:
            timings[name] = time.perf_counter() - started
            if args.timings:
                print(f'Coverage timing: {name}: {timings[name]:.3f}s', file=sys.stderr, flush=True)

    try:
        for path in (args.freeze_baseline, args.json, args.draft_review):
            if path and path.exists():
                raise ValueError(f'Refusing to overwrite: {path}')
        if args.bootstrap and not args.base:
            raise ValueError('--bootstrap requires --base')
        if args.staged and (args.freeze_baseline or args.baseline or args.reviews):
            raise ValueError('--staged uses only staged baseline/reviews and cannot freeze a baseline')
        current_root = measure('staged_tree', lambda: stack.enter_context(staged_tree(ROOT))) if args.staged else ROOT
        current, audit_report = measure('current_snapshot', lambda: snapshot(current_root))
        # Freeze all index-owned metadata before releasing its materialization.
        # Only one full tree is needed at a time, even for a Git-base check.
        reviews = None
        if not args.freeze_baseline:
            reviews = (read_journal(lambda name: args.reviews.read_bytes() if name == REVIEWS
                                   else (current_root/name).read_bytes())[0] if args.reviews
                       else load_journal(current_root)[0])
            if args.bootstrap:
                staged_baseline = load_json(current_root, BASELINE)
            elif not args.base:
                path = args.baseline or current_root/BASELINE
                before = json.loads(path.read_text())
                base_label = str(path)
        stack.close()
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
                before = staged_baseline
                base_label = commit + ' (initial adoption)'
            else:
                before, base_label = measure('git_base_snapshot', lambda: git_snapshot(ROOT, args.base))
        history = measure('merge_history', lambda: inherited_history(ROOT, base_label, current, reviews, args.staged)) if args.base and not args.bootstrap else None
        report = measure('review_check', lambda: check(before, current, reviews, audit_report, history))
        report['base'] = base_label
        if args.timings:
            report['timings_seconds'] = timings
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
