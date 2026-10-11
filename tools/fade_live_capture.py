"""Strict bounded fade physical-plane diagnostics; no pointers or native seed oracle."""
import struct
from pathlib import Path
MAX_BYTES=256*1024*1024
def decode(data,partial=False):
    if len(data)>MAX_BYTES or len(data)<8 or data[:8]!=b'MNMFDLV1':raise ValueError('Fade capture magic/budget')
    rows=[];at=8;previous=(0,0)
    while at<len(data):
        if len(data)-at<32:
            if partial:return rows
            raise ValueError('Truncated fade header')
        header=struct.unpack_from('<8I',data,at)
        index,w,h,stride,format,words,touched,reserved=header
        if index!=len(rows)+1 or index>128 or not 0<w<=2048 or not 0<h<=2048 or not w<=stride<=4096 or words!=stride*h or touched!=(w//2)*2*h or reserved:raise ValueError('Fade record extent/sequence')
        size=32+words*4+16
        if size>len(data)-at:
            if partial:return rows
            raise ValueError('Truncated fade planes/footer')
        start=at+32;before=data[start:start+words*2];after=data[start+words*2:start+words*4]
        native,forwarded,refusals,magic=struct.unpack_from('<4I',data,start+words*4)
        if magic!=0x31444e45 or native!=index or forwarded!=refusals or forwarded<previous[0] or refusals<previous[1]:raise ValueError('Fade completion counters')
        rows.append(dict(index=index,width=w,height=h,stride=stride,format=format,words=words,touched=touched,before=before,after=after,native=native,forwarded=forwarded,refusals=refusals))
        previous=(forwarded,refusals);at+=size
    return rows
def load(path,partial=False):return decode(Path(path).read_bytes(),partial)
