"""Exact, chunk-deduplicated SQLite snapshots with explicit verified restoration."""
from pathlib import Path
from contextlib import closing
import hashlib,json,os,re,shutil,sqlite3,stat,zlib

def fsync_directory(path):
    fd=os.open(path,os.O_RDONLY|os.O_DIRECTORY)
    try:os.fsync(fd)
    finally:os.close(fd)
CHUNK=64*1024
def stamp(path):
    s=path.lstat()
    if not stat.S_ISREG(s.st_mode) or s.st_nlink!=1 or path.is_symlink():raise ValueError('Snapshot must be a private regular file')
    return [s.st_dev,s.st_ino,s.st_size,s.st_mtime_ns,s.st_ctime_ns,s.st_blocks*512,stat.S_IMODE(s.st_mode)]
def filehash(path):
    h=hashlib.sha256()
    with Path(path).open('rb') as f:
        for b in iter(lambda:f.read(1024**2),b''):h.update(b)
    return h.hexdigest()
class Sink:
    def write(self,data):pass
class Store:
    def __init__(self,path,minimum_free=0,readonly=False):
        self.path=Path(path);self.minimum_free=minimum_free
        if "original" in self.path.resolve().parts or ".git" in self.path.resolve().parts:raise ValueError("Unsafe snapshot archive output")
        if self.path.is_symlink():raise ValueError('Snapshot store is a symlink')
        if readonly:self.db=sqlite3.connect(self.path.resolve().as_uri()+'?mode=ro',uri=True)
        else:
            self.path.parent.mkdir(parents=True,exist_ok=True)
            pin=self.path.parent/".retain"
            if not pin.exists():
                try:
                    with pin.open("x") as out:
                        out.write("Exact immutable SQLite snapshots and restore receipts. Preserve this store.\n");out.flush();os.fsync(out.fileno())
                except FileExistsError:
                    if not pin.is_file() or pin.is_symlink():raise ValueError("Unsafe snapshot retention pin")
            self.db=sqlite3.connect(self.path,timeout=60)
            self.db.executescript('''
              pragma foreign_keys=ON;
              pragma synchronous=FULL;
              create table if not exists chunks(sha256 text primary key,size integer not null,payload blob not null);
              create table if not exists snapshots(sha256 text primary key,size integer not null,manifest_sha256 text not null);
              create table if not exists parts(snapshot text references snapshots(sha256),ordinal integer,chunk text references chunks(sha256),primary key(snapshot,ordinal));
              create table if not exists receipts(id integer primary key,path text,sha256 text,state text,metadata blob);
            ''')
            for table in ('chunks','snapshots','parts','receipts'):
                for operation in ('update','delete'):
                    self.db.execute('create trigger if not exists '+table+'_no_'+operation+' before '+operation+' on '+table+" begin select raise(abort,'Immutable snapshot archive'); end")
            self.db.commit()
            fsync_directory(self.path.parent)
    def close(self):self.db.close()
    def has(self,sha):return self.db.execute('select 1 from chunks where sha256=?',(sha,)).fetchone() is not None
    def import_stage(self,stage,sha,size,parts):
        need=0
        for chunk in dict.fromkeys(parts):
            if not self.has(chunk):
                row=stage.execute('select length(payload) from chunks where sha256=?',(chunk,)).fetchone()
                need+=row[0]+12288
        need+=len(parts)*256+1024**2
        if shutil.disk_usage(self.path.parent).free-need<self.minimum_free:raise ValueError('Snapshot archive would breach reserve')
        manifest=hashlib.sha256(('\n'.join(parts)+'\n').encode() if parts else b'').hexdigest()
        old=self.db.execute('select size,manifest_sha256 from snapshots where sha256=?',(sha,)).fetchone()
        if old is not None and old!=(size,manifest):raise ValueError('Snapshot identity collision')
        with self.db:
            for i,chunk in enumerate(dict.fromkeys(parts)):
                if self.has(chunk):continue
                if i%128==0 and shutil.disk_usage(self.path.parent).free<self.minimum_free:raise ValueError('Snapshot archive reserve crossed')
                row=stage.execute('select sha256,size,payload from chunks where sha256=?',(chunk,)).fetchone()
                self.db.execute('insert into chunks values(?,?,?)',row)
            self.db.execute('insert or ignore into snapshots values(?,?,?)',(sha,size,manifest))
            self.db.executemany('insert or ignore into parts values(?,?,?)',((sha,i,h) for i,h in enumerate(parts)))
        with self.path.open('rb') as f:os.fsync(f.fileno())
        return manifest
    def copy(self,sha,out):
        row=self.db.execute('select size,manifest_sha256 from snapshots where sha256=?',(sha,)).fetchone()
        if row is None:raise ValueError('Unknown snapshot')
        h=hashlib.sha256();total=count=0;manifest=hashlib.sha256()
        for ordinal,chunk,size,packed in self.db.execute('select p.ordinal,c.sha256,c.size,c.payload from parts p join chunks c on c.sha256=p.chunk where p.snapshot=? order by p.ordinal',(sha,)):
            if ordinal!=count or not 0<size<=CHUNK:raise ValueError('Invalid snapshot part order/size')
            decoder=zlib.decompressobj();data=decoder.decompress(packed,CHUNK+1)
            if len(data)!=size or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail or hashlib.sha256(data).hexdigest()!=chunk:raise ValueError('Snapshot chunk corrupted')
            total+=size
            if total>row[0]:raise ValueError('Snapshot length overflow')
            h.update(data);manifest.update((chunk+'\n').encode());count+=1;out.write(data)
        if total!=row[0] or h.hexdigest()!=sha or manifest.hexdigest()!=row[1]:raise ValueError('Complete snapshot identity mismatch')
        return total
    def receipt(self,path,sha,state,metadata):
        body=json.dumps(metadata,sort_keys=True).encode()
        with self.db:self.db.execute('insert into receipts(path,sha256,state,metadata) values(?,?,?,?)',(str(path),sha,state,body))
        with self.path.open('rb') as f:os.fsync(f.fileno())
        fsync_directory(self.path.parent)
        row=self.db.execute('select path,sha256,state,metadata from receipts order by id desc limit 1').fetchone()
        if row!=(str(path),sha,state,body):raise ValueError('Receipt readback mismatch')
    def restore(self,sha,target):
        target=Path(target)
        if 'original' in target.parts or target.exists() or target.is_symlink():raise ValueError('Restore requires a new non-original output')
        row=self.db.execute('select size from snapshots where sha256=?',(sha,)).fetchone()
        if row is None:raise ValueError('Unknown snapshot')
        target.parent.mkdir(parents=True,exist_ok=True)
        if shutil.disk_usage(target.parent).free<row[0]+1024**2:raise ValueError('Insufficient restore capacity')
        for p in (target.parent,*target.parent.parents):
            if p.is_symlink():raise ValueError('Restore traverses symlink')
        with target.open('xb') as out:self.copy(sha,out);out.flush();os.fsync(out.fileno())
        with closing(sqlite3.connect(target.resolve().as_uri()+'?mode=ro',uri=True)) as db:
            if db.execute('pragma integrity_check').fetchone()!=('ok',) or db.execute('pragma foreign_key_check').fetchall():raise ValueError('Restored SQLite integrity failure')
        fsync_directory(target.parent)
        return target

