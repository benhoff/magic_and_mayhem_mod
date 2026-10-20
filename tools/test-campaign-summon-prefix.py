#!/usr/bin/env python3
"""Replay accepted-summon attribution against retained observations, never a game."""
import argparse
import hashlib
import json
from pathlib import Path
from campaign_input_admission import resolve_additional_summons
from campaign_gameplay import read_rows, damage_rows
from campaign_spell import read_spell_rows

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--gameplay',type=Path,required=True)
    parser.add_argument('--spells',type=Path,required=True)
    parser.add_argument('--attempt-sequence',type=int,action='append',required=True)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();output=args.output.resolve()
    if output.exists() or not output.is_relative_to(ROOT/'working'):parser.error('Require a new owned output')
    inputs=[args.gameplay.resolve(),args.spells.resolve()]
    if any(not p.is_relative_to(ROOT) or p.relative_to(ROOT).parts[0] not in ('working','research') or not 16<=p.stat().st_size<=4194320 for p in inputs):parser.error('Require bounded retained observations')
    declaration=json.loads(args.claims.read_text())
    if not all(sha(ROOT/n)==h for n,h in declaration['sources'].items()):raise ValueError('Prospective source changed')
    fingerprints={str(p.relative_to(ROOT)):sha(p) for p in inputs}
    gp=read_rows(inputs[0],complete=True);spells=read_spell_rows(inputs[1],complete=True)
    confirmed,pending=resolve_additional_summons(gp,spells,[dict(before_sequence=n) for n in args.attempt_sequence],0,0)
    hits=damage_rows(gp,0,[c['slot'] for c in confirmed],min(args.attempt_sequence))
    result=dict(schema=1,success=bool(confirmed) and not pending,scope=__doc__,claims=declaration['claims'],sources={**declaration['sources'],**fingerprints},recorded_inputs=list(fingerprints),confirmed=confirmed,pending=pending,qualifying_melee_events=len(hits),latencies_ms=[(gp[c['after_sequence']-1][3]-gp[c['before_sequence']-1][3])&0xffffffff for c in confirmed],original_execution_renewed=False)
    result['inputs_stable']=all(sha(ROOT/n)==h for n,h in fingerprints.items())
    result['sources_stable']=all(sha(ROOT/n)==h for n,h in declaration['sources'].items())
    result['success'] &= result['inputs_stable'] and result['sources_stable']
    output.parent.mkdir(parents=True,exist_ok=True)
    with output.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
    print(json.dumps({k:result[k] for k in ('success','confirmed','latencies_ms','qualifying_melee_events')}))
    return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
