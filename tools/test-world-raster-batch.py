#!/usr/bin/env python3
"""Freeze guarded queue captures and compare every native intermediate to actual original entries."""
import argparse,hashlib,importlib.util,json,os,shutil,struct,subprocess,tempfile,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ORIGINAL_SHA='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(name,path):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def source_paths():return module('handoff_sources',ROOT/'tools/test-world-producer-handoff.py').source_paths()
def fnv(b):
 h=2166136261
 for c in b:h=((h^c)*16777619)&0xffffffff
 return h
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--capture-report',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);a=p.parse_args()
 sources={n:sha(ROOT/n) for n in source_paths()};declaration=json.loads(a.claims.read_text())
 for n,h in declaration['sources'].items():
  if sha(ROOT/n)!=h:raise ValueError('Prospective source changed: '+n)
  sources[n]=h
 parent=ROOT/'working/tests/world-raster-batch';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 report={'success':False,'sources':sources,'claims':declaration['claims'],'original_pixels_used_as_native_inputs':False,'scope':'Frozen source-only native CPU/GPU replay and every precise actual original raster/low16AX in guarded complete startup queues. Native pixels stream to the independent original comparator through a bounded FIFO; native reads no original oracle. Live one-canvas writeback and guarded traversal remain separate captured evidence.'}
 inputs={};children=[]
 def pin(path):inputs[path.resolve().relative_to(ROOT).as_posix()]=sha(path)
 def run(name,cmd,timeout=300):
  with (out/(name+'.log')).open('x') as log:subprocess.run([str(c) for c in cmd],cwd=ROOT,env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'},stdout=log,stderr=subprocess.STDOUT,check=True,timeout=timeout)
 run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
 try:
  cap_path=a.capture_report.resolve();pin(cap_path);capture=json.loads(cap_path.read_text());assert capture['success'] and capture['sources_stable'] and capture['world_raster_batch'] and capture['canvas_guard_verified'] and capture['original_work_bypassed']
  assert capture['claims']==declaration['claims'];assert all(sha(ROOT/n)==h for n,h in capture['sources'].items())
  experiment=Path(capture['experiment']);directory=experiment/'capture';inspector=module('batch_inspector',ROOT/'tools/inspect-canvas-producers.py');analysis=inspector.analyze(directory);assert analysis==capture['canvas_producers']
  queues=capture['complete_raster_queues'];assert queues==list(range(1,capture['samples']+1)), 'Use a complete startup prefix for exact batch validation'
  stream=directory/'canvas-producers.bin';pin(stream);pin(directory/'canvas-producers.done');_,ops=inspector.decode(stream.read_bytes());by_sequence={r[1]:(r,b) for r,b in ops}
  active=0;expected={q:[] for q in queues};returns={}
  for r,b in ops:
   if r[2]==11:active=r[14]
   elif r[2]==12:returns[active]=r;active=0
   elif active and r[2]==9:expected[active].append(r[1])
  actual=[];count=0
  for ordinal,batch in enumerate(capture['native_producers']['native_batch_replies'],1):
   q=batch['queue'];request=directory/(batch['wire_base']+'.request');reply=directory/(batch['wire_base']+'.reply');pin(request);pin(reply);raw=request.read_bytes();h=struct.unpack_from('<16I',raw);assert raw[:8]==b'MNMWBQ03' and h[2:6]==(3,64,ordinal,q) and h[8:11]==(800,600,800) and h[13]==1 and h[11]==h[14]*16 and len(raw)==64+h[11] and fnv(raw[64:])==h[12]
   rows=list(struct.iter_unpack('<4I',raw[64:]));assert [d[0] for d in rows]==expected[q]==batch['sequences'];count+=len(rows);assert h[15]==count and batch['cumulative_rasters']==count and h[14]==batch['rasters']
   assert returns[q][15]==count and returns[q][17]==ordinal and returns[q][19]==len(rows) and returns[q][20]==1
   for seq,entry,ax,checksum in rows:
    r,payload=by_sequence[seq];assert ax==r[21] and checksum==fnv(struct.pack('<24I',*r)+payload);actual.append((seq,entry,ax))
   data=reply.read_bytes();rh=struct.unpack_from('<16I',data);assert data[:8]==b'MNMWBR03' and rh[2:11]==h[2:11] and rh[14:]==h[14:] and rh[11]==960000 and len(data)==960064 and fnv(data[64:])==rh[12] and rh[13]==1
  assert count==capture['bypassed_rasters'] and len(actual)==count and len(queues)==capture['native_canvas_writebacks']
  assert [d[0] for d in actual]==[r['sequence'] for r in capture['native_producers']['native_bypass_replies']]
  assert not list(directory.glob('world-raster-*.request')) and not list(directory.glob('world-batch-fault-*.bin'))
  frozen=out/'source';names=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
  for n in sorted(set(names)|set(sources)):
   path=ROOT/n
   if not n or n.split('/')[0] in ('working','original','.git') or not path.is_file():continue
   if n not in sources and path.suffix not in ('.cpp','.hpp','.c','.h','.S','.py','.cmake') and path.name not in ('CMakeLists.txt','README.md'):continue
   dst=frozen/n;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,dst)
  assert all(sha(frozen/n)==h for n,h in sources.items())
  native_inputs=out/'native-inputs';native_inputs.mkdir();shutil.copyfile(stream,native_inputs/stream.name);shutil.copyfile(directory/'canvas-producers.done',native_inputs/'canvas-producers.done')
  assets=out/'native-assets';assets.mkdir();asset_sources=[];encoded={b[:-1].decode('ascii').replace('\\','/').casefold() for r,b in ops if r[2] in (7,20)}
  for path in (experiment/'game').rglob('*'):
   if not path.is_file():continue
   n=path.relative_to(experiment/'game')
   if path.suffix.lower()!='.spr' and n.as_posix().casefold() not in encoded:continue
   pin(path);dst=assets/n;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,dst);asset_sources.append({'path':n.as_posix(),'sha256':sha(path)})
  build=out/'build';run('configure',['cmake','-S',frozen/'compat/legacy/canvas-producers','-B',build,'-DCMAKE_BUILD_TYPE=Debug']);run('build',['cmake','--build',build,'--target','mnm-canvas-producers-live','world-raster-batch-test','canvas-world-test','canvas-sequence-test','scene-renderer-test','scene-history-test','-j4'])
  run('native-tests',['ctest','--test-dir',build,'-R','native-world-raster-batch|native-canvas-world-handoff|native-canvas-sequence|native-shared-scene-renderer|native-scene-history','--output-on-failure'])
  compiled=set()
  for dep in build.rglob('*.o.d'):
   for token in dep.read_text().replace('\\\n',' ').split():
    path=Path(token).resolve()
    if path.is_absolute() and path.is_relative_to(frozen):compiled.add(path.relative_to(frozen).as_posix())
  assert compiled and not compiled-set(sources),sorted(compiled-set(sources))
  pe=ROOT/'original/Arcane_Nocd/Chaos.exe';pin(pe);assert sha(pe)==ORIGINAL_SHA
  reference=out/'original-reference';run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',frozen/'tests/world-raster-batch-reference.cpp','-o',reference])
  entry_list=out/'actual-entries.tsv';entry_list.write_text(''.join(f'{seq} {entry} {ax}\n' for seq,entry,ax in actual));pin(entry_list)
  fifo=out/'native-proof.fifo';os.mkfifo(fifo);result=out/'precise-results.tsv'
  native=out/'native';env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'};started=time.monotonic()
  with (out/'original-comparison.log').open('x') as orig_log,(out/'native-proof.log').open('x') as native_log:
   original=subprocess.Popen([str(c) for c in [reference,pe,native_inputs/stream.name,entry_list,fifo,result]],cwd=ROOT,stdout=orig_log,stderr=subprocess.STDOUT);children.append(original)
   viewer=subprocess.Popen([str(c) for c in ['xvfb-run','-a','-s','-screen 0 1280x1024x24',build/'mnm-canvas-producers-live',native_inputs/stream.name,assets,native,'--world-handoff','--world-proof',fifo]],cwd=ROOT,env=env,stdout=native_log,stderr=subprocess.STDOUT,start_new_session=True);children.append(viewer)
   while viewer.poll() is None or original.poll() is None:
    if original.poll() not in (None,0):raise RuntimeError('Original comparator refused; inspect original-comparison.log')
    if viewer.poll() not in (None,0):raise RuntimeError('Native source-only proof refused; inspect native-proof.log')
    if time.monotonic()-started>3600:raise TimeoutError('Bounded original/native FIFO comparison timed out')
    time.sleep(.2)
   assert original.returncode==viewer.returncode==0
  v=json.loads((native/'live-report.json').read_text());assert v['success'] and v['world_queues']==len(queues) and v['world_readbacks']==len(queues) and not v['remaining_surfaces'] and not v['original_oracles_read'] and not v['original_pixels_used_as_native_inputs']
  checks=[]
  for old,new in zip(analysis['checkpoints'],v['checkpoints'],strict=True):
   assert (old['sequence'],old['canvas'],old['oracle'])==(new['sequence'],new['canvas'],new['oracle']);path=directory/old['path'];pin(path);checks.append({'sequence':old['sequence'],'pixels':old['width']*old['height'],'equal':path.read_bytes()==(native/new['path']).read_bytes()})
  finals=[];checkpoints={c['sequence']:c for c in v['checkpoints']}
  for batch in capture['native_producers']['native_batch_replies']:
   q=batch['queue'];pixels=(native/checkpoints[returns[q][1]-1]['path']).read_bytes();live=(directory/(batch['wire_base']+'.reply')).read_bytes()[64:];finals.append({'queue':q,'pixels':len(pixels)//2,'equal':pixels==live})
  rows=[line.split('\t') for line in result.read_text().splitlines()];assert len(rows)==count and all(len(r)==5 and r[-1]=='1' for r in rows)
  report.update(success=all(c['equal'] for c in checks+finals),source_executable_sha256=ORIGINAL_SHA,complete_raster_queues=queues,precise_rasters_equal=count,caller_ax_equal=count,precise_pixels_compared=sum(int(r[3]) for r in rows),live_final_equal=sum(f['equal'] for f in finals),final_checks=finals,checkpoints=checks,completed=len(checks),checkpoint_pixels_compared=sum(c['pixels'] for c in checks),canvas_guard_verified=True,native_canvas_writebacks=capture['native_canvas_writebacks'],native_world_readbacks=v['world_readbacks'],native_original_oracles_read=False,native_binary_sha256=sha(build/'mnm-canvas-producers-live'),compiled_dependency_review={'declared_count':len(sources),'compiler_dependency_count':len(compiled),'compiled_dependencies':sorted(compiled),'undeclared_dependencies':[],'frozen_sources_stable':all(sha(frozen/n)==h for n,h in sources.items())},asset_sources=asset_sources,comparison_elapsed_seconds=time.monotonic()-started,actual_entry_counts={hex(entry):sum(r[1]==entry for r in actual) for entry in sorted({r[1] for r in actual})},return_ax_counts={str(ax):sum(r[2]==ax for r in actual) for ax in (0,1)},precise_results_sha256=sha(result),native_tests_passed=5)
 except Exception as error:report['error']=str(error)
 finally:
  for child in children:
   if child.poll() is None:child.terminate()
  for child in children:
   if child.poll() is None:
    try:child.wait(timeout=5)
    except subprocess.TimeoutExpired:child.kill();child.wait(timeout=5)
  run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report.update(original_manifest_verified_before_after=True,inputs=inputs,inputs_stable=all(sha(ROOT/n)==h for n,h in inputs.items()),sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()));report['success']=report['success'] and report['inputs_stable'] and report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
 if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
