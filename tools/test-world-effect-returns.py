#!/usr/bin/env python3
"""Compare low16 effect-raster returns using retained authored World inputs."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BEHAVIOR='RS.world-effect-raster-returns'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def declaration():
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    b=next(b for b in r['behaviors'] if b['id']==BEHAVIOR)
    return [dict(behavior=BEHAVIOR,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}))],{p:sha(ROOT/p) for p in sorted(required_sources(b))}
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--corpus',type=Path,action='append',required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh output below working required')
    out.mkdir(parents=True);claims,sources=declaration();inputs={};fixtures=[]
    for corpus in a.corpus:
        corpus=corpus.resolve();old=json.loads((corpus/'report.json').read_text())
        if not old.get('success') or not old.get('all_pixels_equal'):raise ValueError('Require successful retained authored corpus')
        for f in old['fixtures']:
            frame=corpus/f['input'];before=Path(str(frame.with_suffix(''))+'.before.565');expected=Path(str(frame.with_suffix(''))+'.native.565')
            paths=[frame,before,expected];sidecar=Path(str(frame)+'.rows')
            if sidecar.exists():paths.append(sidecar)
            for path in paths:inputs[str(path)]=sha(path)
            if sha(expected)!=f['native_sha256']:raise ValueError('Retained expected canvas changed')
            fixtures.append((frame,before,expected))
        inputs[str(corpus/'report.json')]=sha(corpus/'report.json')
    report=dict(schema=1,success=False,scope=__doc__,claims=claims,sources=sources,inputs=inputs,original_sha256=BUILD,live_replacement=False,original_raster_instructions_unchanged=True)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,args):
        with (out/(name+'.log')).open('x') as f:subprocess.run([str(x) for x in args],cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=300)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        if sha(a.executable)!=BUILD:raise ValueError('Pinned original required')
        common=['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror']
        run('model-build',common+[ROOT/'tests/effect-raster-return-test.cpp','-o',out/'model'])
        run('model',[out/'model'])
        flags=['-m32','-fno-pie','-no-pie']
        run('reference-build',common+flags+[ROOT/'tests/world-effect-return-reference.cpp','-o',out/'reference'])
        run('default-reference-build',common+flags+[ROOT/'tests/world-frame-reference.cpp','-o',out/'default-reference'])
        totals=Counter();results=[]
        for i,(frame,before,expected) in enumerate(fixtures):
            target=out/(str(i)+'.original.565')
            run(str(i),[out/'reference',a.executable,frame,target,before])
            result=json.loads((out/(str(i)+'.log')).read_text().splitlines()[-1])
            totals.update({int(k):v for k,v in result['backend_calls'].items()})
            if target.read_bytes()!=expected.read_bytes():raise ValueError('Retained pixel regression differs: '+str(frame))
            results.append(dict(input=str(frame),backend_calls=result['backend_calls'],original_sha256=sha(target),pixels_equal=True))
        if not all(totals[b] for b in [7,13,14,15,16]):raise ValueError('Missing effect return domain')
        # Default reference must retain its ordinary entry/output without the probe.
        for i,fixture in enumerate(fixtures):
            if i not in [0,len(fixtures)-1]:continue
            frame,before,expected=fixture;target=out/(str(i)+'.default.565')
            run(str(i)+'-default',[out/'default-reference',a.executable,frame,target,before])
            if target.read_bytes()!=expected.read_bytes():raise ValueError('Default reference changed pixels')
        if declaration()!=(claims,sources) or any(sha(Path(x))!=h for x,h in inputs.items()):raise ValueError('Source/input changed')
        report.update(success=True,sources_stable=True,inputs_stable=True,low16_returns_match=True,model_and_refusal_controls_passed=True,default_reference_controls_passed=True,backend_calls=dict(sorted(totals.items())),fixtures=results,fixtures_count=len(results))
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
        report['artifacts']={x.name:sha(x) for x in out.iterdir() if x.is_file()}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
