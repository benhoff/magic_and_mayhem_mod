#!/usr/bin/env python3
"""Compare frozen prechange and current consumers using only the same owned inputs."""
import argparse, hashlib, importlib.util, json, os, shutil, statistics, subprocess, tempfile
from pathlib import Path
from coverage_claims import behavior_contract, scenario_contract, required_sources
ROOT=Path(__file__).resolve().parents[1]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--baseline',type=Path,required=True)
 p.add_argument('--capture-report',type=Path,required=True)
 p.add_argument('--behavior',action='append',help='Behavior IDs to declare; defaults to NR.world-resource-reuse')
 p.add_argument('--scenario',default='world-resource-reuse-replay-20261009')
 p.add_argument('--queues',type=int,choices=(16,32),default=16)
 p.add_argument('--optimized',action='store_true',help='Compare optimized consumers; assertion-enabled fixtures run separately')
 p.add_argument('--budget-ms',type=float,help='Require warm median native work (CPU reference included) within this host-specific budget')
 p.add_argument('--prepared',type=Path,help='Reuse a build only after every compiler dependency matches current declared bytes')
 p.add_argument('--prepared-inputs',type=Path,help='Reuse closed owned inputs only after checking every file against its previous input manifest')
 p.add_argument('--compare-original',action='store_true',help='Also compare current CPU intermediates/AX to precise original entries using historical closed capture inputs')
 a=p.parse_args()
 if a.optimized and a.prepared:p.error('Optimized comparisons require a fresh declared build')
 if a.budget_ms is not None and (a.budget_ms<=0 or not a.optimized):p.error('A positive performance budget requires --optimized')
 baseline=a.baseline.resolve();before=json.loads((baseline/'baseline.json').read_text())
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 ids=a.behavior or ['NR.world-resource-reuse']
 selected=[next(b for b in register['behaviors'] if b['id']==id) for id in ids]
 scenario=next(s for s in register['scenarios'] if s['id']==a.scenario)
 sources={n:sha(ROOT/n) for n in sorted(set().union(*(required_sources(b) for b in selected)))}
 claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{b['id']:b for b in register['builds']}),scenarios={scenario['id']:scenario_contract(scenario)} if b['id'] in scenario['behaviors'] else {}) for b in selected]
 parent=ROOT/'working/tests/world-resource-reuse';parent.mkdir(parents=True,exist_ok=True)
 out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 (out/'claims.json').write_text(json.dumps(dict(sources=sources,claims=claims),indent=2)+'\n')
 report=dict(success=False,sources=sources,claims=claims,original_pixels_used_as_native_inputs=False,scope=scenario['scope'])
 env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'}
 def run(name,command):
  with (out/(name+'.log')).open('x') as log:subprocess.run([str(c) for c in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=600)
 run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
 try:
  assert all(sha(baseline/'source'/n)==h for n,h in before['sources'].items())
  if 'binary_sha256' in before:assert sha(baseline/'build/mnm-canvas-producers-live')==before['binary_sha256']
  capture=json.loads(a.capture_report.read_text());assert capture['success']
  experiment=Path(capture['experiment']);directory=experiment/'capture'
  stream=directory/'canvas-producers.bin'
  inputs=out/'inputs';inputs.mkdir()
  for name in ('canvas-producers.bin','canvas-producers.done'):shutil.copyfile(directory/name,inputs/name)
  input_hashes={str(p.resolve()):sha(p) for p in [stream,directory/'canvas-producers.done',inputs/stream.name,inputs/'canvas-producers.done']}
  input_hashes[str((baseline/'baseline.json').resolve())]=sha(baseline/'baseline.json')
  assets=out/'assets';assets.mkdir();asset_hashes={}
  if a.prepared_inputs:
   previous=a.prepared_inputs.resolve();manifest=previous/'report.json';old=json.loads(manifest.read_text())
   assert old['inputs_stable'] and old['capture_report_sha256']==sha(a.capture_report)
   assert old['owned_stream_sha256']==sha(stream)
   input_hashes[str(manifest)]=sha(manifest)
   for name in ('canvas-producers.bin','canvas-producers.done'):
    assert sha(previous/'inputs'/name)==sha(directory/name)
   paths=[]
   for name,h in old['asset_sources'].items():
    assert not Path(name).is_absolute() and '..' not in Path(name).parts,name
    path=(previous/'assets'/name).resolve()
    assert path.is_relative_to(previous/'assets') and sha(path)==h,name
    paths.append((path,Path(name)))
   report['prepared_input_manifest']=dict(path=str(manifest),sha256=sha(manifest))
  else:
   spec=importlib.util.spec_from_file_location('inspect_producers',ROOT/'tools/inspect-canvas-producers.py');inspector=importlib.util.module_from_spec(spec);spec.loader.exec_module(inspector)
   _,ops=inspector.decode(stream.read_bytes())
   encoded={b[:-1].decode('ascii').replace('\\','/').casefold() for r,b in ops if r[2] in (7,20)}
   paths=[(path,path.relative_to(experiment/'game')) for path in (experiment/'game').rglob('*') if path.is_file() and (path.suffix.lower()=='.spr' or path.relative_to(experiment/'game').as_posix().casefold() in encoded)]
  for path,n in paths:
   dst=assets/n;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,dst);asset_hashes[n.as_posix()]=sha(path);input_hashes[str(path.resolve())]=sha(path);input_hashes[str(dst.resolve())]=sha(dst)
  frozen=out/'source'
  names=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
  for n in sorted(set(names)|set(sources)):
   path=ROOT/n
   if not n or n.split('/')[0] in ('working','original','.git') or not path.is_file():continue
   if n not in sources and path.suffix not in ('.cpp','.hpp','.c','.h','.S','.py','.cmake') and path.name not in ('CMakeLists.txt','README.md'):continue
   dst=frozen/n;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,dst)
  build=(a.prepared.resolve()/'build') if a.prepared else out/'build'
  if not a.prepared:run('configure',['cmake','-S',frozen/'compat/legacy/canvas-producers','-B',build,'-DCMAKE_BUILD_TYPE='+('RelWithDebInfo' if a.optimized else 'Debug')]+(['-DBUILD_TESTING=OFF'] if a.optimized else []))
  if not a.prepared:run('build',['cmake','--build',build,'--target','mnm-canvas-producers-live']+([] if a.optimized else ['world-raster-batch-test','canvas-world-test','canvas-sequence-test','minimap-producer-test','scene-renderer-test','scene-history-test'])+['-j4'])
  if not a.optimized:run('native-tests',['xvfb-run','-a','ctest','--test-dir',build,'-R','native-world-raster-batch|native-canvas-world-handoff|native-canvas-sequence|native-minimap-producer|native-shared-scene-renderer|native-scene-history','--output-on-failure'])
  compiled=set()
  for dep in build.rglob('*.o.d'):
   for token in dep.read_text().replace('\\\n',' ').split():
    path=Path(token).resolve()
    dependency_root=(a.prepared.resolve()/'source') if a.prepared else frozen
    if path.is_relative_to(dependency_root):
     n=path.relative_to(dependency_root).as_posix();compiled.add(n)
     assert n in sources and sha(path)==sources[n],n
  assert compiled and not compiled-set(sources),sorted(compiled-set(sources))
  results=[]
  for binary in [baseline/'build/mnm-canvas-producers-live',build/'mnm-canvas-producers-live']:input_hashes[str(binary.resolve())]=sha(binary)
  for label,binary in [('before',baseline/'build/mnm-canvas-producers-live'),('after',build/'mnm-canvas-producers-live')]:
   destination=out/label
   run(label,['xvfb-run','-a','-s','-screen 0 1280x1024x24',binary,inputs/stream.name,assets,destination,'--world-handoff'])
   result=json.loads((destination/'live-report.json').read_text())
   assert result['success'] and result['world_queues']==a.queues and result['world_readbacks']==a.queues
   assert not result['remaining_surfaces'] and not result['original_oracles_read'] and not result['original_pixels_used_as_native_inputs']
   results.append(result)
  checks=[]
  for old,new in zip(results[0]['checkpoints'],results[1]['checkpoints'],strict=True):
   assert (old['sequence'],old['canvas'],old['oracle'])==(new['sequence'],new['canvas'],new['oracle'])
   oldpath=out/'before'/old['path'];newpath=out/'after'/new['path']
   checks.append(dict(sequence=old['sequence'],pixels=oldpath.stat().st_size//2,equal=oldpath.read_bytes()==newpath.read_bytes()))
  assert all(c['equal'] for c in checks)
  assert all(sha(Path(n))==h for n,h in input_hashes.items())
  frames=results[1]['world_frames'];totals={k:sum(f['reuse'][k] for f in frames) for k in ('visual_checks','visual_reuses','cache_uploads','cache_hits','cache_evictions')}
  identity_totals={}
  if 'NR.world-frame-identity-cache' in ids:
   identity_totals={k:sum(f['identities'][k] for f in frames) for k in ('hashes','hits','evictions','bypasses')}
   assert identity_totals['hits']>identity_totals['hashes']
   assert all(f['identities']['frames']<=4096 and f['identities']['bytes']<=16*1024*1024 for f in frames)
  assert totals['visual_reuses']>totals['visual_checks'] and totals['cache_hits']>totals['cache_uploads']
  timings={label:{key:statistics.median(f['profile'][key] for f in result['world_frames'][1:]) for key in ('cpu_composition_ms','resource_prepare_ms','gpu_submit_ms','gpu_readback_ms','consumer_entry_to_return_ms')} for label,result in zip(('before','after'),results)}
  for label,result in zip(('before','after'),results):
   timings[label]['native_work_ms']=statistics.median(sum(f['profile'][k] for k in ('cpu_composition_ms','history_adopt_ms','resource_prepare_ms','gpu_submit_ms','gpu_readback_ms','gpu_compare_ms')) for f in result['world_frames'][1:])
  if a.compare_original:
   run('original-comparison',['python3',frozen/'tools/compare-world-cpu-original.py','--source',frozen,'--binary',build/'mnm-canvas-producers-live','--pe',ROOT/'original/Arcane_Nocd/Chaos.exe','--capture-report',a.capture_report.resolve(),'--inputs',inputs,'--assets',assets,'--output',out/'original-comparison'])
   report['original_comparison']=json.loads((out/'original-comparison/report.json').read_text())
  report.update(success=True,inputs=input_hashes,inputs_stable=True,baseline_sources=before['sources'],baseline_binary_sha256=sha(baseline/'build/mnm-canvas-producers-live'),native_binary_sha256=sha(build/'mnm-canvas-producers-live'),capture_report_sha256=sha(a.capture_report),owned_stream_sha256=sha(stream),asset_sources=asset_hashes,checkpoints=checks,completed=len(checks),warm_median_ms=timings,reuse_totals=totals,identity_totals=identity_totals,world_frames=frames,native_tests_passed=0 if a.optimized else 6,build_type='RelWithDebInfo' if a.optimized else 'Debug',compiled_dependency_review={'compiled_dependencies':sorted(compiled),'undeclared_dependencies':[]},frozen_sources_stable=all(sha(frozen/n)==h for n,h in sources.items()))
  if a.budget_ms is not None:
   actual=timings['after']['native_work_ms'];queue=timings['after']['consumer_entry_to_return_ms'];report['performance_budget']=dict(native_work_warm_median_limit_ms=a.budget_ms,native_work_measured_ms=actual,diagnostic_queue_limit_ms=100,diagnostic_queue_measured_ms=queue,passed=actual<=a.budget_ms and queue<=100)
   assert report['performance_budget']['passed'],'Native warm work or diagnostic queue exceeds performance budget'
 except Exception as error:report['success']=False;report['error']=str(error)
 finally:
  run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
  report.update(original_manifest_verified_before_after=True,sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()))
  report['success']=report['success'] and report['sources_stable']
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
 if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
