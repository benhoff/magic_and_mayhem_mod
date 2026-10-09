#!/usr/bin/env python3
"""Exercise the public native startup launcher through its automatic finite original Quick Battle route."""
import argparse
import hashlib
import json
import os
import signal
from pathlib import Path
import subprocess
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode',choices=['verify','normal'],default='verify')
    parser.add_argument('--broken-import',action='store_true',help='Fail and record any attempt to use the desktop screenshot utility')
    args=parser.parse_args()
    parent=ROOT/'working/tests/native-world-launcher';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    import sys
    sys.path.insert(0,str(ROOT/'tools'))
    from coverage_claims import behavior_contract,scenario_contract,required_sources
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='NR.world-startup-launcher')
    scenario=next(s for s in register['scenarios'] if s['id']=='world-startup-public-launcher-20261008')
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(required_sources(behavior))}
    claim=dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{b['id']:b for b in register['builds']}),scenarios={scenario['id']:scenario_contract(scenario)})
    (run/'prospective.json').write_text(json.dumps(dict(sources=sources,claims=[claim]),indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    if args.broken_import:
        binary=run/'bin';binary.mkdir()
        stub=binary/'import';stub.write_text('#!/bin/sh\n: > "'+str(run/'import-invoked')+'"\nexit 97\n');stub.chmod(0o755)
        env['PATH']=str(binary)+os.pathsep+env['PATH']
    with (run/'launcher.log').open('x') as log:
        process=subprocess.Popen([str(ROOT/'tools/run-native-world.py'),'--startup-history',*(['--verify'] if args.mode=='verify' else [])],env=env,stdout=log,stderr=subprocess.STDOUT)
        try:
            experiment=None
            deadline=time.monotonic()+360
            while process.poll() is None:
                text=(run/'launcher.log').read_text()
                rows=[line for line in text.splitlines() if line.startswith('Native World shadow session: ')]
                if rows:experiment=Path(rows[-1].split(': ',1)[1])
                if time.monotonic()>deadline:raise RuntimeError('Public startup launcher completion deadline')
                time.sleep(.2)
            if process.returncode:raise RuntimeError('Public startup launcher failed; inspect '+str(run/'launcher.log'))
            completed=json.loads((experiment/'native-world.json').read_text())
            assert completed['success'] and completed['queues']==16
            assert len(completed['checkpoints'])>16
            assert completed['capture_policy']==dict(producer_oracle_mib=3072,window_screenshot_requested=False)
            if args.broken_import:assert not (run/'import-invoked').exists(),'Desktop screenshot utility invoked'
            assert all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items()),'Sources changed during execution'
            report=dict(success=True,mode=args.mode,experiment=str(experiment),sources=sources,claims=[claim],sources_stable=True,
                        broken_import=args.broken_import,screenshot_utility_invoked=(run/'import-invoked').exists(),
                        original_manifest_verified_before_after=True,completed=completed,
                        source_executable_sha256=json.loads((experiment/'manifest.json').read_text())['source_sha256'])
            (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
        finally:
            if process.poll() is None:
                process.send_signal(signal.SIGINT)
                try:process.wait(timeout=15)
                except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
if __name__=='__main__':main()
