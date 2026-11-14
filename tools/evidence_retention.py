"""Hybrid evidence custody: essential objects twice, historical bulk once.

Object identities stay immutable. Classification may only promote a bulk object
to essential during ordinary writes. Demotion is a separate reviewed migration.
"""
from pathlib import Path
import hashlib
import json

ESSENTIAL_SUFFIXES = {
    '.py', '.sh', '.c', '.cc', '.cpp', '.h', '.hpp', '.inc', '.s', '.cmake',
    '.json', '.yaml', '.yml', '.cfg', '.ini', '.reg', '.sav', '.save',
    '.exe', '.dll', '.so', '.dylib', '.sqlite3', '.sqlite', '.db',
}
SMALL_ESSENTIAL_BYTES = 64 * 1024


def file_copies(name, size):
    """Only explicitly admitted workspace files use the bulk classification.

    Reports, SQLite checkpoints and anonymous byte objects default to two copies
    at their callers. Small diagnostics, source, saves, configuration and tested
    executables beat a bulk classification even when shared with another file.
    """
    path = Path(name)
    if (size <= SMALL_ESSENTIAL_BYTES or path.suffix.lower() in ESSENTIAL_SUFFIXES
            or path.name.lower().startswith(('save', 'manifest', 'restore'))
            or 'user-saves' in path.parts or path.name == '.retain'):
        return 2
    return 1


def initialize(store):
    store.db.executescript('''
        CREATE TABLE IF NOT EXISTS retention_classes(
            sha256 TEXT PRIMARY KEY, copies INTEGER NOT NULL CHECK(copies IN (1,2)),
            reason TEXT NOT NULL);
        CREATE TABLE IF NOT EXISTS retention_decisions(
            id TEXT PRIMARY KEY, sha256 TEXT NOT NULL,
            copies INTEGER NOT NULL CHECK(copies IN (1,2)), reason TEXT NOT NULL);
        CREATE TABLE IF NOT EXISTS retention_roots(
            kind TEXT,source TEXT,sha256 TEXT REFERENCES objects(sha256),size INTEGER,
            PRIMARY KEY(kind,source,sha256));
        CREATE TRIGGER IF NOT EXISTS retention_classes_no_delete BEFORE DELETE ON retention_classes
            BEGIN SELECT RAISE(ABORT,'Retained custody classification'); END;
        CREATE TRIGGER IF NOT EXISTS retention_classes_promotion_only BEFORE UPDATE ON retention_classes
            WHEN OLD.sha256!=NEW.sha256 OR OLD.copies!=1 OR NEW.copies!=2
            BEGIN SELECT RAISE(ABORT,'Custody demotion requires reviewed migration'); END;
    ''')
    for table in ('retention_decisions', 'retention_roots'):
        for operation in ('update', 'delete'):
            store.db.execute(f'''CREATE TRIGGER IF NOT EXISTS {table}_no_{operation}
                BEFORE {operation} ON {table}
                BEGIN SELECT RAISE(ABORT,'Immutable custody decision'); END''')
    # A frozen writer must not refill a deliberately sparse mirror. Readers
    # retain the old objects/chunks protocol; only current writers register the
    # connection-local capability required by these triggers.
    for table in ('objects', 'blocks', 'object_parts', 'versions', 'retention_roots'):
        if not store.db.execute('SELECT 1 FROM sqlite_master WHERE name=?', (table,)).fetchone():
            continue
        store.db.execute(f'''CREATE TRIGGER IF NOT EXISTS {table}_retention_writer
            BEFORE INSERT ON {table} WHEN mm_retention_writer()!=1
            BEGIN SELECT RAISE(ABORT,'Use current hybrid-retention writer'); END''')
    store.db.execute("INSERT OR IGNORE INTO metadata VALUES('retention_policy','hybrid-v1')")
    store.db.commit()


def classes(db):
    if not db.execute("SELECT 1 FROM sqlite_master WHERE name='retention_classes'").fetchone():
        return {}
    return dict(db.execute('SELECT sha256,copies FROM retention_classes'))


def decide(store, sha, copies, reason):
    previous = store.db.execute('SELECT copies FROM retention_classes WHERE sha256=?', (sha,)).fetchone()
    if previous is not None and previous[0] >= copies:
        return previous[0]
    if previous is None:
        store.db.execute('INSERT INTO retention_classes VALUES(?,?,?)', (sha, copies, reason))
    else:
        store.db.execute('UPDATE retention_classes SET copies=?,reason=? WHERE sha256=?', (copies, reason, sha))
    raw = json.dumps([sha, copies, reason], separators=(',', ':')).encode()
    store.db.execute('INSERT OR IGNORE INTO retention_decisions VALUES(?,?,?,?)',
                     (hashlib.sha256(raw).hexdigest(), sha, copies, reason))
    return copies


def verify_catalogs(pair, attached=None):
    """Compare required identities; a bulk object's absent mirror is intentional."""
    replica = pair.stores[1].db
    source = attached or 'retention_source'
    if attached is None:
        replica.execute('ATTACH DATABASE ? AS retention_source', (str(pair.paths[0]),))
    try:
        checks = [
            (f'''SELECT 1 FROM {source}.retention_classes c LEFT JOIN {source}.objects p USING(sha256)
                WHERE p.sha256 IS NULL LIMIT 1''', 'Custody classification has no primary object'),
            (f'''SELECT 1 FROM main.objects r LEFT JOIN {source}.objects p USING(sha256)
                WHERE p.sha256 IS NULL OR p.size!=r.size LIMIT 1''', 'Replica has foreign or changed objects'),
            (f'''SELECT 1 FROM {source}.objects p LEFT JOIN main.objects r USING(sha256)
                LEFT JOIN {source}.retention_classes c USING(sha256)
                WHERE r.sha256 IS NULL AND coalesce(c.copies,2)=2 LIMIT 1''', 'Essential replica object is missing'),
        ]
        for sql, message in checks:
            if replica.execute(sql).fetchone():
                raise ValueError(message)
        for table, columns in (('retention_classes', 'sha256,copies,reason'),
                               ('retention_decisions', 'id,sha256,copies,reason'),
                               ('retention_roots', 'kind,source,sha256,size')):
            for left, right in (('main', source), (source, 'main')):
                if replica.execute(f'''SELECT 1 FROM (SELECT {columns} FROM {left}.{table}
                        EXCEPT SELECT {columns} FROM {right}.{table}) LIMIT 1''').fetchone():
                    raise ValueError('Replica custody/root catalog differs: ' + table)
        expected = f'''SELECT p.git_blob,p.path,p.sha256 FROM {source}.versions p
                       JOIN main.objects r ON r.sha256=p.sha256'''
        actual = 'SELECT git_blob,path,sha256 FROM main.versions'
        for left, right in ((expected, actual), (actual, expected)):
            if replica.execute(f'SELECT 1 FROM ({left} EXCEPT {right}) LIMIT 1').fetchone():
                raise ValueError('Replica required Git bindings differ')
    finally:
        if attached is None:
            replica.execute('DETACH DATABASE retention_source')
