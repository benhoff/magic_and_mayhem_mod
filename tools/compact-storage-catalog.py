#!/usr/bin/env python3
"""Retain exact catalog history on both disks while freeing its expanded copies."""
from pathlib import Path
from contextlib import closing
import argparse,gzip,hashlib,json,os,shutil,sqlite3,stat,sys,time
from storage_catalog import digest,verify,install_index
from working_archive import archive_lock,sync_directory
CHUNK=4*1024**2
def identity(path):
    s=path.lstat()
    if not stat.S_ISREG(s.st_mode) or s.st_nlink!=1 or path.is_symlink():raise ValueError('Require private regular catalog')
    return [s.st_dev,s.st_ino,s.st_size,s.st_mtime_ns,s.st_ctime_ns,s.st_blocks*512,stat.S_IMODE(s.st_mode)]
def busy(path):
    expected=str(path)
    for proc in Path('/proc').iterdir():
        if not proc.name.isdecimal() or int(proc.name)==os.getpid():continue
        try:
            for fd in (proc/'fd').iterdir():
                try:value=os.readlink(fd).removesuffix(' (deleted)')
                except OSError:continue
                if value==expected:return True
            for line in (proc/'maps').read_text(errors='replace').splitlines():
                if line.split(None,5)[-1]==expected:return True
        except OSError:continue
    return False
class Receipts:
    def __init__(self,paths):self.paths=paths
    def put(self,key,value):
        body=json.dumps(value,sort_keys=True,separators=(',',':')).encode();sha=hashlib.sha256(body).hexdigest()
        for p in self.paths:
            with closing(sqlite3.connect(p)) as db,db:
                db.execute('pragma synchronous=FULL')
                db.execute('create table if not exists records(name text primary key,sha256 text,payload blob)')
                old=db.execute('select sha256,payload from records where name=?',(key,)).fetchone()
                if old is not None and old!=(sha,body):raise ValueError('Immutable migration receipt changed')
                db.execute('insert or ignore into records values(?,?,?)',(key,sha,body))
            with p.open('rb') as f:os.fsync(f.fileno())
            sync_directory(p.parent)
        for p in self.paths:
            with closing(sqlite3.connect(p.resolve().as_uri()+'?mode=ro',uri=True)) as db:
                if db.execute('select sha256,payload from records where name=?',(key,)).fetchone()!=(sha,body):raise ValueError('Dual receipt readback mismatch')
def copy_bounded(source,target,floor):
    if target.exists():raise ValueError('Preserve existing output: '+str(target))
    need=source.stat().st_size
    if shutil.disk_usage(target.parent).free-need<floor:raise ValueError('Copy would breach reserve')
    with source.open('rb') as src,target.open('xb') as out:
        while data:=src.read(CHUNK):
            if shutil.disk_usage(target.parent).free<floor:raise ValueError('Reserve crossed during catalog copy')
            out.write(data)
        out.flush();os.fsync(out.fileno())
    sync_directory(target.parent)
def compact(local,cold,scratch,receipts,main_floor=90*1024**3,cold_floor=100*1024**3):
    local,cold,scratch,receipts=map(Path,(local,cold,scratch,receipts))
    if any('original' in p.parts for p in (local,cold,scratch,receipts)):raise ValueError('Original inputs excluded')
    for p in (local,cold):
        if p.with_name(p.name+'.history.sqlite3').exists():raise ValueError('History already present; review before another rotation')
        if busy(p):raise ValueError('Active catalog reader/writer')
    scratch.mkdir(parents=True,exist_ok=True);receipts.mkdir(parents=True,exist_ok=True)
    marks=Receipts([receipts/'catalog-rotation.sqlite3',cold.parent/'catalog-rotation-20261026.sqlite3'])
    if any(p.exists() for p in marks.paths):raise ValueError('Prior migration exists; preserve it and recover explicitly')
    before={str(p):identity(p) for p in (local,cold)}
    packed=scratch/'catalog-history-prepared.gz'
    h=hashlib.sha256();total=lines=0
    with local.open('rb') as source,packed.open('xb') as output:
        with gzip.GzipFile(filename='',mode='wb',fileobj=output,compresslevel=1,mtime=0) as encoder:
            while data:=source.read(CHUNK):
                encoder.write(data);h.update(data);total+=len(data);lines+=data.count(b'\n')
        output.flush();os.fsync(output.fileno())
    sha=h.hexdigest();packed_sha=digest(packed)
    verify(packed,sha,total,packed_sha)
    if digest(cold)!=sha:raise ValueError('The two existing durable catalogs differ')
    if any(identity(p)!=before[str(p)] or busy(p) for p in (local,cold)):raise ValueError('Catalog changed during preparation')
    name='catalog-history-'+sha+'.jsonl.gz'
    plan=dict(schema=1,source_sha256=sha,source_bytes=total,source_lines=lines,packed_sha256=packed_sha,packed_bytes=packed.stat().st_size,source_identities=before,history_name=name,main_reserve_bytes=main_floor,cold_reserve_bytes=cold_floor,bootstrap='No new bulk allocation before freeing each expanded catalog. Existing independently verified catalog or compressed history on the other disk remains durable throughout.',reader='tools/storage_catalog.py',source_versions={str(Path(__file__).name):digest(Path(__file__)), 'storage_catalog.py':digest(Path(__file__).with_name('storage_catalog.py'))})
    marks.put('prepared',plan)
    for index,(journal,floor,other) in enumerate(((local,main_floor,cold),(cold,cold_floor,local)),1):
        # Exact original bytes, not an earlier source hash, gate every removal.
        if identity(journal)!=before[str(journal)] or busy(journal) or digest(journal)!=sha:raise ValueError('Fresh source verification failed')
        if index==1:
            if identity(other)!=before[str(other)] or digest(other)!=sha:raise ValueError('Cold original copy changed')
        else:verify(other.parent/name,sha,total,packed_sha)
        marks.put('before_rotation_'+str(index),dict(path=str(journal),original_sha256=sha,independent_copy=str(other if index==1 else other.parent/name)))
        journal.unlink()
        # Preserve the append-journal path for existing writers; history has a new reader.
        with journal.open('xb') as f:f.flush();os.fsync(f.fileno())
        sync_directory(journal.parent)
        destination=journal.parent/name
        copy_bounded(packed if index==1 else local.parent/name,destination,floor)
        verify(destination,sha,total,packed_sha)
        install_index(journal,sha,total,name,packed_sha)
        marks.put('rotation_verified_'+str(index),dict(path=str(journal),compressed=str(destination),original_sha256=sha,compressed_sha256=packed_sha,bytes_reclaimed=before[str(journal)][5]-destination.stat().st_blocks*512))
        print('Catalog rotation verified',index,str(journal),'free GiB',shutil.disk_usage(journal.parent).free/2**30,flush=True)
    result=dict(success=True,source_sha256=sha,source_bytes=total,source_lines=lines,packed_bytes=packed.stat().st_size,reclaimed_per_copy_bytes=total-packed.stat().st_size,local_history=str(local.parent/name),cold_history=str(cold.parent/name),all_original_catalog_bytes_retained=True,new_journals_present=True)
    marks.put('complete',result)
    return result
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--local',type=Path,required=True);p.add_argument('--cold',type=Path,required=True);p.add_argument('--scratch',type=Path,required=True);p.add_argument('--receipts',type=Path,required=True)
    a=p.parse_args()
    with archive_lock(a.cold.parent):result=compact(a.local,a.cold,a.scratch,a.receipts)
    print(json.dumps(result,indent=2))
if __name__=='__main__':main()
