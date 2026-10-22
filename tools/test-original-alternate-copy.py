#!/usr/bin/env python3
"""Compare complete original alternate-buffer physical copy, padding and alias ordering."""
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


def compare(native,original,mode):
    procs=[]
    try:
        procs=[subprocess.Popen(['zstd','-qdc',p],stdout=subprocess.PIPE) for p in (native,original)]
        digest=hashlib.sha256();size=0
        for case,row in enumerate(itertools.product((4,8,16,32),(1,2,5),(-4,-1,0,1,4),(-5,-3,-1,0,1,2,3,4,5,64,256),range(3))):
            records=[p.stdout.read(2072) for p in procs]
            if any(len(r)!=2072 for r in records):raise ValueError(f'Truncated copy case{case}')
            if struct.unpack('<6I',records[0][:24])!=(case,*[x&0xffffffff for x in row]):raise ValueError('Copy identity')
            if records[0]!=records[1]:raise ValueError(f'Copy mismatch case{case} {row}')
            digest.update(records[0]);size+=2072
        if case+1!=1980 or any(p.stdout.read(1) for p in procs):raise ValueError('Unexpected copy denominator/trailing bytes')
        if any(p.wait(timeout=10) for p in procs):raise ValueError('Decompression failed')
        return dict(cases=case+1,physical_words_compared=(case+1)*1024,bytes_compared=size,native_invalid_extent_refusals=4,sha256=digest.hexdigest())
    finally:
        for p in procs:
            if p.poll() is None:p.kill();p.wait()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):parser.error('Require new output in working/tests')
    out.mkdir(parents=True);(out/'tmp').mkdir();env=dict(os.environ,TMPDIR=str(out/'tmp'))
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());bs={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    ids=['RS.alternate-canvas-copy']
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
                ROOT/'tests/alternate-canvas-copy.cpp','-o',out/name])
            for mode in ('copy',):
                run(name+'-'+mode,[out/name,*([executable] if name=='original' else [])],True)
        report.update(results=[compare(out/('native-'+m+'.bin.zst'),out/('original-'+m+'.bin.zst'),m) for m in ('copy',)],success=True,
            original_copy_body_unchanged=True,upstream_allocation_and_writer_provenance_not_claimed=True,
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
