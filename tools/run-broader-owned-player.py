#!/usr/bin/env python3
"""Use the unchanged strict ownership launcher with one exact current binary."""
import argparse,hashlib,importlib.util,json,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_owned_player_binding import player_command
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--plan',required=True);p.add_argument('--case',required=True);p.add_argument('owned_arguments',nargs=argparse.REMAINDER);args=p.parse_args();arguments=args.owned_arguments[1:]if args.owned_arguments[:1]==['--']else args.owned_arguments
 with Reader(ROOT)as r:
  plan=r.json(args.plan);case=next(v for v in plan['broader_cases']if v['id']==args.case);require(plan['success']and all(r.digest(n)==h for n,h in plan['sources'].items()),'Strict current launch source changed')
  binary=ROOT/plan['player_binary']['path'];expected=plan['player_binary']['sha256'];require(hashlib.sha256(binary.read_bytes()).hexdigest()==expected,'Current strict player binary changed')
  ref=dict(record=args.plan,sha256=r.digest(args.plan))
 path=ROOT/'tools/run-native-owned.py';spec=importlib.util.spec_from_file_location('broader_unchanged_owned_launcher',path);core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
 launches=[];original_popen=subprocess.Popen
 class ProcessBinding:
  def __getattr__(self,n):return getattr(subprocess,n)
  def Popen(self,command,*positionals,**keywords):
   if command and Path(str(command[0]))==ROOT/'tools/run-qt-shell.sh':
    actual,env=player_command(ROOT,binary,command,keywords['env'],case['profile']=='player-software');keywords['env']=env
    require(hashlib.sha256(binary.read_bytes()).hexdigest()==expected,'Verified player changed at launch')
    launches.append(dict(original_shell_command=list(map(str,command)),actual_player_command=actual,binary_sha256=expected,environment={n:env.get(n)for n in('DISPLAY','XAUTHORITY','LIBGL_ALWAYS_SOFTWARE','GALLIUM_DRIVER','MESA_LOADER_DRIVER_OVERRIDE','MNM_GLYPH_BACKEND')}));command=actual
   return original_popen(command,*positionals,**keywords)
 core.subprocess=ProcessBinding();q=dict(schema=1,kind='broader_preserved_strict_player_launch',success=False,original_game_runs_added_by_adapter=0,case=args.case,plan=ref,launches=launches,player_reconfigured_or_rebuilt=False,original_launcher_assertions_unchanged=True)
 try:
  sys.argv=[str(path),*arguments];result=core.main();require(result in(None,0),'Original owned launcher refused/failed');require(len(launches)==1,'Expected one current player generation');q['success']=True
 except BaseException as e:
  if isinstance(e,SystemExit)and e.code in(None,0)and len(launches)==1:q['success']=True
  else:q['error']=repr(e)
 finally:
  q['binary_stable']=hashlib.sha256(binary.read_bytes()).hexdigest()==expected;q['success']&=q['binary_stable'];name=P+args.case+'-player-launch-01.json';write_report(ROOT,name,(json.dumps(q,indent=2)+'\n').encode())
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
