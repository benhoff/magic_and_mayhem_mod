#!/usr/bin/env python3
"""Zero-game planning check: every historical primary has a declared future context."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_acceptance import parents
from broader_player_continued_matrix import specs,replacements,P
from broader_player_join import join_rows,expected_cells
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh planning binding check required')
 with Reader(ROOT)as r:
  declared=specs(r);hypothetical=[]
  for s in declared:
   c=s['case'];hypothetical.append(dict(spec=s,value=dict(profile=c['profile'],success=True,sources_stable=True),phase=dict(current_native_source_candidate=True),validation=dict(record=P+c['id']+'-validation-UNEXECUTED.json',sha256='0'*64),acquisition=dict(record=c['report'],sha256='0'*64)))
  inventory=r.json(P+'inventory-04.json');mapping=replacements(r,inventory,hypothetical);accepted=parents(r,r.json('research/runtime/coverage/register.json'));j,o=expected_cells(r.json('docs/native-rendering-closure.json'));counts={}
  for eid,expected,journey in[('JG10.journeys02',j,True),('OG09.merged-operations01',o,False)]:
   parent=accepted[eid];rows=join_rows(parent['proof']['linked_requirements'],expected,mapping,parent['reference'],journey=journey,controlled_overlap=dict(verified=True,natural_player_call_proved=False,approved_amendment='ALIAS09.controlled-opaque-overlap-v1'));counts[eid]=dict(rows=len(rows),primary_attachments=sum(len(v['attachments'])for v in rows))
 q=dict(schema=1,kind='broader_player_prospective_matrix_binding_controls',success=True,planning_only=True,original_game_runs=0,actual_current_cases_proved=0,future_case_count=len(declared),historical_inventory_count=len(mapping),joined_requirements=counts,hypothetical_successes_are_only_test_inputs=True,current_whole_player_proved=False,supported_player_accepted=False,scope='Resolve all historical primary identities and every parent-row attachment against declared prospective contexts. Hypothetical inputs test only routing completeness; no actual acquisition, source freshness, output, lifecycle, admission or acceptance is asserted.')
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,counts=counts,original_game_runs=0)))
if __name__=='__main__':main()
