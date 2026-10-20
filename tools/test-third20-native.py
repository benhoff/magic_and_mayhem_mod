#!/usr/bin/env python3
"""Third-twenty owned graphics, resource lifetime and observed ownership controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from coverage_claims import behavior_contract, required_sources
ROOT=Path(__file__).resolve().parents[1]
IDS=['NP.native-copy-transforms','NP.native-image-row-copy','NR.directdraw-native-opengl-launch',
     'NR.directdraw-public-native-menu','NR.drawing-mutator-entry-closure',
     'NR.drawing-source-read-reservations','NR.drawing-source-version-guard',
     'NR.presentation-resolver-refusal','NR.private-copy-source-handoff','NR.public-resident-world',
     'NR.source-boundary-quiescence','NR.sprite_scene','NR.sprites','NR.surface-fixture-corpus','NR.surfaces']
ORIGINAL=['NR.sprite_scene','NR.sprites']

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--prefix',type=Path,required=True);p.add_argument('--asset-root',type=Path,required=True);a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require a new working/tests output')
 out.mkdir(parents=True);(out/'.retain').write_text('Third-twenty native, sanitizer, original sprite and owned PE32 controls.\n')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());bs={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']};names=set().union(*(required_sources(bs[bid]) for bid in IDS))
 for directory in ['assets','renderer','runtime','reconstruction','protocols','game','compat/legacy','apps/qt-shell','apps/sprite-scene','tools']:
  for f in (ROOT/directory).rglob('*'):
   if f.is_file() and (f.suffix in ['.cpp','.hpp','.c','.h','.S','.inc','.cmake','.py','.json'] or f.name=='CMakeLists.txt'):names.add(f.relative_to(ROOT).as_posix())
 names.update(['tests/sprite-scene-resource-lifetime-test.cpp','tests/scene-resource-fixtures.hpp','tests/native-only-session-test.cpp','tests/game-hard-restart-test.cpp','tests/copy-timing-test.cpp'])
 sources={n:sha(ROOT/n) for n in sorted(names)}
 def declaration(ids):return dict(sources=sources,claims=[dict(behavior=bid,contract_sha256=behavior_contract(bs[bid],builds)) for bid in ids])
 for label,ids in [('native',IDS),('sprite',['NR.sprites']),('original',ORIGINAL)]:
  (out/(label+'-claims.json')).write_text(json.dumps(declaration(ids),indent=2)+'\n')
 report=dict(schema=1,success=False,**declaration(IDS),commands=[],original_game_executed=False,hardware_claimed=False,live_replacement=False,physical_backing_owned=False,scope='Bounded owned graphics/ABI/renderer and observed guard controls on private PE32 fixtures and software OpenGL. Native resource generation refusal is separate from selected unchanged-original ANI/SPR helpers. Hardware launch/gameplay/FPS, all original writers/aliases and broad live replacement remain pending.')
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in ['LD_PRELOAD','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','QT_XCB_GL_INTEGRATION']};env.update(PYTHONDONTWRITEBYTECODE='1',QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',QT_SCALE_FACTOR='1',LP_NUM_THREADS='4',WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d')
 def run(label,argv,extra=None,timeout=900,display=False):
  argv=[str(x) for x in argv]
  if display:argv=['xvfb-run','-a','-s','-screen 0 1280x1024x24',*argv]
  report['commands'].append(dict(label=label,argv=argv))
  with (out/(label+'.log')).open('x') as log:subprocess.run(argv,cwd=ROOT,env=env|dict(extra or {}),stdout=log,stderr=subprocess.STDOUT,timeout=timeout,check=True)
 normal=ROOT/'working/build/third20-native';san=ROOT/'working/build/third20-sanitized'
 def binary(name):
  hits=[f for f in normal.rglob(name) if f.is_file() and os.access(f,os.X_OK)];assert len(hits)==1,(name,hits);return hits[0]
 def legacy(label,script,folder,args):
  parent=ROOT/'working/tests'/folder;before=set(parent.glob('run-*')) if parent.exists() else set();run(label,[sys.executable,'-B',ROOT/'tools'/script,*args]);new=set(parent.glob('run-*'))-before;assert len(new)==1,(label,new);return json.loads((new.pop()/'report.json').read_text())
 verified=False;original=None
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
  exe=ROOT/'original/Arcane_Nocd/Chaos.exe';assert sha(exe)=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
  installed=ROOT/'working/game-clean'
  if not installed.exists():installed.symlink_to(a.asset_root.resolve(),target_is_directory=True)
  assert installed.resolve()==a.asset_root.resolve()
  mmsprite=ROOT/'working/MMSprite'
  if not mmsprite.exists():mmsprite.symlink_to(a.asset_root.resolve().parent/'MMSprite',target_is_directory=True)
  assert mmsprite.is_dir()
  run('configure',['cmake','-S',ROOT/'apps/sprite-scene','-B',normal,'-DCMAKE_BUILD_TYPE=Debug'])
  targets=['sprite-scene-resource-lifetime-test','sprite-scene-test','sprite-scene-queue-test','attachment-recipe-test','sprite-render-test','sprite-atlas-test','render-blit-test','render-surfaces-test','render-resident-world-test','scene-renderer-test','resource-cache-test','mnm-sprite-preview','mnm-sprite-scene-preview','mnm-render-replay']
  run('build',['cmake','--build',normal,'--target',*targets,'--parallel','4'])
  expression='^(opengl-sprite-resource-lifetime|opengl-animation-scene|opengl-sprite-queue-scene|animation-attachment-recipe|opengl-sprite-frames|opengl-sprite-atlas|opengl-native-blits|opengl-persistent-surfaces|opengl-resident-world-import|native-shared-scene-renderer|native-resource-cache)$'
  run('native-tests',['ctest','--test-dir',normal,'-R',expression,'--output-on-failure'])
  assert 'out of 11' in (out/'native-tests.log').read_text(),'Native CTest selection incomplete'
  report['native_ctests']=11
  run('sanitized-configure',['cmake','-S',ROOT/'apps/sprite-scene','-B',san,'-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'])
  run('sanitized-build',['cmake','--build',san,'--target','sprite-scene-resource-lifetime-test','sprite-scene-test','--parallel','4'])
  run('sanitized-tests',['ctest','--test-dir',san,'-R','^(opengl-sprite-resource-lifetime|opengl-animation-scene)$','--output-on-failure'],extra=dict(ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
  assert 'out of 2' in (out/'sanitized-tests.log').read_text(),'Sanitizer CTest selection incomplete'
  report['sanitizer_ctests']=2
  # Legacy original comparators require the exact current inspector path.
  assets=ROOT/'working/build/assets';run('inspectors-configure',['cmake','-S',ROOT/'assets','-B',assets,'-DBUILD_TESTING=OFF'])
  run('inspectors-build',['cmake','--build',assets,'--target','mnm-sprite-inspect','--parallel','4'])
  sprites=legacy('installed-sprites','test-sprite-render.py','sprite-render',['--preview',binary('mnm-sprite-preview'),'--claims',out/'sprite-claims.json'])
  scenes=legacy('installed-scenes','test-sprite-scene.py','sprite-scene',['--preview',binary('mnm-sprite-scene-preview')])
  report['original_sprites']=sprites;report['original_scenes']=scenes
  run('surface-corpus',[sys.executable,'-B',ROOT/'tools/check-surface-fixtures.py','--backend','opengl','--headless','--gl-executable',binary('mnm-render-replay'),'--report',out/'surface-corpus.json'])
  report['surface_corpus']=json.loads((out/'surface-corpus.json').read_text());assert report['surface_corpus']['success']
  for name in ['test-native-campaign.py','test-native-only-contract.py','native-shell-provenance-test.py','test-surface-fixtures.py','test-render-capture.py']:
   run(name,[sys.executable,'-B',ROOT/'tests'/name])
  run('native-functional',[sys.executable,'-B',ROOT/'tools/test-native-functional-closure.py','--claims',out/'native-claims.json','--output',out/'functional','--shared-fixture-prefix',a.prefix])
  report['functional']=json.loads((out/'functional/report.json').read_text());assert report['functional']['success']
  report['ownership']={}
  for label,script,extra in [('version','test-drawing-source-version.py',[]),('reservations','test-drawing-source-reservations.py',[]),('entries-routes','test-drawing-ownership-bypasses.py',[]),('handoff','test-drawing-physical-owners.py',['--handoff']),('source-lock','test-drawing-source-lock.py',[]),('quiescence','test-drawing-whole-update-boundary.py',['--update-routes'])]:
   result=out/(label+'-report.json');run(label,[sys.executable,'-B',ROOT/'tools'/script,'--claims',out/'native-claims.json','--prefix-template',a.prefix,'--shared-fixture-prefix',a.prefix,'--report',result,*extra],display=True)
   report['ownership'][label]=json.loads(result.read_text());assert report['ownership'][label]['success']
  original=dict(schema=1,success=True,**declaration(ORIGINAL),executable_sha256=sha(exe),sprites=sprites,scenes=scenes,original_game_executed=False,scope='Selected unchanged original ANI and SPR helper comparisons only; no original resource unload/reload or live-game/hardware claim.')
  report.update(success=True,qt_mesa_leak_detector_disabled=True)
 except (AssertionError,OSError,ValueError,subprocess.SubprocessError) as error:report['error']=repr(error)
 finally:
  try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  except (OSError,subprocess.SubprocessError) as error:report['cleanup_error']=str(error);report['success']=False
  report.update(sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()),original_manifest_verified_before_after=verified)
  report['success'] &= report['sources_stable'] and verified
  if report['success'] and original:
   original.update(sources_stable=True,original_manifest_verified_before_after=True);(out/'original-result.json').write_text(json.dumps(original,indent=2)+'\n')
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report.get(k) for k in ['success','sources_stable','error']}))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
