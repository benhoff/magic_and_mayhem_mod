#!/usr/bin/env python3
"""Validate ten bounded acquisition, recorded replay and launch scopes."""
import argparse,ast,hashlib,importlib.util,json,os,re,shlex,shutil,subprocess,sys,tempfile
from pathlib import Path
from coverage_evidence import evidence_digest,read_json,write_report
from drawing_fixture_prefix import prepare
from rendering_twelfth_controls import last_json
ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.drawing-results-redraw-repeat-stimulus','NR.drawing-worker-prefix-observation-control','NR.producer-trace32-acquisition','NR.storage-session-corpus','NR.drawing-storage-session-comparison','NR.recorded-hardware-session-parity','NR.directdraw-native-opengl-launch','NR.directdraw-public-native-menu','NR.world-gpu-resident-frame','NR.world-gpu-ack-delivery']
SCENARIO='NR.sixteenth-ten.native-controls-20261022'
TESTS=['tests/test-rendering-session-contracts.py','tests/test-drawing-recorded-hardware.py','tests/test-drawing-storage-session.py','tests/test-drawing-copy-order.py','tests/test-drawing-storage-retention.py','tests/test-drawing-journeys.py','tests/test-native-campaign.py','tests/extended-world-producer-test.py']
TARGETS=['world-gpu-resident-test','world-gpu-resident-replay','world-continuous-normal-test','world-gpu-ack-test','world-continuous-channel-test','world-continuous-hash-test','world-continuous-memory-test','mnm-drawing-session-replay','mnm-drawing-present','mnm-drawing-observe','live-render-route-probe']
def sha(p):return evidence_digest(Path(p),root=ROOT)
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',type=Path,required=True);parser.add_argument('--wine-prefix',type=Path,required=True);parser.add_argument('--prefix-owner-root',type=Path,required=True);parser.add_argument('--capture-parent',type=Path,help='Fresh capture output parent below owned working paths, including an explicitly provisioned volume alias');parser.add_argument('--output-volume',type=Path,help='Explicit task-owned output volume mounted writable inside private controls');a=parser.parse_args()
 destination=a.report if a.report.is_absolute() else ROOT/a.report
 if destination.exists():parser.error('Refusing evidence overwrite')
 if a.capture_parent and (not a.capture_parent.absolute().is_relative_to(ROOT/'working') or '..' in a.capture_parent.parts):parser.error('Capture parent must be below owned working paths')
 if a.output_volume and (not a.output_volume.is_dir() or not (a.output_volume/'.retain').is_file() or a.output_volume.resolve() in (Path('/'),Path('/home'),Path('/var'))):parser.error('Require an explicitly pinned task-owned output volume')
 parent=ROOT/'working/tests/rendering-sixteenth-ten';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(out/'.retain').write_text('Sixteenth raw current controls, binaries, logs, source-only inputs and negative comparisons. Generated evidence is in SQLite.\n');print(out,flush=True)
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());declaration=module('sixteenth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
 todo={ 'tools/test-native-gpu-frame-ownership.py','tools/test-native-opengl-launch.py','tools/test-native-world-continuous.py','tools/test-native-campaign.py','tools/capture-drawing-interactive.py','tools/check-recorded-drawing-session.py','tools/run-rendering-hardware-control.py','tools/build-drawing-x11-delivery.py' }|set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-sixteenth-ten.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_fixture_prefix.py','tools/build-drawing-session.py','tools/build-shadow-bridge.py','tools/drawing_comparison.py','tools/original-manifest.sh','tools/test-world-scene.py','tools/test-multi-world-scene.py','tools/test-terrain-preview.py'}
 for directory in ('runtime','assets','audio','renderer','compat/legacy','apps','protocols','reconstruction','game','tests'):
  todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in {'.c','.h','.cpp','.hpp','.S','.inc'} or p.name=='CMakeLists.txt'))
 todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols').rglob('*.json'))
 todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'tools/native-render-profile').rglob('*') if p.is_file() and (p.suffix in {'.cpp','.hpp','.h','.json'} or p.name=='CMakeLists.txt'))
 for directory in ('tests/fixtures/drawing-recorded-session','tests/fixtures/drawing-storage-session','tests/fixtures/drawing-storage-corrected-session'):
  todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).rglob('*') if p.is_file() and p.suffix in {'.json','.gz'})
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
 (out/'tmp').mkdir();(out/'runtime').mkdir(mode=0o700);env.update(TMPDIR=str(out/'tmp'),XDG_RUNTIME_DIR=str(out/'runtime'),PYTHONDONTWRITEBYTECODE='1',QT_QPA_PLATFORM='xcb',WINEPREFIX=str(a.wine_prefix.resolve()),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',LP_NUM_THREADS='8',XDG_CACHE_HOME=str(out/'cache'),MESA_SHADER_CACHE_DIR=str(out/'mesa-cache'),__GL_SHADER_DISK_CACHE_PATH=str(out/'nvidia-cache'))
 lock=None
 def private(command,bindings=(),hardware=False):
  temporary=Path(tempfile.mkdtemp(prefix='x11-host-',dir=out));(temporary/'.X11-unix').mkdir(mode=0o1777);(temporary/'.X11-unix').chmod(0o1777)
  shared=(ROOT/'working/coverage/evidence-pending.sqlite3').resolve().parent
  originals=Path('/home/hoff/swdev/magic_and_mayhem_mod/original')
  args=['bwrap','--die-with-parent','--new-session','--unshare-user','--unshare-pid','--unshare-net','--unshare-ipc','--unshare-uts','--ro-bind','/','/','--bind',str(ROOT),str(ROOT),'--ro-bind',str(originals),str((ROOT/'original').resolve()),'--bind',str(shared),str(shared),'--bind',str(temporary),'/tmp','--proc','/proc','--dev-bind','/dev','/dev']
  args+=['--bind',str(a.wine_prefix.resolve()),str(a.wine_prefix.resolve())]
  if a.output_volume:args+=['--bind',str(a.output_volume.resolve()),str(a.output_volume.resolve())]
  for source,target in bindings:args+=['--ro-bind',str(source),str(target)]
  return [*args,'--',*map(str,command)]
 def run(label,command,expected=0,extra=None,cwd=ROOT,timeout=180,isolated=False,bindings=(),hardware=False):
  if label!='original-after' and module('sixteenth_precommand_claims','tools/draft-coverage-claims.py').declaration(ROOT,json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),BEHAVIORS,[SCENARIO])['claims']!=declaration['claims']:raise ValueError('Contract changed before '+label)
  reserve=2.0
  if label.startswith('original-'):reserve=.005
  elif label.startswith('python-'):reserve=.01
  elif label=='ack-complete':reserve=.5
  elif label.startswith('ack-'):reserve=.05
  elif label=='public-menu':reserve=1.5 if a.output_volume and (ROOT/'working/build/qt-shell').resolve().is_relative_to(a.output_volume.resolve()) else 3.0
  if shutil.disk_usage('/var/local/magic-and-mayhem-archive').free<100*1024**3:raise ValueError('Archive drive hard reserve')
  if shutil.disk_usage(ROOT).free<((100+reserve) if a.output_volume else 110)*1024**3:raise ValueError('Output-drive reserve plus reviewed bounded command estimate')
  if a.output_volume and shutil.disk_usage(a.output_volume).free<30*1024**3:raise ValueError('External output volume lacks the bounded30GiB batch reservation')
  command=list(map(str,command))
  if command[0]=='xvfb-run':command=private([command[0],'-d','-s','-screen 0 1280x1024x24','-e',str(out/(label+'-server.log')),*command[1:]],bindings)
  elif isolated:command=private(command,bindings,hardware)
  print(label,flush=True);path=out/(label+'.log')
  execution_env={**env,**(extra or {})}
  if label in {'trace32','repeat120','disabled120','gpu-corpus'}:
   # Keep the diagnostic original acquisition independent of native timing and
   # cache controls; retain its exact subprocess arguments in the report.
   execution_env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in {'DISPLAY','WAYLAND_DISPLAY','LP_NUM_THREADS','XDG_CACHE_HOME','MESA_SHADER_CACHE_DIR','__GL_SHADER_DISK_CACHE_PATH'}}
   execution_env.update(TMPDIR=str(ROOT/'working/tmp'),PYTHONDONTWRITEBYTECODE='1',QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
  if label in {'standalone','public-menu'} or label.startswith('ack-'):
   execution_env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in {'DISPLAY','WAYLAND_DISPLAY','LP_NUM_THREADS','XDG_CACHE_HOME','MESA_SHADER_CACHE_DIR','__GL_SHADER_DISK_CACHE_PATH','LIBGL_ALWAYS_SOFTWARE','__EGL_VENDOR_LIBRARY_FILENAMES','__GLX_VENDOR_LIBRARY_NAME','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','QT_XCB_GL_INTEGRATION'}}
   execution_env.update(TMPDIR=str(ROOT/'working/tmp'),PYTHONDONTWRITEBYTECODE='1',QT_QPA_PLATFORM='xcb')
  try:
   with path.open('x') as log:r=subprocess.run(command,cwd=cwd,env=execution_env,stdout=log,stderr=subprocess.STDOUT,timeout=timeout)
  except subprocess.TimeoutExpired:
   text=path.read_text(errors='replace');report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit='timeout',log=text));raise
  text=path.read_text(errors='replace');report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit=r.returncode,log=text,log_sha256=hashlib.sha256(text.encode()).hexdigest()))
  if r.returncode not in (expected if isinstance(expected,tuple) else (expected,)):raise ValueError(label+': '+text[-6000:])
  return text
 try:
  if shutil.disk_usage(ROOT).free<(110 if a.output_volume else 130)*1024**3:raise ValueError('Second-drive100GiB reserve plus30GiB bounded estimate')
  prefix,lock=prepare(a.prefix_owner_root.resolve(),out,a.wine_prefix,a.wine_prefix);report['fixture_prefix']=dict(path=str(prefix),exclusive_lease=True,idle_before_copy=True)
  run('original-before',[Path('/home/hoff/swdev/magic_and_mayhem_mod/tools/original-manifest.sh'),'verify']);report['original_manifest_verified_before']=True
  for i,test in enumerate(TESTS):
   log=run('python-'+str(i),[sys.executable,'-B',ROOT/test]);match=re.search(r'Ran (\d+) tests?',log)
   if not match:raise ValueError('Missing test outcome '+test)
   report['tests_run']+=int(match[1])
  build=ROOT/'working/build/sixteenth'
  run('native-configure',['cmake','-S',ROOT/'tests/rendering-sixteenth-ten','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
  run('native-build',['cmake','--build',build,'--target',*TARGETS,'--parallel','4'],timeout=1200)
  binaries={n:next(p for p in build.rglob(n) if p.is_file() and os.access(p,os.X_OK)) for n in TARGETS}
  report['binaries']={n:dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for n,p in binaries.items()}
  compiled=set()
  for dependency in build.rglob('*.o.d'):
   for token in shlex.split(dependency.read_text().replace('\
',' ').split(':',1)[1]):
    p=Path(token).resolve()
    if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):compiled.add(p.relative_to(ROOT).as_posix())
  if compiled-set(report['sources']):raise ValueError('Undeclared compiler inputs: '+repr(sorted(compiled-set(report['sources']))))
  report['compiled_source_inputs']=len(compiled)
  for name in ('world-gpu-ack-test','world-continuous-channel-test','world-continuous-hash-test','world-continuous-memory-test'):run('native-'+name,[binaries[name]])
  for name in ('world-gpu-resident-test','world-continuous-normal-test'):run('native-'+name,['xvfb-run','-a',binaries[name]])
  claims=out/'claims.json';claims.write_text(json.dumps(declaration));report['components']={}
  def execute_component(label,tool,args,group=None,expected=0,hardware=False,timeout=1200):
   parent=ROOT/'working/tests'/group if group else None;before=set(parent.glob('run-*')) if parent else set()
   command=[sys.executable,'-B',ROOT/'tools'/tool,*args]
   if tool=='capture-drawing-interactive.py':command=['xvfb-run','-a',*command]
   run(label,command,expected=expected,isolated=True,hardware=hardware,timeout=timeout)
   if parent:
    added=set(parent.glob('run-*'))-before
    if len(added)!=1:raise ValueError('Missing unique component '+label)
    path=added.pop()/'report.json'
   else:path=destination.parent/(label+'.json')
   value=read_json(path,root=ROOT)
   durable=destination.parent/(label+'.json')
   if path!=durable:write_report(ROOT,durable,(json.dumps(value,indent=2)+'\n').encode())
   report['components'][label]=dict(path=durable.relative_to(ROOT).as_posix(),sha256=sha(durable),result=value)
   if expected==0 and not value.get('success'):raise ValueError(label+': '+str(value.get('error')))
   return value
  # Complete portable policy-positive and immutable unmodified-negative cohorts.
  for label,catalog,expected in [('storage-corrected','tests/fixtures/drawing-storage-corrected-session/session.json',0),('storage-unmodified','tests/fixtures/drawing-storage-session/session.json',1)]:
   args=['--catalog',catalog,'--native',binaries['mnm-drawing-session-replay'],'--presenter',binaries['mnm-drawing-present'],'--headless','--claims',claims,'--report',destination.parent/(label+'.json')]
   run(label,[sys.executable,'-B',ROOT/'tools/check-recorded-drawing-session.py',*args],expected=expected,isolated=True,timeout=1200)
   value=read_json(destination.parent/(label+'.json'),root=ROOT)
   if (expected==0)!=value['success'] or not value['original_reference_admitted'] or value['original_pixels_used_as_native_inputs']:raise ValueError('Recorded session changed outcome '+label)
   report['components'][label]=dict(path=(destination.parent/(label+'.json')).relative_to(ROOT).as_posix(),sha256=sha(destination.parent/(label+'.json')),result=value)
  for label,catalog,expected in [('hardware-corrected','tests/fixtures/drawing-storage-corrected-session/session.json',0),('hardware-unmodified','tests/fixtures/drawing-storage-session/session.json',1)]:
   run(label,[sys.executable,'-B',ROOT/'tools/run-rendering-hardware-control.py','--output',out/(label+'-seat'),'--',sys.executable,'-B',ROOT/'tools/check-recorded-drawing-session.py','--catalog',catalog,'--native',binaries['mnm-drawing-session-replay'],'--presenter',binaries['mnm-drawing-present'],'--hardware','--claims',claims,'--report',destination.parent/(label+'.json')],expected=expected,isolated=True,hardware=True,timeout=1200)
   value=read_json(destination.parent/(label+'.json'),root=ROOT)
   if (expected==0)!=value['success'] or not value['hardware_admission']['verified']:raise ValueError('Hardware recorded outcome '+label)
   report['components'][label]=dict(path=(destination.parent/(label+'.json')).relative_to(ROOT).as_posix(),sha256=sha(destination.parent/(label+'.json')),result=value)
  # Use the original acquisition's exact declared diagnostic flags, not its pixels.
  run('build-egl-observer',[sys.executable,'-B',ROOT/'tools/build-drawing-x11-delivery.py','--egl'])
  library=ROOT/'working/build/drawing-egl-delivery/libmnm-drawing-egl-delivery.so'
  profiles=[('trace32','draw02-trace32-original-03.json',32,None),('repeat120','draw02-results-redraw-repeat-original120-01.json',120,True),('disabled120','draw02-aux-disabled-current-original120-01.json',120,False)]
  allowed={'wine_worker_producer_points','copy_order','close_physical_on_source_complete','cursor_source_gate','cursor_mutex','hud_input_point','hud_state','semantic_state','wine_resource_copy','canvas_storage_transitions','wine_retained_map_source','wine_client_memory','wine_map_storage_point','wine_resource_map','whole_updates','producer_update_binding','engine_cadence','source_held_snapshot','source_worker_lease','source_readonly_lease','source_byte_witness','wine_worker_context','wine_blit_packet','wine_blit_worker','wine_texture_entry','egl_physical'}
  for label,reference,samples,worker in profiles:
   historical=read_json(ROOT/'research/runtime'/reference,root=ROOT);profile=historical['staging']
   flags=['--'+key.replace('_','-') for key in sorted(allowed) if profile.get(key) is True]
   if profile.get('wine_submission'):flags.append('--wine-submission')
   if worker is False:flags=[f for f in flags if f!='--wine-worker-producer-points']
   if worker is True and '--wine-worker-producer-points' not in flags:flags.append('--wine-worker-producer-points')
   schedule=ROOT/'tests/fixtures/drawing-journeys'/('results-redraw-repeat-schedule.json' if samples==120 else 'interactive-schedule.json')
   dest=destination.parent/(label+'.json')
   value=execute_component(label,'capture-drawing-interactive.py',['--source-game',ROOT/'working/game-nocd','--prefix-template',a.wine_prefix,'--schedule',schedule,'--report',dest,'--claims',claims,'--observer-mode','physical','--physical-peer',binaries['mnm-drawing-observe'],'--producer-samples',str(samples),'--physical-oracle-mib','3072' if samples==120 else '1024','--x11-delivery-library',library,*(['--experiment-parent',a.capture_parent] if a.capture_parent else []),*flags],timeout=1200)
   inspector=module('sixteenth_producers_'+label,'tools/inspect-canvas-producers.py');capture=Path(value['experiment'])/'capture';header,records=inspector.decode((capture/'canvas-producers.bin').read_bytes(),complete=True)
   if header[6]!=samples or not value['original_drawing_active'] or not value['original_sections_unchanged'] or not value['sources_stable'] or not value['original_manifest_verified_before_after']:raise ValueError('Acquisition did not close its real original prefix')
   if samples==120 and (len(value['input_schedule']['actions'])!=31 or not value['normal_original_quit_verified'] or value['staging']['wine_worker_producer_points'] is not worker):raise ValueError('Incomplete original stimulus/quit/sidecar policy')
   if worker is False and (capture/'wine-worker-producer-points.bin').exists():raise ValueError('Disabled sidecar was emitted')
   report['components'][label]['closed_source_queues']=samples;report['components'][label]['producer_records']=len(records)
   report['original_game_executed']=True
   if samples==32:corpus=capture/'canvas-producers.bin'
  # The API trace includes full-canvas producers that the bounded rolling
  # World contract deliberately refuses. Acquire its separate existing V3
  # guarded raster profile; never discard unsupported operations from a trace.
  run('build-gpu-corpus-consumer',['cmake','--build',build,'--target','mnm-canvas-producers-live','--parallel','4'],timeout=600)
  history_consumer=next(p for p in build.rglob('mnm-canvas-producers-live') if p.is_file() and os.access(p,os.X_OK))
  report['binaries']['mnm-canvas-producers-live']=dict(path=history_consumer.relative_to(ROOT).as_posix(),sha256=sha(history_consumer))
  history_parent=ROOT/'working/experiments/scene-observer';before=set(history_parent.glob('run-*'))
  run('gpu-corpus',['xvfb-run','-a',sys.executable,'-B',ROOT/'tools/capture-scene-game.py','--canvas-producers','--world-producer-handoff','--samples','32','--world-raster-prefix','32','--world-raster-batch','--producer-oracle-mib','3072','--skip-window-screenshot','--map','2','--magic-items','0','--source-game',ROOT/'working/game-nocd','--prefix-template',a.wine_prefix,'--producer-live',history_consumer,'--claims',claims],timeout=1200)
  added=set(history_parent.glob('run-*'))-before
  if len(added)!=1:raise ValueError('Missing unique guarded GPU-history corpus')
  history_root=added.pop();value=read_json(history_root/'report.json')
  if not value['success'] or not value['sources_stable'] or not value['original_manifest_verified_before_after']:raise ValueError('Guarded GPU-history input acquisition failed')
  durable=destination.parent/'gpu-corpus.json';write_report(ROOT,durable,(json.dumps(value,indent=2)+'\n').encode())
  report['components']['gpu-corpus']=dict(path=durable.relative_to(ROOT).as_posix(),sha256=sha(durable),capture_root=str(history_root),result=value)
  corpus=history_root/'capture/canvas-producers.bin'
  execute_component('gpu-history','test-native-gpu-frame-ownership.py',['--claims',claims,'--corpus',corpus,'--report',destination.parent/'gpu-history.json'],timeout=1200)
  execute_component('standalone','test-native-opengl-launch.py',['--claims',claims,'--prefix-template',a.wine_prefix,'--build',binaries['live-render-route-probe'].parent,'--source-game',ROOT/'working/game-nocd','--report',destination.parent/'standalone.json'],hardware=True,timeout=1200)
  public=execute_component('public-menu','test-native-campaign.py',['--claims',claims,'--source-game',ROOT/'working/game-nocd','--menu-cycles','1','--normal-quit','--skip-movies','--legacy-audio','--hardware'],group='native-campaign',hardware=True,timeout=1500)
  if not public['native_draw_default_required'] or not public['flow']['normal_quit']:raise ValueError('Public route did not prove default owned graphics/normal Quit')
  prepared=None
  for mode,queues in [('complete',128),('consumer-death',128),('producer-death',128),('complete',128)]:
   label='ack-'+('recovery' if prepared is not None and mode=='complete' else mode)
   args=['--claims',claims,'--resident','--functional-only','--build-current','--hardware','--hardware-driver','zink','--queues',str(queues),'--mode',mode,'--source-game',ROOT/'working/game-nocd']
   if prepared is not None:args+=['--prepared',prepared]
   value=execute_component(label,'test-native-world-continuous.py',args,group='native-world-continuous',hardware=True,timeout=1500)
   if mode=='complete':
    if value['admission_policy']['performance_required'] or not value['resident']:raise ValueError('Acknowledgement scope widened to performance')
    prepared=ROOT/value['artifacts'] if 'artifacts' in value else Path(report['components'][label]['result'].get('unused',''))
    # This legacy tool identifies the run by its captured report parent.
    if not prepared.is_dir() or not (prepared/'report.json').exists():
     candidates=[p for p in (ROOT/'working/tests/native-world-continuous').glob('run-*') if (p/'report.json').exists() and read_json(p/'report.json').get('experiment')==value.get('experiment')]
     if len(candidates)!=1:raise ValueError('Ambiguous prepared current build')
     prepared=candidates[0]
  report['sources_stable']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
  current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());report['contracts_stable']=module('sixteenth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])['claims']==declaration['claims']
  if not report['sources_stable'] or not report['contracts_stable']:raise ValueError('Prospective source/contract drift')
  report['success']=True
 except Exception as error:
  import traceback
  report.update(error=str(error),traceback=traceback.format_exc())
 finally:
  if lock is not None:lock.close()
  try:run('original-after',[Path('/home/hoff/swdev/magic_and_mayhem_mod/tools/original-manifest.sh'),'verify']);report['original_manifest_verified_after']=True
  except Exception as error:report.update(success=False,manifest_error=str(error))
  report['sources_stable']=all(sha(ROOT/p)==h for p,h in report['sources'].items());report['success'] &= report['sources_stable']
  write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],tests=report['tests_run'],commands=len(report['commands']),report=str(destination),error=report.get('error'))),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
