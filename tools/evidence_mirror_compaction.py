"""Reviewed sparse-mirror construction; never delete the surviving bulk copy."""
from contextlib import closing,contextmanager
from pathlib import Path
import fcntl
import hashlib
import io
import json
import os
import shutil
import sqlite3
import time

from coverage_evidence import Store
from evidence_objects import Sink,_sync
from evidence_retention import classes,decide,file_copies,initialize,verify_catalogs

TABLES={
 'objects':'sha256,size,git_blob',
 'blocks':'sha256,size,length(payload)',
 'object_parts':'sha256,ordinal,block',
 'versions':'git_blob,path,sha256',
 'retention_roots':'kind,source,sha256,size',
}

def encoded(data):return json.dumps(data,sort_keys=True,separators=(',',':')).encode()

def identity(path):
    s=Path(path).lstat()
    return [s.st_dev,s.st_ino,s.st_size,s.st_mtime_ns,s.st_ctime_ns]

def fingerprint(db):
    result={}
    for table,columns in TABLES.items():
        h=hashlib.sha256();count=0
        for row in db.execute(f'SELECT {columns} FROM {table} ORDER BY '+columns.replace('length(payload)','size')):
            h.update(encoded(row)+b'\n');count+=1
        result[table]=dict(rows=count,sha256=h.hexdigest())
    return result

def read(store,sha,size):
    out=io.BytesIO();store.copy(sha,size,out);return out.getvalue()

@contextmanager
def locked(primary):
    with Path(primary).with_suffix('.lock').open('a+b')as handle:
        fcntl.flock(handle,fcntl.LOCK_EX);yield

def paths_safe(primary,replica):
    primary,replica=Path(primary).absolute(),Path(replica).absolute()
    for path in(primary,replica):
        if ('working'not in path.parts or any(n in path.parts for n in ('original','.git','..'))
                or any(p.is_symlink()for p in(path,*path.parents))or not path.is_file()
                or path.stat().st_nlink!=1):raise ValueError('Select physical, singly linked evidence stores')
    if primary.stat().st_dev==replica.stat().st_dev:raise ValueError('Independent evidence filesystems required')
    return primary,replica

