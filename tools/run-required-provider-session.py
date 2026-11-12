#!/usr/bin/env python3
"""Bounded passive provider observations through the existing checkpoint route."""
import argparse,hashlib,importlib.util,json,sys
from pathlib import Path
from coverage_evidence import Reader
from rendering_interaction_budget import admit
from rendering_pilot import require
from required_boundary_observation import PREFIX
ROOT=Path(__file__).resolve().parents[1]

def main():
 p=argparse.ArgumentParser(add_help=False);p.add_argument('--plan',required=True);p.add_argument('--report',required=True)
 p.add_argument('--repair-of');p.add_argument('--repair-reason')
 args,remaining=p.parse_known_args()
 require(bool(args.repair_of)==bool(args.repair_reason),'Repair needs its retained refusal and reason')
 require(args.report.startswith(PREFIX) and args.report.endswith('-acquisition.json') and not(ROOT/args.report).exists(),'Fresh provider report required')
 with Reader(ROOT)as reader:
  plan=reader.json(args.plan)
  require(all(reader.digest(n)==h for n,h in plan['runtime_sources'].items()),'Provider source phase changed')
  cases=[]
  for path in sorted((ROOT/PREFIX).glob('*-acquisition.json')):
   name=path.relative_to(ROOT).as_posix();q=reader.json(name)
   comparisons=[reader.json(v.relative_to(ROOT))for v in sorted((ROOT/PREFIX).glob(path.name.replace('-acquisition.json','-comparison')+'*.json'))]
   proved=any(c.get('success') is True and c.get('acquisition')==dict(record=name,sha256=reader.digest(name)) and c.get('proof',{}).get('verified') is True for c in comparisons)
   cases.append(dict(report=name,result=q,repair_of=q.get('repair_of'),proved=proved,comparison_failed=any(c.get('success') is False for c in comparisons),preparation_fixed=False))
  admission=admit(cases,args.repair_of,plan['bounds']['maximum_requests'],plan['bounds']['maximum_original_games'])
 spec=importlib.util.spec_from_file_location('provider_progression',ROOT/'tools/run-rendering-progression.py')
 b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b);b.PREFIX=PREFIX
 meta_sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in ('tools/run-required-provider-session.py','tools/rendering_interaction_budget.py','tools/required_boundary_observation.py','tools/coverage_evidence.py')}
 require(all(plan['sources'].get(n,plan['runtime_sources'].get(n))==h for n,h in meta_sources.items() if n in plan['sources'] or n in plan['runtime_sources']),'Provider controller source changed')
 actual_commands=[]
 writer=b.write_report
 def write(root,path,raw):
  if path in (args.report,args.report.removesuffix('.json')+'-execution.json'):
   q=json.loads(raw);q.update(prospective_plan=args.plan,required_provider_observation=True,attempt_admission=admission,
     original_game_runs=1 if q.get('session') or q.get('experiments') else 0,
     repair_of=args.repair_of,repair_reason=args.repair_reason)
   q.setdefault('sources',{}).update(meta_sources)
   q['sources_stable']=q.get('sources_stable',True) and all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in meta_sources.items())
   if actual_commands:
    q['public_shell_command']=q.get('command');q['command']=actual_commands[-1]
   q['storage_guard']=dict(reservation_gib=1.5,hard_floor_unchanged=True,scope='Same public Python launcher, same policy and continuous storage guard; cached diagnostic output only, no full frame captures')
   raw=(json.dumps(q,indent=2)+'\n').encode()
  return writer(root,path,raw)
 b.write_report=write
 original_popen=b.subprocess.Popen
 def popen(argv,*a,**kw):
  if argv and str(argv[0])==str(ROOT/'run-native.sh'):
   kw['env']=dict(kw['env'],MNM_REQUIRED_BOUNDARIES='1')
   # run-native.sh contains only help handling and the same guard with a
   # four-GiB estimate. Preserve its policy/continuous floor while choosing
   # the measured cached diagnostic reservation explicitly.
   policy=kw['env'].get('MNM_WORKING_STORAGE_POLICY',str(ROOT/'tools/working-retention.json'))
   argv=[sys.executable,'-B',str(ROOT/'tools/working-retention.py'),'--policy',policy,
     'run','--reserve-gib','1.5','--',sys.executable,'-B',str(ROOT/'tools/run-native-game.py'),*argv[1:]]
   actual_commands.append(list(map(str,argv)))
  return original_popen(argv,*a,**kw)
 b.subprocess.Popen=popen
 sys.argv=[sys.argv[0],'--report',args.report,*remaining]
 return b.main()
if __name__=='__main__':raise SystemExit(main())
