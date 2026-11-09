#!/usr/bin/env python3
"""Retain production font hook ABI/reader controls and exact tested binaries."""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess
import tempfile
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
from font_lifetime import decode

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/font-lifetime-results-20261108/'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    if not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('Fresh owned report required')
    def put(suffix,data):
        name=a.report.removesuffix('.json')+'-'+suffix
        return dict(record=name,sha256=write_report(ROOT,name,data))
    result=dict(schema=1,kind='font_lifetime_production_controls',success=False,original_game_runs=0,
                installed_observation=False,newly_passed_requirements=0,commands=[])
    with Reader(ROOT) as reader:
        register=reader.json('research/runtime/coverage/register.json')
        behavior=next(b for b in register['behaviors'] if b['id']=='NP.font-lifecycle-observation')
        names=required_sources(behavior)|{'tools/check-font-lifetime-observer.py'}
        sources={n:reader.digest(n) for n in sorted(names)}
        result['sources']=sources
        result['claims']=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{b['id']:b for b in register['builds']}),scenarios={s['id']:scenario_contract(s) for s in register['scenarios'] if behavior['id'] in s['behaviors']})]
    result['base_commit']=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
    result['unique_sources']={n:put('source-'+hashlib.sha256(n.encode()).hexdigest()[:20]+'.bin',(ROOT/n).read_bytes()) for n in sorted(names)}
    with tempfile.TemporaryDirectory(prefix='mnm-font-controls-',dir='/tmp') as directory:
        scratch=Path(directory)
        def run(label,args):
            command=[str(x) for x in args]
            c=subprocess.run(command,cwd=ROOT,capture_output=True,timeout=120,env=dict(os.environ,TMPDIR=directory))
            result['commands'].append(dict(label=label,argv=command,returncode=c.returncode,log=put(label+'.log',c.stdout+c.stderr)))
            c.check_returncode();return c.stdout
        try:
            run('compiler',['g++','--version'])
            run('compile',['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror',
                           'tests/font-lifetime-observer.cpp','runtime/scene/font_lifetime.S','tests/render-contract-call.S','-o',scratch/'controls'])
            result['native']=json.loads(run('abi-controls',[scratch/'controls',scratch/'journal.bin']))
            result['binary']=put('controls.bin',(scratch/'controls').read_bytes())
            raw=(scratch/'journal.bin').read_bytes();result['transcript']=put('journal.bin',raw)
            observed=decode(raw);result['decoded']=dict(records=len(observed['rows']),loads=len(observed['loads']),uses=len(observed['uses']),retired=len(observed['retired']),live_at_detach=observed['live_at_detach'])
            run('reader-controls',['python3','-B','tests/test-font-lifetime.py'])
            with Reader(ROOT) as reader:
                log=reader.read_bytes(result['commands'][-1]['log']['record'])
            match=re.search(rb'Ran ([0-9]+) tests',log)
            if not match:raise ValueError('Missing executed reader-test denominator')
            result['reader_tests']=int(match[1])
            run('pe32-build',['python3','tools/build-scene-observer.py'])
            dll=ROOT/'working/build/scene-observer/MnmScene.dll';result['production_dll']=put('MnmScene.dll.bin',dll.read_bytes())
            result['production_manifest']=put('production-manifest.bin',(dll.parent/'manifest.json').read_bytes())
            result['success']=True
        except Exception as e:result['error']=str(e)
        with Reader(ROOT) as reader:result['sources_stable']=sources=={n:reader.digest(n) for n in names}
        result['success'] &= result['sources_stable']
    digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],record=a.report,sha256=digest,error=result.get('error'))),flush=True)
    return int(not result['success'])

if __name__=='__main__':raise SystemExit(main())
