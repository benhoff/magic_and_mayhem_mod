"""Lossless archived catalog history plus a backwards-compatible append journal."""
from pathlib import Path
import gzip,hashlib,json,sqlite3,os,zlib
from contextlib import closing

CHUNK=4*1024*1024
def digest(path):
    h=hashlib.sha256()
    with Path(path).open('rb') as source:
        for data in iter(lambda:source.read(CHUNK),b''):h.update(data)
    return h.hexdigest()
def verify(blob,sha,size,packed_sha):
    blob=Path(blob)
    if blob.is_symlink() or digest(blob)!=packed_sha:raise ValueError('Catalog compressed bytes changed')
    h=hashlib.sha256();total=0
    with gzip.open(blob,'rb') as source:
        for data in iter(lambda:source.read(CHUNK),b''):
            total+=len(data)
            if total>size:raise ValueError('Catalog history exceeds recorded size')
            h.update(data)
    if total!=size or h.hexdigest()!=sha:raise ValueError('Catalog original bytes changed')
def histories(catalog):
    catalog=Path(catalog)
    index=catalog.parent/(catalog.name+'.history.sqlite3')
    if not index.exists():return []
    if index.is_symlink():raise ValueError('Catalog history index is a symlink')
    with closing(sqlite3.connect(index.resolve().as_uri()+'?mode=ro',uri=True)) as db:
        rows=db.execute('select sha256,size,blob,packed_sha256 from histories order by ordinal').fetchall()
    for sha,size,name,packed_sha in rows:
        if Path(name).name!=name or not name.startswith('catalog-history-'):raise ValueError('Unsafe history filename')
        verify(catalog.parent/name,sha,size,packed_sha)
    return rows
def records(catalog):
    catalog=Path(catalog);connection=None
    def decode(line):
        nonlocal connection
        item=json.loads(line)
        if 'sqlite_record' not in item:return item
        sha=item['sqlite_record'];size=item['bytes']
        if not isinstance(size,int) or not 0<=size<=64*1024**2:raise ValueError('Invalid catalog record size')
        if connection is None:
            path=catalog.parent/(catalog.name+'.records.sqlite3')
            if path.is_symlink():raise ValueError('Catalog record database is a symlink')
            connection=sqlite3.connect(path.resolve().as_uri()+'?mode=ro',uri=True)
        row=connection.execute('select size,payload from records where sha256=?',(sha,)).fetchone()
        if row is None or row[0]!=size:raise ValueError('Missing SQLite catalog record')
        decoder=zlib.decompressobj();body=decoder.decompress(row[1],size+1)
        if len(body)!=size or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail or hashlib.sha256(body).hexdigest()!=sha:raise ValueError('Catalog record identity mismatch')
        return json.loads(body)
    try:
        for sha,size,name,packed_sha in histories(catalog):
            with gzip.open(catalog.parent/name,'rb') as source:
                for line in source:
                    if len(line)>64*1024**2:raise ValueError('Oversized catalog record')
                    yield decode(line)
        if catalog.is_symlink():raise ValueError('Catalog journal is a symlink')
        with catalog.open('rb') as source:
            for line in source:
                if len(line)>64*1024**2:raise ValueError('Oversized catalog record')
                yield decode(line)
    finally:
        if connection is not None:connection.close()
def install_index(catalog,sha,size,blob,packed_sha):
    catalog=Path(catalog);index=catalog.parent/(catalog.name+'.history.sqlite3')
    with closing(sqlite3.connect(index)) as db, db:
        db.execute('pragma synchronous=FULL')
        db.execute('create table if not exists histories(ordinal integer primary key,sha256 text unique,size integer,blob text,packed_sha256 text)')
        old=db.execute('select size,blob,packed_sha256 from histories where sha256=?',(sha,)).fetchone()
        if old is not None and old!=(size,blob,packed_sha):raise ValueError('History identity conflict')
        db.execute('insert or ignore into histories(sha256,size,blob,packed_sha256) values(?,?,?,?)',(sha,size,blob,packed_sha))
    fd=os.open(index,os.O_RDONLY);os.fsync(fd);os.close(fd)
    fd=os.open(index.parent,os.O_RDONLY|os.O_DIRECTORY);os.fsync(fd);os.close(fd)


def append(catalog,record):
    catalog=Path(catalog);catalog.parent.mkdir(parents=True,exist_ok=True)
    database=catalog.parent/(catalog.name+'.records.sqlite3')
    if database.is_symlink() or catalog.is_symlink():raise ValueError('Catalog paths must be regular')
    body=(json.dumps(record,sort_keys=True)+'\n').encode()
    sha=hashlib.sha256(body).hexdigest()
    with closing(sqlite3.connect(database,timeout=60)) as db,db:
        db.execute('pragma synchronous=FULL')
        db.execute('create table if not exists records(sha256 text primary key,size integer,payload blob)')
        db.execute("create trigger if not exists records_no_update before update on records begin select raise(abort,'Immutable catalog records'); end")
        db.execute("create trigger if not exists records_no_delete before delete on records begin select raise(abort,'Immutable catalog records'); end")
        db.execute('insert or ignore into records values(?,?,?)',(sha,len(body),zlib.compress(body,6)))
        saved=db.execute('select size,payload from records where sha256=?',(sha,)).fetchone()
        if saved is None or saved[0]!=len(body):
            raise ValueError('Catalog record readback mismatch')
        decoder=zlib.decompressobj();decoded=decoder.decompress(saved[1],len(body)+1)
        if decoded!=body or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
            raise ValueError('Catalog record readback mismatch')
    with database.open('rb') as source:os.fsync(source.fileno())
    pointer=dict(sqlite_record=sha,bytes=len(body),path=record.get('path',''),state='sqlite_reference')
    with catalog.open('ab') as output:
        output.write((json.dumps(pointer,sort_keys=True)+'\n').encode());output.flush();os.fsync(output.fileno())
    fd=os.open(catalog.parent,os.O_RDONLY|os.O_DIRECTORY);os.fsync(fd);os.close(fd)
