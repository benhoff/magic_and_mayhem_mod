#!/usr/bin/env python3
"""Original byte-grid/kernel/source comparisons and per-cell lit terrain scenes."""
import argparse,hashlib,json,os,struct,subprocess,tempfile,sys,random,importlib.util
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def module(name,path):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def main():
 if '--inner' not in sys.argv:
  subprocess.run(['xvfb-run','-a',sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=1200);return
 sys.argv.remove('--inner');p=argparse.ArgumentParser();p.add_argument('--preview',type=Path,required=True);p.add_argument('--sanitized-preview',type=Path,required=True);p.add_argument('--source-root',type=Path,default=ROOT);a=p.parse_args();source=a.source_root.resolve()
 parent=ROOT/'working/tests/terrain-light-fields';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 def run(cmd,name,env=None):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120,env=env);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r.stdout
 def verify(phase):run([ROOT/'tools/original-manifest.sh','verify'],'original-'+phase)
 verify('before')
 try:
  pe=ROOT/'working/game-nocd/Chaos.exe';root=ROOT/'working/game-clean';binaries=[a.preview.resolve(),a.sanitized_preview.resolve()]
  if sha(pe)!=HASH:raise ValueError('Executable hash mismatch')
  inputs={path:sha(path) for path in [pe,*binaries,Path(__file__),root/'CFG/Encrypted/chaos.cfg',root/'CFG/prefs.cfg']}
  for folder in ['assets','renderer','renderer/sprites','reconstruction/rendering','apps/terrain-preview','tests']:
   for path in (source/folder).glob('*'):
    if path.is_file():inputs[path]=sha(path)
  for file in ['tools/decode-cfg.py','tools/test-terrain-map.py']:inputs[source/file]=sha(source/file)
  field=out/'field';host=out/'host';palette=out/'palette';world=out/'world';include='-I'+str(source/'reconstruction/rendering')
  for target,test,model,x86 in [(field,'terrain-lighting-reference.cpp','terrain_lighting.cpp',True),(host,'terrain-lighting-native.cpp','terrain_lighting.cpp',False),(palette,'palette-shading-reference.cpp','palette_shading.cpp',True),(world,'world-terrain-reference.cpp',None,True)]:
   run(['g++',*(['-m32','-fno-pie','-no-pie'] if x86 else []),'-std=c++17','-O2',include,source/'tests'/test,*([source/'reconstruction/rendering'/model] if model else []),'-o',target],'compile-'+target.name)
  rng=random.Random(0x4f2140);cases=[(18,19,5,'missing','missing','none')]
  for ramp in range(-127,128):cases.append((18,19,5,'50',str(ramp),'0,0,0,17;17,18,4,17;9,9,3,2'))
  for ambient in [-2147483648,-128,-127,-50,0,50,127,128,2147483647]:
   for ramp in ['missing','-128','7','128']:cases.append((18,19,5,str(ambient),ramp,'0,0,4,17;9,9,1,5'))
  for size in range(2,18):cases.append((size,size,3,'50','7',f'0,0,0,{size};{size-1},{size-1},2,{size}'))
  for _ in range(256):
   w,h,l=rng.randrange(17,33),rng.randrange(17,33),rng.randrange(1,10);sources=';'.join(f'{rng.randrange(w)},{rng.randrange(h)},{rng.randrange(l)},{rng.randrange(2,18)}' for _ in range(rng.randrange(1,6)));cases.append((w,h,l,str(rng.randrange(-200,201)),str(rng.randrange(-200,201)),sources))
  kernel_bytes=field_bytes=stamps=0
  for i,case in enumerate(cases):
   prefix=out/f'field-{i}';actual=json.loads(run([field,pe,*case,prefix],f'field-{i}'));native=out/f'host-{i}';run([host,*case,native],f'host-{i}')
   for suffix in ['.kernels','.buffers']:assert prefix.with_suffix(suffix).read_bytes()==native.with_suffix(suffix).read_bytes(),(i,suffix)
   kernel_bytes+=actual['kernel_bytes'];field_bytes+=actual['field_bytes']*5;stamps+=actual['stamps']
  print('matched',len(cases),'field fixtures',flush=True)
  codec=module('codec',source/'tools/decode-cfg.py');geometry=module('geometry',source/'tools/test-terrain-map.py')
  env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',ASAN_OPTIONS='detect_leaks=0');frames=[]
  # Installed MAPs give an independently reconstructed geometry input to the
  # actual original world producer, with the original field supplied directly.
  for realm in ['Celtic','Greek','Medieval']:
   directory=next(d for d in sorted((root/'Realms'/realm).iterdir()) if d.is_dir() and (d/'Terrain.ttd').exists() and list(d.glob('*.map')))
   path=sorted(directory.glob('*.map'))[0];ttd=directory/'Terrain.ttd';spr=directory/'Terrain.spr'
   for f in [path,ttd,spr]:inputs[f]=sha(f)
   raw=codec.decode(path.read_bytes())[1];w,h,l=struct.unpack_from('<3I',raw,4);final=geometry.geometry(raw,ttd.read_bytes())[1];payload=out/f'{realm}.map';payload.write_bytes(final)
   size=min(17,w,h);source_sets=['none',f'{w//2},{h//2},0,{size}',f'0,0,0,{size};{w-1},{h-1},{l-1},{size}']
   for sources in source_sets:
    field_prefix=out/f'scene-field-{realm}-{len(frames)}';run([field,pe,w,h,l,50,7,sources,field_prefix],f'scene-field-{realm}-{len(frames)}');field_raw=field_prefix.with_suffix('.field').read_bytes()
    for view in range(4):
     matches=[]
     for build,binary in enumerate(binaries):
      i=len(frames);prefix=out/f'frame-{i}-{build}';opts=[] if sources=='none' else [v for s in sources.split(';') for v in ['--light-source',s]]
      run([binary,'--root',root,'--realm',str(directory.relative_to(root)),'--map',str(path.relative_to(root)),'--world','--initialize-terrain','--recovered-camera','--view',view,'--palette-shading','--preferences','cfg\\PREFS.CFG','--lighting-config','CFG/Encrypted/chaos.cfg','--terrain-lighting',*opts,'--output',prefix],f'frame-{i}-{build}',env)
      native=json.loads(prefix.with_suffix('.json').read_text());info=native['map'];assert info['terrain_light_field']['sha256']==hashlib.sha256(field_raw).hexdigest() and info['terrain_light_field']['ambient']==-50 and info['terrain_light_field']['ramp']==7 and 'controlled_light' not in info and native['remaining_surfaces']==0
      assert info['cells_sha256']==hashlib.sha256(final[76:]).hexdigest()
      for tile in native['tiles']:assert tile['light']==struct.unpack_from('<b',field_raw,(tile['level']//2*h+tile['row'])*w+tile['column'])[0]
      camera=info['camera'];expected=json.loads(run([world,pe,ttd,spr,payload,*camera,*info['pan'],0,view,*info['origin_fields'],field_prefix.with_suffix('.field')],f'world-{i}-{build}'))
      queue=[]
      for d in native['queue']:q={k:v for k,v in d.items() if k not in ['tile','role']};q['cell']=native['tiles'][d['tile']]['cell'];queue.append(q)
      assert queue==expected['queue'],(realm,sources,view,build)
      qfile=out/f'queue-{i}-{build}';qfile.write_bytes(b''.join(struct.pack('<Iiii',d['frame'],d['x'],d['y'],d['shade']) for d in expected['queue'] if d['kind']!=-2));original=out/f'original-{i}-{build}.565';run([palette,pe,spr,256,qfile,original,50,50,2.2,2.2],f'original-{i}-{build}');assert original.read_bytes()==prefix.with_suffix('.565').read_bytes();matches.append(sha(original))
     assert matches[0]==matches[1];frames.append(dict(realm=realm,sources=sources,view=view,field_sha256=hashlib.sha256(field_raw).hexdigest(),pixels_sha256=matches[0]))
   print('matched',realm,'lit scenes',flush=True)
  refusals=[]
  baseline=['--map',str(path.relative_to(root)),'--realm',str(directory.relative_to(root)),'--world','--initialize-terrain','--palette-shading','--lighting-config','CFG/Encrypted/chaos.cfg']
  for build,binary in enumerate(binaries):
   for options in [['--light-source','0,0,0,2'],['--terrain-lighting'],[*baseline,'--terrain-lighting','--light','0'],*([*baseline,'--terrain-lighting','--light-source',s] for s in ['0,0,0,1','0,0,0,18','999,0,0,2','0,0,999,2','bad','0,0,0'])]:
    prefix=out/f'refused-{build}-{len(refusals)}';r=subprocess.run(list(map(str,[binary,'--root',root,*options,'--output',prefix])),capture_output=True,text=True,env=env,timeout=30);assert r.returncode==1 and 'Sanitizer' not in r.stderr and not any(prefix.with_suffix(s).exists() for s in ['.565','.png','.json']);refusals.append(r.stderr.splitlines()[-1])
  baseline_pixels=[]
  for build,binary in enumerate(binaries):
   prefix=out/f'unshaded-{build}';run([binary,'--root',root,'--output',prefix],f'unshaded-{build}',env);native=json.loads(prefix.with_suffix('.json').read_text());assert not native['map']['palette_shading'] and 'terrain_light_field' not in native['map'] and all('light' not in tile for tile in native['tiles']) and native['remaining_surfaces']==0;baseline_pixels.append(sha(prefix.with_suffix('.565')))
  assert baseline_pixels[0]==baseline_pixels[1]
  assert all(sha(path)==digest for path,digest in inputs.items());report=dict(all_match=True,live_validated=False,executable_sha256=HASH,field_fixtures=len(cases),kernel_byte_comparisons=kernel_bytes,final_buffer_byte_comparisons=field_bytes,original_source_stamps=stamps,unshaded_compatibility_frames=2,native_frames=2*len(frames),frames=frames,refusals=refusals,source_and_input_sha256={str(path.relative_to(ROOT)):digest for path,digest in inputs.items()},helper_sha256={path.name:sha(path) for path in [field,host,palette,world]},boundary='Preallocated five-buffer initialization, original CFG admission, all size-2..17 distance kernels, selected source stamping and immediate copy publication, original world queues and indexed scene draws; controlled explicit sources; no entity source discovery, occlusion, smoothing/ticks or live replacement')
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(dict(all_match=True,native_frames=report['native_frames']),flush=True)
 finally:verify('after')
if __name__=='__main__':main()
