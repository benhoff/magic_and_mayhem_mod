"""Bounded resident archive reader; source chunks never count as Worlds."""
import struct
import zlib
import ctypes as c
import hashlib


class PrefixDecoder:
    """One bounded previous source body; no captured destination input."""
    def __init__(self):
        self.api=c.CDLL('libzstd.so.1');self.previous=b'';self.count=0;self.context=None
        declarations={
            'ZSTD_createDCtx':([],c.c_void_p),'ZSTD_freeDCtx':([c.c_void_p],c.c_size_t),
            'ZSTD_DCtx_setParameter':([c.c_void_p,c.c_int,c.c_int],c.c_size_t),
            'ZSTD_DCtx_refPrefix':([c.c_void_p,c.c_void_p,c.c_size_t],c.c_size_t),
            'ZSTD_decompressDCtx':([c.c_void_p,c.c_void_p,c.c_size_t,c.c_void_p,c.c_size_t],c.c_size_t),
            'ZSTD_findFrameCompressedSize':([c.c_void_p,c.c_size_t],c.c_size_t),
            'ZSTD_getFrameContentSize':([c.c_void_p,c.c_size_t],c.c_ulonglong),
            'ZSTD_isError':([c.c_size_t],c.c_uint),'ZSTD_versionNumber':([],c.c_uint)}
        for name,(args,result) in declarations.items():
            fn=getattr(self.api,name);fn.argtypes=args;fn.restype=result
        if self.api.ZSTD_versionNumber()<10400:raise ValueError('Resident prefix codec version')
        self.context=self.api.ZSTD_createDCtx()
        if not self.context:raise ValueError('Resident prefix decoder allocation')
        try:self.valid(self.api.ZSTD_DCtx_setParameter(self.context,100,26))
        except BaseException:self.close();raise

    def valid(self,result):
        if self.api.ZSTD_isError(result):raise ValueError('Resident prefix codec failure')
        return result

    def close(self):
        if self.context:self.api.ZSTD_freeDCtx(self.context);self.context=None

    def decode(self,header,packed,expected):
        flags=struct.unpack_from('<I',header,28)[0];ordinal,base=struct.unpack_from('<QQ',header,32)
        key=self.count%128==0
        if ordinal!=self.count+1 or flags!=int(not key) or base!=(0 if key else self.count):
            raise ValueError('Resident prefix identity')
        if not 16<=expected<=16777216+197184 or not 0<len(packed)<=expected+expected//128+1024:
            raise ValueError('Resident prefix body extent')
        source=c.c_char_p(packed)
        if (self.valid(self.api.ZSTD_findFrameCompressedSize(source,len(packed)))!=len(packed)
                or self.api.ZSTD_getFrameContentSize(source,len(packed))!=expected):
            raise ValueError('Resident prefix compressed frame')
        prior=c.c_char_p(self.previous) if not key else None
        self.valid(self.api.ZSTD_DCtx_refPrefix(self.context,prior,0 if key else len(self.previous)))
        output=c.create_string_buffer(expected)
        size=self.valid(self.api.ZSTD_decompressDCtx(self.context,output,expected,source,len(packed)))
        body=output.raw
        if size!=expected or hashlib.sha256(body).digest()!=header[48:80]:
            raise ValueError('Resident prefix decoded hash')
        self.previous=body;self.count+=1;return body

def records(path):
    limit=16*1024**3
    if path.stat().st_size>limit:raise ValueError('Resident stored extent')
    worlds=chunks=count=0;raw=8
    with path.open('rb') as f:
        magic=f.read(8)
        if magic not in (b'MNMPROF1',b'MNMPROF2',b'MNMPROF3',b'MNMPROF4'):raise ValueError('Resident archive version')
        width=80 if magic==b'MNMPROF4' else 32 if magic==b'MNMPROF3' else 20 if magic==b'MNMPROF2' else 16
        prefix=PrefixDecoder() if magic==b'MNMPROF4' else None
        try:
          while True:
            h=f.read(width)
            if not h:break
            if len(h)!=width:raise ValueError('Resident truncated header')
            if magic in (b'MNMPROF3',b'MNMPROF4'):
                g,c,s,q,n,reserved=struct.unpack('<QQIIII',h[:32])
                if magic==b'MNMPROF3' and reserved:raise ValueError('Resident reserved word')
            else:
                g,s,q=struct.unpack('<QII',h[:16]);c=0;n=struct.unpack('<I',h[16:])[0] if width==20 else 0
            if g!=worlds+1 or g>8192 or count>=8192 or not 16<=s<=16777216 or not (q==0 if c else 64<=q<=197184):raise ValueError('Resident identity/extent')
            if c:
                if c!=chunks+1:raise ValueError('Resident source order')
                chunks+=1
            else:worlds+=1
            count+=1;total=s+q;raw+=width+total
            if raw>limit:raise ValueError('Resident raw extent')
            if prefix:
                if not 4<n<=total+total//128+1024:raise ValueError('Resident prefix compressed extent')
                packed=f.read(n)
                if len(packed)!=n:raise ValueError('Resident truncated prefix frame')
                data=prefix.decode(h,packed,total)
            elif magic!=b'MNMPROF1':
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
        finally:
            if prefix:prefix.close()
    if path.stat().st_size>limit:raise ValueError('Resident stored extent')
