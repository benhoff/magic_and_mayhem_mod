#!/usr/bin/env python3
"""Source-bound synthetic atlas validation, with optional captured World replay."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET
from coverage_claims import behavior_contract, required_sources, scenario_contract
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def declaration():
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    b=next(b for b in r['behaviors'] if b['id']=='NR.sprite-atlas')
    s=next(s for s in r['scenarios'] if s['id']=='native-sprite-atlas')
    claims=[{'behavior':b['id'],'contract_sha256':behavior_contract(b,{b['id']:b for b in r['builds']}),'scenarios':{s['id']:scenario_contract(s)}}]
    paths=required_sources(b)|set(s['tests'])
    # Pin the native library/build closure, beyond the policy's direct paths.
    for directory in ('assets','renderer','compat/legacy','tests','protocols'):
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ('.cpp','.hpp','.h','.c','.S','.py','.sh') or p.name=='CMakeLists.txt'))
    return claims,{p:sha(ROOT/p) for p in sorted(paths)}
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--full',action='store_true',help='Build and execute the complete native CTest corpus')
    parser.add_argument('--capture',type=Path,help='Closed experiment with capture/world-*.bin and read-only game assets')
    args=parser.parse_args()
    parent=ROOT/'working/tests/sprite-atlas';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    build=ROOT/'working/build/world-frame';claims,sources=declaration()
    report={'success':False,'sources':sources,'claims':claims,'scope':'Synthetic bounded atlas policy and expanded-word pixel comparison; optional captured queues use identical native zero initialization for both paths. No new original draw-kind, initialization equivalence or live replacement claim.'}
    (out/'prospective.json').write_text(json.dumps({'claims':claims,'sources':sources},indent=2)+'\n')
    def run(name,command,timeout=300):
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=timeout,env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'})
    try:
        run('configure',['cmake','-S',ROOT/'compat/legacy','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,*([] if args.full else ['--target','sprite-atlas-test','resource-cache-test','scene-upload-test','sprite-render-test','scene-renderer-test','scene-history-test','world-live-session-test',*(['world-atlas-replay'] if args.capture else [])]),'-j4'],timeout=600)
        run('ctest',['ctest','--test-dir',build,*([] if args.full else ['-R','^(opengl-sprite-atlas|native-resource-cache|opengl-scene-upload-budget|opengl-sprite-frames|native-shared-scene-renderer|native-scene-history|native-world-live-session)$']),'--output-on-failure','--output-junit',out/'ctest.xml'],timeout=600)
        cases=ET.parse(out/'ctest.xml').findall('.//testcase')
        if (len(cases)<70 if args.full else len(cases)!=7) or any(c.find('failure') is not None or c.find('skipped') is not None for c in cases):raise RuntimeError('Atlas regression corpus incomplete or failed')
        report.update(ctests_passed=len(cases),tests=[c.attrib['name'] for c in cases],outputs={c.attrib['name']:c.findtext('system-out','') for c in cases})
        if args.capture:
            run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
            try:
                capture=args.capture.resolve();snapshots=sorted((capture/'capture').glob('world-*.bin'))
                if not 1<=len(snapshots)<=16:raise ValueError('Require 1..16 closed successful World inputs')
                spec=importlib.util.spec_from_file_location('world_frames',ROOT/'tools/test-world-frames.py');world=importlib.util.module_from_spec(spec);spec.loader.exec_module(world)
                needed={world.identity(raw,h[9]) for p in snapshots for h,raw in world.records(p.read_bytes())}
                bindings=out/'bindings.json';bindings.write_text(json.dumps(world.pinned_bindings(capture/'game',needed),indent=2)+'\n')
                inputs={str(p.relative_to(ROOT)):sha(p) for p in snapshots}
                for b in json.loads(bindings.read_text()):inputs[str((capture/'game'/b['sprite']).relative_to(ROOT))]=b['sha256']
                inputs[str(bindings.relative_to(ROOT))]=sha(bindings)
                report['inputs']=inputs
                run('captured-replay',['xvfb-run','-a',build/'world-atlas-replay',capture/'game',bindings,out/'replay.json',*snapshots],600)
                replay=json.loads((out/'replay.json').read_text());report['replay']=replay
                if not replay['success']:raise RuntimeError('Captured atlas replay failed')
                if any(sha(ROOT/p)!=digest for p,digest in inputs.items()):raise RuntimeError('Captured inputs changed during replay')
            finally:run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        after_claims,after_sources=declaration()
        if claims!=after_claims or sources!=after_sources:raise RuntimeError('Atlas sources/contracts changed during execution')
        report.update(success=True,sources_stable=True)
    except Exception as error:
        report['error']=str(error)
        raise
    finally:
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
