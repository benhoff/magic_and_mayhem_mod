#!/usr/bin/env python3
"""Compare complete original, native and installed world cell production."""
import argparse
import ast
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
IDS=['RS.world-cell-terrain-production','RS.world-cell-effect-production',
     'RS.world-cell-artifact-production','RS.world-cell-creature-production',
     'RS.world-cell-queue-publication','NP.native-world-cell-producer-adapter']
CASES=3040
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def mismatch(out,left,right):
    processes=[subprocess.Popen(['zstd','-q','-dc',str(out/(mode+'.bin.zst'))],stdout=subprocess.PIPE) for mode in (left,right)]
    try:
        while True:
            headers=[p.stdout.read(8) for p in processes]
            if not any(headers):return None
            if any(len(h)!=8 for h in headers):return dict(error='Incomplete case envelope')
            shapes=[struct.unpack('<II',h) for h in headers]
            data=[p.stdout.read(shape[1]) for p,shape in zip(processes,shapes)]
            if shapes[0]!=shapes[1] or data[0]!=data[1]:
                for mode,body in zip((left,right),data):(out/(mode+'-first-mismatch.bin')).write_bytes(body)
                offset=next((i for i,(a,b) in enumerate(zip(*data)) if a!=b),min(map(len,data)))
                return dict(left=left,right=right,left_case=shapes[0][0],right_case=shapes[1][0],
                    left_bytes=shapes[0][1],right_bytes=shapes[1][1],first_offset=offset,
                    left_hex=data[0][max(0,offset-16):offset+32].hex(),right_hex=data[1][max(0,offset-16):offset+32].hex())
    finally:
        for p in processes:
            p.stdout.close()
            if p.poll() is None:p.terminate()
            p.wait()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args();out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):parser.error('Require new retained output under working/tests')
    (out/'tmp').mkdir(parents=True);env=dict(os.environ,TMPDIR=str(out/'tmp'));resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behaviors={b['id']:b for b in r['behaviors']};builds={b['id']:b for b in r['builds']}
    names=set().union(*(required_sources(behaviors[n]) for n in IDS))|{'tools/coverage_claims.py'};sources={p:sha(ROOT/p) for p in sorted(names)}
    report=dict(schema=1,success=False,sources=sources,original_sha256=PIN,
        claims=[dict(behavior=n,contract_sha256=behavior_contract(behaviors[n],builds)) for n in IDS],
        scope=__doc__,live_replacement=False,original_outputs_used_as_native_inputs=False,
        original_producer_and_queue_poisoned_for_native=True,
        external_services='Explicit owned animation-result, gameplay visibility/iteration, RNG/math and picking providers. Actual external engine service bodies are not replaced by this comparison.')
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label,command):
        print(label,flush=True)
        with (out/(label+'.log')).open('x') as f:subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=300)
    before=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True;original=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(original)!=PIN:raise ValueError('Unsupported original')
        run('adapter-build',['clang++','-m32','-std=c++17','-O2','-fwrapv','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-DMNM_WORLD_CELL_TEST','-D__declspec(x)=','-c',ROOT/'runtime/world-cell-producer/adapter.cpp','-o',out/'adapter.o'])
        run('entry-build',['clang','-m32','-c',ROOT/'runtime/world-cell-producer/entry.S','-o',out/'entry.o'])
        run('fixture-build',['g++','-m32','-std=c++17','-O2','-fwrapv','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',ROOT/'tests/world-cell-composition.cpp',ROOT/'tests/world-cell-services.S',out/'adapter.o',out/'entry.o','-o',out/'composition'])
        results={}
        for mode in ['original','native','runtime']:
            print(mode,flush=True);digest=hashlib.sha256();size=0
            with (out/(mode+'.stderr')).open('xb') as error:
                process=subprocess.Popen([str(out/'composition'),str(original),mode],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=error)
                compressor=subprocess.Popen(['zstd','-q','-3','-T1','-o',str(out/(mode+'.bin.zst'))],stdin=subprocess.PIPE)
                try:
                    while data:=process.stdout.read(1<<20):digest.update(data);size+=len(data);compressor.stdin.write(data)
                    compressor.stdin.close()
                    if process.wait(timeout=30) or compressor.wait(timeout=30):raise ValueError('Producer execution failed: '+mode)
                finally:
                    if process.poll() is None:process.kill();process.wait()
                    if compressor.poll() is None:compressor.kill();compressor.wait()
                if (out/(mode+'.stderr')).read_text().strip()!=f'world cell cases {CASES}':raise ValueError('Incomplete producer cases')
            results[mode]=dict(bytes=size,sha256=digest.hexdigest(),compressed_sha256=sha(out/(mode+'.bin.zst')))
        report['results']=results
        if len({(d['bytes'],d['sha256']) for d in results.values()})!=1:
            report['mismatch']=mismatch(out,'original','native') or mismatch(out,'original','runtime');raise ValueError('Producer traces differ')
        run('guards',[out/'composition',original,'guards']);controls=json.loads((out/'guards.log').read_text())
        if not controls['success'] or controls['body_and_table_mutations']!=13472:raise ValueError('Incomplete producer guards')
        run('pe32-build',['python3',ROOT/'tools/build-world-cell-producer.py','--output',out/'pe32'])
        manifest=json.loads((out/'pe32/manifest.json').read_text());tree=ast.parse((ROOT/'tools/capture-scene-game.py').read_text())
        declarations=[statement.value for node in ast.walk(tree) if isinstance(node,ast.If) and isinstance(node.test,ast.Attribute)
            and node.test.attr=='native_world_cell_producer' for statement in node.body if isinstance(statement,ast.AugAssign)
            and isinstance(statement.target,ast.Name) and statement.target.id=='sources']
        if len(declarations)!=1:raise ValueError('Ambiguous producer source declaration')
        declared={p.relative_to(ROOT).as_posix() for p in eval(compile(ast.Expression(declarations[0]),'<capture producer inputs>','eval'),{'ROOT':ROOT})}
        if set(manifest['sources'])-declared or any(sources.get(p)!=h for p,h in manifest['sources'].items()):raise ValueError('Capture/compiler/prospective inputs differ')
        spec=importlib.util.spec_from_file_location('cell_import_stage',ROOT/'tools/prepare-shadow-experiment.py');stage=importlib.util.module_from_spec(spec);spec.loader.exec_module(stage)
        raw=original.read_bytes();staged=stage.add_import(raw,'MnmWorldCellProducer.dll','WorldCellProducerAnchor',b'.mnwcell');pe=struct.unpack_from('<I',raw,60)[0];table=pe+24+struct.unpack_from('<H',raw,pe+20)[0]
        for n in range(struct.unpack_from('<H',raw,pe+6)[0]):
            length,offset=struct.unpack_from('<II',raw,table+40*n+16)
            if staged[offset:offset+length]!=raw[offset:offset+length]:raise ValueError('Import staging modified an original section')
        (out/'staged.exe').write_bytes(staged)
        for name in ['capture-scene-game','prepare-scene-observer']:
            run(name,['python3','-B',ROOT/('tools/'+name+'.py'),'--help'])
            if '--native-world-cell-producer' not in (out/(name+'.log')).read_text():raise ValueError('Missing producer option')
        report.update(success=True,cases=CASES,guards=controls,capture_compiler_sources_match=True,import_staging_pass=True,
            pe32_dll_sha256=sha(out/'pe32/MnmWorldCellProducer.dll'),fixture_binary_sha256=sha(out/'composition'))
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if before:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items());report['success'] &= report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
