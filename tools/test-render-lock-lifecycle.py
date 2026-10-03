#!/usr/bin/env python3
"""Exercise actual PE32 hook forwarding and game-owned pixel lifetimes under Wine."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parent.parent

def load(name,path):
    spec=importlib.util.spec_from_file_location(name,REPO/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    dll=load('render_build','tools/build-render-bridge.py').build(True)
    stage=load('render_stage','tools/prepare-shadow-experiment.py')
    parent=REPO/'working/tests/render-lock-lifecycle';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));reports=[]
    for mode in ('modern','negative','unlock-retry','offscreen','indexed','failed-lock','readonly','partial','bad-mask'):
        case=root/mode;case.mkdir();capture=case/'capture';capture.mkdir();stream=case/'frame.bin'
        with stream.open('wb') as f:f.write(b'MNMGL001'+struct.pack('<2I',1,64)+bytes(48));f.truncate(64+2048*2048*4)
        shutil.copy2(dll,case/dll.name)
        (case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        env=os.environ.copy()
        for key in ('MNM_RENDER_CAPTURE_DIR','MNM_RENDER_HISTORY','MNM_RENDER_FAILURE_LOG','MNM_RENDER_NO_READBACK'):env.pop(key,None)
        env.update(WINEPREFIX=str(REPO/'working/tests/render-wine'),WINEDEBUG='-all',MNM_LOCK_LIFECYCLE_SELFTEST=mode,
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(stream).replace('/','\\'))
        with (case/'wine.log').open('w') as log:subprocess.run(['wine',str(case/'selftest.exe')],cwd=case,env=env,stdout=log,stderr=log,check=True,timeout=30)
        lines=[line.split() for line in (capture/'lifecycle.log').read_text().splitlines()]
        assert all(len(line)==20 for line in lines)
        reasons={line[0] for line in lines}
        expected_reason={'failed-lock':'lock_failed','readonly':'lock_readonly','partial':'lock_partial','bad-mask':'unlock_masks'}.get(mode,'unlock_copied')
        assert expected_reason in reasons,(mode,reasons)
        if mode=='unlock-retry':assert 'unlock_failed' in reasons and 'unlock_succeeded' in reasons
        files=list(capture.glob('lock-*.bin'));expected=mode in ('modern','negative','unlock-retry','offscreen','indexed')
        assert len(files)==int(expected),(mode,files)
        with stream.open('rb') as f:header=struct.unpack('<16I',f.read(64));rgba=f.read(16)
        primary=mode in ('modern','negative','unlock-retry')
        assert header[10]==int(primary),(mode,header)
        if expected:
            data=files[0].read_bytes();h=struct.unpack('<16I',data[:64]);payload=data[64:]
            assert data[:8]==b'MNMLOCK1' and h[2:5]==(1,64,1) and h[9:11]==(2,2)
            assert len(payload)==h[15] and h[6]>0
            if mode=='indexed':assert h[11]==8 and payload==bytes.fromhex('00f81f00')
            else:
                native=bytes.fromhex('00f8e0071f00ffff')
                if mode=='negative':native=native[4:]+native[:4]
                assert payload==native,(mode,payload.hex())
            if primary:
                expected_rgba=bytes.fromhex('ff0000ff00ff00ff0000ffffffffffff')
                if mode=='negative':expected_rgba=expected_rgba[8:]+expected_rgba[:8]
                assert rgba==expected_rgba,(mode,rgba.hex())
        reports.append({'mode':mode,'snapshots':len(files),'primary_frames':header[10],'diagnostic_reasons':sorted(reasons)})
    (root/'report.json').write_text(json.dumps({'origin':'synthetic_game_owned_locks','cases':reports},indent=2)+'\n')
    print(f'Lock/Unlock lifecycle passed: {root}')
if __name__=='__main__':main()
