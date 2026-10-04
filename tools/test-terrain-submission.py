#!/usr/bin/env python3
"""Compare selected terrain producer in an unchanged, pinned PE32 image."""
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
    parent=REPO/'working/tests/terrain-submission';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe'
        if sha(exe)!=HASH:raise ValueError('Unsupported executable')
        sources=[REPO/p for p in ('reconstruction/rendering/terrain_submission.cpp','reconstruction/rendering/terrain_submission.hpp','tests/terrain-submission-reference.cpp','tools/test-terrain-submission.py')]
        catalog=REPO/'working/game-clean/Realms/Celtic/Forest/Terrain.ttd'
        sprite=REPO/'working/game-clean/Realms/Celtic/Forest/Terrain.spr'
        inputs={p:sha(p) for p in sources+[exe,catalog,sprite,REPO/'reconstruction/rendering/sprite_queue.cpp',REPO/'reconstruction/rendering/sprite_queue.hpp']};helper=out/'reference'
        command=['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(REPO/'reconstruction/rendering'),str(sources[2]),str(sources[0]),str(REPO/'reconstruction/rendering/sprite_queue.cpp'),'-o',str(helper)]
        p=subprocess.run(command,capture_output=True,text=True,timeout=60);(out/'compile.log').write_text(p.stdout+p.stderr);p.check_returncode()
        for name,a,z in [('producer',0x4f8960,0x4fbcb1),('builder',0x4ffd30,0x4fff60)]:
            p=subprocess.run(['objdump','-d','-Mintel',f'--start-address={a}',f'--stop-address={z}',str(exe)],check=True,capture_output=True,text=True)
            (out/(name+'.asm')).write_text(p.stdout)
        p=subprocess.run([str(helper),str(exe),str(catalog),str(sprite)],capture_output=True,text=True,timeout=30)
        (out/'reference.stdout').write_text(p.stdout);(out/'reference.stderr').write_text(p.stderr);p.check_returncode()
        if any(sha(p)!=h for p,h in inputs.items()):raise ValueError('Input changed')
        report={'executable_sha256':HASH,'source_and_input_sha256':{str(p.relative_to(REPO)):h for p,h in inputs.items()},'helper_sha256':sha(helper),'all_match':True,'live_validated':False,'summary':{k:v for k,v in json.loads(p.stdout).items() if k!='previews'},'previews':json.loads(p.stdout)['previews'],'scope':'Selected ordinary terrain producer with synthetic definitions and tile state; inactive objects/overlays, pointer outside frames, no Win32 calls'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(report['summary'])
    finally:verify('after')
if __name__=='__main__':main()
