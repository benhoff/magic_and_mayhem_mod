#!/usr/bin/env python3
"""Independent Python V4 decode against the C++ fixture's exact source digests."""
import hashlib,json,struct,sys,tempfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from resident_capture import records


def main():
    root=Path(sys.argv[1]);expected=json.loads((root/'expected-index.json').read_text());seen=decoded=0
    path=root/'sources-v4.bin'
    for g,c,source,request in records(path):
        row=expected[seen];seen+=1;decoded+=len(source)+len(request)
        assert (g,c,len(source),len(request))==(row['generation'],row['chunk'],row['source'],row['request'])
        assert hashlib.sha256(source+request).hexdigest()==row['sha256']
    assert seen==len(expected) and seen>128
    raw=path.read_bytes();offsets=[];at=8
    while at<len(raw):offsets.append(at);at+=80+struct.unpack_from('<I',raw,at+24)[0]
    mutations=[]
    for offset,field,value,fmt in [(offsets[0],28,1,'I'),(offsets[1],40,0,'Q'),(offsets[1],32,99,'Q'),
                                   (offsets[128],28,1,'I'),(offsets[0],16,2**32-1,'I'),(offsets[0],0,2,'Q')]:
        body=bytearray(raw);struct.pack_into('<'+fmt,body,offset+field,value);mutations.append(body)
    body=bytearray(raw);body[offsets[0]+48]^=1;mutations.append(body)
    body=bytearray(raw);body[offsets[0]+80+20]^=1;mutations.append(body)
    mutations.extend((raw[:-1],raw+b'x'))
    with tempfile.TemporaryDirectory(dir=root,prefix='mutations-') as directory:
        for i,body in enumerate(mutations):
            file=Path(directory)/str(i);file.write_bytes(body)
            try:
                for _ in records(file):pass
            except (ValueError,OverflowError):pass
            else:raise AssertionError('Malformed prefix capture admitted '+str(i))
    print(json.dumps(dict(success=True,exact_cpp_python_source_bytes=True,records=seen,decoded_bytes=decoded,
                          stored_bytes=path.stat().st_size,malformed_refused=len(mutations))))


if __name__=='__main__':main()
