"""Exact cold source files for explicitly retired, clean Git worktrees."""
from contextlib import closing
from pathlib import Path
import hashlib,json,os,sqlite3,stat,zlib
from sqlite_snapshot_store import fsync_directory

def connect(path):
    path=Path(path)
    if "original" in path.resolve().parts or ".git" in path.resolve().parts:raise ValueError("Unsafe source archive output")
    path.parent.mkdir(parents=True,exist_ok=True)
    if path.is_symlink():raise ValueError("Source archive is a symlink")
    db=sqlite3.connect(path,timeout=60)
    db.executescript("""
      pragma synchronous=FULL;
      create table if not exists blobs(sha256 text primary key,bytes integer,payload blob);
      create table if not exists snapshots(id text primary key,root text,commit_id text,manifest blob);
      create table if not exists files(snapshot text,path text,sha256 text,bytes integer,mode integer,mtime_ns integer,git_blob text,primary key(snapshot,path));
      create table if not exists receipts(id integer primary key,snapshot text,state text,payload blob);
    """)
    for table in ("blobs","snapshots","files","receipts"):
        for op in ("update","delete"):
            db.execute("create trigger if not exists "+table+"_no_"+op+" before "+op+" on "+table+" begin select raise(abort,'Immutable source archive'); end")
    db.commit();return db

def decode(row):
    size,packed=row
    if not 0<=size<=512*1024**2:raise ValueError("Source object exceeds bound")
    decoder=zlib.decompressobj();data=decoder.decompress(packed,size+1)
    if len(data)!=size or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:raise ValueError("Invalid source object size")
    return data

def safe(name):
    p=Path(name)
    if p.is_absolute() or not p.parts or any(n in ("..",".git","original") for n in p.parts):raise ValueError("Unsafe source restore path")
    return p

def add(db,root,commit,files,minimum_free=100*1024**3):
    """files: exact index paths, blob IDs and refreshed raw identities."""
    root=Path(root);manifest=[];prepared=[]
    for name,git_blob,identity in files:
        p=root/safe(name);s=p.lstat()
        if not stat.S_ISREG(s.st_mode) or p.is_symlink():raise ValueError("Only regular source files")
        if [s.st_dev,s.st_ino,s.st_size,s.st_mtime_ns,s.st_ctime_ns]!=identity:raise ValueError("Source identity changed")
        if s.st_size>512*1024**2:raise ValueError("Source file requires separate large-input review")
        data=p.read_bytes()
        if hashlib.sha1(b"blob "+str(len(data)).encode()+b"\0"+data).hexdigest()!=git_blob:raise ValueError("Source differs from exact Git blob")
        sha=hashlib.sha256(data).hexdigest()
        row=db.execute("select bytes,payload from blobs where sha256=?",(sha,)).fetchone()
        if row:
            if hashlib.sha256(decode(row)).hexdigest()!=sha:raise ValueError("Cold source object corrupted")
        else:prepared.append((sha,len(data),zlib.compress(data,1)))
        manifest.append([name,sha,len(data),stat.S_IMODE(s.st_mode),s.st_mtime_ns,git_blob])
    body=json.dumps(dict(root=str(root),commit=commit,files=manifest),sort_keys=True,separators=(",",":")).encode()
    snapshot=hashlib.sha256(body).hexdigest()
    import shutil
    archive_path=Path(db.execute("pragma database_list").fetchone()[2])
    required=sum(len(item[2])+8192 for item in prepared)+len(manifest)*512+1024**2
    if shutil.disk_usage(archive_path.parent).free-required<minimum_free:raise ValueError("Cold source archive would breach reserve")
    with db:
        db.executemany("insert or ignore into blobs values(?,?,?)",prepared)
        db.execute("insert or ignore into snapshots values(?,?,?,?)",(snapshot,str(root),commit,zlib.compress(body,6)))
        db.executemany("insert or ignore into files values(?,?,?,?,?,?,?)",((snapshot,*row) for row in manifest))
    return snapshot,manifest

def restore(db,snapshot,target):
    target=Path(target)
    if target.exists() or target.is_symlink() or "original" in target.parts:raise ValueError("Source restore requires a new non-original directory")
    for parent in target.parents:
        if parent.is_symlink():raise ValueError("Source restore traverses symlink")
    reference=db.execute("select manifest from snapshots where id=?",(snapshot,)).fetchone()
    if reference is None:raise ValueError("Unknown source snapshot")
    decoder=zlib.decompressobj();body=decoder.decompress(reference[0],64*1024**2+1)
    if len(body)>64*1024**2 or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail or hashlib.sha256(body).hexdigest()!=snapshot:
        raise ValueError("Source manifest identity mismatch")
    manifest=json.loads(body)
    rows=list(db.execute("select path,sha256,bytes,mode,mtime_ns,git_blob from files where snapshot=? order by path",(snapshot,)))
    if not rows:raise ValueError("Unknown or empty source snapshot")
    if [list(row) for row in rows]!=sorted(manifest["files"],key=lambda row:row[0]):raise ValueError("Source path manifest mismatch")
    target.mkdir(parents=True)
    for name,sha,size,mode,mtime,git_blob in rows:
        p=target/safe(name);p.parent.mkdir(parents=True,exist_ok=True)
        row=db.execute("select bytes,payload from blobs where sha256=?",(sha,)).fetchone()
        if row is None or row[0]!=size:raise ValueError("Missing source object")
        data=decode(row)
        if hashlib.sha256(data).hexdigest()!=sha or hashlib.sha1(b"blob "+str(size).encode()+b"\0"+data).hexdigest()!=git_blob:raise ValueError("Cold source identity mismatch")
        with p.open("xb") as out:out.write(data);out.flush();os.fsync(out.fileno())
        os.chmod(p,mode);os.utime(p,ns=(mtime,mtime))
    fsync_directory(target)
    return len(rows)
