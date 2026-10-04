#!/usr/bin/env python3
"""Original queue order plus independent overlapping synthetic/installed frames."""
import argparse, hashlib, json, os, struct, subprocess, tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--reference',type=Path,required=True)
    parser.add_argument('--build-dir',type=Path,default=REPO/'working/build/sprite-scene');parser.add_argument('--synthetic-only',action='store_true')
    args=parser.parse_args();reference=args.reference.resolve();build=args.build_dir.resolve()
    parent=REPO/'working/tests/sprite-queue-scene';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        original=json.loads(reference.read_text())
        if not original['all_match'] or original['executable_sha256']!=HASH:raise ValueError('Unsupported reference')
        for name,digest in original['source_and_input_sha256'].items():
            if sha(REPO/name)!=digest:raise ValueError('Reference inputs changed: '+name)
        exe=build/'sprite-scene-queue-test';preview=build/'mnm-sprite-scene-preview'
        sources=['apps/sprite-scene/scene.cpp','apps/sprite-scene/scene.hpp','apps/sprite-scene/main.cpp','apps/sprite-scene/CMakeLists.txt',
                 'tests/sprite-scene-queue-test.cpp','tools/test-sprite-queue-scene.py','reconstruction/rendering/sprite_queue.cpp','reconstruction/rendering/sprite_queue.hpp',
                 'renderer/sprites/sprite.cpp','renderer/blit.cpp','assets/sprite_loader.cpp','assets/animation.cpp']
        inputs={p:sha(p) for p in [reference,exe,preview]+[REPO/p for p in sources]}
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
        p=subprocess.run(['xvfb-run','-a',str(exe),str(reference)],env=env,capture_output=True,text=True,timeout=60)
        (out/'synthetic.stdout').write_text(p.stdout);(out/'synthetic.stderr').write_text(p.stderr);p.check_returncode();synthetic=json.loads(p.stdout)
        runs=[];root=REPO/'working/game-clean';assets=[]
        if not args.synthetic_only:
            for ani,spr in [('Creatures/redcap.ani','Creatures/RedCap.spr'),('Creatures/bat.ani','Creatures/Bat.spr'),('Creatures/eye.ani','Creatures/eye.spr')]:
                ap,sp=root/ani,root/spr;inputs[ap]=sha(ap);inputs[sp]=sha(sp);b=ap.read_bytes();n=struct.unpack_from('<I',b,20)[0]
                assets.append({'ani':ani,'spr':spr,'records':b,'starts':struct.unpack_from('<'+str(n)+'I',b,44),'base':44+4*n,'sprite':sp.read_bytes(),'frames':{}})
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
        background=[0x2124 if (x//16+y//16)%2 else 0x2945 for y in range(256) for x in range(512)]
        for index,fixture in enumerate(original['scenes'] if not args.synthetic_only else []):
            directory=out/f'installed-{index:02}'
            command=['xvfb-run','-a',str(preview),'--root',str(root),'--ani',assets[0]['ani'],'--sequences','0,4,0,4','--ticks','1',
                     '--layer',assets[1]['ani']+',0,1','--layer',assets[2]['ani']+',0,2','--placement-view',str(fixture['view']),'--overlap','--export-dir',str(directory)]
            for position in fixture['positions']:command+=['--queue-position',','.join(map(str,position))]
            p=subprocess.run(command,env=env,capture_output=True,text=True,timeout=30);(out/f'installed-{index:02}.stderr').write_text(p.stderr);p.check_returncode()
            native=json.loads(p.stdout);row=native['frames'][0];expected=background.copy()
            if [draw['actor']*3+draw['asset'] for draw in row['draw_queue']]!=fixture['order'] or [draw['key'] for draw in row['draw_queue']]!=fixture['depth_keys']:raise ValueError('Installed queue differs from original')
            for id in fixture['order']:
                actor,asset=divmod(id,3);source=assets[asset];sequence=(0,4,0,4)[actor] if asset==0 else 0
                r=struct.unpack_from('<11i',source['records'],source['base']+source['starts'][sequence]*44)
                parent=struct.unpack_from('<11i',assets[0]['records'],assets[0]['base']+assets[0]['starts'][(0,4,0,4)[actor]]*44)
                if r[0]!=0 or parent[0]!=0:raise ValueError('Fixture requires initially displayed record')
                ox,oy,pixels=frame(source,r[1]);x,y=256+r[2]-ox,190+r[3]-oy
                if asset:x+=parent[5+2*asset];y+=parent[6+2*asset]
                for dx,dy,value in pixels:
                    if not 0<=x+dx<512 or not 0<=y+dy<256:raise ValueError('Installed fixture outside canvas')
                    expected[(y+dy)*512+x+dx]=value
            packed=struct.pack('<'+'H'*len(expected),*expected)
            if (directory/'frame-000.565').read_bytes()!=packed:raise ValueError('Installed complete frame differs')
            rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in expected)
            if hashlib.sha256(rgba).hexdigest()!=row['rgba_sha256'] or native['remaining_surfaces']:raise ValueError('Presentation/ownership differs')
            runs.append({'view':fixture['view'],'kind':fixture['kind'],'native_sha256':sha(directory/'frame-000.565'),'all_match':True})
        if any(sha(p)!=digest for p,digest in inputs.items()):raise ValueError('Input changed during comparison')
        report={'reference':str(reference.relative_to(REPO)),'reference_sha256':sha(reference),'sources_and_inputs':{str(p.relative_to(REPO)):h for p,h in inputs.items()},
                'synthetic':synthetic,'installed_frames':len(runs),'installed':runs,'all_match':True,'live_validated':False,
                'scope':'Original builder/sort orders, independent CPU frame composition; no original world occlusion/whole-scene renderer'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print({'synthetic':synthetic,'installed_frames':len(runs)},flush=True)
    finally:verify('after')
if __name__=='__main__':main()
