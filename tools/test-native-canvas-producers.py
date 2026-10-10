#!/usr/bin/env python3
"""Replay closed menu/loading/HUD inputs and compare every completed native canvas."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def source_paths():
 names=['renderer/dib.cpp','renderer/dib.hpp','renderer/canvas_sequence.cpp','renderer/canvas_sequence.hpp','renderer/blit.hpp','compat/legacy/canvas_producers.cpp','compat/legacy/canvas_producers.hpp','compat/legacy/canvas_producers_main.cpp','compat/legacy/canvas-producers/CMakeLists.txt','protocols/include/mnm/canvas_producers_v1.h','tests/canvas-sequence-test.cpp','tests/canvas-panel-reference.cpp','tests/canvas-panel-native.cpp','tests/sprite-binary-reference.cpp','tools/test-native-canvas-producers.py','tools/inspect-canvas-producers.py','tools/inventory-binary.py','tools/audit-file-apis.py','assets/CMakeLists.txt','assets/sprite_loader.cpp','assets/sprite_loader.hpp','assets/sprite_frame_decoder.hpp','assets/jpeg.cpp','assets/jpeg.hpp','assets/pcx.cpp','assets/pcx.hpp','assets/asset_file.cpp','assets/asset_file.hpp','assets/path_resolver.cpp','assets/path_resolver.hpp']
 names += ['renderer/minimap/minimap.cpp','renderer/minimap/minimap.hpp','renderer/minimap/overlays.cpp','renderer/minimap/overlays.hpp','protocols/include/mnm/canvas_producers_v2.h','tests/minimap-producer-test.cpp']
 return {n:sha(ROOT/n) for n in names}
def resolve(root,name):
 parts=name.replace('\\','/').split('/');p=root
 if not parts or any(x in ('','..','.') or ':' in x for x in parts):raise ValueError('Unsafe captured source filename')
 for x in parts:
  choices=[c for c in p.iterdir() if c.name.casefold()==x.casefold()]
  if len(choices)!=1:raise ValueError('Ambiguous/unavailable captured source filename: '+name)
  p=choices[0]
 if not p.resolve().is_relative_to(root.resolve()) or not p.is_file():raise ValueError('Source escaped installation')
 return p
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--capture-report',type=Path,required=True);parser.add_argument('--claims',type=Path);a=parser.parse_args();a.capture_report=a.capture_report.resolve()
 sources=source_paths();claims=json.loads(a.claims.read_text()) if a.claims else None
 if claims:
  for n,h in claims['sources'].items():
   if sha(ROOT/n)!=h:raise ValueError('Prospective source changed: '+n)
   sources[n]=h
 parent=ROOT/'working/tests/native-canvas-producers';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 report={'success':False,'sources':sources,'scope':'Closed selected menu/loading/font/HUD and raster producers, contiguous startup prefix, each available completed native canvas compared to a separate original oracle. Unknown/unlocked destination writers and presentation-driver behavior remain outside the admitted contract.','original_pixels_used_as_native_inputs':False,'live_replacement':False,'original_work_bypassed':False}
 if claims:report['claims']=claims['claims']
 inputs={};sourceRoot=out/'source'
 names=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
 snapshot={}
 for n in sorted(set(names)|set(sources)):
  p=ROOT/n
  if not n or n.split('/')[0] in ('working','original','.git') or not p.is_file():continue
  if n not in sources and p.suffix not in ('.cpp','.hpp','.c','.h','.S','.py','.cmake') and p.name not in ('CMakeLists.txt','README.md'):continue
  b=p.read_bytes();t=sourceRoot/n;t.parent.mkdir(parents=True,exist_ok=True);t.write_bytes(b);snapshot[n]=hashlib.sha256(b).hexdigest()
 for n,h in sources.items():
  if snapshot[n]!=h:raise RuntimeError('Source changed during freeze: '+n)
 (out/'source-manifest.json').write_text(json.dumps(snapshot,indent=2)+'\n')
 def run(name,args,check=True,timeout=180):
  with (out/(name+'.log')).open('x') as log:return subprocess.run([str(x) for x in args],cwd=ROOT,env={**os.environ,'QT_QPA_PLATFORM':'offscreen'},stdout=log,stderr=subprocess.STDOUT,check=check,timeout=timeout)
 def pin(p):inputs[str(p.relative_to(ROOT))]=sha(p)
 run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
 try:
  pin(a.capture_report);capture=json.loads(a.capture_report.read_text());report['source_executable_sha256']=capture['source_executable_sha256'];experiment=Path(capture['experiment']);directory=experiment/'capture';inspector=load('producer_inspector','tools/inspect-canvas-producers.py');analysis=inspector.analyze(directory)
  if analysis!=capture['canvas_producers']:raise ValueError('Capture oracle/source identities changed')
  stream=directory/'canvas-producers.bin';pin(stream);pin(directory/'canvas-producers.done');_,records=inspector.decode(stream.read_bytes())
  nativeInputs=out/'native-inputs';nativeInputs.mkdir();shutil.copyfile(stream,nativeInputs/stream.name);bindings={};encoded={}
  for r,b in records:
   if r[2] not in (7,20):continue
   name=b[:-1].decode('ascii')
   if name in bindings:continue
   asset=resolve(experiment/'game',name);pin(asset);filename=f'source-{len(bindings)+1:04}.asset';shutil.copyfile(asset,nativeInputs/filename);bindings[name]={'file':filename,'sha256':sha(asset)};encoded[name]={'path':str(asset.relative_to(ROOT)),'sha256':sha(asset)}
  binding=nativeInputs/'sources.json';binding.write_text(json.dumps(bindings,indent=2)+'\n');pin(binding)
  build=out/'build';run('configure',['cmake','-S',sourceRoot/'compat/legacy/canvas-producers','-B',build,'-DCMAKE_BUILD_TYPE=Debug']);run('build',['cmake','--build',build,'--target','mnm-canvas-producers-preview','canvas-sequence-test','minimap-producer-test','-j4']);run('native-tests',['ctest','--test-dir',build,'-R','^native-(canvas-sequence|minimap-producer)$','--output-on-failure'])
  reference=out/'panel-reference';fixture=out/'panel-native';run('panel-reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',sourceRoot/'tests/canvas-panel-reference.cpp','-o',reference])
  import shlex
  qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui']).decode());run('panel-native-build',['c++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fPIC',sourceRoot/'tests/canvas-panel-native.cpp',sourceRoot/'renderer/canvas_sequence.cpp',*qt,'-o',fixture])
  pe=ROOT/'original/Arcane_Nocd/Chaos.exe';pin(pe)
  if sha(pe)!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':raise ValueError('Panel reference executable changed')
  panels=[]
  for mode in (15,16,17,21,22,26):
   old=out/f'original-panel-{mode}.565';new=out/f'native-panel-{mode}.565';run(f'original-panel-{mode}',[reference,pe,mode,old]);run(f'native-panel-{mode}',[fixture,mode,new]);panels.append({'mode':mode,'pixels':256,'equal':old.read_bytes()==new.read_bytes(),'original_sha256':sha(old),'native_sha256':sha(new)})
  copies=[]
  for mode,address in ((23,0x58c6a0),(24,0x58cd50),(25,0x58cf20)):
   dest=out/f'original-copy-{mode}.565';run(f'original-copy-{mode}',[reference,pe,mode,dest]);copies.append({'entry':hex(address),'returned':True,'policy':'Private COM success adapter; original wrapper executes, no pixel-copy equivalence claim'})
  report['copy_entry_execution']=copies
  from importlib.machinery import SourceFileLoader
  inventory=SourceFileLoader('producer_pe_inventory',str(ROOT/'tools/inventory-binary.py')).load_module();_,sections=inventory.pe_sections(pe.read_bytes());anchors={}
  for address in (0x58c6a0,0x58cd50,0x58cf20,0x58ed80,0x553a40):
   section=next(s for s in sections if s['start']<=address<s['end']);offset=section['file_offset']+address-section['start'];anchors[hex(address)]=pe.read_bytes()[offset:offset+16].hex()
  report['original_entry_anchors']=anchors
  report['panel_fixtures']=panels
  if not all(c['equal'] for c in panels):raise ValueError('Independent original panel corner fixture differs')
  completed=out/'native';execution=run('native-replay',[build/'mnm-canvas-producers-preview',nativeInputs/stream.name,binding,completed],check=False);native=json.loads((completed/'native-report.json').read_text());nativeByID={c['oracle']:c for c in native['checkpoints']};compared=[]
  for original in analysis['checkpoints']:
   oracle=directory/original['path'];pin(oracle);checkpoint={**original,'native_completed':False};n=nativeByID.get(original['oracle'])
   if n and n.get('completed'):
    path=completed/n['path'];actual=path.read_bytes();expected=oracle.read_bytes()
    if len(actual)!=len(expected):raise ValueError('Native completed extent changed')
    differences=[] if actual==expected else [i for i,(x,y) in enumerate(zip(struct.iter_unpack('<H',actual),struct.iter_unpack('<H',expected))) if x!=y];w=original['width'];checkpoint.update(native_completed=True,mismatches=len(differences),native_sha256=sha(path),bounds=[min(i%w for i in differences),min(i//w for i in differences),max(i%w for i in differences),max(i//w for i in differences)] if differences else None,examples=[{'x':i%w,'y':i//w,'native':struct.unpack_from('<H',actual,i*2)[0],'original':struct.unpack_from('<H',expected,i*2)[0]} for i in differences[:8]])
   elif n:checkpoint['native_error']=n.get('error')
   compared.append(checkpoint)
  report.update(capture_report_sha256=sha(a.capture_report),capture=analysis,encoded_sources=encoded,native=native,checkpoints=compared,completed=sum(c['native_completed'] for c in compared),equal=sum(c['native_completed'] and c['mismatches']==0 for c in compared),pixels_compared=sum(c['width']*c['height'] for c in compared if c['native_completed']),mismatches=sum(c.get('mismatches',0) for c in compared),native_binary_sha256=sha(build/'mnm-canvas-producers-preview'))
  report['success']=execution.returncode==0 and not analysis['failures'] and report['equal']==len(compared)
 except Exception as e:report['error']=str(e)
 finally:
  run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['inputs']=inputs;report['inputs_stable']=all(sha(ROOT/n)==h for n,h in inputs.items());report['current_sources_changed']=[n for n,h in sources.items() if sha(ROOT/n)!=h];report['sources_stable']=not report['current_sources_changed'];report['success']=report['success'] and report['inputs_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
 if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
