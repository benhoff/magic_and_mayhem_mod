"""Content-addressed SQLite evidence, portable Git pointers, and verified restore.

Pointers change storage only. Original report bytes, fingerprints, receipt values,
and validation claims remain unchanged. Archives are pinned by SHA-256 in Git.
"""
from contextlib import contextmanager
from contextvars import ContextVar
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import sqlite3
import subprocess
import tempfile
import urllib.request
import zlib

MAGIC = b'magic-and-mayhem-evidence-v1 '
CONFIG = '.evidence-store.json'
COMMIT_MAP = '.evidence-commits.json'
CHUNK = 1024 * 1024
_active_reader = ContextVar('sqlite_evidence_reader', default=None)
KEEP = {'research/runtime/coverage/register.json',
        'research/runtime/coverage/change-reviews.json',
        'research/runtime/coverage/gate-baseline.json',
        'research/formats/mmsprite-source-lock.json'}


def artifact(path):
    """Versioned storage policy: retain research prose, source, and live registers."""
    p = Path(path)
    return (p.as_posix().startswith(('research/runtime/', 'research/formats/')) and p.as_posix() not in KEEP
            and not p.name.endswith(('register.json', '-coverage.json'))
            and (p.suffix in {'.json', '.bin', '.png', '.log', '.gz'}
                 or (p.suffix == '.md' and p.name.startswith('summary')
                     and p.parent.as_posix() == 'research/runtime/coverage')
                 or p.as_posix() == 'research/runtime/coverage-summary.md'))


def pointer(sha, size):
    if not re.fullmatch('[0-9a-f]{64}', sha) or not isinstance(size, int) or size < 0:
        raise ValueError('Invalid evidence identity')
    return MAGIC + sha.encode() + b' ' + str(size).encode() + b'\n'


def identity(data):
    if not data.startswith(MAGIC):
        return None
    match = re.fullmatch(re.escape(MAGIC) + rb'([0-9a-f]{64}) (0|[1-9][0-9]*)\n', data)
    if not match:
        raise ValueError('Malformed evidence pointer')
    return match[1].decode(), int(match[2])


