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

from world_channel import create
from world_refusal import log_refusal

ROOT=Path(__file__).resolve().parents[1]


class CaptureCancelled(Exception):
    """An interactive window closed before the finite capture completed."""


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-game',type=Path,help='Pinned read-only source installation for an isolated capture')
    parser.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64')
    parser.add_argument('--samples',type=int,choices=[*range(1,17),32],default=4)
    parser.add_argument('--map',type=int,choices=range(1,129),default=2)
    parser.add_argument('--magic-items',type=int,choices=range(22),default=0,help='Original Quick Battle item count; nonzero accepts its initial spell assignments through the existing V3 bridge')
    parser.add_argument('--word-sprites',choices=['shadow','takeover','clip-shadow','clip-takeover'],help='Observe the explicit partial direct-word raster experiment')
    parser.add_argument('--world-frames',action='store_true',help='Capture complete effective raster inputs and separate original output oracles')
    parser.add_argument('--world-lifetime',action='store_true',help='Bounded diagnostic canvas identity/initial-content observations from first queue call')
    parser.add_argument('--producer-live',type=Path,help='Native Qt closed-producer consumer executable, starts before original launch')
    parser.add_argument('--world-producer-handoff',action='store_true',help='Use reconstructed producer history in native GPU World drawing')
    parser.add_argument('--world-raster-queue',type=int,choices=range(1,17),help='Replace every admitted raster in this selected World queue')
    parser.add_argument('--world-raster-prefix',type=int,choices=[*range(1,17),32],help='Replace every admitted raster in contiguous startup queues1..N')
    parser.add_argument('--world-raster-batch',action='store_true',help='Guard the original destination and complete selected raster queues with one native canvas reply')
    parser.add_argument('--world-producer-bypass',type=int,choices=range(1,9),help='Replace this many selected generic raster entries in the final captured queue')
    parser.add_argument('--canvas-producers',action='store_true',help='Capture owned menu/loading/font/raster inputs and completed canvases through each sampled World return')
    parser.add_argument('--producer-oracle-mib',type=int,choices=range(1,3073),default=1024,metavar='1..3072',help='Bounded original completion storage in MiB (default: 1024)')
    parser.add_argument('--skip-window-screenshot',action='store_true',help='Skip the diagnostic desktop screenshot; raw completion comparisons remain independent')
    parser.add_argument('--manual-input',action='store_true',help='Bounded producer/batch session controlled through the original window; closing either window cancels')
    parser.add_argument('--canvas-startup',action='store_true',help='Trace selected creation/lock/bind/clear/copy paths from process startup through first World queue')
    parser.add_argument('--startup-queues',type=int,choices=range(1,257),help='Retain every raw queue in this startup prefix (automatic with canvas/refusal diagnostics)')
    parser.add_argument('--kind8-objects',action='store_true',help='Finite read-only kind8 object/vtable diagnostics')
    parser.add_argument('--sample-kind8',action='store_true',help='Finite full-queue samples only when kind8 occurs; retain the startup journal')
    parser.add_argument('--claims',type=Path,help='Prospective coverage claims; sources must match before and after execution')
    parser.add_argument('--observe-world-refusals',action='store_true',help='Retain refused startup traces as diagnostics, without claiming native admission')
    parser.add_argument('--startup-replay',action='store_true',help='Finite diagnostic capture of wave kinds16/17/20 and the original lazy wave initializer return')
    parser.add_argument('--world-live',choices=['normal','verify','history-normal','history-verify'],help='Continuous Qt native World shadow presentation; optional independent pixel comparison')
    parser.add_argument('--live-frames',type=int,choices=range(1,257),default=24)
    parser.add_argument('--skip-queues',type=int,default=None,help='Wait this many queue calls before the first sample (World default: 120)')
    parser.add_argument('--interval',type=int,default=1,help='Queue calls between samples')
    parser.add_argument('--minimap-owned',action='store_true',help='Observe owned four-view minimap inputs in producer journal V2; original drawing stays active')
    parser.add_argument('--minimap-input-fixture',type=Path,help='Private-Xvfb XTest schedule executable; owned minimap only, 250ms pacing between first16 World calls')
    args=parser.parse_args()
    if args.minimap_input_fixture and (not args.minimap_owned or args.samples!=16 or args.magic_items):parser.error('Minimap input fixture requires --minimap-owned --samples 16 --magic-items 0')
    if args.minimap_owned and (not args.canvas_producers or args.world_producer_bypass or args.world_raster_queue or args.world_raster_prefix):parser.error('--minimap-owned requires original-active --canvas-producers')
    if args.producer_oracle_mib!=1024 and not args.canvas_producers:parser.error('--producer-oracle-mib requires --canvas-producers')
    if args.producer_live and not args.canvas_producers:parser.error('--producer-live requires --canvas-producers')
    if args.world_producer_handoff and not args.producer_live:parser.error('--world-producer-handoff requires --producer-live')
    if args.world_producer_bypass and not args.world_producer_handoff:parser.error('--world-producer-bypass requires --world-producer-handoff')
    full_rasters=args.world_raster_queue or args.world_raster_prefix
    if full_rasters and (not args.world_producer_handoff or args.world_producer_bypass or (args.world_raster_queue and args.world_raster_prefix) or full_rasters>args.samples):parser.error('Complete raster queues require handoff, valid sample bounds and a separate bypass mode')
    if args.samples>16 and (not args.world_raster_batch or args.world_raster_prefix!=args.samples or args.minimap_owned or args.magic_items or args.map!=2):parser.error('Extended32 samples require guarded prefix32, map2 and zero items')
    if args.world_raster_batch and not full_rasters:parser.error('--world-raster-batch requires selected complete raster queues')
    if args.manual_input and (not args.world_raster_batch or args.samples not in (16,32) or args.world_raster_prefix!=args.samples or args.magic_items):parser.error('Manual input requires the bounded first16/32 World batch route with zero-items fixture')
    history=args.world_live in ('history-normal','history-verify')
    verify=args.world_live in ('verify','history-verify')
    if args.canvas_producers:
        if args.world_frames or args.world_live or args.canvas_startup or args.word_sprites:parser.error('Producer capture is an isolated finite pipeline; do not combine World/startup hooks')
        if args.startup_queues is None:args.startup_queues=args.samples
    if args.world_live:args.world_frames=True
    if history:
        if args.live_frames>16:parser.error('History target must be 1..16; specify --live-frames 16')
        if args.skip_queues not in (None,0) or args.interval!=1:parser.error('History requires --skip-queues 0 --interval 1')
        args.skip_queues=0;args.startup_replay=True
        if args.startup_queues is None:args.startup_queues=args.live_frames
        if args.startup_queues!=args.live_frames:parser.error('History journal must match the live target')
    if args.observe_world_refusals and (not args.world_frames or not args.world_lifetime or args.world_live):parser.error('Refusal diagnostics require finite --world-frames --world-lifetime')
    if args.world_frames and args.word_sprites:parser.error('Use separate World and word takeover experiments')
    if args.startup_replay:
        if not args.world_frames or (args.world_live and not history):parser.error('Startup replay requires finite --world-frames')
        if args.startup_queues is None and not history:args.startup_queues=args.samples
    if args.canvas_startup or args.observe_world_refusals:
        if args.startup_queues is None:args.startup_queues=args.samples
    if args.kind8_objects and args.startup_queues is None:args.startup_queues=256
    if args.sample_kind8 and (not args.kind8_objects or not args.world_frames or args.world_live):parser.error('--sample-kind8 requires finite --kind8-objects --world-frames')
    if args.startup_queues:
        if args.world_live and not history:parser.error('Startup queue journals require a finite capture or bounded live history')
        if args.skip_queues not in (None,0) or args.interval!=1:parser.error('Startup capture requires --skip-queues 0 --interval 1')
        if not history and args.startup_queues<args.samples:parser.error('Startup prefix must cover every requested sample')
        args.skip_queues=0
    if args.skip_queues is None:args.skip_queues=120 if args.world_frames else 0
    if not 0<=args.skip_queues<=3600 or not 1<=args.interval<=3600:parser.error('Queue sample bounds exceeded')
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a -s "-screen 0 1280x1024x24"')
    sources=[*sorted((ROOT/'runtime/scene').glob('*.[chS]')),ROOT/'protocols/include/mnm/scene_snapshot_v1.h',
             ROOT/'runtime/shadow/win32_min.h',ROOT/'tools/build-scene-observer.py',ROOT/'tools/prepare-scene-observer.py',ROOT/'tools/capture-scene-game.py',ROOT/'tools/scene-game-runner.py',ROOT/'protocols/include/mnm/world_frame_v1.h',ROOT/'protocols/include/mnm/world_channel_v1.h',ROOT/'protocols/include/mnm/world_channel_v2.h',ROOT/'tools/world_channel.py']
    if args.startup_queues:sources.append(ROOT/'tools/inspect-startup-queues.py')
    if args.kind8_objects:sources.append(ROOT/'tools/inspect-kind8.py')
    if history:sources.append(ROOT/'tools/world_refusal.py')
    if args.magic_items:
        sources += [*sorted((ROOT/'runtime/menu').glob('*.[chS]')),ROOT/'tools/build-menu-observer.py',
                    *[ROOT/('protocols/include/mnm/menu_v'+str(v)+'.h') for v in (1,2,3)]]
    if args.canvas_producers:sources += [ROOT/'tools/inspect-canvas-producers.py',ROOT/'protocols/include/mnm/canvas_producers_v1.h',ROOT/'protocols/include/mnm/canvas_producers_v2.h',ROOT/'protocols/include/mnm/canvas_producers_v3.h']
    claims=json.loads(args.claims.read_text()) if args.claims else None
    if claims:
        for source,digest in claims['sources'].items():
            path=ROOT/source
            if path.resolve().is_relative_to(ROOT) and sha(path)==digest:sources.append(path)
            else:raise RuntimeError('Coverage declaration source changed: '+source)
    sources=sorted(set(sources))
    if args.world_live:
        for directory in ['assets','renderer','compat/legacy']:
            sources += [p for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ['.cpp','.hpp','.c','.h','.S'] or p.name in ['CMakeLists.txt','README.md'])]
        sources += [ROOT/p for p in ['apps/qt-shell/live_world_session.cpp','apps/qt-shell/live_world_session.hpp','apps/qt-shell/world_live_main.cpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp']]
    if args.producer_live:
        sources += [ROOT/p for p in ['apps/qt-shell/canvas_producer_session.cpp','apps/qt-shell/canvas_producer_session.hpp','apps/qt-shell/canvas_producer_main.cpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp','compat/legacy/canvas_producers.cpp','compat/legacy/canvas_producers.hpp','renderer/canvas_sequence.cpp','renderer/canvas_sequence.hpp','renderer/dib.cpp','renderer/dib.hpp']]
    if args.world_raster_batch:
        sources += [ROOT/p for p in ['protocols/include/mnm/world_raster_batch_v3.h','compat/legacy/world_raster_batch.cpp','compat/legacy/world_raster_batch.hpp']]
    if args.word_sprites:sources += [ROOT/p for p in ['renderer/sprites/word_raster.h','renderer/sprites/word_raster.c','runtime/shadow/win32_min.h','tools/build-word-sprites.py','tools/build-shadow-bridge.py','renderer/sprites/word_clipped.c','renderer/sprites/word_clipped.h','reconstruction/rendering/word_backend_state.c','reconstruction/rendering/word_backend_state.h']]
    fingerprints={str(p.relative_to(ROOT)):sha(p) for p in sources}
    spec=importlib.util.spec_from_file_location('scene_prepare',ROOT/'tools/prepare-scene-observer.py')
    staging=importlib.util.module_from_spec(spec);spec.loader.exec_module(staging)
    process=None;viewer=None;input_fixture=None;channel=None;trace=[];report=None;root=None
    def stop_original():
        nonlocal process
        if process and process.poll() is None:
            os.killpg(process.pid,signal.SIGTERM)
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
        if process:
            subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
            process=None
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        root=staging.prepare(menu=True,word_mode=args.word_sprites,world_frames=args.world_frames,world_live=bool(args.world_live),source_game=args.source_game);print(root,flush=True)
        if args.canvas_producers:
            policy={'producer_oracle_mib':args.producer_oracle_mib,'window_screenshot_requested':not args.skip_window_screenshot}
            (root/'capture-policy.json').write_text(json.dumps(policy,indent=2)+'\n')
        if args.producer_live:
            viewer_log=(root/'native-producers.log').open('x')
            viewer=subprocess.Popen([str(args.producer_live.resolve()),str(root/'capture/canvas-producers.bin'),str(root/'game'),str(root/'native-producers'),*(['--world-handoff'] if args.world_producer_handoff else []),*(['--world-batch'] if args.world_raster_batch else [])],env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'},stdout=viewer_log,stderr=subprocess.STDOUT)
        if args.world_live:
            create(root/'world-channel.bin',verify,args.live_frames if history else 0)
            viewer_log=(root/'native-world.log').open('x')
            viewer=subprocess.Popen([str(ROOT/'working/build/world-frame/mnm-world-live'),'--root',str(root/'game'),'--channel',str(root/'world-channel.bin'),
                '--frames',str(args.live_frames),'--timeout','600' if history else '180','--report',str(root/'native-world.json'),'--diagnostics',str(root)],env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'},stdout=viewer_log,stderr=subprocess.STDOUT)
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(MNM_SCENE_EXPERIMENT=str(root),MNM_SCENE_SAMPLES=str(args.samples),WINEDEBUG='-all',
                   WINEPREFIX=str(root/'wineprefix'))
        env.update(MNM_SCENE_SKIP=str(args.skip_queues),MNM_SCENE_INTERVAL=str(args.interval))
        if args.world_raster_queue:env['MNM_WORLD_RASTER_QUEUE']=str(args.world_raster_queue)
        if args.world_raster_prefix:env['MNM_WORLD_RASTER_PREFIX']=str(args.world_raster_prefix)
        if args.world_raster_batch:env['MNM_WORLD_RASTER_BATCH']='1'
        if args.world_lifetime:env['MNM_SCENE_LIFETIME']='1'
        if args.canvas_startup:env['MNM_CANVAS_STARTUP']='1'
        if args.minimap_owned:env['MNM_MINIMAP_OWNED']='1'
        if args.minimap_input_fixture:env['MNM_MINIMAP_INPUT_FIXTURE']='1'
        if args.canvas_producers:env.update(MNM_CANVAS_PRODUCERS='1',MNM_CANVAS_ORACLE_MIB=str(args.producer_oracle_mib))
        if args.world_producer_bypass:env['MNM_WORLD_PRODUCER_BYPASS']=str(args.world_producer_bypass)
        if args.startup_queues:env['MNM_STARTUP_QUEUES']=str(args.startup_queues)
        if args.kind8_objects:env['MNM_KIND8_QUEUES']=str(3600 if args.sample_kind8 else args.startup_queues)
        if args.sample_kind8:env['MNM_SCENE_KIND8_SAMPLES']='1'
        if args.startup_replay:env['MNM_SCENE_STARTUP_REPLAY']='1'
        env.pop('WAYLAND_DISPLAY',None)
        subprocess.run(['cp','-a','--reflink=auto',str(args.prefix_template.resolve()),env['WINEPREFIX']],check=True)
        with (root/'wineboot.log').open('x') as log:
            subprocess.run(['wineboot','-u'],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=90,check=True)
        menu_version=3 if args.magic_items else 2;menu_size=65536 if args.magic_items else 32768
        raw=bytearray(menu_size);raw[:16]=(b'MNMMCMD3' if args.magic_items else b'MNMMCMD2')+struct.pack('<II',menu_version,menu_size);struct.pack_into('<III',raw,16,2,1,1)
        with (root/'channel.bin').open('xb') as f:f.write(raw)
        channel_file=(root/'channel.bin').open('r+b');channel=mmap.mmap(channel_file.fileno(),0)
        heartbeat=1;request=0;sequence=2

        def state():
            before=struct.unpack_from('<I',channel,128)[0]
            row=struct.unpack_from('<7I',channel,132)
            payload=bytes(channel[160:16924])
            slots=struct.unpack_from('<63I',channel,18028) if args.magic_items else None
            return (row,payload,slots) if not before&1 and before==struct.unpack_from('<I',channel,128)[0] else None

        def beat(action=0,generation=0,argument=0,rules=None,slots=None):
            nonlocal heartbeat,request,sequence
            heartbeat+=1;sequence+=2
            if action:request+=1
            struct.pack_into('<I',channel,16,sequence-1)
            struct.pack_into('<6I',channel,20,1,heartbeat,request,action,generation,argument)
            if rules is not None:struct.pack_into('<17I',channel,44,*rules)
            if slots is not None:struct.pack_into('<63I',channel,40000,*slots)
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

        def send(screen,action,argument=0,rules=None,slots=None):
            s=wait(screen);beat(action,s[0][0],argument,rules,slots);reply=wait(ack=request)
            if reply[0][4]!=1:raise RuntimeError('Original menu request refused: '+str(reply[0]))
            trace.append({'screen':screen,'action':action,'argument':argument,'ack':request,'status':reply[0][4]})

        with (root/'game.log').open('x') as log:
            process=subprocess.Popen([str(ROOT/'tools/run-game.sh'),'launch','--no-gamescope','--prefix',env['WINEPREFIX'],
                '--runner',str(ROOT/'tools/scene-game-runner.py')],env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            if not args.manual_input:
                send(3,1);send(22,4)
                s=wait(14);rules=list(struct.unpack_from('<13I',s[1],8))
                rules+= [struct.unpack_from('<I',s[1],60+i*48+12)[0] for i in range(4)]
                rules[2]=args.magic_items # Zero retains the established no-spell fixture.
                send(14,6,rules=rules);send(25,9,args.map)
                if args.minimap_input_fixture:
                    input_fixture=subprocess.Popen([str(args.minimap_input_fixture.resolve()),str(root/'capture'),str(root/'minimap-input.json')],env={**os.environ,'QT_QPA_PLATFORM':'xcb'},stdout=(root/'minimap-input.log').open('x'),stderr=subprocess.STDOUT)
                send(14,7,rules=rules)
                if args.magic_items:send(7,12,slots=wait(7)[2])
            started_live=time.monotonic();deadline=started_live+(3600 if full_rasters else 300 if args.world_producer_handoff else 60)
            while time.monotonic()<deadline:
                if args.world_live:
                    if viewer.poll() is not None:
                        if viewer.returncode:raise RuntimeError('Native World viewer failed; inspect '+str(root/'native-world.json'))
                        paths=[];worlds=[];break
                    deadline=max(deadline,time.monotonic()+1)
                    if process.poll() is not None:raise RuntimeError('Original exited during continuous World presentation')
                    if time.monotonic()>started_live+(600 if history else 180):raise RuntimeError('Continuous World presentation timed out')
                    time.sleep(.05);continue
                paths=sorted((root/'capture').glob('scene-*.bin'))
                complete=[]
                for path in paths:
                    with path.open('rb') as capture:header=capture.read(64)
                    if len(header)==64 and header[:8]==b'MNMSCNE1' and path.stat().st_size==struct.unpack_from('<I',header,16)[0]:complete.append(path)
                worlds=sorted((root/'capture').glob('world-*.bin')) if args.world_frames else []
                world_complete=all(p.stat().st_size>=80 and p.stat().st_size==struct.unpack_from('<I',p.read_bytes(),16)[0] for p in worlds)
                if args.producer_live and viewer.poll() is not None and viewer.returncode:raise RuntimeError('Native producer viewer failed; inspect '+str(root/'native-producers/live-report.json'))
                if args.manual_input and viewer.poll()==0 and not (root/'capture/canvas-producers.done').exists():raise CaptureCancelled('Native window closed')
                producer_complete=not args.producer_live or viewer.poll()==0
                startup_complete=True
                if args.startup_queues:
                    queue_paths=sorted((root/'capture').glob('startup-queue-*.bin'))
                    startup_complete=len(queue_paths)==args.startup_queues and all(p.stat().st_size>=64 and p.stat().st_size==struct.unpack_from('<I',p.read_bytes(),16)[0] for p in queue_paths)
                if len(complete)>=args.samples and startup_complete and producer_complete and (not args.canvas_producers or (root/'capture/canvas-producers.done').exists()) and (not args.world_frames or len(worlds)==args.samples and world_complete):paths=complete;break
                heartbeat+=1;sequence+=2;struct.pack_into('<I',channel,16,sequence-1)
                struct.pack_into('<I',channel,24,heartbeat);struct.pack_into('<I',channel,16,sequence)
                if process.poll() is not None:
                    if args.manual_input and not list((root/'capture').glob('world-batch-fault-*.bin')):
                        if viewer.poll() is None:time.sleep(.1)
                        if viewer.poll() not in (None,0):raise RuntimeError('Native producer viewer refused the interactive stream')
                        # Producer failure markers distinguish engine refusal from user closure.
                        marker=root/'capture/canvas-producers.done'
                        if marker.exists() and (len(marker.read_bytes())!=32 or struct.unpack_from('<I',marker.read_bytes(),20)[0]):raise RuntimeError('Original producer refused the interactive stream')
                        raise CaptureCancelled('Original window closed')
                    raise RuntimeError('Original game exited during scene capture')
                time.sleep(.05)
            else:raise RuntimeError('Original simulation produced no complete bounded scene samples')
            if not args.skip_window_screenshot:
                subprocess.run(['import','-window','root',str(root/'original-window.png')],env=env,timeout=10,check=True)
        if input_fixture:
            if input_fixture.wait(timeout=5):raise RuntimeError('Minimap input schedule failed; inspect '+str(root/'minimap-input.json'))
        # Freeze lifetime diagnostics before recording their final hashes.
        stop_original()
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
            if args.word_sprites in ('clip-shadow','clip-takeover'):
                raw_clip=(root/'word-sprites/clip-stats.bin').read_bytes()
                if not raw_clip or len(raw_clip)%80:raise RuntimeError('Incomplete clipping shadow statistics')
                clip_stats=struct.unpack_from('<20I',raw_clip,len(raw_clip)-80)
                if clip_stats[:5]!=(0x574d4e4d,0x31304c43 if args.word_sprites=='clip-shadow' else 0x31304b54,1,80,1) or not clip_stats[7] or clip_stats[12] or clip_stats[13] or clip_stats[15] or (stats[8] if args.word_sprites=='clip-shadow' else not stats[8] or stats[10] or clip_stats[18]) or clip_stats[14]!=8:
                    raise RuntimeError('Clipping shadow failed or no eight clipped samples: '+str(clip_stats))
                raw_admission=(root/'word-sprites/admission-stats.bin').read_bytes()
                if not raw_admission or len(raw_admission)%128:raise RuntimeError('Incomplete word admission statistics')
                admission_stats=struct.unpack_from('<32I',raw_admission,len(raw_admission)-128)
                if admission_stats[:4]!=(0x574d4e4d,0x31304441 if args.word_sprites=='clip-shadow' else 0x31305441,1,128) or not admission_stats[15] or not admission_stats[20]:raise RuntimeError('No auxiliary clipped shadow coverage')
                sample_frames=[];clipped_samples=0
                for sample_path in sorted((root/'word-sprites').glob('word-*.bin')):
                    captured=sample_path.read_bytes();frame_bytes=struct.unpack_from('<I',captured,32)[0]
                    if not any(struct.unpack_from('<II',captured,240)):raise RuntimeError('Expected auxiliary-bearing clipped sample')
                    fw,fh,ox,oy=struct.unpack_from('<IIii',captured,212)
                    ax,ay=struct.unpack_from('<ii',captured,52);right,bottom=struct.unpack_from('<II',captured,40)
                    left,top=struct.unpack_from('<ii',captured,64)
                    clipped_samples+=ax-ox<left or ay-oy<top or ax-ox+fw>=right or ay-oy+fh>=bottom
                    sample_frames.append(hashlib.sha256(captured[208:208+frame_bytes]).hexdigest())
                if len(set(sample_frames))<4 or clipped_samples!=2 or clip_stats[17]!=2:raise RuntimeError('Expected distinct inputs with two clipped and six interior samples')
        if {str(p.relative_to(ROOT)):sha(p) for p in sources}!=fingerprints:raise RuntimeError('Capture sources changed during validation')
        report={'success':True,'live_observation':True,'live_replacement':False,'original_pixels_compared':False,
                'scope':'Original Single Player battle setup and ordered queue consumer entry. Original simulation/drawing remain active; bounded immutable snapshots, no whole-scene visual equivalence.',
                'sources':fingerprints,'sources_stable':True,'source_executable_sha256':metadata['source_sha256'],
                'samples':args.samples,'map_selection':None if args.manual_input else args.map,'manual_input':args.manual_input,'magic_items':None if args.manual_input else args.magic_items,'menu_trace':trace,'snapshots':{str(p.relative_to(root)):sha(p) for p in paths},'experiment':str(root)}
        if claims:report['claims']=claims['claims']
        report['window_screenshot_requested']=not args.skip_window_screenshot
        if args.canvas_producers:report['producer_oracle_mib']=args.producer_oracle_mib
        report['startup_replay']=args.startup_replay
        if args.startup_queues:
            spec=importlib.util.spec_from_file_location('startup_queues',ROOT/'tools/inspect-startup-queues.py')
            inspector=importlib.util.module_from_spec(spec);spec.loader.exec_module(inspector)
            report['startup_queues']=inspector.collect(root/'capture',args.startup_queues,args.startup_replay,32 if args.samples==32 and args.world_raster_batch else 16)
        if args.world_live:
            native=json.loads((root/'native-world.json').read_text())
            if not native['success'] or native['presentations']!=args.live_frames or native['mismatches'] or native['remaining_surfaces'] or native['viewport_image_uploads']:raise RuntimeError('Continuous native World invariant failed')
            if not verify and (native['native_readbacks'] or native['pixels_compared']):raise RuntimeError('Ordinary live rendering performed diagnostic readback')
            report.update(native_world=native,live_native_presentation=True,live_equivalence=verify,original_pixels_compared=verify,
                original_pixels_used_as_native_inputs=False,original_work_bypassed=False,skip_queues=args.skip_queues,queue_interval=args.interval)
            if history:
                frames=native['frames']
                if ([f['source_queue'] for f in frames]!=list(range(1,args.live_frames+1)) or
                        not native['history'] or native['target_frames']!=args.live_frames or
                        native['dropped'] or native['superseded'] or native.get('capture_refusals',0)):
                    raise RuntimeError('Incomplete native history prefix')
                times=[f['frame_processing_ms'] for f in frames]
                report['processing_latency_ms']={'cold_first':times[0],'warm_min':min(times[1:] or times),'warm_max':max(times[1:] or times),'per_frame':times}
            report['complete_native_admission']=native.get('capture_refusals',0)==0
            report['scope']='Continuous native World Qt shadow presentation of selected original Quick Battle requests; normal capability refusals are whole-frame diagnostics, original drawing retained, HUD and whole-scene bypass pending'
            if history:report['scope']='Live native Qt presentation of every first contiguous Quick Battle World queue with owned native zero history, asynchronous CPU/GPU preparation and optional exact original post-consumer comparison; original drawing retained, HUD/init gaps excluded, no bypass'
        elif args.world_frames:
            refusals=[]
            for path in worlds:
                raw=path.read_bytes()
                if raw[:8]!=b'MNMWRLD1':raise RuntimeError('Invalid World trace envelope: '+str(path))
                failure=struct.unpack_from('<I',raw,40)[0]
                if failure:
                    if not args.observe_world_refusals:raise RuntimeError('World trace refused a raster input: '+str(path))
                    refusal={'snapshot':str(path.relative_to(root)),'reason':failure,'captured_draws':struct.unpack_from('<I',raw,36)[0]}
                    if args.startup_queues:
                        sample=struct.unpack_from('<I',raw,20)[0]
                        queues=[q for q in report['startup_queues']['queues'] if q['sample']==sample]
                        if len(queues)!=1:
                            if not args.sample_kind8:raise RuntimeError('Refused World sample lacks startup queue correlation')
                            refusal['startup_prefix_exhausted']=True
                        else:
                            refusal.update(queue=queues[0]['queue'],queue_draw_count=queues[0]['draw_count'],kind_counts=queues[0]['kind_counts'],unsupported_draws=queues[0]['unsupported_draws'])
                            if failure==8 and not refusal['unsupported_draws']:raise RuntimeError('Kind refusal lacks the actual unsupported draw kinds')
                    refusals.append(refusal)
            report.update(world_frames={str(p.relative_to(root)):sha(p) for p in sorted((root/'capture').glob('world-*'))},skip_queues=args.skip_queues,queue_interval=args.interval)
            if args.observe_world_refusals:report.update(world_refusals=refusals,complete_native_admission=not refusals)
        if args.canvas_producers:
            spec=importlib.util.spec_from_file_location('producer_inspector',ROOT/'tools/inspect-canvas-producers.py')
            inspector=importlib.util.module_from_spec(spec);spec.loader.exec_module(inspector)
            report['canvas_producers']=inspector.analyze(root/'capture')
            if args.producer_live:
                native=json.loads((root/'native-producers/live-report.json').read_text())
                checks=native['checkpoints'];original=report['canvas_producers']['checkpoints']
                if (not native['success'] or native['queues']!=args.samples or native['records']!=report['canvas_producers']['records'] or native['remaining_surfaces'] or native['viewport_image_uploads'] or len(checks)!=len(original)):
                    raise RuntimeError('Incomplete native producer presentation')
                if args.world_producer_handoff and (not native.get('world_handoff') or native.get('world_queues')!=args.samples or not all(f['gpu_world_equal'] for f in native['world_frames'])):
                    raise RuntimeError('Incomplete native producer/World handoff')
                if args.world_producer_bypass and native.get('native_bypass_count')!=args.world_producer_bypass:
                    raise RuntimeError('Bounded native World bypass did not complete every selected request')
                if full_rasters:
                    selected=set(range(1,args.world_raster_prefix+1)) if args.world_raster_prefix else {args.world_raster_queue}
                    _,operations=inspector.decode((root/'capture/canvas-producers.bin').read_bytes());active=0;expected=[]
                    for row,_ in operations:
                        if row[2]==11:active=row[14]
                        elif row[2]==12:active=0
                        elif active in selected and row[2]==9:expected.append(row[1])
                    replies=native['native_bypass_replies']
                    if not native.get('complete_raster_queues') or not expected or [row['sequence'] for row in replies]!=expected:raise RuntimeError('Incomplete complete-queue native raster replacement')
                    report['complete_raster_queues']=sorted(selected)
                    if args.world_raster_batch:
                        batches=native.get('native_batch_replies',[])
                        returns={r[14]:r for r,_ in operations if r[2]==12 and r[14] in selected}
                        if (not native.get('world_raster_batch') or [b['queue'] for b in batches]!=sorted(selected) or native.get('native_canvas_writebacks')!=len(selected) or native.get('world_readbacks')!=args.samples):raise RuntimeError('Incomplete one-canvas World batch replacement')
                        for i,b in enumerate(batches,1):
                            r=returns[b['queue']]
                            if r[17]!=i or r[19]!=b['rasters'] or r[15]!=b['cumulative_rasters'] or r[20]!=1 or not b['guarded']:raise RuntimeError('World batch return/guard/count changed')
                        if list((root/'capture').glob('world-raster-*.request')):raise RuntimeError('Batch mode emitted a per-raster handshake')
                        report.update(world_raster_batch=True,canvas_guard_verified=True,native_canvas_writebacks=len(batches),capture_elapsed_seconds=time.monotonic()-started_live)
                compared=[]
                for old,new in zip(original,checks):
                    if (old['sequence'],old['oracle'],old['canvas'])!=(new['sequence'],new['oracle'],new['canvas']) or not new['gpu_equal']:raise RuntimeError('Native producer completion identity changed')
                    expected=(root/'capture'/old['path']).read_bytes();actual=(root/'native-producers'/new['path']).read_bytes()
                    if len(actual)!=len(expected):raise RuntimeError('Native producer extent changed')
                    differences=sum(x!=y for x,y in zip(struct.iter_unpack('<H',actual),struct.iter_unpack('<H',expected))) if expected!=actual else 0
                    compared.append({'sequence':old['sequence'],'oracle':old['oracle'],'canvas':old['canvas'],'pixels':old['width']*old['height'],'mismatches':differences})
                report.update(native_producers=native,producer_comparison=compared,producer_live_binary_sha256=sha(args.producer_live.resolve()),original_pixels_compared=True,original_pixels_used_as_native_inputs=False,original_work_bypassed=False)
                report['success']=all(c['mismatches']==0 for c in compared)
                report['scope']='Live native Qt retained producer history, closed source inputs only, all observed completed canvases compared to separate original outputs; original simulation and rendering remain active, no bypass'
                if args.world_producer_handoff:report['scope']='Live native startup/HUD history handed to GPU World drawing at each queue, GPU output committed back to native producer storage; no original destination seeds'
                if full_rasters:
                    report.update(original_work_bypassed=True,live_replacement=True,bypassed_rasters=native['native_bypass_count'],original_oracle_policy='All admitted raster bodies in selected queues bypassed; independent exact-original checks required for every intermediate native canvas')
                    if args.world_raster_batch:report['original_oracle_policy']='One guarded native canvas writeback per selected World queue; post-bypass checkpoints contain native work, independent exact-entry intermediate and AX checks still required'
                if args.world_producer_bypass:
                    report.update(original_work_bypassed=True,live_replacement=True,bypassed_rasters=args.world_producer_bypass,original_oracle_policy='After native bypass, observed original destination contains native contribution; independent original replay required for equivalence')

        if args.world_lifetime:
            lifetime=root/'capture/canvas-lifetime.bin';raw=lifetime.read_bytes()
            if len(raw)<128 or raw[:8]!=b'MNMCLIF1' or (len(raw)-64)%64:raise RuntimeError('Incomplete World lifetime diagnostics')
            rows=list(struct.iter_unpack('<16I',raw[64:]))
            if len(rows)>256 or any(row[0]!=i+1 for i,row in enumerate(rows)):raise RuntimeError('Invalid World lifetime diagnostic sequence')
            report.update(canvas_lifetime={'path':str(lifetime.relative_to(root)),'sha256':sha(lifetime),'records':len(rows),
                'pointer_tokens':sorted({row[3] for row in rows}),'first_nonzero_pixels':rows[0][7],
                'allocation_generations_observed':False,'original_pixels_used_as_native_inputs':False})
        if args.canvas_startup:
            startup=root/'capture/canvas-startup.bin';raw=startup.read_bytes()
            if len(raw)<320 or raw[:8]!=b'MNMCST01' or (len(raw)-160)%160:raise RuntimeError('Incomplete canvas startup diagnostics')
            rows=list(struct.iter_unpack('<40I',raw[160:]))
            if len(rows)>16384 or any(row[0]!=i+1 for i,row in enumerate(rows)) or rows[-1][1]!=12 or any(row[1]==13 for row in rows):raise RuntimeError('Canvas startup trace truncated or missing first World boundary')
            report.update(canvas_startup={'path':str(startup.relative_to(root)),'sha256':sha(startup),'records':len(rows),
                'first_world_nonzero_pixels':rows[-1][31],'first_world_content_fnv1a':rows[-1][32],
                'original_pixels_used_as_native_inputs':False,'complete_writer_census':False})
        if args.kind8_objects:
            spec=importlib.util.spec_from_file_location('kind8',ROOT/'tools/inspect-kind8.py');kind8=importlib.util.module_from_spec(spec);spec.loader.exec_module(kind8)
            observed=kind8.collect(root/'capture')
            report['kind8_objects']={'files':{str(p.relative_to(root)):sha(p) for p in sorted((root/'capture').glob('kind8-*.bin'))},'classes':observed['classes'],'rows':sum(q['captured'] for q in observed['records']),'queues':len(observed['records'])}
        if args.minimap_input_fixture:
            report['minimap_input_fixture']=json.loads((root/'minimap-input.json').read_text())
            report['minimap_input_fixture']['executable_sha256']=sha(args.minimap_input_fixture)
            report['minimap_input_fixture']['world_return_pace_ms']=250
        if args.word_sprites in ('clip-shadow','clip-takeover'):report.update(word_admission_stats=list(admission_stats),word_admission_stats_sha256=sha(root/'word-sprites/admission-stats.bin'),distinct_word_frames=len(set(sample_frames)),word_clipped_sample_count=clipped_samples,word_clip_stats=list(clip_stats),word_clip_stats_sha256=sha(root/'word-sprites/clip-stats.bin'),word_clipped_samples={str(p.relative_to(root)):sha(p) for p in sorted((root/'word-sprites').glob('word-*.bin'))})
        if args.word_sprites:report.update(word_sprites_mode=args.word_sprites,word_stats=list(stats),word_directory=metadata['word_directory'],word_scope='Partial direct-word backend entries; full scene pixels and other original drawing are outside the replacement claim')
    except CaptureCancelled as cancelled:
        report={'success':False,'cancelled':True,'reason':str(cancelled),'experiment':str(root),
                'manual_input':True,'sources':fingerprints,'original_pixels_used_as_native_inputs':False,
                'scope':'Interactive window closure before the bounded producer chain completed; no equivalence or replacement validation asserted'}
    finally:
        if input_fixture and input_fixture.poll() is None:
            input_fixture.terminate();input_fixture.wait(timeout=5)
        if viewer and viewer.poll() is None:
            viewer.terminate()
            try:viewer.wait(timeout=5)
            except subprocess.TimeoutExpired:viewer.kill();viewer.wait(timeout=5)
        stop_original()
        if channel:channel.close()
        if history and root is not None:log_refusal(root)
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    if report is not None:
        report['original_manifest_verified_before_after']=True
        with (root/'report.json').open('x') as f:json.dump(report,f,indent=2);f.write('\n')
        print(root/'report.json',flush=True)


if __name__=='__main__':main()
