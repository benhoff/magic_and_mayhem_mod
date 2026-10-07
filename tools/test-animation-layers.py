#!/usr/bin/env python3
"""Original ANI selection plus independent multi-asset placement/pixel oracle."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def digest(data):return hashlib.sha256(data).hexdigest()
def main():
    if '--display-owned' not in sys.argv:
        subprocess.run(['xvfb-run','-a','env','QT_QPA_PLATFORM=xcb','LIBGL_ALWAYS_SOFTWARE=1','python3',str(Path(__file__).resolve()),'--display-owned'],check=True);return
    parent=REPO/'working/tests/animation-layers';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        root=REPO/'working/game-clean';exe=REPO/'working/game-nocd/Chaos.exe'
        if sha(exe)!=HASH:raise ValueError('Unsupported executable')
        preview=REPO/'working/build/sprite-scene/mnm-sprite-scene-preview'
        sources=['apps/sprite-scene/main.cpp','apps/sprite-scene/scene.cpp','apps/sprite-scene/scene.hpp',
                 'reconstruction/animation/no_cd.cpp','reconstruction/animation/no_cd.hpp','reconstruction/animation/placement.cpp','reconstruction/animation/placement.hpp','reconstruction/animation/attachment.cpp','reconstruction/animation/attachment.hpp',
                 'tests/animation-binary-reference.cpp','tools/test-animation-layers.py']
        inputs={p:sha(p) for p in [exe,preview]+[REPO/p for p in sources]};helper=out/'reference'
        subprocess.run(['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(REPO/'assets'),'-I'+str(REPO/'reconstruction/animation'),
                        str(REPO/'tests/animation-binary-reference.cpp'),str(REPO/'reconstruction/animation/no_cd.cpp'),'-o',str(helper)],check=True,capture_output=True,timeout=60)
        names=[('Creatures/redcap.ani','Creatures/RedCap.spr'),('Creatures/bat.ani','Creatures/Bat.spr'),('Creatures/eye.ani','Creatures/eye.spr')]
        assets=[]
        for ani,spr in names:
            ap,sp=root/ani,root/spr;inputs[ap]=sha(ap);inputs[sp]=sha(sp)
            b=ap.read_bytes();n=struct.unpack_from('<I',b,20)[0]
            assets.append({'ani':ani,'spr':spr,'records':b,'starts':struct.unpack_from('<'+str(n)+'I',b,44),'base':44+4*n,'sprite':sp.read_bytes(),'frames':{}})
        def record(asset,sequence,row):return struct.unpack_from('<11i',asset['records'],asset['base']+(asset['starts'][sequence]+row[4])*44)
        def frame(asset,index):
            if index in asset['frames']:return asset['frames'][index]
            b=asset['sprite'];count,palettes=struct.unpack_from('<II',b,12);base=24+768*palettes+4*count
            if not 0<=index<count:raise ValueError('SPR index extent')
            at=base+struct.unpack_from('<I',b,24+768*palettes+index*4)[0];size,w,h,ox,oy=struct.unpack_from('<IIIii',b,at);pi=struct.unpack_from('<i',b,at+28)[0];pixels=[]
            if at+size>len(b):raise ValueError('SPR frame extent')
            for y in range(h):
                delta,pixel=struct.unpack_from('<II',b,at+40+8*y);delta+=at;pixel+=at;x=0;colour=False
                while x<w:
                    run=b[delta];delta+=1
                    if x+run>w:raise ValueError('SPR run extent')
                    if colour:
                        for dx in range(run):
                            if palettes:
                                idx=b[pixel];pixel+=1;r,g,blue=b[24+pi*768+idx*3:27+pi*768+idx*3];value=(r>>3)<<11|(g>>2)<<5|(blue>>3)
                            else:value=struct.unpack_from('<H',b,pixel)[0];pixel+=2
                            pixels.append((x+dx,y,value))
                    x+=run;colour=not colour
                if delta>at+size or pixel>at+size:raise ValueError('SPR plane extent')
            asset['frames'][index]=(ox,oy,pixels);return asset['frames'][index]
        traces={}
        def original(asset,sequence,switch=False):
            key=(asset['ani'],sequence,switch)
            if key not in traces:
                command=[str(helper),str(exe),str(root/asset['ani']),str(sequence),'64','-1']+(['7','7'] if switch else [])
                p=subprocess.run(command,capture_output=True,text=True,timeout=5);p.check_returncode();traces[key]=json.loads(p.stdout)
                (out/f'original-{len(traces):02}.json').write_text(p.stdout)
            return traces[key]
        background=[0x2124 if (x//16+y//16)%2 else 0x2945 for y in range(256) for x in range(512)]
        # Surface2 RGB565 comparison established bit replication; keep the CPU
        # oracle explicit and independent of the renderer.
        rgba=[bytes(((((p>>11)&31)<<3)|(p>>13),(((p>>5)&63)<<2)|((p>>9)&3),((p&31)<<3)|((p>>2)&7),255)) for p in range(65536)]
        command=[str(preview),'--root',str(root),'--ani','c:\\MagicMayhem\\cReAtUrEs\\rEdCaP.aNi','--sequences','0,4','--ticks','32',
                 '--layer','cReAtUrEs\\bAt.aNi,0,1','--layer','Creatures/eye.ani,0,2','--tile-size','2']
        runs=[]
        for view,loop,switch in [(v,False,False) for v in range(4)]+[(0,True,False),(0,False,True)]:
            directory=out/f'view-{view}-loop-{int(loop)}-switch-{int(switch)}'
            p=subprocess.run(command+['--placement-view',str(view),'--export-dir',str(directory)]+(['--loop'] if loop else [])+(['--facing-change','7,0,7'] if switch else []),capture_output=True,text=True,timeout=60)
            (out/(directory.name+'.stderr')).write_text(p.stderr);p.check_returncode();report=json.loads(p.stdout)
            if len(report['frames'])!=33 or report['remaining_surfaces']!=0 or len(report['layer_inputs'])!=2:raise ValueError('Layer output bounds differ')
            shift=(32,-16) if view==1 else ((0,-32) if view==2 else ((-32,-16) if view==3 else (0,0)))
            positions=[[0,0,0],[0,0,0]];frames=[]
            for tick,row in enumerate(report['frames']):
                if row['tick']!=tick or len(row['actors'])!=2 or len(row['layers'])!=4:raise ValueError('Layer trace order differs')
                expected=background.copy()
                def paint(asset,selected,anchor):
                    if selected is None:return
                    ox,oy,pixels=frame(asset,selected)
                    for x,y,value in pixels:
                        dx,dy=anchor[0]-ox+x,anchor[1]-oy+y
                        if not 0<=dx<512 or not 0<=dy<256:raise ValueError('CPU composition outside bounded canvas')
                        expected[dy*512+dx]=value
                for actor,g in enumerate((0,4)):
                    series=[original(assets[0],g,switch and actor==0),original(assets[1],0),original(assets[2],0)]
                    for l,trace in enumerate(series):
                        if tick:positions[actor][l]=0 if loop and not trace[positions[actor][l]][5] else positions[actor][l]+1
                    states=[series[l][positions[actor][l]] for l in range(3)];body=states[0];native=row['actors'][actor]
                    selected=body[2] if body[2]>=0 else None;sequence=7 if switch and actor==0 and tick>=7 else g
                    base=(170 if actor==0 else 341,190)
                    if native['sprite']!=selected or native['event']!=body[1] or native['sequence']!=sequence:raise ValueError('Original parent selection/event mismatch')
                    parent_record=record(assets[0],sequence,body) if selected is not None else None
                    anchor=None if parent_record is None else (base[0]+parent_record[2]+shift[0],base[1]+parent_record[3]+shift[1])
                    if (native['draw_anchor_x'],native['draw_anchor_y'])!=((None,None) if anchor is None else anchor):raise ValueError('Parent record placement differs')
                    paint(assets[0],selected,anchor)
                    for l in range(2):
                        state=states[l+1];native=row['layers'][actor*2+l]
                        selected=state[2] if parent_record is not None and state[2]>=0 else None
                        if native['actor']!=actor or native['layer']!=l or native['sprite']!=selected or native['event']!=state[1]:raise ValueError('Original child selection/event mismatch')
                        child=record(assets[l+1],0,state) if selected is not None else None
                        anchor=None if child is None else (base[0]+parent_record[7+l*2]+shift[0]+child[2],base[1]+parent_record[8+l*2]+shift[1]+child[3])
                        if (native['draw_anchor_x'],native['draw_anchor_y'])!=((None,None) if anchor is None else anchor):raise ValueError('Attachment plus child local placement differs')
                        paint(assets[l+1],selected,anchor)
                packed=struct.pack('<'+str(len(expected))+'H',*expected);actual=(directory/f'frame-{tick:03}.565').read_bytes();presented=b''.join(rgba[p] for p in expected)
                if actual!=packed or row['native_sha256']!=digest(packed) or row['rgba_sha256']!=digest(presented):raise ValueError('Layer pixels or presentation differ')
                frames.append({'tick':tick,'native_sha256':digest(packed),'rgba_sha256':digest(presented)})
            runs.append({'view':view,'loop':loop,'switch':switch,'frames':len(frames),'actor_states':len(frames)*2,'layer_states':len(frames)*4,'all_match':True,'frames_sha256':frames})
        p=subprocess.run(command+['--placement-view','1','--interval','10','--smoke-test'],capture_output=True,text=True,timeout=30)
        (out/'window.stderr').write_text(p.stderr);p.check_returncode()
        invalid=subprocess.run(command+['--layer','Creatures/redcap.ani,0,1','--export-dir',str(out/'invalid')],capture_output=True,text=True,timeout=20)
        if invalid.returncode!=2 or (out/'invalid').exists():raise ValueError('Third layer was not rejected before output creation')
        if any(sha(path)!=value for path,value in inputs.items()):raise ValueError('Experiment source/input changed')
        report={'scope':'Explicit two-actor, two-child native preview; original selection and independently checked record placement/SPR pixels; not original whole-scene sorting or asset/action mapping',
                'input_unchanged':True,'live_validated':False,'source_and_input_sha256':{str(p.relative_to(REPO)):v for p,v in inputs.items()},'helper_sha256':sha(helper),
                'window_smoke_passed':True,'third_layer_rejected':True,'summary':{'frames':sum(r['frames'] for r in runs),'actor_states':sum(r['actor_states'] for r in runs),
                'layer_states':sum(r['layer_states'] for r in runs),'distinct_sprite_frames':sum(len(a['frames']) for a in assets),'all_match':True},'runs':runs}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(report['summary'],flush=True)
    finally:verify('after')
if __name__=='__main__':main()
