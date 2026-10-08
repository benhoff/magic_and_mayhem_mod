#!/usr/bin/env python3
"""Bound native additive rectangles to independent pinned PE32 execution."""
import argparse,hashlib,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BUILD_HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def declaration():
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behaviors=[b for b in r['behaviors'] if b['id'] in ('RS.world-kind8-additive','RS.world-kind8-colour','RS.world-kind8-noop')];s=next(s for s in r['scenarios'] if s['id']=='native-kind8-additive')
    claims=[{'behavior':b['id'],'contract_sha256':behavior_contract(b,{b['id']:b for b in r['builds']}),'scenarios':{s['id']:scenario_contract(s)}} for b in behaviors]
    # The active raster ABI executable links world_trace/world_entry with test
    # stubs. Other producer objects are only compiled by the observer smoke build.
    # Pin required observer dependencies individually, not unrelated hook bodies.
    paths=set(s['tests'])|set().union(*(required_sources(b) for b in behaviors))
    for directory in ('renderer','assets','compat/legacy','protocols'):
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ('.cpp','.hpp','.h','.c','.S') or p.name=='CMakeLists.txt'))
    return claims,{p:sha(ROOT/p) for p in sorted(paths)}
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--capture',type=Path,help='Closed finite capture directory containing complete kind8 World inputs');args=parser.parse_args()
    parent=ROOT/'working/tests/kind8';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='raster-',dir=parent));print(out,flush=True)
    claims,sources=declaration();report={'success':False,'claims':claims,'sources':sources,'source_executable_sha256':BUILD_HASH,'scope':'Selected RGB565 class wrapper54ab00 and raster545c10: synthetic exhaustive destination words, low16 wrap/saturation, ordered clipped rectangles, asset-free worker and strict wire rejection. No live object simulation, alternate mode or original bypass.'}
    (out/'prospective.json').write_text(json.dumps({'claims':claims,'sources':sources},indent=2)+'\n')
    def run(name,cmd,timeout=600):
        with (out/(name+'.log')).open('x') as f:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=timeout,check=True,env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'})
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        exe=ROOT/'working/game-nocd/Chaos.exe';assert sha(exe)==BUILD_HASH
        raw=exe.read_bytes();pe=struct.unpack_from('<I',raw,60)[0];count=struct.unpack_from('<H',raw,pe+6)[0];optional=struct.unpack_from('<H',raw,pe+20)[0];anchors={}
        for address,expected in [(0x54ac00,'c64140016633c0c20800'),(0x54ab60,'8b4424088b542404505283c11ce80ed2ffffc20800'),(0x546540,'8b416085c074238b'),(0x547d80,'83ec24a1881f6e00'),(0x54ab00,'8b4424088b542404c6414401505283c130e8fab0ffffc20800'),(0x545c10,'83ec1ca1881f6e00')]:
            for i in range(count):
                size,va,stored,offset=struct.unpack_from('<4I',raw,pe+24+optional+i*40+8)
                rva=address-0x400000
                if va<=rva and rva+len(expected)//2<=va+stored:
                    actual=raw[offset+rva-va:offset+rva-va+len(expected)//2].hex();assert actual==expected;anchors[hex(address)]=actual;break
            else:raise ValueError('Original entry not file-backed')
        report['anchors']=anchors
        build=ROOT/'working/build/world-frame';run('configure',['cmake','-S',ROOT/'compat/legacy','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','kind8-raster-test','mnm-world-live','mnm-world-frame-preview','-j4'])
        run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/world-frame-reference.cpp','-o',out/'reference'])
        run('observer-build',['python3',ROOT/'tools/build-scene-observer.py','--selftest'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-DMNM_SCENE_STARTUP_REPLAY_SELFTEST']
        objects=[]
        for name in ['tests/kind8-hook-test.c','tests/canvas-startup-call.S','runtime/scene/world_trace.c','runtime/scene/world_entry.S']:
            obj=out/(Path(name).name+'.obj');run('hook-build-'+Path(name).name,[*flags,'-c',ROOT/name,'-o',obj]);objects.append(obj)
        run('hook-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(out/'hook.exe'),*objects,ROOT/'working/build/scene-observer-selftest/kernel32.lib'])
        run('hook-forwarding',['env','WINEPREFIX='+str(ROOT/'working/tests/scene-selftest-wine'),'WINEDEBUG=-all','wine',out/'hook.exe'])
        report['active_hook_abi_and_class_gates_passed']=True
        run('native',['xvfb-run' ,'-a',build/'kind8-raster-test',out/'fixture'])
        fixtures=[]
        for path in sorted(out.glob('*.bin')):
            prefix=path.with_suffix('');output=Path(str(prefix)+'.original.565');run(prefix.name+'-original',[out/'reference',exe,path,output,Path(str(prefix)+'.before.565')])
            native=Path(str(prefix)+'.native.565');assert native.read_bytes()==output.read_bytes(),path.name+' original pixels differ'
            fixtures.append({'input':path.name,'native_sha256':sha(native),'original_sha256':sha(output),'pixels':native.stat().st_size//2,'equal':True})
        assert len(fixtures)==17
        empty=out/'empty-assets';empty.mkdir();empty_bindings=out/'empty-bindings.json';empty_bindings.write_text('[]\n')
        run('primitive-only-cli',['xvfb-run','-a',build/'mnm-world-frame-preview','--root',empty,'--bindings',empty_bindings,'--snapshot',out/'fixture.bin','--output',out/'primitive-zero'])
        run('primitive-only-original-zero',[out/'reference',exe,out/'fixture.bin',out/'primitive-original-zero.565'])
        assert (out/'primitive-zero.565').read_bytes()==(out/'primitive-original-zero.565').read_bytes()
        report['primitive_only_cli_match']=True
        if args.capture:
            capture=args.capture.resolve();metadata=json.loads((capture/'report.json').read_text());assert metadata['success'] and metadata['source_executable_sha256']==BUILD_HASH
            spec=importlib.util.spec_from_file_location('world_frames',ROOT/'tools/test-world-frames.py');world=importlib.util.module_from_spec(spec);spec.loader.exec_module(world)
            snapshots=sorted((capture/'capture').glob('world-*.bin'));assert 1<=len(snapshots)<=16
            needed={world.identity(raw,h[9]) for p in snapshots for h,raw in world.records(p.read_bytes()) if h[1]<6}
            bindings=out/'bindings.json';bindings.write_text(json.dumps(world.pinned_bindings(capture/'game',needed),indent=2)+'\n')
            captured=[];inputs={str((capture/'report.json').relative_to(ROOT)):sha(capture/'report.json')}
            for path in snapshots:
                rows=world.records(path.read_bytes());additive=sum(h[1]>=6 for h,_ in rows);assert additive>0
                prefix=out/path.stem;before=path.with_suffix('.before.565');after=path.with_suffix('.after.565')
                run(path.stem+'-native',['xvfb-run','-a',build/'mnm-world-frame-preview','--root',capture/'game','--bindings',bindings,'--snapshot',path,'--output',prefix])
                zero=out/(path.stem+'-original-zero.565');observed=out/(path.stem+'-original-before.565')
                run(path.stem+'-original-zero',[out/'reference',exe,path,zero]);run(path.stem+'-original-before',[out/'reference',exe,path,observed,before])
                native=prefix.with_suffix('.565');assert native.read_bytes()==zero.read_bytes(),'Native ordered kind8 queue differs from private original on independent zero background'
                width,height,stride=struct.unpack_from('<3I',path.read_bytes(),24);live=after.read_bytes();cropped=b''.join(live[y*stride*2:(y*stride+width)*2] for y in range(height))
                assert observed.read_bytes()==cropped,'Captured ordered primitives do not reproduce original live output from diagnostic before'
                for p in [path,before,after]:inputs[str(p.relative_to(ROOT))]=sha(p)
                captured.append({'sequence':path.stem,'draws':len(rows),'additive_rectangles':additive,'pixels':width*height,'native_zero_matches_private_original':True,'captured_before_replay_matches_live':True,'native_zero_matches_live':native.read_bytes()==cropped,'native_sha256':sha(native),'original_zero_sha256':sha(zero),'live_cropped_sha256':hashlib.sha256(cropped).hexdigest()})
            for item in json.loads(bindings.read_text()):inputs[str((capture/'game'/item['sprite']).relative_to(ROOT))]=item['sha256']
            assert all(sha(ROOT/p)==h for p,h in inputs.items()),'Captured input changed'
            report.update(captured=captured,capture_inputs=inputs,captured_ordered_queue_match=True)
        assert sha(exe)==BUILD_HASH
        assert declaration()==(claims,sources),'Sources/contracts changed during execution'
        report.update(success=True,sources_stable=True,fixtures=fixtures,pixels_compared=sum(f['pixels'] for f in fixtures),independent_original_pixels_match=True,original_pixels_used_as_native_inputs=False,original_work_bypassed=False,native_integration=True)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
