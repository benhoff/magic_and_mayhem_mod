#!/usr/bin/env python3
"""Run an unchanged specialized comparer with a separately certified edge oracle."""
import argparse,hashlib,json,runpy,sys
from pathlib import Path
import comparison_process,coverage_evidence
from coverage_evidence import Reader,write_report
from broader_world_policy import POLICY,assess
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
def sha(path):
 with Path(path).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('plan','case','raw-comparison','oracle-build','controls','report'):p.add_argument('--'+n,required=True)
 args=p.parse_args();require(not(ROOT/args.report).exists(),'Fresh World policy comparison required')
 with Reader(ROOT)as r:
  ref=lambda n:dict(record=n,sha256=r.digest(n));phase=r.json(args.plan);case=next(v for v in phase['broader_cases']if v['id']==args.case);raw=r.json(args.raw_comparison);built=r.json(args.oracle_build);controls=r.json(args.controls)
  require(raw['acquisition']==ref(case['report'])and raw['sources_stable']and raw['comparison_processes_stopped']and raw['original_manifest_verified_before_after'],'Unbound/incomplete raw World comparison')
  if not raw['success']:require(raw.get('comparison',{}).get('mismatching_pixels',0)>0,'Raw failure is not a complete pixel disagreement')
  require(built['success']and controls['success']and controls['cases']==26 and all(r.digest(n)==h for n,h in built['sources'].items()),'World policy oracle not current')
  binary=Path(built['binary']['path']);require(sha(binary)==built['binary']['sha256']==r.digest(built['binary']['record']),'Exact World policy oracle differs')
  refs={n:ref(v)for n,v in [('raw_original_comparison',args.raw_comparison),('oracle_build',args.oracle_build),('controls',args.controls)]}
  sources=built['sources']|{n:r.digest(n)for n in('tools/compare-broader-world-policy.py','tools/broader_world_policy.py')}
 command=list(case['comparison_command']);require(command[:1]==['python3']and command.count('--report')==1 and command.count('--acquisition')==1 and command[command.index('--acquisition')+1]==case['report'],'Unknown specialized comparer command');command[command.index('--report')+1]=args.report
 filename=ROOT/command[1];require(filename.is_relative_to(ROOT/'tools')and filename.suffix=='.py','Unsafe specialized comparer')
 original_run=comparison_process.Processes.run;original_write=coverage_evidence.write_report;groups=[]
 def execute(owner,argv,*positional,**keywords):
  actual=list(argv);history=ROOT/'working/build/qt-shell/mnm-acceptance-world-history'
  selected=bool(actual)and Path(str(actual[0])).resolve()==history.resolve()
  if selected:
   require(len(actual)==6 and Path(str(actual[3])).resolve()==(ROOT/'working/original-reference').resolve(),'Unknown original World invocation');actual[3]=str(binary)
  result=original_run(owner,actual,*positional,**keywords)
  if selected:
   source,output=Path(str(actual[1])),Path(str(actual[5]));index_bytes=(output/'worlds.json').read_bytes();index=json.loads(index_bytes)
   require(index['success']and index['native_player_pixels_read']is False and index['original_destination_capture_read']is False and 0<len(index['worlds'])<=8192,'Incomplete independent policy World index')
   rows=[]
   for generation,world in enumerate(index['worlds'],1):
    require(int(world['generation'])==generation and Path(world['path']).name==world['path'],'Policy World source order')
    log=output/(world['path'].removesuffix('.qz')+'.565.finite-edge.json');require(log.stat().st_size<=1024**2,'Policy tap certificate budget')
    value=json.loads(log.read_bytes());require(value['schema']==1 and value['policy']==POLICY and value['original_padding_equivalence_claimed']is False,'Unknown per-World policy certificate')
    rows.append(dict(generation=generation,width=world['width'],height=world['height'],taps=value['taps']))
   body=dict(schema=1,policy=POLICY,original_padding_equivalence=False,index_sha256=hashlib.sha256(index_bytes).hexdigest(),worlds=rows)
   name=args.report.removesuffix('.json')+'-edge-certificates-%02d.json'%len(groups);reference=dict(record=name,sha256=original_write(ROOT,name,(json.dumps(body,separators=(',',':'))+'\n').encode()))
   groups.append(dict(source_sha256=sha(source),source_bytes=source.stat().st_size,output_identity=str(output),worlds=len(rows),retained_taps=sum(len(v['taps'])for v in rows),index_sha256=body['index_sha256'],certificate=reference))
  return result
 def writer(root,path,body):
  if str(path)==args.report:
   q=json.loads(body)
   q['native_edge_policy']=dict(id=POLICY,**refs,reference_groups=groups,certified_worlds=sum(v['worlds']for v in groups),retained_taps=sum(v['retained_taps']for v in groups),ignored_pixels=0,all_output_pixels_compared=True,original_padding_equivalence=False,production_native_code_changed=False,only_original_history_oracle_executable_bound=True,original_game_runs_added=0)
   if isinstance(q.get('original_reference'),dict):q['original_reference'].update(reference_model='original-rasters-with-explicit-native-finite-edge',original_equivalence_for_padding=False)
   if 'binaries'in q:
    old=q['binaries'].pop('working/original-reference',None)
    if old:q['baseline_oracle_binary_checked']=dict(path='working/original-reference',sha256=old,executed_for_this_policy_comparison=False)
    q['binaries'][str(binary)]=built['binary']['sha256']
   q['sources'].update(sources)
   with Reader(ROOT)as r:
    stable=all(r.digest(n)==h for n,h in q['sources'].items());q['sources_stable']&=stable;q['success']&=stable
    if q['success']:
     try:require(assess(r,q)['verified'],'World policy certificate admission failed')
     except Exception as error:q.update(success=False,policy_admission_error=str(error))
   body=(json.dumps(q,indent=2)+'\n').encode()
  return original_write(root,path,body)
 comparison_process.Processes.run=execute;coverage_evidence.write_report=writer
 try:
  sys.argv=[str(filename),*command[2:]]
  try:runpy.run_path(str(filename),run_name='__main__')
  except SystemExit as stopped:
   require(stopped.code in(None,0,1),'Specialized policy comparer setup refused')
 finally:comparison_process.Processes.run=original_run;coverage_evidence.write_report=original_write
 with Reader(ROOT)as r:q=r.json(args.report)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