def prepare(primary,replica,output):
    primary,replica=paths_safe(primary,replica);output=Path(output).absolute()
    if output.exists()or output.suffix!='.sqlite3' or 'working'not in output.parts or any(p.is_symlink()for p in output.parents):
        raise ValueError('Plan requires a new ignored SQLite path')
    output.parent.mkdir(parents=True,exist_ok=True)
    with locked(primary),closing(Store(primary,readonly=True))as source,closing(Store(replica,readonly=True))as mirror,closing(sqlite3.connect(output))as plan:
        source.db.execute('BEGIN');mirror.db.execute('BEGIN')
        plan.executescript('''PRAGMA synchronous=FULL;
            CREATE TABLE metadata(key TEXT PRIMARY KEY,payload BLOB);
            CREATE TABLE bulk(sha256 TEXT PRIMARY KEY,size INTEGER,reason TEXT);
            CREATE TABLE protect(sha256 TEXT PRIMARY KEY,reason TEXT);
            CREATE TABLE checkpoint_rows(sha256 TEXT PRIMARY KEY,size INTEGER);
            CREATE TABLE events(at_ns INTEGER,state TEXT,payload BLOB);''')
        first=fingerprint(source.db);second=fingerprint(mirror.db)
        if first!=second:raise ValueError('Full source/mirror catalogs differ; preserve and reconcile before compaction')
        plan.execute('INSERT INTO metadata VALUES(?,?)',('catalog',encoded(first)))
        plan.execute('INSERT INTO metadata VALUES(?,?)',('paths',encoded([str(primary),str(replica)])))
        # Every existing root and Git-bound report stays mirrored. Only explicit
        # workspace file members can enter the one-copy class.
        plan.executemany('INSERT OR IGNORE INTO protect VALUES(?,?)',((sha,'root metadata')for sha, in source.db.execute('SELECT sha256 FROM retention_roots')))
        plan.executemany('INSERT OR IGNORE INTO protect VALUES(?,?)',((sha,'Git-bound evidence')for sha, in source.db.execute('SELECT sha256 FROM versions')))
        if source.db.execute("SELECT 1 FROM sqlite_master WHERE name='retention_classes'").fetchone():
            plan.executemany('INSERT OR IGNORE INTO protect VALUES(?,?)',((sha,'explicit essential custody')for sha, in source.db.execute('SELECT sha256 FROM retention_classes WHERE copies=2')))
        roots=list(source.db.execute("SELECT kind,sha256,size FROM retention_roots WHERE kind IN ('execution_workspace','sqlite_logical','legacy_evidence','tracked_worktree_sources')"))
        for index,(kind,sha,size)in enumerate(roots):
            value=json.loads(read(source,sha,size))
            if kind in ('execution_workspace','tracked_worktree_sources'):
                for row in value['members']:
                    if row.get('kind','file')!='file':continue
                    target='bulk'if kind=='execution_workspace'and file_copies(row['path'],row['size'])==1 else 'protect'
                    if target=='bulk':plan.execute('INSERT OR IGNORE INTO bulk VALUES(?,?,?)',(row['sha256'],row['size'],'historical workspace bulk'))
                    else:plan.execute('INSERT OR IGNORE INTO protect VALUES(?,?)',(row['sha256'],'source/save/configuration/tested binary'))
            elif kind=='sqlite_logical':
                for table in value['catalog']['tables']:
                    plan.executemany('INSERT OR IGNORE INTO checkpoint_rows VALUES(?,?)',((r['sha256'],r['size'])for r in table['rows']))
            else:
                plan.executemany('INSERT OR IGNORE INTO protect VALUES(?,?)',((r[0],'legacy report catalog')for r in value['catalog']['objects']))
            if index%100==0:plan.commit();print('Reviewed root',index,'of',len(roots),flush=True)
        # Retain complete logical backup closures, not just their root hashes.
        for sha,size in plan.execute('SELECT sha256,size FROM checkpoint_rows'):
            plan.execute('INSERT OR IGNORE INTO protect VALUES(?,?)',(sha,'SQLite row'))
            for value in json.loads(read(source,sha,size)):
                if isinstance(value,dict)and('blob'in value or'text'in value):
                    ref=value.get('blob',value.get('text'));plan.execute('INSERT OR IGNORE INTO protect VALUES(?,?)',(ref['sha256'],'SQLite BLOB/text'))
        plan.execute('DELETE FROM bulk WHERE sha256 IN(SELECT sha256 FROM protect)')
        # No accidental executable demotion, even if an old observer called its
        # executable a .bin file. Only the first four decoded bytes are needed.
        for sha,size in list(plan.execute('SELECT sha256,size FROM bulk')):
            row=source.db.execute('SELECT payload FROM chunks WHERE sha256=? AND ordinal=0',(sha,)).fetchone()
            if row is None:raise ValueError('Bulk member is absent from primary')
            import zlib
            head=zlib.decompressobj().decompress(row[0],4)
            if head.startswith((b'MZ',b'\x7fELF')):plan.execute('DELETE FROM bulk WHERE sha256=?',(sha,))
        # A private TEMP selection avoids copying any payload to estimate the
        # new mirror. Blocks shared with essential data remain in that mirror.
        source.db.execute('CREATE TEMP TABLE one_copy(sha256 TEXT PRIMARY KEY)')
        source.db.executemany('INSERT INTO one_copy VALUES(?)',((r[0],)for r in plan.execute('SELECT sha256 FROM bulk')))
        retained=source.db.execute('''SELECT coalesce(sum(length(payload)),0)FROM blocks WHERE sha256 IN(
            SELECT DISTINCT p.block FROM object_parts p LEFT JOIN one_copy b ON b.sha256=p.sha256 WHERE b.sha256 IS NULL)''').fetchone()[0]
        count=plan.execute('SELECT count(*)FROM bulk').fetchone()[0]
        rows=source.db.execute('SELECT count(*)FROM objects').fetchone()[0]
        result=dict(schema=1,primary=str(primary),replica=str(replica),catalog=first,
                    bulk_objects=count,essential_objects=rows-count,
                    replica_allocated_bytes=replica.stat().st_blocks*512,
                    estimated_retained_block_bytes=retained,
                    reason='User-approved hybrid retention: verified historical bulk one copy; metadata, source, saves, tested executables and complete review/checkpoint closures two copies.')
        plan.execute('INSERT INTO metadata VALUES(?,?)',('plan',encoded(result)));plan.commit()
        h=hashlib.sha256(encoded(result))
        for row in plan.execute('SELECT * FROM bulk ORDER BY sha256'):h.update(encoded(row)+b'\n')
        sha=h.hexdigest();plan.execute('INSERT INTO metadata VALUES(?,?)',('plan_sha256',sha.encode()));plan.commit()
        return dict(result,plan_sha256=sha)

