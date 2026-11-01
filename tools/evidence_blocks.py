"""Bounded compression preparation; SQLite access stays on the caller thread.

Object/block identities and the shared-v1 zlib encoding stay unchanged. A prepared
object spools each missing block once for all destinations. Existing blocks are
looked up before compression; a missing replica reuses verified encoded bytes.
"""
from collections import deque
from concurrent.futures import Future, ThreadPoolExecutor, wait
from contextlib import contextmanager
import hashlib
import shutil
import tempfile
from pathlib import Path
import zlib
from coverage_evidence import CHUNK


class CompressionPool:
    def __init__(self, workers=4):
        if type(workers) is not int or not 1 <= workers <= 8:
            raise ValueError('Compression workers must be an integer from 1 to 8')
        self.workers=workers
        self.limit=workers*2
        self.executor=None
        self.peak_pending=0

    def submit(self, data):
        # Small reports need no thread startup; larger blocks release the GIL
        # inside zlib. Workers never receive a database or source-file handle.
        if self.workers==1 or len(data)<64*1024:
            f=Future()
            try:f.set_result(zlib.compress(data,6))
            except BaseException as exc:f.set_exception(exc)
            return f
        if self.executor is None:
            self.executor=ThreadPoolExecutor(max_workers=self.workers,thread_name_prefix='evidence-compress')
        return self.executor.submit(zlib.compress,data,6)

    def close(self):
        if self.executor is not None:
            self.executor.shutdown(wait=True,cancel_futures=True)
            self.executor=None

    def __enter__(self):return self
    def __exit__(self,*args):self.close()


def verify_block(sha,size,payload):
    decoder=zlib.decompressobj()
    raw=decoder.decompress(payload,CHUNK+1)
    if len(raw)!=size or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail or hashlib.sha256(raw).hexdigest()!=sha:
        raise ValueError('Existing encoded evidence block differs from its identity')


class Prepared:
    def __init__(self,size):
        self.size=size
        self.sha256=None
        self.parts=[]
        self.blocks={}
        self.spool=tempfile.SpooledTemporaryFile(max_size=8*CHUNK)

    def encoded(self,sha):
        size,offset,length=self.blocks[sha]
        if offset is None:
            raise ValueError('Previously present evidence block is unavailable')
        self.spool.seek(offset)
        value=self.spool.read(length)
        if len(value)!=length:raise ValueError('Truncated prepared evidence block')
        return value

    def save(self,sha,size,value):
        self.spool.seek(0,2)
        offset=self.spool.tell()
        self.spool.write(value)
        self.blocks[sha]=(size,offset,len(value))


@contextmanager
def prepare(stores,stream,size,pool):
    if type(size) is not int or size<0:raise ValueError('Invalid evidence object length')
    p=Prepared(size);pending=deque();h=hashlib.sha256();remaining=size
    def drain():
        sha,length,future=pending.popleft()
        p.save(sha,length,future.result())
    try:
        while remaining:
            data=stream.read(min(CHUNK,remaining))
            if not data or len(data)>min(CHUNK,remaining):raise ValueError('Truncated or oversized evidence input')
            remaining-=len(data);h.update(data)
            sha=hashlib.sha256(data).hexdigest();p.parts.append(sha)
            if sha in p.blocks:continue
            rows=[s.db.execute('SELECT size FROM blocks WHERE sha256=?',(sha,)).fetchone() for s in stores]
            if any(row is not None and row[0]!=len(data) for row in rows):raise ValueError('Evidence block size collision')
            p.blocks[sha]=(len(data),None,None)
            if all(row is not None for row in rows):continue
            if any(row is not None for row in rows):
                source=next(s for s,row in zip(stores,rows) if row is not None)
                value=source.db.execute('SELECT payload FROM blocks WHERE sha256=?',(sha,)).fetchone()[0]
                verify_block(sha,len(data),value)
                p.save(sha,len(data),value)
            else:
                pending.append((sha,len(data),pool.submit(data)))
                pool.peak_pending=max(pool.peak_pending,len(pending))
                if len(pending)>=pool.limit:drain()
        while pending:drain()
        p.sha256=h.hexdigest()
        yield p
    finally:
        for _,_,f in pending:f.cancel()
        if pending:wait([f for _,_,f in pending])
        p.spool.close()


def insert(store,p,git_blob=None):
    existing=store.db.execute('SELECT size FROM objects WHERE sha256=?',(p.sha256,)).fetchone()
    if existing is not None:
        if existing!=(p.size,):raise ValueError('Evidence size collision')
        return p.sha256
    need=CHUNK+len(p.parts)*8192
    present=set()
    for sha,(size,offset,length) in p.blocks.items():
        row=store.db.execute('SELECT size FROM blocks WHERE sha256=?',(sha,)).fetchone()
        if row is not None:
            if row!=(size,):raise ValueError('Evidence block size collision')
            present.add(sha)
        elif offset is None:raise ValueError('Previously present evidence block is unavailable')
        else:need+=length
    journal=Path(str(store.path)+'-wal')
    frames=journal.stat().st_size if journal.exists() else 0
    if shutil.disk_usage(store.path.parent).free-2*need-frames<getattr(store,'minimum_free',0):
        raise ValueError('Shared evidence write would breach reserve')
    store.db.execute('INSERT INTO objects VALUES(?,?,?)',(p.sha256,p.size,git_blob))
    for ordinal,sha in enumerate(p.parts):
        if sha not in present:
            size,_,_=p.blocks[sha]
            store.db.execute('INSERT INTO blocks VALUES(?,?,?)',(sha,size,p.encoded(sha)))
            present.add(sha)
        store.db.execute('INSERT INTO object_parts VALUES(?,?,?)',(p.sha256,ordinal,sha))
    return p.sha256


@contextmanager
def prepare_small(stores,bodies,pool):
    """Overlap encoding across small files while bounding buffers and futures."""
    if len(bodies)>pool.limit or any(len(body)>CHUNK for body in bodies):
        raise ValueError('Small-file batch exceeds compression bounds')
    prepared=[];encodings={};futures=[]
    try:
        for body in bodies:
            sha=hashlib.sha256(body).hexdigest()
            p=Prepared(len(body));p.sha256=sha;prepared.append(p)
            if not body:continue
            p.parts.append(sha)
            if sha in encodings:continue
            rows=[s.db.execute('SELECT size FROM blocks WHERE sha256=?',(sha,)).fetchone() for s in stores]
            if any(row is not None and row[0]!=len(body) for row in rows):raise ValueError('Evidence block size collision')
            if all(row is not None for row in rows):encodings[sha]=None
            elif any(row is not None for row in rows):
                source=next(s for s,row in zip(stores,rows) if row is not None)
                value=source.db.execute('SELECT payload FROM blocks WHERE sha256=?',(sha,)).fetchone()[0]
                verify_block(sha,len(body),value)
                f=Future();f.set_result(value);encodings[sha]=f;futures.append(f)
            else:
                f=pool.submit(body);encodings[sha]=f;futures.append(f)
                pool.peak_pending=max(pool.peak_pending,len(futures))
        for p in prepared:
            if not p.parts:continue
            f=encodings[p.sha256]
            if f is None:p.blocks[p.sha256]=(p.size,None,None)
            else:p.save(p.sha256,p.size,f.result())
        yield prepared
    finally:
        for f in futures:f.cancel()
        if futures:wait(futures)
        for p in prepared:p.spool.close()
