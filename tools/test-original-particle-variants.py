#!/usr/bin/env python3
"""Compare remaining concrete particle constructors, RNG, planes and inherited destruction."""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import struct
import subprocess
from coverage_claims import behavior_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def cells():
    return itertools.product(range(9),range(6),range(3),range(5),(0,1),(0,1,0x12345678,0xffffffff,0x80000000,0xdeadbeef),(-1,2,17),(0,1))


def compare(native,original,mode):
    procs=[]
    try:
        procs=[subprocess.Popen(['zstd','-qdc',p],stdout=subprocess.PIPE) for p in (native,original)]
        digest=hashlib.sha256();size=planes=0
        for case,row in enumerate(cells()):
            records=[]
            for proc in procs:
                record=proc.stdout.read(2920)
                if len(record)!=2920:raise ValueError(f'Incomplete variant case {case}')
                fields=struct.unpack_from('<12I',record)
                if fields[:9]!=(case,*[v&0xffffffff for v in row]) or fields[9]>768 or fields[10]>1 or fields[11]!=fields[10] or struct.unpack_from('<I',record,2916)[0]!=fields[9]*2:
                    raise ValueError(f'Variant identity or allocation extent {case}')
                records.append(record)
            if records[0]!=records[1]:
                at=next(i for i,(x,y) in enumerate(zip(*records)) if x!=y)
                raise ValueError(f'Variant case {case}, {row}, record byte {at}, native {records[0][max(0,at-12):at+20].hex()}, original {records[1][max(0,at-12):at+20].hex()}')
            digest.update(records[0]);size+=2920;planes+=fields[10]
        if case+1!=58320 or any(p.stdout.read(1) for p in procs):raise ValueError('Unexpected denominator/trailing bytes')
        if any(p.wait(timeout=10) for p in procs):raise ValueError('Decompression failed')
        return dict(cases=case+1,complete_planes=planes,bytes_compared=size,sha256=digest.hexdigest())
    finally:
        for p in procs:
            if p.poll() is None:p.kill();p.wait()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):parser.error('Require new output in working/tests')
    out.mkdir(parents=True);(out/'tmp').mkdir();env=dict(os.environ,TMPDIR=str(out/'tmp'))
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());bs={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    ids=['RS.particle-variant-construction']
    names=set().union(*(required_sources(bs[i]) for i in ids))|{'tools/coverage_claims.py'};sources={n:sha(ROOT/n) for n in sorted(names)}
    report=dict(schema=1,success=False,scope=bs[ids[0]]['scope'],sources=sources,
        claims=[dict(behavior=i,contract_sha256=behavior_contract(bs[i],builds)) for i in ids],
        original_sha256=BUILD,original_pixels_used_as_native_inputs=False,live_replacement=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label,command,compressed=False):
        print(label,flush=True)
        with (out/(label+'.log')).open('x') as log:
            if not compressed:
                subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180);return
            with (out/(label+'.bin.zst')).open('xb') as destination:
                encoder=subprocess.Popen(['zstd','-q','-3','-c'],stdin=subprocess.PIPE,stdout=destination,stderr=log)
                process=subprocess.Popen([str(x) for x in command],cwd=ROOT,env=env,stdout=encoder.stdin,stderr=log)
                encoder.stdin.close()
                try:
                    if process.wait(timeout=180):raise ValueError(label+' failed')
                    if encoder.wait(timeout=30):raise ValueError('Compression failed')
                finally:
                    for p in (process,encoder):
                        if p.poll() is None:p.kill();p.wait()
    before=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        executable=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(executable)!=BUILD:raise ValueError('Original build changed')
        for name,flags in [('native',[]),('original',['-m32','-fno-pie','-no-pie','-DMNM_ORIGINAL_REFERENCE'])]:
            run(name+'-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,
                ROOT/'tests/particle-variant-construction.cpp','-o',out/name])
            for mode in ('variants',):
                run(name+'-'+mode,[out/name,*([executable] if name=='original' else [])],True)
        report.update(results=[compare(out/('native-'+m+'.bin.zst'),out/('original-'+m+'.bin.zst'),m) for m in ('variants',)],success=True,
            original_constructor_shape_resource_and_destructor_bodies_unchanged=True,private_owned_seh_chain=True,allocator_boundary_owned=True,actual_original_custom_allocator=False,preloaded_resource_domain=True,
            outputs_sha256={p.name:sha(p) for p in sorted(out.glob('*.bin.zst'))},binaries_sha256={n:sha(out/n) for n in ('native','original')})
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if before:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
