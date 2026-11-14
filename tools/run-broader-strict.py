#!/usr/bin/env python3
"""Preserve the strict campaign contract while binding the current player/provider."""
import argparse,hashlib,importlib.util,json,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_native_host import start_nvidia,gpu_environment
from native_render_driver import require_nvidia
from rendering_pilot import require
from broader_strict_inputs import installation_inputs
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def strict_arguments(path,plan,case,report):
 require(case['stress_seconds']in(10,30)and case['size']==[800,600],'Strict supplemental context changed')
 require(case['profile']in('player-default','player-software')and case['provider']==('nvidia'if case['profile']=='player-default'else'llvmpipe'),'Strict provider mismatch')
 return [str(path),'--native-only','--functional-lifecycle','--stress-seconds',str(case['stress_seconds']),'--qt-build-type','Release','--source-game',str(ROOT/plan['source_game']),'--prefix-template',str(ROOT/plan['prefix_template']),'--claims',str(ROOT/plan['claims_input']),'--report',report,*(['--hardware']if case['provider']=='nvidia'else[])]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('plan','case','report'):p.add_argument('--'+n,required=True)
 args=p.parse_args()
 with Reader(ROOT)as r:
  plan=r.json(args.plan);case=next(v for v in plan['broader_cases']if v['id']==args.case)
  require(plan['success']and case['report']==args.report and case['reserved_original_games']==1,'Unbound strict case')
  require(all(r.digest(n)==h for n,h in plan['sources'].items()),'Strict prospective sources changed')
  phase=dict(record=args.plan,sha256=r.digest(args.plan))
 inputs=installation_inputs(ROOT/plan['source_game'])
 path=ROOT/'tools/test-native-owned-campaign.py';old_spec=importlib.util.spec_from_file_location;old_popen=subprocess.Popen
 spec=old_spec('broader_unchanged_strict',path);core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
 hosts=[];launches=[]
 def spec_binding(name,path,*a,**k):
  spec=old_spec(name,path,*a,**k)
  if Path(path).resolve()==ROOT/'tools/test-native-campaign.py'and case['provider']=='nvidia':
   delegate=spec.loader
   class Loader:
    def create_module(self,s):return delegate.create_module(s)
    def exec_module(self,m):
     delegate.exec_module(m)
     def hardware(directory,environment):
      server,receipt,_=start_nvidia(directory);hosts.append(receipt)
      clean=gpu_environment(environment);environment.clear();environment.update(clean,DISPLAY=receipt['display'],XAUTHORITY=receipt['authority']);return server
     m.start_native_hardware=hardware
    def __getattr__(self,n):return getattr(delegate,n)
   spec.loader=Loader()
  return spec
 def popen(command,*a,**k):
  if command and Path(str(command[0]))==ROOT/'tools/run-native-owned.py':
   actual=[sys.executable,str(ROOT/'tools/run-broader-owned-player.py'),'--plan',args.plan,'--case',args.case,'--',*map(str,command[1:])]
   require(not launches,'One bounded original owner required');launches.append(dict(original_command=list(map(str,command)),actual_command=actual))
   k['env']=dict(k['env'],MNM_RESIDENT_STAGE_PROFILE='1');command=actual
  return old_popen(command,*a,**k)
 old_write=core.write_report
 def writer(root,name,raw):
  if str(name)==args.report:
   q=json.loads(raw);q['delegate_success']=q['success'];q.update(kind='current_broader_strict_campaign_acquisition',broader_phase=phase,configuration=dict(software_flag=case['profile']=='player-software',size=case['size']),player_binary=plan['player_binary'],compiler_sources=plan['compiler_sources'],installation_inputs=inputs)
   q['provider_adaptation']=dict(host_receipts=hosts,owner_launches=launches,game_actions_changed=False,response_deadlines_changed=False,original_game_runs_added_by_adapter=0,physical_device_reset=False)
   q['original_game_runs']=1 if launches else 0;q['actual_start_count_complete']=bool(q['success']and len(launches)==1)
   q['sources'].update(plan['sources'])
   with Reader(ROOT)as r:
    q['sources_stable']=all(r.digest(n)==h for n,h in q['sources'].items())
    try:
     launch=P+args.case+'-player-launch-01.json';v=r.json(launch);require(v['success']and len(v['launches'])==1,'Current owned player generation missing');q['player_launch']=dict(record=launch,sha256=r.digest(launch))
     driver=q['resident']['renderer']
     if case['provider']=='nvidia':
      require_nvidia([driver]);require(len(hosts)==1,'Hardware owner missing');e=v['launches'][0]['environment'];require(e['DISPLAY']==hosts[0]['display']and e['XAUTHORITY']==hosts[0]['authority']and e['LIBGL_ALWAYS_SOFTWARE']is None,'Wrong hardware display')
     else:require(driver.lower().startswith(('llvmpipe','softpipe'))and not hosts,'Software route missing')
     require(v['launches'][0]['binary_sha256']==plan['player_binary']['sha256'],'Changed player executable');q['actual_provider_verified']=True
    except(ValueError,KeyError,OSError)as error:q.update(success=False,actual_provider_verified=False,provider_error=str(error))
   q['success']&=q['sources_stable'];raw=(json.dumps(q,indent=2)+'\n').encode()
  return old_write(root,name,raw)
 try:
  importlib.util.spec_from_file_location=spec_binding;subprocess.Popen=popen;core.write_report=writer
  # The exact original test retains its physical actions, callbacks and deadlines.
  sys.argv=strict_arguments(path,plan,case,args.report)
  return core.main()
 finally:importlib.util.spec_from_file_location=old_spec;subprocess.Popen=old_popen;core.write_report=old_write
if __name__=='__main__':raise SystemExit(main())
