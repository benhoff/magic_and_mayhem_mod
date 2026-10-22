#!/usr/bin/env python3
"""Validate bounded 640/800 World extent admission in host batches and real PE32 entries."""
from coverage_evidence import read_json as evidence_json
import argparse,hashlib,json,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1];BEHAVIOR='NP.world-raster-surface-extents'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def declaration():
    r=evidence_json(ROOT/'research/runtime/coverage/register.json');b=next(b for b in r['behaviors'] if b['id']==BEHAVIOR);s=next(s for s in r['scenarios'] if s['id']=='native-world-surface-extents')
    paths=required_sources(b)|set(s['tests'])
    for directory in ['runtime/scene','runtime/shadow','runtime/render','protocols','compat/legacy','renderer','assets']:
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ['.h','.hpp','.c','.cpp','.S','.py','.json'] or p.name=='CMakeLists.txt'))
    return [dict(behavior=BEHAVIOR,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={s['id']:scenario_contract(s)})],{p:sha(ROOT/p) for p in sorted(paths)}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--build-dir',type=Path,required=True);a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh output below working required')
    out.mkdir(parents=True);claims,sources=declaration();report=dict(schema=1,success=False,claims=claims,sources=sources,scope='Native admission for tight640x480/800x600 only; existing host batch identity/source/AX/reply controls at both extents, synthetic PE32 protected640canvas and actual entry bypass, public owned-Lock bindings at both extents, malformed and midqueue extent controls. No original pixels, natural campaign or complete replacement claim.',live_replacement=False,original_renderer_executed=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,cmd):
        with (out/(name+'.log')).open('x') as f:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=900)
    try:
        run('configure',['cmake','-S',ROOT/'compat/legacy/canvas-producers','-B',a.build_dir,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',a.build_dir,'--target','world-raster-batch-test','render-resident-world-test','-j4']);run('batch',[a.build_dir/'world-raster-batch-test'])
        run('gpu',['xvfb-run','-a',a.build_dir/'renderer/render-resident-world-test'])
        gpu=[json.loads(line) for line in (out/'gpu.log').read_text().splitlines() if line.startswith('{\"width\":')]
        if [(row['width'],row['height']) for row in gpu]!=[(640,480),(800,600)] or any(row['world_readbacks'] or row['hud_and_copy_checks']!=2 for row in gpu):raise ValueError('Incomplete GPU extent composition')
        (out/'hook-claims.json').write_text(json.dumps(dict(claims=claims,sources=sources),indent=2)+'\n')
        run('hooks',['python3',ROOT/'tools/test-canvas-producers.py','--claims',out/'hook-claims.json','--report',out/'hook-report.json']);hook=evidence_json(out/'hook-report.json')
        if not hook['surface_extent640_native_bypassed'] or hook['surface_extent640_invalid_controls']!=4 or hook['surface_extent640_midqueue_identity_controls']!=5:raise ValueError('Incomplete extent controls')
        binds=[]
        for width in [640,800]:
            label='bind-'+str(width)
            run(label,['xvfb-run','-a','python3',ROOT/'tools/test-resident-bind-admission.py','--claims',out/'hook-claims.json','--width',width])
            logs=(out/(label+'.log')).read_text().splitlines()
            paths=[Path(line.strip()) for line in logs if line.strip().startswith(str(ROOT/'working/tests/resident-bind-admission/run-')) and not line.strip().endswith('report.json')]
            if not paths:raise ValueError('Missing bind fixture output')
            result=paths[0]/'report.json';bind=evidence_json(result)
            if not bind.get('success') or not bind.get('bind_succeeded') or bind.get('width')!=width:raise ValueError('Incomplete public bind proof')
            binds.append(dict(width=width,record=str(result),sha256=sha(result)))
        if declaration()!=(claims,sources):raise ValueError('Source/contract changed')
        report.update(success=True,sources_stable=True,host_batches_both_extents=True,gpu_composition_both_extents=True,gpu=gpu,public_binds_both_extents=True,binds=binds,pe32_protected640_entry=True,malformed_surface_controls=4,midqueue_identity_controls=5,legacy_continuous_pixel640_refused=hook['surface_extent640_pixel_mode_refused'],hook_report_sha256=sha(out/'hook-report.json'),hook_result=hook)
    finally:
        report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
