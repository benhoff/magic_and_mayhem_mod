"""Bounded resident archive reader; source chunks never count as Worlds."""
import struct
import zlib

def records(path):
    limit=16*1024**3
    if path.stat().st_size>limit:raise ValueError('Resident stored extent')
    worlds=chunks=count=0;raw=8
    with path.open('rb') as f:
        magic=f.read(8)
        if magic not in (b'MNMPROF1',b'MNMPROF2',b'MNMPROF3'):raise ValueError('Resident archive version')
        width=32 if magic==b'MNMPROF3' else 20 if magic==b'MNMPROF2' else 16
        while True:
            h=f.read(width)
            if not h:break
            if len(h)!=width:raise ValueError('Resident truncated header')
            if magic==b'MNMPROF3':
                g,c,s,q,n,reserved=struct.unpack('<QQIIII',h)
                if reserved:raise ValueError('Resident reserved word')
            else:
                g,s,q=struct.unpack('<QII',h[:16]);c=0;n=struct.unpack('<I',h[16:])[0] if width==20 else 0
            if g!=worlds+1 or g>8192 or count>=8192 or not 16<=s<=16777216 or not (q==0 if c else 64<=q<=197184):raise ValueError('Resident identity/extent')
            if c:
                if c!=chunks+1:raise ValueError('Resident source order')
                chunks+=1
            else:worlds+=1
            count+=1;total=s+q;raw+=width+total
            if raw>limit:raise ValueError('Resident raw extent')
            if magic!=b'MNMPROF1':
                if not 4<n<=total+total//1000+128:raise ValueError('Resident compressed extent')
                packed=f.read(n)
                if len(packed)!=n or struct.unpack('>I',packed[:4])[0]!=total:raise ValueError('Resident compressed length')
                decoder=zlib.decompressobj()
                try:data=decoder.decompress(packed[4:],total+1)
                except zlib.error as e:raise ValueError('Resident compression') from e
                if len(data)!=total or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:raise ValueError('Resident compression tail')
            else:
                data=f.read(total)
                if len(data)!=total:raise ValueError('Resident truncated payload')
            yield g,c,data[:s],data[s:]
    if path.stat().st_size>limit:raise ValueError('Resident stored extent')
