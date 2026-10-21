#!/usr/bin/env python3
"""Execute owned entity map/light integration and bounded atomic lifecycle controls."""
import argparse,hashlib,json,os,resource,shlex,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--build-root',type=Path);p.add_argument('--sanitized',action='store_true');a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch();(out/'tmp').mkdir();os.environ['TMPDIR']=str(out/'tmp');build=a.build_root.resolve() if a.build_root else out/'build';build.mkdir(parents=True,exist_ok=False)
 report=dict(schema=1,success=False,original_game_executed=False,scope='Owned MAP/TTD geometry bindings feed actual recovered terrain queries, signature classification, regional repair queue and blocked/resumed maintenance. Wall close/reopen changes real graph connections; EntityMap reset publishes geometry and queue together. Synthetic CPU integration; no new original whole-reset, installed/full-game/GPU/live claim.')
 def run(name,cmd):
  r=subprocess.run([str(v) for v in cmd],cwd=ROOT,capture_output=True,text=True,timeout=600);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r.stdout
 sources={}
 try:
  run('configure',['cmake','-S',ROOT/'apps/scene-session','-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS_RELEASE=-O2 -DNDEBUG','-DBUILD_TESTING=ON',*(['-DCMAKE_CXX_COMPILER=clang++','-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'] if a.sanitized else [])])
  # Derive the actual selected target's compiler inputs, without executing any
  # emitted command text through a shell. System headers are versioned by the
  # installed toolchain; retain all project compiler dependencies explicitly.
  commands=run('target-commands',['ninja','-C',build,'-t','commands','scene-map-navigation-test']);paths=set()
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
  register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in register['behaviors'] if b['id']=='NR.scene-map-navigation');scenario=next(s for s in register['scenarios'] if s['id']=='SCENE.scene-map-navigation-headless-20261020');builds={b['id']:b for b in register['builds']};paths.update(required_sources(b));sources={n:sha(ROOT/n) for n in sorted(paths)};report.update(sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={scenario['id']:scenario_contract(scenario)})])
  run('build',['cmake','--build',build,'--target','scene-map-navigation-test','--parallel','4'])
  raw=run('native',[build/'scene-map-navigation-test']);result=json.loads(raw.strip().splitlines()[-1]);report['result']=result
  if not result or not all(v is True for v in result.values()):raise ValueError('Entity integration controls incomplete')
  report['sanitized']=a.sanitized
  report['binary_sha256']=sha(build/'scene-map-navigation-test');report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  report['sources_stable']=bool(sources) and all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],result=report.get('result'),sources=len(sources),error=report.get('error'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
