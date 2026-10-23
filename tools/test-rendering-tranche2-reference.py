#!/usr/bin/env python3
"""Bounded unchanged-original startup branch comparisons, with owned adapters."""
from coverage_evidence import read_json, evidence_digest, write_report
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--executable',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--case',choices=['create','bind','clear','wave','palette','uniform','caller'],action='append',required=True)
    p.add_argument('--claims',type=Path)
    a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    paths=['tools/test-rendering-tranche2-reference.py','tests/rendering-tranche2-reference.cpp',
           'tests/sprite-binary-reference.cpp','reconstruction/rendering/startup_canvas.hpp',
           'reconstruction/rendering/palette_shading.cpp','reconstruction/rendering/palette_shading.hpp']
    paths.append('reconstruction/rendering/generic_producer_caller.h')
    paths += ['assets/sprite_run_clipping.hpp','assets/sprite_loader.hpp','tests/rendering-wave-fixture.hpp']
    def sha(p):return evidence_digest(p)
    claims=json.loads(a.claims.read_text()) if a.claims else None
    sources={n:sha(ROOT/n) for n in paths}
    if claims:
        for n,h in claims['sources'].items():
            if sha(ROOT/n)!=h:raise ValueError('Declared source changed: '+n)
            sources[n]=h
    report=dict(schema=1,success=False,sources=sources,required_cases=a.case,results=[],
                scope='Selected original startup branch fixtures with scripted external boundaries; no real driver allocation/lifetime or live replacement.')
    if claims:report['claims']=claims['claims']
    def run(label,argv):
        result=subprocess.run([str(v) for v in argv],cwd=ROOT,capture_output=True,text=True,timeout=120)
        (out/(label+'.log')).write_text(result.stdout+result.stderr)
        result.check_returncode();return result.stdout
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        if sha(a.executable)!=BUILD:raise ValueError('Original executable hash mismatch')
        binary=out/'reference'
        run('build',['g++','-m32','-std=c++17','-O2','-mno-sse','-mno-mmx','-mstackrealign','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',
                     ROOT/'tests/rendering-tranche2-reference.cpp',ROOT/'reconstruction/rendering/palette_shading.cpp','-o',binary])
        outcomes=[]
        for case in a.case:
            try:
                rows=[json.loads(line) for line in run(case,[binary,a.executable.resolve(),case,*([out/'wave-pixels.565'] if case=='wave' else [])]).splitlines()]
                if not rows:raise ValueError('Empty original fixture result')
                if case=='wave':report['wave_pixels']={'path':str(out/'wave-pixels.565'),'sha256':sha(out/'wave-pixels.565'),'bytes':(out/'wave-pixels.565').stat().st_size}
                report['results'].extend(rows);outcomes.append(dict(case=case,success=True))
            except Exception as error:outcomes.append(dict(case=case,success=False,error=str(error)))
        report.update(success=all(c['success'] for c in outcomes),case_outcomes=outcomes,binary_sha256=sha(binary),original_sha256=BUILD,input_unchanged=sha(a.executable)==BUILD)
    except Exception as error:report['error']=str(error)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        write_report(ROOT,out/'report.json',(json.dumps(report,indent=2)+'\n').encode())
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
