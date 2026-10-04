#!/usr/bin/env python3
"""Independent DAT schemas, exact byte roundtrip and complete native comparisons."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile


def packed(words):
    return struct.pack('<'+'I'*len(words),*words)


def reference(data,mode):
    words=struct.unpack('<'+'I'*(len(data)//4),data)
    at=0
    records=[]
    while at<len(words):
        key,count=words[at:at+2];at+=2
        if mode=='experience':
            values=list(words[at:at+count]);at+=count
            parameters=list(words[at:at+2]);at+=2
            assert len(values)==count and len(parameters)==2
            records.append(dict(key=key,values=values,parameters=parameters))
        else:
            layers=[];layer_count=words[at];at+=1
            for _ in range(layer_count):
                scalar,n=words[at:at+2];at+=2
                nodes=[list(words[at+i*12:at+(i+1)*12]) for i in range(n)];at+=n*12
                tail=words[at];at+=1
                matrix=list(words[at:at+n*n]);at+=n*n
                assert all(len(node)==12 for node in nodes) and len(matrix)==n*n
                layers.append(dict(scalar_bits=scalar,tail_bits=tail,nodes=nodes,matrix=matrix))
            records.append(dict(key=key,dimension=count,layers=layers))
    assert at==len(words)
    return dict(source_bytes=len(data),**{('models' if mode=='brain' else 'samples'):records})


def encode(asset,mode):
    words=[]
    if mode=='brain':
        for m in asset['models']:
            words.extend((m['key'],m['dimension'],len(m['layers'])))
            for l in m['layers']:
                words.extend((l['scalar_bits'],len(l['nodes'])))
                for n in l['nodes']:words.extend(n)
                words.append(l['tail_bits']);words.extend(l['matrix'])
    else:
        for s in asset['samples']:words.extend([s['key'],len(s['values']),*s['values'],*s['parameters']])
    return packed(words)


def sha(data):return hashlib.sha256(data).hexdigest()


def compare(inspector,root,path,mode):
    data=(root/path).read_bytes()
    actual=json.loads(subprocess.check_output([inspector,mode,str(root),path.lower()],text=True))
    assert actual==reference(data,mode),path
    assert encode(actual,mode)==data,path
    records=actual['models' if mode=='brain' else 'samples']
    report=dict(path=path,mode=mode,source_bytes=len(data),source_sha256=sha(data),records=len(records),
                decoded_sha256=sha(json.dumps(actual,sort_keys=True,separators=(',',':')).encode()))
    if mode=='brain':
        report.update(models=[dict(key=m['key'],dimension=m['dimension'],nodes=[len(l['nodes']) for l in m['layers']]) for m in records],
                      node_records=sum(len(l['nodes']) for m in records for l in m['layers']),
                      matrix_words=sum(len(l['matrix']) for m in records for l in m['layers']))
    else:
        from collections import Counter
        report.update(value_words=sum(len(s['values']) for s in records),
                      shapes=[dict(key=k,values=n,count=c) for (k,n),c in sorted(Counter((s['key'],len(s['values'])) for s in records).items())],
                      parameters=[dict(words=list(k),count=c) for k,c in sorted(Counter(tuple(s['parameters']) for s in records).items())])
    return report


def run(args):
    with tempfile.TemporaryDirectory() as temp:
        root=Path(temp)
        fixtures={'brain':[[],[6,0,0],[6,99,1,0x7fc01234,1,*range(12),0xffffffff,0x80000000]*2,[6,4,1,7,0,9]],
                  'experience':[[],[6,0,3,0],[7,3,0xffffffff,0x80000000,0x7fc01234,3,8]*2]}
        for mode,cases in fixtures.items():
            for values in cases:
                (root/'Odd.DAT').write_bytes(packed(values));compare(args.inspector,root,'Odd.DAT',mode)
            data=packed(cases[-1])
            for broken in (data[:-1],data+b'\0',b'bad',packed([7,0xffffffff,0xffffffff])):
                (root/'bad.dat').write_bytes(broken)
                assert subprocess.run([args.inspector,mode,str(root),'bad.dat'],capture_output=True).returncode==2
            for path in ('../Odd.DAT','missing.dat'):
                assert subprocess.run([args.inspector,mode,str(root),path],capture_output=True).returncode==2
    records=[]
    if args.installation:
        root=args.installation.resolve()
        for path,mode in [('AI/Brain.dat','brain'),('AI/Experien.dat','experience')]:
            records.append(compare(args.inspector,root,path,mode))
        for r in records:assert sha((root/r['path']).read_bytes())==r['source_sha256']
    report=dict(files=len(records),source_bytes=sum(r['source_bytes'] for r in records),records=records)
    if args.report:args.report.write_text(json.dumps(report,indent=2)+'\n')
    print(f"DAT comparisons passed: 7 synthetic fixtures, {len(records)} installed files")


def main():
    p=argparse.ArgumentParser();p.add_argument('inspector');p.add_argument('--installation',type=Path);p.add_argument('--report',type=Path);args=p.parse_args()
    manifest=Path(__file__).resolve().parents[1]/'tools/original-manifest.sh'
    if args.installation:subprocess.run([str(manifest),'verify'],check=True)
    try:run(args)
    finally:
        if args.installation:subprocess.run([str(manifest),'verify'],check=True)

if __name__=='__main__':main()
