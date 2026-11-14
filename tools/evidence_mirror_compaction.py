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
import tempfile
import subprocess
import inspect
import math

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
    with scratch(replica),locked(primary),closing(Store(primary,readonly=True))as source,closing(Store(replica,readonly=True))as mirror,closing(sqlite3.connect(output))as plan:
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

@contextmanager
def scratch(replica):
    """Scope SQLite spill for this isolated, single-threaded migration process.

    SQLite may cache its default directory before environment changes. Set and
    verify its process-wide directory as well; restore both settings on exit.
    """
    previous={key:os.environ.get(key)for key in ('SQLITE_TMPDIR','TMPDIR')}
    with tempfile.TemporaryDirectory(prefix='mirror-scratch-',dir=replica.parent)as directory,closing(sqlite3.connect(':memory:'))as control:
        row=control.execute('PRAGMA temp_store_directory').fetchone()
        prior=row[0]if row else''
        try:
            for key in previous:os.environ[key]=directory
            control.execute("PRAGMA temp_store_directory='"+directory.replace("'","''")+"'")
            row=control.execute('PRAGMA temp_store_directory').fetchone()
            if row is None or row[0]!=directory:raise ValueError('SQLite cannot select the budgeted temporary directory')
            yield
        finally:
            control.execute("PRAGMA temp_store_directory='"+prior.replace("'","''")+"'")
            for key,value in previous.items():
                if value is None:os.environ.pop(key,None)
                else:os.environ[key]=value


def reserve_check(paths,floors):
    if any(shutil.disk_usage(path.parent).free<floor+128*1024**2
           for path,floor in zip(paths,floors)):
        raise ValueError('Evidence or temporary-storage reserve crossed')


def guard_connection(db,paths,floors):
    def guard():
        return int(any(shutil.disk_usage(path.parent).free<floor+128*1024**2
                       for path,floor in zip(paths,floors)))
    db.set_progress_handler(guard,1000)


def physical_state(primary):
    result={str(primary):identity(primary)}
    for suffix in ('-wal','-journal'):
        path=Path(str(primary)+suffix)
        if path.exists()and path.stat().st_size:
            if suffix=='-journal':raise ValueError('Primary has a rollback journal; review before reuse')
            result[str(path)]=identity(path)
    return result


def decoder_revision(commit):
    root=Path(__file__).resolve().parents[1]
    if not commit:raise ValueError('Retained primary verification needs its reviewed source commit')
    revision=subprocess.check_output(['git','-C',str(root),'rev-parse','--verify','--end-of-options',commit+'^{commit}'],text=True).strip()
    original=subprocess.check_output(['git','-C',str(root),'show',revision+':tools/coverage_evidence.py'])
    current=Path(inspect.getfile(Store)).read_bytes()
    if original!=current:raise ValueError('Verified primary decoder source changed; perform a new full scrub')
    return dict(commit=revision,decoder_sha256=hashlib.sha256(current).hexdigest())


def retained_primary_proof(plan,source,value,commit):
    """Reuse an actual completed scrub, never a cached pass or changed source.

    Legacy phase receipts lack a physical stamp. Their file and nonempty WAL
    timestamps must precede the recorded scrub start, ruling out intervening
    filesystem writes/replacement on this trusted local filesystem. Catalogs
    are freshly compared separately. The reviewed decoder revision must match.
    """
    proof=plan.execute("SELECT at_ns,payload FROM events WHERE state='primary-full-hash-verified' ORDER BY at_ns DESC LIMIT 1").fetchone()
    if proof is None:raise ValueError('No completed primary full-hash verification to reuse')
    payload=json.loads(proof[1])
    start=plan.execute("SELECT at_ns FROM events WHERE state='verification-started' AND at_ns<=? ORDER BY at_ns DESC LIMIT 1",(proof[0],)).fetchone()
    if start is None or payload.get('catalog')!=value['catalog']:
        raise ValueError('Retained primary verification does not bind this catalog')
    state=physical_state(source.path)
    if any(max(stamp[3:])>start[0]for stamp in state.values()):
        raise ValueError('Primary storage changed since the retained scrub began')
    if 'physical_state'in payload and state!=payload['physical_state']:
        raise ValueError('Primary storage identity differs from verified phase')
    for table in ('objects','blocks','object_parts'):
        for operation in ('update','delete'):
            row=source.db.execute("SELECT sql FROM sqlite_master WHERE type='trigger' AND name=?",(table+'_no_'+operation,)).fetchone()
            if row is None or 'RAISE(ABORT'not in row[0].upper():
                raise ValueError('Primary immutable payload guard is missing')
    revision=decoder_revision(commit)
    return dict(mode='retained_full_scrub',verified_at_ns=proof[0],started_at_ns=start[0],
                physical_state=state,reviewed_decoder=revision,
                limitation='Fresh catalog/physical no-write checks and unchanged reviewed decoder preserve the prior object scrub; this invocation does not repeat it.')


