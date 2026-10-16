#!/usr/bin/env python3
"""Reproduce native campaign session checks under the main storage guard."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--arena',action='store_true')
    args=parser.parse_args()
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
    declaration=claims.declaration(ROOT,json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),behaviors,scenarios)
    (run/'claims.json').write_text(json.dumps(declaration,indent=2)+'\n')
    build=run/'build';source=ROOT/('apps/skirmish' if args.arena else 'game')
    commands=[
        ['cmake','-S',str(source),'-B',str(build),'-DCMAKE_BUILD_TYPE=Debug'],
        ['cmake','--build',str(build),'-j4'],
        ['ctest','--test-dir',str(build),'--output-on-failure'],
    ]
    result={'schema':1,'scope':'Native authored campaign session: transactional attempts and resource-bound checkpoint ownership; no original campaign/reward equivalence.',
        'claims':declaration['claims'],'sources':declaration['sources'],'commands':[],'arena':args.arena,'success':False}
    try:
        for index,command in enumerate(commands):
            log=run/f'command-{index}.log'
            with log.open('w') as output:
                completed=subprocess.run(command,cwd=ROOT,stdout=output,stderr=subprocess.STDOUT)
            result['commands'].append({'argv':command,'exit_code':completed.returncode,
                'log':str(log.relative_to(ROOT)),'sha256':digest(log)})
            if completed.returncode:raise RuntimeError('Validation failed: '+str(log))
        if any(digest(ROOT/p)!=h for p,h in declaration['sources'].items()):
            raise RuntimeError('Declared campaign sources changed during execution')
        result['success']=True
    except Exception as error:result['error']=str(error)
    (run/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'success':result['success'],'report':str((run/'report.json').relative_to(ROOT)),'error':result.get('error')},indent=2))
    return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
