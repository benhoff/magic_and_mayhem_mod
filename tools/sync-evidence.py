#!/usr/bin/env python3
"""Publish missing immutable evidence, commit availability metadata, optionally push."""
import argparse
import io
import json
from pathlib import Path
import subprocess
import sys

from coverage_evidence import Resolver, Store, artifact, config, fetch_archive, identity
from coverage_bookkeeping import Workspace, finish


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args])


def collect_missing(root, revision, database):
    """Preserve new-history object versions, including ones absent from HEAD."""
    published = []
    resolver = Resolver(root)
    target = None
    process = None
    try:
        for entry in (config(root) or {}).get('archives', []):
            published.append(Store(fetch_archive(root, entry), readonly=True))
        objects = git(root, 'rev-list', '--objects', revision, '--not', '--remotes=origin')
        candidates = {}
        for row in objects.splitlines():
            fields = row.split(b' ', 1)
            if len(fields) == 2 and artifact(fields[1].decode()):
                candidates[fields[0].decode()] = fields[1].decode()
        checks = subprocess.check_output(['git', '-C', str(root), 'cat-file', '--batch-check'],
                                         input=('\n'.join(candidates) + '\n').encode() if candidates else b'')
        for line in checks.splitlines():
            fields = line.split()
            if len(fields) != 3 or fields[1] != b'blob' or int(fields[2]) > 160:
                raise ValueError('Unconverted evidence in outgoing history; install the evidence filter')
        target = Store(database)
        process = subprocess.Popen(['git', '-C', str(root), 'cat-file', '--batch'],
                                   stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        missing = set()
        for oid in candidates:
            process.stdin.write((oid + '\n').encode()); process.stdin.flush()
            header = process.stdout.readline().split()
            if len(header) != 3 or header[1] != b'blob':
                raise ValueError('Invalid evidence Git object')
            data = process.stdout.read(int(header[2]))
            if process.stdout.read(1) != b'\n':
                raise ValueError('Invalid evidence object framing')
            ident = identity(data)
            if ident is None:
                raise ValueError('Invalid evidence pointer: ' + candidates[oid])
            sha, size = ident
            if sha in missing or any(store.has(sha, size) for store in published):
                continue
            payload = io.BytesIO()
            resolver.copy(sha, size, payload); payload.seek(0)
            with target.db:
                if target.put(payload, size) != sha:
                    raise ValueError('Evidence changed while synchronizing')
            missing.add(sha)
        verified = target.verify()
        return {'missing_objects': len(missing), 'objects_verified': verified}
    finally:
        if process:
            process.stdin.close(); process.stdout.close()
            code = process.wait()
            if code:
                raise subprocess.CalledProcessError(code, process.args)
        if target:
            target.close()
        resolver.close()
        for store in published:
            store.close()


def publish_missing(root, database, output, tag):
    subprocess.run([sys.executable, str(root/'tools/evidence-store.py'), '--root', str(root),
                    'publish', '--database', str(database), '--output', str(output), '--tag', tag], check=True)


def push_branch(root, branch):
    subprocess.run(['git', '-C', str(root), 'push', 'origin', 'HEAD:refs/heads/' + branch], check=True)


def synchronize(root, output, tag, push=False, database=None):
    root = Path(root).resolve()
    output = (root/output).resolve()
    if not output.is_relative_to(root/'working') or output.exists():
        raise ValueError('Use a new output directory inside this worktree working/')
    if git(root, 'status', '--porcelain', '--untracked-files=normal').strip():
        raise ValueError('Synchronize from a clean, owned worktree after the local commit')
    head = git(root, 'rev-parse', 'HEAD').decode().strip()
    branch = None
    if push:
        branch = git(root, 'symbolic-ref', '--short', 'HEAD').decode().strip()
    output.mkdir(parents=True)
    (output/'.retain').write_text('Evidence synchronization inputs and publication receipts.\n')
    collected = collect_missing(root, head, output/'missing.sqlite3')
    if collected['missing_objects']:
        publish_missing(root, output/'missing.sqlite3', output/'publication', tag)
        if git(root, 'rev-parse', 'HEAD').decode().strip() != head:
            raise ValueError('HEAD changed during publication; retain the manifest and review before pushing')
        workspace = Workspace(root, database)
        try:
            finish(workspace, 'HEAD', 'Reviewed verified SQLite archive availability metadata.',
                   'Storage availability only; no engine behavior or milestone is renewed.',
                   'Record verified evidence archive availability')
        finally:
            workspace.close()
    if push:
        # Publication is explicit here; ordinary Git pushes do not require it.
        push_branch(root, branch)
    return dict(collected, pushed=push, publication='verified' if collected['missing_objects'] else 'already available')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--db', type=Path)
    parser.add_argument('--push', action='store_true', help='Also push the current branch after verified synchronization')
    args = parser.parse_args()
    print(json.dumps(synchronize(args.root, args.output, args.tag, args.push, args.db), indent=2))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
