#!/usr/bin/env python3
"""Keep the provider controller exact; bind bounded drain admission and repair ancestry."""
import hashlib,importlib.util,json,os,sys
from pathlib import Path
from types import SimpleNamespace
from coverage_evidence import Reader
import coverage_evidence
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]

def repair_arguments(argv,case):
 if argv and Path(argv[0]).name=='run-rendering-transition.py'and case.get('repair_of'):
  require('--repair-of'not in argv and '--repair-reason'not in argv,'Duplicate repair ancestry')
  return [*argv,'--repair-of',case['repair_of']['record'],'--repair-reason',case['repair_reason']]
 return argv

def main():
 args=sys.argv[1:];plan_name=args[args.index('--plan')+1];report=args[args.index('--report')+1]
 if os.environ.get('MNM_BROADER_RECOVERY_BUS_OWNED')!='1':
  runtime=ROOT/'working/broader-player/recovery-runtime'/Path(report).stem;runtime.mkdir(parents=True,mode=0o700,exist_ok=False);env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_RECOVERY_BUS_OWNED='1',MNM_TRANSITION_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon');env.pop('AT_SPI_BUS_ADDRESS',None);os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*args],env)
 with Reader(ROOT)as r:
  plan=r.json(plan_name);case=next(v for v in plan['broader_cases']if v['report']==report);sources=dict(plan['sources']);sources['tools/run-broader-drained-recovery.py']=r.digest('tools/run-broader-drained-recovery.py');require(all(r.digest(n)==h for n,h in sources.items()),'Drain controller source drift')
  if case.get('repair_of'):
   prior=case['repair_of'];require(r.digest(prior['record'])==prior['sha256']and r.json(prior['record'])['success']is False and case['repair_reason'],'Missing exact failed repair ancestor')
 original_write=coverage_evidence.write_report
 def writer(root,path,raw):
  if path==report:
   value=json.loads(raw);stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());value.setdefault('sources',{}).update(sources);value['sources_stable']=value.get('sources_stable',True)and stable;value['success']=value['success']and stable;value['drained_admission_adapter']=dict(native_player_changed=False,response_deadlines_changed=False,physical_actions_changed=False,helper_call_timeout_seconds=5,helper_wait_budget_seconds=2,only_exact_pre_mutation_drain_refusal_retried=True,repair_of=case.get('repair_of'),repair_reason=case.get('repair_reason'));raw=(json.dumps(value,indent=2)+'\n').encode()
  return original_write(root,path,raw)
 coverage_evidence.write_report=writer
 spec=importlib.util.spec_from_file_location('unchanged_provider_recovery',ROOT/'tools/run-broader-recovery.py');core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
 class System:
  def __getattr__(self,name):return getattr(sys,name)
  def __setattr__(self,name,value):
   require(name=='argv','Only collector argument binding may change');sys.argv=repair_arguments(value,case)
 core.sys=System()
 return core.main()
if __name__=='__main__':raise SystemExit(main())
