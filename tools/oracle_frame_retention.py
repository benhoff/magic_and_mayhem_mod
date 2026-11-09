"""Classify newly generated successful oracle planes as reproducible temporary output.

This never deletes files or weakens pixel comparison. Raw acquired observations,
results, hashes, reconstruction inputs and tested executables remain durable.
"""
import hashlib
from pathlib import Path
import stat
import struct
import zlib


def frame_manifest(reference,index):
    reference=Path(reference)
    if reference.is_symlink() or not reference.is_dir():raise ValueError('Physical oracle directory required')
    if (index.get('success') is not True or index.get('native_player_pixels_read') is not False
            or index.get('original_destination_capture_read') is not False):
        raise ValueError('Independent complete original history required')
    worlds=index.get('worlds')
    if not isinstance(worlds,list) or not 0<len(worlds)<=65536:raise ValueError('Bounded complete oracle history required')
    rows=[]
    for generation,row in enumerate(worlds,1):
        name=f'world-{generation:06d}.qz'
        if str(row.get('generation'))!=str(generation) or row.get('path')!=name:
            raise ValueError('Discontinuous or redirected oracle plane')
        w,h=row.get('width'),row.get('height')
        if type(w) is not int or type(h) is not int or w<=0 or h<=0 or w*h*2>2*1024**2:
            raise ValueError('Oracle plane extent exceeds classification bound')
        path=reference/name;before=path.lstat()
        if not stat.S_ISREG(before.st_mode) or path.is_symlink() or before.st_nlink!=1:
            raise ValueError('Physical single-owner oracle plane required')
        body=path.read_bytes();after=path.lstat()
        identity=lambda s:(s.st_dev,s.st_ino,s.st_size,s.st_mtime_ns,s.st_ctime_ns)
        if identity(before)!=identity(after):raise ValueError('Oracle plane changed during classification')
        expected=w*h*2
        if not 4<len(body)<=expected*2+4096 or struct.unpack_from('>I',body)[0]!=expected:
            raise ValueError('Oracle plane envelope differs')
        decoder=zlib.decompressobj()
        raw=decoder.decompress(body[4:],expected+1)
        if len(raw)!=expected or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
            raise ValueError('Oracle plane compression/extent differs')
        rows.append(dict(path='reference/'+name,generation=generation,width=w,height=h,
            bytes=len(body),sha256=hashlib.sha256(body).hexdigest(),raw_bytes=len(raw),
            raw_sha256=hashlib.sha256(raw).hexdigest(),classification='reproducible_temporary'))
    if {p.name for p in reference.glob('*.qz')}!={Path(r['path']).name for r in rows}:
        raise ValueError('Unindexed oracle plane requires review')
    return rows
