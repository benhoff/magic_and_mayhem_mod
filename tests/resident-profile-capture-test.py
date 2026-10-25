#!/usr/bin/env python3
"""Qt/Python capture byte equality and bounded compressed-file negative controls."""
import json
import copy
from pathlib import Path
import struct
import sys
import tempfile
import zlib
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from profile_native_resident_helpers import normalize_capture, completed_prefix_worlds
from resident_capture import records

def main():
    worker=Path(sys.argv[1]);normalized=worker/'python-v1.bin';result=normalize_capture(worker/'worker.bin',normalized)
    assert result['worlds']==4 and normalized.read_bytes()==(worker/'expected-v1.bin').read_bytes()
    prefix=normalize_capture(worker/'worker.bin',worker/'python-prefix.bin',completed_prefix=2)
    expected=(worker/'expected-v1.bin').read_bytes();end=8
    for _ in range(2):
        _,source,request=struct.unpack_from('<QII',expected,end);end+=16+source+request
    assert prefix['worlds']==2 and prefix['captured_worlds']==4 and (worker/'python-prefix.bin').read_bytes()==expected[:end]
    history=list(records(worker/'sources.bin'))
    assert [(g,c) for g,c,s,q in history]==[(1,1),(1,2),(1,0),(2,3),(2,0)]
    assert all(len(s)==4096 and len(q)==(0 if c else 64) for g,c,s,q in history)
    try:normalize_capture(worker/'sources.bin',worker/'flat-source.bin')
    except ValueError:pass
    else:raise AssertionError('Source history silently flattened')
    raw=(worker/'sources.bin').read_bytes()
    offsets=[];at=8
    while at<len(raw):
        offsets.append(at);at+=32+struct.unpack_from('<I',raw,at+24)[0]
    bad3=[]
    for at,offset,value,fmt in [(offsets[0],28,1,'I'),(offsets[0],0,2,'Q'),(offsets[1],8,1,'Q'),(offsets[2],8,3,'Q'),(offsets[0],20,64,'I'),(offsets[0],24,2**32-1,'I')]:
        data=bytearray(raw);struct.pack_into('<'+fmt,data,at+offset,value);bad3.append(bytes(data))
    bad3.extend([raw[:-1],raw+b'x',raw[:offsets[-1]+1]])
    for index,data in enumerate(bad3):
        path=worker/('bad-v3-'+str(index));path.write_bytes(data)
        try:list(records(path))
        except ValueError:pass
        else:raise AssertionError('Invalid V3 archive accepted '+str(index))
    assert sum(1 for _ in records(worker/'source-cap.bin'))==8192
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
        # A valid selected prefix never excuses a malformed excluded tail.
        p=root/'bad-tail.bin';p.write_bytes(v2()+struct.pack('<QIII',2,16,64,len(packed))+packed+b'x')
        try:normalize_capture(p,root/'bad-tail.out',completed_prefix=1)
        except ValueError:pass
        else:raise AssertionError('Malformed excluded tail admitted')
        for i,count in enumerate((0,8193,True,5)):
            try:normalize_capture(worker/'worker.bin',root/('prefix-'+str(i)),completed_prefix=count)
            except ValueError:pass
            else:raise AssertionError('Invalid/oversized prefix admitted')
    game=dict(sources_unchanged=True,private_wine_stopped=True,original_manifest_verified_after=True,
        resident_world=dict(completed='64',imports='64',verified_returns='64',world_readbacks=0,native_readbacks=0,rgba_readbacks=0,world_pixel_reply_bytes=0,
            stage_profile=dict(samples_dropped=0,gpu_queries_dropped=0,capture_truncated=False,
                samples=[dict(generation=str(i+1),producer_return_ms=0) for i in range(64)],
                capture=dict(accepted_worlds=65,written_worlds=64,truncated=False,error=''))))
    assert completed_prefix_worlds(game)==64
    changes=[('resident_world','completed','63'),('resident_world','imports','63'),('resident_world','verified_returns','63'),
        ('resident_world','native_readbacks',1),('resident_world','world_pixel_reply_bytes',2),
        ('resident_world.stage_profile','samples_dropped',1),('resident_world.stage_profile','gpu_queries_dropped',1),
        ('resident_world.stage_profile','capture_truncated',True),('resident_world.stage_profile.capture','error','disk'),
        ('resident_world.stage_profile.capture','written_worlds',63),('', 'sources_unchanged',False)]
    bad=[]
    for path,key,value in changes:
        item=copy.deepcopy(game);target=item
        for part in path.split('.') if path else ():target=target[part]
        target[key]=value;bad.append(item)
    item=copy.deepcopy(game);item['resident_world']['stage_profile']['samples'][4]['generation']='4';bad.append(item)
    item=copy.deepcopy(game);del item['resident_world']['stage_profile']['samples'][4]['producer_return_ms'];bad.append(item)
    for item in bad:
        try:completed_prefix_worlds(item)
        except ValueError:pass
        else:raise AssertionError('Unverified completed prefix admitted')
    print(json.dumps(dict(success=True,worlds=4,raw_bytes=result['raw_bytes'],qt_python_bytes_equal=True,negative_controls=len(cases),
        prefix_worlds=prefix['worlds'],excluded_tail_validated=True,prefix_negative_controls=5+len(bad))))

if __name__=='__main__':main()
