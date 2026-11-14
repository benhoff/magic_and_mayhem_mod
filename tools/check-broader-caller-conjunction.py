#!/usr/bin/env python3
"""Conjoin only the four completed current caller cases; other requests may be active."""
import argparse,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_claims_matrix import specs,current,P
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh caller conjunction required')
 with sqlite3.connect('file:'+str(ROOT/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
  db.row_factory=sqlite3.Row;rows={v['case_id']:dict(v)for v in db.execute('select *from requests')}
 with Reader(ROOT)as r:
  selected=[s for s in specs(r)if s['family']=='caller'];require(len(selected)==4,'Caller denominator changed');items=[];sources={};worlds=cases=pixels=0
  for s in selected:
   cid=s['case']['id'];v=current(ROOT,r,s,rows[cid]);c=r.json(v['value']['comparison']['record']);require(c['success']and c['caller_state']['verified']and c['caller_state']['normal_retirement']and c['comparison']['mismatching_pixels']==0,'Incomplete caller comparison');binding=c['analysis_schema_binding'];require(binding['phase']==v['acquired']['prospective_plan']and not binding['hashes_or_snapshots_waived']and not binding['old_payload_changed'],'Caller phase schema drift');worlds+=c['original_reference']['worlds'];cases+=c['caller_state']['cases'];pixels+=c['comparison']['pixels_compared'] if 'pixels_compared'in c['comparison']else c['comparison'].get('compared_pixels',0);sources.update(v['current_sources']);sources.update(v['coordinator_sources']);ram=P+cid+'-RAM-handoff-01.json';held=r.json(ram);require(held['success']and held['validation']==v['validation']and all(x['real_restore_verified']and x['replicas_verified']==2 and x['physical_source_held']for x in held['results']),'Caller RAM custody incomplete');sources.update(held['sources']);items.append(dict(case=cid,profile=s['case']['profile'],size=s['case']['size'],actual_drivers=v['drivers'],acquisition=v['acquisition'],validation=v['validation'],coordinator=v['coordinator'],RAM_handoff=dict(record=ram,sha256=r.digest(ram)),worlds=c['original_reference']['worlds'],state_cases=c['caller_state']['cases']))
  require({(x['profile'],tuple(x['size']))for x in items}=={(p,s)for p in('player-default','player-software')for s in((640,480),(800,600))},'Caller provider/extent cell lost');sources['tools/check-broader-caller-conjunction.py']=r.digest('tools/check-broader-caller-conjunction.py');require(all(r.digest(n)==h for n,h in sources.items()),'Caller conjunction source drift')
 q=dict(schema=1,kind='broader_current_four_caller_context_conjunction',success=True,original_game_runs=0,new_native_builds=0,new_pixel_comparisons=0,current_cases=4,actual_retained_original_sessions=4,independently_reconstructed_worlds=worlds,state_model_cases=cases,model_and_required_pixel_mismatches=0,cases=items,sources=sources,sources_stable=True,whole_player_accepted=False,original_recovery_complete=False,scope='Only four completed current caller contexts, exact Native/compiler/runtime/B05binary, actual NVIDIA/software profiles and both extents. Require complete original-history pixels, separately original-compared caller models, exact schema interpretation, normal retirement, admission and paired/real-restored raw+heldRAM custody. No old failure retirement,new execution,global request-gate pass or full-cohort acceptance; independent later domains may be active.')
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,worlds=worlds,state_cases=cases)),flush=True)
if __name__=='__main__':main()