def copy_blocks(new,paths,floors):
    # INSERT..SELECT with BEFORE INSERT guards makes SQLite materialize the
    # entire BLOB result in TEMP before inserting. Stream single blocks instead;
    # the IN set contains hashes only and deduplicates without another DISTINCT.
    rows=new.db.execute('''SELECT b.sha256,b.size,b.payload FROM retained_source.blocks b
        WHERE b.sha256 IN(SELECT p.block FROM retained_source.object_parts p
            JOIN main.objects o ON o.sha256=p.sha256)''')
    for row in rows:
        reserve_check(paths,floors)
        new.db.execute('INSERT INTO main.blocks VALUES(?,?,?)',row)


def apply(plan_path,expected_sha,*,minimum_free=(100*2**30,90*2**30),progress_every=10000,
          reuse_primary_verification=False,verified_source_commit=None):
    from compact_snapshot_activity import busy
    if len(minimum_free)!=2 or any(not math.isfinite(n)or n<0 for n in minimum_free):
        raise ValueError('Finite nonnegative storage floors required')
    if verified_source_commit and not reuse_primary_verification:
        raise ValueError('A verified source commit applies only to explicit phase reuse')
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
        with scratch(replica),locked(primary),closing(Store(primary,readonly=True))as source:
            paths=(primary,replica);reserve_check(paths,minimum_free)
            guard_connection(source.db,paths,minimum_free)
            source.db.execute('BEGIN')
            if fingerprint(source.db)!=value['catalog']:raise ValueError('Primary changed after planning')
            with closing(Store(replica,readonly=True))as old:
                guard_connection(old.db,paths,minimum_free)
                if fingerprint(old.db)!=value['catalog']:raise ValueError('Mirror changed after planning')
            if reuse_primary_verification:
                verification=retained_primary_proof(plan,source,value,verified_source_commit)
                event(plan,'primary-verification-reused',verification)
                print('Retained primary scrub accepted after fresh catalog, physical no-write and decoder-source checks',flush=True)
            else:
                before=physical_state(primary)
                event(plan,'verification-started',dict(primary=str(primary),physical_state=before))
                for index,(sha,size)in enumerate(source.db.execute('SELECT sha256,size FROM objects')):
                    source.copy(sha,size,Sink())
                    if index%progress_every==0:print('Verified surviving object',index,flush=True)
                if physical_state(primary)!=before:raise ValueError('Primary storage changed during full scrub')
                event(plan,'primary-full-hash-verified',dict(catalog=value['catalog'],physical_state=before))
                verification=dict(mode='fresh_full_scrub',physical_state=before)
            bulk={r[0]for r in plan.execute('SELECT sha256 FROM bulk')}
            reserve_check(paths,minimum_free)
            with closing(Store(replacement,shared=True))as new:
                initialize(new);new.minimum_free=minimum_free[1]
                new.db.execute('PRAGMA journal_mode=DELETE')
                guard_connection(new.db,paths,minimum_free)
                # Copy only essential objects. The original primary remains
                # untouched and the old full mirror remains in place here.
                new.db.execute('ATTACH DATABASE ? AS retained_source',(str(primary),))
                new.db.execute('CREATE TEMP TABLE one_copy(sha256 TEXT PRIMARY KEY)')
                new.db.executemany('INSERT INTO one_copy VALUES(?)',((sha,)for sha in bulk))
                with new.db:
                    new.db.execute('''INSERT INTO objects SELECT o.*FROM retained_source.objects o
                        LEFT JOIN one_copy b USING(sha256)WHERE b.sha256 IS NULL''')
                    copy_blocks(new,paths,minimum_free)
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
                guard_connection(writer.db,paths,minimum_free)
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
                        primary_full_hash_executed=not reuse_primary_verification,
                        primary_verification=verification,
                        reclaimed_allocated_bytes=before-replica.stat().st_blocks*512)
            event(plan,'completed',result);return result
