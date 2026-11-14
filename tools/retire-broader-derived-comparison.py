#!/usr/bin/env python3
"""Preserve and retire only exact completed temporary recovery comparison trees."""
import argparse,hashlib,json,shutil,tempfile,uuid
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured,_sync
from evidence_workspace import idle,scan,materialize,MARKER
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];P='research/runtime/broader-player-acceptance-results-20261110/'
PARENTS={'task-owned-tmpfs-derived-comparison-v1':Path('/tmp/mnm-broader-recovery-comparisons-20261110'),'task-owned-tmpfs-derived-policy-comparison-v1':Path('/tmp/mnm-broader-policy-comparisons-20261110')}
def target(name,c):
 s=c['comparison_storage'];path=Path(s['path']);require(s['temporary']is True and s['policy']in PARENTS and path.parent==PARENTS[s['policy']]and path.name==Path(name).stem,'Unknown or foreign temporary comparison root');return path

def seal_temporary(path,expected,pair):
 require(path.parent in PARENTS.values()and not any(v.is_symlink()for v in[path,*path.parents]),'External temporary root refused');idle(path)
 rows,identity,allocated=scan(path);require(rows==expected,'Temporary outputs changed after planning')
 with pair.batch():
  packed,ids,_=scan(path,pair);require(packed==rows and ids==identity,'Temporary output changed while sealing');manifest=pair.root('execution_workspace',path,dict(schema=1,kind='execution_workspace',source=str(path),members=rows,pin_scope='Every original temporary output byte, metadata and pin; native execution and original captures remain separately retained.'))
 require(sum(v.get('size',0)for v in rows)<=12*1024**3,'Temporary real restore exceeds declared budget')
 with tempfile.TemporaryDirectory(prefix='mm-derived-restore-')as tmp:materialize(pair,manifest,Path(tmp)/'restored',replica=1)
 idle(path);require(scan(path)[1]==identity,'Temporary output changed before retirement');result=dict(success=True,source=str(path),manifest=manifest,real_restore_verified=True,replicas_verified=2,allocated_bytes=allocated);pair.root('temporary_derived_comparison_verified',path,result)
 quarantine=path.with_name(path.name+'.handoff-'+uuid.uuid4().hex);path.rename(quarantine)
 try:
  idle(quarantine);require(scan(quarantine)[0]==rows,'Temporary outputs changed after quarantine');path.mkdir(mode=0o700);(path/MARKER).write_text(json.dumps(manifest,separators=(',',':'))+'\n');(path/'.retain').write_text('Exact completed temporary comparison bytes and original metadata retained in two verified stores with a real restore.\nManifest: '+manifest['sha256']+'\nRestore into a new directory: python3 tools/retire-broader-derived-comparison.py --manifest '+manifest['sha256']+' --size '+str(manifest['size'])+' --output NEW_DIRECTORY\n');_sync(path/MARKER);_sync(path/'.retain')
 except BaseException:
  if path.exists()and set(v.name for v in path.iterdir())<={MARKER,'.retain'}:
   for v in path.iterdir():v.unlink()
   path.rmdir()
  if not path.exists():quarantine.rename(path)
  raise
 shutil.rmtree(quarantine);result['retired']=True;result['reclaimed_allocated_bytes']=allocated-sum(v.stat().st_blocks*512 for v in path.iterdir());pair.root('temporary_derived_comparison_retired',path,result);return result

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('validation','report','manifest','size','output'):p.add_argument('--'+n)
 args=p.parse_args()
 if args.manifest:
  require(args.size and args.output and not args.validation and not args.report,'Explicit new restore destination required')
  with configured(ROOT)as pair:materialize(pair,dict(sha256=args.manifest,size=int(args.size)),Path(args.output))
  return
 require(args.validation and args.report and not(args.size or args.output),'Complete retirement arguments required');require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh derived-output handoff required')
 with Reader(ROOT)as r:
  v=r.json(args.validation);require(v['success']and v['sources_stable']and all(r.digest(n)==h for n,h in v['sources'].items()),'Current admitted case required');ar=v['acquisition'];require(r.digest(ar['record'])==ar['sha256'],'Acquisition changed');a=r.json(ar['record']);require(a['success']and a['private_wine_stopped']and a['prefix_pool']['returned'],'Actual producer still active')
  cr=v['comparison'];require(r.digest(cr['record'])==cr['sha256'],'Comparison changed');c=r.json(cr['record']);require(c['success']and c['comparison_processes_stopped'],'Failed/active comparison');refs=[cr]
  if c.get('native_edge_policy'):refs.append(c['native_edge_policy']['raw_original_comparison'])
  plans=[]
  for ref in refs:
   require(r.digest(ref['record'])==ref['sha256'],'Derived comparison identity changed');q=r.json(ref['record']);storage_name=ref['record']
   if q.get('retained_policy_analysis'):
    rebind=q['retained_policy_analysis'];parent=rebind['parent'];require(r.digest(parent['record'])==parent['sha256']and rebind['new_original_game_runs']==rebind['new_original_reference_executions']==rebind['new_pixel_comparisons']==0 and rebind['oracle_binary_identical']and rebind['expected_and_actual_pixels_unchanged'],'Unknown derived-analysis rebind');old=r.json(parent['record']);require(old['artifacts']==q['artifacts']and old['comparison_storage']==q['comparison_storage'],'Rebound output changed');storage_name=parent['record']
   require(q['comparison_processes_stopped']and q['sources_stable']and q['original_manifest_verified_before_after'],'Incomplete derived comparison');path=target(storage_name,q);require(not any(v.is_symlink()for v in[path,*path.parents]),'Temporary root traverses symlink');idle(path);require(not(path/MARKER).exists(),'Already retired comparison');rows,identities,allocated=scan(path)
   require(all(row['kind']in('file','directory')for row in rows),'Unexpected temporary output type')
   for artifact in q['artifacts']:
    require(artifact['path']=='comparison-outputs.tar.gz'and r.digest(artifact['record'])==artifact['sha256']==hashlib.sha256((path/artifact['path']).read_bytes()).hexdigest(),'Exact immutable comparison archive missing')
   require(q['archive_content_references']['all_member_bytes_verified'],'Original archive verification missing');plans.append(dict(comparison=ref,path=str(path),members=rows,allocated_bytes=allocated))
  vr=dict(record=args.validation,sha256=r.digest(args.validation))
 plan=dict(schema=1,kind='validated_broader_derived_RAM_handoff_plan',success=True,original_game_runs=0,validation=vr,acquisition=ar,paths=plans,scope='Only exact temporary derived recovery output directories. Preserve every byte, metadata and original pin, both raw-original negative and native-policy positive phases. Shared build, input, capture and all other paths remain held.')
 pn=args.report.removesuffix('.json')+'-plan.json';ph=write_report(ROOT,pn,(json.dumps(plan,indent=2)+'\n').encode())
 with configured(ROOT)as pair:results=[seal_temporary(Path(item['path']),item['members'],pair)for item in plans]
 q=dict(schema=1,kind='validated_broader_derived_RAM_handoff',success=True,original_game_runs=0,validation=vr,plan=dict(record=pn,sha256=ph),results=results,acceptance_status_changed=False,source=dict(path='tools/retire-broader-derived-comparison.py',sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()));h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,trees=len(results))),flush=True)
if __name__=='__main__':main()
