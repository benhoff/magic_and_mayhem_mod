#!/usr/bin/env python3
"""Installed MAP slices -> original terrain queues -> native complete images."""
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
    p=argparse.ArgumentParser();p.add_argument('binary',type=Path);p.add_argument('--sanitized',type=Path);p.add_argument('--inner',action='store_true');p.add_argument('--source-root',type=Path,default=REPO);args=p.parse_args();source=args.source_root.resolve()
    parent=REPO/'working/tests/map-terrain-preview';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        run=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120);(out/f'original-{phase}.log').write_text(run.stdout+run.stderr);run.check_returncode()
    verify('before')
    try:
        root=REPO/'working/game-clean';realm=root/'Realms/Celtic/Forest';exe=REPO/'working/game-nocd/Chaos.exe';assert sha(exe)==HASH
        codec=module('map_codec',source/'tools/decode-cfg.py');decoder=module('sprite_codec',source/'tools/test-terrain-preview.py')
        binaries=[args.binary.resolve()]+([args.sanitized.resolve()] if args.sanitized else [])
        paths=[exe,realm/'Terrain.ttd',realm/'Terrain.spr']+binaries+[p for p in (source/'apps/terrain-preview').glob('*') if p.is_file()]+[source/p for p in ['assets/map.cpp','assets/map.hpp','assets/terrain_catalog.cpp','assets/terrain_catalog.hpp','assets/packed_container.cpp','assets/sprite_loader.cpp','assets/sprite_loader.hpp','reconstruction/rendering/terrain_submission.cpp','reconstruction/rendering/terrain_submission.hpp','reconstruction/rendering/sprite_queue.cpp','reconstruction/rendering/sprite_visibility.cpp','tests/map-terrain-reference.cpp','tools/test-map-terrain-preview.py','tools/test-terrain-preview.py','tools/decode-cfg.py']]
        selections=[('CFsec01.map',(8,7,1,3,3)),('CFsec01.map',(7,0,6,3,3)),('CFsec02.map',(6,0,1,3,3)),('CFsec02.map',(11,2,6,3,3)),('CFsec38.map',(6,0,1,3,3)),('CFsec38.map',(1,0,6,3,3)),('CFsec01.map',(0,0,0,3,3))]
        decoded={};fixtures={}
        for name,_ in selections:
            if name in decoded:continue
            path=realm/name;paths.append(path);_,raw=codec.decode(path.read_bytes());decoded[name]=raw;fixture=out/(name+'.decoded');fixture.write_bytes(raw);fixtures[name]=fixture
        inputs={p:sha(p) for p in paths}
        helper=out/'reference';run=subprocess.run(['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(source/'reconstruction/rendering'),str(source/'tests/map-terrain-reference.cpp'),'-o',str(helper)],capture_output=True,text=True,timeout=60);(out/'compile.log').write_text(run.stdout+run.stderr);run.check_returncode()
        asset={'sprite':(realm/'Terrain.spr').read_bytes(),'frames':{}};env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0');runs=[]
        index=0
        for name,region in selections:
            x,y,z,w,h=region;raw=decoded[name];mw,mh,ml=struct.unpack_from('<3I',raw,4)
            tileInfo=[]
            for row in range(h):
                for col in range(w):
                    fields=struct.unpack_from('<6H',raw,76+12*((z*mh+y+row)*mw+x+col));tileInfo.append({'definition':fields[0],'column':x+col,'row':y+row,'level':z,'flags8':fields[4],'flags10':fields[5]})
            for view in range(4):
                overlap=int(bool(view&1))
                for visibility in range(2):
                    run=subprocess.run([str(helper),str(exe),str(realm/'Terrain.ttd'),str(realm/'Terrain.spr'),str(fixtures[name]),*map(str,region),str(view),str(overlap),str(visibility)],capture_output=True,text=True,timeout=30);(out/f'original-{index:03}.json').write_text(run.stdout);(out/f'original-{index:03}.stderr').write_text(run.stderr);run.check_returncode();oracle=json.loads(run.stdout)
                    expected=[0x2124]*(512*256)
                    for draw in oracle['queue']:
                        if draw['kind']==-2:continue
                        ox,oy,pixels=decoder.frame(asset,draw['frame'])
                        for dx,dy,value in pixels:
                            px,py=draw['x']-ox+dx,draw['y']-oy+dy
                            assert 0<=px<512 and 0<=py<256;expected[py*512+px]=value
                    words=struct.pack('<'+'H'*len(expected),*expected);rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for v in expected)
                    for build,binary in enumerate(binaries):
                        prefix=out/f'frame-{index:03}-{build}';request='Realms\\Celtic\\Forest\\'+name.swapcase();command=[str(binary),'--root',str(root),'--map',request,'--region',','.join(map(str,region)),'--view',str(view),'--output',str(prefix)]
                        if overlap:command+=['--overlap']
                        if visibility:command+=['--visibility']
                        run=subprocess.run(command,env=env,capture_output=True,text=True,timeout=30);(out/f'frame-{index:03}-{build}.stderr').write_text(run.stderr);run.check_returncode();native=json.loads(prefix.with_suffix('.json').read_text())
                        actual=[{k:v for k,v in d.items() if k!='role'} for d in native['queue']]
                        assert actual==oracle['queue'] and native['owners']==oracle['owners'] and native['tiles']==tileInfo
                        assert all(d['role']==0 for d in native['queue']) and native['map']=={'path':request,'width':mw,'height':mh,'layers':ml,'region':','.join(map(str,region))}
                        assert prefix.with_suffix('.565').read_bytes()==words and native['rgba_sha256']==hashlib.sha256(rgba).hexdigest() and native['remaining_surfaces']==0
                        runs.append({'map':name,'region':region,'view':view,'overlap':overlap,'visibility':visibility,'build':build,'draws':len(actual),'hidden_draws':sum(d['kind']==-2 for d in actual),'all_match':True,'pixels_sha256':sha(prefix.with_suffix('.565'))})
                    index+=1
        # User-controlled slice bounds/options must fail before producing files.
        for bad in ['40,0,0,1,1','0,0,19,1,1','0,0,0,0,1','0,0,0,4,1','0,0,0,1','-1,0,0,1,1']:
            prefix=out/'rejected';run=subprocess.run([str(binaries[0]),'--root',str(root),'--map','Realms/Celtic/Forest/CFsec01.map','--region',bad,'--output',str(prefix)],env=env,capture_output=True,text=True,timeout=30);assert run.returncode!=0 and not list(out.glob('rejected.*'))
        assert all(sha(p)==h for p,h in inputs.items())
        report={'all_match':True,'live_validated':False,'cases':index,'frames':len(runs),'builds':len(binaries),'rejected_regions':6,'runs':runs,'source_snapshot':str(source.relative_to(REPO)),'source_and_input_sha256':{str(p.relative_to(REPO)):h for p,h in inputs.items()},'helper_sha256':sha(helper),'scope':'Installed MAP slices in layers 0,1,6; original ordinary terrain producer/sort/visibility with object references disabled and raw flags retained; independent unshaded SPR pixel oracle'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print({'cases':index,'frames':len(runs),'hidden_draws':sum(r['hidden_draws'] for r in runs)},flush=True)
    finally:verify('after')
if __name__=='__main__':main()
