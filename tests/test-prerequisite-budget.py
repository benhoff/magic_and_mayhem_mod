import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_prerequisite_budget import validate_rows
class BudgetTests(unittest.TestCase):
 def setUp(self):
  self.old=[dict(case_id='old-'+str(i),report='old-'+str(i)+'.json',reserved_games=2 if i<40 else 1,state='failed'if i==0 else'admitted',acquisition_sha256='old-hash',plan='old-plan',started_ns=i,validation=None)for i in range(109)]
  self.new={'bp30-'+str(i):2 if i<8 else 1 for i in range(77)}
 def positive(self,i):
  cid='bp30-'+str(i);return dict(case_id=cid,report=cid+'.json',reserved_games=self.new[cid],state='admitted',acquisition_sha256='new-hash')
 def test_initial(self):self.assertEqual(validate_rows(self.old,self.old,self.new),149)
 def test_complete(self):self.assertEqual(validate_rows(self.old+[self.positive(i)for i in range(77)],self.old,self.new),234)
 def test_preserved_failure_allowed(self):self.assertEqual(validate_rows(self.old+[self.positive(0)],self.old,self.new),151)
 def test_new_failure_refuses(self):
  v=self.positive(0);v['state']='failed'
  with self.assertRaises(ValueError):validate_rows(self.old+[v],self.old,self.new)
 def test_requested_refuses(self):
  v=self.positive(0);v['state']='requested'
  with self.assertRaises(ValueError):validate_rows(self.old+[v],self.old,self.new)
 def test_duplicate_refuses(self):
  with self.assertRaises(ValueError):validate_rows(self.old+[self.old[0]],self.old,self.new)
 def test_refund_refuses(self):
  v=copy.deepcopy(self.old);v[0]['reserved_games']=1
  with self.assertRaises(ValueError):validate_rows(v,self.old,self.new)
 def test_failure_promotion_refuses(self):
  v=copy.deepcopy(self.old);v[0]['state']='admitted'
  with self.assertRaises(ValueError):validate_rows(v,self.old,self.new)
 def test_missing_history_refuses(self):
  with self.assertRaises(ValueError):validate_rows(self.old[1:],self.old,self.new)
 def test_spare_case_refuses(self):
  v=self.positive(0);v['case_id']='spare'
  with self.assertRaises(ValueError):validate_rows(self.old+[v],self.old,self.new)
 def test_missing_hash_refuses(self):
  v=self.positive(0);v['acquisition_sha256']=None
  with self.assertRaises(ValueError):validate_rows(self.old+[v],self.old,self.new)
 def test_wrong_charge_refuses(self):
  v=self.positive(0);v['reserved_games']=1
  with self.assertRaises(ValueError):validate_rows(self.old+[v],self.old,self.new)
 def test_foreign_report_refuses(self):
  v=self.positive(0);v['report']='foreign.json'
  with self.assertRaises(ValueError):validate_rows(self.old+[v],self.old,self.new)
 def test_expanded_domain_refuses(self):
  new=dict(self.new,spare=1)
  with self.assertRaises(ValueError):validate_rows(self.old,self.old,new)
 def test_unknown_fields_refuse(self):
  v=self.positive(0);v['waiver']=True
  with self.assertRaises(ValueError):validate_rows(self.old+[v],self.old,self.new)
if __name__=='__main__':unittest.main()
