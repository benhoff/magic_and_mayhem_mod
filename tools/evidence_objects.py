"""Project-wide immutable objects, two replicas and logical SQLite checkpoints.

Report identities are original-byte hashes. Database page layouts are not new
backup identities. Legacy files and their historical backup receipts remain
readable; adoption is explicit through mm.evidencePrimary/evidenceReplica.
"""
from contextlib import contextmanager, closing
import fcntl
import hashlib
import io
import json
import math
import os
from pathlib import Path
import shutil
import sqlite3
import subprocess
import tempfile


class Sink:
    def write(self, data):
        pass


def setting(root, key):
    p = subprocess.run(['git', '-C', str(root), 'config', '--get', key], capture_output=True, text=True)
    return p.stdout.strip() if p.returncode == 0 else ''


def settings(root):
    if not (Path(root)/'.git').exists():
        return None
    values = [setting(root, 'mm.evidencePrimary'), setting(root, 'mm.evidenceReplica')]
    if not any(values):
        return None
    if not all(values):
        raise ValueError('Configure both shared evidence replicas')
    paths = tuple(Path(v) for v in values)
    for p in paths:
        if not p.is_absolute() or 'working' not in p.parts or any(x in p.parts for x in ('original', '.git', '..')):
            raise ValueError('Shared evidence requires absolute ignored working paths')
        if any(x.is_symlink() for x in (p, *p.parents)):
            raise ValueError('Shared evidence paths must not traverse aliases')
    return paths


def storage_policy(root):
    policy = Path(setting(root, 'mm.evidenceStoragePolicy') or Path(root)/'tools/evidence-storage.json')
    return json.loads(policy.read_text())


def configured(root):
    paths = settings(root)
    if paths is None:
        raise ValueError('Shared evidence is not configured')
    data = storage_policy(root)
    if data['schema'] != 1 or data['durable_copies'] != 2:
        raise ValueError('Unsupported object storage policy')
    values = tuple(data[k] for k in ('primary_minimum_free_gib', 'replica_minimum_free_gib'))
    if any(type(n) not in (int, float) or not math.isfinite(n) or n < 0 for n in values):
        raise ValueError('Invalid evidence reserve')
    return Pair(*paths, minimum_free=tuple(n*1024**3 for n in values))