def event(plan,state,value):
    plan.execute('INSERT INTO events VALUES(?,?,?)',(time.time_ns(),state,encoded(value)));plan.commit()

def apply(plan_path,expected_sha,*,minimum_free=(100*2**30,90*2**30),progress_every=10000):
    from compact_snapshot_activity import busy
    with closing(sqlite3.connect(plan_path))as plan:
        value=json.loads(plan.execute("SELECT payload FROM metadata WHERE key='plan'").fetchone()[0])
        h=hashlib.sha256(encoded(value))
        for row in plan.execute('SELECT * FROM bulk ORDER BY sha256'):h.update(encoded(row)+b'\n')
        if h.hexdigest()!=expected_sha:raise ValueError('Reviewed compaction plan changed')
        primary,replica=paths_safe(value['primary'],value['replica'])
        if busy(primary)or busy(replica):raise ValueError('Evidence store has an active consumer; retry at idle')
        replacement=replica.with_name(replica.name+'.hybrid-prepared')
        if replacement.exists():raise ValueError('Prior prepared mirror must be reviewed')
        # Refuse before producing output when the essential mirror cannot fit.
        estimate=(value['estimated_retained_block_bytes']*1.25
                  +value['essential_objects']*768+value['bulk_objects']*512+1024**3)
        if shutil.disk_usage(replica.parent).free-estimate<minimum_free[1]:raise ValueError('Essential mirror staging would breach reserve')
        with locked(primary),closing(Store(primary,readonly=True))as source:
            source.db.execute('BEGIN')
            if fingerprint(source.db)!=value['catalog']:raise ValueError('Primary changed after planning')
            with closing(Store(replica,readonly=True))as old:
                if fingerprint(old.db)!=value['catalog']:raise ValueError('Mirror changed after planning')
            event(plan,'verification-started',dict(primary=str(primary)))
            # Verify every surviving object, including bulk, before discarding
            # any old replica payload. This is a real full remaining-copy scrub.
            for index,(sha,size)in enumerate(source.db.execute('SELECT sha256,size FROM objects')):
                source.copy(sha,size,Sink())
                if index%progress_every==0:print('Verified surviving object',index,flush=True)
            event(plan,'primary-full-hash-verified',dict(catalog=value['catalog']))
            bulk={r[0]for r in plan.execute('SELECT sha256 FROM bulk')}
            with closing(Store(replacement,shared=True))as new:
                initialize(new);new.minimum_free=minimum_free[1]
                new.db.execute('PRAGMA journal_mode=DELETE')
                new.db.set_progress_handler(lambda: int(shutil.disk_usage(replica.parent).free<minimum_free[1]+64*1024**2),10000)
                # Copy only essential objects. The original primary remains
                # untouched and the old full mirror remains in place here.
                new.db.execute('ATTACH DATABASE ? AS retained_source',(str(primary),))
                new.db.execute('CREATE TEMP TABLE one_copy(sha256 TEXT PRIMARY KEY)')
                new.db.executemany('INSERT INTO one_copy VALUES(?)',((sha,)for sha in bulk))
                with new.db:
                    new.db.execute('''INSERT INTO objects SELECT o.*FROM retained_source.objects o
                        LEFT JOIN one_copy b USING(sha256)WHERE b.sha256 IS NULL''')
                    new.db.execute('''INSERT INTO blocks SELECT b.*FROM retained_source.blocks b
                        WHERE b.sha256 IN(SELECT DISTINCT p.block FROM retained_source.object_parts p
                            JOIN main.objects o ON o.sha256=p.sha256)''')
                    new.db.execute('''INSERT INTO object_parts SELECT p.*FROM retained_source.object_parts p
                        JOIN main.objects o ON o.sha256=p.sha256''')
                    new.db.execute('INSERT INTO retention_roots SELECT *FROM retained_source.retention_roots')
                    for sha in bulk:decide(new,sha,1,'approved historical workspace bulk')
                    new.db.execute('''INSERT INTO versions SELECT v.*FROM retained_source.versions v
                        JOIN main.objects o ON o.sha256=v.sha256''')
                    new.db.execute('''INSERT OR IGNORE INTO metadata SELECT *FROM retained_source.metadata''')
                    if source.db.execute("SELECT 1 FROM sqlite_master WHERE name='retention_classes'").fetchone():
                        new.db.execute('''INSERT OR IGNORE INTO retention_classes SELECT *FROM retained_source.retention_classes''')
                        new.db.execute('''INSERT OR IGNORE INTO retention_decisions SELECT *FROM retained_source.retention_decisions''')
                new.db.execute('DETACH DATABASE retained_source')
                print('Essential catalogs copied; verifying replacement',flush=True)
                new.verify();_sync(replacement)
            source.db.rollback()
            # Publish classification and frozen-writer guards only after both
            # the complete survivor and the replacement mirror have verified.
            with closing(Store(primary))as writer:
                initialize(writer)
                writer.db.set_progress_handler(lambda: int(shutil.disk_usage(primary.parent).free<minimum_free[0]+64*1024**2),10000)
                with writer.db:
                    for sha in bulk:decide(writer,sha,1,'approved historical workspace bulk')
                _sync(primary)
            class View:pass
            with closing(Store(primary,readonly=True))as checked,closing(Store(replacement,readonly=True))as new:
                pair=View();pair.stores=[checked,new];pair.paths=[primary,replacement];verify_catalogs(pair)
            if busy(replica):raise ValueError('Old mirror acquired a consumer before retirement')
            before=replica.stat().st_blocks*512
            event(plan,'replacement-verified',dict(replacement=str(replacement),essential_objects=value['essential_objects'],bulk_objects=len(bulk)))
            # All connections to replacement are closed and checkpointed before
            # the atomic pathname swap. Never unlink a live WAL or SHM file.
            with closing(sqlite3.connect(replica))as checkpoint:
                if checkpoint.execute('PRAGMA wal_checkpoint(TRUNCATE)').fetchone()[0]:raise ValueError('Old mirror has an active SQLite reader/writer')
                if checkpoint.execute('PRAGMA journal_mode=DELETE').fetchone()[0]!='delete':raise ValueError('Could not close old mirror journal')
            for suffix in('-wal','-shm','-journal'):
                if Path(str(replacement)+suffix).exists()or Path(str(replica)+suffix).exists():raise ValueError('SQLite sidecars remain; retain and review before swap')
            os.replace(replacement,replica);_sync(replica)
            result=dict(success=True,primary=str(primary),replica=str(replica),bulk_copies=1,essential_copies=2,
                        bulk_objects=len(bulk),essential_objects=value['essential_objects'],
                        remaining_primary_full_hash_verified=True,replacement_verified=True,
                        reclaimed_allocated_bytes=before-replica.stat().st_blocks*512)
            event(plan,'completed',result);return result
