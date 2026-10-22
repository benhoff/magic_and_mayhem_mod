"""Local-only evidence access and consistent, verified SQLite backups."""
from contextlib import closing, contextmanager
import os
from pathlib import Path
import sqlite3
import tempfile

from coverage_evidence import Store, cache_root, config, digest_file


@contextmanager
def local_evidence(root):
    """Bind temporary Git trees to local evidence; refuse missing cache files.

    Published archives are preflighted before any reader can attempt a fetch.
    Source and contract checks still run against each actual working/staged tree.
    """
    root = Path(root).resolve()
    cache = cache_root(root)
    for archive in (config(root) or {}).get('archives', []):
        path = cache/(archive['sha256'] + '.sqlite3')
        if not path.is_file() or digest_file(path) != archive['sha256']:
            raise ValueError('Evidence archive unavailable locally; run evidence-store.py fetch --verify: '
                             + archive['sha256'])
    previous = {key: os.environ.get(key) for key in ('MM_EVIDENCE_DB', 'MM_EVIDENCE_CACHE')}
    pending = root/'working/coverage/evidence-pending.sqlite3'
    try:
        os.environ['MM_EVIDENCE_CACHE'] = str(cache)
        if not previous['MM_EVIDENCE_DB'] and pending.is_file():
            os.environ['MM_EVIDENCE_DB'] = str(pending.resolve())
        yield
    finally:
        for key, value in previous.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value


def backup_sqlite(source, directory):
    """Snapshot a live database through SQLite, verify, fsync and retain it.

    A local SQLite catalog records backups; immutable files are keyed by SHA-256.
    No report export, publication or deletion is performed.
    """
    source, directory = Path(source).resolve(), Path(directory).resolve()
    if not source.is_file():
        raise ValueError('SQLite backup input does not exist: ' + str(source))
    directory.mkdir(parents=True, exist_ok=True)
    fd, name = tempfile.mkstemp(prefix='snapshot-', suffix='.sqlite3', dir=directory)
    os.close(fd)
    temporary = Path(name)
    try:
        with closing(sqlite3.connect(source.as_uri() + '?mode=ro', uri=True)) as src:
            with closing(sqlite3.connect(temporary)) as dest:
                src.backup(dest)
                if dest.execute('PRAGMA integrity_check').fetchone() != ('ok',):
                    raise ValueError('SQLite snapshot integrity failure')
                if dest.execute('PRAGMA foreign_key_check').fetchall():
                    raise ValueError('SQLite snapshot foreign-key failure')
                tables = {row[0] for row in dest.execute("SELECT name FROM sqlite_master WHERE type='table'")}
        objects = None
        if {'objects', 'chunks'} <= tables:
            store = Store(temporary, readonly=True)
            try:
                objects = store.verify()
            finally:
                store.close()
        sha = digest_file(temporary)
        destination = directory/(sha + '.sqlite3')
        with temporary.open('rb') as stream:
            os.fsync(stream.fileno())
        if destination.exists():
            if digest_file(destination) != sha:
                raise ValueError('Existing backup checksum mismatch: ' + str(destination))
        else:
            os.replace(temporary, destination)
        pin = directory/'.retain'
        with pin.open('a') as stream:
            if pin.stat().st_size == 0:
                stream.write('Verified local SQLite evidence/review backups. Preserve referenced snapshots.\n')
            stream.flush()
            os.fsync(stream.fileno())
        with closing(sqlite3.connect(directory/'catalog.sqlite3')) as catalog, catalog:
            catalog.execute('CREATE TABLE IF NOT EXISTS backups ('
                            'sha256 TEXT PRIMARY KEY, source TEXT, bytes INTEGER, objects INTEGER)')
            catalog.execute('INSERT OR IGNORE INTO backups VALUES (?,?,?,?)',
                            (sha, str(source), destination.stat().st_size, objects))
        descriptor = os.open(directory, os.O_RDONLY | os.O_DIRECTORY)
        try:
            os.fsync(descriptor)
        finally:
            os.close(descriptor)
        return {'path': str(destination), 'sha256': sha,
                'bytes': destination.stat().st_size, 'objects_verified': objects}
    finally:
        temporary.unlink(missing_ok=True)
