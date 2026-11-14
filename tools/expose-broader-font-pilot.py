#!/usr/bin/env python3
"""Expose an exact qualified font comparison to the unchanged expansion guard."""
import argparse,hashlib,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--validation',required=True);p.add_argument('--alias',required=True);p.add_argument('--report',required=True);a=p.parse_args();require(a.alias.startswith('research/runtime/font-dc-lifetime-results-20261108/bp16/world-font/')and a.report.startswith(P)and not(ROOT/a.alias).exists()and not(ROOT/a.report).exists(),'Fresh exact font pilot alias required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));v=r.json(a.validation);require(v['success']and v['sources_stable']and v['family']=='font'and v['current_case_proved']and v['current_native_phase']['current_native_source_candidate']and all(r.digest(n)==h for n,h in v['sources'].items()),'Current qualified font pilot required');c=v['comparison'];require(r.digest(c['record'])==c['sha256'],'Pilot comparison changed');q=r.json(c['record']);require(q['success']and q['sources_stable']and q['comparison_processes_stopped']and q['font_lifetimes_verified']and q['independent_output_verified']and q['native_menu_fonts']['verified'],'Exact unchanged font expansion assertions missing');raw=r.read_bytes(c['record']);require(hashlib.sha256(raw).hexdigest()==c['sha256'],'Pilot bytes changed');h=write_report(ROOT,a.alias,raw);require(h==c['sha256'],'Alias did not preserve exact payload');vr=ref(a.validation)
 q=dict(schema=1,kind='qualified_current_font_pilot_reference_binding',success=True,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,validation=vr,comparison=c,collector_pilot_reference=dict(record=a.alias,sha256=h),payload_bytes_unchanged=True,all_collector_positive_pilot_assertions_preserved=True,provider_or_extent_acceptance_borrowed=False,scope='Only an exact-payload reference for the existing collector expansion guard. A raw or independently certified native-policy comparison may supply the first fully qualified current font result. Every later provider/extent still requires its own fresh execution, complete independent pixels, font semantics and resource retirement.')
 print(write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode()),flush=True)
if __name__=='__main__':main()
