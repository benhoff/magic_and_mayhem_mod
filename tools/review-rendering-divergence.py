#!/usr/bin/env python3
"""Verify the oracle defect, unchanged native witnesses and corrected comparisons."""
import argparse,json,hashlib,struct,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-divergence-results-20261101/'
BEHAVIORS=['TOOL.rendering-first-divergence','TOOL.original-distortion-oracle-phase','TOOL.rendering-reference-recomparison']
def sha(data):return hashlib.sha256(data).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
 if (ROOT/a.report).exists():p.error('New immutable review required')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());builds={b['id']:b for b in register['builds']};behaviors={b['id']:b for b in register['behaviors'] if b['id'] in BEHAVIORS}
 scenario=next(s for s in register['scenarios'] if s['id']=='SC.rendering-divergence-regression')
 result=dict(schema=1,success=False,sources={},inputs={},claims=[dict(behavior=n,contract_sha256=behavior_contract(behaviors[n],builds),scenarios={scenario['id']:scenario_contract(scenario)}) for n in BEHAVIORS],
             player_accepted=False,original_pixels_used_as_native_inputs=False,fresh_native_game_execution=False,
             scope='Verified correction of a retained comparison-oracle defect. Fresh mapped-original controls and whole-prefix original replay, immutable native captures, and unchanged native CPU/GPU witnesses. No broad renderer, original-driver, initialization/writer or nominal acceptance promotion.')
 try:
  with Reader(ROOT) as reader:
   def read(stem,positive=False):
    name=PREFIX+stem+'.json';result['inputs'][name]=reader.digest(name);data=reader.json(name)
    if positive:
     for n,h in data['sources'].items():
      if n in result['sources'] and result['sources'][n]!=h:raise ValueError('Different positive source epochs: '+n)
      result['sources'][n]=h
    return data
   before=read('phase-before06');after=read('phase-after07',True)
   if before['comparison']['mismatching_cases']!=384 or before['comparison']['unclipped_mismatches'] or not after['success'] or after['comparison']['mismatching_cases']:raise ValueError('Original phase regression differs')
   result['phase_regression']=dict(cases=after['comparison']['cases'],words_compared=after['comparison']['words_compared'],clipped_before_failures=384,after_failures=0,entries=2,phases=16)
   old=read('high-selected05');fixed=read('high-corrected-draws10',True)
   if old['first_divergent_draw']['native']['ordinal']!=83 or old['exact_prefix_checkpoint_matches']!=82:raise ValueError('First divergence identity differs')
   if not fixed['success'] or fixed['first_divergent_draw'] is not None or not fixed['fresh_original_world50_matches_native'] or fixed['fresh_original_world50_matches_retained']:raise ValueError('Corrected original/native trace differs')
   def artifact(report,path):
    row=next(r for r in report['artifacts'] if r['path']==path);body=reader.read_bytes(row['record'])
    if sha(body)!=row['sha256']:raise ValueError('Witness identity differs')
    return body
   for suffix in ['before','after']:
    native=artifact(old,'native/selected-native-'+suffix+'.565')
    if native!=artifact(fixed,'native/selected-native-'+suffix+'.565') or native!=artifact(fixed,'selected-original-'+suffix+'.565'):raise ValueError('Native witness changed or corrected original differs')
   if artifact(old,'native/selected-native-before.565')!=artifact(old,'selected-original-before.565'):raise ValueError('Pre-fix original entry differs')
   old_native=artifact(old,'native/selected-native-after.565');old_original=artifact(old,'selected-original-after.565')
   mismatch=sum(old_native[i:i+2]!=old_original[i:i+2] for i in range(0,len(old_native),2))
   if mismatch!=493:raise ValueError('Selected pre-fix output difference changed')
   result['cause']=dict(world=50,draw=83,producer_record=164,entry='0x596490',backend=9,top=-19,clip_top=0,skipped_rows=19,
                       before_equal=True,pre_fix_differing_pixels=493,corrected_differing_pixels=0,
                       matched_corrected_draw_checkpoints=fixed['exact_prefix_checkpoint_matches'],native_witnesses_unchanged=True,
                       diagnosis='Effective table already includes vertical clipping. Restore original phase skip-1 rather than UINT32_MAX when replaying that table.')
   result['cases']={}
   for label,stem in [('high','high-recomparison08'),('low','low-recomparison09')]:
    data=read(stem,True)
    if not data['success'] or not data['sources_stable'] or not data['fresh_original_execution'] or data['comparison']['mismatching_pixels'] or not data['input_acknowledgements']['verified'] or not data['fullscreen']['verified']:raise ValueError('Corrected full capture failed: '+label)
    result['cases'][label]=dict(pixels_compared=data['comparison']['pixels_compared'],mismatching_pixels=0,original_reference=data['original_reference'],world_presentations=data['comparison']['world_presentations'],previous=data['previous_result'],input_ack_verified=True,fullscreen_verified=True)
   result['source_executable_sha256']=after['source_executable_sha256'];result['fresh_original_execution']=True
  for name in ['tools/review-rendering-divergence.py','tests/test-rendering-normal-acceptance.py','tests/test-rendering-pilot.py']:
   result['sources'][name]=sha((ROOT/name).read_bytes())
  result['controls']=[]
  for name in ['tests/test-rendering-normal-acceptance.py','tests/test-rendering-pilot.py']:
   run=subprocess.run([sys.executable,'-B',ROOT/name],capture_output=True,text=True,timeout=60)
   result['controls'].append(dict(path=name,returncode=run.returncode,output=run.stdout+run.stderr))
   if run.returncode:raise ValueError('Admission/pilot control failed')
  for b in behaviors.values():
   if not required_sources(b)<=set(result['sources']):raise ValueError('Incomplete source closure: '+b['id'])
  result['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in result['sources'].items())
  result['total_pixels_compared']=sum(c['pixels_compared'] for c in result['cases'].values())
  result['success']=result['sources_stable']
 except Exception as e:result.update(success=False,error=str(e))
 write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps({k:result.get(k) for k in ['success','error','sources_stable','total_pixels_compared','cause']}),flush=True)
 return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
