#!/usr/bin/env python3
"""Original SPR visibility pass plus independent overlapping installed frames."""
import argparse, hashlib, json, os, struct, subprocess, sys, tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--queue-reference',type=Path,required=True);parser.add_argument('--visibility-reference',type=Path,required=True)
    parser.add_argument('--build-dir',type=Path,default=REPO/'working/build/sprite-scene');parser.add_argument('--display-owned',action='store_true')
    args=parser.parse_args()
    if not args.display_owned:
        subprocess.run(['xvfb-run','-a','env','QT_QPA_PLATFORM=xcb','LIBGL_ALWAYS_SOFTWARE=1',sys.executable,str(Path(__file__).resolve()),*sys.argv[1:],'--display-owned'],check=True);return
    reference=args.queue_reference.resolve();visibilityReference=args.visibility_reference.resolve();build=args.build_dir.resolve()
    parent=REPO/'working/tests/sprite-visibility-scene';parent.mkdir(parents=True,exist_ok=True)
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
        visibility=json.loads(visibilityReference.read_text());helper=visibilityReference.parent/'reference'
        if not visibility['all_match'] or sha(helper)!=visibility['helper_sha256']:raise ValueError('Unsupported visibility reference')
        for name,digest in visibility['source_and_input_sha256'].items():
            if sha(REPO/name)!=digest:raise ValueError('Visibility reference input changed')
        preview=build/'mnm-sprite-scene-preview'
        sources=['apps/sprite-scene/scene.cpp','apps/sprite-scene/scene.hpp','apps/sprite-scene/main.cpp','apps/sprite-scene/CMakeLists.txt',
                 'tests/sprite-scene-queue-test.cpp','tools/test-sprite-visibility-scene.py','reconstruction/rendering/sprite_visibility.cpp','reconstruction/rendering/sprite_visibility.hpp','assets/sprite_loader.hpp','reconstruction/rendering/sprite_queue.cpp','reconstruction/rendering/sprite_queue.hpp',
                 'renderer/sprites/sprite.cpp','renderer/blit.cpp','assets/sprite_loader.cpp','assets/animation.cpp']
        inputs={p:sha(p) for p in [reference,visibilityReference,helper,preview]+[REPO/p for p in sources]}
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
        runs=[];root=REPO/'working/game-clean';assets=[]
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
        for index,fixture in enumerate(original['scenes']*2):
            directory=out/f'installed-{index:02}'
            command=[str(preview),'--root',str(root),'--ani',assets[0]['ani'],'--sequences','0,4,0,4','--ticks','1',
                     '--layer',assets[1]['ani']+',0,1','--layer',assets[2]['ani']+',0,2','--placement-view',str(fixture['view']),'--visibility','--overlap','--export-dir',str(directory)]
            expanded=index>=24
            if expanded:command+=['--visibility-expanded']
            for position in fixture['positions']:command+=['--queue-position',','.join(map(str,position))]
            p=subprocess.run(command,env=env,capture_output=True,text=True,timeout=30);(out/f'installed-{index:02}.stderr').write_text(p.stderr);p.check_returncode()
            native=json.loads(p.stdout);row=native['frames'][0];expected=background.copy()
            if [draw['actor']*3+draw['asset'] for draw in row['draw_queue']]!=fixture['order'] or [draw['key'] for draw in row['draw_queue']]!=fixture['depth_keys']:raise ValueError('Installed queue differs from original')
            wire=bytearray(struct.pack('<II',12,int(expanded)));selected=[]
            for id in fixture['order']:
                actor,asset=divmod(id,3);source=assets[asset];sequence=(0,4,0,4)[actor] if asset==0 else 0
                r=struct.unpack_from('<11i',source['records'],source['base']+source['starts'][sequence]*44)
                parent=struct.unpack_from('<11i',assets[0]['records'],assets[0]['base']+assets[0]['starts'][(0,4,0,4)[actor]]*44)
                if r[0]!=0 or parent[0]!=0:raise ValueError('Fixture requires initially displayed record')
                anchorX,anchorY=256+r[2],190+r[3]
                if asset:anchorX+=parent[5+2*asset];anchorY+=parent[6+2*asset]
                raw=source['sprite'];count,palettes=struct.unpack_from('<II',raw,12);base=24+palettes*768+4*count
                at=base+struct.unpack_from('<I',raw,24+palettes*768+r[1]*4)[0];length=struct.unpack_from('<I',raw,at)[0]
                wire+=struct.pack('<iiiI',anchorX,anchorY,0,length)+raw[at:at+length]
                selected.append((source,r[1],anchorX,anchorY))
            fixtureFile=out/f'queue-{index:02}.bin';fixtureFile.write_bytes(wire)
            p=subprocess.run([str(helper),str(REPO/'working/game-nocd/Chaos.exe'),str(fixtureFile)],capture_output=True,text=True,timeout=10)
            (out/f'original-{index:02}.stdout').write_text(p.stdout);(out/f'original-{index:02}.stderr').write_text(p.stderr);p.check_returncode()
            oracle=json.loads(p.stdout);kinds=oracle['kinds']
            if kinds!=[draw['kind'] for draw in row['draw_queue']]:raise ValueError('Visibility kinds differ from original')
            for kind,(source,frameIndex,anchorX,anchorY) in zip(kinds,selected):
                if kind==-2:continue
                ox,oy,pixels=frame(source,frameIndex)
                for dx,dy,value in pixels:
                    x,y=anchorX-ox+dx,anchorY-oy+dy
                    if not 0<=x<512 or not 0<=y<256:raise ValueError('Installed fixture outside canvas')
                    expected[y*512+x]=value
            packed=struct.pack('<'+'H'*len(expected),*expected)
            if (directory/'frame-000.565').read_bytes()!=packed:raise ValueError('Installed complete frame differs')
            rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in expected)
            if hashlib.sha256(rgba).hexdigest()!=row['rgba_sha256'] or native['remaining_surfaces']:raise ValueError('Presentation/ownership differs')
            runs.append({'view':fixture['view'],'kind':fixture['kind'],'expanded':expanded,'hidden_draws':kinds.count(-2),'kinds':kinds,'native_sha256':sha(directory/'frame-000.565'),'all_match':True})
        # Installed terrain mask with real full-coverage decisions. Only the
        # container/ANI are fixture-authored; selected encoded frame is unchanged.
        terrain=root/'Realms/Celtic/Forest/Terrain.spr';inputs[terrain]=sha(terrain);raw=terrain.read_bytes();frameIndex=1745
        count,palettes=struct.unpack_from('<II',raw,12);base=24+palettes*768+4*count
        at=base+struct.unpack_from('<I',raw,24+palettes*768+frameIndex*4)[0];length=struct.unpack_from('<I',raw,at)[0];record=raw[at:at+length]
        fixtureRoot=out/'terrain-fixture';fixtureRoot.mkdir();prefix=bytearray(raw[:24+palettes*768]);prefix+=struct.pack('<I',0)
        struct.pack_into('<I',prefix,4,len(prefix)+len(record));struct.pack_into('<I',prefix,12,1)
        (fixtureRoot/'fixture.spr').write_bytes(prefix+record)
        records=b''.join(struct.pack('<11i',op,arg,*([0]*9)) for _ in range(4) for op,arg in ((0,0),(6,-1)))
        ani=b'ANI\0'+struct.pack('<5I',64+len(records),8,5,0,5)+b'fixture.spr\0'.ljust(20,b'\0')+struct.pack('<5I',0,2,4,6,8)+records
        (fixtureRoot/'fixture.ani').write_bytes(ani)
        for expanded in (False,True):
            directory=out/f'terrain-{int(expanded)}';command=[str(preview),'--root',str(fixtureRoot),'--ani','fixture.ani','--sequences','0,1,2,3','--ticks','1','--overlap','--visibility','--export-dir',str(directory)]
            if expanded:command+=['--visibility-expanded']
            p=subprocess.run(command,env=env,capture_output=True,text=True,timeout=30);(out/f'terrain-{int(expanded)}.stderr').write_text(p.stderr);p.check_returncode();native=json.loads(p.stdout);row=native['frames'][0]
            wire=struct.pack('<II',4,int(expanded))+(struct.pack('<iiiI',256,190,0,len(record))+record)*4
            fixtureFile=out/f'terrain-{int(expanded)}.bin';fixtureFile.write_bytes(wire)
            p=subprocess.run([str(helper),str(REPO/'working/game-nocd/Chaos.exe'),str(fixtureFile)],capture_output=True,text=True,timeout=10);p.check_returncode();oracle=json.loads(p.stdout)
            (out/f'terrain-{int(expanded)}-original.json').write_text(p.stdout)
            kinds=oracle['kinds']
            if kinds!=[0,-2,-2,0] or kinds!=[draw['kind'] for draw in row['draw_queue']]:raise ValueError('Installed terrain did not hide original middle draws')
            if [draw['actor'] for draw in row['draw_queue']]!=[0,1,2,3] or [draw['key'] for draw in row['draw_queue']]!=[6,38,70,102]:raise ValueError('Terrain fixture depth differs')
            source={'sprite':raw,'frames':{}};ox,oy,pixels=frame(source,frameIndex);expected=background.copy()
            for kind in kinds:
                if kind==-2:continue
                for x,y,value in pixels:
                    dx,dy=256-ox+x,190-oy+y
                    if not 0<=dx<512 or not 0<=dy<256:raise ValueError('Terrain fixture bounds')
                    expected[dy*512+dx]=value
            packed=struct.pack('<'+'H'*len(expected),*expected)
            rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in expected)
            if (directory/'frame-000.565').read_bytes()!=packed or hashlib.sha256(rgba).hexdigest()!=row['rgba_sha256'] or native['remaining_surfaces']:raise ValueError('Terrain complete frame/presentation differs')
            runs.append({'asset':str(terrain.relative_to(REPO)),'frame':frameIndex,'expanded':expanded,'hidden_draws':2,'kinds':kinds,'native_sha256':sha(directory/'frame-000.565'),'encoded_frame_sha256':hashlib.sha256(record).hexdigest(),'all_match':True})
        changed=[str(p) for p,digest in inputs.items() if sha(p)!=digest]
        if changed:raise ValueError('Input changed during comparison: '+', '.join(changed))
        report={'reference':str(reference.relative_to(REPO)),'reference_sha256':sha(reference),'visibility_reference_sha256':sha(visibilityReference),'sources_and_inputs':{str(p.relative_to(REPO)):h for p,h in inputs.items()},
                'visibility_reference':str(visibilityReference.relative_to(REPO)),'installed_frames':len(runs),'installed':runs,'all_match':True,'live_validated':False,
                'scope':'Original sorted orders and visibility pass kinds/grid matches; independent full native frame/presentation matches; no live world owner/activation inputs'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print({'installed_frames':len(runs),'hidden_draws':sum(run['hidden_draws'] for run in runs)},flush=True)
    finally:verify('after')
if __name__=='__main__':main()
