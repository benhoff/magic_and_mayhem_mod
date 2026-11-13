#!/usr/bin/env python3
"""Retain exact stopped strict raw observations and source preimages in both stores."""
import argparse,hashlib,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from evidence_workspace import seal,scan,safe,idle,MARKER
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--validation',required=True);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh strict handoff required')
 with Reader(ROOT)as r:
  v=r.json(args.validation);require(v['success']and v['sources_stable']and all(r.digest(n)==h for n,h in v['sources'].items()),'Current strict proof required');ar=v['acquisition'];require(r.digest(ar['record'])==ar['sha256'],'Strict acquisition changed');a=r.json(ar['record']);require(a['success']and a['original_process']['cleanup_after_original_exit']and a['shell_exit']==0,'Strict processes did not retire')
  paths=[ROOT/a['artifacts'],ROOT/a['session']];require(len(set(paths))==2,'Unexpected strict root overlap');rows=[]
  for path in paths:
   safe(ROOT,path);idle(path);require(not(path/MARKER).exists(),'Strict evidence already handed off');members,identities,allocated=scan(path);rows.append(dict(path=str(path),members=members,allocated_bytes=allocated))
  source_bytes={n:(h,r.read_bytes(n))for n,h in v['sources'].items()};require(all(hashlib.sha256(raw).hexdigest()==h for h,raw in source_bytes.values()),'Strict source preimage changed');vr=dict(record=args.validation,sha256=r.digest(args.validation))
 plan=dict(schema=1,kind='admitted_broader_strict_handoff_plan',success=True,original_game_runs=0,validation=vr,paths=rows,scope='Exact owned stopped execution trees, pins and unique observations. Shared immutable source game, prefix seed, build and source checkout remain held.')
 pn=args.report.removesuffix('.json')+'-plan.json';ph=write_report(ROOT,pn,(json.dumps(plan,indent=2)+'\n').encode())
 with configured(ROOT)as pair:
  with pair.batch():snapshot={n:dict(sha256=pair.bytes(raw),size=len(raw))for n,(h,raw)in source_bytes.items()}
  sources=pair.root('broader_strict_analysis_sources',Path(ar['record']).stem,dict(schema=1,sources=snapshot));results=[seal(ROOT,path,pair,retire=True,restore_budget=16*1024**3)for path in paths]
 q=dict(schema=1,kind='admitted_broader_strict_handoff',success=True,original_game_runs=0,validation=vr,plan=dict(record=pn,sha256=ph),analysis_sources=sources,results=results,acceptance_status_changed=False);h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,trees=len(results))))
if __name__=='__main__':main()
