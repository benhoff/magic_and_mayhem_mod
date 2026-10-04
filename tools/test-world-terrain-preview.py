#!/usr/bin/env python3
"""Guarded four-orientation world traversal and complete native image checks."""
import argparse,hashlib,importlib.util,json,os,struct,subprocess,sys,tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    if '--inner' not in sys.argv:
        subprocess.run(['xvfb-run','-a',sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=600);return
    p=argparse.ArgumentParser();p.add_argument('binary',type=Path);p.add_argument('--sanitized',type=Path);p.add_argument('--source-root',type=Path,default=REPO);p.add_argument('--inner',action='store_true');args=p.parse_args();source=args.source_root.resolve()
    parent=REPO/'working/tests/world-terrain';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,stem,**kwargs):
        r=subprocess.run(command,capture_output=True,text=True,timeout=120,**kwargs);(out/(stem+'.stdout')).write_text(r.stdout);(out/(stem+'.stderr')).write_text(r.stderr);r.check_returncode();return r
    def verify(phase):run([str(REPO/'tools/original-manifest.sh'),'verify'],'original-'+phase)
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe';assert sha(exe)==HASH;root=REPO/'working/game-clean';realm=root/'Realms/Celtic/Forest'
        binaries=[args.binary.resolve()]+([args.sanitized.resolve()] if args.sanitized else [])
        paths=[exe,realm/'Terrain.ttd',realm/'Terrain.spr']+binaries
        for directory in ['apps/terrain-preview','reconstruction/rendering','renderer/sprites']:
            paths += [x for x in (source/directory).glob('*') if x.is_file()]
        paths += [source/x for x in ['tests/terrain-traversal-reference.cpp','tests/terrain-traversal-test.cpp','tests/world-terrain-reference.cpp','tools/test-world-terrain-preview.py','tools/decode-cfg.py','tools/test-terrain-preview.py']]
        paths += [x for x in (source/'assets').glob('*') if x.suffix in ('.cpp','.hpp')]
        paths += [realm/name for name in ['CFsec01.map','CFsec02.map','CFsec38.map']]
        inputs={path:sha(path) for path in paths}
        include='-I'+str(source/'reconstruction/rendering');model=[str(source/'reconstruction/rendering'/name) for name in ['terrain_traversal.cpp','terrain_submission.cpp','sprite_queue.cpp']]
        reference=out/'traversal-reference';run(['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror',include,str(source/'tests/terrain-traversal-reference.cpp'),*model,'-o',str(reference)],'compile-traversal')
        traversal=json.loads(run([str(reference),str(exe)],'traversal').stdout)
        unit=out/'traversal-sanitized';run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer',include,str(source/'tests/terrain-traversal-test.cpp'),*model,'-o',str(unit)],'compile-unit');run([str(unit)],'unit',env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
        helper=out/'world-reference';run(['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror',include,str(source/'tests/world-terrain-reference.cpp'),'-o',str(helper)],'compile-world')
        for name,a,z in [('origin',0x4f8230,0x4f8310),('traversal0',0x4f83f0,0x4f8955),('traversal1',0x4fbe00,0x4fc3e0),('traversal2',0x4fc3f0,0x4fc925),('traversal3',0x4fc930,0x4fcf40)]:
            r=run(['objdump','-d','-Mintel',f'--start-address={a}',f'--stop-address={z}',str(exe)],name);(out/(name+'.asm')).write_text(r.stdout)
        codec=module('world_codec',source/'tools/decode-cfg.py');decoder=module('world_sprite',source/'tools/test-terrain-preview.py');sprite={'sprite':(realm/'Terrain.spr').read_bytes(),'frames':{}}
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0');cases=[];index=0
        for name in ['CFsec01.map','CFsec02.map','CFsec38.map']:
            _,raw=codec.decode((realm/name).read_bytes());w,h,l=struct.unpack_from('<3I',raw,4);decoded=out/(name+'.decoded');decoded.write_bytes(raw)
            selections=[((w//2,h//2,20,l),(256,64)),((0,0,20,min(l,3)),(256,64)),((w-1,h-1,12,min(l,6)),(-16,16)),((w-10,h-10,20,l-1),(256,64))]
            for camera,pan in selections:
                for view,visibility in [(v,b) for v in range(4) for b in (0,1)]:
                    oracle=json.loads(run([str(helper),str(exe),str(realm/'Terrain.ttd'),str(realm/'Terrain.spr'),str(decoded),*map(str,camera),*map(str,pan),str(visibility),str(view)],f'oracle-{index:02}').stdout)
                    pixels=[0x2124]*(512*256);partial=0
                    for draw in oracle['queue']:
                        if draw['kind']==-2:continue
                        ox,oy,values=decoder.frame(sprite,draw['frame']);inside=0
                        for dx,dy,v in values:
                            x,y=draw['x']-ox+dx,draw['y']-oy+dy
                            if 0<=x<512 and 0<=y<256:pixels[y*512+x]=v;inside+=1
                        partial+=bool(inside and inside<len(values))
                    words=struct.pack('<'+'H'*len(pixels),*pixels);rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in pixels)
                    for build,binary in enumerate(binaries):
                        prefix=out/f'frame-{index:02}-{build}';request='Realms\\Celtic\\Forest\\'+name.swapcase();command=[str(binary),'--root',str(root),'--map',request,'--world','--view',str(view),'--camera',','.join(map(str,camera)),'--pan',','.join(map(str,pan)),'--output',str(prefix)]
                        if visibility:command+=['--visibility']
                        run(command,f'native-{index:02}-{build}',env=env);native=json.loads(prefix.with_suffix('.json').read_text());actual=[]
                        for draw in native['queue']:
                            d={k:v for k,v in draw.items() if k not in ('tile','role')};assert draw['role']==0;d['cell']=native['tiles'][draw['tile']]['cell'];actual.append(d)
                        assert actual==oracle['queue'],(name,camera,visibility,'queue')
                        assert native['world'] and native['remaining_surfaces']==0
                        assert native['map']=={'path':request,'width':w,'height':h,'layers':l,'camera':list(camera),'pan':list(pan)}
                        assert len(native['tiles'])==len(native['owners'])
                        for tile,owner in zip(native['tiles'],native['owners']):
                            cell=tile['cell'];fields=struct.unpack_from('<6H',raw,76+12*cell)
                            assert tile['definition']==fields[0] and tile['flags8']==fields[4] and tile['flags10']==fields[5] and owner==oracle['owners'][cell]
                            assert 0<=cell<w*h*l and 0<=tile['column']<=w and 0<=tile['row']<h
                        assert prefix.with_suffix('.565').read_bytes()==words and native['rgba_sha256']==hashlib.sha256(rgba).hexdigest()
                        cases.append({'map':name,'camera':camera,'pan':pan,'view':view,'visibility':visibility,'build':build,'aliases':len(native['tiles'])-len({t['cell'] for t in native['tiles']}),'tiles':len(native['tiles']),'draws':len(actual),'hidden':sum(d['kind']==-2 for d in actual),'partially_clipped':partial,'pixels_sha256':sha(prefix.with_suffix('.565')),'all_match':True})
                    index+=1
        # Existing slice behavior is retained; unsupported world inputs reject.
        for options in [['--world'],['--world','--map','Realms/Celtic/Forest/CFsec01.map','--view','4'],['--world','--map','Realms/Celtic/Forest/CFsec01.map','--region','0,0,0,3,3'],['--world','--map','Realms/Celtic/Forest/CFsec01.map','--camera','0,0,41,1']]:
            prefix=out/'rejected';r=subprocess.run([str(binaries[0]),'--root',str(root),*options,'--output',str(prefix)],capture_output=True,text=True,env=env,timeout=30);assert r.returncode and not list(out.glob('rejected.*'))
        assert all(sha(path)==hash_ for path,hash_ in inputs.items()),'Inputs changed during run'
        report={'all_match':True,'live_validated':False,'traversal':traversal,'cases':index,'images':len(cases),'runs':cases,'rejected_inputs':4,'source_snapshot':str(source.relative_to(REPO)),'source_and_input_sha256':{str(path.relative_to(REPO)):hash_ for path,hash_ in inputs.items()},'helper_sha256':{'traversal':sha(reference),'world':sha(helper),'sanitized_unit':sha(unit)},'scope':'Unchanged four-orientation traversal and ordinary terrain producer/sort/visibility; references and creature branch disabled in oracle; independent clipped unshaded SPR images'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print({'traversal':traversal,'images':len(cases),'hidden':sum(c['hidden'] for c in cases),'partial':sum(c['partially_clipped'] for c in cases)},flush=True)
    finally:verify('after')
if __name__=='__main__':main()
