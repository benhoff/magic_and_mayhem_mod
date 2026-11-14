#!/usr/bin/env python3
"""Freeze two fresh fullscreen providers without reusing an older native phase."""
import argparse,copy,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_player_sources import python_sources
from required_boundary_observation import CONTROLS
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(P)and not(ROOT/a.report).exists(),'Fresh fullscreen phase required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));parent=ref('research/runtime/required-boundaries-results-20261110/closeout-admission-02.json');proof=r.json(r.json(parent['record'])['proof']['record']);require(proof['gate_ready'],'Historical finite supplier proof missing')
  q=copy.deepcopy(r.json('research/runtime/viewport-cursor-results-20261108/bp13/world-viewport/input-plan-01.json'));prefix='research/runtime/required-boundaries-results-20261110/bp13/';q.update(family='boundary',collector_prefix=prefix,controller_modules=['run-required-boundary-session.py','run-rendering-interaction.py'],counter_bindings={str(ROOT/'working/interaction-attempts.sqlite3'):'working/broader-player/bp13/boundary/interaction-attempts.sqlite3'},domains=[dict(software=s,size=[800,600])for s in(False,True)],bounds=dict(primary_games=2,maximum_requests=2,maximum_original_games=2,stop_on_first_failure=True,automatic_retries=False),boundary_parent=parent,retained_current_software_boundary=None,scope='Two actual current-source fullscreen provider executions, one NVIDIA/default and one Mesa/software. Each requires twelve physical transitions, exact original hover/input, complete independent output,800/640/800recreation and normal retirement. Preserve all156historical rows, supplier boundaries and independent original-recovery gaps; no old source phase is reused as current.')
  seeds=['tools/prepare-broader-worker-boundary.py','tools/compare-broader-boundary.py','tools/check-broader-boundary.py','tools/run-required-boundary-session.py'];q['sources'].update({n:r.digest(n)for n in python_sources(ROOT,seeds)|set(CONTROLS)});cases=[]
  for software in(False,True):
   profile='player-software'if software else'player-default';key='bp13-boundary-'+('software'if software else'default')+'800-01';report=prefix+key+'-acquisition.json';comparison=prefix+key+'-comparison-01.json';arguments=['--plan',a.report,'--report',report]
   if software:arguments.append('--software')
   prior=next(v for v in proof['fullscreen']if v['profile']==profile)
   cases.append(dict(id=key,report=report,reserved_original_games=1,profile=profile,provider='mesa'if software else'nvidia',size=[800,600],command=['python3','tools/run-broader-worker-world.py','--plan',a.report,'--report',report],environment={'ATSPI_DBUS_IMPLEMENTATION':'dbus-daemon'},controller_file='run-required-boundary-session.py',controller_arguments=arguments,comparison=comparison,comparison_command=['python3','tools/compare-broader-boundary.py','--plan',a.report,'--acquisition',report,'--report',comparison],historical_primary=dict(acquisition=prior['acquisition'],comparison=prior['comparison'])))
  q['broader_cases']=cases;require(all(r.digest(n)==h for n,h in(q['sources']|q['compiler_sources']|q['runtime_sources']).items()),'Fullscreen dependency drift');h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,new_original_starts=2,current_software_reused=False)),flush=True)
if __name__=='__main__':main()
