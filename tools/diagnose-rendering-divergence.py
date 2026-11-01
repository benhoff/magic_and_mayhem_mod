#!/usr/bin/env python3
"""Compare source-only native CPU/GPU replay against a pinned original oracle."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import zlib
import numpy as np
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources

ROOT=Path(__file__).resolve().parents[1]

def sha(data):return hashlib.sha256(data).hexdigest()

def compiled_sources(build,target):
    pending=[build/'CMakeFiles'/f'{target}.dir/link.txt'];seen=set();sources=set()
    while pending:
        link=pending.pop().resolve()
        if link in seen:continue
        seen.add(link);base=link.parents[2];objects=0
        for token in shlex.split(link.read_text()):
            path=(base/token).resolve()
            if path.suffix not in ('.o','.a') or not path.is_relative_to(build):continue
            if path.suffix=='.a':
                pending.append(path.parent/'CMakeFiles'/f'{path.stem.removeprefix("lib")}.dir/link.txt');continue
            objects+=1
            for dependency in shlex.split(Path(str(path)+'.d').read_text().replace('\\\n',' ')):
                source=(base/dependency).resolve()
                if source.is_relative_to(ROOT) and not source.is_relative_to(ROOT/'working'):
                    sources.add(source.relative_to(ROOT).as_posix())
        if not objects:raise ValueError('Missing compiler dependency receipts: '+str(link))
    return sources

def differences(actual,expected,width,height):
    if len(actual)!=width*height*2 or len(expected)!=len(actual):raise ValueError('World plane extent')
    a=np.frombuffer(actual,dtype='<u2');b=np.frombuffer(expected,dtype='<u2');indices=np.flatnonzero(a!=b)
    return dict(pixels=width*height,mismatching_pixels=len(indices),actual_sha256=sha(actual),expected_sha256=sha(expected),
                examples=[dict(x=int(i%width),y=int(i//width),actual=int(a[i]),expected=int(b[i])) for i in indices[:8]])

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ['acquisition','comparison','report']:p.add_argument('--'+name,required=True)
    for name in ['assets','build','output']:p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--original-executable',type=Path)
    p.add_argument('--capture-draw',type=int,default=0)
    a=p.parse_args();a.build=a.build.resolve();a.output=a.output.resolve();a.assets=a.assets.resolve()
    if (ROOT/a.report).exists() or a.output.exists():p.error('New immutable report/output required')
    if not a.output.is_relative_to(ROOT/'working'):p.error('Owned ignored execution directory required')
    a.output.mkdir(parents=True)
    result=dict(schema=1,success=False,sources={},inputs={},worlds=[],original_oracles_used_as_native_inputs=False,
                fresh_original_execution=False,nominal_player_accepted=False,
                scope='Source-only complete native CPU/GPU history replay versus retained independent original planes. Diagnostic reads occur after delivery; this is an offline failure isolation, not fresh original execution or player acceptance.')
    result['source_records']={}
    initial=['tools/diagnose-rendering-divergence.py','tests/rendering-divergence-replay.cpp','compat/legacy/world-rolling/CMakeLists.txt',
             'tests/rendering-divergence-reference.cpp','tests/acceptance-world-reference.cpp',
             'tests/sprite-binary-reference.cpp','tests/world-raster-reference.hpp','tests/clipped-word-grayscale-cases.hpp']
    for name in initial:
        data=(ROOT/name).read_bytes();record=str(Path(a.report).with_suffix(''))+'-source-'+sha(name.encode())[:16]+'.bin'
        result['sources'][name]=sha(data)
        result['source_records'][name]=dict(record=record,sha256=write_report(ROOT,record,data))
    before=False
    def run(label,command,env=None):
        path=a.output/(label+'.log')
        with path.open('xb') as out:subprocess.run([str(c) for c in command],cwd=ROOT,env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=300)
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        with Reader(ROOT) as reader:
            def read(name):result['inputs'][name]=reader.digest(name);return reader.json(name)
            acquired=read(a.acquisition);compared=read(a.comparison)
            if not acquired['success'] or not acquired['private_wine_stopped'] or compared['acquisition']['sha256']!=reader.digest(a.acquisition):raise ValueError('Acquisition/comparison binding')
            profiles=[x for x in acquired['artifacts'] if x['path'].endswith('/'+Path(acquired['temporary_root']).name+'/worlds.bin')]
            if len(profiles)!=1:raise ValueError('Unique acquired source profile required')
            artifact=profiles[0]
            data=reader.read_bytes(artifact['record'])
            if sha(data)!=artifact['sha256']:raise ValueError('Profile payload identity')
            profile=a.output/'worlds.bin';profile.write_bytes(data)
            result['inputs'][artifact['record']]=artifact['sha256']
            originals={}
            for artifact in compared['artifacts']:
                path=Path(artifact['path'])
                if path.parent.as_posix()=='reference' and path.suffix=='.qz':
                    data=reader.read_bytes(artifact['record']);result['inputs'][artifact['record']]=artifact['sha256']
                    if sha(data)!=artifact['sha256']:raise ValueError('Original plane identity')
                    inflated=zlib.decompress(data[4:])
                    if len(inflated)!=int.from_bytes(data[:4],'big'):raise ValueError('Original plane inflation')
                    originals[int(path.stem.removeprefix('world-'))]=inflated
            if len(originals)!=compared['original_reference']['worlds']:raise ValueError('Incomplete original cohort')
        asset_hashes=compared['asset_inputs']
        def stable_assets():return all(sha((a.assets/name).read_bytes())==h for name,h in asset_hashes.items())
        if not stable_assets():raise ValueError('Asset inputs differ from retained independent reference')
        result['asset_inputs']=asset_hashes
        run('configure',['cmake','-S',ROOT/'compat/legacy/world-rolling','-B',a.build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
        run('build',['cmake','--build',a.build,'--target','rendering-divergence-replay','--parallel','4'])
        sources=compiled_sources(a.build,'rendering-divergence-replay')|set(initial)|{'tools/coverage_claims.py'}
        result['sources']={n:sha((ROOT/n).read_bytes()) for n in sorted(sources)}
        register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        behavior=next(b for b in register['behaviors'] if b['id']=='TOOL.rendering-first-divergence')
        result['claims']=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{}),scenarios={})]
        if not required_sources(behavior)<=sources:raise ValueError('Incomplete behavior source closure')
        binary=a.build/'rendering-divergence-replay';result['binary_sha256']=sha(binary.read_bytes())
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4')
        if a.capture_draw:env['MNM_DIVERGENCE_CAPTURE_DRAW']=str(a.capture_draw)
        native=a.output/'native';run('native',['xvfb-run','-a',binary,profile,a.assets,native],env)
        measured=json.loads((native/'native.json').read_text());result['native']=measured
        if a.original_executable:
            executable_hash=sha(a.original_executable.read_bytes())
            if executable_hash!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':raise ValueError('Unsupported original binary')
            result['source_executable_sha256']=executable_hash
            original=a.output/'original-reference'
            run('original-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/rendering-divergence-reference.cpp','-lcrypto','-o',original])
            result['original_reference_binary_sha256']=sha(original.read_bytes())
            original_env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
            if a.capture_draw:original_env.update(MNM_DIVERGENCE_CAPTURE_DRAW=str(a.capture_draw),MNM_DIVERGENCE_ORIGINAL_PREFIX=str(a.output/'selected-original'))
            original_output=a.output/'original-world50.565'
            run('original-trace',[original,a.original_executable,native/'world-50-draws.bin',native/'world-50-cpu-before.565',original_output],original_env)
            original_trace=[]
            for line in (a.output/'original-trace.log').read_text().splitlines():
                if not line.startswith('DRAW '):continue
                _,ordinal,entry,kind,mode,backend,digest=line.split()
                original_trace.append(dict(ordinal=int(ordinal),entry=int(entry,16),kind=int(kind),mode=int(mode),backend=int(backend),sha256=digest))
            if len(original_trace)!=len(measured['trace']) or not original_trace:raise ValueError('Closed original/native trace cohort required')
            divergent=[]
            for number,(actual,expected) in enumerate(zip(measured['trace'],original_trace),1):
                if actual['ordinal']!=number or any(actual[k]!=expected[k] for k in ['ordinal','entry','kind','mode','backend']):raise ValueError('Original/native draw identity differs')
                if actual['sha256']!=expected['sha256']:divergent.append(dict(native=actual,original=expected))
            result['first_divergent_draw']=divergent[0] if divergent else None
            result['divergent_draw_checkpoints']=len(divergent)
            result['exact_prefix_checkpoint_matches']=divergent[0]['native']['ordinal']-1 if divergent else len(original_trace)
            result['fresh_original_world50_matches_retained']=original_output.read_bytes()==originals[50]
            result['fresh_original_world50_matches_native']=original_output.read_bytes()==(native/'world-50-cpu.565').read_bytes()
            result['original_entry_scope']='Source-derived native entry plane, original-only execution; full-prefix independent comparison is a separate result.'
            result['fresh_original_execution']=True
            result['original_executable_stable']=sha(a.original_executable.read_bytes())==executable_hash
            if not result['original_executable_stable']:raise ValueError('Original binary changed')
        for row in measured['worlds']:
            g=row['generation'];width=row['width'];height=row['height'];cpu=(native/f'world-{g}-cpu.565').read_bytes();gpu=(native/f'world-{g}-gpu.565').read_bytes()
            result['worlds'].append(dict(world=g,cpu_original=differences(cpu,originals[g],width,height),gpu_original=differences(gpu,originals[g],width,height),gpu_cpu=differences(gpu,cpu,width,height)))
        result['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in result['sources'].items())
        result['asset_inputs_stable']=stable_assets();result['success']=measured['success'] and result['sources_stable'] and result['asset_inputs_stable']
    except Exception as error:result['error']=str(error)
    finally:
        if before:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);result['original_manifest_verified_before_after']=True
            except Exception as error:result.update(success=False,original_manifest_error=str(error))
        result['artifacts']=[]
        for f in sorted(a.output.rglob('*')):
            if f.is_file():
                name=str(Path(a.report).with_suffix(''))+'-'+sha(str(f.relative_to(a.output)).encode())[:20]+'.bin';data=f.read_bytes()
                result['artifacts'].append(dict(path=str(f.relative_to(a.output)),record=name,sha256=write_report(ROOT,name,data),bytes=len(data)))
        write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    failed={key:[dict(world=r['world'],mismatching_pixels=r[key]['mismatching_pixels'],examples=r[key]['examples'][:1]) for r in result['worlds'] if r[key]['mismatching_pixels']] for key in ['cpu_original','gpu_original','gpu_cpu']}
    print(json.dumps(dict(success=result['success'],error=result.get('error'),divergences=failed,first_divergent_draw=result.get('first_divergent_draw'))),flush=True)
    return 0 if result['success'] else 1

if __name__=='__main__':raise SystemExit(main())
