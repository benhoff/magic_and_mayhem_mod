#!/usr/bin/env python3
"""Storage controls for canonical report materializations; no game execution."""
import hashlib,io,json,os
from pathlib import Path
import subprocess,sys,tempfile,time,unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import worktree_report_compaction as compaction
from coverage_evidence import Store,pointer
from evidence_objects import Pair


class Compaction(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory(dir=ROOT/'working');self.addCleanup(self.temp.cleanup)
  self.shm=tempfile.TemporaryDirectory(dir='/dev/shm');self.addCleanup(self.shm.cleanup)
  self.base=Path(self.temp.name);self.main=self.base/'main';self.target=self.base/'old'
  self.main.mkdir();self.git(self.main,'init','-q','-b','fixture')
  self.git(self.main,'config','user.name','Fixture');self.git(self.main,'config','user.email','fixture@example.invalid')
  self.data=(json.dumps({'success':True,'payload':'unchanged '*3000})+'\n').encode()
  self.sha=hashlib.sha256(self.data).hexdigest();self.name='research/runtime/report.json'
  path=self.main/self.name;path.parent.mkdir(parents=True);path.write_bytes(pointer(self.sha,len(self.data)))
  (self.main/'.gitignore').write_text('working/\n')
  self.git(self.main,'add','.');self.git(self.main,'commit','-qm','source')
  self.git(self.main,'branch','master');self.git(self.main,'worktree','add','-q','-b','historical',str(self.target))
  self.path=self.target/self.name;self.path.write_bytes(self.data)
  old=time.time_ns()-48*3600*10**9;os.utime(self.path,ns=(old,old))
  self.primary=self.main/'working/primary.sqlite3';self.replica=Path(self.shm.name)/'working/replica.sqlite3'
  policy=self.main/'policy.json';policy.write_text(json.dumps(dict(schema=2,durable_copies=2,bulk_copies=1,primary_minimum_free_gib=0,replica_minimum_free_gib=0)))
  for key,value in {'mm.evidencePrimary':str(self.primary),'mm.evidenceReplica':str(self.replica),'mm.evidencePending':str(self.primary),'mm.evidenceExtra':str(self.replica),'mm.evidenceStoragePolicy':str(policy)}.items():self.git(self.main,'config',key,value)
  with Pair(self.primary,self.replica,hybrid=True) as pair:pair.put(io.BytesIO(self.data),len(self.data))
  self.plan=self.main/'working/plan.sqlite3'
 def git(self,root,*args):
  return subprocess.check_output(['git','-C',str(root),*args],stderr=subprocess.PIPE)
 def prepare(self):return compaction.prepare(self.target,self.main,self.main,self.plan,'Reviewed closed fixture; original bytes and working state preserved',self.shm.name)
 def test_exact_payload_and_metadata_are_preserved(self):
  old=self.path.stat().st_mtime_ns;result=self.prepare()
  self.assertEqual(result['files'],1)
  applied=compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  self.assertEqual(applied['files'],1);self.assertGreater(applied['reclaimed_allocated_bytes'],0)
  self.assertEqual(self.path.read_bytes(),pointer(self.sha,len(self.data)))
  self.assertEqual(self.path.stat().st_mtime_ns,old)
  self.assertEqual(self.git(self.target,'show','HEAD:'+self.name),self.path.read_bytes())
 def test_unique_edit_is_not_selected(self):
  self.path.write_bytes(self.data+b'unique')
  result=self.prepare();self.assertEqual(result['files'],0)
  self.assertEqual(self.path.read_bytes(),self.data+b'unique')
 def test_same_size_unique_edit_is_not_selected(self):
  changed=self.data.replace(b'unchanged',b'different',1);self.assertEqual(len(changed),len(self.data))
  self.path.write_bytes(changed);old=time.time_ns()-48*3600*10**9;os.utime(self.path,ns=(old,old))
  self.assertEqual(self.prepare()['files'],0);self.assertEqual(self.path.read_bytes(),changed)
 def test_stale_plan_refuses_before_replacement(self):
  result=self.prepare();self.path.write_bytes(b'new owner work')
  with self.assertRaisesRegex(ValueError,'Stale source'):compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  self.assertEqual(self.path.read_bytes(),b'new owner work')
 def test_plan_hash_is_required(self):
  self.prepare()
  with self.assertRaisesRegex(ValueError,'Plan hash changed'):compaction.apply(self.plan,'0'*64,self.main,self.main,self.shm.name)
  self.assertEqual(self.path.read_bytes(),self.data)
 def test_missing_second_copy_refuses_retirement(self):
  self.replica.unlink()
  with self.assertRaisesRegex(ValueError,'Two independent'):self.prepare()
  self.assertEqual(self.path.read_bytes(),self.data)
 def test_missing_copy_retention_is_explicit_and_verified(self):
  self.replica.unlink()
  result=compaction.prepare(self.target,self.main,self.main,self.plan,'Closed fixture; explicit essential retention',self.shm.name,retain_missing=True)
  self.assertEqual(result['files'],1)
  applied=compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  self.assertEqual(applied['files'],1)
 def test_same_filesystem_copies_do_not_count_twice(self):
  second=self.main/'working/same-device.sqlite3'
  a=Store(self.primary,readonly=True);dest=__import__('sqlite3').connect(second)
  try:a.db.backup(dest)
  finally:dest.close();a.close()
  copies=compaction.Copies.__new__(compaction.Copies)
  copies.stores=[Store(self.primary,readonly=True),Store(second,readonly=True)]
  try:
   with self.assertRaisesRegex(ValueError,'Two independent'):copies.verify(self.sha,len(self.data),self.shm.name)
  finally:copies.close()
 def test_corrupt_copy_cannot_authorize_replacement(self):
  with patch.object(Store,'copy',side_effect=ValueError('corrupt payload')):
   with self.assertRaisesRegex(ValueError,'corrupt'):self.prepare()
  self.assertEqual(self.path.read_bytes(),self.data)
 def test_locked_worktree_is_held(self):
  self.git(self.main,'worktree','lock',str(self.target))
  with self.assertRaisesRegex(ValueError,'lock'):self.prepare()
 def test_hardlinked_source_is_not_rewritten(self):
  alias=self.base/'shared-report';os.link(self.path,alias)
  self.assertEqual(self.prepare()['files'],0);self.assertEqual(alias.read_bytes(),self.data)
 def test_report_symlink_is_not_followed(self):
  alias=self.base/'external-report';alias.write_bytes(self.data);self.path.unlink();self.path.symlink_to(alias)
  self.assertEqual(self.prepare()['files'],0);self.assertEqual(alias.read_bytes(),self.data)
 def test_authored_accounting_is_not_selected(self):
  p=self.target/'research/runtime/coverage/register.json';p.parent.mkdir();p.write_bytes(self.data)
  self.assertEqual(self.prepare()['files'],1);self.assertEqual(p.read_bytes(),self.data)
 def test_activity_refusal_preserves_source(self):
  with patch('worktree_compaction.source_idle',side_effect=ValueError('active target')):
   with self.assertRaisesRegex(ValueError,'active'):self.prepare()
  self.assertEqual(self.path.read_bytes(),self.data)
 def test_other_working_data_is_untouched(self):
  p=self.target/'working/save.cfg';p.parent.mkdir();p.write_bytes(b'keep this configuration')
  result=self.prepare();compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  self.assertEqual(p.read_bytes(),b'keep this configuration')
 def test_plan_restore_failure_cannot_admit_replacement(self):
  with patch('evidence_objects.restore_logical',side_effect=ValueError('restore refused')):
   with self.assertRaisesRegex(ValueError,'restore refused'):self.prepare()
  self.assertEqual(self.path.read_bytes(),self.data)
 def test_checkpoint_has_actual_schema_and_row_restore(self):
  result=self.prepare();proof=result['retained_plan']
  self.assertTrue(proof['real_restore_performed']);self.assertTrue(proof['logical_restore_verified'])
  self.assertEqual(proof['replicas_verified'],2);self.assertIn('verified_receipt',proof)
 def test_marker_symlink_cannot_modify_external_data(self):
  result=self.prepare();other=self.base/'external-notice';other.write_text('keep')
  (self.target/'WORKTREE-REPORTS-CANONICALIZED.txt').symlink_to(other)
  with self.assertRaisesRegex(ValueError,'aliased'):compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  self.assertEqual(other.read_text(),'keep');self.assertEqual(self.path.read_bytes(),self.data)
 def test_existing_different_marker_is_preserved(self):
  result=self.prepare();marker=self.target/'WORKTREE-REPORTS-CANONICALIZED.txt';marker.write_text('unreviewed recovery instructions')
  with self.assertRaisesRegex(ValueError,'marker needs review'):compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  self.assertEqual(marker.read_text(),'unreviewed recovery instructions');self.assertEqual(self.path.read_bytes(),self.data)
 def test_binary_materialization_preserves_exact_bytes(self):
  self.path.write_bytes(pointer(self.sha,len(self.data)))
  data=bytes(range(256))*128;sha=hashlib.sha256(data).hexdigest()
  binary=self.target/'research/runtime/report.bin';binary.write_bytes(pointer(sha,len(data)))
  self.git(self.target,'add',str(binary));self.git(self.target,'commit','-qm','binary evidence')
  self.git(self.target,'branch','-f','master','HEAD')
  with Pair(self.primary,self.replica,hybrid=True) as pair:pair.put(io.BytesIO(data),len(data),copies=2)
  binary.write_bytes(data);old=time.time_ns()-48*3600*10**9;os.utime(binary,ns=(old,old))
  result=self.prepare();self.assertEqual(result['files'],1)
  compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  self.assertEqual(binary.read_bytes(),pointer(sha,len(data)))
  self.assertIn('Selected generated report payloads',(self.target/'WORKTREE-REPORTS-CANONICALIZED.txt').read_text())
 def test_plan_retains_exact_prior_allocation(self):
  self.prepare()
  with __import__('sqlite3').connect(self.plan) as db:
   row=json.loads(db.execute('SELECT record FROM files').fetchone()[0])
  self.assertEqual(row['allocated_bytes'],self.path.stat().st_blocks*512)
 def test_failure_flushes_completed_journal_prefix(self):
  other=self.target/'research/runtime/second.json';other.write_bytes(pointer(self.sha,len(self.data)))
  self.git(self.target,'add',str(other));self.git(self.target,'commit','-qm','second report')
  self.git(self.target,'branch','-f','master','HEAD');other.write_bytes(self.data)
  old=time.time_ns()-48*3600*10**9;os.utime(other,ns=(old,old))
  result=self.prepare();replace=compaction.replace_one
  def controlled(root,row):
   if row['path'].endswith('second.json'):raise ValueError('injected second-file refusal')
   return replace(root,row)
  with patch.object(compaction,'replace_one',side_effect=controlled):
   with self.assertRaisesRegex(ValueError,'injected'):compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  with __import__('sqlite3').connect(self.plan) as db:
   self.assertEqual(dict(db.execute('SELECT path,state FROM files')),{self.name:'canonicalized','research/runtime/second.json':'prepared'})
  self.assertEqual(self.path.read_bytes(),pointer(self.sha,len(self.data)));self.assertEqual(other.read_bytes(),self.data)
 def test_unrecorded_replacement_keeps_both_original_payloads(self):
  result=self.prepare();replace=compaction.replace_one
  def interrupted(root,row):
   replace(root,row);raise KeyboardInterrupt('after durable pointer, before journal row')
  with patch.object(compaction,'replace_one',side_effect=interrupted):
   with self.assertRaises(KeyboardInterrupt):compaction.apply(self.plan,result['sha256'],self.main,self.main,self.shm.name)
  with __import__('sqlite3').connect(self.plan) as db:
   self.assertEqual(db.execute('SELECT state FROM files').fetchone()[0],'prepared')
  self.assertEqual(self.path.read_bytes(),pointer(self.sha,len(self.data)))
  for name in (self.primary,self.replica):
   store=Store(name,readonly=True);restored=io.BytesIO()
   try:store.copy(self.sha,len(self.data),restored)
   finally:store.close()
   self.assertEqual(restored.getvalue(),self.data)

if __name__=='__main__':unittest.main()
