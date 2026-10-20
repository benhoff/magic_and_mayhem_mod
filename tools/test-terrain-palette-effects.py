#!/usr/bin/env python3
"""Execute terrain palette chains and CLI binding through native OpenGL scenes."""
import argparse,hashlib,json,os,resource,shlex,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--build-root',type=Path);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch();(out/'tmp').mkdir();os.environ['TMPDIR']=str(out/'tmp');build=a.build_root.resolve() if a.build_root else out/'build';build.mkdir(parents=True,exist_ok=False)
 report=dict(schema=1,success=False,original_game_executed=False,scope='Terrain preview modes, ordinal selection, CLI profiles, non-power count admission and native software OpenGL. Owned ASCII effect-profile production and current resident revision/frame binding to actual native SceneDraw/software OpenGL; synthetic two-palette SPRs, copied colour-table values, immutable shared assets, atomic budgets and owner-thread retirement. No original corpus, foreign source provider, hardware acceptance or live replacement.')
 def run(name,cmd):
  r=subprocess.run([str(v) for v in cmd],cwd=ROOT,capture_output=True,text=True,timeout=600);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r.stdout
 sources={}
 try:
  run('configure',['cmake','-S',ROOT/'apps/terrain-preview','-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
  run('generate-platform-moc',['cmake','--build',build,'--target','mnm_scene_palette_headless_autogen'])
  # Derive the actual selected target's compiler inputs, without executing any
  # emitted command text through a shell. System headers are versioned by the
  # installed toolchain; retain all project compiler dependencies explicitly.
  commands=run('target-commands',['ninja','-C',build,'-t','commands','terrain-shaded-cache-test','mnm-terrain-preview']);paths=set()
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
  register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in register['behaviors'] if b['id']=='NR.terrain-palette-effects-preview');scenario=next(s for s in register['scenarios'] if s['id']=='SCENE.terrain-palette-effects-headless-20261020');builds={b['id']:b for b in register['builds']};paths.update(required_sources(b));sources={n:sha(ROOT/n) for n in sorted(paths)};report.update(sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={scenario['id']:scenario_contract(scenario)})])
  run('build',['cmake','--build',build,'--target','terrain-shaded-cache-test','mnm-terrain-preview','--parallel','4'])
  raw=run('native',['env','QT_QPA_PLATFORM=mnm-headless-egl','QT_QPA_PLATFORM_PLUGIN_PATH='+str(build/'scene-session/platforms'),'MNM_PROFILE_EGL_VENDOR=Mesa','LIBGL_ALWAYS_SOFTWARE=1','QT_FORCE_STDERR_LOGGING=1',build/'terrain-shaded-cache-test',out/'inputs']);result=json.loads(raw.strip().splitlines()[-1]);report['result']=result
  if not result['success'] or result['gpu_frames']!=14 or not result['nonpower_counts_passed']:raise ValueError('Native palette binding controls incomplete')
  env=['env','QT_QPA_PLATFORM=mnm-headless-egl','QT_QPA_PLATFORM_PLUGIN_PATH='+str(build/'scene-session/platforms'),'MNM_PROFILE_EGL_VENDOR=Mesa','LIBGL_ALWAYS_SOFTWARE=1','QT_FORCE_STDERR_LOGGING=1']
  base=[build/'mnm-terrain-preview','--root',out/'inputs','--realm','realm','--definitions','0','--palette-shading','--light','0']
  controls=0
  for mode in range(10):
   prefix=out/('mode-'+str(mode));run('cli-mode-'+str(mode),[*env,*base,'--palette-effect',mode,'--effect-config','effects.ini','--output',prefix]);data=json.loads(prefix.with_suffix('.json').read_text())
   if data['map']['palette_effects']!=[0,mode] or data['map']['effect_ordinal']!=1 or data['remaining_surfaces']!=0:raise ValueError('CLI effect metadata differs')
   controls+=1
  for mode in (0,8):
   prefix=out/('nonpower-'+str(mode));run('cli-nonpower-'+str(mode),[*env,*base,'--palette-effect',mode,'--preferences','prefs.cfg','--output',prefix]);data=json.loads(prefix.with_suffix('.json').read_text())
   if data['map']['palette_count']!=17:raise ValueError('CLI non-power preference refused')
   controls+=1
  prefix=out/'ordinal';run('cli-ordinal', [*env,*base,'--palette-effect',1,'--effect-ordinal',0,'--output',prefix])
  if json.loads(prefix.with_suffix('.json').read_text())['map']['effect_ordinal']!=0:raise ValueError('CLI explicit ordinal differs')
  controls+=1
  for option,value in [('--palette-effect','10'),('--effect-ordinal','99')]:
   r=subprocess.run([str(v) for v in [*env,*base,option,value,'--output',out/'refused']],cwd=ROOT,capture_output=True,text=True,timeout=60)
   (out/('cli-refused-'+option[2:]+'.log')).write_text(r.stdout+r.stderr)
   if r.returncode!=1 or (out/'refused.json').exists():raise ValueError('Invalid CLI effect request published output')
   controls+=1
  report['cli_controls']=controls
  report['binary_sha256']=sha(build/'terrain-shaded-cache-test');report['platform_plugin_sha256']=sha(build/'scene-session/platforms/libmnm_scene_palette_headless.so');report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  report['sources_stable']=bool(sources) and all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],result=report.get('result'),sources=len(sources),error=report.get('error'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
