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
    parser.add_argument('--word-sprites',choices=['shadow','takeover'],help='Observe the explicit partial direct-word raster experiment')
    parser.add_argument('--world-frames',action='store_true',help='Capture complete effective raster inputs and separate original output oracles')
    parser.add_argument('--world-lifetime',action='store_true',help='Bounded diagnostic canvas identity/initial-content observations from first queue call')
    parser.add_argument('--observe-world-refusals',action='store_true',help='Retain refused startup traces as diagnostics, without claiming native admission')
    parser.add_argument('--world-live',choices=['normal','verify'],help='Continuous Qt native World shadow presentation; optional independent pixel comparison')
    parser.add_argument('--live-frames',type=int,choices=range(1,257),default=24)
    parser.add_argument('--skip-queues',type=int,default=None,help='Wait this many queue calls before the first sample (World default: 120)')
    parser.add_argument('--interval',type=int,default=1,help='Queue calls between samples')
    args=parser.parse_args()
    if args.world_live:args.world_frames=True
    if args.observe_world_refusals and (not args.world_frames or not args.world_lifetime or args.world_live):parser.error('Refusal diagnostics require finite --world-frames --world-lifetime')
    if args.world_frames and args.word_sprites:parser.error('Use separate World and word takeover experiments')
    if args.skip_queues is None:args.skip_queues=120 if args.world_frames else 0
    if not 0<=args.skip_queues<=3600 or not 1<=args.interval<=3600:parser.error('Queue sample bounds exceeded')
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a -s "-screen 0 1280x1024x24"')
    sources=[*sorted((ROOT/'runtime/scene').glob('*.[chS]')),ROOT/'protocols/include/mnm/scene_snapshot_v1.h',
             ROOT/'tools/build-scene-observer.py',ROOT/'tools/prepare-scene-observer.py',ROOT/'tools/capture-scene-game.py',ROOT/'tools/scene-game-runner.py',ROOT/'protocols/include/mnm/world_frame_v1.h',ROOT/'protocols/include/mnm/world_channel_v1.h']
    if args.world_live:
        for directory in ['assets','renderer','compat/legacy']:
            sources += [p for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ['.cpp','.hpp','.c','.h','.S'] or p.name in ['CMakeLists.txt','README.md'])]
        sources += [ROOT/p for p in ['apps/qt-shell/live_world_session.cpp','apps/qt-shell/live_world_session.hpp','apps/qt-shell/world_live_main.cpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp']]
    if args.word_sprites:sources += [ROOT/p for p in ['renderer/sprites/word_raster.h','renderer/sprites/word_raster.c','runtime/shadow/win32_min.h','tools/build-word-sprites.py']]
    fingerprints={str(p.relative_to(ROOT)):sha(p) for p in sources}
    spec=importlib.util.spec_from_file_location('scene_prepare',ROOT/'tools/prepare-scene-observer.py')
    staging=importlib.util.module_from_spec(spec);spec.loader.exec_module(staging)
    process=None;viewer=None;channel=None;trace=[]
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        root=staging.prepare(menu=True,word_mode=args.word_sprites,world_frames=args.world_frames,world_live=bool(args.world_live));print(root,flush=True)
        if args.world_live:
            size=128+2*(16+32*1024*1024+8*1024*1024)
            header=bytearray(128);header[:8]=b'MNMWCH01'
            struct.pack_into('<15I',header,8,1,size,1,0,0,0,0,0,0,int(args.world_live=='verify'),0,2,32*1024*1024,8*1024*1024,0x40209ca7)
            with (root/'world-channel.bin').open('xb') as f:f.write(header);f.truncate(size)
            viewer_log=(root/'native-world.log').open('x')
            viewer=subprocess.Popen([str(ROOT/'working/build/world-frame/mnm-world-live'),'--root',str(root/'game'),'--channel',str(root/'world-channel.bin'),
                '--frames',str(args.live_frames),'--timeout','180','--report',str(root/'native-world.json'),'--diagnostics',str(root)],env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'},stdout=viewer_log,stderr=subprocess.STDOUT)
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(MNM_SCENE_EXPERIMENT=str(root),MNM_SCENE_SAMPLES=str(args.samples),WINEDEBUG='-all',
                   WINEPREFIX=str(root/'wineprefix'))
        env.update(MNM_SCENE_SKIP=str(args.skip_queues),MNM_SCENE_INTERVAL=str(args.interval))
        if args.world_lifetime:env['MNM_SCENE_LIFETIME']='1'
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
            started_live=time.monotonic();deadline=started_live+60
            while time.monotonic()<deadline:
                if args.world_live:
                    if viewer.poll() is not None:
                        if viewer.returncode:raise RuntimeError('Native World viewer failed; inspect '+str(root/'native-world.json'))
                        paths=[];worlds=[];break
                    deadline=max(deadline,time.monotonic()+1)
                    if process.poll() is not None:raise RuntimeError('Original exited during continuous World presentation')
                    if time.monotonic()>started_live+180:raise RuntimeError('Continuous World presentation timed out')
                    time.sleep(.05);continue
                paths=sorted((root/'capture').glob('scene-*.bin'))
                complete=[]
                for path in paths:
                    with path.open('rb') as capture:header=capture.read(64)
                    if len(header)==64 and header[:8]==b'MNMSCNE1' and path.stat().st_size==struct.unpack_from('<I',header,16)[0]:complete.append(path)
                worlds=sorted((root/'capture').glob('world-*.bin')) if args.world_frames else []
                world_complete=all(p.stat().st_size>=80 and p.stat().st_size==struct.unpack_from('<I',p.read_bytes(),16)[0] for p in worlds)
                if len(complete)>=args.samples and (not args.world_frames or len(worlds)==args.samples and world_complete):paths=complete;break
                heartbeat+=1;sequence+=2;struct.pack_into('<I',channel,16,sequence-1)
                struct.pack_into('<I',channel,24,heartbeat);struct.pack_into('<I',channel,16,sequence)
                if process.poll() is not None:raise RuntimeError('Original game exited during scene capture')
                time.sleep(.05)
            else:raise RuntimeError('Original simulation produced no complete bounded scene samples')
            subprocess.run(['import','-window','root',str(root/'original-window.png')],env=env,timeout=10,check=True)
        metadata=json.loads((root/'manifest.json').read_text())
        for name,key in [('Chaos.exe','staged_sha256'),('MnmScene.dll','scene_dll_sha256'),('MnmMenu.dll','menu_dll_sha256')]:
            if sha(root/'game'/name)!=metadata[key]:raise RuntimeError('Staged binary changed')
        if args.word_sprites:
            if sha(root/'game/MnmWord.dll')!=metadata['word_dll_sha256']:raise RuntimeError('Word adapter changed')
            raw=(root/'word-sprites/stats.bin').read_bytes()
            if not raw or len(raw)%64:raise RuntimeError('Incomplete word-sprite statistics')
            stats=struct.unpack_from('<16I',raw,len(raw)-64)
            if not stats[5] or not stats[7] or stats[11] or stats[14] or stats[15]:raise RuntimeError('Word-sprite route failed: '+str(stats))
            if args.word_sprites=='takeover' and (not stats[8] or stats[13]!=8):raise RuntimeError('No complete takeover samples')
        if {str(p.relative_to(ROOT)):sha(p) for p in sources}!=fingerprints:raise RuntimeError('Capture sources changed during validation')
        report={'success':True,'live_observation':True,'live_replacement':False,'original_pixels_compared':False,
                'scope':'Original Single Player battle setup and ordered queue consumer entry. Original simulation/drawing remain active; bounded immutable snapshots, no whole-scene visual equivalence.',
                'sources':fingerprints,'sources_stable':True,'source_executable_sha256':metadata['source_sha256'],
                'samples':args.samples,'map_selection':args.map,'menu_trace':trace,'snapshots':{str(p.relative_to(root)):sha(p) for p in paths},'experiment':str(root)}
        if args.world_live:
            native=json.loads((root/'native-world.json').read_text())
            if not native['success'] or native['presentations']!=args.live_frames or native['mismatches'] or native['remaining_surfaces'] or native['viewport_image_uploads']:raise RuntimeError('Continuous native World invariant failed')
            if args.world_live=='normal' and (native['native_readbacks'] or native['pixels_compared']):raise RuntimeError('Ordinary live rendering performed diagnostic readback')
            report.update(native_world=native,live_native_presentation=True,live_equivalence=args.world_live=='verify',original_pixels_compared=args.world_live=='verify',
                original_pixels_used_as_native_inputs=False,original_work_bypassed=False,skip_queues=args.skip_queues,queue_interval=args.interval)
            report['complete_native_admission']=native.get('capture_refusals',0)==0
            report['scope']='Continuous native World Qt shadow presentation of selected original Quick Battle requests; normal capability refusals are whole-frame diagnostics, original drawing retained, HUD and whole-scene bypass pending'
        elif args.world_frames:
            refusals=[]
            for path in worlds:
                raw=path.read_bytes()
                if raw[:8]!=b'MNMWRLD1':raise RuntimeError('Invalid World trace envelope: '+str(path))
                failure=struct.unpack_from('<I',raw,40)[0]
                if failure:
                    if not args.observe_world_refusals:raise RuntimeError('World trace refused a raster input: '+str(path))
                    refusals.append({'snapshot':str(path.relative_to(root)),'reason':failure,'captured_draws':struct.unpack_from('<I',raw,36)[0]})
            report.update(world_frames={str(p.relative_to(root)):sha(p) for p in sorted((root/'capture').glob('world-*'))},skip_queues=args.skip_queues,queue_interval=args.interval)
            if args.observe_world_refusals:report.update(world_refusals=refusals,complete_native_admission=not refusals)
        if args.world_lifetime:
            lifetime=root/'capture/canvas-lifetime.bin';raw=lifetime.read_bytes()
            if len(raw)<128 or raw[:8]!=b'MNMCLIF1' or (len(raw)-64)%64:raise RuntimeError('Incomplete World lifetime diagnostics')
            rows=list(struct.iter_unpack('<16I',raw[64:]))
            if len(rows)>256 or any(row[0]!=i+1 for i,row in enumerate(rows)):raise RuntimeError('Invalid World lifetime diagnostic sequence')
            report.update(canvas_lifetime={'path':str(lifetime.relative_to(root)),'sha256':sha(lifetime),'records':len(rows),
                'pointer_tokens':sorted({row[3] for row in rows}),'first_nonzero_pixels':rows[0][7],
                'allocation_generations_observed':False,'original_pixels_used_as_native_inputs':False})
        if args.word_sprites:report.update(word_sprites_mode=args.word_sprites,word_stats=list(stats),word_directory=metadata['word_directory'],word_scope='Partial direct-word backend entries; full scene pixels and other original drawing are outside the replacement claim')
        with (root/'report.json').open('x') as f:json.dump(report,f,indent=2);f.write('\n')
        print(root/'report.json',flush=True)
    finally:
        if viewer and viewer.poll() is None:
            viewer.terminate()
            try:viewer.wait(timeout=5)
            except subprocess.TimeoutExpired:viewer.kill();viewer.wait(timeout=5)
        if process and process.poll() is None:
            os.killpg(process.pid,signal.SIGTERM)
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
        if channel:channel.close()
        if process:
            subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
