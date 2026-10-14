#!/usr/bin/env python3
"""Source-bound native command/rule-session execution; no original media or Wine."""
import argparse,hashlib,json,os,re,shlex,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--claims',type=Path);p.add_argument('--ani',type=Path);a=p.parse_args();out=a.output.resolve()
 if out.exists():p.error('Refusing existing evidence output')
 claims=json.loads(a.claims.read_text()) if a.claims else {'sources':{},'claims':[]}
 if any(sha(ROOT/k)!=v for k,v in claims['sources'].items()):raise ValueError('Declared source changed')
 out.mkdir(parents=True);r={'success':False,'claims':claims['claims'],'source_sha256':claims['sources'],'scope':'Native CreatureRuleSession with owned ANI, canonical geometry and recovered creature-pass scheduling; explicit staged effect/world providers, no whole original frame or campaign equivalence.','wine_required':False,'live_replacement':False,'commands':[]}
 def run(name,c,env=None):
  r['commands'].append([str(x) for x in c]);x=subprocess.run(c,cwd=ROOT,capture_output=True,text=True,env=env,timeout=300);(out/(name+'.stdout')).write_text(x.stdout);(out/(name+'.stderr')).write_text(x.stderr)
  if x.returncode or 'ERROR: AddressSanitizer' in x.stderr or 'runtime error:' in x.stderr:raise RuntimeError(name+' failed: '+x.stdout[-3000:]+x.stderr[-3000:])
  return x.stdout
 try:
  source_files=['tests/native-command-action-bodies-session-test.cpp','reconstruction/creature-ai/command_action_bodies.cpp','reconstruction/creature-ai/action_callbacks.cpp','reconstruction/creature-ai/full_motion_action.cpp','reconstruction/creature-ai/special_flight_actions.cpp','reconstruction/creature-ai/object_action.cpp','reconstruction/creature-ai/command_actions_owned.cpp','reconstruction/creature-ai/action_execution.cpp','tests/native-command-action-session-test.cpp','reconstruction/creature-ai/command_behaviors_owned.cpp','reconstruction/creature-ai/movement_behavior.cpp','reconstruction/creature-ai/ranged_execution.cpp','tests/native-command-behavior-session-test.cpp','apps/command-session/runtime.cpp','apps/command-session/codec.cpp','tests/native-command-session-test.cpp','game/simulation/world.cpp','game/simulation/tick_clock.cpp','game/simulation/movement.cpp','game/simulation/occupancy.cpp','game/simulation/creature_rules_session.cpp','game/persistence/snapshot.cpp','game/persistence/store_posix.cpp','reconstruction/combat/control.cpp','reconstruction/creature-ai/transfer_commands_owned.cpp','reconstruction/creature-ai/owner_transfer.cpp','reconstruction/creature-ai/transfer_owned.cpp','reconstruction/creature-ai/ownership_helpers.cpp','reconstruction/creature-ai/owner_counts.cpp','reconstruction/creature-ai/resource.cpp','reconstruction/creature-ai/transfer_targets_owned.cpp','reconstruction/creature-ai/owner_target_repair.cpp','reconstruction/creature-ai/transfer_reset_owned.cpp','reconstruction/creature-ai/execution.cpp','reconstruction/creature-ai/transfer_idle_owned.cpp','reconstruction/creature-ai/behavior_execution.cpp','reconstruction/creature-ai/transfer_follow_owned.cpp','reconstruction/creature-ai/orders.cpp','reconstruction/creature-ai/transfer_queue_owned.cpp','reconstruction/creature-ai/transfer_delivery_owned.cpp']
  source_files += ['tests/native-creature-providers-test.cpp','apps/command-session/native_providers.cpp','apps/command-session/native_geometry.cpp','reconstruction/animation/controller.cpp','reconstruction/pathfinding/route_line.cpp','reconstruction/creature-ai/maintenance.cpp','reconstruction/creature-ai/segment_completion.cpp','reconstruction/creature-ai/full_segment_setup.cpp','reconstruction/creature-ai/environment_pass.cpp','reconstruction/creature-ai/periodic.cpp','reconstruction/creature-ai/route_execution.cpp','reconstruction/creature-ai/death_behavior.cpp','reconstruction/combat/autonomy.cpp','reconstruction/combat/ai.cpp','reconstruction/combat/melee.cpp','reconstruction/motion/creature_profile.cpp','assets/animation_decode.cpp','reconstruction/animation/no_cd.cpp','reconstruction/animation/placement.cpp','reconstruction/animation/attachment.cpp']
  source_files += [f.relative_to(ROOT).as_posix() for f in (ROOT/'reconstruction/pathfinding').glob('route_*.cpp') if not f.name.endswith('replay.cpp')]
  text=run('pre-build-dependencies',['g++','-std=c++17','-Igame','-Iassets','-Ireconstruction/animation','-Ireconstruction/pathfinding','-MM']+source_files)
  initial={'tools/test-native-command-session.py',*source_files}
  for token in shlex.split(text.replace('\\\n',' ')):
   f=(ROOT/token).resolve()
   if f.is_file() and f.is_relative_to(ROOT):initial.add(f.relative_to(ROOT).as_posix())
  for base in ['apps/command-session','game','reconstruction/creature-ai','reconstruction/combat','reconstruction/motion','reconstruction/world','reconstruction/animation','reconstruction/pathfinding','assets']:
   d=ROOT/base
   if d.exists():
    initial.update(f.relative_to(ROOT).as_posix() for f in d.rglob('CMakeLists.txt'))
  r['build_inputs_sha256']={k:sha(ROOT/k) for k in sorted(initial)}
  configure=['cmake','-S',str(ROOT/'apps/command-session')]
  if a.ani:
   a.ani=a.ani.resolve();r['installed_ani_input']={'path':str(a.ani),'sha256':sha(a.ani)};configure+=['-DMNM_NATIVE_PROVIDER_ANI='+str(a.ani)]
  results=[]
  for name,opts in [('native',[]),('sanitized',['-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
   build=out/name;run(name+'-configure',configure+['-B',str(build)]+opts)
   run(name+'-build',['cmake','--build',str(build),'--target','native-command-session-test','native-command-behavior-session-test','native-command-action-session-test','native-command-action-bodies-session-test','native-creature-providers-test','-j2'])
   if name=='native':run('ctest',['ctest','--test-dir',str(build),'-R','^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$','--output-on-failure'])
   results.append(json.loads(run(name,[str(build/'native-command-session-test')],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})))
   results[-1].update(json.loads(run(name+'-behavior',[str(build/'native-command-behavior-session-test')],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})))
   results[-1].update(json.loads(run(name+'-action',[str(build/'native-command-action-session-test')],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})))
   results[-1].update(json.loads(run(name+'-action-bodies',[str(build/'native-command-action-bodies-session-test')],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})))
   results[-1].update(json.loads(run(name+'-providers',[str(build/'native-creature-providers-test')]+([str(a.ani)] if a.ani else []),env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})))
  if a.ani and sha(a.ani)!=r['installed_ani_input']['sha256']:raise ValueError('ANI changed during execution')
  deps={'tools/test-native-command-session.py','apps/command-session/CMakeLists.txt','game/CMakeLists.txt','reconstruction/creature-ai/transfer-commands-owned/CMakeLists.txt'}
  for d in out.rglob('*.o.d'):
   for token in shlex.split(d.read_text().replace('\\\n',' ')):
    f=Path(token)
    if f.is_absolute() and f.is_relative_to(ROOT) and f.is_file():deps.add(f.resolve().relative_to(ROOT).as_posix())
  for cm in out.rglob('CMakeFiles/Makefile.cmake'):
   for token in re.findall(r'"([^"]+)"',cm.read_text()):
    f=Path(token)
    if f.is_absolute() and f.is_file() and f.resolve().is_relative_to(ROOT) and not f.resolve().relative_to(ROOT).as_posix().startswith('working/'):
     deps.add(f.resolve().relative_to(ROOT).as_posix())
  r['compiler_dependencies']=sorted(deps)
  if deps-initial:raise ValueError('Unreviewed compiler/configure input: '+str(sorted(deps-initial)))
  if any(sha(ROOT/k)!=v for k,v in r['build_inputs_sha256'].items()):raise ValueError('Build source changed during compilation')
  if a.claims and deps-claims['sources'].keys():raise ValueError('Unbound build inputs: '+str(sorted(deps-claims['sources'].keys())))
  assert results[0]==results[1]
  r.update(results[0],native_passed=True,sanitizer_passed=True,ctest_passed=True,compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0])
 finally:
  r['sources_unchanged']=all(sha(ROOT/k)==v for k,v in claims['sources'].items());r['build_inputs_unchanged']=all(sha(ROOT/k)==v for k,v in r.get('build_inputs_sha256',{}).items());(out/'report.json').write_text(json.dumps(r,indent=2)+'\n')
 if not r['sources_unchanged']:raise ValueError('Source changed during execution')
 print(json.dumps({k:r[k] for k in ['success','restored_ticks','unsafe_queue_rollbacks','sanitizer_passed']}))
if __name__=='__main__':main()
