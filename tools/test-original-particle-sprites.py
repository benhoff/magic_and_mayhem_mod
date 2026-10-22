#!/usr/bin/env python3
"""Compare every remaining original particle sprite draw method and full RNG/pixels."""
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
    for kind in range(4):
        yield from ((kind,*row) for row in itertools.product(range(6),
            (0,1,0x12345678,0xffffffff,0x80000000,0xdeadbeef,50,51,99),
            range(12 if kind==3 else 1),((-1,-1),(2,3),(12,11),(28,22)),
            (0,1),(-127,-8,127),(0,1),(0,1)))


def compare(native,original,mode):
    procs=[]
    try:
        procs=[subprocess.Popen(['zstd','-qdc',p],stdout=subprocess.PIPE) for p in (native,original)]
        digests=[hashlib.sha256(),hashlib.sha256()];size=events=0
        for case,row in enumerate(cells()):
            kind,gate,seed,phase,xy,palette,shade,format_,split=row
            expected=(case,kind,gate,seed,phase,xy[0]&0xffffffff,xy[1]&0xffffffff,palette,shade&0xffffffff,format_,split,0)
            records=[]
            for index,proc in enumerate(procs):
                head=proc.stdout.read(52)
                if len(head)!=52:raise ValueError(f'{mode} incomplete case {case}')
                fields=struct.unpack('<13I',head)
                if fields[:12]!=expected or fields[12]>12 or fields[12]%6:raise ValueError(f'{mode} invalid case identity {case}')
                body=proc.stdout.read(1012+(fields[12]*4 if mode=='trace' else 2400))
                if len(body)!=1012+(fields[12]*4 if mode=='trace' else 2400):raise ValueError('Truncated state')
                record=head+body;digests[index].update(record);records.append(record)
            if records[0]!=records[1]:
                at=next(i for i,(x,y) in enumerate(zip(*records)) if x!=y)
                raise ValueError(f'{mode} case {case}, {row}, record byte {at}, native {records[0][max(0,at-12):at+20].hex()}, original {records[1][max(0,at-12):at+20].hex()}')
            size+=len(records[0]);events+=struct.unpack_from('<I',records[0],48)[0]//6
        if case+1!=77760 or any(p.stdout.read(1) for p in procs):raise ValueError('Unexpected denominator or trailing bytes')
        if any(p.wait(timeout=10) for p in procs):raise ValueError('Decompression failed')
        return dict(mode=mode,cases=case+1,ordered_calls=events,bytes_compared=size,sha256=digests[0].hexdigest())
    finally:
        for p in procs:
            if p.poll() is None:p.kill();p.wait()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):parser.error('Require new output in working/tests')
    out.mkdir(parents=True);(out/'tmp').mkdir();env=dict(os.environ,TMPDIR=str(out/'tmp'))
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());bs={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    ids=['RS.particle-sprite-composition']
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
                ROOT/'tests/particle-sprite-composition.cpp',ROOT/'reconstruction/rendering/palette_shading.cpp','-o',out/name])
            for mode in ('trace','pixels'):
                run(name+'-'+mode,[out/name,*([executable] if name=='original' else []),mode],True)
        report.update(results=[compare(out/('native-'+m+'.bin.zst'),out/('original-'+m+'.bin.zst'),m) for m in ('trace','pixels')],success=True,
            original_particle_draw_and_rng_bodies_unchanged=True,trace_raster_boundaries_recording_only=True,pixel_raster_bodies_unchanged=True,
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
