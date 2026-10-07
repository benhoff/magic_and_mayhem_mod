#!/usr/bin/env python3
"""Observe an original single-player battle queue under isolated Xvfb/Wine."""
import argparse
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import signal
import struct
import subprocess
import time

ROOT=Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64')
    parser.add_argument('--samples',type=int,choices=range(1,17),default=4)
    parser.add_argument('--map',type=int,choices=range(1,129),default=2)
    args=parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a -s "-screen 0 1280x1024x24"')
    sources=[*sorted((ROOT/'runtime/scene').glob('*.[chS]')),ROOT/'protocols/include/mnm/scene_snapshot_v1.h',
             ROOT/'tools/build-scene-observer.py',ROOT/'tools/prepare-scene-observer.py',ROOT/'tools/capture-scene-game.py',ROOT/'tools/scene-game-runner.py']
    fingerprints={str(p.relative_to(ROOT)):sha(p) for p in sources}
    spec=importlib.util.spec_from_file_location('scene_prepare',ROOT/'tools/prepare-scene-observer.py')
    staging=importlib.util.module_from_spec(spec);spec.loader.exec_module(staging)
    process=None;channel=None;trace=[]
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        root=staging.prepare(menu=True);print(root,flush=True)
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(MNM_SCENE_EXPERIMENT=str(root),MNM_SCENE_SAMPLES=str(args.samples),WINEDEBUG='-all',
                   WINEPREFIX=str(root/'wineprefix'))
        env.pop('WAYLAND_DISPLAY',None)
        subprocess.run(['cp','-a','--reflink=auto',str(args.prefix_template.resolve()),env['WINEPREFIX']],check=True)
        with (root/'wineboot.log').open('x') as log:
            subprocess.run(['wineboot','-u'],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=90,check=True)
        raw=bytearray(32768);raw[:16]=b'MNMMCMD2'+struct.pack('<II',2,32768);struct.pack_into('<III',raw,16,2,1,1)
        with (root/'channel.bin').open('xb') as f:f.write(raw)
        channel_file=(root/'channel.bin').open('r+b');channel=mmap.mmap(channel_file.fileno(),0)
        heartbeat=1;request=0;sequence=2

        def state():
            before=struct.unpack_from('<I',channel,128)[0]
            row=struct.unpack_from('<7I',channel,132)
            payload=bytes(channel[160:16924])
            return (row,payload) if not before&1 and before==struct.unpack_from('<I',channel,128)[0] else None

        def beat(action=0,generation=0,argument=0,rules=None):
            nonlocal heartbeat,request,sequence
            heartbeat+=1;sequence+=2
            if action:request+=1
            struct.pack_into('<I',channel,16,sequence-1)
            struct.pack_into('<6I',channel,20,1,heartbeat,request,action,generation,argument)
            if rules is not None:struct.pack_into('<17I',channel,44,*rules)
            struct.pack_into('<I',channel,16,sequence)

        def wait(screen=None,ack=None,timeout=60):
            nonlocal heartbeat,sequence
            deadline=time.monotonic()+timeout
            while time.monotonic()<deadline:
                s=state()
                if s and (screen is None or s[0][1]==screen and s[0][2]) and (ack is None or s[0][3]==ack):return s
                # Refresh heartbeat without replacing an outstanding command.
                heartbeat+=1;sequence+=2;struct.pack_into('<I',channel,16,sequence-1)
                struct.pack_into('<I',channel,24,heartbeat);struct.pack_into('<I',channel,16,sequence)
                if process.poll() is not None:raise RuntimeError('Original game exited before scene capture')
                time.sleep(.02)
            raise RuntimeError('Original menu/scene startup timed out')

        def send(screen,action,argument=0,rules=None):
            s=wait(screen);beat(action,s[0][0],argument,rules);reply=wait(ack=request)
            if reply[0][4]!=1:raise RuntimeError('Original menu request refused: '+str(reply[0]))
            trace.append({'screen':screen,'action':action,'argument':argument,'ack':request,'status':reply[0][4]})

        with (root/'game.log').open('x') as log:
            process=subprocess.Popen([str(ROOT/'tools/run-game.sh'),'launch','--no-gamescope','--prefix',env['WINEPREFIX'],
                '--runner',str(ROOT/'tools/scene-game-runner.py')],env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            send(3,1);send(22,4)
            s=wait(14);rules=list(struct.unpack_from('<13I',s[1],8))
            rules+= [struct.unpack_from('<I',s[1],60+i*48+12)[0] for i in range(4)]
            rules[2]=0 # Established diagnostic menu fixture: bypass spell-selection handoff.
            send(14,6,rules=rules);send(25,9,args.map)
            send(14,7,rules=rules)
            deadline=time.monotonic()+60
            while time.monotonic()<deadline:
                paths=sorted((root/'capture').glob('scene-*.bin'))
                complete=[]
                for path in paths:
                    with path.open('rb') as capture:header=capture.read(64)
                    if len(header)==64 and header[:8]==b'MNMSCNE1' and path.stat().st_size==struct.unpack_from('<I',header,16)[0]:complete.append(path)
                if len(complete)>=args.samples:paths=complete;break
                heartbeat+=1;sequence+=2;struct.pack_into('<I',channel,16,sequence-1)
                struct.pack_into('<I',channel,24,heartbeat);struct.pack_into('<I',channel,16,sequence)
                if process.poll() is not None:raise RuntimeError('Original game exited during scene capture')
                time.sleep(.05)
            else:raise RuntimeError('Original simulation produced no complete bounded scene samples')
            subprocess.run(['import','-window','root',str(root/'original-window.png')],env=env,timeout=10,check=True)
        metadata=json.loads((root/'manifest.json').read_text())
        for name,key in [('Chaos.exe','staged_sha256'),('MnmScene.dll','scene_dll_sha256'),('MnmMenu.dll','menu_dll_sha256')]:
            if sha(root/'game'/name)!=metadata[key]:raise RuntimeError('Staged binary changed')
        if {str(p.relative_to(ROOT)):sha(p) for p in sources}!=fingerprints:raise RuntimeError('Capture sources changed during validation')
        report={'success':True,'live_observation':True,'live_replacement':False,'original_pixels_compared':False,
                'scope':'Original Single Player battle setup and ordered queue consumer entry. Original simulation/drawing remain active; bounded immutable snapshots, no whole-scene visual equivalence.',
                'sources':fingerprints,'sources_stable':True,'source_executable_sha256':metadata['source_sha256'],
                'samples':args.samples,'map_selection':args.map,'menu_trace':trace,'snapshots':{str(p.relative_to(root)):sha(p) for p in paths},'experiment':str(root)}
        with (root/'report.json').open('x') as f:json.dump(report,f,indent=2);f.write('\n')
        print(root/'report.json',flush=True)
    finally:
        if process and process.poll() is None:
            os.killpg(process.pid,signal.SIGTERM)
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
        if channel:channel.close()
        if process:
            subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
