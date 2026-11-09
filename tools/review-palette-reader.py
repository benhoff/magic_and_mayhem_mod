#!/usr/bin/env python3
"""Retain one exact Python reader correction; preserve every native source hash."""
import argparse,hashlib,json,struct
from pathlib import Path
from coverage_evidence import Reader,write_report
from palette_observation import decode
from rendering_pilot import require
from palette_source_phase import PARSER,ANALYSIS_ONLY
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/palette-cycling-results-20261108/'
OLD="   require(h[5]>=last_queue,'Palette queue regression');last_queue=h[5]"
NEW="   if h[5]:require(h[5]>=last_queue,'Palette queue regression');last_queue=h[5]"
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned reader review required')
 q=dict(schema=1,kind='palette_retained_reader_source_applicability',success=False,original_game_runs=0,new_native_engine_executions=0,closes_palette_rows=False)
 with Reader(ROOT)as r:
  aq=r.json(a.acquisition);q['acquisition']=dict(record=a.acquisition,sha256=r.digest(a.acquisition));plan=r.json(aq['input_plan']['record']);require(r.digest(aq['input_plan']['record'])==aq['input_plan']['sha256'],'Input-plan identity changed')
  q['old_sources']=aq['sources'];current={n:r.digest(n)for n in aq['sources']};drift={n for n,h in aq['sources'].items()if current[n]!=h}
  require(drift<={PARSER,ANALYSIS_ONLY}and PARSER in drift,'Not a narrow Python reader/analysis change')
  oldref=plan['source_inputs'][PARSER];old=r.read_bytes(oldref['record']);require(hashlib.sha256(old).hexdigest()==oldref['sha256']==aq['sources'][PARSER],'Old reader source mismatch')
  now=(ROOT/PARSER).read_bytes();require(old.count(OLD.encode())==1 and old.replace(OLD.encode(),NEW.encode())==now,'Reader changed beyond zero local-queue interpretation')
  oldcheck=r.read_bytes(plan['source_inputs'][ANALYSIS_ONLY]['record']);nowcheck=(ROOT/ANALYSIS_ONLY).read_bytes();marker=b"   artifacts={v['path']:v for v in acquired['artifacts']}"
  require(oldcheck[oldcheck.index(marker):]==nowcheck[nowcheck.index(marker):],'Pixel/source/oracle/route comparison changed beyond admission binding')
  run=r.read_bytes(plan['source_inputs']['tools/run-palette-cycle.py']['record']);require(run.count(ANALYSIS_ONLY.encode())==1 and b"'tools/palette_cycle_acceptance.py','tools/check-palette-cycle.py'"in run,'Old acquisition unexpectedly invokes the standalone analysis checker')
  namespace=dict(__name__='palette_exact_historical_reader',__file__=str(ROOT/PARSER));exec(compile(old.decode(),str(ROOT/PARSER)+'@acquired','exec'),namespace)
  artifact=next(v for v in aq['artifacts']if v['path'].endswith('/palette-observation-0001.bin'));raw=r.read_bytes(artifact['record']);require(hashlib.sha256(raw).hexdigest()==artifact['sha256'],'Raw palette custody changed')
  try:namespace['decode'](raw)
  except ValueError as e:q['old_full_refusal']=str(e)
  else:raise ValueError('Expected preserved old full-reader refusal disappeared')
  require(q['old_full_refusal']=='Palette queue regression','Different historical failure')
  at=64;seen=False;boundary=None
  while at<len(raw):
   h=struct.unpack_from('<32I',raw,at)
   if h[2]!=6:
    if seen and not h[5]:boundary=at;break
    seen|=bool(h[5])
   at+=h[0]
  require(boundary is not None,'Missing exact outside-World boundary')
  before=namespace['decode'](raw[:boundary],complete=False);after=decode(raw[:boundary],complete=False);require(before==after,'Reader changes executed World-prefix behavior')
  full=decode(raw);require(full['complete']and full['terminal'][11:13]==[0,0],'Current complete source custody/retirement missing')
  q.update(old_new_world_prefix_equal=True,current_complete_custody=True,world_prefix_bytes=boundary,native_runtime_sources_unchanged=True,effective_sources=current,
   original_input=artifact,analysis_only_paths=[ANALYSIS_ONLY],complete_comparison_suffix_unchanged=True,
   scope='Exact Python-only correction permits local queue0 outside World while preserving positive local-tag regression, every other byte of the strict decoder, and exact decode results for the complete retained World prefix actually available during capture. Continuous local queue/sequence reset is resolved by ordered companion source-packet joins, never sorting or inventing a global generation. The standalone comparison checker is conservatively listed but not invoked by acquisition; its complete source/route/oracle/pixel comparison suffix is unchanged. All native/runtime/player/capture-controller and other recorded sources remain exact. Historical queue refusal and source hashes remain immutable. This supplies source applicability and retained reanalysis, not a new original game or a native full-hash exception.')
  names={PARSER,ANALYSIS_ONLY,'tools/palette_source_phase.py','tools/review-palette-reader.py','tools/rendering_pilot.py','tools/scene_actor_observation.py'};q['sources']={n:r.digest(n)for n in names}
  q['sources_stable']=all(r.digest(n)==h for n,h in q['sources'].items());q['success']=q['sources_stable']
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],record=a.report,sha256=h,world_prefix_bytes=q.get('world_prefix_bytes'))));return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
