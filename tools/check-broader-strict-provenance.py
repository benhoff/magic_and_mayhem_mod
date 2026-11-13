#!/usr/bin/env python3
"""Validate retained strict bytes with exact predeclared runtime provenance."""
import argparse,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_strict_provenance import derive,native_phase,P
from broader_strict_extent import observe
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ['acquisition','report','analysis-plan']:p.add_argument('--'+n,required=True)
 a=p.parse_args();require(not(ROOT/a.report).exists(),'Fresh strict successor validation required')
 with Reader(ROOT)as r:
  acq=dict(record=a.acquisition,sha256=r.digest(a.acquisition));value=r.json(a.acquisition);phase=r.json(value['broader_phase']['record']);cid=next(c['id']for c in phase['broader_cases']if c['report']==a.acquisition);jn=P+cid+'-provenance-join-01.json'
  require(not(ROOT/jn).exists(),'Never overwrite join report');join=derive(ROOT,r,acq);write_report(ROOT,jn,(json.dumps(join,indent=2)+'\n').encode())
 with Reader(ROOT)as r:
  plan=r.json(a.analysis_plan);require(plan['success']and plan['original_game_runs']==0 and plan['strict_phase']==value['broader_phase']and all(r.digest(n)==h for n,h in plan['sources'].items()),'Foreign strict extent contract')
  s=importlib.util.spec_from_file_location('unchanged_strict_functional_assessment',ROOT/'tools/check-broader-strict.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);m.native_phase=native_phase;q=m.assess(r,a.acquisition);q['actual_context']=observe(ROOT,value,plan['actual_native_frame_size']);q['actual_context_analysis']=dict(record=a.analysis_plan,sha256=r.digest(a.analysis_plan));q['runtime_provenance_join']=dict(record=jn,sha256=r.digest(jn));q['sources'].update(plan['sources']);q['sources'].update(phase['runtime_sources']);q['sources'].update({n:r.digest(n)for n in ['tools/broader_strict_provenance.py','tools/check-broader-strict-provenance.py']});q['sources_stable']=all(r.digest(n)==h for n,h in q['sources'].items());q['success']&=q['sources_stable'];q['raw_acquisition_rewritten']=False;q['new_original_games']=0
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,original_game_runs=0)))
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
