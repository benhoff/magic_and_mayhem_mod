#!/usr/bin/env python3
"""Qt/Python capture byte equality and bounded compressed-file negative controls."""
import json
from pathlib import Path
import struct
import sys
import tempfile
import zlib
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from profile_native_resident_helpers import normalize_capture

def main():
    worker=Path(sys.argv[1]);normalized=worker/'python-v1.bin';result=normalize_capture(worker/'worker.bin',normalized)
    assert result['worlds']==4 and normalized.read_bytes()==(worker/'expected-v1.bin').read_bytes()
    payload=b'x'*80;packed=struct.pack('>I',80)+zlib.compress(payload,1)
    def v2(data=packed,generation=1,source=16,request=64):return b'MNMPROF2'+struct.pack('<QIII',generation,source,request,len(data))+data
    cases=[b'BADMAGIC',b'MNMPROF2'+b'x',v2(generation=2),v2(source=16777217),v2(request=63),
           v2(struct.pack('>I',2147483648)+packed[4:]),v2(packed)[:-1],v2(struct.pack('>I',80)+zlib.compress(b'x'*81)),
           v2(packed+b'extra'),v2(struct.pack('>I',80)+b'badzlib'),b'MNMPROF1'+struct.pack('<QII',1,16,64)+b'x']
    with tempfile.TemporaryDirectory(dir=worker,prefix='controls-') as temporary:
        root=Path(temporary)
        for i,data in enumerate(cases):
            p=root/(str(i)+'.bin');p.write_bytes(data)
            try:normalize_capture(p,root/(str(i)+'.out'))
            except ValueError:pass
            else:raise AssertionError('Malformed compressed capture admitted '+str(i))
    print(json.dumps(dict(success=True,worlds=4,raw_bytes=result['raw_bytes'],qt_python_bytes_equal=True,negative_controls=len(cases))))

if __name__=='__main__':main()
