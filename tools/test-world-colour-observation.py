#!/usr/bin/env python3
"""Exercise a finite read-only World colour diagnostic under private Wine/Xvfb."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def declaration():
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors=[b for b in r['behaviors'] if b['id'] in ('NP.world-colour-observation','NP.world-word-entry-observation')]
    paths=set().union(*(required_sources(b) for b in behaviors))|{str(p.relative_to(ROOT)) for p in (ROOT/'runtime/scene').glob('*.[chS]')}
    for directory in ('protocols','runtime/menu','reconstruction/rendering','renderer','assets','compat/legacy'):
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in ('.h','.hpp','.cpp','.c','.S') or p.name=='CMakeLists.txt'))
    return [dict(behavior=b['id'],contract_sha256=behavior_contract(b,{b['id']:b for b in r['builds']})) for b in behaviors],{p:sha(ROOT/p) for p in sorted(paths)}

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--source-game',type=Path,required=True);parser.add_argument('--prefix-template',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):parser.error('Require fresh output under working/')
    out.mkdir(parents=True);claims,sources=declaration()
    report=dict(schema=1,success=False,claims=claims,sources=sources,scope='Finite original-active map1/item15 read-only selected WORD0x45ff counts, position hashes and bounds before hooked raster entries and at consumer completion. Synthetic padding/hash/read-only controls and strict stream refusal. No writer identity, class causality, original pixel forwarding, physical ownership or replacement claim.',original_sha256=BUILD,live_replacement=False,writer_identified=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label,command,timeout=600):
        with (out/(label+'.log')).open('x') as f:subprocess.run([str(c) for c in command],cwd=ROOT,env=os.environ,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=timeout)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        if sha(args.source_game/'Chaos.exe')!=BUILD:raise ValueError('Unsupported source game')
        run('native-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',ROOT/'tests/world-colour-test.cpp','-o',out/'scan-test'])
        run('native-scan',[out/'scan-test']);run('reader-tests',['python3','-B',ROOT/'tests/test-world-colour.py'])
        run('abi-and-kind8', ['python3',ROOT/'tools/test-kind8-raster.py','--build-dir','/tmp/mnm-tint-outline-build-20261020'])
        nested_paths=[Path(l) for l in (out/'abi-and-kind8.log').read_text().splitlines() if l.startswith(str(ROOT/'working/tests/kind8/raster-'))]
        if not nested_paths:raise ValueError('Missing active ABI report')
        nested=json.loads((nested_paths[0]/'report.json').read_text())
        if not nested.get('success') or not nested.get('active_hook_abi_and_class_gates_passed'):raise ValueError('Hook ABI/capture/class controls failed')
        report.update(active_hook_abi_and_class_gates_passed=True,hook_artifacts=str(nested_paths[0]),hook_report_sha256=sha(nested_paths[0]/'report.json'))
        run('input-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',ROOT/'tests/world-colour-input.cpp','-lX11','-lXtst','-o',out/'input-helper'])
        # One bounded drag in this private display; no semantic outcome is assumed.
        run('capture',['xvfb-run','-a','-s','-screen 0 1280x1024x24','python3',ROOT/'tools/world-colour-input-runner.py','--input-executable',out/'input-helper','--journal',out/'input-journal.json','--source-game',args.source_game,'--prefix-template',args.prefix_template,'--admitted-prefix-template','--world-frames','--world-colour-word',str(0x45ff),'--kind8-objects','--sample-kind8','--startup-queues','160','--samples','1','--map','1','--magic-items','15','--skip-window-screenshot'],timeout=600)
        lines=(out/'capture.log').read_text().splitlines();runs=[Path(l) for l in lines if l.startswith(str(ROOT/'working/experiments/scene-observer/run-'))]
        if not runs:raise ValueError('Capture did not retain its output location')
        capture=runs[0];result=json.loads((capture/'report.json').read_text())
        if not result.get('success') or len(result.get('world_colour_checkpoints',[]))!=1:raise ValueError('Incomplete finite colour observation')
        if declaration()!=(claims,sources) or sha(args.source_game/'Chaos.exe')!=BUILD:raise ValueError('Source or declared contract changed')
        report.update(success=True,sources_stable=True,read_only_controls_passed=True,finite_original_observation_passed=True,capture=str(capture),capture_report_sha256=sha(capture/'report.json'),checkpoint_streams=result['world_colour_checkpoints'],kind8_observations=result.get('kind8_objects'))
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
        report['artifacts']={f.name:sha(f) for f in out.iterdir() if f.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