def _sync(path):
    with Path(path).open('rb') as f:
        os.fsync(f.fileno())
    fd = os.open(Path(path).parent, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


class Pair:
    def __init__(self, primary, replica, *, minimum_free=(0, 0), require_independent=True):
        from coverage_evidence import Store
        self.paths = tuple(Path(p).absolute() for p in (primary, replica))
        if self.paths[0] == self.paths[1]:
            raise ValueError('Distinct evidence replicas required')
        for p in self.paths:
            if any(x in p.parts for x in ('original', '.git', '..')) or any(x.is_symlink() for x in (p, *p.parents)):
                raise ValueError('Unsafe shared object store')
            p.parent.mkdir(parents=True, exist_ok=True)
        if require_independent and self.paths[0].parent.stat().st_dev == self.paths[1].parent.stat().st_dev:
            raise ValueError('Evidence replicas require independent filesystems')
        self.lock = self.paths[0].with_suffix('.lock').open('a+b')
        fcntl.flock(self.lock, fcntl.LOCK_EX)
        self.stores = []
        self.batching = False
        self.batch_verified=set()
        try:
            for p, floor in zip(self.paths, minimum_free):
                if shutil.disk_usage(p.parent).free < floor + 2*1024**2:
                    raise ValueError('Shared evidence reserve crossed')
                s = Store(p, shared=True)
                s.minimum_free = floor
                self.stores.append(s)
                pin = p.parent/'.retain'
                if not pin.exists():
                    pin.write_text('Shared immutable evidence objects, run roots and verified replica receipts. Preserve.\n')
        except BaseException:
            self.close()
            raise

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def close(self):
        for s in self.stores:
            s.close()
        self.stores = []
        self.lock.close()

    @contextmanager
    def batch(self):
        """Commit and fsync both stores before exposing a grouped handoff root.

        Failure may leave one durable prefix, but never authorizes publication
        or source retirement. Immutable identities make explicit retry safe.
        """
        if self.batching:
            raise ValueError('Nested object transactions are not supported')
        self.batching=True
        self.batch_verified.clear()
        try:
            for store in self.stores:store.db.execute('BEGIN')
            yield
            for store in self.stores:
                store.db.commit();_sync(store.path)
        except BaseException:
            for store in self.stores:store.db.rollback()
            raise
        finally:self.batching=False

    @contextmanager
    def writing(self,store):
        if self.batching:
            yield
        else:
            with store.db:yield

    def put(self, stream, size, git_blob=None):
        if type(size) is not int or size < 0:
            raise ValueError('Invalid evidence object length')
        start = stream.tell()
        h = hashlib.sha256()
        remaining = size
        while remaining:
            data = stream.read(min(1024**2, remaining))
            if not data:
                raise ValueError('Truncated evidence input')
            remaining -= len(data)
            h.update(data)
        sha = h.hexdigest()
        for index,s in enumerate(self.stores):
            if not s.has(sha, size):
                stream.seek(start)
                with self.writing(s):
                    if s.put(stream, size, git_blob) != sha:
                        raise ValueError('Object changed during replication')
                if not self.batching:_sync(s.path)
            # A successful put is not permission to trust a pre-existing row.
            identity=(index,sha,size)
            if not self.batching or identity not in self.batch_verified:
                s.copy(sha,size,Sink())
                if self.batching:self.batch_verified.add(identity)
        return sha

    def bytes(self, body):
        return self.put(io.BytesIO(body), len(body))

    def read(self, sha, size, replica=0):
        out = io.BytesIO()
        self.stores[replica].copy(sha, size, out)
        return out.getvalue()

    def root(self, kind, source, value):
        """An immutable root is itself ordinary evidence, replicated first."""
        body = (json.dumps(value, sort_keys=True, separators=(',', ':'))+'\n').encode()
        sha = self.bytes(body)
        for s in self.stores:
            with self.writing(s):
                s.db.execute('CREATE TABLE IF NOT EXISTS retention_roots(kind TEXT,source TEXT,sha256 TEXT REFERENCES objects(sha256),size INTEGER,PRIMARY KEY(kind,source,sha256))')
                for operation in ('update','delete'):
                    s.db.execute(f"CREATE TRIGGER IF NOT EXISTS roots_no_{operation} BEFORE {operation} ON retention_roots BEGIN SELECT RAISE(ABORT,'Immutable retention root'); END")
                s.db.execute('INSERT OR IGNORE INTO retention_roots VALUES(?,?,?,?)', (kind,str(source),sha,len(body)))
            if not self.batching:_sync(s.path)
        return dict(sha256=sha, size=len(body))

    def import_legacy(self, source):
        with self.batch():
            return self._import_legacy(source)

    def _import_legacy(self, source):
        from coverage_evidence import Store
        from evidence_consolidation import description
        old = Store(source, readonly=True)
        try:
            if old.shared:
                raise ValueError('Legacy import requires a source-specific legacy store')
            old.db.execute('BEGIN')
            catalog = json.loads(json.dumps(description(old)))
            added = 0
            for sha, size, blob in catalog['objects']:
                if all(s.has(sha, size) for s in self.stores):
                    for s in self.stores:
                        s.copy(sha, size, Sink())
                    continue
                with tempfile.TemporaryFile() as f:
                    old.copy(sha, size, f)
                    f.seek(0)
                    if self.put(f, size, blob) != sha:
                        raise ValueError('Imported evidence differs')
                added += 1
            for s in self.stores:
                with self.writing(s):
                    for blob, path, sha in catalog['versions']:
                        prior = s.db.execute('SELECT sha256 FROM versions WHERE git_blob=? AND path=?',(blob,path)).fetchone()
                        if prior is not None and prior != (sha,):
                            raise ValueError('Historical Git blob binding conflict')
                        s.db.execute('INSERT OR IGNORE INTO versions VALUES(?,?,?)',(blob,path,sha))
            manifest = dict(schema=1, kind='legacy_evidence_catalog', source=str(Path(source).absolute()), catalog=catalog,
                source_schema=[list(row) for row in old.db.execute("SELECT type,name,tbl_name,sql FROM sqlite_master WHERE sql IS NOT NULL ORDER BY type,name")])
            root = self.root('legacy_evidence', source, manifest)
            return dict(success=True, objects=len(catalog['objects']), added_objects=added, manifest=root,
                        original_report_hashes_preserved=True, replicas_verified=2)
        finally:
            old.close()


def restore_legacy(pair, manifest, target, replica=1):
    """Reconstruct all original logical tables; no container-byte identity claim."""
    from coverage_evidence import Store
    data = json.loads(pair.read(manifest['sha256'], manifest['size'], replica))
    if data['kind'] != 'legacy_evidence_catalog':
        raise ValueError('Wrong legacy catalog')
    target = Path(target)
    if target.exists() or target.is_symlink() or any(x in target.parts for x in ('original', '.git', '..')):
        raise ValueError('Legacy restore requires a new safe file')
    target.parent.mkdir(parents=True, exist_ok=True)
    dest = Store(target)
    try:
        with dest.db:
            for sha, size, blob in data['catalog']['objects']:
                dest.db.execute('INSERT INTO objects VALUES(?,?,?)', (sha,size,blob))
                dest.db.executemany('INSERT INTO chunks VALUES(?,?,?)', pair.stores[replica].db.execute('SELECT sha256,ordinal,payload FROM chunks WHERE sha256=? ORDER BY ordinal', (sha,)))
            dest.db.executemany('INSERT INTO versions VALUES(?,?,?)', data['catalog']['versions'])
            for key, value in data['catalog']['metadata']:
                if key != 'schema':
                    dest.db.execute('INSERT INTO metadata VALUES(?,?)', (key,value))
            for t in data['catalog']['auxiliary']:
                dest.db.execute(t['sql'])
                if t['rows']:
                    dest.db.executemany('INSERT INTO "'+t['name']+'" VALUES('+','.join('?' for _ in t['rows'][0])+')', t['rows'])
        # Preserve original auxiliary indexes/views/triggers, not only rows.
        with dest.db:
            for kind,name in list(dest.db.execute("SELECT type,name FROM sqlite_master WHERE type IN ('index','view','trigger') AND sql IS NOT NULL")):
                dest.db.execute('DROP '+kind+' '+_quote(name))
            for kind,name,table,sql in data['source_schema']:
                if kind!='table':dest.db.execute(sql)
        actual_schema=[list(row) for row in dest.db.execute("SELECT type,name,tbl_name,sql FROM sqlite_master WHERE sql IS NOT NULL ORDER BY type,name")]
        if actual_schema!=data['source_schema']:raise ValueError('Legacy source schema requires a dedicated restore adapter')
        dest.verify()
        from evidence_consolidation import description
        if json.loads(json.dumps(description(dest))) != data['catalog']:
            raise ValueError('Restored legacy logical catalog differs')
    finally:
        dest.close()
    _sync(target)
    return target


def _quote(name):
    return '"'+name.replace('"','""')+'"'


def _sqlite_catalog(db,pair):
    """Preserve rowids, BLOBs, embedded NULs and IEEE values explicitly."""
    schema=list(db.execute("SELECT type,name,tbl_name,sql FROM sqlite_master WHERE sql IS NOT NULL ORDER BY type,name"))
    tables=[]
    def encode(value):
        if isinstance(value,bytes):return {'blob':dict(sha256=pair.bytes(value),size=len(value))}
        if isinstance(value,str) and len(value)>4096:
            data=value.encode();return {'text':dict(sha256=pair.bytes(data),size=len(data))}
        if isinstance(value,float):return {'float':value.hex()}
        return value
    for kind,name,table_name,sql in schema:
        if kind!='table':continue
        if sql.lstrip().upper().startswith('CREATE VIRTUAL TABLE'):
            raise ValueError('Virtual SQLite tables require an explicit checkpoint adapter')
        info=list(db.execute('PRAGMA table_xinfo('+_quote(name)+')'))
        if any(r[6] for r in info):raise ValueError('Generated/hidden columns require a checkpoint adapter')
        columns=[r[1] for r in info]
        alias=next((n for n in ('rowid','_rowid_','oid') if n not in columns),None)
        use_rowid=alias is not None
        try:
            cursor=db.execute('SELECT '+(alias+',' if use_rowid else '')+'* FROM '+_quote(name)+(' ORDER BY '+alias if use_rowid else ''))
        except sqlite3.OperationalError:
            use_rowid=False
            keys=[r[1] for r in sorted(info,key=lambda r:r[5]) if r[5]] or columns
            cursor=db.execute('SELECT * FROM '+_quote(name)+' ORDER BY '+','.join(map(_quote,keys)))
        refs=[]
        for row in cursor:
            raw=(json.dumps([encode(v) for v in row],ensure_ascii=True,separators=(',',':'))+'\n').encode()
            refs.append(dict(sha256=pair.bytes(raw),size=len(raw)))
        tables.append(dict(name=name,columns=columns,rowid=alias if use_rowid else None,rows=refs))
    return dict(schema=[list(row) for row in schema],tables=tables,user_version=db.execute('PRAGMA user_version').fetchone()[0],application_id=db.execute('PRAGMA application_id').fetchone()[0])


def restore_logical(pair,manifest,target,replica=1):
    data=json.loads(pair.read(manifest['sha256'],manifest['size'],replica))
    if data['kind']!='sqlite_logical_checkpoint':raise ValueError('Wrong logical checkpoint')
    target=Path(target)
    if target.exists() or target.is_symlink() or any(p.is_symlink() for p in target.parents):
        raise ValueError('Logical restore requires a new safe file')
    target.parent.mkdir(parents=True,exist_ok=True)
    def decode(value):
        if not isinstance(value,dict):return value
        if 'float' in value:return float.fromhex(value['float'])
        key='blob' if 'blob' in value else 'text';r=value[key]
        raw=pair.read(r['sha256'],r['size'],replica)
        return raw if key=='blob' else raw.decode()
    catalog=data['catalog']
    with closing(sqlite3.connect(target)) as db:
        db.execute('PRAGMA foreign_keys=OFF')
        for kind,name,_,sql in catalog['schema']:
            if kind=='table' and name!='sqlite_sequence':db.execute(sql)
        for table in sorted(catalog['tables'],key=lambda t:t['name']=='sqlite_sequence'):
            name=table['name'];columns=([table['rowid']] if table['rowid'] else [])+table['columns']
            if name=='sqlite_sequence':db.execute('DELETE FROM sqlite_sequence')
            insert='INSERT INTO '+_quote(name)+'('+','.join(map(_quote,columns))+') VALUES('+','.join('?' for _ in columns)+')'
            for ref in table['rows']:
                values=json.loads(pair.read(ref['sha256'],ref['size'],replica))
                db.execute(insert,[decode(v) for v in values])
        for kind,_,_,sql in catalog['schema']:
            if kind!='table':db.execute(sql)
        db.execute('PRAGMA user_version='+str(catalog['user_version']))
        db.execute('PRAGMA application_id='+str(catalog['application_id']))
        if db.execute('PRAGMA integrity_check').fetchone()!=('ok',) or db.execute('PRAGMA foreign_key_check').fetchall():
            raise ValueError('Restored logical SQLite integrity failure')
        if _sqlite_catalog(db,pair)!=catalog:raise ValueError('Logical SQLite records changed on restore')
        db.commit()
    _sync(target)
    return target


def logical_checkpoint(root, source):
    """Back up each schema/row/value object once, including committed WAL data."""
    source=Path(source).resolve()
    if any(p in source.parts for p in ('original','.git')):raise ValueError('Unsafe logical checkpoint')
    with configured(root) as pair:
        with pair.batch():
            with closing(sqlite3.connect(source.as_uri()+'?mode=ro',uri=True)) as db:
                db.execute('BEGIN')
                if db.execute('PRAGMA integrity_check').fetchone()!=('ok',):raise ValueError('Invalid SQLite source')
                catalog=_sqlite_catalog(db,pair)
            value=dict(schema=1,kind='sqlite_logical_checkpoint',source=str(source),catalog=catalog)
            manifest=pair.root('sqlite_logical',source,value)
        with tempfile.TemporaryDirectory(prefix='mm-logical-restore-') as tmp:
            restore_logical(pair,manifest,Path(tmp)/'restored.sqlite3')
    return dict(success=True,source=str(source),manifest=manifest,logical_restore_verified=True,
                replicas_verified=2,byte_exact_container=False)


def _roots(store):
    if not store.db.execute("SELECT 1 FROM sqlite_master WHERE name='retention_roots'").fetchone():
        return []
    return list(store.db.execute('SELECT kind,source,sha256,size FROM retention_roots ORDER BY kind,source,sha256'))


def verify_pair(pair):
    counts=[s.verify() for s in pair.stores]
    for sql in ('SELECT sha256,size FROM objects ORDER BY sha256',
                'SELECT git_blob,path,sha256 FROM versions ORDER BY git_blob,path'):
        if list(pair.stores[0].db.execute(sql))!=list(pair.stores[1].db.execute(sql)):
            raise ValueError('Replica object/version identities differ')
    if _roots(pair.stores[0])!=_roots(pair.stores[1]):
        raise ValueError('Replica retention roots differ')
    return counts


def checkpoint_objects(root):
    """Incremental mirror completion; immutable objects/roots are the backup unit.

    Existing objects are not a full scrub here; verify_pair performs that check.
    Interrupted transfers preserve primary roots and immutable retry identities.
    """
    with configured(root) as pair:
        objects=list(pair.stores[0].db.execute('SELECT sha256,size,git_blob FROM objects ORDER BY sha256'))
        transferred=0
        with pair.batch():
            for sha,size,blob in objects:
                if pair.stores[1].has(sha,size):continue
                with tempfile.TemporaryFile() as f:
                    pair.stores[0].copy(sha,size,f);f.seek(0);pair.put(f,size,blob)
                transferred+=1
            target=pair.stores[1].db
            for blob,path,sha in pair.stores[0].db.execute('SELECT git_blob,path,sha256 FROM versions'):
                previous=target.execute('SELECT sha256 FROM versions WHERE git_blob=? AND path=?',(blob,path)).fetchone()
                if previous is not None and previous!=(sha,):raise ValueError('Replica historical Git binding conflict')
                target.execute('INSERT OR IGNORE INTO versions VALUES(?,?,?)',(blob,path,sha))
            for kind,source,sha,size in _roots(pair.stores[0]):
                if pair.root(kind,source,json.loads(pair.read(sha,size)))!=dict(sha256=sha,size=size):
                    raise ValueError('Noncanonical retention root requires review')
        for sql in ('SELECT sha256,size FROM objects ORDER BY sha256',
                    'SELECT git_blob,path,sha256 FROM versions ORDER BY git_blob,path'):
            if list(pair.stores[0].db.execute(sql))!=list(pair.stores[1].db.execute(sql)):
                raise ValueError('Replica has divergent immutable identities; retain and review')
        if _roots(pair.stores[0])!=_roots(pair.stores[1]):raise ValueError('Replica retention roots diverge')
        return dict(success=True,objects=len(objects),transferred_objects=transferred,
                    missing_objects_replicated_and_verified=True,catalogs_match=True,
                    backup_unit='immutable_object',physical_database_snapshot=False)
