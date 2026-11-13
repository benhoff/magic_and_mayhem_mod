#!/usr/bin/env python3
"""Retain ordinary top-level execution files; keep publication hardlinks and pins."""
import argparse,hashlib,json,os,shutil,stat,tempfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from evidence_objects import configured,_sync
from evidence_workspace import idle,materialize,safe
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
MARKER='TOP-FILES-ARCHIVED.txt'
def identity(path):
 s=path.lstat();return[s.st_dev,s.st_ino,s.st_size,s.st_mtime_ns,s.st_ctime_ns]
def row(path,name):
 s=path.lstat();return dict(path=name,kind='directory'if stat.S_ISDIR(s.st_mode)else'file',mode=stat.S_IMODE(s.st_mode),mtime_ns=s.st_mtime_ns,uid=s.st_uid,gid=s.st_gid,xattrs={k:os.getxattr(path,k,follow_symlinks=False).hex()for k in sorted(os.listxattr(path,follow_symlinks=False))})
def prepare(root,metadata):
 require(root.is_dir()and not any(p.is_symlink()for p in[root,*root.parents])and not(root/MARKER).exists(),'Expected pinned retained publication root')
 idle(root);expected={v['path']:v for v in metadata};files=[];stamps={}
 require(('.retain'in expected)==(root/'.retain').exists(),'Original pin state changed')
 if '.retain'in expected:
  require((root/'.retain').is_file()and hashlib.sha256((root/'.retain').read_bytes()).hexdigest()==expected['.retain']['sha256'],'Original pin changed')
 for f in sorted(root.iterdir()):
  s=f.lstat()
  if not stat.S_ISREG(s.st_mode)or f.name=='.retain'or s.st_nlink!=1:continue
  require(f.name in expected,'Unreviewed top-level file');old=expected[f.name];data=row(f,f.name);data.update(size=s.st_size,sha256=hashlib.sha256(f.read_bytes()).hexdigest())
  require(old['kind']=='file'and all(data[k]==old[k]for k in('mode','mtime_ns','uid','gid','xattrs','size','sha256')),'Retained top-level file changed: '+f.name)
  files.append(data);stamps[f.name]=identity(f)
 require(files,'No reviewed ordinary files remain')
 return dict(source=str(root),members=[row(root,'.'),*files],identities=stamps,allocated_bytes=sum((root/f['path']).stat().st_blocks*512 for f in files))
def transfer(pair,plan,handoff,scratch):
 root=Path(plan['source']);idle(root)
 require(all(identity(root/n)==v for n,v in plan['identities'].items()),'Top-level execution changed before retention')
 with pair.batch():
  for member in plan['members'][1:]:
   f=root/member['path'];require(identity(f)==plan['identities'][member['path']],'Top-level execution identity changed')
   with f.open('rb')as stream:require(pair.put(stream,member['size'])==member['sha256'],'Top-level file changed during retention')
  manifest=pair.root('execution_workspace',root,dict(schema=1,kind='execution_workspace',source=str(root),members=plan['members'],scope='Selected ordinary top-level files only. Publication hardlinks, original pin, symlinks and all children stay physically intact.',completed_handoff=handoff))
 with tempfile.TemporaryDirectory(prefix='mm-held-top-restore-',dir=scratch)as tmp:materialize(pair,manifest,Path(tmp)/'restored',replica=1)
 idle(root);require(all(identity(root/n)==v for n,v in plan['identities'].items()),'Top-level execution changed before retirement')
 result=dict(source=str(root),manifest=manifest,completed_handoff=handoff,real_restore_verified=True,replicas_verified=2,original_pin_preserved=True,publication_hardlinks_modified=False,selected_files=[m['path']for m in plan['members'][1:]])
 pair.root('broader_held_top_files_prepared',root,result)
 marker=root/MARKER
 with marker.open('x')as f:
  f.write(json.dumps(dict(manifest=manifest,scope='Selected top-level ordinary files only; original publication hardlinks and pin remain here.'),sort_keys=True)+'\n')
  f.write('Restore to a NEW directory: python3 '+str(ROOT/'tools/retire-broader-held-top-files.py')+' restore --manifest '+manifest['sha256']+' --size '+str(manifest['size'])+' --output <new-directory>\n')
 _sync(marker)
 for member in plan['members'][1:]:
  f=root/member['path'];require(identity(f)==plan['identities'][member['path']],'Top-level execution changed at retirement');f.unlink()
 _sync(marker);result.update(success=True,reclaimed_allocated_bytes=plan['allocated_bytes']-marker.stat().st_blocks*512)
 pair.root('broader_held_top_files_completed',root,result);return result

def main():
 parser=argparse.ArgumentParser(description=__doc__);sub=parser.add_subparsers(dest='operation',required=True)
 p=sub.add_parser('retire');p.add_argument('--handoff',required=True);p.add_argument('--report',required=True)
 p=sub.add_parser('restore');p.add_argument('--manifest',required=True);p.add_argument('--size',type=int,required=True);p.add_argument('--output',required=True)
 args=parser.parse_args()
 if args.operation=='restore':
  with configured(ROOT)as pair:materialize(pair,dict(sha256=args.manifest,size=args.size),Path(args.output),replica=1)
  return
 require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh selective handoff report required')
 with Reader(ROOT)as r:
  h=r.json(args.handoff);require(h['success']and h['original_hardlinks_modified']is False,'Completed exact execution handoff required')
  require(r.digest(h['plan']['record'])==h['plan']['sha256'],'Changed original handoff plan');original=r.json(h['plan']['record'])
  require(original['success']and original['validation']==h['validation'],'Unbound completed handoff plan')
  validation=r.json(h['validation']['record']);require(validation['success']and r.digest(h['validation']['record'])==h['validation']['sha256'],'Completed validation identity differs')
  ref=dict(record=args.handoff,sha256=r.digest(args.handoff));plans=[]
  for held in original['held_native_publication_roots']:
   root=safe(ROOT,Path(held['path']));require(str(root)in h['held_native_publication_roots'],'Unreviewed held root')
   for group in held['original_hardlink_groups']:
    files=[root/v['path']for v in group];require(len(files)==2 and {v.name for v in files}=={'surface-loss-request-v1.json','surface-loss-request-v1.json.pending'}and all(f.stat().st_nlink==2 for f in files)and files[0].stat().st_ino==files[1].stat().st_ino,'Publication pair changed')
   plans.append(prepare(root,held['metadata']))
 plan_name=args.report.removesuffix('.json')+'-plan.json';plan_sha=write_report(ROOT,plan_name,(json.dumps(dict(schema=1,success=True,completed_handoff=ref,plans=plans),indent=2)+'\n').encode())
 with configured(ROOT)as pair:results=[transfer(pair,plan,ref,'/tmp')for plan in plans]
 q=dict(schema=1,kind='broader_selected_held_top_file_handoff',success=True,original_game_runs=0,completed_handoff=ref,plan=dict(record=plan_name,sha256=plan_sha),results=results,acceptance_status_changed=False,scope='Only exact reviewed ordinary top-level files of completed stopped execution roots transfer. Both original publication hardlinks, original pin and all other paths remain untouched; exact bytes and metadata survive two verified stores and a real restore.')
 sha=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=sha,reclaimed_allocated_bytes=sum(v['reclaimed_allocated_bytes']for v in results))),flush=True)
if __name__=='__main__':main()
