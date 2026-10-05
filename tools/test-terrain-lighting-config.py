#!/usr/bin/env python3
"""Offline original GLOBAL_OPTIONS admission and configured palette/scene checks."""
import argparse,hashlib,json,os,struct,subprocess,tempfile,sys,random
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 if '--inner' not in sys.argv:
  subprocess.run(['xvfb-run','-a',sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=1200);return
 sys.argv.remove('--inner');p=argparse.ArgumentParser();p.add_argument('--preview',type=Path,required=True);p.add_argument('--sanitized-preview',type=Path,required=True);p.add_argument('--source-root',type=Path,default=ROOT);a=p.parse_args();source=a.source_root.resolve()
 parent=ROOT/'working/tests/terrain-lighting-config';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 def run(cmd,name,env=None):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120,env=env);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r
 def verify(phase):run([ROOT/'tools/original-manifest.sh','verify'],'original-'+phase)
 verify('before')
 try:
  pe=ROOT/'working/game-nocd/Chaos.exe';root=ROOT/'working/game-clean';config=root/'CFG/Encrypted/chaos.cfg';binaries=[a.preview.resolve(),a.sanitized_preview.resolve()]
  if sha(pe)!=HASH:raise ValueError('Executable hash mismatch')
  inputs={path:sha(path) for path in [pe,config,*binaries,Path(__file__)]}
  for folder in ['assets','renderer','renderer/sprites','reconstruction/rendering','apps/terrain-preview']:
   for path in (source/folder).glob('*'):
    if path.is_file():inputs[path]=sha(path)
  for name in ['sprite-binary-reference.cpp','palette-lighting-reference.cpp','palette-shading-reference.cpp','palette-shading-native.cpp','palette-lighting-test.cpp']:inputs[source/'tests'/name]=sha(source/'tests'/name)
  controls=out/'controls';palettes=out/'palettes';host=out/'host';include='-I'+str(source/'reconstruction/rendering');model=source/'reconstruction/rendering/palette_shading.cpp'
  for target,test,x86 in [(controls,'palette-lighting-reference.cpp',True),(palettes,'palette-shading-reference.cpp',True),(host,'palette-shading-native.cpp',False)]:
   run(['g++',*(['-m32','-fno-pie','-no-pie'] if x86 else []),'-std=c++17','-O2',include,source/'tests'/test,model,'-o',target],'compile-'+target.name)
  rng=random.Random(0x4e4f75);cases=[['missing']*4,['50','50','2.2','2.2']]
  for value in [-2147483648,-1,0,1,2,50,127,999,1000,1001,2147483647]:
   for field in range(2):c=['missing']*4;c[field]=str(value);cases.append(c)
  for value in [-1e6,-1,-0.,0,0.5,2.2,999.9,1000,1000.1,1e6]:
   for field in range(2,4):c=['missing']*4;c[field]=str(value);cases.append(c)
  for _ in range(1024):cases.append([str(rng.randrange(-2000,2001)) if rng.randrange(4) else 'missing' for _ in range(2)]+[str(rng.randrange(-10000,20001)/10) if rng.randrange(4) else 'missing' for _ in range(2)])
  for i,values in enumerate(cases):run([controls,pe,*values],f'controls-{i}')
  print('matched',len(cases),'configuration admissions',flush=True)
  empty=out/'empty.queue';empty.write_bytes(b'');profiles=[(1,1,2,2),(50,50,2.2,2.2),(1000,1000,0,0),(35,70,1.5,2.5)]
  sprite=root/'Realms/Celtic/Forest/Terrain.spr';inputs[sprite]=sha(sprite);matrix=[]
  for profile_index,profile in enumerate(profiles):
   for count in [2,4,8,16,32,64,128,256]:
    prefix=out/f'palette-{profile_index}-{count}.565';run([palettes,pe,sprite,count,empty,prefix,*profile],f'palette-{profile_index}-{count}')
    native=out/f'host-{profile_index}-{count}';run([host,sprite,count,native,*profile],f'host-{profile_index}-{count}');assert native.read_bytes()==Path(str(prefix)+'.palettes').read_bytes();matrix.append(dict(profile=profile,count=count))
  env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0');frames=[]
  fixtures=[]
  for realm,id in [('Celtic',0),('Greek',0),('Medieval',1)]:
   cfg=root/f'Realms/{realm}/{realm}.cfg';inputs[cfg]=sha(cfg)
   for view in range(4):
    for shade in [-127,-17,-16,-1,0,16,64,127]:fixtures.append((realm,id,view,shade))
  for i,(realm,id,view,shade) in enumerate(fixtures):
   matches=[]
   for build,binary in enumerate(binaries):
    prefix=out/f'frame-{i}-{build}';run([binary,'--root',root,'--region-config',f'Realms/{realm}/{realm}.cfg','--region-id',id,'--generation-seed',0,'--world','--initialize-terrain','--recovered-camera','--view',view,'--palette-shading','--lighting-config','cfg\\encrypted\\CHAOS.CFG','--light',shade,'--output',prefix],f'frame-{i}-{build}',env)
    native=json.loads(prefix.with_suffix('.json').read_text());assert native['map']['palette_lighting']==dict(light_curve=50,colour_factor=50,light_power=2.2,colour_power=2.2,config='cfg\\encrypted\\CHAOS.CFG') and native['remaining_surfaces']==0
    queue=out/f'queue-{i}-{build}';queue.write_bytes(b''.join(struct.pack('<Iiii',d['frame'],d['x'],d['y'],d['shade']) for d in native['queue'] if d['kind']!=-2));sprite=root/(native['map']['recipe']['sprite_path'].replace('\\','/')+'/Terrain.spr');inputs.setdefault(sprite,sha(sprite))
    original=out/f'original-{i}-{build}.565';run([palettes,pe,sprite,16,queue,original,50,50,2.2,2.2],f'original-{i}-{build}');assert original.read_bytes()==prefix.with_suffix('.565').read_bytes()
    rgba=b''.join(bytes((((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31,255)) for (v,) in struct.iter_unpack('<H',original.read_bytes()));assert hashlib.sha256(rgba).hexdigest()==native['rgba_sha256'];matches.append(sha(original))
   assert matches[0]==matches[1];frames.append(dict(realm=realm,view=view,shade=shade,pixels_sha256=matches[0]))
   if i%24==0:print('matched',i+1,'scene fixtures',flush=True)
  refusals=[]
  for build,binary in enumerate(binaries):
   for options in [['--lighting-config','CFG/Encrypted/chaos.cfg'],['--palette-shading','--lighting-config','CFG/Encrypted/absent.cfg']]:
    prefix=out/f'refused-{build}-{len(refusals)}';r=subprocess.run(list(map(str,[binary,'--root',root,*options,'--output',prefix])),capture_output=True,text=True,env=env,timeout=30);assert r.returncode==1 and 'Sanitizer' not in r.stderr and not any(prefix.with_suffix(s).exists() for s in ['.565','.png','.json']);refusals.append(r.stderr.splitlines()[-1])
  assert all(sha(path)==digest for path,digest in inputs.items());report=dict(all_match=True,live_validated=False,executable_sha256=HASH,admission_fixtures=len(cases),profiles=profiles,palette_matrix=matrix,palette_word_comparisons=4112384,native_frames=2*len(frames),frames=frames,refusals=refusals,source_and_input_sha256={str(path.relative_to(ROOT)):digest for path,digest in inputs.items()},helper_sha256={path.name:sha(path) for path in [controls,palettes,host]},boundary='Selected profile read/admission/setter window; private callback and checked return boundary; mode 0; count 16 scenes; controlled uniform light; strict native numeric syntax; no spatial lighting, terrain preference dispatch or live replacement')
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(dict(all_match=True,native_frames=report['native_frames']),flush=True)
 finally:verify('after')
if __name__=='__main__':main()
