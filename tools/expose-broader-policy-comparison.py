#!/usr/bin/env python3
"""Expose an exact policy result through the legacy comparison-name interface."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_policy_acceptance import assess
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('source','alias','report'):p.add_argument('--'+n,required=True)
 args=p.parse_args();require(not(ROOT/args.alias).exists()and not(ROOT/args.report).exists(),'Fresh compatibility reference required')
 with Reader(ROOT)as r:
  c=r.json(args.source);require(c['success']and c['sources_stable']and all(r.digest(n)==h for n,h in c['sources'].items()),'Current complete policy comparison required')
  require(assess(r,c)['verified'],'Policy certificates refused')
  a=c['acquisition']['record'];require(args.alias==a.removesuffix('-acquisition.json')+'-comparison-policy-01.json','Only exact legacy comparison lookup name allowed')
  body=r.read_bytes(args.source);digest=r.digest(args.source);require(write_report(ROOT,args.alias,body)==digest,'Compatibility reference changed payload bytes')
  q=dict(schema=1,kind='broader_policy_legacy_comparison_binding',success=True,original_game_runs=0,new_comparisons=0,source=dict(record=args.source,sha256=digest),compatibility_reference=dict(record=args.alias,sha256=digest),exact_payload_shared=True,raw_original_negative_changed=False,consumer=dict(path='tools/run-rendering-recovery-closeout.py',sha256=r.digest('tools/run-rendering-recovery-closeout.py')),reason='The unchanged collector discovers prior comparisons by acquisition-stem plus -comparison*.json. This small SQLite compatibility reference exposes the exact existing policy payload, including its raw-original negative, through that required interface. No report payload is duplicated or edited and no execution is rerun.')
  h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,payload_sha256=digest)),flush=True)
if __name__=='__main__':main()
