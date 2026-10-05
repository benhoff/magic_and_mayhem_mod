#!/usr/bin/env python3
"""Bounded original preference/dispatch checks and configured offline scenes."""
import argparse,hashlib,json,os,struct,subprocess,tempfile,sys,shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 if '--inner' not in sys.argv:
  subprocess.run(['xvfb-run','-a',sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=1200);return
 sys.argv.remove('--inner');p=argparse.ArgumentParser();p.add_argument('--preview',type=Path,required=True);p.add_argument('--sanitized-preview',type=Path,required=True);p.add_argument('--source-root',type=Path,default=ROOT);a=p.parse_args();source=a.source_root.resolve()
 parent=ROOT/'working/tests/terrain-preferences';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 def run(cmd,name,env=None):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120,env=env);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r
 def verify(phase):run([ROOT/'tools/original-manifest.sh','verify'],'original-'+phase)
 verify('before')
 try:
  pe=ROOT/'working/game-nocd/Chaos.exe';installed=ROOT/'working/game-clean';binaries=[a.preview.resolve(),a.sanitized_preview.resolve()]
  if sha(pe)!=HASH:raise ValueError('Executable hash mismatch')
  inputs={path:sha(path) for path in [pe,*binaries,Path(__file__)]}
  for folder in ['assets','renderer','renderer/sprites','reconstruction/rendering','apps/terrain-preview']:
   for path in (source/folder).glob('*'):
    if path.is_file():inputs[path]=sha(path)
  for name in ['sprite-binary-reference.cpp','terrain-palette-preference-reference.cpp','palette-shading-reference.cpp','palette-lighting-test.cpp']:inputs[source/'tests'/name]=sha(source/'tests'/name)
  preference=out/'preference';palette=out/'palette';include='-I'+str(source/'reconstruction/rendering');model=source/'reconstruction/rendering/palette_shading.cpp'
  for target,test in [(preference,'terrain-palette-preference-reference.cpp'),(palette,'palette-shading-reference.cpp')]:run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2',include,source/'tests'/test,model,'-o',target],'compile-'+target.name)
  values=['missing',*map(str,range(-2,259)),str(-2147483648),str(2147483647)]
  for initial in [2,4,8,16,32,64,128,256]:
   for i,value in enumerate(values):run([preference,pe,initial,value],f'admission-{initial}-{i}')
  print('matched',len(values)*8,'preference admissions and dispatches',flush=True)
  root=out/'assets';shutil.copytree(installed,root,copy_function=os.link)
  for path in installed.rglob('*'):
   if path.is_file():inputs[path]=sha(path)
  env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0');frames=[]
  counts=[2,4,8,16,32,64,128,256];fixtures=[]
  for i,count in enumerate(counts):
   (root/f'count-{count}.cfg').write_text(f'[video]\nTerrainLightLevels=+{count} ; fixture\n')
   realm,id=[('Celtic',0),('Greek',0),('Medieval',1)][i%3]
   for view in range(4):
    for shade in [-127,0,127]:fixtures.append((realm,id,view,shade,count,f'count-{count}.cfg'))
  for realm,id in [('Celtic',0),('Greek',0),('Medieval',1)]:
   for view in range(4):
    for shade in [-16,16]:fixtures.append((realm,id,view,shade,256,'cfg\\PREFS.CFG'))
  for i,(realm,id,view,shade,count,prefs) in enumerate(fixtures):
   matches=[]
   for build,binary in enumerate(binaries):
    prefix=out/f'frame-{i}-{build}';run([binary,'--root',root,'--region-config',f'Realms/{realm}/{realm}.cfg','--region-id',id,'--generation-seed',0,'--world','--initialize-terrain','--recovered-camera','--view',view,'--palette-shading','--lighting-config','CFG/Encrypted/chaos.cfg','--preferences',prefs,'--light',shade,'--output',prefix],f'frame-{i}-{build}',env)
    native=json.loads(prefix.with_suffix('.json').read_text());assert native['map']['palette_count']==count and native['map']['preferences']==prefs and native['remaining_surfaces']==0
    queue=out/f'queue-{i}-{build}';queue.write_bytes(b''.join(struct.pack('<Iiii',d['frame'],d['x'],d['y'],d['shade']) for d in native['queue'] if d['kind']!=-2));sprite=root/(native['map']['recipe']['sprite_path'].replace('\\','/')+'/Terrain.spr')
    original=out/f'original-{i}-{build}.565';run([palette,pe,sprite,count,queue,original,50,50,2.2,2.2],f'original-{i}-{build}');assert original.read_bytes()==prefix.with_suffix('.565').read_bytes()
    rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for (v,) in struct.iter_unpack('<H',original.read_bytes()));assert hashlib.sha256(rgba).hexdigest()==native['rgba_sha256'];matches.append(sha(original))
   assert matches[0]==matches[1];frames.append(dict(realm=realm,view=view,shade=shade,count=count,preferences=prefs,pixels_sha256=matches[0]))
   if i%24==0:print('matched',i+1,'scene fixtures',flush=True)
  refusals=[]
  for name,value in [('nonpower','17'),('malformed','32junk'),('empty','')]: (root/(name+'.cfg')).write_text('[VIDEO]\nTerrainLightLevels='+value+'\n')
  (root/'missing-key.cfg').write_text('[OTHER]\nTerrainLightLevels=32\n')
  (root/'low.cfg').write_text('[VIDEO]\nTerrainLightLevels=-99\n');(root/'high.cfg').write_text('[VIDEO]\nTerrainLightLevels=999\n')
  for build,binary in enumerate(binaries):
   for name,count in [('missing-key',16),('low',2),('high',256)]:
    prefix=out/f'fallback-{build}-{name}';run([binary,'--root',root,'--palette-shading','--preferences',name+'.cfg','--output',prefix],f'fallback-{build}-{name}',env);assert json.loads(prefix.with_suffix('.json').read_text())['map']['palette_count']==count
   for options in [['--preferences','CFG/prefs.cfg'],*(['--palette-shading','--preferences',name+'.cfg'] for name in ['absent','nonpower','malformed','empty'])]:
    prefix=out/f'refused-{build}-{len(refusals)}';r=subprocess.run(list(map(str,[binary,'--root',root,*options,'--output',prefix])),capture_output=True,text=True,env=env,timeout=30);assert r.returncode==1 and 'Sanitizer' not in r.stderr and not any(prefix.with_suffix(s).exists() for s in ['.565','.png','.json']);refusals.append(r.stderr.splitlines()[-1])
  assert all(sha(path)==digest for path,digest in inputs.items())
  report=dict(all_match=True,live_validated=False,executable_sha256=HASH,admission_dispatch_fixtures=len(values)*8,native_frames=2*len(frames),frames=frames,successful_fallbacks=6,refusals=refusals,source_and_input_sha256={str(path.relative_to(ROOT)):digest for path,digest in inputs.items()},helper_sha256={path.name:sha(path) for path in [preference,palette]},boundary='Selected original VIDEO admission and terrain mode-0 loader argument dispatch; private profile callback, loader observation stub and checked returns; supported powers of two; controlled uniform light; no spatial light generation, effect chains or live replacement')
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(dict(all_match=True,native_frames=report['native_frames']),flush=True)
 finally:verify('after')
if __name__=='__main__':main()
