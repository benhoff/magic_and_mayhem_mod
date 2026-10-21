#!/usr/bin/env python3
"""Compare original, native, and installed runtime actor lifecycles in owned storage."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import resource
import struct
import subprocess
from coverage_claims import behavior_contract,required_sources

ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS=['RS.actor-pool-storage','RS.actor-activation','RS.actor-release-cascade','NP.native-actor-lifecycle-adapter']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):parser.error('Require new output under working/tests')
    (out/'tmp').mkdir(parents=True);env=dict(os.environ,TMPDIR=str(out/'tmp'))
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    names=set().union(*(required_sources(behaviors[i]) for i in IDS))|{'tools/coverage_claims.py'}
    sources={n:sha(ROOT/n) for n in sorted(names)}
    report=dict(schema=1,success=False,sources=sources,original_sha256=PIN,
        claims=[dict(behavior=i,contract_sha256=behavior_contract(behaviors[i],builds)) for i in IDS],
        scope=__doc__,external_services='Owned, explicitly instrumented service boundaries; actual map/gameplay/audio managers are not replaced by this comparison.',
        original_lifecycle_bodies_poisoned_for_native=True,live_replacement=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label,command):
        print(label,flush=True)
        with (out/(label+'.log')).open('x') as log:subprocess.run([str(c) for c in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=240)
    before=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        original=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(original)!=PIN:raise ValueError('Wrong original build')
        run('adapter-build',['clang++','-m32','-std=c++17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-DMNM_ACTOR_TEST','-D__declspec(x)=','-c',ROOT/'runtime/actor-lifecycle/adapter.cpp','-o',out/'adapter.o'])
        run('entry-build',['clang','-m32','-c',ROOT/'runtime/actor-lifecycle/entry.S','-o',out/'entry.o'])
        run('composition-build',['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',ROOT/'tests/actor-lifecycle-composition.cpp',ROOT/'tests/actor-lifecycle-services.S',out/'adapter.o',out/'entry.o','-o',out/'composition'])
        results={}
        for mode in ['original','native','runtime']:
            print(mode,flush=True);digest=hashlib.sha256();size=0
            with (out/(mode+'.stderr')).open('xb') as error:
                process=subprocess.Popen([str(out/'composition'),str(original),mode],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=error)
                compressor=subprocess.Popen(['zstd','-q','-3','-T1','-o',str(out/(mode+'.bin.zst'))],stdin=subprocess.PIPE)
                try:
                    while data:=process.stdout.read(1<<20):digest.update(data);size+=len(data);compressor.stdin.write(data)
                    compressor.stdin.close()
                    if process.wait(timeout=30) or compressor.wait(timeout=30):raise ValueError('Composition execution failed: '+mode)
                finally:
                    if process.poll() is None:process.kill();process.wait()
                    if compressor.poll() is None:compressor.kill();compressor.wait()
                if (out/(mode+'.stderr')).read_text().strip()!='actor lifecycle cases 192':raise ValueError('Incomplete actor cases')
            results[mode]=dict(bytes=size,sha256=digest.hexdigest(),compressed_sha256=sha(out/(mode+'.bin.zst')))
        if len({(r['bytes'],r['sha256']) for r in results.values()})!=1:raise ValueError('Original/native/runtime actor traces differ')
        run('guards',[out/'composition',original,'guards']);guards=json.loads((out/'guards.log').read_text())
        if not guards['success'] or guards['body_mutations']!=7138:raise ValueError('Incomplete guard controls')
        run('pe32-build',['python3',ROOT/'tools/build-actor-lifecycle.py','--output',out/'pe32'])
        spec=importlib.util.spec_from_file_location('actor_stage',ROOT/'tools/prepare-shadow-experiment.py');stage=importlib.util.module_from_spec(spec);spec.loader.exec_module(stage)
        raw=original.read_bytes();staged=stage.add_import(raw,'MnmActorLifecycle.dll','ActorLifecycleAnchor',b'.mnactor')
        pe=struct.unpack_from('<I',raw,60)[0];sections=pe+24+struct.unpack_from('<H',raw,pe+20)[0]
        for i in range(struct.unpack_from('<H',raw,pe+6)[0]):
            size,offset=struct.unpack_from('<II',raw,sections+40*i+16)
            if staged[offset:offset+size]!=raw[offset:offset+size]:raise ValueError('Import staging changed an original section')
        if b'MnmActorLifecycle.dll\0' not in staged:raise ValueError('Missing staged actor import')
        (out/'staged.exe').write_bytes(staged)
        for name in ['prepare-scene-observer','capture-scene-game']:
            run(name,['python3','-B',ROOT/('tools/'+name+'.py'),'--help'])
            if '--native-actor-lifecycle' not in (out/(name+'.log')).read_text():raise ValueError('Missing lifecycle CLI option')
        report.update(success=True,cases=192,results=results,guards=guards,import_staging_pass=True,
            pe32_dll_sha256=sha(out/'pe32/MnmActorLifecycle.dll'),fixture_binary_sha256=sha(out/'composition'),
            retained_reset_histogram_generations_per_case=2)
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if before:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
