#!/usr/bin/env python3
"""Compare bounded scene selection/events to original ANI ticks and native pixels to independent SPR rows."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
    if '--display-owned' not in sys.argv:
        subprocess.run(['xvfb-run','-a','env','QT_QPA_PLATFORM=xcb','LIBGL_ALWAYS_SOFTWARE=1',
                        'python3',str(Path(__file__).resolve()),'--display-owned'],check=True)
        return
    parent=REPO/'working/tests/sprite-scene';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(f'Native scene evidence: {out}',flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);print(p.stdout,end='',flush=True);p.check_returncode()
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe';root=REPO/'working/game-clean'
        ani=root/'Creatures/redcap.ani';spr=root/'Creatures/RedCap.spr'
        if sha(exe.read_bytes())!=HASH:raise ValueError('Executable hash differs')
        preview=REPO/'working/build/sprite-scene/mnm-sprite-scene-preview';preview_hash=sha(preview.read_bytes())
        sources=['apps/sprite-scene/scene.cpp','apps/sprite-scene/scene.hpp','apps/sprite-scene/main.cpp',
                 'reconstruction/animation/no_cd.cpp','reconstruction/animation/no_cd.hpp',
                 'assets/animation.cpp','assets/animation.hpp','renderer/sprites/sprite.cpp','renderer/blit.cpp',
                 'tools/test-sprite-scene.py','tests/animation-binary-reference.cpp']
        hashes={path:sha((REPO/path).read_bytes()) for path in sources}
        inputs={str(p.relative_to(REPO)):sha(p.read_bytes()) for p in (exe,ani,spr)}
        helper=out/'reference'
        subprocess.run(['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(REPO/'assets'),
                        '-I'+str(REPO/'reconstruction/animation'),str(REPO/'tests/animation-binary-reference.cpp'),
                        str(REPO/'reconstruction/animation/no_cd.cpp'),'-o',str(helper)],check=True,capture_output=True,text=True,timeout=60)
        original={}
        for group in (0,4):
            p=subprocess.run([str(helper),str(exe),str(ani),str(group),'64','-1'],capture_output=True,text=True,timeout=5);p.check_returncode()
            original[group]=json.loads(p.stdout);(out/f'original-sequence-{group}.json').write_text(p.stdout)
        b=spr.read_bytes();count,palettes=struct.unpack_from('<2I',b,12);base=24+768*palettes+count*4
        decoded={}
        def frame(index):
            if index in decoded:return decoded[index]
            if index<0 or index>=count:raise ValueError('SPR frame out of range')
            at=base+struct.unpack_from('<I',b,24+768*palettes+index*4)[0]
            size,w,h,ox,oy=struct.unpack_from('<IIIii',b,at);pi=struct.unpack_from('<i',b,at+28)[0]
            if at+size>len(b):raise ValueError('Frame extent')
            pixels=[]
            for y in range(h):
                delta,pixel=struct.unpack_from('<2I',b,at+40+8*y);delta+=at;pixel+=at;x=0;colour=False
                while x<w:
                    run=b[delta];delta+=1
                    if x+run>w:raise ValueError('SPR run extent')
                    if colour:
                        for dx in range(run):
                            if palettes:
                                idx=b[pixel];pixel+=1;r,g,blue=b[24+pi*768+idx*3:27+pi*768+idx*3]
                                value=(r>>3)<<11|(g>>2)<<5|(blue>>3)
                            else:value=struct.unpack_from('<H',b,pixel)[0];pixel+=2
                            pixels.append((x+dx,y,value))
                    x+=run;colour=not colour
                if delta>at+size or pixel>at+size:raise ValueError('SPR planes outside frame')
            decoded[index]=(ox,oy,pixels);return decoded[index]
        background=[0x2124 if (x//16+y//16)%2 else 0x2945 for y in range(256) for x in range(512)]
        rgba=[bytes((((p>>11)&31)*255//31,((p>>5)&63)*255//63,(p&31)*255//31,255)) for p in range(65536)]
        results=[]
        base_command=[str(preview),
                      '--root',str(root),'--ani','c:\\MagicMayhem\\cReAtUrEs\\rEdCaP.aNi','--sequences','0,4']
        for loop in (False,True):
            directory=out/('loop' if loop else 'stop')
            command=base_command+['--ticks','64','--export-dir',str(directory)]+(['--loop'] if loop else [])
            p=subprocess.run(command,capture_output=True,text=True,timeout=60);(out/f'{directory.name}.stderr').write_text(p.stderr);p.check_returncode()
            report=json.loads(p.stdout);positions={0:0,4:0};matches=[]
            if report['loop_policy']!=loop or len(report['frames'])!=65 or report['remaining_surfaces']!=0:raise ValueError('Scene bounds/lifetimes differ')
            for tick,rendered in enumerate(report['frames']):
                if rendered['tick']!=tick or len(rendered['actors'])!=2:raise ValueError('Scene trace order differs')
                expected=background.copy()
                for actor,g in zip(rendered['actors'],(0,4)):
                    if tick:
                        previous=original[g][positions[g]]
                        positions[g]=0 if loop and not previous[5] else positions[g]+1
                    oracle=original[g][positions[g]]
                    selected=oracle[2] if oracle[2]>=0 else None
                    if actor['sequence']!=g or actor['sprite']!=selected or actor['event']!=oracle[1] or actor['anchor_y']!=190:
                        raise ValueError(f'Scene selection/event differs from original at tick {tick}')
                    anchor=(170 if g==0 else 341,190)
                    if (actor['anchor_x'],actor['anchor_y'])!=anchor:raise ValueError('Scene anchor differs')
                    if selected is not None:
                        ox,oy,pixels=frame(selected)
                        for x,y,value in pixels:
                            dx,dy=anchor[0]-ox+x,anchor[1]-oy+y
                            if not 0<=dx<512 or not 0<=dy<256:raise ValueError('Independent scene placement outside viewport')
                            expected[dy*512+dx]=value
                packed=struct.pack('<'+str(len(expected))+'H',*expected);actual=(directory/f'frame-{tick:03}.565').read_bytes()
                expected_rgba=b''.join(rgba[p] for p in expected)
                if actual!=packed or rendered['native_sha256']!=sha(packed) or rendered['rgba_sha256']!=sha(expected_rgba):
                    raise ValueError(f'Scene native pixels/presentation differs at tick {tick}')
                matches.append({'tick':tick,'native_sha256':sha(actual),'rgba_sha256':sha(expected_rgba),'matched':True})
            results.append({'loop_policy':loop,'frames':65,'original_selection_event_matches':65,'independent_pixel_presentation_matches':65,
                            'uploads':report['uploads'],'copies':report['copies'],'renderer':report['renderer'],'frames_sha256':matches})
        p=subprocess.run(base_command+['--ticks','3','--interval','10','--smoke-test'],capture_output=True,text=True,timeout=20)
        (out/'window-smoke.stderr').write_text(p.stderr);p.check_returncode()
        # New-directory output must reject an existing export without altering it.
        old=sha((out/'stop/report.json').read_bytes())
        p=subprocess.run(base_command+['--ticks','1','--export-dir',str(out/'stop')],capture_output=True,text=True,timeout=20)
        (out/'existing-output.stderr').write_text(p.stderr)
        if p.returncode!=2 or sha((out/'stop/report.json').read_bytes())!=old:raise ValueError(f'Existing-output check failed: exit={p.returncode}, stderr={p.stderr}')
        if sha(preview.read_bytes())!=preview_hash or any(sha((REPO/path).read_bytes())!=digest for path,digest in inputs.items()) or any(sha((REPO/path).read_bytes())!=digest for path,digest in hashes.items()):raise ValueError('Input/source changed')
        report={'scope':'Bounded two-actor scene; explicit sequences and preview policies, not a recovered whole scene/game clock',
                'preview_sha256':preview_hash,'source_sha256':hashes,'inputs_sha256':inputs,'input_unchanged':True,
                'live_game_validated':False,'window_timer_smoke_passed':True,'existing_output_protected':True,
                'summary':{'scene_frames':130,'actor_states':260,'original_selection_event_matches':260,
                           'independent_pixel_presentation_matches':130,'distinct_sprites':len(decoded)},'runs':results}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report['summary']),flush=True)
    finally:verify('after')
if __name__=='__main__':main()
