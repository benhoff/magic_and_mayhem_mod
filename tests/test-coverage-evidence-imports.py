#!/usr/bin/env python3
"""Evidence-ID imports require unchanged records from independent actual co-parents."""
import copy,json,subprocess,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_merge_history import inherited_history
from coverage_gate import immutable_errors
REGISTER='research/runtime/coverage/register.json'
class EvidenceImportTests(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name)
  self.git('init','-q');self.git('config','user.name','Coverage import fixture');self.git('config','user.email','fixture@example.invalid')
  self.write([]);self.commit('base');self.base=self.git('rev-parse','HEAD').strip();self.git('switch','-qc','incoming')
  self.old=dict(id='proof',kind='native_integration',record='research/incoming.json',sha256='a'*64)
  self.write([self.old]);self.commit('incoming evidence');self.incoming=self.git('rev-parse','HEAD').strip()
  self.git('switch','-qc','integration',self.base);self.canonical=dict(self.old,record='research/ours.json',sha256='b'*64)
  self.write([self.canonical]);self.commit('independent evidence');self.ours=self.git('rev-parse','HEAD').strip()
  self.alias=dict(self.old,id='incoming-proof');self.mapping=dict(commit=self.incoming,historical_id='proof',imported_id='incoming-proof')
  self.current=dict(behaviors={},builds={},evidence={'proof':self.canonical,'incoming-proof':self.alias},evidence_id_imports=[self.mapping]);self.reviews={'reviews':[]}
 def git(self,*args):return subprocess.check_output(['git','-C',str(self.root),*args],stderr=subprocess.PIPE).decode()
 def write(self,evidence):
  p=self.root/REGISTER;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(dict(builds=[],behaviors=[],evidence=evidence)))
  (p.parent/'change-reviews.json').write_text(json.dumps(dict(reviews=[])))
 def commit(self,m):self.git('add','.');self.git('commit','-qm',m)
 def merge(self,committed=False):
  p=subprocess.run(['git','-C',str(self.root),'merge','--no-commit','--no-ff',self.incoming],stdout=subprocess.PIPE,stderr=subprocess.PIPE);self.assertEqual(p.returncode,1)
  self.write([self.canonical,self.alias]);self.git('add','.')
  if committed:self.commit('resolve collision')
 def history(self,staged=True):return inherited_history(self.root,self.incoming,self.current,self.reviews,staged)
 def immutable(self,history):
  before=dict(inventories={},builds={},evidence={'proof':self.old},evidence_keys={'proof':['fixture.cpp']})
  after=dict(inventories={},builds={},evidence=self.current['evidence'],evidence_keys={'proof':['fixture.cpp'],'incoming-proof':['fixture.cpp']},evidence_id_imports=self.current['evidence_id_imports'])
  return immutable_errors(before,after,history)
 def test_actual_pending_merge_preserves_both_reports(self):
  self.merge();h=self.history();self.assertEqual(h['errors'],[]);self.assertEqual(len(h['evidence_id_imports']),1);self.assertEqual(self.immutable(h),[])
 def test_committed_merge_valid_against_push_base(self):
  self.merge(True);h=self.history(False);self.assertEqual(h['errors'],[]);self.assertEqual(self.immutable(h),[])
 def test_unmerged_branch_cannot_authorize_import(self):
  h=self.history();self.assertTrue(h['errors']);self.assertTrue(self.immutable(h))
 def test_modified_hash_cannot_authorize_import(self):
  self.merge();self.current['evidence']['incoming-proof']['sha256']='c'*64;h=self.history();self.assertTrue(h['errors']);self.assertTrue(self.immutable(h))
 def test_modified_assertions_cannot_authorize_import(self):
  self.merge();self.current['evidence']['incoming-proof']['assertions']=[];self.assertTrue(self.history()['errors'])
 def test_ancestor_is_not_independent_collision(self):
  self.merge();self.current['evidence_id_imports'][0]['commit']=self.base;self.assertTrue(self.history()['errors'])
 def test_duplicate_import_refused(self):
  self.merge();self.current['evidence_id_imports'].append(copy.deepcopy(self.mapping));self.assertTrue(self.history()['errors'])
 def test_no_mapping_cannot_rewrite_evidence(self):
  self.assertTrue(self.immutable({}))
 def test_existing_mapping_cannot_be_removed(self):
  v=dict(inventories={},builds={},evidence={},evidence_keys={},evidence_id_imports=[self.mapping]);after=dict(v,evidence_id_imports=[]);self.assertTrue(immutable_errors(v,after))
 def test_source_keys_must_be_preserved_under_alias(self):
  self.merge();h=self.history();before=dict(inventories={},builds={},evidence={'proof':self.old},evidence_keys={'proof':['fixture.cpp']});after=dict(inventories={},builds={},evidence=self.current['evidence'],evidence_keys={'proof':['fixture.cpp'],'incoming-proof':[]});self.assertTrue(immutable_errors(before,after,h))
if __name__=='__main__':unittest.main()
