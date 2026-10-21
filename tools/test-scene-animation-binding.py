#!/usr/bin/env python3
"""Execute owned resource-bound animation view application without a second clock."""
import argparse,hashlib,json,os,resource,shlex,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--build-root',type=Path);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch();(out/'tmp').mkdir();os.environ['TMPDIR']=str(out/'tmp');build=a.build_root.resolve() if a.build_root else out/'build';build.mkdir(parents=True,exist_ok=False)
 report=dict(schema=1,success=False,original_game_executed=False,scope='Owner-thread revision-bound actual ANI controller binding, phase-preserving switches and explicit receiver clocks through synthetic SPR/ANI resources. Atomic invalid command/continuation/budget refusal and retirement; CPU headless application execution, no original game, renderer hardware or full-scene/live claim.')
 def run(name,cmd):
  r=subprocess.run([str(v) for v in cmd],cwd=ROOT,capture_output=True,text=True,timeout=600);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r.stdout
 sources={}
 try:
  run('configure',['cmake','-S',ROOT/'apps/scene-session','-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
  # Derive the actual selected target's compiler inputs, without executing any
  # emitted command text through a shell. System headers are versioned by the
  # installed toolchain; retain all project compiler dependencies explicitly.
  commands=run('target-commands',['ninja','-C',build,'-t','commands','scene-animation-binding-test']);paths=set()
  for index,line in enumerate(commands.splitlines()):
   args=shlex.split(line)
   if '-c' not in args:continue
   position=args.index('-c');src=Path(args[position+1]);flags=[];skip=False
   for i,value in enumerate(args[1:]):
    if skip:skip=False;continue
    if value in ('-o','-MF','-MT'):skip=True;continue
    if value in ('-c','-MD','-MMD') or value==str(src):continue
    flags.append(value)
   deps=run(f'dependencies-{index}',[args[0],*flags,'-MM',src]).replace('\\\n',' ').split(':',1)[1]
   for name in shlex.split(deps):
    path=(build/name).resolve() if not Path(name).is_absolute() else Path(name).resolve()
    if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):paths.add(path.relative_to(ROOT).as_posix())
  register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in register['behaviors'] if b['id']=='NR.scene-animation-views');scenario=next(s for s in register['scenarios'] if s['id']=='SCENE.animation-views-headless-20261020');builds={b['id']:b for b in register['builds']};paths.update(required_sources(b));sources={n:sha(ROOT/n) for n in sorted(paths)};report.update(sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={scenario['id']:scenario_contract(scenario)})])
  run('build',['cmake','--build',build,'--target','scene-animation-binding-test','--parallel','4'])
  raw=run('native',[build/'scene-animation-binding-test']);result=json.loads(raw.strip().splitlines()[-1]);report['result']=result
  if not result['success'] or not result['phase_preserved'] or not result['revision_budget_thread_retirement'] or not result['double_clock_refused']:raise ValueError('Native animation binding controls incomplete')
  report['binary_sha256']=sha(build/'scene-animation-binding-test');report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  report['sources_stable']=bool(sources) and all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],result=report.get('result'),sources=len(sources),error=report.get('error'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
