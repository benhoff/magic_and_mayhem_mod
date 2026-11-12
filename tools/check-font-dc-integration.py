#!/usr/bin/env python3
"""Review exact observer partitioning and retain the accepted execution phases."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
FEATURE='8bde9db09';PREFIX='research/runtime/font-dc-master-results-20261109/'
BID='NP.installed-font-dc-acceptance';SID='native.installed-font-dc-acceptance'
def sha(raw):return hashlib.sha256(raw).hexdigest()
def original(name):return subprocess.check_output(['git','show',FEATURE+':'+name],cwd=ROOT)
def analyze(reader):
 accepted=reader.json('research/runtime/font-dc-lifetime-results-20261108/acceptance-01.json')
 require(accepted['success']and accepted['passed_requirements']==126 and accepted['pending_requirements']==30 and len(accepted['cases'])==4,'Wrong accepted four-case parent')
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes());b=next(b for b in reg['behaviors']if b['id']==BID);s=next(s for s in reg['scenarios']if s['id']==SID)
 names=required_sources(b)|{'tools/check-font-dc-integration.py','tools/coverage_claims.py'}
 source_phases=[];normalizations=[]
 for stem in ['run','prepare']:
  old='tools/'+stem+'-font-lifetime.py';new='tools/'+stem+'-font-dc-lifetime.py';raw=(ROOT/new).read_bytes();normalized=raw.replace(b'run-font-dc-lifetime.py',b'run-font-lifetime.py').replace(b'prepare-font-dc-lifetime.py',b'prepare-font-lifetime.py').replace(b'MNM_FONT_DC_LIFETIME',b'MNM_FONT_LIFETIME')
  require(normalized==original(old),'Frontend changed beyond reviewed path/activation literals: '+new)
  master=subprocess.check_output(['git','show','HEAD:'+old],cwd=ROOT);require((ROOT/old).read_bytes()==master,'Established font CLI was replaced')
  normalizations.append(dict(before=old,after=new,original_sha256=sha(original(old)),current_sha256=sha(raw),only_path_and_activation_literals_changed=True))
 for name in ['apps/qt-shell/font_lifetime_trace.hpp','runtime/render/cpu_reader_journal.h']:
  raw=(ROOT/name).read_bytes();require(raw.replace(b'MNM_FONT_DC_LIFETIME',b'MNM_FONT_LIFETIME')==original(name),'Activation changed another native operation: '+name);normalizations.append(dict(path=name,only_activation_literal_changed=True,original_sha256=sha(original(name)),current_sha256=sha(raw)))
 name='runtime/scene/font_observation.h';raw=(ROOT/name).read_bytes();normalized=raw.replace(b'\n char legacy[2];if(GetEnvironmentVariableA("MNM_FONT_LIFETIME",legacy,sizeof legacy))return 0;SetLastError(0);',b'').replace(b'MNM_FONT_DC_LIFETIME',b'MNM_FONT_LIFETIME');require(normalized==original(name),'FDC hook changed beyond activation/early refusal');normalizations.append(dict(path=name,only_activation_and_prepatch_refusal_changed=True,original_sha256=sha(original(name)),current_sha256=sha(raw)))
 for name in ['apps/qt-shell/menu_fonts.cpp','tools/font_lifetime_acceptance.py','tools/font_lifetime_observation.py','tools/font_native_lifetime.py','tests/world-raster-reference.hpp','tests/acceptance-world-reference.cpp','tests/acceptance-world-history.cpp']:
  require((ROOT/name).read_bytes()==original(name),'Font/data/oracle algorithm changed: '+name);names.add(name)
 controls={}
 for label,path in [('observer','research/runtime/font-dc-lifetime-results-20261108/merged-observer-controls-02.json'),('legacy','research/runtime/font-lifetime-results-20261108/font-dc-merged-controls-02.json'),('dc','research/runtime/font-dc-lifetime-results-20261108/merged-dc-controls-01.json'),('native','research/runtime/font-dc-lifetime-results-20261108/merged-native-font-controls-01.json'),('hidden','research/runtime/font-dc-lifetime-results-20261108/merged-hidden-copy-controls-02.json')]:
  q=reader.json(path);require(q['success']and q['sources_stable']and all(reader.digest(n)==h for n,h in q['sources'].items()),'Current merged control proof missing: '+label)
  controls[label]=dict(record=path,sha256=reader.digest(path));names.update(q['sources'])
  if label=='observer':require(q['control_cases']==18 and q['production_hook_abi_cases']==24,'Current FDC activation/ABI controls missing')
  if label=='legacy':require(q['conflicting_activation_refused']and q['native']['assembly_entries']==11 and q['reader_tests']==22,'Current legacy activation/ABI controls missing')
  if label=='dc':require(any(c.get('name')=='legacy-font-does-not-admit'and c.get('journal_created')is False for c in q['cases']),'Legacy mode admitted nominal DC journal')
 native=reader.json(controls['native']['record']);require(native['registered_fonts_retired']and native['reader_controls']==12 and native['native_menu_variants']==22,'Native menu ownership controls missing');key=native['known_catalogs'][0]['key_sha256']
 for c in accepted['cases']:
  ar=c['pair']['acquisition'];cr=c['pair']['comparison'];aq=reader.json(ar['record']);cq=reader.json(cr['record'])
  require(reader.digest(ar['record'])==ar['sha256']and reader.digest(cr['record'])==cr['sha256']and aq['success']and cq['success'],'Historical execution/comparison identity changed')
  require(cq['font_lifetimes_verified']and cq['independent_output_verified']and cq['native_menu_fonts']['normal_retirement']and all(g['key_sha256']==key for g in cq['native_menu_fonts']['source_catalogs']),'Retained actual font/output proof differs from current tested catalog')
  source_phases.append(dict(acquisition=ar,comparison=cr,execution_sources=aq['sources'],comparison_sources=cq['sources'],current_combined_execution_claimed=False))
 sources={n:reader.digest(n)for n in sorted(names)}
 drift={n:dict(recorded=h,current=reader.digest(n)if(ROOT/n).exists()else None)for n,h in accepted['sources'].items()if not(ROOT/n).exists()or reader.digest(n)!=h}
 result=dict(accepted);result.update(schema=1,kind='font_dc_merged_source_phase_review',success=True,task_complete=True,newly_passed_requirements=0,fresh_native_engine_executions=0,original_game_runs=0,current_combined_player_proved=False,parent=dict(record='research/runtime/font-dc-lifetime-results-20261108/acceptance-01.json',sha256=reader.digest('research/runtime/font-dc-lifetime-results-20261108/acceptance-01.json')),sources=sources,analysis_sources=sources,sources_stable=True,execution_source_phases=source_phases,retained_execution_source_drift=drift,exact_literal_normalizations=normalizations,current_controls=controls,claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],scope='Preserve exactly126accepted rows and both pending decisions from the actual four-case font/DC source phase. Fresh merged headless controls verify both exclusive observer modes, nominal journal gate, unchanged native font catalog/ownership and imported copy policy. Exact normalized source checks permit only this task frontend path/activation literals and prepatch mutual refusal. Every old full native/runtime/execution hash remains in its phase, including unrelated caller-state/viewport/master drift; no current combined-player, new original run or old hash refresh is asserted.')
 return result

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--guards',required=True);a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh merged review required')
 try:
  with Reader(ROOT)as r:
   guards=r.json(a.guards);require(guards['success']and guards['tests']==8 and all(r.digest(n)==h for n,h in guards['sources'].items()),'Current source-phase guard controls required');q=analyze(r);q['guard_controls']=dict(record=a.guards,sha256=r.digest(a.guards))
 except Exception as e:q=dict(schema=1,success=False,error=str(e),original_game_runs=0)
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h,passed=q.get('passed_requirements'),pending=q.get('pending_requirements'))),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
