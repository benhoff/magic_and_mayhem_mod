#!/usr/bin/env python3
"""Validate ten bounded startup, dispatch and finite World-history scopes."""
import argparse,ast,hashlib,importlib.util,json,os,re,shlex,shutil,subprocess,sys,tempfile
from pathlib import Path
from coverage_evidence import evidence_digest,read_json,write_report
from drawing_fixture_prefix import prepare
from rendering_twelfth_controls import last_json
ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['RS.canvas-create-lock', 'RS.canvas-bind-clip', 'RS.canvas-startup-clear', 'RS.world-startup-queues', 'RS.world-startup-wave', 'RS.world-startup-palette', 'RS.world-dispatch-targets', 'RS.world-kind8-virtual', 'NR.world-history-channel', 'NR.world-live-history']
SCENARIO='NR.fourteenth-ten.native-controls-20261022'
TESTS=['tests/test-canvas-driver-observation.py']
TARGETS=['world-channel-test','world-live-session-test','world-frame-test','rendering-tranche2-test','rendering-tranche2-gpu-test','fourteenth_headless_egl']
def sha(p):return evidence_digest(Path(p),root=ROOT)
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',type=Path,required=True);parser.add_argument('--wine-prefix',type=Path,required=True);parser.add_argument('--prefix-owner-root',type=Path,required=True);a=parser.parse_args()
 destination=a.report if a.report.is_absolute() else ROOT/a.report
 if destination.exists():parser.error('Refusing evidence overwrite')
 parent=ROOT/'working/tests/rendering-fourteenth-ten';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(out/'.retain').write_text('Fourteenth raw current controls, binaries, logs, source-only inputs and negative comparisons. Generated evidence is in SQLite.\n');print(out,flush=True)
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());declaration=module('fourteenth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
 todo={ 'tools/test-canvas-startup.py','tools/test-startup-queues.py','tools/test-kind8.py','tools/test-world-dispatch.py','tools/test-rendering-tranche2-reference.py','tools/test-rendering-tranche2-wave.py','tools/build-scene-observer.py','tools/world_channel.py' }|set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-fourteenth-ten.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_fixture_prefix.py','tools/build-drawing-session.py','tools/build-shadow-bridge.py','tools/drawing_comparison.py','tools/original-manifest.sh','tools/test-world-scene.py','tools/test-multi-world-scene.py','tools/test-terrain-preview.py'}
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
  args+=['--bind',str(a.wine_prefix.resolve()),str(a.wine_prefix.resolve())]
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
  if shutil.disk_usage('/var/local/magic-and-mayhem-archive').free<110*1024**3:raise ValueError('Second-drive reserve plus bounded10GiB experiment estimate')
  run('original-before',[Path('/home/hoff/swdev/magic_and_mayhem_mod/tools/original-manifest.sh'),'verify']);report['original_manifest_verified_before']=True
  for i,test in enumerate(TESTS):
   log=run('python-'+str(i),[sys.executable,'-B',ROOT/test]);match=re.search(r'Ran (\d+) tests?',log)
   if not match:raise ValueError('Missing test outcome '+test)
   report['tests_run']+=int(match[1])
  build=ROOT/'working/build/fourteenth'
  run('native-configure',['cmake','-S',ROOT/'tests/rendering-fourteenth-ten','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
  run('native-build',['cmake','--build',build,'--target',*TARGETS,'--parallel','4'],timeout=600)
  binaries={n:next(p for p in build.rglob(n) if p.is_file() and os.access(p,os.X_OK)) for n in TARGETS if n!='fourteenth_headless_egl'}
  report['binaries']={n:dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for n,p in binaries.items()}
  compiled=set()
  for dependency in build.rglob('*.o.d'):
   for token in shlex.split(dependency.read_text().replace('\\\n',' ').split(':',1)[1]):
    p=Path(token).resolve()
    if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):compiled.add(p.relative_to(ROOT).as_posix())
  if compiled-set(report['sources']):raise ValueError('Undeclared compiler inputs: '+repr(sorted(compiled-set(report['sources']))))
  report['compiled_source_inputs']=len(compiled)
  for name in ('world-channel-test','rendering-tranche2-test'):run('native-'+name,[binaries[name]])
  for name in ('world-live-session-test','world-frame-test','rendering-tranche2-gpu-test'):run('native-'+name,['xvfb-run','-a',binaries[name]])
  owner=a.prefix_owner_root.resolve();prefix=a.wine_prefix.resolve()
  common=lambda p:(p/Path(subprocess.check_output(['git','-C',str(p),'rev-parse','--git-common-dir'],text=True).strip())).resolve()
  if common(owner)!=common(ROOT):raise ValueError('Fixture prefix owner must share this Git checkout')
  prefix,lock=prepare(owner,out,prefix,prefix);report['fixture_prefix']=dict(path=str(prefix),owner=str(owner),exclusive_lock=True,idle_before_use=True)
  run('wineboot',['wineboot','-u']);run('prefix-bootstrap-stop',['wineserver','-k'],expected=(0,1));run('prefix-bootstrap-retired',['wineserver','-w']);report['fixture_prefix']['bootstrap_retired_before_namespace_clients']=True
  claims=out/'claims.json';claims.write_text(json.dumps(declaration))
  report['components']={}
  for label,tool,directory,flags in (
   ('startup','test-canvas-startup.py','canvas-startup',['--wine-prefix',prefix]),
   ('queues','test-startup-queues.py','startup-queues',['--claims',claims]),
   ('kind8','test-kind8.py','kind8',[])):
   parent=ROOT/'working/tests'/directory;before=set(parent.glob('run-*'))
   run('pe32-'+label,['xvfb-run','-a',sys.executable,'-B',ROOT/'tools'/tool,*flags],timeout=600)
   added=set(parent.glob('run-*'))-before
   if len(added)!=1:raise ValueError('Missing unique component '+label)
   path=added.pop()/'report.json';component=read_json(path)
   if not component['success']:raise ValueError('Component failed '+label)
   report['components'][label]=dict(path=path.relative_to(ROOT).as_posix(),sha256=sha(path),result=component)
  original=out/'original'
  run('original-startup',[sys.executable,'-B',ROOT/'tools/test-rendering-tranche2-reference.py','--executable',ROOT/'original/Arcane_Nocd/Chaos.exe','--output',original,'--claims',claims,*[v for case in ('create','bind','clear','wave','palette') for v in ('--case',case)]],isolated=True,timeout=600)
  reference=read_json(original/'report.json');assert reference['success'] and reference['sources_stable'];report['original_startup']=reference;report['unchanged_original_fixture_executed']=True
  # The two legacy native executables are under the composed build's canvas directory.
  native_build=binaries['rendering-tranche2-test'].parent
  wave=out/'wave'
  run('original-native-wave',['xvfb-run','-a',sys.executable,'-B',ROOT/'tools/test-rendering-tranche2-wave.py','--reference-report',original/'report.json','--build',native_build,'--claims',claims,'--output',wave],timeout=600)
  report['wave']=read_json(wave/'report.json');assert report['wave']['success']
  hardware=out/'hardware-wave.565'
  extra=dict(QT_QPA_PLATFORM='mnm-headless-egl',QT_QPA_PLATFORM_PLUGIN_PATH=str(build/'platforms'),MNM_PROFILE_EGL_VENDOR='NVIDIA',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json')
  hardware_log=run('hardware-wave',[binaries['rendering-tranche2-gpu-test'],'--wave-pixels',hardware],extra=extra)
  if hardware.read_bytes()!=Path(reference['wave_pixels']['path']).read_bytes():raise ValueError('NVIDIA wave differs')
  report['hardware_wave']=last_json(hardware_log);report['hardware_wave']['sha256']=sha(hardware)
  if 'TITAN' not in report['hardware_wave']['driver']['renderer']:raise ValueError('Actual NVIDIA renderer required')
  dispatch=out/'dispatch'
  run('original-dispatch',['xvfb-run','-a',sys.executable,'-B',ROOT/'tools/test-world-dispatch.py','--output',dispatch,'--wine-prefix',ROOT/'working/tests/scene-selftest-wine'],timeout=600)
  report['dispatch']=read_json(dispatch/'report.json');assert report['dispatch']['success']
  run('stream-build',[sys.executable,'-B',ROOT/'tools/build-scene-observer.py','--selftest'],timeout=600)
  # Execute the actual entry/return assembly with register/flags/floating-point
  # and LastError sentinels, including real shade-minus127 palette admission.
  pebuild=ROOT/'working/build/scene-observer-selftest';objects=[]
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-DMNM_SCENE_STARTUP_REPLAY_SELFTEST']
  for name in ('tests/world-startup-wave-abi.c','tests/canvas-startup-call.S','runtime/scene/world_trace.c','runtime/scene/world_entry.S'):
   obj=out/(Path(name).name+'.obj');run('wave-abi-build-'+Path(name).name,[*flags,'-c',ROOT/name,'-o',obj]);objects.append(obj)
  abi=out/'wave-abi.exe';run('wave-abi-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(abi),*objects,pebuild/'kernel32.lib'])
  run('wave-palette-abi',['xvfb-run','-a','wine',abi]);report['wave_return_abi_passed']=True;report['shade_minus127_palette_capture_passed']=True;report['wave_abi_sha256']=sha(abi)
  from world_channel import create,validate_fresh
  import struct
  producer=ROOT/'working/build/scene-observer-selftest/world-stream-selftest.exe';report['producer_sha256']=sha(producer);modes=[]
  for verify in (False,True):
   for mode in ('','gap','missing','failure','changed'):
    label='stream-'+str(int(verify))+'-'+(mode or 'complete');path=out/(label+'.bin');create(path,verify,16);assert validate_fresh(path)
    run(label,['xvfb-run','-a','wine',producer],extra={'MNM_WORLD_CHANNEL':'Z:'+str(path).replace('/','\\'),'MNM_HISTORY_TEST':mode})
    with path.open('rb') as stream:words=struct.unpack('<32I',stream.read(128))
    expected=(2,16,0) if not mode else (3,1 if mode=='changed' else 0,{'gap':103,'missing':103,'failure':8,'changed':104}[mode])
    if (words[5],words[7],words[10])!=expected:raise ValueError('PE32 history outcome '+label)
    modes.append(dict(verify=verify,mode=mode or 'complete',state=words[5],published=words[7],reason=words[10]))
  report['producer_modes']=modes
  report['sources_stable']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
  current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());report['contracts_stable']=module('fourteenth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])['claims']==declaration['claims']
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
