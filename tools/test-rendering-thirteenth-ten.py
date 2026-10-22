#!/usr/bin/env python3
"""Execute ten bounded scene, ownership and diagnostic implementation scopes."""
import argparse,ast,hashlib,importlib.util,json,os,re,shlex,shutil,subprocess,sys,tempfile
from pathlib import Path
from coverage_evidence import evidence_digest,read_json,write_report
from drawing_fixture_prefix import prepare
from rendering_thirteenth_controls import providers,diagnostic_inputs,diagnostic_replays
from rendering_twelfth_controls import last_json
ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.terrain_scene','NR.world_scene','NR.minimap-write-candidates','NR.drawing-source-point-native-prediction','NR.drawing-held-source-egl-causality','NR.drawing-copy-checkpoint-comparison','NR.drawing-copy-order-value-diagnostics','NR.drawing-aux-readback-native-comparison','NR.external-backing-ownership','NP.legacy-ne-dpmi-inspection']
SCENARIO='NR.thirteenth-ten.native-controls-20261022'
TESTS=['tests/test-rendering-thirteenth-ten.py','tests/test-drawing-source-prediction.py','tests/test-drawing-held-egl-candidates.py','tests/test-legacy-ne-fault.py','tests/test-drawing-source-handoff.py','tests/test-drawing-source-ownership.py','tests/test-drawing-physical-owners.py','tests/test-drawing-wine-source-map.py']
TARGETS=['world-scene-test','world-picking-test','terrain-shaded-cache-test','terrain-lighting-cycle-test','minimap-candidates-test','minimap-producer-test','mnm-drawing-scene-replay','drawing-scene-test','canvas-sequence-test','thirteenth_headless_egl','thirteenth-minimap','mnm-terrain-preview','mnm-world-scene-preview','mnm-world-sandbox']
def sha(p):return evidence_digest(Path(p),root=ROOT)
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',type=Path,required=True);parser.add_argument('--wine-prefix',type=Path,required=True);parser.add_argument('--prefix-owner-root',type=Path,required=True);a=parser.parse_args()
 destination=a.report if a.report.is_absolute() else ROOT/a.report
 if destination.exists():parser.error('Refusing evidence overwrite')
 parent=ROOT/'working/tests/rendering-thirteenth-ten';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(out/'.retain').write_text('Thirteenth raw current controls, binaries, logs, source-only inputs and negative comparisons. Generated evidence is in SQLite.\n');print(out,flush=True)
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());declaration=module('thirteenth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
 todo=set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-thirteenth-ten.py','tools/rendering_thirteenth_controls.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_fixture_prefix.py','tools/build-drawing-session.py','tools/build-shadow-bridge.py','tools/drawing_comparison.py','tools/original-manifest.sh','tools/test-world-scene.py','tools/test-multi-world-scene.py','tools/test-terrain-preview.py'}
 for directory in ('runtime','assets','renderer','compat/legacy','apps','protocols','reconstruction','game','tests'):
  todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in {'.c','.h','.cpp','.hpp','.S','.inc'} or p.name=='CMakeLists.txt'))
 todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols').rglob('*.json'))
 todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'tools/native-render-profile').rglob('*') if p.is_file() and (p.suffix in {'.cpp','.hpp','.h','.json'} or p.name=='CMakeLists.txt'))
 visited=set()
 while todo:
  name=todo.pop()
  if name in visited:continue
  visited.add(name);path=ROOT/name;declaration['sources'][name]=sha(path)
  if path.suffix=='.py':
   for node in ast.walk(ast.parse(path.read_text())):
    if isinstance(node,ast.Constant) and isinstance(node.value,str) and node.value.endswith('.py'):
     for candidate in (ROOT/node.value,path.parent/node.value,ROOT/'tools'/node.value,ROOT/'tests'/node.value):
      if candidate.is_file() and candidate.resolve().is_relative_to(ROOT):todo.add(candidate.resolve().relative_to(ROOT).as_posix())
    names=([alias.name for alias in node.names] if isinstance(node,ast.Import) else [node.module]+[node.module+'.'+alias.name for alias in node.names] if isinstance(node,ast.ImportFrom) and node.module else [])
    for imported in names:
     relative=Path(*imported.split('.'))
     for base in (ROOT/'tools',ROOT/'protocols/python'):
      for candidate in (base/relative.with_suffix('.py'),base/relative/'__init__.py'):
       if candidate.is_file():todo.add(candidate.relative_to(ROOT).as_posix())
 report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims'],commands=[],tests_run=0,reviewed_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix(),original_game_executed=False,original_comparison_stage_promoted=False,live_replacement=False,complete_gameplay_or_performance_acceptance=False)
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in {'DISPLAY','WAYLAND_DISPLAY','QT_QPA_PLATFORM_PLUGIN_PATH','__EGL_VENDOR_LIBRARY_FILENAMES','__GLX_VENDOR_LIBRARY_NAME','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','QT_XCB_GL_INTEGRATION','LIBGL_ALWAYS_SOFTWARE'}}
 (out/'tmp').mkdir();(out/'runtime').mkdir(mode=0o700);env.update(TMPDIR=str(out/'tmp'),XDG_RUNTIME_DIR=str(out/'runtime'),PYTHONDONTWRITEBYTECODE='1',QT_QPA_PLATFORM='xcb',WINEPREFIX=str(a.wine_prefix.resolve()),WINEDEBUG='-all',XDG_CACHE_HOME=str(out/'cache'),MESA_SHADER_CACHE_DIR=str(out/'mesa-cache'),__GL_SHADER_DISK_CACHE_PATH=str(out/'nvidia-cache'))
 lock=None
 def private(command,bindings=()):
  temporary=Path(tempfile.mkdtemp(prefix='x11-host-',dir=out));(temporary/'.X11-unix').mkdir(mode=0o1777);(temporary/'.X11-unix').chmod(0o1777)
  shared=(ROOT/'working/coverage/evidence-pending.sqlite3').resolve().parent
  originals=Path('/home/hoff/swdev/magic_and_mayhem_mod/original')
  args=['bwrap','--die-with-parent','--new-session','--unshare-user','--unshare-pid','--unshare-net','--unshare-ipc','--unshare-uts','--ro-bind','/','/','--bind',str(ROOT),str(ROOT),'--ro-bind',str(originals),str(ROOT/'original'),'--bind',str(shared),str(shared),'--bind',str(temporary),'/tmp','--proc','/proc','--dev','/dev','--setenv','LIBGL_ALWAYS_SOFTWARE','1','--setenv','__EGL_VENDOR_LIBRARY_FILENAMES','/usr/share/glvnd/egl_vendor.d/50_mesa.json']
  for source,target in bindings:args+=['--ro-bind',str(source),str(target)]
  return [*args,'--',*map(str,command)]
 def run(label,command,expected=0,extra=None,cwd=ROOT,timeout=180,isolated=False,bindings=()):
  command=list(map(str,command))
  if command[0]=='xvfb-run':command=private([command[0],'-d','-s','-screen 0 1280x1024x24','-e',str(out/(label+'-server.log')),*command[1:]],bindings)
  elif isolated:command=private(command,bindings)
  print(label,flush=True);path=out/(label+'.log')
  try:
   with path.open('x') as log:r=subprocess.run(command,cwd=cwd,env={**env,**(extra or {})},stdout=log,stderr=subprocess.STDOUT,timeout=timeout)
  except subprocess.TimeoutExpired:
   text=path.read_text(errors='replace');report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit='timeout',log=text));raise
  text=path.read_text(errors='replace');report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit=r.returncode,log=text,log_sha256=hashlib.sha256(text.encode()).hexdigest()))
  if r.returncode not in (expected if isinstance(expected,tuple) else (expected,)):raise ValueError(label+': '+text[-6000:])
  return text
 try:
  if shutil.disk_usage('/var/local/magic-and-mayhem-archive').free<125*1024**3:raise ValueError('Second-drive reserve plus bounded25GiB experiment estimate')
  run('original-before',[Path('/home/hoff/swdev/magic_and_mayhem_mod/tools/original-manifest.sh'),'verify']);report['original_manifest_verified_before']=True
  for i,test in enumerate(TESTS):
   log=run('python-'+str(i),[sys.executable,'-B',ROOT/test]);match=re.search(r'Ran (\d+) tests?',log)
   if not match:raise ValueError('Missing test outcome '+test)
   report['tests_run']+=int(match[1])
  build=ROOT/'working/build/thirteenth'
  run('native-configure',['cmake','-S',ROOT/'tests/rendering-thirteenth-ten','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
  run('native-build',['cmake','--build',build,'--target',*TARGETS,'--parallel','4'],timeout=600)
  binaries={n:next(p for p in build.rglob(n) if p.is_file() and os.access(p,os.X_OK)) for n in TARGETS if n!='thirteenth_headless_egl'}
  report['binaries']={n:dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for n,p in binaries.items()}
  compiled=set()
  for dependency in build.rglob('*.o.d'):
   for token in shlex.split(dependency.read_text().replace('\\\n',' ').split(':',1)[1]):
    p=Path(token).resolve()
    if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):compiled.add(p.relative_to(ROOT).as_posix())
  if compiled-set(report['sources']):raise ValueError('Undeclared compiler inputs: '+repr(sorted(compiled-set(report['sources']))))
  report['compiled_source_inputs']=len(compiled);report['native']={}
  for backend in ('software','hardware'):
   report['native'][backend]={}
   extra={} if backend=='software' else dict(QT_QPA_PLATFORM='mnm-headless-egl',QT_QPA_PLATFORM_PLUGIN_PATH=str(build/'platforms'),MNM_PROFILE_EGL_VENDOR='NVIDIA',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json')
   for name in ('world-scene-test','terrain-shaded-cache-test'):
    log=run(backend+'-'+name,['xvfb-run','-a',binaries[name]] if backend=='software' else [binaries[name]],extra=extra)
    report['native'][backend][name]=last_json(log) if name.startswith('terrain') else dict(success=True,outcome=log.strip())
   driver=report['native'][backend]['terrain-shaded-cache-test']['driver']['renderer']
   if ('TITAN' not in driver if backend=='hardware' else 'llvmpipe' not in driver):raise ValueError('Unexpected native backend '+driver)
  report['hardware_plugin_sha256']=sha(build/'platforms/libthirteenth_headless_egl.so')
  for name in ('world-picking-test','drawing-scene-test'):run('native-'+name,['xvfb-run','-a',binaries[name]])
  for name in ('terrain-lighting-cycle-test','minimap-candidates-test','minimap-producer-test','canvas-sequence-test'):
   text=run('native-'+name,[binaries[name]])
   if name=='minimap-candidates-test':report['minimap']=last_json(text)
  report['minimap_performance']=last_json(run('native-minimap-performance',[binaries['thirteenth-minimap']]))
  main_root=Path('/home/hoff/swdev/magic_and_mayhem_mod')
  game_binding=[(main_root/'working/game-clean',ROOT/'working/game-clean')]
  # These legacy verification tools consume a physical bounded oracle input.
  terrain_reference=out/'terrain-reference.json'
  old_terrain=read_json(ROOT/'research/runtime/native-terrain-preview.json',root=ROOT)['normal']
  original_path=next(name for name in old_terrain['source_and_input_sha256'] if name.endswith('/report.json'))
  from coverage_evidence import read_bytes
  terrain_raw=read_bytes(main_root/original_path);assert hashlib.sha256(terrain_raw).hexdigest()==old_terrain['source_and_input_sha256'][original_path]
  terrain_reference.write_bytes(terrain_raw)
  existing=set((ROOT/'working/tests/terrain-preview').glob('run-*')) if (ROOT/'working/tests/terrain-preview').exists() else set()
  run('installed-terrain-preview',['xvfb-run','-a',sys.executable,'-B',ROOT/'tools/test-terrain-preview.py',binaries['mnm-terrain-preview'],terrain_reference,'--display-conversion','bit-replicated','--inner'],isolated=True,bindings=game_binding,timeout=300)
  created=set((ROOT/'working/tests/terrain-preview').glob('run-*'))-existing;assert len(created)==1
  report['installed_terrain']=read_json(created.pop()/'report.json')
  multi=out/'multi-world';run('installed-multi-world',['xvfb-run','-a',sys.executable,'-B',ROOT/'tools/test-multi-world-scene.py',binaries['mnm-world-scene-preview'],binaries['mnm-world-sandbox'],multi,'--inner'],isolated=True,bindings=game_binding,timeout=300)
  report['installed_multi_world']=read_json(multi/'report.json')
  owner=a.prefix_owner_root.resolve();prefix=a.wine_prefix.resolve()
  common=lambda p:(p/Path(subprocess.check_output(['git','-C',str(p),'rev-parse','--git-common-dir'],text=True).strip())).resolve()
  if common(owner)!=common(ROOT):raise ValueError('Fixture prefix owner must share this Git checkout')
  prefix,lock=prepare(owner,out,prefix,prefix);report['fixture_prefix']=dict(path=str(prefix),owner=str(owner),exclusive_lock=True,idle_before_use=True)
  run('wineboot',['wineboot','-u']);run('prefix-bootstrap-stop',['wineserver','-k'],expected=(0,1));run('prefix-bootstrap-retired',['wineserver','-w']);run('prefix-keepalive',['wineserver','-p'],expected=(0,1))
  report['providers']=providers(ROOT,out,run,env)
  for directory in ('drawing-session','drawing-session-selftest'):
   for name,digest in read_json(ROOT/'working/build'/directory/'manifest.json',root=ROOT)['sources'].items():
    if report['sources'].get(name)!=digest:raise ValueError('Undeclared production compiler input '+name)
  inputs=diagnostic_inputs(ROOT,out);report['retained_encoded_inputs']=inputs['encoded_inputs'];report['original_artifacts_consumed']=True
  claims=out/'claims.json';claims.write_text(json.dumps(declaration))
  report['diagnostics']=diagnostic_replays(ROOT,out,run,binaries['mnm-drawing-scene-replay'],claims,inputs,destination.parent)
  main_root=Path('/home/hoff/swdev/magic_and_mayhem_mod');legacy=destination.parent/'legacy-ne.json'
  run('legacy-ne-inspection',[sys.executable,'-B',ROOT/'tools/inspect-legacy-dibeng-fault.py','--dll',main_root/'working/downloads/dibeng-win98-20261020/extracted/dibeng.dll','--log',main_root/'working/tests/open-recovery-20261021/d98final/legacy-execute.log','--admission-report',main_root/'working/tests/open-recovery-20261021/d98final/report.json','--report',legacy])
  report['legacy_ne']=dict(path=legacy.relative_to(ROOT).as_posix(),sha256=sha(legacy),faults=[f['service'] for f in read_json(legacy,root=ROOT)['faults']])
  if report['legacy_ne']['faults']!=['get_descriptor','set_descriptor']:raise ValueError('Legacy descriptor attribution changed')
  report['sources_stable']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
  current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());report['contracts_stable']=module('thirteenth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])['claims']==declaration['claims']
  if not report['sources_stable'] or not report['contracts_stable']:raise ValueError('Prospective source/contract drift')
  report['success']=True
 except Exception as error:
  import traceback
  report['error']=str(error);report['traceback']=traceback.format_exc()
 finally:
  if lock is not None:
   try:run('prefix-stop',['wineserver','-k'],expected=(0,1));run('prefix-retired',['wineserver','-w']);report['private_wine_stopped']=True
   except Exception as error:report['success']=False;report['retirement_error']=str(error)
   finally:lock.close()
  try:run('original-after',[Path('/home/hoff/swdev/magic_and_mayhem_mod/tools/original-manifest.sh'),'verify']);report['original_manifest_verified_after']=True
  except Exception as error:report['success']=False;report['manifest_error']=str(error)
  report['sources_stable']=all(sha(ROOT/p)==h for p,h in report['sources'].items());report['success'] &= report['sources_stable']
  write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],tests=report['tests_run'],commands=len(report['commands']),report=str(destination),error=report.get('error'))),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
