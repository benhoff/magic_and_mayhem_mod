#!/usr/bin/env python3
"""Exercise owned effect producer decode and GPU pixels; live hook admission remains separate."""
from coverage_evidence import read_json as evidence_json
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BEHAVIOR='NP.world-effect-producer-admission'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def declaration():
    r=evidence_json(ROOT/'research/runtime/coverage/register.json');b=next(b for b in r['behaviors'] if b['id']==BEHAVIOR)
    scenario=next(s for s in r['scenarios'] if s['id']=='native-world-effect-producers')
    paths=required_sources(b)|{'tests/resource-fixtures.hpp','compat/legacy/canvas-producers/CMakeLists.txt'}
    for directory in ['renderer','assets','compat/legacy','protocols']:
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ['.cpp','.hpp','.h','.c','.S'] or p.name=='CMakeLists.txt'))
    return [dict(behavior=BEHAVIOR,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={scenario['id']:scenario_contract(scenario)})],{p:sha(ROOT/p) for p in sorted(paths)}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--return-report',type=Path,required=True);p.add_argument('--build-dir',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh output below working required')
    out.mkdir(parents=True);prior=evidence_json(a.return_report)
    if not prior.get('success') or not prior.get('low16_returns_match'):raise ValueError('Require successful original effect result')
    manifest=[];inputs={str(a.return_report.resolve()):sha(a.return_report)}
    for i,f in enumerate(prior['fixtures']):
        frame=Path(f['input']);before=Path(str(frame.with_suffix(''))+'.before.565');expected=a.return_report.resolve().parent/(str(i)+'.original.565')
        if sha(frame)!=prior['inputs'][str(frame)] or sha(before)!=prior['inputs'][str(before)] or sha(expected)!=f['original_sha256']:raise ValueError('Original fixture changed')
        manifest.append(dict(input=str(frame),before=str(before),expected=str(expected)))
        for path in [frame,before,expected]:inputs[str(path)]=sha(path)
    (out/'fixtures.json').write_text(json.dumps(manifest,indent=2)+'\n');claims,sources=declaration()
    report=dict(schema=1,success=False,claims=claims,sources=sources,inputs=inputs,scope=__doc__,live_replacement=False,original_pixels_used_as_native_inputs=False,live_hook_executed=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1','LP_NUM_THREADS':'4'}
    def run(name,cmd):
        with (out/(name+'.log')).open('x') as f:subprocess.run([str(x) for x in cmd],cwd=ROOT,env=env,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=900)
    try:
        run('configure',['cmake','-S',ROOT/'compat/legacy/canvas-producers','-B',a.build_dir,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',a.build_dir,'--target','world-effect-producer-test','world-raster-batch-test','canvas-world-test','-j4'])
        run('native',['xvfb-run','-a',a.build_dir/'world-effect-producer-test',out/'fixtures.json'])
        result=json.loads((out/'native.log').read_text().splitlines()[-1])
        # Run the entry/ABI, signature and non-drawing preparation proofs with
        # the same frozen parent contract/source declaration as the GPU run.
        (out/'hook-claims.json').write_text(json.dumps(dict(claims=claims,sources=sources),indent=2)+'\n')
        run('hooks',['python3',ROOT/'tools/test-canvas-producers.py','--claims',out/'hook-claims.json','--report',out/'hook-report.json'])
        hook=evidence_json(out/'hook-report.json')
        if not hook.get('effect_wire_normalized') or hook.get('effect_entries_native_bypassed')!=5:raise ValueError('Incomplete effect entry proof')
        run('callers',['python3',ROOT/'tools/test-world-raster-callers.py','--output',out/'callers'])
        caller=evidence_json(out/'callers/report.json')
        run('tables',['python3',ROOT/'tools/test-world-effect-tables.py','--output',out/'tables'])
        table=evidence_json(out/'tables/report.json')
        if not caller.get('success') or not table.get('empty_frame_non_drawing'):raise ValueError('Incomplete caller/preparation proof')
        run('batch-regression',[a.build_dir/'world-raster-batch-test'])
        run('world-regression',['xvfb-run','-a',a.build_dir/'canvas-world-test'])
        if declaration()!=(claims,sources) or any(sha(Path(x))!=h for x,h in inputs.items()):raise ValueError('Source/input changed')
        report.update(success=True,sources_stable=True,inputs_stable=True,native_result=result,producer_gpu_pixels_match=True,regressions_passed=True,pe32_synthetic_entry_executed=True,hook_result_sha256=sha(out/'hook-report.json'),caller_result_sha256=sha(out/'callers/report.json'),table_result_sha256=sha(out/'tables/report.json'),original_manifest_verified_before_after=bool(caller.get('original_manifest_verified_before_after') and table.get('original_manifest_verified_before_after')))
    finally:
        report['artifacts']={x.name:sha(x) for x in out.iterdir() if x.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
