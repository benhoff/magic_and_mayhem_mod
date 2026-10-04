#!/usr/bin/env python3
"""Installed mode-one attachment recipe: original selection and independent pixels."""
import hashlib, importlib.util, json, struct, subprocess, sys, tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def digest(b):return hashlib.sha256(b).hexdigest()
def main():
    if '--display-owned' not in sys.argv:
        subprocess.run(['xvfb-run','-a','env','QT_QPA_PLATFORM=xcb','LIBGL_ALWAYS_SOFTWARE=1','python3',str(Path(__file__).resolve()),'--display-owned'],check=True);return
    parent=REPO/'working/tests/attachment-recipe-scene';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        root=REPO/'working/game-clean';exe=REPO/'working/game-nocd/Chaos.exe';preview=REPO/'working/build/sprite-scene/mnm-sprite-scene-preview'
        if sha(exe)!=HASH:raise ValueError('Unsupported executable')
        sources=['apps/sprite-scene/main.cpp','apps/sprite-scene/scene.cpp','apps/sprite-scene/scene.hpp','apps/sprite-scene/attachment_recipe.cpp','apps/sprite-scene/attachment_recipe.hpp',
                 'reconstruction/animation/no_cd.cpp','reconstruction/animation/no_cd.hpp','reconstruction/animation/attachment.cpp','reconstruction/animation/attachment.hpp',
                 'reconstruction/animation/placement.cpp','reconstruction/animation/placement.hpp','assets/persistence.hpp','assets/config_loader.cpp','assets/packed_container.cpp',
                 'tests/animation-binary-reference.cpp','tests/animation-attachment-reference.cpp','tools/test-attachment-recipe-scene.py','tools/decode-cfg.py']
        inputs={p:sha(p) for p in [exe,preview,root/'CFG/Encrypted/effectani.cfg']+[REPO/s for s in sources]}
        spec=importlib.util.spec_from_file_location('cfg',REPO/'tools/decode-cfg.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        _,clear=module.decode((root/'CFG/Encrypted/effectani.cfg').read_bytes());sections={};section=None
        for line in clear.decode('ascii').splitlines():
            line=line.split(';')[0].strip()
            if line.startswith('['):section=line[1:-1].upper();sections[section]={}
            elif '=' in line:
                key,value=line.split('=',1);sections[section][key.strip().upper()]=value.strip()
        recipe=sections['ANI_36'];recipe_base=int(recipe['ANIMATIONNO'])
        if recipe['ANIMATIONFILEREF']!='EFFECTS2' or recipe['SPRITEPRINTER']!='NORMAL':raise ValueError('Unsupported installed recipe')
        helpers=[]
        for name,cpp in [('body','animation-binary-reference'),('child','animation-attachment-reference')]:
            helper=out/name
            command=['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(REPO/'assets'),'-I'+str(REPO/'reconstruction/animation'),str(REPO/'tests'/ (cpp+'.cpp')),str(REPO/'reconstruction/animation/no_cd.cpp')]
            if name=='child':command.append(str(REPO/'reconstruction/animation/attachment.cpp'))
            p=subprocess.run(command+['-o',str(helper)],capture_output=True,text=True,timeout=60);(out/(name+'-compile.log')).write_text(p.stdout+p.stderr);p.check_returncode();helpers.append(helper)
        names=[('Creatures/redcap.ani','Creatures/RedCap.spr'),('Sprites/effects2.ani','Sprites/effects2.spr')]
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

        p=subprocess.run([str(helpers[0]),str(exe),str(root/names[0][0]),'0','64','-1'],capture_output=True,text=True,timeout=10);p.check_returncode();body=json.loads(p.stdout);(out/'original-body.json').write_text(p.stdout)
        child={}
        for facing in range(8):
            p=subprocess.run([str(helpers[1]),str(exe),str(root/names[1][0]),str(recipe_base),'2',str(facing),'64'],capture_output=True,text=True,timeout=10);p.check_returncode();child[facing]=json.loads(p.stdout)['rows'];(out/f'original-child-{facing}.json').write_text(p.stdout)
        background=[0x2124 if (x//16+y//16)%2 else 0x2945 for y in range(256) for x in range(512)]
        rgba=[bytes((((p>>11)&31)*255//31,((p>>5)&63)*255//63,(p&31)*255//31,255)) for p in range(65536)]
        cases=[(f,0,1,False) for f in range(8)]+[(0,v,1,False) for v in (1,2,3)]+[(0,0,h,False) for h in (0,-1)]+[(0,0,1,True)]
        runs=[]
        command=[str(preview),'--root',str(root),'--ani','c:\\MagicMayhem\\cReAtUrEs\\rEdCaP.aNi','--sequences','0','--ticks','32','--loop','--attachment-mode-one','--tile-size','2']
        for case,(facing,view,health,transition) in enumerate(cases):
            directory=out/f'case-{case:02}'
            args=command+['--attachment-facing',str(facing),'--placement-view',str(view),'--attachment-health',str(health)]
            if transition:args+=['--attachment-remove-at','8','--attachment-reenter-at','16']
            p=subprocess.run(args+['--export-dir',str(directory)],capture_output=True,text=True,timeout=60);(out/(directory.name+'.stderr')).write_text(p.stderr);p.check_returncode();report=json.loads(p.stdout)
            if report['remaining_surfaces']!=0 or len(report['frames'])!=33 or len(report['layer_inputs'])!=1:raise ValueError('Recipe output extent/resources')
            li=report['layer_inputs'][0]
            if li['sequence']!=recipe_base+facing or li['asset_index']!=2 or li['slot']!=1 or li['config_entry']!=36:raise ValueError('Installed recipe selection differs')
            shift=(32,-16) if view==1 else ((0,-32) if view==2 else ((-32,-16) if view==3 else (0,0)))
            bi=ci=0;enabled=True;hashes=[]
            for tick,row in enumerate(report['frames']):
                if tick:
                    bi=0 if not body[bi][5] else bi+1
                    if transition and tick==8:enabled=False
                    if transition and tick==16:enabled=True;ci=0
                    if enabled:ci+=1
                br=body[bi];cr=child[facing][ci];selected=br[2] if br[2]>=0 else None
                parent=record(assets[0],0,br) if selected is not None else None
                body_anchor=None if parent is None else (256+parent[2]+shift[0],190+parent[3]+shift[1])
                native=row['actors'][0]
                if native['sprite']!=selected or native['event']!=br[1] or (native['draw_anchor_x'],native['draw_anchor_y'])!=((None,None) if body_anchor is None else body_anchor):raise ValueError('Body controller/placement mismatch')
                child_selected=cr[2] if enabled and health!=0 and parent is not None and cr[2]>=0 else None
                raw=struct.unpack_from('<11i',assets[1]['records'],assets[1]['base']+(assets[1]['starts'][recipe_base+facing]+cr[3])*44) if child_selected is not None else None
                child_anchor=None if raw is None else (256+parent[7]+shift[0]+raw[2],190+parent[8]+shift[1]+raw[3])
                native=row['layers'][0]
                if native['sprite']!=child_selected or native['event']!=(cr[1] if enabled else 0) or (native['draw_anchor_x'],native['draw_anchor_y'])!=((None,None) if child_anchor is None else child_anchor):raise ValueError('Recipe visibility/controller/placement mismatch')
                expected=background.copy()
                for asset,sprite,anchor in [(assets[0],selected,body_anchor),(assets[1],child_selected,child_anchor)]:
                    if sprite is None:continue
                    ox,oy,pixels=frame(asset,sprite)
                    for x,y,value in pixels:
                        dx,dy=anchor[0]-ox+x,anchor[1]-oy+y
                        if not 0<=dx<512 or not 0<=dy<256:raise ValueError('Recipe outside bounded canvas')
                        expected[dy*512+dx]=value
                packed=struct.pack('<'+str(len(expected))+'H',*expected);presented=b''.join(rgba[p] for p in expected)
                if (directory/f'frame-{tick:03}.565').read_bytes()!=packed or row['native_sha256']!=digest(packed) or row['rgba_sha256']!=digest(presented):raise ValueError('Whole native recipe frame differs')
                hashes.append({'tick':tick,'native_sha256':digest(packed),'rgba_sha256':digest(presented)})
            runs.append({'facing':facing,'view':view,'health':health,'remove_reenter':transition,'frames':33,'all_match':True,'frames_sha256':hashes})
        p=subprocess.run(command+['--interval','10','--smoke-test'],capture_output=True,text=True,timeout=30);(out/'window.stderr').write_text(p.stderr);p.check_returncode()
        p=subprocess.run(command+['--layer','Creatures/bat.ani,0,1','--export-dir',str(out/'invalid')],capture_output=True,text=True,timeout=20)
        if p.returncode!=2 or (out/'invalid').exists():raise ValueError('Mixed recipe/explicit layers not rejected')
        if any(sha(p)!=v for p,v in inputs.items()):raise ValueError('Source/input changed')
        report={'scope':'Installed ANI_36 mode-one recipe, original admission/child traces and independent CPU placement/pixels. Preview admission, health fixtures, parent clock, palette and layer ordering are bounded policies; no live game replacement.',
                'input_unchanged':True,'live_validated':False,'source_and_input_sha256':{str(p.relative_to(REPO)):v for p,v in inputs.items()},'helper_sha256':{p.name:sha(p) for p in helpers},
                'window_smoke_passed':True,'mixed_layers_rejected':True,'summary':{'runs':len(runs),'frames':sum(r['frames'] for r in runs),'actor_states':sum(r['frames'] for r in runs),'layer_states':sum(r['frames'] for r in runs),'all_match':True},'runs':runs}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(report['summary'],flush=True)
    finally:verify('after')
if __name__=='__main__':main()
