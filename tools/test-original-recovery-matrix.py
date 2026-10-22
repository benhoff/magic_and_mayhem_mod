#!/usr/bin/env python3
"""Compare original release or complete kind9 helper/pixel matrices."""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import struct
import subprocess
from coverage_claims import behavior_contract,required_sources

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def validate(path,suite,mode):
    cases=events=0
    with path.open('rb') as f:
        if suite=='release':
            cells=((count,at,first,second,*flags) for count in (1,2,5,128)
                   for at in sorted({0,count//2,count-1}) for first in (1,2,999,1000,1001)
                   for second in (1,2,999,1000,1001) for flags in itertools.product(range(2),repeat=4))
            for expected in cells:
                raw=f.read(60)
                if len(raw)!=60:raise ValueError('Incomplete release cell')
                h=struct.unpack('<15I',raw)
                if h[0]!=cases or h[1:9]!=expected or h[11]>3009 or h[11]%3:
                    raise ValueError('Release denominator/identity/event budget')
                body=f.read(128*64+512+h[11]*4)
                if len(body)!=128*64+512+h[11]*4:raise ValueError('Truncated release state/events')
                cases+=1;events+=h[11]//3
            if cases!=3600:raise ValueError('Release matrix incomplete')
        else:
            for cases in range(72576):
                raw=f.read(8)
                if len(raw)!=8:raise ValueError('Incomplete line cell')
                ordinal,count=struct.unpack('<2I',raw)
                if ordinal!=cases or count>45 or count%5 or (mode!='trace' and count):
                    raise ValueError('Line denominator/trace budget')
                size=count*4 if mode=='trace' else 40*30*2
                if len(f.read(size))!=size:raise ValueError('Truncated line output')
                events+=count//5
            cases+=1
        if f.read(1):raise ValueError('Unexpected trailing comparison data')
    return cases,events


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--suite',choices=('release','line'),required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output under working/tests')
    out.mkdir(parents=True)
    (out/'tmp').mkdir()
    environment=dict(os.environ,TMPDIR=str(out/'tmp'))
    ids=['RS.canvas-release-alias-lifetime'] if a.suite=='release' else ['RS.world-kind9-line']
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bs={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    names=set().union(*(required_sources(bs[i]) for i in ids))|{'tools/coverage_claims.py'}
    sources={n:sha(ROOT/n) for n in sorted(names)}
    claims=[{'behavior':i,'contract_sha256':behavior_contract(bs[i],builds)} for i in ids]
    report={'schema':1,'success':False,'scope':bs[ids[0]]['scope'],'sources':sources,
            'claims':claims,'original_sha256':BUILD,'live_replacement':False,
            'physical_allocation_or_all_writer_ownership_proved':False,
            'original_pixels_used_as_native_inputs':False}
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,cmd,output=None):
        print(name,flush=True)
        with (out/(name+'.log')).open('x') as log:
            if output:
                with (out/output).open('xb') as target:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=target,stderr=log,check=True,timeout=240,env=environment)
            else:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=240,env=environment)
    before=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        if sha(a.executable)!=BUILD:raise ValueError('Require pinned unchanged executable')
        fixture='tests/canvas-release-reference.cpp' if a.suite=='release' else 'tests/world-line-reference.cpp'
        modes=['release'] if a.suite=='release' else ['trace','pixels','queue']
        for name,flags in [('native',[]),('original',['-m32','-DMNM_ORIGINAL_REFERENCE','-fno-pie','-no-pie'])]:
            extra=[] if a.suite=='release' else [ROOT/'reconstruction/rendering/palette_shading.cpp']
            run(name+'-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,ROOT/fixture,*extra,'-o',out/name])
            for mode in modes:
                args=[out/name]+([a.executable.resolve()] if name=='original' else [])+([] if a.suite=='release' else [mode])
                run(name+'-'+mode,args,name+'-'+mode+'.bin')
        results=[]
        for mode in modes:
            native=out/('native-'+mode+'.bin');original=out/('original-'+mode+'.bin')
            cells,events=validate(native,a.suite,mode)
            if validate(original,a.suite,mode)!=(cells,events):raise ValueError('Original denominator differs')
            with native.open('rb') as n,original.open('rb') as o:
                offset=0
                while True:
                    left=n.read(1024*1024);right=o.read(1024*1024)
                    if left!=right:
                        at=next((i for i,(x,y) in enumerate(zip(left,right)) if x!=y),min(len(left),len(right)))
                        raise ValueError(f'{mode} original/native byte mismatch at {offset+at}')
                    if not left:break
                    offset+=len(left)
            results.append({'mode':mode,'cases':cells,'ordered_events':events,
                            'native_sha256':sha(native),'original_sha256':sha(original),'bytes_compared':native.stat().st_size})
        if sha(a.executable)!=BUILD:raise ValueError('Original input changed')
        report.update(success=True,results=results,
            selected_original_bodies_unchanged=True,
            scripted_com_results=a.suite=='release',
            actual_raster_pixels_compared=a.suite=='line',
            binary_sha256={name:sha(out/name) for name in ('native','original')})
    except (OSError,ValueError,subprocess.SubprocessError) as e:report['error']=str(e)
    finally:
        if before:
            try:
                run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as e:report.update(success=False,preservation_error=str(e))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success']&=report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
