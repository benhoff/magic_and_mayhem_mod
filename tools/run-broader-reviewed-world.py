#!/usr/bin/env python3
"""Preserve each existing World adapter while fixing its exact canonical-plan guard."""
import argparse,hashlib,importlib.util,json,os,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
from broader_canonical_plan_guard import ENTRYPOINTS,binding_class
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--plan',required=True);p.add_argument('--report',required=True);a=p.parse_args()
 with Reader(ROOT)as r:
  phase=r.json(a.plan);family=phase['family'];require(family in ENTRYPOINTS and phase['success'],'Unreviewed guarded World family');case=next(c for c in phase['broader_cases']if c['report']==a.report);controller,entry=ENTRYPOINTS[family];require(case['controller_file']==controller and phase['delegated_entrypoint']=='tools/'+entry,'Changed original World adapter')
  review=phase['canonical_plan_guard_review'];require(r.digest(review['record'])==review['sha256'],'Canonical-plan review changed');q=r.json(review['record']);require(q['success']and q['original_game_runs']==0 and q['Native_sources_unchanged']and all(r.digest(n)==h for n,h in q['sources'].items()),'Stale canonical-plan controls');require(a.report.startswith(phase['collector_prefix'])and a.plan.startswith(P),'Unbound guarded report/phase')
  sources={n:r.digest(n)for n in['tools/run-broader-reviewed-world.py','tools/broader_canonical_plan_guard.py','tools/broader_attachment_plan_path.py']}
 if os.environ.get('MNM_BROADER_WORLD_BUS_OWNED')!='1':
  runtime=ROOT/'working/broader-player/world-runtime'/Path(a.report).stem;runtime.mkdir(mode=0o700,parents=True,exist_ok=False);env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_WORLD_BUS_OWNED='1',MNM_INTERACTION_BUS_OWNED='1',MNM_PAUSE_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon');env.pop('AT_SPI_BUS_ADDRESS',None);os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
 spec=importlib.util.spec_from_file_location('reviewed_canonical_World',ROOT/'tools'/entry);core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core);core.WorldBinding=binding_class(core.WorldBinding,controller);original=coverage_evidence.write_report
 def writer(root,path,raw):
  if str(path)==a.report:
   q=json.loads(raw);q['sources'].update(sources);q['sources_stable']&=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());q['success']&=q['sources_stable'];q['canonical_plan_guard_review']=review;raw=(json.dumps(q,indent=2)+'\n').encode()
  return original(root,path,raw)
 coverage_evidence.write_report=writer
 try:return core.main()
 finally:coverage_evidence.write_report=original
if __name__=='__main__':raise SystemExit(main())
