"""Explicit, restored custody for cold tracked directories in merged worktrees."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tempfile
import time

from evidence_objects import _sync
from evidence_workspace import idle


def git(root, *args):
    # Compare physical historical sources with their exact index blobs. Current
    # evidence cleaning would rewrite old raw-report identities during status.
    return subprocess.check_output(['git', '-C', str(root),
        '-c', 'filter.mm-evidence.clean=cat', '-c', 'filter.mm-evidence.smudge=cat',
        '-c', 'filter.mm-evidence.process=', '-c', 'filter.mm-evidence.required=false',
        *args], stderr=subprocess.PIPE)


def identity(path):
    s = path.lstat()
    return [s.st_dev, s.st_ino, s.st_size, s.st_mtime_ns, s.st_ctime_ns]


def source_idle(root):
    idle(root)
    prefix = os.fsencode(str(root) + '/')
    for proc in Path('/proc').iterdir():
        if not proc.name.isdecimal() or int(proc.name) == os.getpid():
            continue
        try:
            arguments = (proc / 'cmdline').read_bytes().split(b'\0')
        except OSError:
            continue
        if any(a.startswith(prefix) for a in arguments):
            raise ValueError('Active command names this source worktree')


def inspect(root, main, prefixes=('research', 'tests'), recent_hours=24):
    root, main = Path(root).absolute(), Path(main).absolute()
    if root == main or not (root / '.git').is_file() or (root / '.git').is_symlink():
        raise ValueError('Select a linked worktree, excluding the main checkout')
    if any(p.is_symlink() for p in (root, *root.parents)) or (root / '.retain').exists():
        raise ValueError('Aliased or source-pinned worktree')
    gitdir = Path(os.fsdecode(git(root, 'rev-parse', '--absolute-git-dir')).strip())
    if any((gitdir / name).exists() for name in ('locked', 'index.lock', 'MERGE_HEAD', 'CHERRY_PICK_HEAD', 'REVERT_HEAD', 'rebase-apply', 'rebase-merge', 'sequencer')):
        raise ValueError('Locked worktree or unfinished Git operation')
    if git(root, 'rev-parse', '--git-common-dir') != git(main, 'rev-parse', '--git-common-dir'):
        # The main checkout can return a relative .git directory.
        common = lambda p: (p / os.fsdecode(git(p, 'rev-parse', '--git-common-dir')).strip()).resolve()
        if common(root) != common(main):
            raise ValueError('Worktree belongs to another repository')
    head = git(root, 'rev-parse', 'HEAD').decode().strip()
    git(main, 'merge-base', '--is-ancestor', head, 'master')
    if git(root, 'status', '--porcelain', '--untracked-files=no'):
        raise ValueError('Modified worktree')
    sparse = subprocess.run(['git', '-C', str(root), 'config', '--bool', 'core.sparseCheckout'], capture_output=True)
    if sparse.returncode not in (0, 1):
        raise ValueError('Cannot read sparse checkout state')
    if sparse.stdout.strip() == b'true':
        raise ValueError('Already sparse; retain its existing recovery marker')
    prefixes = tuple(prefixes)
    if not prefixes or any(p not in ('research', 'tests') for p in prefixes):
        raise ValueError('Only research and tests tracked directories are supported')
    source_idle(root)
    cutoff = time.time_ns() - int(recent_hours * 3600 * 10**9)
    rows = []
    for entry in git(root, 'ls-files', '-s', '-z', '--', *prefixes).split(b'\0'):
        if not entry:
            continue
        header, raw = entry.split(b'\t', 1)
        mode, blob, stage = header.decode().split()
        name = os.fsdecode(raw)
        path = root / name
        if mode not in ('100644', '100755') or stage != '0' or 'original' in Path(name).parts:
            raise ValueError('Unsupported tracked member: ' + name)
        if any(p.is_symlink() for p in (path, *path.parents) if p == root or root in p.parents):
            raise ValueError('Tracked member traverses an alias: ' + name)
        s = path.lstat()
        if not stat.S_ISREG(s.st_mode) or s.st_nlink != 1 or s.st_mtime_ns > cutoff:
            raise ValueError('Recent, shared or nonregular tracked member: ' + name)
        rows.append(dict(path=name, git_blob=blob, identity=identity(path), size=s.st_size,
                         mode=stat.S_IMODE(s.st_mode), mtime_ns=s.st_mtime_ns,
                         uid=s.st_uid, gid=s.st_gid, allocated=s.st_blocks * 512,
                         xattrs={k: os.getxattr(path, k).hex() for k in sorted(os.listxattr(path))}))
    for prefix in prefixes:
        for directory, dirs, files in os.walk(root / prefix, followlinks=False):
            if '.retain' in files:
                raise ValueError('Pinned selected directory: ' + directory)
    if not rows:
        raise ValueError('No physical tracked sources selected')
    return dict(schema=1, kind='tracked_worktree_sources', source=str(root), head=head,
                prefixes=list(prefixes), members=rows,
                scope='Exact tracked bytes and file metadata; working data, originals, untracked files and Git registration stay local')


def restore(pair, manifest, output, replica=1):
    data = json.loads(pair.read(manifest['sha256'], manifest['size'], replica))
    if data['kind'] != 'tracked_worktree_sources':
        raise ValueError('Wrong tracked-source manifest')
    output = Path(output).absolute()
    if output.exists() or any(p.is_symlink() for p in (output, *output.parents)) or 'original' in output.parts:
        raise ValueError('Restore requires a new safe directory')
    if shutil.disk_usage(output.parent).free < sum(r['size'] for r in data['members']) + 2**20:
        raise ValueError('Insufficient real restore capacity')
    output.mkdir()
    seen = set()
    for row in data['members']:
        name = Path(row['path'])
        if name.is_absolute() or '..' in name.parts or name.parts[0] not in ('research', 'tests') or 'original' in name.parts or name in seen:
            raise ValueError('Unsafe tracked-source member')
        seen.add(name)
        p = output / name
        p.parent.mkdir(parents=True, exist_ok=True)
        with p.open('xb') as stream:
            pair.stores[replica].copy(row['sha256'], row['size'], stream)
            stream.flush()
            os.fsync(stream.fileno())
        h = hashlib.sha1(b'blob ' + str(row['size']).encode() + b'\0')
        with p.open('rb') as stream:
            for block in iter(lambda: stream.read(2**20), b''):
                h.update(block)
        if h.hexdigest() != row['git_blob']:
            raise ValueError('Restored Git blob differs')
        if (p.stat().st_uid, p.stat().st_gid) != (row['uid'], row['gid']):
            os.chown(p, row['uid'], row['gid'])
        os.chmod(p, row['mode'])
        for key, value in row['xattrs'].items():
            os.setxattr(p, key, bytes.fromhex(value))
        os.utime(p, ns=(row['mtime_ns'], row['mtime_ns']))
        if stat.S_IMODE(p.stat().st_mode) != row['mode'] or p.stat().st_mtime_ns != row['mtime_ns'] or {k: os.getxattr(p, k).hex() for k in os.listxattr(p)} != row['xattrs']:
            raise ValueError('Restored metadata differs')
    return output


def compact(root, main, pair, *, recent_hours=24, scratch=None, prefixes=('research', 'tests')):
    data = inspect(root, main, prefixes=prefixes, recent_hours=recent_hours)
    root = Path(root)
    for start in range(0, len(data['members']), 32):
        with pair.batch():
            for row in data['members'][start:start + 32]:
                path = root / row['path']
                if identity(path) != row['identity']:
                    raise ValueError('Source changed before admission')
                h = hashlib.sha1(b'blob ' + str(row['size']).encode() + b'\0')
                with path.open('rb') as stream:
                    for block in iter(lambda: stream.read(2**20), b''):
                        h.update(block)
                if h.hexdigest() != row['git_blob']:
                    raise ValueError('Physical source differs from Git blob: ' + row['path'])
                with path.open('rb') as stream:
                    row['sha256'] = pair.put(stream, row['size'])
                if identity(path) != row['identity']:
                    raise ValueError('Source changed during admission')
    manifest = pair.root('tracked_worktree_sources', root, data)
    with tempfile.TemporaryDirectory(prefix='mm-tracked-restore-', dir=scratch) as tmp:
        restore(pair, manifest, Path(tmp) / 'restored')
    source_idle(root)
    current = inspect(root, main, prefixes=prefixes, recent_hours=recent_hours)
    if current['head'] != data['head'] or [dict(r, sha256=data['members'][i]['sha256']) for i, r in enumerate(current['members'])] != data['members']:
        raise ValueError('Tracked tree changed before sparse checkout')
    result = dict(source=str(root), head=data['head'], manifest=manifest,
                  real_restore_verified=True, replicas_verified=2,
                  selected_allocated_bytes=sum(r['allocated'] for r in data['members']))
    pair.root('tracked_worktree_compaction_prepared', root, result)
    marker = root / 'WORKTREE-ARCHIVED.txt'
    if marker.exists() or marker.is_symlink():
        raise ValueError('Existing recovery marker requires review')
    marker.write_text('Tracked directories ' + ', '.join(data['prefixes']) + ' retained in both shared evidence stores.\n'
                      + json.dumps(manifest, sort_keys=True) + '\n'
                      + 'Restore: tools/compact-evidence-storage.py restore-worktree --manifest '
                      + manifest['sha256'] + ' --size ' + str(manifest['size']) + ' --output <new-directory>\n'
                      + 'Git reconstruction: git sparse-checkout disable (original file timestamps require the archived restore).\n')
    _sync(marker)
    git(root, 'sparse-checkout', 'set', '--no-cone', '/*', *('!/' + p + '/' for p in data['prefixes']))
    if git(root, 'status', '--porcelain', '--untracked-files=no'):
        raise ValueError('Sparse checkout changed tracked content')
    remaining = sum((root / r['path']).lstat().st_blocks * 512 for r in data['members'] if (root / r['path']).exists())
    result.update(success=True, reclaimed_allocated_bytes=result['selected_allocated_bytes'] - remaining)
    pair.root('tracked_worktree_compaction_completed', root, result)
    return result
