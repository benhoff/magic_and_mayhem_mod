#!/usr/bin/env python3
"""Independent packed NOD decoding, byte roundtrip and native inspector comparison."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile


def sha(data):
    return hashlib.sha256(data).hexdigest()


def reference(data):
    magic,size,version,count=struct.unpack_from('<4I',data)
    assert magic==0x00444f4e and size==len(data) and version==1
    assert size==24+count*494
    nodes=[]
    for i in range(count):
        at=16+i*494
        state,opaque,x,y,z=struct.unpack_from('<2I3i',data,at)
        connections=[]
        for j in range(18):
            value,target,metadata=struct.unpack_from('<Ii17s',data,at+20+j*25)
            connections.append(dict(value=value,target=target,metadata_hex=metadata.hex()))
        nodes.append(dict(state=state,word4=opaque,position=[x,y,z],connections=connections,
                          tail_words=list(struct.unpack_from('<6I',data,at+470))))
    return dict(version=version,source_bytes=size,node_count=count,nodes=nodes,
                trailer_words=list(struct.unpack_from('<2I',data,size-8)))


def encode(asset):
    result=struct.pack('<4I',0x00444f4e,asset['source_bytes'],asset['version'],asset['node_count'])
    for node in asset['nodes']:
        result+=struct.pack('<2I3i',node['state'],node['word4'],*node['position'])
        for connection in node['connections']:
            result+=struct.pack('<Ii17s',connection['value'],connection['target'],bytes.fromhex(connection['metadata_hex']))
        result+=struct.pack('<6I',*node['tail_words'])
    return result+struct.pack('<2I',*asset['trailer_words'])


def fixture(count):
    body=bytearray()
    for i in range(count):
        body+=struct.pack('<2I3i',i%2,0xffffffff,-2147483648,2147483647,-i)
        for j in range(18):
            body+=struct.pack('<Ii17s',j*88,-j if j%2 else 0x7fffffff,bytes((i+j+k)%256 for k in range(17)))
        body+=struct.pack('<6I',i,0xffffffff,1,2,3,0x87654321)
    return struct.pack('<4I',0x00444f4e,24+count*494,1,count)+body+struct.pack('<2I',0x12345678,999)


def compare(inspector,root,path):
    data=(root/path).read_bytes()
    actual=json.loads(subprocess.check_output([inspector,str(root),path.lower()],text=True))
    assert actual==reference(data),path
    assert encode(actual)==data,path
    canonical=json.dumps(actual,sort_keys=True,separators=(',',':')).encode()
    active=sum(c['value']!=0 for n in actual['nodes'] for c in n['connections'])
    return dict(path=path,source_bytes=len(data),source_sha256=sha(data),node_count=actual['node_count'],
                nonzero_connections=active,decoded_sha256=sha(canonical),trailer_words=actual['trailer_words'])


def run(args):
    with tempfile.TemporaryDirectory() as temp:
        root=Path(temp)
        for count in (0,1,2,18):
            (root/'Odd.NOD').write_bytes(fixture(count))
            compare(args.inspector,root,'Odd.NOD')
        for data in (b'bad',fixture(2)[:-1],fixture(2)+b'\0'):
            (root/'bad.nod').write_bytes(data)
            assert subprocess.run([args.inspector,str(root),'bad.nod'],capture_output=True).returncode==2
        for path in ('../Odd.NOD','missing.nod'):
            assert subprocess.run([args.inspector,str(root),path],capture_output=True).returncode==2
    records=[]
    if args.installation:
        root=args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower()=='.nod':
                records.append(compare(args.inspector,root,path.relative_to(root).as_posix()))
        assert records,'No installed NOD inputs'
        for record in records:
            assert sha((root/record['path']).read_bytes())==record['source_sha256']
    canonical=json.dumps(records,sort_keys=True,separators=(',',':')).encode()
    report=dict(files=len(records),source_bytes=sum(r['source_bytes'] for r in records),
                nodes=sum(r['node_count'] for r in records),connections=sum(r['node_count']*18 for r in records),
                nonzero_connections=sum(r['nonzero_connections'] for r in records),
                comparison_sha256=sha(canonical),records=records)
    if args.report:
        args.report.parent.mkdir(parents=True,exist_ok=True)
        args.report.write_text(json.dumps(report,indent=2)+'\n')
    print(f"NOD comparisons passed: 4 synthetic fixtures, {report['files']} installed files, {report['nodes']} nodes, {report['connections']} connection slots")


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('inspector')
    parser.add_argument('--installation',type=Path)
    parser.add_argument('--report',type=Path)
    args=parser.parse_args()
    manifest=Path(__file__).resolve().parents[1]/'tools/original-manifest.sh'
    if args.installation:
        subprocess.run([str(manifest),'verify'],check=True)
    try:
        run(args)
    finally:
        if args.installation:
            subprocess.run([str(manifest),'verify'],check=True)


if __name__=='__main__':
    main()
