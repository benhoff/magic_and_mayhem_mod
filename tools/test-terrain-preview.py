#!/usr/bin/env python3
"""Compare installed terrain preview queues, owner flags and complete pixels offline."""
import hashlib,json,os,struct,subprocess,sys,tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
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
def main():
    if '--inner' not in sys.argv:
        subprocess.run(['xvfb-run','-a',sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=300);return
    args=[x for x in sys.argv[1:] if x!='--inner']
    if len(args)!=2:raise ValueError('Supply native executable and original report')
    preview=Path(args[0]).resolve();reference=Path(args[1]).resolve()
    parent=REPO/'working/tests/terrain-preview';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        oracle=json.loads(reference.read_text());assert oracle['all_match'] and oracle['executable_sha256']=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
        root=REPO/'working/game-clean';source=root/'Realms/Celtic/Forest/Terrain.spr';catalog=root/'Realms/Celtic/Forest/Terrain.ttd'
        paths=[preview,reference,source,catalog]+[p for folder in ['apps/terrain-preview','reconstruction/rendering'] for p in (REPO/folder).glob('*') if p.is_file()]+[REPO/p for p in ['assets/terrain_catalog.cpp','assets/terrain_catalog.hpp','assets/sprite_loader.cpp','assets/sprite_loader.hpp','renderer/sprites/sprite.cpp','renderer/blit.cpp','tools/test-terrain-preview.py']]
        inputs={p:sha(p) for p in paths};asset={'sprite':source.read_bytes(),'frames':{}}
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
        runs=[]
        for i,fixture in enumerate(oracle['previews']):
            prefix=out/f'frame-{i:02}'
            ids=[1745]*9 if fixture['repeated'] else [5+4*j for j in range(9)]
            command=[str(preview),'--root',str(root),'--definitions',','.join(map(str,ids)),'--view',str(fixture['view']),'--output',str(prefix)]
            if fixture['overlap']:command+=['--overlap']
            if fixture['visibility']:command+=['--visibility']
            p=subprocess.run(command,env=env,capture_output=True,text=True,timeout=30);(out/f'frame-{i:02}.stderr').write_text(p.stderr);p.check_returncode()
            native=json.loads(prefix.with_suffix('.json').read_text())
            if native['queue']!=fixture['queue'] or native['owners']!=fixture['owners']:raise ValueError('Original terrain queue/owner mismatch')
            expected=[0x2124]*(512*256)
            for draw in fixture['queue']:
                if draw['kind']==-2:continue
                ox,oy,pixels=frame(asset,draw['frame'])
                for x,y,word in pixels:
                    x+=draw['x']-ox;y+=draw['y']-oy
                    if not 0<=x<512 or not 0<=y<256:raise ValueError('Fixture outside bounded preview')
                    expected[y*512+x]=word
            if prefix.with_suffix('.565').read_bytes()!=struct.pack('<'+'H'*len(expected),*expected):raise ValueError('Complete terrain frame differs')
            rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in expected)
            if hashlib.sha256(rgba).hexdigest()!=native['rgba_sha256'] or native['remaining_surfaces']:raise ValueError('Presentation/ownership mismatch')
            runs.append({'view':fixture['view'],'overlap':fixture['overlap'],'visibility':fixture['visibility'],'repeated':fixture['repeated'],'hidden_draws':sum(d['kind']==-2 for d in fixture['queue']),'all_match':True,'pixels_sha256':sha(prefix.with_suffix('.565'))})
        if any(sha(p)!=h for p,h in inputs.items()):raise ValueError('Input changed during run')
        report={'all_match':True,'live_validated':False,'frames':len(runs),'runs':runs,'source_and_input_sha256':{str(p.relative_to(REPO)):h for p,h in inputs.items()},'pixel_oracle':'Independent Python SPR decode/composition using original producer/sort/visibility records; unshaded embedded palette'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print({'frames':len(runs),'hidden_draws':sum(r['hidden_draws'] for r in runs)})
    finally:verify('after')
if __name__=='__main__':main()