def archive(source,local,cold,scratch,local_floor,cold_floor,remove=False,expected_sha=None):
    source=Path(source).absolute();scratch=Path(scratch);scratch.mkdir(parents=True,exist_ok=True)
    if 'original' in source.parts:raise ValueError('Original inputs excluded')
    from compact_snapshot_activity import busy
    if busy(source):raise ValueError('Snapshot has an active consumer')
    before=stamp(source)
    if expected_sha is not None and not re.fullmatch('[0-9a-f]{64}',expected_sha):raise ValueError('Invalid expected snapshot SHA')
    if any(source.with_name(source.name+s).exists() for s in ('-wal','-shm','-journal')):raise ValueError('Live or unfinished SQLite database')
    if source.stat().st_uid!=os.geteuid():raise ValueError('Snapshot belongs to another user')
    with closing(sqlite3.connect(source.resolve().as_uri()+'?mode=ro&immutable=1',uri=True)) as db:
        if db.execute('pragma integrity_check').fetchone()!=('ok',):raise ValueError('Source SQLite integrity failure')
    stage_path=scratch/'snapshot-stage.sqlite3'
    if stage_path.exists():raise ValueError('Prior staging attempt exists; inspect it')
    stage=sqlite3.connect(stage_path)
    stage.execute('create table chunks(sha256 text primary key,size integer,payload blob)')
    whole=hashlib.sha256();size=0;parts=[]
    try:
        with source.open('rb') as f,stage:
            for data in iter(lambda:f.read(CHUNK),b''):
                whole.update(data);size+=len(data);sha=hashlib.sha256(data).hexdigest();parts.append(sha)
                stage.execute('insert or ignore into chunks values(?,?,?)',(sha,len(data),zlib.compress(data,1)))
        sha=whole.hexdigest()
        if expected_sha is not None and sha!=expected_sha:raise ValueError('Snapshot filename/catalog SHA does not match bytes')
        if stamp(source)!=before:raise ValueError('Snapshot changed during ingestion')
        metadata=dict(source_identity=before,source_sha256=sha,source_bytes=size,chunk_bytes=CHUNK,local_catalog=str(local),cold_catalog=str(cold),restore_command='python3 tools/sqlite-snapshot-store.py restore --catalog '+str(cold)+' --sha256 '+sha+' --output <new-output.sqlite3>')
        stores=[]
        try:
            stores.append(Store(local,local_floor));stores.append(Store(cold,cold_floor))
            manifests=[s.import_stage(stage,sha,size,parts) for s in stores]
            if manifests[0]!=manifests[1]:raise ValueError('Dual snapshot manifests differ')
            stores[0].copy(sha,Sink())
            restored=scratch/('restore-'+sha+'.sqlite3')
            stores[1].restore(sha,restored)
            if filehash(restored)!=sha:raise ValueError('Real snapshot restore differs')
            restored.unlink()
            for s in stores:s.receipt(source,sha,'verified',metadata)
            if remove:
                if busy(source) or stamp(source)!=before or filehash(source)!=sha:raise ValueError('Snapshot acquired a consumer or changed before removal')
                source.unlink()
                fd=os.open(source.parent,os.O_RDONLY|os.O_DIRECTORY);os.fsync(fd);os.close(fd)
                for s in stores:s.receipt(source,sha,'archived',metadata)
            return dict(success=True,sha256=sha,source=str(source),bytes=size,reclaimed_allocated_bytes=before[5] if remove else 0,local_catalog=str(local),cold_catalog=str(cold),exact_restore_verified=True)
        finally:
            for s in stores:s.close()
    finally:
        stage.close()
        # Staging only duplicates the still-retained source or the verified archives.
        stage_path.unlink()
