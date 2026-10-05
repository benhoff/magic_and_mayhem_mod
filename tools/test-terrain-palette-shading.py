#!/usr/bin/env python3
"""Guarded, hash-pinned palette construction and whole-frame draw comparison."""
import argparse,hashlib,json,os,struct,subprocess,tempfile,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 if "--inner" not in sys.argv:
  subprocess.run(["xvfb-run","-a",sys.executable,__file__,*sys.argv[1:],"--inner"],check=True,timeout=1200);return
 sys.argv.remove("--inner")
 p=argparse.ArgumentParser();p.add_argument('--preview',type=Path,required=True);p.add_argument('--sanitized-preview',type=Path,required=True);p.add_argument('--source-root',type=Path,default=ROOT);a=p.parse_args()
 source=a.source_root.resolve();parent=ROOT/'working/tests/terrain-palette-shading';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 def run(cmd,name,env=None):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120,env=env);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r
 def verify(phase):run([ROOT/'tools/original-manifest.sh','verify'],'original-'+phase)
 verify('before')
 try:
  pe=ROOT/'working/game-nocd/Chaos.exe'
  if sha(pe)!=HASH:raise ValueError('Executable hash mismatch')
  binaries=[a.preview.resolve(),a.sanitized_preview.resolve()];root=ROOT/'working/game-clean';paths=[pe,Path(__file__),*binaries]
  for folder in ['assets','renderer','renderer/sprites','reconstruction/rendering','apps/terrain-preview']:
   paths.extend(path for path in (source/folder).glob('*') if path.is_file())
  paths.extend(source/'tests'/name for name in ['palette-shading-reference.cpp','sprite-binary-reference.cpp','palette-shading-test.cpp','palette-shading-native.cpp','sprite-render-test.cpp','terrain-shaded-cache-test.cpp'])
  paths.extend(root/path for path in ['Realms/Celtic/Forest/Terrain.spr','Realms/Celtic/Forest/Terrain.ttd','Realms/Celtic/Celtic.cfg','Realms/Greek/Greek.cfg','Realms/Medieval/Medieval.cfg'])
  inputs={path:sha(path) for path in paths};helper=out/'reference'
  run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie','-I'+str(source/'reconstruction/rendering'),source/'tests/palette-shading-reference.cpp',source/'reconstruction/rendering/palette_shading.cpp','-o',helper],'compile-reference')
  host=out/'native-palettes'
  run(['g++','-std=c++17','-O2','-I'+str(source/'reconstruction/rendering'),source/'tests/palette-shading-native.cpp',source/'reconstruction/rendering/palette_shading.cpp','-o',host],'compile-host-palettes')
  empty=out/'empty.queue';empty.write_bytes(b'');palettes=[]
  for count in [2,4,8,16,32,64,128,256]:
   run([helper,pe,root/'Realms/Celtic/Forest/Terrain.spr',count,empty,out/f'count-{count}.565'],f'count-{count}');palettes.append(count)
   host_output=out/f'host-{count}.palettes';run([host,root/'Realms/Celtic/Forest/Terrain.spr',count,host_output],f'host-{count}')
   assert host_output.read_bytes()==(out/f'count-{count}.565.palettes').read_bytes()
  env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0');frames=[]
  # Every table edge, negative exact multiples and both shade endpoints.
  shades=[-127,-113,-112,-97,-96,-81,-80,-65,-64,-49,-48,-33,-32,-17,-16,-1,0,15,16,31,32,47,48,63,64,79,80,95,96,111,112,127]
  fixtures=[]
  for view in range(4):
   for shade in shades:fixtures.append((['--view',str(view),'--overlap','--visibility'],shade,'tiles'))
  for realm,id in [('Celtic',0),('Greek',0),('Medieval',1)]:
   for view in range(4):
    for shade in [-127,-17,-16,0,16,127]:fixtures.append((['--region-config',f'Realms/{realm}/{realm}.cfg','--region-id',str(id),'--generation-seed','0','--world','--initialize-terrain','--recovered-camera','--view',str(view)],shade,realm))
  for index,(options,shade,realm) in enumerate(fixtures):
   matched=[]
   for build,binary in enumerate(binaries):
    prefix=out/f'frame-{index}-{build}'
    run([binary,'--root',root,*options,'--palette-shading','--light',shade,'--output',prefix],f'frame-{index}-{build}',env)
    native=json.loads(prefix.with_suffix('.json').read_text());assert native['remaining_surfaces']==0 and native['map']['palette_shading'] and all(d['shade']==shade for d in native['queue'])
    queue=out/f'queue-{index}-{build}';queue.write_bytes(b''.join(struct.pack('<Iiii',d['frame'],d['x'],d['y'],d['shade']) for d in native['queue'] if d['kind']!=-2))
    if realm=='tiles':sprite=root/'Realms/Celtic/Forest/Terrain.spr'
    else:
     recipe=native['map']['recipe'];sprite=root/(recipe['sprite_path'].replace('\\','/')+'/Terrain.spr')
    if sprite not in inputs:inputs[sprite]=sha(sprite)
    reference=out/f'original-{index}-{build}.565';run([helper,pe,sprite,16,queue,reference],f'original-{index}-{build}')
    assert prefix.with_suffix('.565').read_bytes()==reference.read_bytes(),(index,build,shade)
    raw=reference.read_bytes();rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for (v,) in struct.iter_unpack('<H',raw))
    assert hashlib.sha256(rgba).hexdigest()==native['rgba_sha256'];matched.append(sha(reference))
   assert matched[0]==matched[1];frames.append(dict(realm=realm,view=native['view'],shade=shade,queue=len(native['queue']),pixels_sha256=matched[0]))
   if index%32==0:print('matched',index+1,'fixtures',flush=True)
  refusals=[]
  for build,binary in enumerate(binaries):
   for options in [['--light','1'],['--palette-shading','--light','-128'],['--palette-shading','--light','128']]:
    prefix=out/f'refused-{build}-{len(refusals)}';r=subprocess.run([str(binary),'--root',str(root),*options,'--output',str(prefix)],capture_output=True,text=True,env=env,timeout=30)
    assert r.returncode==1 and 'Sanitizer' not in r.stderr and not any(prefix.with_suffix(ext).exists() for ext in ['.565','.png','.json']);refusals.append(r.stderr.splitlines()[-1])
  assert all(sha(path)==digest for path,digest in inputs.items())
  report=dict(all_match=True,live_validated=False,executable_sha256=HASH,palette_counts=palettes,palette_formats=['RGB565','RGB555'],reference_helper_sha256=sha(helper),fixture_count=len(frames),native_frames=2*len(frames),refusals=refusals,frames=frames,source_and_input_sha256={str(path.relative_to(ROOT)):digest for path,digest in inputs.items()},boundary='Mode 0 with explicit levels 1 and powers 2; controlled uniform light; original allocator substituted in private memory; builder/draw instructions unchanged; no spatial lighting or effect chains')
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(dict(all_match=True,native_frames=report['native_frames']),flush=True)
 finally:verify('after')
if __name__=='__main__':main()
