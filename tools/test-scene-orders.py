#!/usr/bin/env python3
"""Synthetic production-widget/native-order validation and fresh-process restore."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
ROOT = Path(__file__).resolve().parents[1]
def main():
    if '--inner' not in sys.argv:
        subprocess.run(['xvfb-run','-a','--server-num',str(200+os.getpid()%10000),sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=120)
        return
    args=[a for a in sys.argv[1:] if a!='--inner']
    if len(args)!=2:raise ValueError('Supply world-controls-test and new output directory')
    binary=Path(args[0]).resolve();out=Path(args[1]).resolve();out.mkdir(parents=True,exist_ok=False)
    os.environ['QT_QPA_PLATFORM']='xcb'
    def run(*command):
        p=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60)
        if p.returncode or 'runtime error:' in p.stderr or 'AddressSanitizer' in p.stderr:raise RuntimeError((command,p.returncode,p.stdout,p.stderr))
        return p.stdout
    result=json.loads(run(binary,out));assert result['all_match']
    run(binary,'resume',out/'pending.mnw',out/'resumed.mnw',3)
    assert (out/'resumed.mnw').read_bytes()==(out/'whole.mnw').read_bytes()
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    paths=sorted(set([*ROOT.glob('apps/world-scene/*.cpp'),*ROOT.glob('apps/world-scene/*.hpp'),*ROOT.glob('game/**/*.cpp'),*ROOT.glob('game/**/*.hpp'),ROOT/'apps/world-scene/CMakeLists.txt',ROOT/'tests/world-controls-test.cpp',Path(__file__).resolve()]))
    result.update(live_validated=False,fresh_process_continuations=1,
        scope='Synthetic production Qt movement controls connected to native Orders and owned multi-creature session: full-generation selection, queued per-actor commands, Step-only application, invalid/stale/cleanup refusal and pending-order exact restart. No original input mappings, installed scene-picking/lighting or live equivalence.',
        source_sha256={str(p.relative_to(ROOT)):sha(p) for p in paths},binary_sha256=sha(binary),
        artifact_sha256={p.name:sha(p) for p in sorted(out.iterdir()) if p.is_file()})
    (out/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'all_match':True,'fresh_process_continuations':1,'report':str(out/'report.json')}))
if __name__=='__main__':main()
