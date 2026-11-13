import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_font_reference_final_gate import exact_font_reference_ledger,FAILED,REFERENCE_FAILED
class LedgerTests(unittest.TestCase):
 def setUp(self):
  self.old=[dict(case_id='old-'+str(i),reserved_games=2 if i<40 else 1,state='admitted')for i in range(93)];self.failed=dict(case_id=FAILED,reserved_games=1,state='failed',acquisition_sha256='negative');self.zero=dict(case_id='bp30-world-caller-default800-01',reserved_games=1,state='failed',acquisition_sha256=None);self.font=dict(case_id='bp31-world-font-default800-01',reserved_games=1,state='failed',acquisition_sha256='font-negative');self.reference=dict(case_id=REFERENCE_FAILED,reserved_games=1,state='failed',acquisition_sha256='reference-negative');self.values=[dict(spec=dict(case=dict(id='positive-'+str(i),reserved_original_games=2 if i<8 else 1)))for i in range(90)];self.rows={v['case_id']:copy.deepcopy(v)for v in self.old+[self.failed,self.zero,self.font,self.reference]};self.rows.update({v['spec']['case']['id']:dict(reserved_games=v['spec']['case']['reserved_original_games'],state='admitted')for v in self.values})
 def run_gate(self):exact_font_reference_ledger(self.rows,self.values,self.old,self.failed,self.zero,self.font,self.reference)
 def test_complete(self):self.run_gate()
 def test_old231refuses(self):
  self.rows.pop(FAILED)
  with self.assertRaises(ValueError):self.run_gate()
 def test_refund_refuses(self):
  self.rows[FAILED]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_negative_promoted_refuses(self):
  self.rows[FAILED]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_zero_game_refund_refuses(self):
  self.rows[self.zero['case_id']]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_zero_game_promotion_refuses(self):
  self.rows[self.zero['case_id']]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_font_refund_refuses(self):
  self.rows[self.font['case_id']]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_font_promotion_refuses(self):
  self.rows[self.font['case_id']]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_font_hash_changed_refuses(self):
  self.rows[self.font['case_id']]['acquisition_sha256']='changed'
  with self.assertRaises(ValueError):self.run_gate()
 def test_missing_positive_refuses(self):
  self.rows.pop('positive-0')
  with self.assertRaises(ValueError):self.run_gate()
 def test_extra_request_refuses(self):
  self.rows['spare']=dict(reserved_games=1)
  with self.assertRaises(ValueError):self.run_gate()
 def test_duplicate_positive_refuses(self):
  self.values[-1]=copy.deepcopy(self.values[-2])
  with self.assertRaises(ValueError):self.run_gate()
 def test_old_metadata_changed_refuses(self):
  self.rows['old-0']['state']='failed'
  with self.assertRaises(ValueError):self.run_gate()
 def test_negative_hash_changed_refuses(self):
  self.rows[FAILED]['acquisition_sha256']='changed'
  with self.assertRaises(ValueError):self.run_gate()
 def test_failure_receives_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']=FAILED
  with self.assertRaises(ValueError):self.run_gate()
 def test_history_receives_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']='old-0'
  with self.assertRaises(ValueError):self.run_gate()
 def test_start_domain_shrinks_refuses(self):
  self.values[0]['spec']['case']['reserved_original_games']=1
  with self.assertRaises(ValueError):self.run_gate()
 def test_reference_missing_refuses(self):
  self.rows.pop(REFERENCE_FAILED)
  with self.assertRaises(ValueError):self.run_gate()
 def test_reference_refund_refuses(self):
  self.rows[REFERENCE_FAILED]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_reference_promoted_refuses(self):
  self.rows[REFERENCE_FAILED]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_reference_hash_changed_refuses(self):
  self.rows[REFERENCE_FAILED]['acquisition_sha256']='changed'
  with self.assertRaises(ValueError):self.run_gate()
 def test_reference_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']=REFERENCE_FAILED
  with self.assertRaises(ValueError):self.run_gate()
if __name__=='__main__':unittest.main()
