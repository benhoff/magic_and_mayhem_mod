#!/usr/bin/env python3
"""Execute the caller-serialized diagnostic mutex lifetime state fixture."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from coverage_claims import behavior_contract, required_sources
ROOT=Path(__file__).resolve().parents[1]
def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    if args.report.exists():parser.error('Refusing report overwrite')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='NR.drawing-cursor-mutex-lifetime')
    declaration=json.loads(args.claims.read_text())
    contract=behavior_contract(behavior,{b['id']:b for b in register['builds']})
    if declaration['claims']!=[dict(behavior=behavior['id'],contract_sha256=contract,scenarios={})] or set(declaration['sources'])!=set(required_sources(behavior)):
        raise ValueError('Prospective contract/sources mismatch')
    def check_sources():
        if any(digest(ROOT/p)!=h for p,h in declaration['sources'].items()):raise ValueError('Source drift')
    check_sources()
    parent=ROOT/'working/tests/drawing-cursor-mutex-lifetime';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));binary=run/'fixture'
    with (run/'build.log').open('x') as log:
        subprocess.run(['c++','-std=c++17','-O2','-pthread','-Wall','-Wextra','-Werror',str(ROOT/'tests/test-drawing-cursor-mutex-lifetime.cpp'),'-o',str(binary)],stdout=log,stderr=subprocess.STDOUT,check=True)
    with (run/'execution.log').open('x') as log:
        result=subprocess.run([str(binary)],stdout=log,stderr=subprocess.STDOUT,timeout=10)
    check_sources()
    current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    current_behavior=next(b for b in current['behaviors'] if b['id']==behavior['id'])
    if behavior_contract(current_behavior,{b['id']:b for b in current['builds']})!=contract:raise ValueError('Contract drift')
    report=dict(schema=1,success=result.returncode==0,return_code=result.returncode,sources=declaration['sources'],claims=declaration['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=digest(binary),retirement_lease_counts=list(range(33)),host_threads=8,attempts_per_thread=1000,caller_serialized=True,original_game_executed=False,os_mutex_handle_calls=False,live_capture_integrated=False,scope='Owned serialized state transitions only:0..32leases, retired reservation refusal, stale identity, deferred close, one close claim, failed close retained, reused identity and overflow. Host-thread stress uses an explicit caller mutex; no game handles/callbacks or source/native parity.')
    with args.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
    if result.returncode:raise ValueError('Lifetime fixture failed')
    print(json.dumps(dict(success=True,retirement_lease_counts=33)))
if __name__=='__main__':main()
