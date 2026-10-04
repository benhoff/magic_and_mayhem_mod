#!/usr/bin/env python3
"""Compare ANI placement/attachment helpers in an unchanged, pinned PE32 image."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parent=REPO/'working/tests/animation-placement';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe'
        if sha(exe)!=HASH:raise ValueError('Unsupported executable')
        sources=[REPO/p for p in ('reconstruction/animation/placement.cpp','reconstruction/animation/placement.hpp','assets/animation.hpp','tests/animation-placement-reference.cpp','tools/test-animation-placement.py')]
        inputs={p:sha(p) for p in sources+[exe]};helper=out/'reference'
        command=['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(REPO/'assets'),'-I'+str(REPO/'reconstruction/animation'),str(sources[3]),str(sources[0]),'-o',str(helper)]
        p=subprocess.run(command,capture_output=True,text=True,timeout=60);(out/'compile.log').write_text(p.stdout+p.stderr);p.check_returncode()
        ranges={'body':(0x507190,0x507250),'attachment_one':(0x507250,0x5072b1),'attachment_two':(0x5072c0,0x507321),
                'overlay':(0x507330,0x5073c1),'creature_draw':(0x4fa190,0x4fb5b0),'queue':(0x4ffd30,0x4fff60),'sprite_origin':(0x57e1b0,0x57e250),'tile_size_config':(0x502df0,0x502e63)}
        for name,(a,z) in ranges.items():
            p=subprocess.run(['objdump','-d','-Mintel',f'--start-address={a}',f'--stop-address={z}',str(exe)],check=True,capture_output=True,text=True)
            (out/(name+'.asm')).write_text(p.stdout)
        cases=[];root=REPO/'working/game-clean'
        def compare(path):
            inputs[path]=sha(path);p=subprocess.run([str(helper),str(exe),str(path)],capture_output=True,text=True,timeout=10)
            (out/f'case-{len(cases):03}.json').write_text(p.stdout);(out/f'case-{len(cases):03}.stderr').write_text(p.stderr)
            if p.returncode:raise ValueError(f'{path}: {p.stderr}')
            cases.append({'asset':str(path.relative_to(REPO)),**json.loads(p.stdout),'all_match':True})
        for path in sorted(root.rglob('*')):
            if path.is_file() and path.suffix.lower()=='.ani' and struct.unpack_from('<I',path.read_bytes(),12)[0]==5:compare(path)
        # Boundary/random bit patterns prove signed offsets and wrapping without
        # invoking undefined signed arithmetic in the native reconstruction.
        import random
        rng=random.Random(730);records=[]
        for v in (0,1,0x7fffffff,0x80000000,0xffffffff):records.append(struct.pack('<11I',0,0,*([v]*9)))
        for _ in range(128):records.append(struct.pack('<11I',0,0,*(rng.getrandbits(32) for _ in range(9))))
        data=b'ANI\0'+struct.pack('<5I',52+44*(len(records)+1),len(records)+1,5,0,2)+b'fixture.spr\0'.ljust(20,b'\0')+struct.pack('<2I',0,len(records)+1)+b''.join(records)+struct.pack('<11I',6,0,*([0]*9))
        fixture=out/'boundaries.ani';fixture.write_bytes(data);compare(fixture)
        if any(sha(p)!=digest for p,digest in inputs.items()):raise ValueError('Input/source changed')
        report={'scope':'Four original placement/attachment helpers; no live entity, world projection, palette or scene replacement',
                'executable_sha256':HASH,'source_and_input_sha256':{str(p.relative_to(REPO)):v for p,v in inputs.items()},
                'compile_command':command,'helper_sha256':sha(helper),'static_artifacts':{p.name:sha(p) for p in out.glob('*.asm')},
                'input_unchanged':True,'live_validated':False,'summary':{'assets':len(cases)-1,'sprite_records':sum(c['sprite_records'] for c in cases),
                'helper_matches':sum(c['helper_matches'] for c in cases),'all_match':True},'cases':cases}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(report['summary'],flush=True)
    finally:verify('after')
if __name__=='__main__':main()
