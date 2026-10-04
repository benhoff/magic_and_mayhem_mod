#!/usr/bin/env python3
"""Verify native ANI bytes and bounded forward controller traces against original x86."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(b):return hashlib.sha256(b).hexdigest()
def fixture(path,records):
    b=bytearray(b'ANI\0'+struct.pack('<5I',44+8+44*len(records),len(records),5,0,2)+b'fixture.spr\0'.ljust(20,b'\0'))
    b.extend(struct.pack('<2I',0,len(records)))
    for opcode,arg in records:b.extend(struct.pack('<Ii9I',opcode,arg,*([0]*9)))
    path.write_bytes(b)
def main():
    parent=REPO/'working/tests/animation';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(f'Animation contract evidence: {out}',flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);print(p.stdout,end='',flush=True);p.check_returncode()
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe'
        if sha(exe.read_bytes())!=HASH:raise ValueError('Unsupported executable')
        sources=['assets/animation.cpp','assets/animation.hpp','assets/animation_inspect_main.cpp',
                 'reconstruction/animation/no_cd.cpp','reconstruction/animation/no_cd.hpp',
                 'tests/animation-binary-reference.cpp','tools/test-animation-contract.py']
        hashes={path:sha((REPO/path).read_bytes()) for path in sources}
        helper=out/'reference'
        command=['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(REPO/'reconstruction/animation'),
                 '-I'+str(REPO/'assets'),str(REPO/'tests/animation-binary-reference.cpp'),
                 str(REPO/'reconstruction/animation/no_cd.cpp'),'-o',str(helper)]
        subprocess.run(command,check=True,capture_output=True,text=True,timeout=60)
        root=REPO/'working/game-clean';files=sorted(p for p in root.rglob('*') if p.is_file() and p.suffix.lower()=='.ani')
        snapshot={str(p.relative_to(root)):sha(p.read_bytes()) for p in files}
        requests=[''.join(c.upper() if n%2 else c.lower() for n,c in enumerate(str(p.relative_to(root)))).replace('/','\\') for p in files]
        requests=[('c:\\MagicMayhem\\' if i%2 else '')+s for i,s in enumerate(requests)]
        manifest=out/'manifest.json';manifest.write_text(json.dumps(requests))
        inspector=REPO/'working/build/assets/mnm-animation-inspect';inspector_hash=sha(inspector.read_bytes())
        p=subprocess.run([str(inspector),str(root),str(manifest)],capture_output=True,text=True,timeout=60);p.check_returncode()
        (out/'native-inspection.json').write_text(p.stdout);native=json.loads(p.stdout)['files']
        if len(native)!=len(files):raise ValueError('Inspection omitted files')
        cases=[];inventory=[];ticks=64
        def trace(path,group,kind,break_tick=-1):
            p=subprocess.run([str(helper),str(exe),str(path),str(group),str(ticks),str(break_tick)],capture_output=True,text=True,timeout=5)
            index=len(cases);(out/f'trace-{index:03}.json').write_text(p.stdout);(out/f'trace-{index:03}.stderr').write_text(p.stderr)
            if p.returncode:raise ValueError(f'Controller trace failed for {path}:{group}: {p.stderr}')
            rows=json.loads(p.stdout)
            if len(rows)!=ticks+1:raise ValueError('Incomplete trace')
            cases.append({'path':str(path.relative_to(root)) if path.is_relative_to(root) else path.name,'kind':kind,
                          'sequence':group,'break_tick':break_tick,'states':len(rows),'matched_original':True,
                          'trace_sha256':sha(p.stdout.encode()),'nonzero_events':sum(r[1]!=0 for r in rows),
                          'distinct_sprites':len({r[2] for r in rows if r[2]>=0})})
        for path,request,decoded in zip(files,requests,native):
            b=path.read_bytes();version=struct.unpack_from('<I',b,12)[0]
            if decoded['path']!=request:raise ValueError('Inspection reordered requests')
            if version!=5:
                count,n=struct.unpack_from('<I',b,8)[0],struct.unpack_from('<I',b,20)[0]
                stride={3:28,4:36}[version];base=44+n*4
                normalized=b''.join(b[at:at+stride]+bytes(44-stride) for at in range(base,len(b),stride))
                starts=list(struct.unpack_from('<'+str(n)+'I',b,44))
                if not decoded['decoded'] or decoded['version']!=version or decoded['records']!=count or decoded['starts']!=starts or decoded['records_sha256']!=sha(normalized):
                    raise ValueError('Legacy ANI native expansion differs')
                inventory.append({'path':str(path.relative_to(root)),'sha256':sha(b),'version':version,'supported':True,'records':count,'sequences':n-1,'native_record_bytes_match':True,'loader_only':True});continue
            _,count,_,opaque,n=struct.unpack_from('<5I',b,4);base=44+4*n
            starts=list(struct.unpack_from('<'+str(n)+'I',b,44))
            if not decoded['decoded'] or decoded['records']!=count or decoded['starts']!=starts or decoded['opaque_header']!=opaque or decoded['sprite_name_hex']!=b[24:44].hex() or decoded['records_sha256']!=sha(b[base:]):
                raise ValueError('Native ANI metadata/record bytes differ')
            inventory.append({'path':str(path.relative_to(root)),'sha256':sha(b),'version':version,'supported':True,
                              'records':count,'sequences':n-1,'native_record_bytes_match':True})
            # One sequence per file plus representative control/empty sequences.
            groups={0}
            for opcode in range(1,7):
                for g,(a,z) in enumerate(zip(starts,starts[1:])):
                    if any(struct.unpack_from('<I',b,base+i*44)[0]==opcode for i in range(a,z)):
                        groups.add(g);break
            for group in sorted(groups):trace(path,group,'installed')
        fixtures=[('sprite-stop',[(0,0),(0,1),(6,-1)],-1),
                  ('delay-event',[(1,2),(0,3),(5,-7),(0,4),(6,-1)],-1),
                  ('repeat',[(2,3),(0,0),(0,1),(3,-2),(6,-1)],-1),
                  ('jump-break',[(0,0),(0,1),(4,-2),(0,2),(6,-1)],7),
                  ('initial-event',[(5,8),(0,0),(5,9),(0,1),(6,-1)],-1),
                  ('unknown',[(8,123),(0,0),(6,-1)],-1),('empty',[(6,-1)],-1)]
        for name,records,break_tick in fixtures:
            path=out/(name+'.ani');fixture(path,records);trace(path,0,'synthetic',break_tick)
        if sha(exe.read_bytes())!=HASH or sha(inspector.read_bytes())!=inspector_hash or any(sha((root/path).read_bytes())!=digest for path,digest in snapshot.items()) or any(sha((REPO/path).read_bytes())!=digest for path,digest in hashes.items()):
            raise ValueError('Experiment input/source changed')
        report={'executable_sha256':HASH,'inspector_sha256':inspector_hash,'harness_sha256':sha(helper.read_bytes()),
                'source_sha256':hashes,'compile_command':command,'input_unchanged':True,'live_game_validated':False,
                'scope':'Version-3/4 expansion, version-5 bytes and selected version-5 forward/no-reverse start/tick traces; bounded native safety differs for malformed inputs',
                'summary':{'files':len(files),'decoded':sum(f['supported'] for f in inventory),'legacy_rejected':sum(not f['supported'] for f in inventory),'legacy_decoded':sum(f['version']!=5 and f['supported'] for f in inventory),
                           'records':sum(f.get('records',0) for f in inventory),'traces':len(cases),'states':sum(c['states'] for c in cases),
                           'all_traces_match':True},'files':inventory,'cases':cases}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report['summary']),flush=True)
    finally:verify('after')
if __name__=='__main__':main()
