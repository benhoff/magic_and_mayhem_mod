#!/usr/bin/env python3
"""Compare bounded forward phase switches with the hash-pinned original x86."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parent.parent
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parent = REPO/'working/tests/animation-switch'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    def verify(phase):
        p = subprocess.run([str(REPO/'tools/original-manifest.sh'), 'verify'], capture_output=True, text=True, timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr); p.check_returncode()
    verify('before')
    try:
        exe = REPO/'working/game-nocd/Chaos.exe'
        if sha(exe) != HASH: raise ValueError('Unsupported executable')
        sources = [REPO/p for p in ('reconstruction/animation/no_cd.cpp', 'reconstruction/animation/no_cd.hpp',
                   'tests/animation-binary-reference.cpp', 'tools/test-animation-switch.py')]
        inputs = {p: sha(p) for p in sources+[exe]}
        helper = out/'reference'
        command = ['g++', '-m32', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I'+str(REPO/'assets'),
                   '-I'+str(REPO/'reconstruction/animation'), str(sources[2]), str(sources[0]), '-o', str(helper)]
        subprocess.run(command, check=True, capture_output=True, timeout=60)
        # Retain static evidence supporting numeric eight-facing groups. No
        # compass labels or inferred gameplay action names enter the preview.
        ranges = {'switch':(0x464e20,0x464ebd), 'group_validator':(0x504cc0,0x505161),
                  'facing':(0x508250,0x508416), 'attached_facing':(0x50e52f,0x50e5ad),
                  'configured_action':(0x513ff0,0x51403d), 'direction_lookup':(0x4eade0,0x4eaded)}
        for name,(a,z) in ranges.items():
            p = subprocess.run(['objdump','-d','-Mintel',f'--start-address={a}',f'--stop-address={z}',str(exe)],check=True,capture_output=True,text=True)
            (out/(name+'.asm')).write_text(p.stdout)
        cases=[]
        root=REPO/'working/game-clean'
        for path in sorted(root.rglob('*.ani')):
            b=path.read_bytes()
            if len(b)<44 or struct.unpack_from('<I',b,12)[0]!=5: continue
            n=struct.unpack_from('<I',b,20)[0];starts=struct.unpack_from('<'+str(n)+'I',b,44);base=44+4*n
            records=[struct.unpack_from('<Ii',b,base+i*44) for i in range(starts[-1])]
            # One structurally aligned, initially visible group per asset.
            for group in range(0,n-8,8):
                seqs=[records[starts[g]:starts[g+1]] for g in range(group,group+8)]
                shape=lambda seq:[(op,0 if op==0 else arg) for op,arg in seq]
                if not seqs[0] or seqs[0][0][0]!=0 or not all(shape(s)==shape(seqs[0]) for s in seqs): continue
                inputs[path]=sha(path)
                for target in range(group+1,group+8):
                    for at in (0,3,7):
                        # Only switch while still active: avoid original null
                        # pointer arithmetic and short target overrun policies.
                        before=subprocess.run([str(helper),str(exe),str(path),str(group),str(at),'-1'],capture_output=True,text=True,timeout=5)
                        before.check_returncode()
                        if not json.loads(before.stdout)[-1][5]: continue
                        p=subprocess.run([str(helper),str(exe),str(path),str(group),'32','-1',str(target),str(at)],capture_output=True,text=True,timeout=5)
                        if p.returncode: raise ValueError(f'{path}:{group}->{target}@{at}: {p.stderr}')
                        trace=json.loads(p.stdout)
                        if len(trace)!=33: raise ValueError('Incomplete trace')
                        name=f'trace-{len(cases):04}.json';(out/name).write_text(p.stdout)
                        cases.append({'asset':str(path.relative_to(root)), 'from':group,'to':target,'switch_tick':at,
                                      'states':len(trace),'sha256':hashlib.sha256(p.stdout.encode()).hexdigest()})
                break
        # Explicit delay/repeat/break state preservation and signed event fixture.
        seq=[(0,2),(1,2),(2,3),(0,3),(5,-7),(3,-2),(6,-1)]
        fixture=out/'controls.ani';count=2*len(seq)
        blob=b'ANI\0'+struct.pack('<5I',56+44*count,count,5,0,3)+b'fixture.spr\0'.ljust(20,b'\0')+struct.pack('<3I',0,len(seq),count)
        for delta in (0,10):
            for op,arg in seq:blob+=struct.pack('<Ii9I',op,arg+delta if op==0 else arg,*([0]*9))
        fixture.write_bytes(blob)
        for at in (0,2,3,5,7):
            p=subprocess.run([str(helper),str(exe),str(fixture),'0','32','1','1',str(at)],capture_output=True,text=True,timeout=5)
            p.check_returncode();name=f'controls-{at}.json';(out/name).write_text(p.stdout)
            cases.append({'asset':'synthetic controls','from':0,'to':1,'switch_tick':at,'states':33,'sha256':hashlib.sha256(p.stdout.encode()).hexdigest()})
        if not cases or any(sha(p)!=digest for p,digest in inputs.items()):raise ValueError('Missing traces or changed input')
        report={'executable_sha256':HASH,'sources':{str(p.relative_to(REPO)):v for p,v in inputs.items()},
                'helper_sha256':sha(helper),'compile_command':command,'input_unchanged':True,'live_validated':False,
                'summary':{'traces':len(cases),'states':sum(c['states'] for c in cases),'installed_assets':len({c['asset'] for c in cases if c['asset']!='synthetic controls'}),'all_match':True},
                'static_artifacts':{p.name:sha(p) for p in out.glob('*.asm')},'cases':cases}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(report['summary'],flush=True)
    finally: verify('after')
if __name__=='__main__': main()
