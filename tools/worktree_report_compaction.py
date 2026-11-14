"""Canonicalize redundant report expansions without changing their payloads.

This is storage retirement, not validation of engine behavior. Only files whose
exact bytes match their existing Git evidence pointer are eligible. Authored
accounting, unique changes, working data and original artifacts stay untouched.
"""
import hashlib
import json
import os
from pathlib import Path
import sqlite3
import shutil
import stat
import subprocess
import tempfile

from coverage_evidence import MAGIC, Store, artifact, cache_root, config, extra_pending_paths, identity, pending_path


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), '-c', 'filter.mm-evidence.clean=cat',
        '-c', 'filter.mm-evidence.smudge=cat', '-c', 'filter.mm-evidence.process=',
        '-c', 'filter.mm-evidence.required=false', *args], stderr=subprocess.PIPE,
        env=dict(os.environ, GIT_OPTIONAL_LOCKS='0'))


def digest(path):
    with Path(path).open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def stamp(path):
    s = Path(path).lstat()
    return dict(device=s.st_dev, inode=s.st_ino, size=s.st_size, mtime_ns=s.st_mtime_ns,
                ctime_ns=s.st_ctime_ns, mode=s.st_mode, uid=s.st_uid, gid=s.st_gid, links=s.st_nlink)


def check_target(root, main, own):
    root, main, own = map(lambda p: Path(p).absolute(), (root, main, own))
    if root in (main, own) or any(p.is_symlink() for p in (root, *root.parents)):
        raise ValueError('Select an inactive historical linked worktree without path aliases')
    if not (root/'.git').is_file() or (root/'.git').is_symlink():
        raise ValueError('Target must retain its linked Git registration')
    if Path(git(root, 'rev-parse', '--show-toplevel').decode().strip()) != root:
        raise ValueError('Git root does not match target')
    common = Path(git(root, 'rev-parse', '--path-format=absolute', '--git-common-dir').decode().strip())
    if common != Path(git(main, 'rev-parse', '--path-format=absolute', '--git-common-dir').decode().strip()):
        raise ValueError('Target belongs to another source repository')
    gd = Path(git(root, 'rev-parse', '--absolute-git-dir').decode().strip())
    if Path((gd/'gitdir').read_text().strip()) != root/'.git':
        raise ValueError('Registered path needs repair before maintenance')
    for name in ('locked', 'index.lock', 'MERGE_HEAD', 'CHERRY_PICK_HEAD', 'REVERT_HEAD',
                 'rebase-apply', 'rebase-merge', 'sequencer'):
        if (gd/name).exists():
            raise ValueError('Worktree lock or unfinished Git operation: '+name)
    head = git(root, 'rev-parse', 'HEAD').decode().strip()
    git(main, 'merge-base', '--is-ancestor', head, 'master')
    if git(root, 'diff', '--cached', '--name-only', 'HEAD'):
        raise ValueError('Staged changes remain owned by their original task')
    from worktree_compaction import source_idle
    source_idle(root)
    # Existing guard also checks cwd/open descriptors. Include exact --root
    # arguments as well as source filenames, excluding this maintenance process.
    raw_root = os.fsencode(root)
    for proc in Path('/proc').iterdir():
        if not proc.name.isdecimal() or int(proc.name) == os.getpid():
            continue
        try:
            args = (proc/'cmdline').read_bytes().split(b'\0')
        except OSError:
            continue
        if any(a == raw_root or a.startswith(raw_root+b'/') for a in args):
            raise ValueError('Host-visible command consumes target')
    return head


