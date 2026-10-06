#!/usr/bin/env python3
"""Reject invalid recovery launches before game-artifact staging; synthetic only."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import frame_v1 as frame, render_commands_v1 as bounded, render_commands_v2 as ring, render_control_v1 as control

def create(path,module,session=0):
    b=bytearray(module.initial_header())
    if session:struct.pack_into('<I',b,16,session)
    with path.open('wb') as f:f.write(b);f.truncate(module.SIZE)

def main():
    parent=ROOT/'working/tests/render-control-launcher';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    paths=['tools/test-render-control-launcher.py','tools/run-opengl-game.py']+[f'protocols/python/mnm_protocols/{name}_v{version}.py' for name,version in [('frame',1),('render_commands',1),('render_commands',2),('render_control',1)]]
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();sources={p:sha(ROOT/p) for p in paths}
    report=dict(success=True,sources=sources,cases=[],scope='Synthetic invalid CLI launch admission before original/game staging; no original artifacts consumed.')
    for mode in ['outside','not-continuous','size','magic','pending','cancel','identity','v1','claimed']:
        case=run/mode;case.mkdir();f=case/'frame';r=case/'ring';c=case/'control';create(f,frame);create(r,bounded if mode=='v1' else ring,123);create(c,control,123)
        if mode=='size':c.write_bytes(c.read_bytes()[:-1])
        if mode=='magic':
            with c.open('r+b') as stream:stream.write(b'badmagic')
        at=20 if mode=='pending' else 40 if mode=='cancel' else 16 if mode=='identity' else None
        if at is not None:
            with c.open('r+b') as stream:stream.seek(at);stream.write(struct.pack('<I',999 if mode=='identity' else 1))
        if mode=='claimed':
            with r.open('r+b') as stream:stream.seek(24);stream.write(struct.pack('<I',1))
        paths_before={p:sha(p) for p in [f,r,c]}
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        if mode!='not-continuous':env['MNM_RENDER_CONTINUOUS']='1'
        command=['python3',str(ROOT/'tools/run-opengl-game.py'),'--stream',str(f),'--command-channel',str(r),'--render-control','/tmp/no-render-control' if mode=='outside' else str(c)]
        result=subprocess.run(command,cwd=ROOT,env=env,capture_output=True,text=True,timeout=15)
        expected='Recovery requires' if mode in ['outside','not-continuous'] else 'launch ID mismatch' if mode=='identity' else 'requires a v2' if mode=='v1' else 'not a fresh session' if mode=='claimed' else 'Invalid fresh rendering control'
        assert result.returncode and expected in result.stderr,(mode,result.stderr)
        assert all(sha(p)==h for p,h in paths_before.items()),'input mutated on refusal'
        report['cases'].append(dict(mode=mode,success=True,refusal=expected))
    assert all(sha(ROOT/p)==h for p,h in sources.items())
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json')
if __name__=='__main__':main()
