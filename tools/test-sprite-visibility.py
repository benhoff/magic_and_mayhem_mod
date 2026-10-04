#!/usr/bin/env python3
"""Compare SPR visibility helpers in an unchanged, pinned PE32 image."""
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
    parent=REPO/'working/tests/sprite-visibility';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe'
        if sha(exe)!=HASH:raise ValueError('Unsupported executable')
        sources=[REPO/p for p in ('reconstruction/rendering/sprite_visibility.cpp','reconstruction/rendering/sprite_visibility.hpp','tests/sprite-visibility-reference.cpp','tools/test-sprite-visibility.py')]
        inputs={p:sha(p) for p in sources+[exe]};helper=out/'reference'
        command=['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(REPO/'reconstruction/rendering'),str(sources[2]),str(sources[0]),'-o',str(helper)]
        p=subprocess.run(command,capture_output=True,text=True,timeout=60);(out/'compile.log').write_text(p.stdout+p.stderr);p.check_returncode()
        for name,a,z in [('mask_helper',0x5013c0,0x5015e3),('pass',0x5015f0,0x5017da),('grid_init',0x501330,0x5013b1),('caller',0x4fd3c4,0x4fd42e)]:
            p=subprocess.run(['objdump','-d','-Mintel',f'--start-address={a}',f'--stop-address={z}',str(exe)],check=True,capture_output=True,text=True)
            (out/(name+'.asm')).write_text(p.stdout)
        p=subprocess.run([str(helper),str(exe)],capture_output=True,text=True,timeout=30)
        (out/'reference.stdout').write_text(p.stdout);(out/'reference.stderr').write_text(p.stderr);p.check_returncode()
        if any(sha(p)!=h for p,h in inputs.items()):raise ValueError('Input changed')
        report={'executable_sha256':HASH,'source_and_input_sha256':{str(p.relative_to(REPO)):h for p,h in inputs.items()},'helper_sha256':sha(helper),'all_match':True,'live_validated':False,'summary':{k:v for k,v in json.loads(p.stdout).items() if k!='scenes'},'scenes':json.loads(p.stdout).get('scenes',[]),'scope':'Selected visibility helper and complete reverse pass; grid bytes, draw kinds and resolved terrain-owner flags; no live world inputs'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(report['summary'])
    finally:verify('after')
if __name__=='__main__':main()