def expanded_reports(root):
    """Read exact index blobs in one Git process; never normalize old hashes."""
    entries = []
    for entry in git(root, 'ls-files', '-s', '-z', '--', 'research').split(b'\0'):
        if not entry:
            continue
        header, name = entry.split(b'\t', 1)
        mode, oid, stage = header.decode().split()
        name = os.fsdecode(name)
        if stage != '0' or mode not in ('100644', '100755') or not artifact(name):
            continue
        path = root/name
        if not path.is_file() or path.is_symlink():
            continue
        before = stamp(path)
        if before['links'] != 1 or before['uid'] != os.getuid():
            continue
        with path.open('rb') as source:
            if source.read(len(MAGIC)) == MAGIC:
                continue
        if before['size'] <= 4096:
            continue
        entries.append((name, oid, before))
    process = subprocess.Popen(['git', '-C', str(root), 'cat-file', '--batch'],
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    try:
        for name, oid, before in entries:
            process.stdin.write(oid.encode()+b'\n');process.stdin.flush()
            header = process.stdout.readline().split()
            if len(header) != 3 or header[1] != b'blob':
                raise ValueError('Index object unavailable')
            size = int(header[2])
            if size > 160:
                remaining=size
                while remaining:
                    chunk=process.stdout.read(min(remaining,1024**2))
                    if not chunk:raise ValueError('Truncated Git object')
                    remaining-=len(chunk)
                if process.stdout.read(1)!=b'\n':raise ValueError('Truncated Git object')
                continue
            body = process.stdout.read(size)
            if len(body) != size or process.stdout.read(1) != b'\n':
                raise ValueError('Truncated Git object')
            ref = identity(body)
            if ref is None:
                continue  # Old raw Git blobs need their separate preservation path.
            sha, payload_size = ref
            path = root/name
            if payload_size != before['size'] or digest(path) != sha:
                continue  # A unique edit is never replaced by the committed pointer.
            if stamp(path) != before:
                raise ValueError('Source changed while acquiring plan: '+name)
            xattrs = {key:os.getxattr(path,key).hex() for key in sorted(os.listxattr(path))}
            yield dict(path=name, git_blob=oid, sha256=sha, size=payload_size,
                       pointer=body.decode(), before=before, xattrs=xattrs,
                       allocated_bytes=path.stat().st_blocks*512)
    finally:
        process.stdin.close();process.stdout.close();process.wait()


class Copies:
    """Existing actual copies, deduplicated by inode and independent filesystem."""
    def __init__(self, own, main):
        self.paths = []
        for root in (Path(own), Path(main)):
            self.paths.extend([pending_path(root), root/'working/coverage/evidence-pending.sqlite3'])
            self.paths.extend(extra_pending_paths(root))
            self.paths.extend(cache_root(root)/(entry['sha256']+'.sqlite3')
                              for entry in (config(root) or {}).get('archives', []))
        self.stores = []; seen = set()
        for path in self.paths:
            if not path.is_file():
                continue
            s = path.stat(); key = (s.st_dev,s.st_ino)
            if key in seen:
                continue
            seen.add(key); self.stores.append(Store(path,readonly=True))

    def close(self):
        for store in self.stores:
            store.close()

    def verify(self, sha, size, restore_directory):
        if shutil.disk_usage(restore_directory).free < size+64*1024**2:
            raise ValueError('Bounded restore scratch lacks capacity')
        selected = []; devices = set()
        for store in self.stores:
            if not store.has(sha,size) or store.path.stat().st_dev in devices:
                continue
            # A sparse metadata row is not a retained payload. copy() hashes all
            # bytes; failure never counts as a second copy or falls back silently.
            with tempfile.TemporaryFile(dir=restore_directory) as restored:
                store.copy(sha,size,restored)
                restored.flush();os.fsync(restored.fileno());restored.seek(0)
                if hashlib.file_digest(restored,'sha256').hexdigest() != sha:
                    raise ValueError('Restored payload mismatch')
            devices.add(store.path.stat().st_dev);selected.append(str(store.path))
            if len(selected) == 2:
                return selected
        raise ValueError('Two independent verified payload copies are unavailable: '+sha)


def open_plan(path):
    db = sqlite3.connect(path)
    db.execute('CREATE TABLE IF NOT EXISTS metadata(key TEXT PRIMARY KEY,value TEXT NOT NULL)')
    db.execute('CREATE TABLE IF NOT EXISTS files(path TEXT PRIMARY KEY,record TEXT NOT NULL,state TEXT NOT NULL,reclaimed INTEGER NOT NULL DEFAULT 0)')
    db.execute('CREATE TABLE IF NOT EXISTS proofs(sha TEXT PRIMARY KEY,size INTEGER NOT NULL,providers TEXT NOT NULL)')
    db.commit()
    return db


def checkpoint(own, path, restore_directory):
    """Restore complete plan records while grouping immutable store writes.

    This remains an actual restore. Pair.batch verifies both object copies and
    commits/fsyncs both stores before the verified receipt becomes visible.
    """
    from evidence_objects import configured, logical_checkpoint, restore_logical
    retained = logical_checkpoint(own, path, verify_restore=False)
    with configured(own) as pair:
        with tempfile.TemporaryDirectory(dir=restore_directory, prefix='plan-restore-') as temporary:
            with pair.batch():
                restore_logical(pair, retained['manifest'], Path(temporary)/'plan.sqlite3')
        retained.update(logical_restore_verified=True, real_restore_performed=True,
                        retirement_authorized=True)
        retained['verified_receipt'] = pair.root('worktree_report_plan_restore_verified', path, retained)
    return retained


def prepare(root, main, own, plan, reason, restore_directory, retain_missing=False):
    if not reason.strip():
        raise ValueError('A concrete pin/consumer review reason is required')
    root = Path(root).absolute();plan = Path(plan)
    if plan.exists():
        raise ValueError('Use a new plan, preserving prior phases')
    plan.parent.mkdir(parents=True,exist_ok=True)
    head = check_target(root,main,own)
    db = open_plan(plan); copies = Copies(own,main)
    try:
        with db:
            for key,value in dict(root=str(root),head=head,reason=reason,operator_sha256=digest(__file__),kind='canonicalize_existing_report_expansions',
                                  scope='Storage only. Working data, authored accounting and unique changes are preserved. Frozen physical readers must hydrate before replay.').items():
                db.execute('INSERT INTO metadata VALUES(?,?)',(key,value))
        count=0;allocated=0
        for row in expanded_reports(root):
            if row['before']['mtime_ns'] > (__import__('time').time_ns()-24*3600*10**9):
                continue
            if not db.execute('SELECT 1 FROM proofs WHERE sha=?',(row['sha256'],)).fetchone():
                try:
                    providers=copies.verify(row['sha256'],row['size'],restore_directory)
                except ValueError as error:
                    if not retain_missing or not str(error).startswith('Two independent verified payload copies are unavailable:'):
                        raise
                    from evidence_objects import configured
                    with configured(own) as pair:
                        with (root/row['path']).open('rb') as source:
                            retained=pair.put(source,row['size'],copies=2)
                        if retained!=row['sha256']:
                            raise ValueError('Source changed while retaining missing copies')
                    copies.close();copies=Copies(own,main)
                    providers=copies.verify(row['sha256'],row['size'],restore_directory)
                db.execute('INSERT INTO proofs VALUES(?,?,?)',(row['sha256'],row['size'],json.dumps(providers)))
            if stamp(root/row['path']) != row['before']:
                raise ValueError('Source changed during retention verification')
            db.execute('INSERT INTO files VALUES(?,?,?,0)',(row['path'],json.dumps(row,sort_keys=True),'prepared'))
            count+=1;allocated+=(root/row['path']).stat().st_blocks*512
            if count%100==0:db.commit()
        db.commit()
        check_target(root,main,own)
        # The execution input is also complete recoverable review state. Its
        # independent checkpoint and actual restore precede any replacement.
        retained=checkpoint(own,plan,restore_directory)
        return dict(files=count,allocated_bytes=allocated,plan=str(plan),sha256=digest(plan),head=head,
                    retained_plan=retained)
    finally:
        copies.close();db.close()


def replace_one(root,row):
    path=Path(root)/row['path']
    if path.is_symlink() or stamp(path)!=row['before'] or digest(path)!=row['sha256']:
        raise ValueError('Stale source identity or bytes: '+row['path'])
    if any(p.is_symlink() for p in path.parents if p==root or root in p.parents):
        raise ValueError('Source acquired a directory alias')
    before=path.stat().st_blocks*512
    fd,name=tempfile.mkstemp(prefix='.report-reference-',dir=path.parent)
    try:
        with os.fdopen(fd,'wb') as output:
            output.write(row['pointer'].encode());output.flush();os.fsync(output.fileno())
            os.fchmod(output.fileno(),stat.S_IMODE(row['before']['mode']))
            if os.fstat(output.fileno()).st_gid!=row['before']['gid']:
                os.fchown(output.fileno(),row['before']['uid'],row['before']['gid'])
        for key,value in row['xattrs'].items():os.setxattr(name,key,bytes.fromhex(value))
        os.utime(name,ns=(row['before']['mtime_ns'],row['before']['mtime_ns']))
        if stamp(path)!=row['before']:
            raise ValueError('Concurrent source change before replacement')
        os.replace(name,path)
        directory=os.open(path.parent,os.O_RDONLY|os.O_DIRECTORY)
        try:os.fsync(directory)
        finally:os.close(directory)
        return before-path.stat().st_blocks*512
    finally:
        Path(name).unlink(missing_ok=True)


def apply(plan, expected, main, own, restore_directory):
    plan=Path(plan)
    if digest(plan)!=expected:
        raise ValueError('Plan hash changed')
    db=open_plan(plan);copies=Copies(own,main)
    try:
        meta=dict(db.execute('SELECT key,value FROM metadata'))
        if digest(__file__)!=meta['operator_sha256']:raise ValueError('Operator changed after preparation')
        root=Path(meta['root']);head=check_target(root,main,own)
        if head!=meta['head']:raise ValueError('Target HEAD changed')
        # This fresh invocation verifies every surviving report copy before any
        # replacement. A prior prepare result is not a cached storage pass.
        for sha,size,_ in db.execute('SELECT sha,size,providers FROM proofs'):
            copies.verify(sha,size,restore_directory)
        check_target(root,main,own)
        from evidence_objects import configured
        marker=root/'WORKTREE-REPORTS-CANONICALIZED.txt'
        notice=('Selected generated report payloads retain their original SHA-256 identities and two verified independent SQLite copies.\n'
                'Selected reports use their existing Git references; source, working data and authored accounting are unchanged.\n'
                'Readers requiring physical JSON, binary or source bytes must first hydrate selected inputs with current main tools/evidence-store.py --root '+str(root)+' hydrate <report-path>...\n'
                'Exact prior file metadata and execution outcomes are in the retained plan: '+str(plan)+'\n')
        with configured(own) as pair:
            pair.root('worktree_report_compaction_admitted',root,dict(head=head,reason=meta['reason'],plan_sha256=expected,
                operator_sha256=meta['operator_sha256'],notice=notice,report_payload_copies=2,
                scope='No engine validation claim; preserve source/working data and restore physical report materializations before frozen-reader replay.'))
        if marker.is_symlink() or (marker.exists() and
                (not marker.is_file() or marker.stat().st_nlink!=1 or marker.stat().st_uid!=os.getuid())):
            raise ValueError('Shared or aliased maintenance marker needs review')
        if marker.exists() and marker.read_text()!=notice:raise ValueError('Existing maintenance marker needs review')
        marker.write_text(notice)
        total=0;count=0
        try:
            for path,raw in db.execute('SELECT path,record FROM files WHERE state=? ORDER BY path',('prepared',)).fetchall():
                row=json.loads(raw);saved=replace_one(root,row)
                db.execute('UPDATE files SET state=?,reclaimed=? WHERE path=?',('canonicalized',saved,path))
                total+=saved;count+=1
                if count%64==0:db.commit()
        finally:
            # Flush the completed prefix on ordinary failures/interruption. A
            # power loss can leave at most one group needing reconciliation:
            # exact before metadata, allocation and both payload copies already
            # survive in the admitted immutable plan, and pointers remain exact.
            db.commit()
        retained=checkpoint(own,plan,restore_directory)
        result=dict(root=str(root),head=head,files=count,reclaimed_allocated_bytes=total,
                    unchanged_payload_hashes=True,engine_validation_performed=False)
        with configured(own) as pair:
            result['retained_outcome']=pair.root('worktree_report_compaction_completed',root,dict(result,checkpoint=retained))
        return result
    finally:
        copies.close();db.close()
