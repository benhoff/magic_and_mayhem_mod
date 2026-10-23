#!/usr/bin/env python3
"""Validate ten bounded palette, canvas, ownership and diagnostic scopes."""
import argparse,ast,hashlib,importlib.util,json,os,re,shlex,shutil,subprocess,sys,tempfile
from pathlib import Path
from coverage_evidence import evidence_digest,read_json,write_report
from drawing_fixture_prefix import prepare
from rendering_fifteenth_controls import publication,retained_inputs
from rendering_twelfth_controls import last_json
ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.palette-resources', 'NR.command-checkpoint-attachment', 'RS.world-initial-state', 'RS.canvas-preworld-reuse', 'RS.canvas-producer-sequence', 'RS.world-producer-bypass', 'RS.drawing-publication', 'NP.pixel-forwarding-guard', 'NR.wine-private-backing-ownership', 'NP.legacy-ddraw-admission']
SCENARIO='NR.fifteenth-ten.native-controls-20261022'
TESTS=['tests/test-coverage-review-shards.py', 'tests/test-canvas-initial-history.py', 'tests/test-fixture-prefix-borrow.py', 'tests/test-legacy-ddraw-admission.py', 'tests/test-drawing-publication.py', 'tests/test-pixel-forwarding-guard.py', 'tests/test-drawing-physical-owners.py', 'tests/test-drawing-source-handoff.py', 'tests/test-drawing-source-ownership.py']
TARGETS=['render-palette-resources-test', 'live-command-checkpoint-test', 'live-render-channel-test', 'render-control-test', 'rendering-tranche2-test', 'canvas-world-test', 'mnm-drawing-scene-replay', 'fifteenth_headless_egl']
def sha(p):return evidence_digest(Path(p),root=ROOT)
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',type=Path,required=True);parser.add_argument('--wine-prefix',type=Path,required=True);parser.add_argument('--prefix-owner-root',type=Path,required=True);a=parser.parse_args()
 destination=a.report if a.report.is_absolute() else ROOT/a.report
 if destination.exists():parser.error('Refusing evidence overwrite')
 parent=ROOT/'working/tests/rendering-fifteenth-ten';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(out/'.retain').write_text('Fifteenth raw current controls, binaries, logs, source-only inputs and negative comparisons. Generated evidence is in SQLite.\n');print(out,flush=True)
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());declaration=module('fifteenth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
 todo={ 'tools/test-render-checkpoint.py','tools/test-render-drawing-recovery.py','tools/test-rendering-tranche-fixtures.py','tools/test-canvas-producers.py','tools/test-rendering-tranche2-reference.py','tools/test-drawing-producer-trace.py','tools/test-native-pixel-forward-guard.py','tools/test-drawing-physical-owners.py','tools/test-drawing-source-lock.py','tools/check-first-world-history.py','tools/test-original-legacy-ddraw.py','tools/rendering_fifteenth_controls.py' }|set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-fifteenth-ten.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_fixture_prefix.py','tools/build-drawing-session.py','tools/build-shadow-bridge.py','tools/drawing_comparison.py','tools/original-manifest.sh','tools/test-world-scene.py','tools/test-multi-world-scene.py','tools/test-terrain-preview.py'}
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
  build=ROOT/'working/build/fifteenth'
  run('native-configure',['cmake','-S',ROOT/'tests/rendering-fifteenth-ten','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
  run('native-build',['cmake','--build',build,'--target',*TARGETS,'--parallel','4'],timeout=600)
  binaries={n:next(p for p in build.rglob(n) if p.is_file() and os.access(p,os.X_OK)) for n in TARGETS if n!='fifteenth_headless_egl'}
  report['binaries']={n:dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for n,p in binaries.items()}
  compiled=set()
  for dependency in build.rglob('*.o.d'):
   for token in shlex.split(dependency.read_text().replace('\\\n',' ').split(':',1)[1]):
    p=Path(token).resolve()
    if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):compiled.add(p.relative_to(ROOT).as_posix())
  if compiled-set(report['sources']):raise ValueError('Undeclared compiler inputs: '+repr(sorted(compiled-set(report['sources']))))
  report['compiled_source_inputs']=len(compiled)
  for name in ('render-control-test','rendering-tranche2-test'):run('native-'+name,[binaries[name]])
  for name in ('render-palette-resources-test','canvas-world-test'):run('native-'+name,['xvfb-run','-a',binaries[name]])
  owner=a.prefix_owner_root.resolve();prefix=a.wine_prefix.resolve()
  common=lambda p:(p/Path(subprocess.check_output(['git','-C',str(p),'rev-parse','--git-common-dir'],text=True).strip())).resolve()
  if common(owner)!=common(ROOT):raise ValueError('Fixture prefix owner must share this Git checkout')
  prefix,lock=prepare(owner,out,prefix,prefix);report['fixture_prefix']=dict(path=str(prefix),owner=str(owner),exclusive_lock=True,idle_before_use=True)
  run('wineboot',['wineboot','-u']);run('prefix-bootstrap-stop',['wineserver','-k'],expected=(0,1));run('prefix-bootstrap-retired',['wineserver','-w'])
  claims=out/'claims.json';claims.write_text(json.dumps(declaration));report['components']={}
  def component(label,tool,args,directory=None,timeout=900):
   parent=ROOT/'working/tests'/directory if directory else None;before=set(parent.glob('run-*')) if parent else set()
   run(label,['xvfb-run','-a',sys.executable,'-B',ROOT/'tools'/tool,*args],timeout=timeout)
   if parent:
    added=set(parent.glob('run-*'))-before
    if len(added)!=1:raise ValueError('Missing unique component '+label)
    path=added.pop()/'report.json'
   else:path=destination.parent/(label+'.json')
   value=read_json(path)
   if not value['success']:raise ValueError('Component failed '+label+': '+str(value.get('error')))
   report['components'][label]=dict(path=path.relative_to(ROOT).as_posix(),sha256=sha(path),result=value)
   return value
  component('checkpoint','test-render-checkpoint.py',[binaries['live-command-checkpoint-test'].parent,'--wine-prefix',ROOT/'working/tests/render-wine','--claims',claims],'render-checkpoint')
  component('drawing-recovery','test-render-drawing-recovery.py',[binaries['live-render-channel-test'].parent,'--wine-prefix',ROOT/'working/tests/render-wine'],'render-drawing-recovery')
  palette=out/'palette-lifetime'
  run('palette-lifetime',['xvfb-run','-a',sys.executable,'-B',ROOT/'tools/test-rendering-tranche-fixtures.py','--case','palette','--output',palette,'--wine-prefix',ROOT/'working/tests/render-wine'],timeout=600)
  report['palette_lifetime']=read_json(palette/'report.json');assert report['palette_lifetime']['success']
  for label,tool,flags in (
    ('producer-trace','test-drawing-producer-trace.py',[]),
    ('pixel-guard','test-native-pixel-forward-guard.py',[]),
    ('physical-owners','test-drawing-physical-owners.py',[]),
    ('source-lock','test-drawing-source-lock.py',[])):
   dest=destination.parent/(label+'.json')
   component(label,tool,['--prefix-template',prefix,'--borrow-fixture-prefix',prefix,'--claims',claims,'--report',dest,*flags],timeout=1200)
  producer=destination.parent/'producer-hooks.json'
  component('producer-hooks','test-canvas-producers.py',['--claims',claims,'--report',producer],timeout=1200)
  report['publication']=publication(ROOT,out,run)
  borrow_out=out/'opaque-borrow'
  run('opaque-borrow',[sys.executable,'-B',ROOT/'tools/test-rendering-tranche-fixtures.py','--case','borrow','--output',borrow_out]);report['opaque_borrow']=read_json(borrow_out/'report.json');assert report['opaque_borrow']['success']
  original=out/'original'
  run('original-uniform-caller',[sys.executable,'-B',ROOT/'tools/test-rendering-tranche2-reference.py','--executable',ROOT/'original/Arcane_Nocd/Chaos.exe','--output',original,'--claims',claims,'--case','uniform','--case','caller'],isolated=True,timeout=600)
  report['original_uniform_caller']=read_json(original/'report.json');assert report['original_uniform_caller']['success']
  inputs=retained_inputs(ROOT,out);report['retained_encoded_inputs']=inputs['encoded_inputs'];report['retained_restore_provenance']=str(inputs['restore_provenance'])
  bindings=[(inputs['capture'],inputs['original_capture']),(inputs['game'],inputs['original_game'])]
  replay=destination.parent/'copy-checkpoints.json'
  run('retained-current-replay',[sys.executable,'-B',ROOT/'tools/compare-drawing-copy-checkpoints.py','--reference',inputs['reference'],'--assets-reference',inputs['assets_reference'],'--input-manifest',inputs['input_manifest'],'--executable',binaries['mnm-drawing-scene-replay'],'--claims',claims,'--report',replay],isolated=True,bindings=bindings,timeout=1200)
  report['replay']=dict(path=replay.relative_to(ROOT).as_posix(),sha256=sha(replay),checkpoints=len(read_json(replay)['comparisons']));assert read_json(replay)['success']
  initial=destination.parent/'first-world.json'
  run('first-world-history',[sys.executable,'-B',ROOT/'tools/check-first-world-history.py','--reference',inputs['reference'],'--replay-report',replay,'--claims',claims,'--report',initial],isolated=True,bindings=bindings)
  report['initial_history']=read_json(initial);assert report['initial_history']['success']
  report['legacy_admission']={}
  for variant in ('win98','win95'):
   target=ROOT/'working/tests'/('l98-' if variant=='win98' else 'l95-');target=target.with_name(target.name+out.name[-8:])
   run('legacy-'+variant,[sys.executable,'-B',ROOT/'tools/test-original-legacy-ddraw.py','--output',target,'--prefix-template',prefix,'--session-prefix',prefix,'--borrow-fixture-prefix','--dibeng',Path('/home/hoff/swdev/magic_and_mayhem_mod/working/downloads/dibeng-win98-20261020/extracted/dibeng.dll'),'--runtime-variant',variant,'--windows-version','win98','--trace-module'],isolated=True,timeout=600)
   report['legacy_admission'][variant]=read_json(target/'report.json');assert report['legacy_admission'][variant]['success'] and report['legacy_admission'][variant]['probe_completed'] and not report['legacy_admission'][variant]['builtin_ddraw_fallback_allowed']
  report['sources_stable']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
  current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());report['contracts_stable']=module('fifteenth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])['claims']==declaration['claims']
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
