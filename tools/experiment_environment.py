"""Bounded, exclusively leased experiment trees with retained SQLite state deltas.

A family owns one immutable baseline and two working trees. A loan is a real
private directory at the caller's usual path, never a link to another session.
Before returning it to the pool, preserve every difference from the baseline.
Interrupted/active loans are quarantined; there is no unbounded clone fallback.
"""
from contextlib import contextmanager
import atexit
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import sqlite3
import stat
import subprocess
import sys
import time
import uuid
import zlib

GIB = 1024 ** 3
MAX_FILE = 256 * 1024 ** 2
_LEASES = []
_CODE_HASH = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()


def enabled():
    value = os.environ.get('MNM_REUSE_ENVIRONMENTS', '1')
    if value not in ('0', '1'):
        raise ValueError('MNM_REUSE_ENVIRONMENTS must be 0 or 1')
    return value == '1'


def _raw(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':')).encode()


def _sync(path):
    fd = os.open(path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


@contextmanager
def _db(path):
    path=Path(path)
    if path.is_symlink():
        raise ValueError('Environment database is a symlink')
    result = sqlite3.connect(path, timeout=10)
    result.execute('pragma synchronous=FULL')
    result.execute('pragma journal_mode=DELETE')
    try:
        with result:
            yield result
    finally:
        result.close()


def _stamp(s):
    return s.st_dev, s.st_ino, s.st_size, s.st_mtime_ns, s.st_ctime_ns


def _scan(tree, *, payloads=False, private=False):
    """Never follow links; reject source snapshots and unsupported special files."""
    if tree.is_symlink() or not tree.is_dir():
        raise ValueError('Environment must be a real directory')
    root_stat=tree.lstat()
    root_attrs={key:os.getxattr(tree,key,follow_symlinks=False).hex() for key in sorted(os.listxattr(tree,follow_symlinks=False))}
    rows, bodies, identities = {'.':['directory',stat.S_IMODE(root_stat.st_mode),root_stat.st_mtime_ns,None,root_attrs,root_stat.st_uid,root_stat.st_gid]}, {}, {'.':_stamp(root_stat)}
    for directory, dirs, files in os.walk(tree, followlinks=False):
        here = Path(directory)
        if 'original' in dirs or '.git' in dirs or '.git' in files or 'AGENTS.md' in files:
            raise ValueError('An environment cannot contain original/ or source checkouts')
        for name in sorted(dirs + files):
            path = here / name
            s = path.lstat()
            rel = path.relative_to(tree).as_posix()
            mode = stat.S_IMODE(s.st_mode)
            if stat.S_ISLNK(s.st_mode):
                row = ['link', mode, s.st_mtime_ns, os.readlink(path)]
            elif stat.S_ISDIR(s.st_mode):
                row = ['directory', mode, s.st_mtime_ns, None]
            elif stat.S_ISREG(s.st_mode):
                if s.st_size > MAX_FILE:
                    raise ValueError('Oversized environment file; retain the environment: ' + rel)
                if private and s.st_nlink != 1:
                    raise ValueError('Working environment contains a hardlinked file: ' + rel)
                with path.open('rb') as stream:
                    if _stamp(os.fstat(stream.fileno())) != _stamp(s):
                        raise ValueError('Environment changed before reading: ' + rel)
                    body = stream.read()
                    if _stamp(os.fstat(stream.fileno())) != _stamp(s):
                        raise ValueError('Environment changed while reading: ' + rel)
                row = ['file', mode, s.st_mtime_ns, hashlib.sha256(body).hexdigest()]
                if payloads:
                    bodies[rel] = body
            else:
                raise ValueError('Special file in environment; retain it: ' + rel)
            row.extend([{key:os.getxattr(path,key,follow_symlinks=False).hex() for key in sorted(os.listxattr(path,follow_symlinks=False))},s.st_uid,s.st_gid])
            rows[rel] = row
            identities[rel] = _stamp(s)
    # Recheck membership and identities, including directories, after reading.
    after = {'.':_stamp(tree.lstat())}
    for directory, dirs, files in os.walk(tree, followlinks=False):
        for name in dirs + files:
            path = Path(directory) / name
            after[path.relative_to(tree).as_posix()] = _stamp(path.lstat())
    if identities != after:
        raise ValueError('Environment changed during its snapshot')
    return rows, bodies


def _pin(token):
    return ('Owned reusable environment loan ' + token + '\nRetain this tree until its lease is returned or recovered.\n').encode()


def _digest(rows):
    return hashlib.sha256(_raw(rows)).hexdigest()


def _busy(tree):
    name = str(tree.resolve())
    prefix = name + '/'
    for proc in Path('/proc').iterdir():
        if not proc.name.isdecimal():
            continue
        try:
            paths = [proc / 'cwd', proc / 'exe', *(proc / 'fd').iterdir()]
        except OSError:
            continue
        for item in paths:
            try:
                value = os.readlink(item).removesuffix(' (deleted)')
            except OSError:
                continue
            if value == name or value.startswith(prefix):
                return True
        if int(proc.name) != os.getpid():
            try:
                values = (proc / 'environ').read_bytes().split(b'\0')
                if b'WINEPREFIX=' + os.fsencode(name) in values:
                    return True
                for line in (proc / 'maps').read_text(errors='replace').splitlines():
                    value = line.split(None, 5)[-1].removesuffix(' (deleted)')
                    if value == name or value.startswith(prefix):
                        return True
            except OSError:
                pass
    return False


def _idle(tree, kind):
    if kind == 'wineprefix':
        subprocess.run(['wineserver', '-w'], env=dict(os.environ, WINEPREFIX=str(tree), WINEDEBUG='-all'),
                       check=True, timeout=5, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    if _busy(tree):
        raise ValueError('Environment has a host process or Wine client: ' + str(tree))


def _admission_roots(root):
    allowed=[root/'working']
    # Only explicitly provisioned output aliases are admitted, never arbitrary
    # links encountered while walking an environment or its snapshot.
    for category in ('tests','experiments','build'):
        group=root/'working'/category
        if group.is_symlink():allowed.append(group.resolve())
        if group.is_dir():
            allowed.extend(p.resolve() for p in group.iterdir() if p.is_symlink() and p.is_dir())
    return allowed


def _safe(path, root):
    if not path.is_absolute() or any(p in ('.','..','original') for p in path.parts):
        raise ValueError('Environment destination must be an owned working path')
    admitted=[base for base in _admission_roots(root) if path.is_relative_to(base)]
    if not admitted:
        raise ValueError('Environment destination must be an owned working path or provisioned output alias')
    base=max(admitted,key=lambda p:len(p.parts))
    for parent in (path,*path.parents):
        if parent.is_symlink():
            if parent.resolve() in _admission_roots(root):break
            raise ValueError('Environment path traverses a symlink: '+str(parent))
        if parent==base:break
    return base


def _owner(root):
    result = subprocess.run(['git', '-C', str(root), 'rev-parse', '--git-common-dir'],
                            capture_output=True, text=True)
    return ((root / result.stdout.strip()).resolve().parent if result.returncode == 0 else root)


def _mount_root(path):
    # Device IDs alone cannot distinguish bind mounts. Rename requires both
    # endpoints to use the same mount, even for the same underlying device.
    path=Path(path).absolute();matches=[]
    for line in Path('/proc/self/mountinfo').read_text(errors='surrogateescape').splitlines():
        fields=line.split()
        name=fields[4]
        for encoded,decoded in ((r'\040',' '),(r'\011','\t'),(r'\012','\n'),(r'\134','\\')):
            name=name.replace(encoded,decoded)
        mount=Path(name)
        if path.is_relative_to(mount):matches.append(mount)
    if not matches:raise ValueError('Cannot establish the output mount boundary')
    return max(matches,key=lambda value:len(value.parts))


def _pool(root, target):
    owner=_owner(root)
    base = owner / 'working/environment-pool'
    # Keep the pool and loan on the same mount, including bind mounts that
    # share a device ID with their parent filesystem.
    target_mount=_mount_root(target.parent)
    if _mount_root(base.parent)!=target_mount:
        admitted=_safe(target,root)
        boundary=target_mount if target_mount.is_relative_to(admitted) else admitted
        base=boundary/'.environment-pool'
    for parent in (base,*base.parents):
        if parent.is_symlink():raise ValueError('Environment pool traverses a symlink')
        if parent == owner:break
    base.mkdir(parents=True, exist_ok=True)
    if (base / '.retain').is_symlink():raise ValueError('Environment retention marker is a symlink')
    if not (base / '.retain').exists():
        (base / '.retain').write_text('Reusable environment baselines and state receipts; retain.\n')
    return base


@contextmanager
def _lock(path):
    with os.fdopen(os.open(path,os.O_RDWR|os.O_CREAT|os.O_NOFOLLOW,0o600),'a+b') as handle:
        fcntl.flock(handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
        yield handle


def _copy(source, target, root):
    command = ['cp', '-a', '--no-preserve=links', '--reflink=auto', str(source), str(target)]
    owner=_owner(root)
    guard = owner / 'tools/working-retention.py'
    # The main-drive reserve applies only to copies onto that drive. A RAM or
    # other provisioned output filesystem uses the caller's own capacity guard.
    if guard.is_file() and owner.stat().st_dev==target.parent.stat().st_dev:
        size = int(subprocess.check_output(['du', '-s', '-B1', '--', str(source)]).split()[0])
        command = [sys.executable, str(guard), 'run', '--reserve-gib', str(size / GIB + .02), '--', *command]
    subprocess.run(command, check=True)


def _failure(family, slot, token, error):
    with _db(family / 'pool.sqlite3') as db:
        db.execute('create table if not exists failures(time_ns integer,slot integer,token text,error text)')
        db.execute('insert into failures values(?,?,?,?)', (time.time_ns(),slot,token,repr(error)))
        db.execute("update slots set phase='held',error=? where slot=? and token=?", (repr(error),slot,token))


def _baseline(family):
    with _db(family / 'pool.sqlite3') as db:
        row = db.execute('select payload,sha256 from baseline').fetchone()
        if row is None:
            raise ValueError('Incomplete pool baseline is retained; review it before reuse')
        b, expected = row
    if hashlib.sha256(b).hexdigest() != expected:
        raise ValueError('Baseline manifest changed')
    return json.loads(b), expected


def _path(tree, name):
    if name=='.':return tree
    relative = Path(name)
    if relative.is_absolute() or not relative.parts or any(p in ('.', '..', 'original', '.git') for p in relative.parts):
        raise ValueError('Unsafe snapshot path: ' + name)
    result = tree / relative
    for parent in result.parents:
        if parent == tree:
            break
        if parent.is_symlink():
            raise ValueError('Snapshot parent is a link: ' + name)
    return result


def _metadata(path,row):
    current=path.lstat()
    if (current.st_uid,current.st_gid)!=(row[5],row[6]):
        os.chown(path,row[5],row[6],follow_symlinks=False)
    if row[0]!='link':path.chmod(row[1])
    attrs=row[4]
    for key in os.listxattr(path,follow_symlinks=False):
        if key not in attrs:os.removexattr(path,key,follow_symlinks=False)
    for key,value in attrs.items():os.setxattr(path,key,bytes.fromhex(value),follow_symlinks=False)
    os.utime(path,ns=(row[2],row[2]),follow_symlinks=False)


def _reset(tree, baseline, expected, current):
    # current is the verified final snapshot retained by the previous borrower.
    for name in sorted(current, key=lambda n: (len(Path(n).parts), n), reverse=True):
        if name not in expected or current[name][0] != expected[name][0]:
            path = _path(tree, name)
            if current[name][0] == 'directory':
                path.rmdir()
            else:
                path.unlink()
    for name in sorted(expected, key=lambda n: (len(Path(n).parts), n)):
        row = expected[name]
        path = _path(tree, name)
        if row[0] == 'directory':
            path.mkdir(exist_ok=True)
        elif current.get(name) != row:
            if path.is_symlink():
                path.unlink()
            if row[0] == 'link':
                if path.exists():
                    path.unlink()
                path.symlink_to(row[3])
            else:
                source = _path(baseline, name)
                body = source.read_bytes()
                if hashlib.sha256(body).hexdigest() != row[3]:
                    raise ValueError('Baseline bytes changed: ' + name)
                path.write_bytes(body)
            _metadata(path,row)
    for name in sorted(expected, key=lambda n: len(Path(n).parts), reverse=True):
        row = expected[name]
        if row[0] == 'directory':
            path = _path(tree, name)
            _metadata(path,row)
    actual, _ = _scan(tree, private=True)
    if actual != expected:
        raise ValueError('Clean environment reset differs from the baseline')


class Lease:
    def __init__(self, root, target, family, slot, handle, token, kind, before):
        self.root, self.target, self.family, self.slot = root, target, family, slot
        self.handle, self.token, self.kind, self.before = handle, token, kind, before
        self.pid = os.getpid()
        self.closed = False

    @property
    def receipt(self):
        return getattr(self, '_receipt', self.target.parent / ('environment-' + self.target.name + '.sqlite3'))

    def close(self):
        if self.closed:
            return
        try:
            _safe(self.target, self.root)
            if hashlib.sha256(Path(__file__).read_bytes()).hexdigest()!=_CODE_HASH:
                raise ValueError('Environment helper changed during the run; retain and recover with reviewed code')
            _idle(self.target, self.kind)
            final, bodies = _scan(self.target, payloads=True, private=True)
            if (self.target/'.retain').read_bytes()!=_pin(self.token) or any(Path(name).name == '.retain' and name!='.retain' and name not in self.before for name in final):
                raise ValueError('Newly pinned environment stays in its run')
            changes = {name: final.get(name) for name in self.before.keys() | final.keys()
                       if self.before.get(name) != final.get(name)}
            delta = _raw(changes)
            with _db(self.receipt) as db:
                db.execute('create table delta(path text primary key, payload blob,sha256 text)')
                for name, row in changes.items():
                    if row and row[0] == 'file':
                        body = bodies[name]
                        db.execute('insert into delta values(?,?,?)', (name, zlib.compress(body), row[3]))
                db.execute('insert into records values(?,?,?)', ('final', _digest(final), _raw(final)))
                db.execute('insert into records values(?,?,?)', ('changes', hashlib.sha256(delta).hexdigest(), delta))
            # Read every retained delta back before returning the only working copy.
            with _db(self.receipt) as db:
                for name, packed, digest in db.execute('select * from delta'):
                    if hashlib.sha256(zlib.decompress(packed)).hexdigest() != digest:
                        raise ValueError('Retained environment delta verification failed: ' + name)
            _idle(self.target, self.kind)
            if _scan(self.target, private=True)[0] != final:
                raise ValueError('Environment changed after retaining its final state')
            with _db(self.family / 'pool.sqlite3') as db:
                db.execute("update slots set phase='returning' where slot=? and token=?", (self.slot, self.token))
            workspace = self.family / ('slot-' + str(self.slot)) / 'tree'
            os.rename(self.target, workspace)
            _sync(self.target.parent); _sync(workspace.parent)
            with _db(self.family / 'pool.sqlite3') as db:
                db.execute("update slots set phase='idle',location=?,final_sha256=? where slot=? and token=?",
                           (str(workspace), _digest(final), self.slot, self.token))
            with _db(self.receipt) as db:
                db.execute('insert into records values(?,?,?)', ('returned', self.token, b'pool-owned; restore before historical consumption'))
        except BaseException as error:
            _failure(self.family, self.slot, self.token, error)
            raise
        finally:
            self.closed = True
            self.handle.close()


def acquire(root, target, template, *, kind='wineprefix'):
    root, template = Path(root).resolve(), Path(template).resolve()
    supplied=Path(target).absolute()
    if any(part=='..' for part in supplied.parts):raise ValueError('Parent traversal in environment destination')
    # Validate the supplied path too: canonicalization must not hide an
    # arbitrary symlink under working/. Only declared output aliases qualify.
    _safe(supplied, root)
    target=supplied.parent.resolve()/supplied.name
    if kind not in ('wineprefix', 'game'):
        raise ValueError('Unsupported reusable environment kind')
    _safe(target, root)
    if target.exists() or target.is_symlink() or not template.is_dir() or 'original' in template.parts:
        raise ValueError('Require a fresh target and a disposable environment template')
    receipt=target.parent/('environment-'+target.name+'.sqlite3')
    if receipt.exists() or receipt.is_symlink():
        raise ValueError('Environment receipt already exists; preserve prior evidence')
    _idle(template, kind)
    before, _ = _scan(template)
    if any(Path(name).name == '.retain' for name in before):
        raise ValueError('Pinned template contents cannot become a reusable environment')
    signature = _digest(before)
    family = _pool(root, target) / hashlib.sha256(_raw([2,kind, str(template)])).hexdigest()[:24]
    family.mkdir(exist_ok=True)
    if family.is_symlink():raise ValueError('Environment family is a symlink')
    with _lock(family / 'setup.lock'):
        database = family / 'pool.sqlite3'
        fresh = not database.exists()
        with _db(database) as db:
            if fresh:
                db.executescript('create table baseline(payload blob,sha256 text); create table slots(slot integer primary key,phase text,token text,location text,final_sha256 text,error text);')
        pristine = family / 'pristine'
        if fresh:
            _copy(template, pristine, root)
            copied, _ = _scan(pristine)
            if copied != before or _scan(template)[0] != before:
                raise ValueError('Template changed during initial baseline preparation')
            with _db(database) as db:
                db.execute('insert into baseline values(?,?)', (_raw(before), signature))
        expected, expected_sha = _baseline(family)
        if signature != expected_sha or _scan(pristine)[0] != expected:
            raise ValueError('Template or immutable baseline changed; review a new pool family explicitly')
        _idle(template, kind)
        for slot in range(2):
            parent = family / ('slot-' + str(slot)); parent.mkdir(exist_ok=True)
            if parent.is_symlink():raise ValueError('Environment slot is a symlink')
            handle = os.fdopen(os.open(parent/'lease.lock',os.O_RDWR|os.O_CREAT|os.O_NOFOLLOW,0o600),'a+b')
            try:
                fcntl.flock(handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except BlockingIOError:
                handle.close(); continue
            with _db(database) as db:
                state = db.execute('select phase,final_sha256 from slots where slot=?', (slot,)).fetchone()
            if state and state[0] != 'idle':
                handle.close(); continue
            token = uuid.uuid4().hex
            with _db(database) as db:
                db.execute('insert or replace into slots values(?,?,?,?,?,?)', (slot, 'preparing', token, str(target), None, None))
            workspace = parent / 'tree'
            try:
                if state:
                    _idle(workspace, kind)
                    current, _ = _scan(workspace, private=True)
                    if _digest(current) != state[1]:
                        raise ValueError('Unrecorded changes in a pooled environment; held for review')
                    _reset(workspace, pristine, before, current)
                else:
                    _copy(pristine, workspace, root)
                    if _scan(workspace, private=True)[0] != before:
                        raise ValueError('Initial working environment differs from baseline')
                receipt = target.parent / ('environment-' + target.name + '.sqlite3')
                if receipt.exists():
                    raise ValueError('Environment receipt already exists; preserve prior evidence')
                with _db(receipt) as db:
                    db.execute('create table records(name text primary key,sha256 text,payload blob)')
                    meta = _raw(dict(schema=2,kind=kind,root=str(root),target=str(target),family=str(family),baseline_sha256=signature,token=token,slot=slot,template=str(template),helper_sha256=_CODE_HASH))
                    db.execute('insert into records values(?,?,?)', ('lease', hashlib.sha256(meta).hexdigest(), meta))
                with (workspace/'.retain').open('xb') as marker:
                    marker.write(_pin(token));marker.flush();os.fsync(marker.fileno())
                os.rename(workspace, target); _sync(parent); _sync(target.parent)
                with _db(database) as db:
                    db.execute("update slots set phase='leased' where slot=? and token=?", (slot, token))
            except BaseException as error:
                _failure(family, slot, token, error)
                handle.close(); raise
            lease = Lease(root, target, family, slot, handle, token, kind, before)
            _LEASES.append(lease)
            return lease
    raise ValueError('Both environment slots are busy or held; no additional clone was created')


def stage_prefix(root, template, target, *, log=None, persistent=False):
    """In-process replacement for a private cp; auto-return after all run checks."""
    lease = stage(Path(root), Path(target), Path(template),persistent=persistent)
    if log is not None:
        with Path(log).open('x') as output:
            output.write('Reusable environment state: ' + str(lease.receipt) + '\n' if lease else 'Explicit private environment copy\n')
    return lease


def _temporary(root,target):
    root=Path(root).resolve();target=Path(target).absolute()
    canonical=target.parent.resolve()/target.name
    categories=[root/'working'/name for name in ('tests','experiments','build')]
    allowed=categories+[path.resolve() for path in categories]+_admission_roots(root)[1:]
    return any(path.is_relative_to(base) for path in (target,canonical) for base in allowed)


def stage(root, target, template, *, kind='wineprefix', persistent=False):
    reuse=enabled()
    allowed=_admission_roots(Path(root).resolve())
    supplied=Path(target).absolute()
    canonical=supplied.parent.resolve()/supplied.name
    pinned=any((parent/'.retain').exists() for path in (supplied,canonical) for parent in path.parents if any(parent==base or parent.is_relative_to(base) for base in allowed))
    if persistent or not reuse or pinned or not _temporary(root,target):
        subprocess.run(['cp','-a','--reflink=auto',str(Path(template).resolve()),str(target)],check=True)
        return None
    return acquire(root, target, template, kind=kind)


def release(target):
    """Return a completed in-process stage after all assertions/readers finish."""
    target=Path(target).absolute()
    for lease in _LEASES:
        if lease.target==target and not lease.closed:
            lease.close()
            return


def restore(receipt):
    receipt = Path(receipt).absolute()
    if receipt.is_symlink() or not receipt.is_file():raise ValueError('Require an existing real receipt')
    with _db(receipt) as db:
        values = {name: (digest, payload) for name, digest, payload in db.execute('select * from records')}
        for name in ('lease', 'final', 'changes'):
            digest, payload = values[name]
            if hashlib.sha256(payload).hexdigest() != digest:
                raise ValueError('Environment receipt changed: ' + name)
        meta, final = json.loads(values['lease'][1]), json.loads(values['final'][1])
        delta = {name: (packed, digest) for name, packed, digest in db.execute('select * from delta')}
    root, target, family = Path(meta['root']), Path(meta['target']), Path(meta['family'])
    _safe(target, root)
    if target.exists() or target.is_symlink():
        raise ValueError('Restoration refuses an existing destination')
    before, signature = _baseline(family)
    if signature != meta['baseline_sha256'] or _scan(family/'pristine')[0] != before:
        raise ValueError('Historical baseline changed')
    for name,row in final.items():
        _path(target,name)
        if row[0]=='file' and row!=before.get(name):
            packed,digest=delta[name]
            if hashlib.sha256(zlib.decompress(packed)).hexdigest()!=digest or digest!=row[3]:
                raise ValueError('Retained delta bytes changed')
    _copy(family/'pristine', target, root)
    # Materialize changed data in a private baseline view, then apply the same
    # safe tree transformation. The real immutable baseline remains untouched.
    for name in sorted(before, key=lambda n: len(Path(n).parts), reverse=True):
        if name not in final or before[name][0] != final[name][0]:
            path = _path(target, name)
            path.rmdir() if before[name][0]=='directory' else path.unlink()
    for name in sorted(final, key=lambda n: (len(Path(n).parts), n)):
        row = final[name];path = _path(target, name)
        if row[0]=='directory':path.mkdir(exist_ok=True)
        elif row != before.get(name):
            if path.exists() or path.is_symlink():path.unlink()
            if row[0]=='link':path.symlink_to(row[3])
            else:
                packed, digest = delta[name];body = zlib.decompress(packed)
                if hashlib.sha256(body).hexdigest()!=digest or digest!=row[3]:raise ValueError('Retained delta bytes changed')
                path.write_bytes(body)
            _metadata(path,row)
    for name in sorted(final,key=lambda n:len(Path(n).parts),reverse=True):
        if final[name][0]=='directory':
            _metadata(_path(target,name),final[name])
    if _scan(target,private=True)[0]!=final:raise ValueError('Restored environment differs from retained state')
    return target


def recover(receipt):
    """Retain and return an interrupted loan, preserving its original receipt.

    Recovery only admits a still-present original run tree with the same token.
    Half-completed moves/resets and unknown foreign changes need manual review.
    """
    receipt=Path(receipt).absolute()
    if receipt.is_symlink() or not receipt.is_file():raise ValueError('Require an existing real receipt')
    with _db(receipt) as db:
        digest,payload=db.execute("select sha256,payload from records where name='lease'").fetchone()
    if hashlib.sha256(payload).hexdigest()!=digest:raise ValueError('Lease receipt changed')
    meta=json.loads(payload);root=Path(meta['root']);target=Path(meta['target']);family=Path(meta['family'])
    _safe(target,root)
    before,signature=_baseline(family)
    if signature!=meta['baseline_sha256'] or _scan(family/'pristine')[0]!=before:raise ValueError('Historical baseline changed')
    slot=meta['slot'];token=meta['token'];parent=family/('slot-'+str(slot))
    handle=os.fdopen(os.open(parent/'lease.lock',os.O_RDWR|os.O_NOFOLLOW),'a+b')
    try:
        fcntl.flock(handle,fcntl.LOCK_EX|fcntl.LOCK_NB)
        with _db(family/'pool.sqlite3') as db:
            state=db.execute('select phase,token,location from slots where slot=?',(slot,)).fetchone()
        if not state or state[0] not in ('leased','held') or state[1]!=token or state[2]!=str(target):
            raise ValueError('Recovery requires the unchanged interrupted loan')
        if not target.is_dir() or target.is_symlink() or (parent/'tree').exists():
            raise ValueError('Recovery requires the original run tree; review partial moves separately')
        _idle(target,meta['kind'])
        recovered=receipt.with_name(receipt.stem+'-recovery-'+uuid.uuid4().hex+'.sqlite3')
        with _db(recovered) as db:
            db.execute('create table records(name text primary key,sha256 text,payload blob)')
            db.execute('insert into records values(?,?,?)',('lease',digest,payload))
            db.execute('insert into records values(?,?,?)',('recovery-helper',_CODE_HASH,str(Path(__file__)).encode()))
            db.execute('insert into records values(?,?,?)',('recovery-source',hashlib.sha256(receipt.read_bytes()).hexdigest(),str(receipt).encode()))
        lease=Lease(root,target,family,slot,handle,token,meta['kind'],before)
        lease._receipt=recovered
        lease.close()
        return recovered
    finally:
        handle.close()


def _finish():
    failed = []
    for lease in reversed(_LEASES):
        if lease.pid != os.getpid() or lease.closed:
            continue
        try:
            lease.close()
        except BaseException as error:
            failed.append(str(error))
    if failed:
        print('Environment return failed; data retained in held slots: ' + '; '.join(failed), file=sys.stderr, flush=True)
        os._exit(2)


atexit.register(_finish)

if __name__ == '__main__':
    import argparse
    parser=argparse.ArgumentParser(description='Restore retained state or recover an idle interrupted loan')
    action=parser.add_mutually_exclusive_group(required=True)
    action.add_argument('--restore',type=Path)
    action.add_argument('--recover',type=Path)
    args=parser.parse_args()
    print(restore(args.restore) if args.restore else recover(args.recover))
