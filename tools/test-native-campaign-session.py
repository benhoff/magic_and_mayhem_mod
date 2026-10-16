#!/usr/bin/env python3
"""Reproduce native campaign session checks under the main storage guard."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--arena',action='store_true')
    parser.add_argument('--window',action='store_true')
    args=parser.parse_args()
    if args.window:args.arena=True
    parent=ROOT/'working/tests/campaign-session'
    parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    (run/'.retain').write_text('Native campaign session execution evidence.\n')
    spec=importlib.util.spec_from_file_location('campaign_claims',ROOT/'tools/draft-coverage-claims.py')
    claims=importlib.util.module_from_spec(spec);spec.loader.exec_module(claims)
    behaviors=['NP.campaign-persistence','NP.campaign-session']
    scenarios=['NP.campaign-persistence.restart','NP.campaign-session.transaction']
    if args.arena:
        behaviors.append('NP.campaign-arena');scenarios.append('NP.campaign-arena.journey')
    if args.window:
        behaviors.extend(['NP.campaign-window','NP.skirmish-window'])
        scenarios.extend(['NP.campaign-window.journey','window.native-skirmish'])
    declaration=claims.declaration(ROOT,json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),behaviors,scenarios)
    (run/'claims.json').write_text(json.dumps(declaration,indent=2)+'\n')
    build=run/'build';source=ROOT/('apps/skirmish' if args.arena else 'game')
    commands=[
        ['cmake','-S',str(source),'-B',str(build),'-DCMAKE_BUILD_TYPE=Debug'],
        ['cmake','--build',str(build),'-j4'],
        ['ctest','--test-dir',str(build),'--output-on-failure'],
    ]
    result={'schema':1,'scope':'Native authored campaign session: transactional attempts and resource-bound checkpoint ownership; no original campaign/reward equivalence.',
        'claims':declaration['claims'],'sources':declaration['sources'],'commands':[],'arena':args.arena,'window':args.window,'success':False}
    environment=os.environ.copy()
    if args.arena:environment['MNM_CAMPAIGN_JOURNEY_OUTPUT']=str(run/'journey')
    if args.window:environment['MNM_CAMPAIGN_WINDOW_OUTPUT']=str(run/'window')
    try:
        for index,command in enumerate(commands):
            log=run/f'command-{index}.log'
            with log.open('w') as output:
                completed=subprocess.run(command,cwd=ROOT,env=environment,stdout=output,stderr=subprocess.STDOUT)
            result['commands'].append({'argv':command,'exit_code':completed.returncode,
                'log':str(log.relative_to(ROOT)),'sha256':digest(log)})
            if completed.returncode:raise RuntimeError('Validation failed: '+str(log))
        if any(digest(ROOT/p)!=h for p,h in declaration['sources'].items()):
            raise RuntimeError('Declared campaign sources changed during execution')
        if args.arena:
            journey=run/'journey/report.json';details=json.loads(journey.read_text())
            if not details['success']:raise RuntimeError('Native campaign journey did not pass')
            result['journey']={'report':str(journey.relative_to(ROOT)),'sha256':digest(journey),
                'victories':details['victories'],'defeats':details['defeats'],
                'retry_attempt':details['retry_attempt'],'final_transition':details['final_transition']}
        if args.window:
            journey=run/'window/report.json';details=json.loads(journey.read_text())
            if not details['success']:raise RuntimeError('Native campaign window journey did not pass')
            result['window_journey']={'report':str(journey.relative_to(ROOT)),'sha256':digest(journey),
                'final_transition':details['final_transition'],'retry_attempt':details['retry_attempt'],
                'refused_loads':details['refused_loads'],'timer_tick':details['timer_tick']}
        result['success']=True
    except Exception as error:result['error']=str(error)
    (run/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'success':result['success'],'report':str((run/'report.json').relative_to(ROOT)),'error':result.get('error')},indent=2))
    return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