def digest_file(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for chunk in iter(lambda: f.read(CHUNK), b''):
            h.update(chunk)
    return h.hexdigest()


def safe_path(root, name):
    p = Path(name)
    root = Path(root).resolve()
    if p.is_absolute() or '..' in p.parts or not p.parts or p.parts[0] != 'research':
        raise ValueError('Unsafe evidence path: ' + name)
    target = root/p
    if target.is_symlink() or not target.resolve().is_relative_to(root):
        raise ValueError('Escaping evidence path: ' + name)
    return target


class Store:
    def __init__(self, path, readonly=False):
        self.path = Path(path)
        if readonly:
            self.db = sqlite3.connect(self.path.resolve().as_uri() + '?mode=ro', uri=True)
        else:
            self.path.parent.mkdir(parents=True, exist_ok=True)
            self.db = sqlite3.connect(self.path, timeout=60)
            self.db.executescript('''
                PRAGMA foreign_keys=ON;
                CREATE TABLE IF NOT EXISTS objects (
                    sha256 TEXT PRIMARY KEY, size INTEGER NOT NULL, git_blob TEXT);
                CREATE TABLE IF NOT EXISTS chunks (
                    sha256 TEXT NOT NULL REFERENCES objects(sha256), ordinal INTEGER NOT NULL,
                    payload BLOB NOT NULL, PRIMARY KEY(sha256,ordinal));
                CREATE TABLE IF NOT EXISTS versions (
                    git_blob TEXT NOT NULL, path TEXT NOT NULL, sha256 TEXT NOT NULL REFERENCES objects(sha256),
                    PRIMARY KEY(git_blob,path));
                CREATE INDEX IF NOT EXISTS git_blob_lookup ON objects(git_blob);
                CREATE TABLE IF NOT EXISTS metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL);
                INSERT OR IGNORE INTO metadata VALUES ('schema','1');
                CREATE TRIGGER IF NOT EXISTS objects_no_update BEFORE UPDATE ON objects
                    BEGIN SELECT RAISE(ABORT,'Immutable evidence'); END;
                CREATE TRIGGER IF NOT EXISTS objects_no_delete BEFORE DELETE ON objects
                    BEGIN SELECT RAISE(ABORT,'Immutable evidence'); END;
                CREATE TRIGGER IF NOT EXISTS chunks_no_update BEFORE UPDATE ON chunks
                    BEGIN SELECT RAISE(ABORT,'Immutable evidence'); END;
                CREATE TRIGGER IF NOT EXISTS chunks_no_delete BEFORE DELETE ON chunks
                    BEGIN SELECT RAISE(ABORT,'Immutable evidence'); END;
            ''')
        if self.db.execute("SELECT value FROM metadata WHERE key='schema'").fetchone() != ('1',):
            raise ValueError('Unsupported evidence database')

    def close(self):
        self.db.close()

    def has(self, sha, size=None):
        row = self.db.execute('SELECT size FROM objects WHERE sha256=?', (sha,)).fetchone()
        return row is not None and (size is None or row[0] == size)

    def put(self, stream, size, git_blob=None):
        h, compressed, remaining = hashlib.sha256(), [], size
        while remaining:
            data = stream.read(min(CHUNK, remaining))
            if not data:
                raise ValueError('Truncated evidence input')
            remaining -= len(data)
            h.update(data)
            compressed.append(zlib.compress(data, 6))
        sha = h.hexdigest()
        if not self.has(sha):
            self.db.execute('INSERT INTO objects VALUES (?,?,?)', (sha, size, git_blob))
            self.db.executemany('INSERT INTO chunks VALUES (?,?,?)',
                                ((sha, i, value) for i, value in enumerate(compressed)))
        elif not self.has(sha, size):
            raise ValueError('Evidence size collision')
        return sha

    def copy(self, sha, size, out):
        if not self.has(sha, size):
            raise ValueError('Missing evidence object: ' + sha)
        h, total, count = hashlib.sha256(), 0, 0
        for ordinal, payload in self.db.execute('SELECT ordinal,payload FROM chunks WHERE sha256=? ORDER BY ordinal', (sha,)):
            if ordinal != count:
                raise ValueError('Missing evidence chunk: ' + sha)
            decoder = zlib.decompressobj()
            data = decoder.decompress(payload, CHUNK + 1)
            if len(data) > CHUNK or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
                raise ValueError('Invalid evidence chunk: ' + sha)
            total += len(data)
            if total > size:
                raise ValueError('Evidence size overflow: ' + sha)
            h.update(data)
            out.write(data)
            count += 1
        if total != size or h.hexdigest() != sha:
            raise ValueError('Evidence hash mismatch: ' + sha)

    def verify(self):
        if self.db.execute('PRAGMA integrity_check').fetchone() != ('ok',):
            raise ValueError('Evidence SQLite integrity failure')
        if self.db.execute('PRAGMA foreign_key_check').fetchall():
            raise ValueError('Evidence SQLite foreign key failure')
        class Sink:
            def write(self, data):
                pass
        count = 0
        for sha, size in self.db.execute('SELECT sha256,size FROM objects'):
            self.copy(sha, size, Sink())
            count += 1
        return count


def cache_root(root):
    value = os.environ.get('MM_EVIDENCE_CACHE')
    if value is None and (Path(root)/'.git').exists():
        setting = subprocess.run(['git', '-C', str(root), 'config', '--get', 'mm.evidenceCache'], capture_output=True, text=True)
        value = setting.stdout.strip() if setting.returncode == 0 else None
    return Path(value or Path(root)/'working/coverage/evidence-cache').resolve()


@contextmanager
def shared_cache(root):
    """Use the caller's pinned cache for temporary staged and historical trees."""
    previous = os.environ.get('MM_EVIDENCE_CACHE')
    if previous is None:
        os.environ['MM_EVIDENCE_CACHE'] = str(cache_root(root))
    try:
        yield
    finally:
        if previous is None:
            os.environ.pop('MM_EVIDENCE_CACHE', None)


def config(root):
    path = Path(root)/CONFIG
    if not path.exists():
        return None
    value = json.loads(path.read_text())
    if value.get('schema') != 1 or not isinstance(value.get('archives'), list):
        raise ValueError('Invalid evidence archive manifest')
    return value


def fetch_archive(root, spec):
    """Download only pinned release assets, then verify the complete SQLite file."""
    sha = spec['sha256']
    if not re.fullmatch('[0-9a-f]{64}', sha):
        raise ValueError('Invalid archive hash')
    cache = cache_root(root)
    cache.mkdir(parents=True, exist_ok=True)
    target = cache/(sha + '.sqlite3')
    # Trust neither a pre-existing cache filename nor its contents.
    if target.exists() and digest_file(target) == sha:
        return target
    fd, name = tempfile.mkstemp(prefix=sha + '.', suffix='.partial', dir=cache)
    try:
        with os.fdopen(fd, 'wb') as dest, tempfile.TemporaryFile(dir=cache) as compressed:
            for part in spec['parts']:
                url = part['url']
                if not re.fullmatch(r'https://github\.com/benhoff/magic_and_mayhem_mod/releases/download/[^/]+/[A-Za-z0-9_.-]+', url):
                    raise ValueError('Unexpected evidence download URL')
                h, size = hashlib.sha256(), 0
                with urllib.request.urlopen(url, timeout=120) as response:
                    for data in iter(lambda: response.read(CHUNK), b''):
                        size += len(data)
                        if size > part['size']:
                            raise ValueError('Archive download exceeds pinned size')
                        h.update(data)
                        compressed.write(data)
                if size != part['size'] or h.hexdigest() != part['sha256']:
                    raise ValueError('Archive download hash mismatch')
            compressed.seek(0)
            total = 0
            with gzip.GzipFile(fileobj=compressed, mode='rb') as source:
                for data in iter(lambda: source.read(CHUNK), b''):
                    total += len(data)
                    if total > spec['size']:
                        raise ValueError('Uncompressed archive exceeds pinned size')
                    dest.write(data)
            dest.flush()
            os.fsync(dest.fileno())
        if total != spec['size'] or digest_file(name) != sha:
            raise ValueError('Evidence database hash mismatch')
        with opened_store(name, readonly=True) as store:
            if store.db.execute('PRAGMA integrity_check').fetchone() != ('ok',):
                raise ValueError('Downloaded SQLite integrity failure')
        os.replace(name, target)
    finally:
        if os.path.exists(name):
            os.unlink(name)
    return target


@contextmanager
def opened_store(path, readonly=False):
    store = Store(path, readonly)
    try:
        yield store
    finally:
        store.close()


class Resolver:
    def __init__(self, root):
        self.root, self.stores = Path(root), []
        self.spec = config(root)
        alternate = os.environ.get('MM_EVIDENCE_DB')
        if alternate:
            self.stores.append(Store(alternate, readonly=True))
        pending = self.root/'working/coverage/evidence-pending.sqlite3'
        if pending.is_file() and not any(s.path.resolve() == pending.resolve() for s in self.stores):
            self.stores.append(Store(pending, readonly=True))

    def close(self):
        for store in self.stores:
            store.close()

    def find(self, sha, size):
        for store in self.stores:
            if store.has(sha, size):
                return store
        for archive in (self.spec or {}).get('archives', []):
            expected = cache_root(self.root)/(archive['sha256'] + '.sqlite3')
            if any(s.path == expected for s in self.stores):
                continue
            path = fetch_archive(self.root, archive)
            if any(s.path == path for s in self.stores):
                continue
            store = Store(path, readonly=True)
            self.stores.append(store)
            if store.has(sha, size):
                return store
        raise ValueError('Evidence unavailable; publish/fetch its pinned archive: ' + sha)

    def copy(self, sha, size, out):
        self.find(sha, size).copy(sha, size, out)


class Reader:
    """Read original evidence bytes from SQLite without expanding working files.

    Connections belong to one invocation. A later audit always observes the
    current pointer or edited file, and every resolved payload is hash checked.
    """
    def __init__(self, root):
        self.root = Path(root).resolve()
        self.resolver = Resolver(self.root)
        self._token = None

    def __enter__(self):
        self._token = _active_reader.set(self)
        return self

    def __exit__(self, *args):
        try:
            _active_reader.reset(self._token)
            self._token = None
        finally:
            self.close()

    def close(self):
        self.resolver.close()

    def path(self, path):
        path = Path(path)
        target = path if path.is_absolute() else self.root/path
        if not target.resolve().is_relative_to(self.root):
            raise ValueError('Escaping evidence read: ' + str(path))
        return target.resolve()

    def read_bytes(self, path):
        target = self.path(path)
        data = target.read_bytes()
        if len(data) > 160 or not data.startswith(MAGIC):
            return data
        ident = identity(data)
        relative = target.relative_to(self.root).as_posix()
        if not artifact(relative):
            raise ValueError('Evidence pointer outside report storage policy: ' + relative)
        safe_path(self.root, relative)
        output = io.BytesIO()
        self.resolver.copy(*ident, output)
        return output.getvalue()

    def read_text(self, path, encoding=None, errors=None, newline=None):
        with io.TextIOWrapper(io.BytesIO(self.read_bytes(path)), encoding=encoding,
                              errors=errors, newline=newline) as text:
            return text.read()

    def json(self, path):
        return json.loads(self.read_text(path))

    def digest(self, path):
        target = self.path(path)
        with target.open('rb') as stream:
            head = stream.read(161)
        if len(head) > 160 or not head.startswith(MAGIC):
            return digest_file(target)
        ident = identity(head)
        relative = target.relative_to(self.root).as_posix()
        if not artifact(relative):
            raise ValueError('Evidence pointer outside report storage policy: ' + relative)
        safe_path(self.root, relative)
        class DigestSink:
            def write(self, chunk):
                pass
        # Store.copy checks the original SHA-256, size and chunk boundaries.
        self.resolver.copy(*ident, DigestSink())
        return ident[0]


def _reader_root(path, root=None):
    if root is not None:
        return Path(root).resolve()
    path = Path(path).absolute()
    for parent in path.parents:
        if (parent/CONFIG).is_file():
            return parent
    raise ValueError('Evidence pointer has no repository manifest: ' + str(path))


def read_bytes(path, *, root=None):
    """Compatibility read for tools accepting either ordinary files or reports."""
    path = Path(path)
    if root is not None and not path.is_absolute():
        path = Path(root)/path
    if root is not None and not path.resolve().is_relative_to(Path(root).resolve()):
        raise ValueError('Escaping evidence read: ' + str(path))
    current = _active_reader.get()
    if current is not None and path.resolve().is_relative_to(current.root) and (
            root is None or Path(root).resolve() == current.root):
        return current.read_bytes(path.absolute())
    data = path.read_bytes()
    if len(data) > 160 or not data.startswith(MAGIC):
        return data
    with Reader(_reader_root(path, root)) as reader:
        return reader.read_bytes(path.absolute())


def read_text(path, encoding=None, errors=None, newline=None, *, root=None):
    with io.TextIOWrapper(io.BytesIO(read_bytes(path, root=root)), encoding=encoding,
                          errors=errors, newline=newline) as text:
        return text.read()


def read_json(path, *, root=None):
    return json.loads(read_text(path, root=root))


def evidence_digest(path, *, root=None):
    path = Path(path)
    if root is not None and not path.is_absolute():
        path = Path(root)/path
    if root is not None and not path.resolve().is_relative_to(Path(root).resolve()):
        raise ValueError('Escaping evidence read: ' + str(path))
    current = _active_reader.get()
    if current is not None and path.resolve().is_relative_to(current.root) and (
            root is None or Path(root).resolve() == current.root):
        return current.digest(path.absolute())
    with path.open('rb') as stream:
        head = stream.read(161)
    if len(head) > 160 or not head.startswith(MAGIC):
        return digest_file(path)
    with Reader(_reader_root(path, root)) as reader:
        return reader.digest(path.absolute())


def write_report(root, path, data):
    """Create an immutable report; adopted research paths go straight to SQLite.

    Existing files are never overwritten or compacted implicitly. Non-research
    output and pre-adoption fixtures retain the ordinary JSON export interface.
    """
    root = Path(root).resolve()
    target = Path(path)
    if not target.is_absolute():
        target = root/target
    if target.is_symlink() or target.resolve().is_relative_to(root/'original'):
        raise ValueError('Unsafe report output: '+str(path))
    if target.exists():
        if read_bytes(target, root=root if target.resolve().is_relative_to(root) else None) != data:
            raise ValueError('Refusing to overwrite immutable report: '+str(path))
        return hashlib.sha256(data).hexdigest()
    from coverage_bookkeeping import atomic_replace
    managed = (target.resolve().is_relative_to(root) and (root/CONFIG).is_file()
               and artifact(target.resolve().relative_to(root).as_posix()))
    if managed:
        safe_path(root, target.resolve().relative_to(root).as_posix())
        with opened_store(root/'working/coverage/evidence-pending.sqlite3') as store:
            with store.db:
                sha = store.put(io.BytesIO(data), len(data))
        atomic_replace(target, pointer(sha, len(data)), None)
        return sha
    atomic_replace(target, data, None)
    return hashlib.sha256(data).hexdigest()


def decode_bytes(root, data):
    ident = identity(data)
    if ident is None:
        return data
    import io
    result = io.BytesIO()
    resolver = Resolver(root)
    try:
        resolver.copy(*ident, result)
    finally:
        resolver.close()
    return result.getvalue()


def refresh_pointer_stats(root, pending):
    """Invalidate pointer stat entries without staging any working-file bytes."""
    root = Path(root)
    if not pending or not (root/'.git').exists():
        return
    name = subprocess.check_output(['git', '-C', str(root), 'rev-parse', '--git-path', 'index'], text=True).strip()
    index = Path(name) if Path(name).is_absolute() else root/name
    if not index.exists():
        return
    lock = index.with_name(index.name+'.lock')
    fd = os.open(lock, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    temporary = None
    try:
        previous = index.read_bytes()
        tfd, temporary = tempfile.mkstemp(prefix='evidence-index-', dir=index.parent)
        with os.fdopen(tfd, 'wb') as out:
            out.write(previous)
        env = dict(os.environ, GIT_INDEX_FILE=temporary)
        rows = subprocess.check_output(['git', '-C', str(root), 'ls-files', '--stage', '-z'], env=env)
        expected = {p.relative_to(root).as_posix(): value for p, value, _ in pending}
        updates = []
        for row in filter(None, rows.split(b'\0')):
            metadata, path = row.split(b'\t', 1)
            mode, oid, stage = metadata.split()
            if stage != b'0' or path.decode() not in expected:
                continue
            actual = subprocess.check_output(['git', '-C', str(root), 'cat-file', 'blob', oid.decode()])
            if actual == expected[path.decode()]:
                updates.append(mode+b' '+oid+b'\t'+path+b'\0')
        if updates:
            subprocess.run(['git', '-C', str(root), 'update-index', '-z', '--index-info'],
                           input=b''.join(updates), env=env, check=True)
            if index.read_bytes() != previous:
                raise ValueError('Git index changed during evidence stat refresh')
            with os.fdopen(fd, 'wb') as out:
                fd = None
                out.write(Path(temporary).read_bytes())
                out.flush()
                os.fsync(out.fileno())
            os.replace(lock, index)
    finally:
        if fd is not None:
            os.close(fd)
        if lock.exists():
            lock.unlink()
        if temporary and Path(temporary).exists():
            Path(temporary).unlink()


def hydrate(root, refresh=True, paths=None):
    """Restore pointers only; preserve changed reports and all non-evidence files."""
    root = Path(root).resolve()
    if not (root/CONFIG).exists():
        return 0
    pending = []
    candidates = (safe_path(root, name) for name in paths) if paths is not None else (root/'research').rglob('*')
    for path in sorted(candidates):
        if not path.is_file() or path.is_symlink() or path.stat().st_size > 160:
            continue
        data = path.read_bytes()
        ident = identity(data)
        if ident:
            safe_path(root, path.relative_to(root).as_posix())
            pending.append((path, data, ident))
    resolver = Resolver(root)
    try:
        for path, previous, ident in pending:
            fd, name = tempfile.mkstemp(prefix=path.name + '.', suffix='.partial', dir=path.parent)
            try:
                with os.fdopen(fd, 'wb') as dest:
                    os.fchmod(dest.fileno(), path.stat().st_mode & 0o777)
                    resolver.copy(*ident, dest)
                    dest.flush()
                    os.fsync(dest.fileno())
                if path.is_symlink() or path.read_bytes() != previous:
                    raise ValueError('Evidence changed during restore: ' + str(path))
                os.replace(name, path)
            finally:
                if os.path.exists(name):
                    os.unlink(name)
    finally:
        resolver.close()
    if refresh:
        refresh_pointer_stats(root, pending)
    return len(pending)


def mapped_commit(root, commit):
    path = Path(root)/COMMIT_MAP
    if not path.exists():
        path = Path(__file__).resolve().parents[1]/COMMIT_MAP
    if path.exists():
        return json.loads(path.read_text()).get('commits', {}).get(commit, commit)
    return commit


def install_filter(root, script):
    root = Path(root).resolve()
    command = 'python3 ' + shlex.quote(str(Path(script).resolve())) + ' clean %f'
    subprocess.run(['git', '-C', str(root), 'config', 'filter.mm-evidence.clean', command], check=True)
    # Required filters must define the reverse direction too. Git archive and
    # checkout-index keep pointers; explicit hydration supplies verified bytes.
    subprocess.run(['git', '-C', str(root), 'config', 'filter.mm-evidence.smudge', 'cat'], check=True)
    subprocess.run(['git', '-C', str(root), 'config', 'filter.mm-evidence.required', 'true'], check=True)


def verify_push(root, updates):
    """Reject old history and pointers absent from the pinned, published archives."""
    root = Path(root).resolve()
    mapping_path = root/COMMIT_MAP
    if not mapping_path.exists():
        mapping_path = Path(__file__).resolve().parents[1]/COMMIT_MAP
    old = set(json.loads(mapping_path.read_text()).get('commits', {})) if mapping_path.exists() else set()
    resolver = Resolver(root)
    # Pending local objects cannot prove remote availability.
    for store in resolver.stores:
        store.close()
    resolver.stores = []
    seen, checked = set(), 0
    try:
        for line in updates.splitlines():
            fields = line.split()
            if len(fields) != 4:
                raise ValueError('Invalid Git pre-push input')
            _, local, _, remote = fields
            if set(local) == {'0'}:
                continue
            commits = subprocess.check_output(['git', '-C', str(root), 'rev-list', local,
                                               '--not', '--remotes=origin']).decode().splitlines()
            if old.intersection(commits):
                raise ValueError('Push would restore pre-migration Git history; migrate this branch first')
            objects = subprocess.check_output(['git', '-C', str(root), 'rev-list', '--objects', local,
                                               '--not', '--remotes=origin']).splitlines()
            for row in objects:
                parts = row.split(b' ', 1)
                if len(parts) != 2 or not artifact(parts[1].decode('utf-8')) or parts[0] in seen:
                    continue
                seen.add(parts[0])
                oid = parts[0].decode()
                size = int(subprocess.check_output(['git', '-C', str(root), 'cat-file', '-s', oid]))
                if size > 160:
                    raise ValueError('Push contains a raw evidence artifact; install the evidence filter')
                data = subprocess.check_output(['git', '-C', str(root), 'cat-file', 'blob', oid])
                ident = identity(data)
                if ident is None:
                    raise ValueError('Push contains an unconverted evidence artifact')
                if not any(s.has(*ident) for s in resolver.stores):
                    for spec in (resolver.spec or {}).get('archives', []):
                        path = fetch_archive(root, spec)
                        if not any(s.path == path for s in resolver.stores):
                            resolver.stores.append(Store(path, readonly=True))
                        if any(s.has(*ident) for s in resolver.stores):
                            break
                if not any(s.has(*ident) for s in resolver.stores):
                    raise ValueError('Publish evidence before pushing: ' + ident[0])
                checked += 1
    finally:
        resolver.close()
    return checked


def clean(root, path, stream, out):
    if not artifact(path):
        shutil.copyfileobj(stream, out, CHUNK)
        return
    with tempfile.TemporaryFile() as data:
        shutil.copyfileobj(stream, data, CHUNK)
        size = data.tell()
        data.seek(0)
        head = data.read(160)
        data.seek(0)
        if size <= 160 and identity(head):
            out.write(head)
            return
        # An unchanged hydrated report already has an exact pointer in the
        # index. Avoid copying the entire historical archive into the pending
        # database whenever Git refreshes its stat cache.
        indexed = subprocess.run(['git', '-C', str(root), 'show', ':'+path],
                                 capture_output=True)
        prior = identity(indexed.stdout) if indexed.returncode == 0 and len(indexed.stdout) <= 160 else None
        if prior and prior[1] == size:
            h = hashlib.sha256()
            for block in iter(lambda: data.read(CHUNK), b''):
                h.update(block)
            if h.hexdigest() == prior[0]:
                out.write(indexed.stdout)
                return
            data.seek(0)
        store_path = Path(root)/'working/coverage/evidence-pending.sqlite3'
        with opened_store(store_path) as store, store.db:
            sha = store.put(data, size)
        out.write(pointer(sha, size))


def pack_git(root, database):
    """Preserve every artifact blob version, including versions absent at HEAD."""
    names = {}
    versions = set()
    commits = subprocess.check_output(['git', '-C', str(root), 'rev-list', '--all']).splitlines()
    for commit in commits:
        entries = subprocess.check_output(['git', '-C', str(root), 'ls-tree', '-rz', commit, 'research/runtime/']).split(b'\0')
        for entry in filter(None, entries):
            metadata, path = entry.split(b'\t', 1)
            mode, kind, oid = metadata.split()
            name = path.decode('utf-8')
            if artifact(name):
                if kind != b'blob' or mode not in {b'100644', b'100755'}:
                    raise ValueError('Unsupported evidence file mode: ' + name)
                names[oid.decode()] = name
                versions.add((oid.decode(), name))
    process = subprocess.Popen(['git', '-C', str(root), 'cat-file', '--batch'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    try:
        with opened_store(database) as store:
            for i, (oid, name) in enumerate(sorted(names.items())):
                if store.db.execute('SELECT 1 FROM objects WHERE git_blob=?', (oid,)).fetchone():
                    continue
                process.stdin.write((oid + '\n').encode())
                process.stdin.flush()
                header = process.stdout.readline().split()
                if len(header) != 3 or header[1] != b'blob':
                    raise ValueError('Expected artifact blob: ' + oid)
                size = int(header[2])
                with store.db:
                    sha = store.put(process.stdout, size, oid)
                    store.db.execute('INSERT OR IGNORE INTO versions VALUES (?,?,?)', (oid, name, sha))
                if process.stdout.read(1) != b'\n':
                    raise ValueError('Invalid Git blob framing')
                if i % 500 == 0:
                    print(json.dumps({'imported': i + 1, 'total': len(names)}), flush=True)
            with store.db:
                for oid, name in sorted(versions):
                    row = store.db.execute('SELECT sha256 FROM objects WHERE git_blob=?', (oid,)).fetchone()
                    if row is None:
                        raise ValueError('Unstored artifact version: ' + oid)
                    store.db.execute('INSERT OR IGNORE INTO versions VALUES (?,?,?)', (oid, name, row[0]))
            count = store.verify()
    finally:
        process.stdin.close()
        process.stdout.close()
        process.wait()
    return {'objects_verified': count, 'git_artifact_blobs': len(names)}


def freeze(database, output, tag, part_mib=48):
    """Create immutable, bounded release assets and a deterministic pinned manifest."""
    output = Path(output)
    output.mkdir(parents=True, exist_ok=False)
    # The SQLite backup API captures a consistent database, including live writers.
    snapshot = output/'snapshot.sqlite3'
    with opened_store(database, readonly=True) as store:
        dest = sqlite3.connect(snapshot)
        try:
            store.db.backup(dest)
        finally:
            dest.close()
    sha, size = digest_file(snapshot), snapshot.stat().st_size
    compressed = output/'snapshot.sqlite3.gz'
    with snapshot.open('rb') as src, compressed.open('xb') as raw:
        with gzip.GzipFile(fileobj=raw, mode='wb', mtime=0, compresslevel=6) as dest:
            shutil.copyfileobj(src, dest, CHUNK)
    parts = []
    with compressed.open('rb') as src:
        ordinal = 0
        while True:
            data = src.read(part_mib * CHUNK)
            if not data:
                break
            name = sha + '.sqlite3.gz.part' + str(ordinal).zfill(4)
            (output/name).write_bytes(data)
            parts.append({'name': name, 'size': len(data), 'sha256': hashlib.sha256(data).hexdigest(),
                          'url': f'https://github.com/benhoff/magic_and_mayhem_mod/releases/download/{tag}/{name}'})
            ordinal += 1
    manifest = {'schema': 1, 'archives': [{'sha256': sha, 'size': size, 'parts': parts}]}
    (output/CONFIG).write_text(json.dumps(manifest, indent=2) + '\n')
    return manifest
