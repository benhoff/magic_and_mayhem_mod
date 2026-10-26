"""Merge immutable evidence payloads; keep source-specific catalogs restorable."""
from contextlib import closing
from pathlib import Path
import hashlib,json,os,shutil,sqlite3,zlib
from coverage_evidence import Store
from sqlite_snapshot_store import filehash,stamp,fsync_directory
from compact_snapshot_activity import busy

class Sink:
    def write(self,data):pass

def description(store):
    names={r[0] for r in store.db.execute("select name from sqlite_master where type='table'")}
    if names-{"objects","chunks","versions","metadata","json_sweeps","json_documents"}:raise ValueError("Unknown evidence tables require preservation review")
    auxiliary=[]
    for name in ("json_sweeps","json_documents"):
        if name not in names:continue
        sql=store.db.execute("select sql from sqlite_master where type='table' and name=?",(name,)).fetchone()[0]
        keys=[r[1] for r in sorted(store.db.execute('pragma table_info("'+name+'")'),key=lambda r:r[5]) if r[5]]
        rows=list(store.db.execute('select * from "'+name+'" order by '+','.join('"'+k+'"' for k in keys)))
        auxiliary.append(dict(name=name,sql=sql,rows=rows,keys=keys))
    return dict(auxiliary=auxiliary,objects=list(store.db.execute("select sha256,size,git_blob from objects order by sha256")),
                versions=list(store.db.execute("select git_blob,path,sha256 from versions order by git_blob,path")),
                metadata=list(store.db.execute("select key,value from metadata order by key")))

def merge(source,target,floor):
    """Append payloads and bindings, checking all shared original bytes."""
    dest=Store(target)
    try:
        dest.db.execute("pragma synchronous=FULL")
        need=1024**2
        for sha,size,blob in source.db.execute("select sha256,size,git_blob from objects"):
            if not dest.has(sha,size):
                if dest.has(sha):raise ValueError("Evidence object size conflict")
                need+=source.db.execute("select coalesce(sum(length(payload)+8192),0) from chunks where sha256=?",(sha,)).fetchone()[0]
        if shutil.disk_usage(Path(target).parent).free-need<floor:raise ValueError("Evidence union would breach reserve")
        auxiliary=description(source)["auxiliary"]
        with dest.db:
            for table in auxiliary:dest.db.execute(table["sql"].replace("CREATE TABLE ","CREATE TABLE IF NOT EXISTS ",1))
            for table in auxiliary:
                name=table["name"]
                source_columns=list(source.db.execute('pragma table_info("'+name+'")'))
                if list(dest.db.execute('pragma table_info("'+name+'")'))!=source_columns:raise ValueError("Auxiliary evidence schema conflict")
                columns=[r[1] for r in source_columns]
                positions=[columns.index(key) for key in table["keys"]]
                for row in table["rows"]:
                    previous=dest.db.execute('select * from "'+name+'" where '+' and '.join('"'+key+'" IS ?' for key in table["keys"]),[row[i] for i in positions]).fetchone()
                    if previous is not None and previous!=row:raise ValueError("Auxiliary evidence row conflict")
                    dest.db.execute('insert or ignore into "'+name+'" values('+','.join('?' for _ in row)+')',row)
            for sha,size,blob in source.db.execute("select sha256,size,git_blob from objects"):
                if not dest.has(sha,size):
                    if shutil.disk_usage(Path(target).parent).free<floor:raise ValueError("Evidence union reserve crossed")
                    dest.db.execute("insert into objects values(?,?,?)",(sha,size,blob))
                    dest.db.executemany("insert into chunks values(?,?,?)",source.db.execute("select sha256,ordinal,payload from chunks where sha256=? order by ordinal",(sha,)))
                dest.copy(sha,size,Sink())
            for blob,path,sha in source.db.execute("select git_blob,path,sha256 from versions"):
                old=dest.db.execute("select sha256 from versions where git_blob=? and path=?",(blob,path)).fetchone()
                if old is not None and old!=(sha,):raise ValueError("Historical evidence binding conflict")
                dest.db.execute("insert or ignore into versions values(?,?,?)",(blob,path,sha))
        with Path(target).open("rb") as out:os.fsync(out.fileno())
        fsync_directory(Path(target).parent)
    finally:dest.close()

def receipt(catalog,key,value):
    body=json.dumps(value,sort_keys=True,separators=(",",":")).encode();sha=hashlib.sha256(body).hexdigest()
    catalog=Path(catalog);catalog.parent.mkdir(parents=True,exist_ok=True)
    with closing(sqlite3.connect(catalog)) as db,db:
        db.execute("pragma synchronous=FULL")
        db.execute("create table if not exists receipts(id integer primary key,source text,state text,sha256 text,bytes integer,payload blob)")
        if len(body)>64*1024**2:raise ValueError("Source catalog exceeds bounded receipt size")
        for operation in ("update","delete"):
            db.execute("create trigger if not exists receipts_no_"+operation+" before "+operation+" on receipts begin select raise(abort,'Immutable retention receipts'); end")
        db.execute("insert into receipts(source,state,sha256,bytes,payload) values(?,?,?,?,?)",(key,value["state"],sha,len(body),zlib.compress(body,6)))
        row=db.execute("select sha256,bytes,payload from receipts order by id desc limit 1").fetchone()
        if row!=(sha,len(body),zlib.compress(body,6)):raise ValueError("Evidence receipt readback mismatch")
    with catalog.open("rb") as out:os.fsync(out.fileno())
    fsync_directory(catalog.parent)

