#!/usr/bin/env python3
"""Prove bounded primary kind8 class admission with owned packets and original pixels."""
import argparse,hashlib,importlib.util,json,os,struct,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
from kind8_sprite_fixture import BUILD
ROOT=Path(__file__).resolve().parents[1];BID='NP.world-kind8-sprite-admission'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def declaration():
    r=json.load(open(ROOT/'research/runtime/coverage/register.json'));b=next(b for b in r['behaviors'] if b['id']==BID);s=next(s for s in r['scenarios'] if s['id']=='native-kind8-sprite-admission');paths=required_sources(b)|set(s['tests'])
    for directory in ['runtime/scene','runtime/shadow','protocols','compat/legacy','renderer','assets']:
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ['.h','.hpp','.c','.cpp','.S','.py','.json'] or p.name=='CMakeLists.txt'))
    return [dict(behavior=BID,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={s['id']:scenario_contract(s)})],{p:sha(ROOT/p) for p in sorted(paths)}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--build-dir',type=Path,required=True);p.add_argument('--hook-report',type=Path,required=True);a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh output below working required')
    out.mkdir(parents=True);claims,sources=declaration();report=dict(schema=1,success=False,claims=claims,sources=sources,original_sha256=BUILD,scope='Exact two sprite classes under original primary virtual call, six context PCs/ten tag pairs, protected batch owner thread only. Actual Win32 protected entry ABI and original virtual fragment/method execution with synthetic RNG/pixel tails; Linux full guard controls and144000original method traces; owned actual CPS and canonical batch decoded into GPU pixels versus independently executed original five pixel entries from authored zero history. No natural reachability, original RNG/lifetime/writers, other modes/classes, complete drawing or live release claim.',live_replacement=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,cmd):
        with (out/(name+'.stdout')).open('xb') as f,(out/(name+'.stderr')).open('xb') as e:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=f,stderr=e,check=True,timeout=900)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify']);exe=ROOT/'original/Arcane_Nocd/Chaos.exe'
    try:
        if sha(exe)!=BUILD:raise ValueError('Wrong original')
        hook=json.load(open(a.hook_report))
        if not hook.get('success') or hook.get('sprite_primary_calls')!=10 or hook.get('sprite_guard_controls')!=708 or not hook.get('sprite_original_canvas_protected_unchanged'):raise ValueError('Missing actual PE32 proof')
        if any(sha(ROOT/path)!=h for path,h in hook['sources'].items()) or any(sha(Path(path))!=h for path,h in hook['sprite_artifacts'].items()):raise ValueError('PE32 proof source/artifact changed')
        run('production-observer-build',['python3',ROOT/'tools/build-scene-observer.py'])
        report.update(hook_record=str(a.hook_report),hook_sha256=sha(a.hook_report),hook_sources=hook['sources'],hook_artifacts=hook['sprite_artifacts'])
        # Extract the production admission wrapper, keeping the original ordinary
        # policy compiled as-is. No test implementation shadows this predicate.
        source=(ROOT/'runtime/scene/world_producer_bypass.c').read_text();begin=source.index('static int raster_caller(');end=source.index('static u32 bypass_hash',begin);batch=source.index('static int raster_batch_caller(')
        (out/'sprite-batch-caller.inc').write_text(source[begin:end]+source[batch:])
        observer=(ROOT/'runtime/scene/world_trace.c').read_text();start=observer.index('static int kind8_supported(');stop=observer.index('void world_begin(',start);(out/'sprite-observer-caller.inc').write_text(observer[start:stop])
        empty=bytearray(64);empty[:8]=b'MNMPRO01';struct.pack_into('<5I',empty,8,1,64,0x40209ca7,32768,1)
        for sequence,op in [(1,11),(2,12)]:
            row=[0]*24;row[0]=96;row[1]=sequence;row[2]=op;row[14]=1;empty.extend(struct.pack('<24I',*row))
        (out/'empty.bin').write_bytes(empty)
        run('generator',['python3',ROOT/'tools/inspect-world-raster-callers.py','--stream',out/'empty.bin','--header',out/'generated.h','--report',out/'generated.json'])
        guard=[]
        for mode in ['production','generated']:
            extra=['-DMNM_SPRITE_CALLER_HEADER="generated.h"','-DMNM_EXPECT_CALLERS=12','-DMNM_EXPECT_PAIRS=12'] if mode=='generated' else []
            run(mode+'-guard-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie','-I'+str(out),*extra,ROOT/'tests/kind8-sprite-admission-test.cpp','-o',out/(mode+'-guard')]);run(mode+'-guard',[out/(mode+'-guard'),exe]);g=json.loads((out/(mode+'-guard.stdout')).read_text());guard.append(g)
            if not g['success'] or g['context_pairs']!=10 or g['protected_short_spans']!=628 or not g['legacy_sprite_transport_refused']:raise ValueError('Incomplete guard proof')
        for mode in ['native','original']:
            run(mode+'-primary-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-omit-frame-pointer','-fno-pie','-no-pie','-DMNM_SPRITE_PRIMARY_FRAGMENT',*(['-DMNM_ORIGINAL_REFERENCE'] if mode=='original' else []),ROOT/'tests/kind8-sprite-method-test.cpp','-o',out/(mode+'-primary')]);run(mode+'-primary',[out/(mode+'-primary'),*([exe] if mode=='original' else [])])
        native=(out/'native-primary.stdout').read_bytes()
        if len(native)!=144000*140 or native!=(out/'original-primary.stdout').read_bytes() or (out/'original-primary.stderr').read_text()!='primary_context_hits=17344\n':raise ValueError('Primary method trace mismatch')
        spec=importlib.util.spec_from_file_location('inspect_producers',ROOT/'tools/inspect-canvas-producers.py');inspector=importlib.util.module_from_spec(spec);spec.loader.exec_module(inspector)
        _,operations=inspector.decode(Path(hook['sprite_fixture']).read_bytes(),complete=False);definitions={r[1]:payload for r,payload in operations if r[2]==25};entries=[0x5947b2,0x59521a,0x57de00,0x597086,0x596cb8]*3;records=[]
        for row,payload in operations:
            if row[2]!=9:continue
            row=list(row);payload=definitions[row[4]] if row[4] else payload;row[4]=0;row[18]=len(payload);row[0]=96+len(payload);row[22]=entries[len(records)]
            path=out/('oracle-record-'+str(len(records))+'.bin');path.write_bytes(struct.pack('<24I',*row)+payload);records.append(path)
        if len(records)!=15:raise ValueError('Incomplete owned pixel corpus')
        run('original-pixels-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/kind8-sprite-pixel-reference.cpp','-o',out/'original-pixels']);run('original-pixels',[out/'original-pixels',exe,out/'expected.565',*records]);pixels=json.loads((out/'original-pixels.stdout').read_text())
        if pixels!={'success':True,'draws':15,'generic':6,'clipped':3,'word':6}:raise ValueError('Original pixel route counts changed')
        run('configure',['cmake','-S',ROOT/'compat/legacy/canvas-producers','-B',a.build_dir,'-DCMAKE_BUILD_TYPE=Debug']);run('build',['cmake','--build',a.build_dir,'--target','kind8-sprite-producer-test','-j4']);run('gpu',['xvfb-run','-a','--server-num='+str(20000+os.getpid()%30000),a.build_dir/'kind8-sprite-producer-test',hook['sprite_fixture'],hook['sprite_batch'],out/'expected.565']);gpu=json.loads((out/'gpu.stdout').read_text())
        if not gpu['success'] or gpu['actual_owned_draws']!=15 or gpu['original_pixel_words']!=480000 or gpu['ordinary_readbacks']:raise ValueError('Incomplete owned GPU proof')
        if declaration()!=(claims,sources) or sha(exe)!=BUILD:raise ValueError('Source/contract/input changed')
        report.update(success=True,sources_stable=True,primary_original_cases=144000,primary_context_hits=17344,primary_trace_sha256=sha(out/'native-primary.stdout'),guard=guard,actual_pe32_primary_calls=10,actual_pe32_direct_abi_entries=5,actual_pe32_guard_controls=708,actual_pe32_protected_short_spans=628,actual_pe32_random_calls=15,actual_pe32_destination_unchanged=True,original_pixels=pixels,gpu=gpu,owned_source_frames=2,ordinary_callers_unchanged=True,protected_batch_only=True)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
