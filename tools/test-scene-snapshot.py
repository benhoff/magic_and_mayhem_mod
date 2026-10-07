#!/usr/bin/env python3
"""Check bounded PE32 observation and native snapshot replay against an independent SPR oracle."""
import argparse, hashlib, importlib.util, json, os, struct, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
    paths=set()
    for directory in ['assets','renderer','compat/legacy','runtime/scene']:
        for p in (ROOT/directory).rglob('*'):
            if p.is_file() and (p.suffix in ['.cpp','.hpp','.c','.h','.S'] or p.name in ['CMakeLists.txt','README.md']):paths.add(p)
    paths.update(ROOT/p for p in ['runtime/shadow/win32_min.h','protocols/include/mnm/scene_snapshot_v1.h','tests/scene-snapshot-test.cpp','tests/resource-fixtures.hpp','tools/build-scene-observer.py','tools/build-shadow-bridge.py','tools/test-scene-snapshot.py','tools/test-terrain-preview.py','research/formats/scene-snapshot-v1.md','research/runtime/native-scene-snapshot.md'])
    return {str(p.relative_to(ROOT)):sha(p) for p in sorted(paths)}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--capture-report',type=Path);p.add_argument('--bindings',type=Path);p.add_argument('--root',type=Path,default=ROOT/'working/game-clean');args=p.parse_args()
    if not os.environ.get('DISPLAY'):p.error('Run under xvfb-run -a')
    if bool(args.capture_report)!=bool(args.bindings):p.error('Supply both --capture-report and --bindings')
    args.root=args.root.resolve()
    if args.capture_report:
        args.capture_report=args.capture_report.resolve();args.bindings=args.bindings.resolve()
    parent=ROOT/'working/tests/scene-snapshot';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report={'success':False,'sources':sources(),'runs':[],'scope':'Bounded observation forwarding and native unshaded replay; independent native policy pixels, no original full-frame comparison or replacement'}
    env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all')
    def run(name,command,expected=0,extra=None):
        with (out/(name+'.log')).open('x') as log:r=subprocess.run([str(c) for c in command],cwd=ROOT,env=extra or env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
        if (r.returncode==0)!=(expected==0):raise RuntimeError(name+' unexpected exit '+str(r.returncode))
    def verify(phase):run('original-'+phase,[ROOT/'tools/original-manifest.sh','verify'])
    build=ROOT/'working/build/scene-snapshot';preview=build/'mnm-scene-snapshot-preview'
    inputs={}
    if args.capture_report:verify('before')
    try:
        run('configure',['cmake','-S',ROOT/'compat/legacy','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','scene-snapshot-test','mnm-scene-snapshot-preview','scene-renderer-test','resource-cache-test','-j4'])
        run('ctest',['ctest','--test-dir',build,'-R','^(native-scene-snapshot|native-shared-scene-renderer|native-resource-cache)$','--output-on-failure'])
        run('pe32-build',['python3',ROOT/'tools/build-scene-observer.py','--selftest'])
        fixture=out/'fixture';fixture.mkdir();capture=out/'capture';capture.mkdir()
        prefix=ROOT/'working/tests/scene-selftest-wine'
        if not (prefix/'system.reg').is_file():raise RuntimeError('Initialize a dedicated 32-bit-capable Wine prefix and place it at working/tests/scene-selftest-wine')
        testenv=dict(env,WINEPREFIX=str(prefix),MNM_SCENE_DIR='Z:'+str(capture).replace('/','\\'),MNM_SCENE_SAMPLES='4')
        run('pe32-selftest',['wine',ROOT/'working/build/scene-observer-selftest/selftest.exe'],extra=testenv)
        captures=sorted(capture.glob('scene-*.bin'));assert len(captures)==4
        blob=captures[0].read_bytes();assert struct.unpack_from('<4I',blob,20)==(1,3,1,2)
        assert struct.unpack_from('<2I',blob,56)==(8,4)
        at=struct.unpack_from('<I',blob,52)[0];token,size,indexed,reserved=struct.unpack_from('<4I',blob,at)
        assert (token,size,indexed,reserved)==(1,54,0,0)
        raw=bytearray(blob[at+16:]);struct.pack_into('<I',raw,28,0xffffffff)
        sprite=struct.pack('<7I',0x00525053,28+len(raw),4,1,0,0,0)+raw;(fixture/'fixture.spr').write_bytes(sprite)
        bindings=out/'fixture-bindings.json';bindings.write_text(json.dumps([{'id':'fixture','sprite':'fixture.spr','sha256':sha(fixture/'fixture.spr')}]))
        spec=importlib.util.spec_from_file_location('oracle',ROOT/'tools/test-terrain-preview.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
        def check_snapshot(name,path,assetroot,bindingpath,partial):
            inputs[path]=sha(path);inputs[bindingpath]=sha(bindingpath)
            bindings_data=json.loads(bindingpath.read_text());byid={}
            for binding in bindings_data:
                source=assetroot/binding['sprite'];assert sha(source)==binding['sha256'];inputs[source]=sha(source)
                byid['ui:'+binding['id']]={'sprite':source.read_bytes(),'frames':{}}
            output=out/name
            command=[preview,'--root',assetroot,'--snapshot',path,'--bindings',bindingpath,'--output',output,'--unshaded']
            if partial:command+=['--supported-only']
            run(name,command);native=json.loads(output.with_suffix('.json').read_text());data=path.read_bytes();count=struct.unpack_from('<I',data,24)[0]
            rows=[struct.unpack_from('<IiiiiIII',data,64+i*32) for i in range(count)]
            frames={};offset=struct.unpack_from('<I',data,52)[0]
            for _ in range(struct.unpack_from('<I',data,28)[0]):
                tok,n,ix,_=struct.unpack_from('<4I',data,offset);frames[tok]=(ix,data[offset+16:offset+16+n]);offset+=16+n
            supported=[r for r in rows if r[4] in [0,33] and r[0] and r[6]==0x8ad08ad0]
            assert len(native['draws'])==len(supported) and native['unmapped']==0
            assert native['hidden']==sum(r[4]==-2 for r in rows)
            expected=[0x1234]*(native['width']*native['height'])
            for row,draw in zip(supported,native['draws']):
                assert (draw['x'],draw['y'])==(row[2],row[3])
                asset=byid[draw['id']];b=asset['sprite'];n,palettes=struct.unpack_from('<II',b,12);base=24+palettes*768+n*4;start=base+struct.unpack_from('<I',b,24+palettes*768+draw['frame']*4)[0];length=struct.unpack_from('<I',b,start)[0];encoded=bytearray(b[start:start+length]);encoded[28:32]=b'\0'*4
                assert frames[row[0]]==(int(bool(palettes)),bytes(encoded))
                ox,oy,pixels=oracle.frame(asset,draw['frame'])
                for x,y,word in pixels:
                    x+=draw['x']-ox;y+=draw['y']-oy
                    if 0<=x<native['width'] and 0<=y<native['height']:expected[y*native['width']+x]=word
            expected_bytes=struct.pack('<'+'H'*len(expected),*expected)
            assert output.with_suffix('.565').read_bytes()==expected_bytes and native['remaining_surfaces']==0
            report['runs'].append({'name':name,'snapshot_sha256':sha(path),'records':count,'draws':len(supported),'hidden':native['hidden'],'unsupported':native['unsupported'],'unmapped':0,'shaded_records':native['shaded_records'],'gaps':native['gaps'],'complete_mapping':native['complete_mapping'],'complete_native_policy_pixels_match':True,'pixel_sha256':sha(output.with_suffix('.565'))})
            return command
        for i,path in enumerate(captures):
            assert struct.unpack_from('<I',path.read_bytes(),20)[0]==i+1
            check_snapshot('fixture-'+str(i),path,fixture,bindings,False)
        command=[preview,'--root',fixture,'--snapshot',captures[0],'--bindings',bindings,'--output',out/'refusal']
        run('requires-unshaded',command,expected=2)
        protected=out/'fixture-0.565';before=sha(protected)
        run('existing-output',[*command[:-1],out/'fixture-0','--unshaded'],expected=2);assert sha(protected)==before
        damaged=out/'malformed.bin';damaged.write_bytes(captures[0].read_bytes()[:-1]);run('malformed',[preview,'--root',fixture,'--snapshot',damaged,'--bindings',bindings,'--output',out/'malformed','--unshaded'],expected=2)
        if args.capture_report:
            reference=json.loads(args.capture_report.read_text());assert reference['success'] and reference['live_observation'] and reference['sources_stable']
            inputs[args.capture_report]=sha(args.capture_report)
            # Capture/source freshness is independent of native replay fingerprints.
            for source,digest in reference['sources'].items():assert sha(ROOT/source)==digest,source+' capture source stale'
            experiment=Path(reference['experiment'])
            for i,(name,digest) in enumerate(reference['snapshots'].items()):
                path=experiment/name;assert sha(path)==digest
                check_snapshot('original-'+str(i),path,args.root,args.bindings,True)
            first=experiment/next(iter(reference['snapshots']))
            run('strict-incomplete',[preview,'--root',args.root,'--snapshot',first,'--bindings',args.bindings,'--output',out/'strict','--unshaded'],expected=2)
            report['capture_report']=str(args.capture_report.relative_to(ROOT));report['live_observation']=True
        report['sources_stable']=sources()==report['sources'];assert report['sources_stable']
        assert all(sha(path)==digest for path,digest in inputs.items())
        report['inputs']={str(path.relative_to(ROOT)):digest for path,digest in inputs.items()}
        report['binaries']={str(path.relative_to(ROOT)):sha(path) for path in [preview,build/'scene-snapshot-test',ROOT/'working/build/scene-observer-selftest/selftest.exe',ROOT/'working/build/scene-observer-selftest/MnmScene.dll']}
        report['ctests_passed']=3;report['forwarding_selftest_passed']=True;report['original_pixels_compared']=False;report['live_replacement']=False;report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        if args.capture_report:verify('after')
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out/'report.json',flush=True)
    if not report['success']:raise RuntimeError(report['error'])
if __name__=='__main__':main()
