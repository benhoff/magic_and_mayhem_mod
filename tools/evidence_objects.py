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
import stat
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
    return Pair(*paths, minimum_free=tuple(n*1024**3 for n in values), compression_workers=data.get("compression_workers",4))


def _sync(path):
    with Path(path).open('rb') as f:
        os.fsync(f.fileno())
    fd = os.open(Path(path).parent, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


class Pair:
    def __init__(self, primary, replica, *, minimum_free=(0, 0), require_independent=True, compression_workers=4):
        from coverage_evidence import Store
        from evidence_blocks import CompressionPool
        self.compression=CompressionPool(compression_workers)
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
        self.compression.close()
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

    def put(self, stream, size, git_blob=None, *, prepared=None):
        if type(size) is not int or size < 0:
            raise ValueError('Invalid evidence object length')
        if prepared is not None:
            if prepared.size!=size:raise ValueError('Prepared evidence size differs')
            return self._put_prepared(prepared,git_blob)
        from evidence_blocks import prepare
        with prepare(self.stores,stream,size,self.compression) as value:
            return self._put_prepared(value,git_blob)

    def _put_prepared(self,value,git_blob):
        sha=value.sha256
        for index,s in enumerate(self.stores):
            if not s.has(sha,value.size):
                with self.writing(s):
                    if s.put(None,value.size,git_blob,prepared=value)!=sha:
                        raise ValueError('Object changed during replication')
                if not self.batching:_sync(s.path)
            # Reuse encoding, never the integrity decision for either replica.
            identity=(index,sha,value.size)
            if not self.batching or identity not in self.batch_verified:
                s.copy(sha,value.size,Sink())
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
    sealed = _sealed_source(source)
    with configured(root) as pair:
        if sealed is not None:
            reused = _reuse_logical(pair, source, sealed)
            if reused is not None:
                return reused
        with pair.batch():
            with closing(sqlite3.connect(source.as_uri()+'?mode=ro',uri=True)) as db:
                db.execute('BEGIN')
                if db.execute('PRAGMA integrity_check').fetchone()!=('ok',):raise ValueError('Invalid SQLite source')
                catalog=_sqlite_catalog(db,pair)
            value=dict(schema=1,kind='sqlite_logical_checkpoint',source=str(source),catalog=catalog)
            manifest=pair.root('sqlite_logical',source,value)
        with tempfile.TemporaryDirectory(prefix='mm-logical-restore-') as tmp:
            restore_logical(pair,manifest,Path(tmp)/'restored.sqlite3')
        if sealed is not None and _sealed_identity(source) == sealed['identity']:
            pair.root('sqlite_logical_verified', source, dict(
                schema=1, kind='verified_sealed_sqlite_checkpoint',
                source=str(source), source_sha256=sealed['sha256'],
                source_size=sealed['size'], codec_sha256=_checkpoint_codec(),
                manifest=manifest, logical_restore_verified=True, replicas_verified=2))
    return dict(success=True,source=str(source),manifest=manifest,logical_restore_verified=True,
                replicas_verified=2,byte_exact_container=False,
                checkpoint_reused=False,real_restore_performed=True)


def _sealed_identity(source):
    """Only quiescent, mode-read-only files can reuse a restore proof. Never chmod."""
    s = source.stat()
    if not stat.S_ISREG(s.st_mode) or s.st_mode & 0o222:
        return None
    if any(p.exists() or p.is_symlink() for p in
           (Path(str(source)+suffix) for suffix in ('-wal', '-shm', '-journal'))):
        return None
    return (s.st_dev, s.st_ino, s.st_size, s.st_mtime_ns, s.st_ctime_ns,
            stat.S_IMODE(s.st_mode), s.st_uid, s.st_gid)


def _sealed_source(source):
    before = _sealed_identity(source)
    if before is None:
        return None
    with source.open('rb') as stream:
        sha = hashlib.file_digest(stream, 'sha256').hexdigest()
    if _sealed_identity(source) != before:
        raise ValueError('Sealed SQLite source changed during checksum')
    return dict(identity=before, sha256=sha, size=before[2])


def _checkpoint_codec():
    # Backup reuse is bound to the exact serializer/reader implementation, not
    # to a cached gate pass or to timestamps alone.
    files = {name: hashlib.sha256(Path(__file__).with_name(name).read_bytes()).hexdigest()
             for name in ('evidence_objects.py', 'coverage_evidence.py', 'evidence_blocks.py')}
    return hashlib.sha256(json.dumps(files, sort_keys=True).encode()).hexdigest()


def _verify_logical_closure(pair, manifest):
    """Freshly hash every referenced row/BLOB/text object in BOTH stores."""
    data = json.loads(pair.read(manifest['sha256'], manifest['size']))
    pair.read(manifest['sha256'], manifest['size'], 1)
    if data.get('kind') != 'sqlite_logical_checkpoint':
        raise ValueError('Wrong retained SQLite checkpoint kind')
    leaves = {}
    for table in data['catalog']['tables']:
        for ref in table['rows']:
            raw = pair.read(ref['sha256'], ref['size'])
            pair.read(ref['sha256'], ref['size'], 1)
            for value in json.loads(raw):
                if isinstance(value, dict) and ('blob' in value or 'text' in value):
                    leaf = value.get('blob', value.get('text'))
                    previous = leaves.setdefault(leaf['sha256'], leaf['size'])
                    if previous != leaf['size']:
                        raise ValueError('Conflicting checkpoint object sizes')
    for sha, size in leaves.items():
        for store in pair.stores:
            store.copy(sha, size, Sink())
    return data


def _reuse_logical(pair, source, sealed):
    if not pair.stores[0].db.execute(
            "SELECT 1 FROM sqlite_master WHERE name='retention_roots'").fetchone():
        return None
    codec = _checkpoint_codec()
    candidates = pair.stores[0].db.execute(
        "SELECT sha256,size FROM retention_roots WHERE kind='sqlite_logical_verified' AND source=?",
        (str(source),)).fetchall()
    for sha, size in candidates:
        value = json.loads(pair.read(sha, size))
        if (value.get('kind') != 'verified_sealed_sqlite_checkpoint'
                or value.get('schema') != 1 or value.get('source') != str(source)
                or value.get('source_sha256') != sealed['sha256']
                or value.get('source_size') != sealed['size']
                or value.get('codec_sha256') != codec):
            continue
        if value.get('logical_restore_verified') is not True or value.get('replicas_verified') != 2:
            raise ValueError('Invalid retained SQLite restore proof')
        pair.read(sha, size, 1)
        if not pair.stores[1].db.execute(
                "SELECT 1 FROM sqlite_master WHERE name='retention_roots'").fetchone():
            raise ValueError('Replica is missing the retained SQLite restore proof')
        row = pair.stores[1].db.execute(
            "SELECT size FROM retention_roots WHERE kind='sqlite_logical_verified' AND source=? AND sha256=?",
            (str(source), sha)).fetchone()
        if row != (size,):
            raise ValueError('Replica is missing the retained SQLite restore proof')
        data = _verify_logical_closure(pair, value['manifest'])
        if data['source'] != str(source) or _sealed_identity(source) != sealed['identity']:
            raise ValueError('Sealed SQLite source changed during checkpoint verification')
        return dict(success=True, source=str(source), manifest=value['manifest'],
            logical_restore_verified=True, replicas_verified=2, byte_exact_container=False,
            checkpoint_reused=True, real_restore_performed=False,
            source_sha256=sealed['sha256'],
            verification='Fresh source checksum and two-copy object closure; retained real-restore proof')
    return None


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
        return _checkpoint_objects(pair)


def _checkpoint_objects(pair):
    target = pair.stores[1].db
    # Store connections do not enable URI filenames. This canonical path was
    # validated by Pair; the attached schema is used only on SELECT sides.
    target.execute('ATTACH DATABASE ? AS checkpoint_source',
                   (str(pair.paths[0]),))
    try:
        if target.execute('''SELECT 1 FROM checkpoint_source.objects p JOIN main.objects r
                USING(sha256) WHERE p.size!=r.size LIMIT 1''').fetchone():
            raise ValueError('Replica object size collision')
        if target.execute('''SELECT 1 FROM checkpoint_source.versions p JOIN main.versions r
                USING(git_blob,path) WHERE p.sha256!=r.sha256 LIMIT 1''').fetchone():
            raise ValueError('Replica historical Git binding conflict')
        target.execute('''CREATE TEMP TABLE checkpoint_missing_objects AS
            SELECT p.sha256,p.size,p.git_blob FROM checkpoint_source.objects p
            LEFT JOIN main.objects r USING(sha256) WHERE r.sha256 IS NULL''')
        count = target.execute('SELECT COUNT(*) FROM checkpoint_source.objects').fetchone()[0]
        transferred = 0
        with pair.batch():
            for sha, size, blob in target.execute('SELECT * FROM checkpoint_missing_objects'):
                with tempfile.TemporaryFile() as stream:
                    pair.stores[0].copy(sha, size, stream)
                    stream.seek(0)
                    pair.put(stream, size, blob)
                transferred += 1
            target.execute('''INSERT OR IGNORE INTO main.versions
                SELECT p.git_blob,p.path,p.sha256 FROM checkpoint_source.versions p
                WHERE NOT EXISTS (SELECT 1 FROM main.versions r
                                  WHERE r.git_blob=p.git_blob AND r.path=p.path)''')
            # Existing root payloads still receive fresh two-copy integrity and
            # canonical-JSON checks. Only missing root metadata needs an insert.
            roots_exist = target.execute("SELECT 1 FROM sqlite_master WHERE name='retention_roots'").fetchone()
            for kind, source, sha, size in _roots(pair.stores[0]):
                body = pair.read(sha, size)
                pair.read(sha, size, 1)
                value = json.loads(body)
                canonical = (json.dumps(value, sort_keys=True, separators=(',', ':'))+'\n').encode()
                if canonical != body:
                    raise ValueError('Noncanonical retention root requires review')
                present = roots_exist and target.execute('''SELECT 1 FROM retention_roots
                    WHERE kind=? AND source=? AND sha256=? AND size=?''', (kind, source, sha, size)).fetchone()
                if not present:
                    if pair.root(kind, source, value) != dict(sha256=sha, size=size):
                        raise ValueError('Noncanonical retention root requires review')
                    roots_exist = True
        for columns, table in (('sha256,size', 'objects'), ('git_blob,path,sha256', 'versions')):
            for left, right in (('main', 'checkpoint_source'), ('checkpoint_source', 'main')):
                if target.execute(f'SELECT 1 FROM (SELECT {columns} FROM {left}.{table} EXCEPT SELECT {columns} FROM {right}.{table}) LIMIT 1').fetchone():
                    raise ValueError('Replica has divergent immutable identities; retain and review')
        if _roots(pair.stores[0]) != _roots(pair.stores[1]):
            raise ValueError('Replica retention roots diverge')
        return dict(success=True, objects=count, transferred_objects=transferred,
            missing_objects_replicated_and_verified=True, catalogs_match=True,
            backup_unit='immutable_object', physical_database_snapshot=False,
            catalog_comparison='bulk-sql', existing_objects_scrubbed=False)
    finally:
        target.execute('DROP TABLE IF EXISTS temp.checkpoint_missing_objects')
        target.execute('DETACH DATABASE checkpoint_source')