def restore(catalog,source,target,cold):
    """Restore a source's original logical tables and every original report byte."""
    with closing(sqlite3.connect(Path(catalog).resolve().as_uri()+"?mode=ro",uri=True)) as db:
        row=db.execute("select sha256,bytes,payload from receipts where source=? and state='verified' order by id desc limit 1",(str(source),)).fetchone()
    if row is None:raise ValueError("No verified source catalog")
    if not 0<=row[1]<=64*1024**2:raise ValueError("Invalid source receipt size")
    decoder=zlib.decompressobj();body=decoder.decompress(row[2],row[1]+1)
    if len(body)!=row[1] or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:raise ValueError("Source receipt size mismatch")
    if hashlib.sha256(body).hexdigest()!=row[0]:raise ValueError("Source receipt corrupted")
    plan=json.loads(body);target=Path(target)
    if target.exists() or target.is_symlink() or "original" in target.parts:raise ValueError("Restore requires new non-original path")
    target.parent.mkdir(parents=True,exist_ok=True)
    if shutil.disk_usage(target.parent).free<plan["identity"][2]*1.1+1024**2:
        raise ValueError("Insufficient logical evidence restore capacity")
    source_store=Store(cold,readonly=True);dest=Store(target)
    try:
        with dest.db:
            for sha,size,blob in plan["description"]["objects"]:
                source_store.copy(sha,size,Sink())
                dest.db.execute("insert into objects values(?,?,?)",(sha,size,blob))
                dest.db.executemany("insert into chunks values(?,?,?)",source_store.db.execute("select sha256,ordinal,payload from chunks where sha256=? order by ordinal",(sha,)))
            dest.db.executemany("insert into versions values(?,?,?)",plan["description"]["versions"])
            for table in plan["description"].get("auxiliary",[]):dest.db.execute(table["sql"])
            for table in plan["description"].get("auxiliary",[]):
                if table["rows"]:dest.db.executemany('insert into "'+table["name"]+'" values('+','.join('?' for _ in table["rows"][0])+')',table["rows"])
            for key,value in plan["description"]["metadata"]:
                if key=="schema":
                    if value!="1":raise ValueError("Unknown source schema")
                else:dest.db.execute("insert into metadata values(?,?)",(key,value))
        dest.verify()
        if json.loads(json.dumps(description(dest)))!=plan["description"]:raise ValueError("Restored source catalog mismatch")
        # JSON catalogs encode tuples as lists.
        with target.open("rb") as out:os.fsync(out.fileno())
        fsync_directory(target.parent)
        return target
    finally:source_store.close();dest.close()

def consolidate(source,local,cold,catalogs,scratch,local_floor=90*2**30,cold_floor=100*2**30):
    source=Path(source).absolute();local=Path(local).resolve();cold=Path(cold).resolve()
    if "original" in source.parts or source.is_symlink() or source.resolve()==local:raise ValueError("Only separate mutable evidence stores can be consolidated")
    for path in (local,cold,*catalogs,Path(scratch)):
        if any(part in ("original",".git") for part in Path(path).resolve().parts):raise ValueError("Unsafe evidence consolidation output")
    before=stamp(source)
    if busy(source):raise ValueError("Evidence source has an active consumer")
    if any(source.with_name(source.name+tail).exists() for tail in ("-wal","-shm","-journal")):raise ValueError("Live or unfinished evidence database")
    old=Store(source,readonly=True)
    try:
        count=old.verify();desc=description(old)
        # Store source catalogs as ordinary JSON-compatible arrays, not expanded files.
        desc=json.loads(json.dumps(desc))
        plan=dict(state="planned",source=str(source),identity=before,source_sha256=filehash(source),
                  description=desc,objects=count,local=str(local),cold=str(cold),
                  storage_scope="Every original report byte and source binding retained; physical mutable SQLite page layout is not an evidence identity.")
        if stamp(source)!=before:raise ValueError("Evidence source changed during review")
        for catalog in catalogs:receipt(catalog,str(source),plan)
        merge(old,cold,cold_floor);merge(old,local,local_floor)
    finally:old.close()
    plan["state"]="verified"
    for catalog in catalogs:receipt(catalog,str(source),plan)
    scratch=Path(scratch);scratch.mkdir(parents=True,exist_ok=True)
    restored=scratch/"restored-evidence.sqlite3"
    restore(catalogs[1],source,restored,cold);restored.unlink()
    if busy(source) or stamp(source)!=before or filehash(source)!=plan["source_sha256"]:raise ValueError("Evidence source changed or acquired a consumer")
    alias=source.with_name(source.name+".shared")
    if alias.exists() or alias.is_symlink():raise ValueError("Prior alias preparation needs review")
    alias.symlink_to(local);os.replace(alias,source);fsync_directory(source.parent)
    plan["state"]="consolidated";plan["reclaimed_allocated_bytes"]=before[5];plan["exact_payload_restore_verified"]=True
    for catalog in catalogs:receipt(catalog,str(source),plan)
    return {k:v for k,v in plan.items() if k!="description"}
