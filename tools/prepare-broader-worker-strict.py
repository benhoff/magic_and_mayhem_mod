#!/usr/bin/env python3
"""Freeze four supplemental strict B05 controls within the exact126-reservation source-phase cap."""
import argparse,copy,hashlib,json,subprocess,sys
from pathlib import Path
from coverage_claims import behavior_contract,scenario_contract,required_sources
from coverage_evidence import Reader,write_report
from broader_player_sources import python_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--global-plan',required=True);args=p.parse_args()
 require(all(n.startswith(P)and not(ROOT/n).exists()for n in(args.report,)),'Fresh strict plans required')
 test_names=['tests/test-broader-strict-binding.py','tests/test-broader-owned-player.py','tests/test-native-functional-lifecycle.py','tests/test-camera-dialogue.py','tests/test-native-campaign.py'];tests=[]
 for name in test_names:
  v=subprocess.run([sys.executable,name],cwd=ROOT,capture_output=True);tests.append(dict(command=[sys.executable,name],returncode=v.returncode,log=(v.stdout+v.stderr).decode()));require(v.returncode==0,'Strict controls failed')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));g=r.json(args.global_plan);require(g['maximum_additional_original_games']==128 and g['fresh_source_original_starts']==98,'Exact B03 prospective budget required');gh=r.digest(args.global_plan);basis=r.json('research/runtime/surface-loss-restore-results-20261109/bp13-input-plan-01.json');reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors']if v['id']=='NP.functional-rendering-acceptance');scenarios=[v for v in reg['scenarios']if b['id']in v['behaviors']];claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)for s in scenarios})]
  seeds=['tools/run-broader-strict.py','tools/run-broader-owned-player.py','tools/check-broader-strict.py','tools/retire-broader-strict.py','tools/prepare-broader-worker-strict.py']
  names=python_sources(ROOT,seeds)|required_sources(b)|set(test_names)|set(basis['runtime_sources'])|set(basis['compiler_sources']);sources={n:r.digest(n)for n in sorted(names)}
  game=ROOT/'working/broader-player/game-source'
  from broader_strict_inputs import installation_inputs
  installation=installation_inputs(game)
  attempt=Path(args.report).stem.rsplit('-',1)[1];require(attempt.isdecimal(),'Typed prospective revision required')
  declaration=dict(schema=1,claims=claims,sources=sources);raw=(json.dumps(declaration,indent=2)+'\n').encode();claims_name=P+'strict-source-declaration-'+attempt+'.json';ch=write_report(ROOT,claims_name,raw);physical='working/broader-player/strict-claims-'+attempt+'.json';target=ROOT/physical;require(not target.exists(),'Preserve strict input declaration');target.write_bytes(raw)
  controls_name=P+'strict-controls-'+attempt+'.json';controls=dict(schema=1,kind='broader_strict_prospective_controls',success=True,sources_stable=True,original_game_runs=0,test_count=52,tests=tests,sources={n:sources[n]for n in test_names+seeds});cr=dict(record=controls_name,sha256=write_report(ROOT,controls_name,(json.dumps(controls,indent=2)+'\n').encode()))
  cases=[]
  for software in(False,True):
   for seconds in(10,30):
    key='bp13-strict-'+('software'if software else'default')+str(seconds)+'-01';report=P+key+'-acquisition.json';cases.append(dict(id=key,report=report,profile='player-software'if software else'player-default',provider='llvmpipe'if software else'nvidia',size=[800,600],stress_seconds=seconds,reserved_original_games=1,command=['python3','tools/run-broader-strict.py','--plan',args.report,'--case',key,'--report',report]))
  q=dict(schema=1,kind='current_broader_strict_prospective_plan',success=True,original_game_runs=0,broader_plan=dict(record=args.global_plan,sha256=gh),sources=sources,compiler_sources=basis['compiler_sources'],runtime_sources=basis['runtime_sources'],player_binary=basis['player_binary'],broader_cases=cases,source_game='working/broader-player/game-source',prefix_template='working/fixtures/prefix-seed',claims_input=physical,claims_declaration=dict(record=claims_name,sha256=ch),claims=claims,installation_inputs=installation,controls=cr,request_driver='tools/run-broader-worker-source-case.py',automatic_retries=False,maximum_new_original_starts=4,scope='Supplemental strict current B05 controls only; no viewport-pixel credit, FPS threshold, denominator reduction or original-recovery promotion.')
  h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,record=args.report,sha256=h,cases=len(cases),sources=len(sources))))
if __name__=='__main__':main()
