#!/usr/bin/env python3
"""Exercise immutable history, finite reservations and failure boundaries."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_strict_binding_budget import validate_rows

def old():return [dict(case_id='old'+str(i),report='old'+str(i),reserved_games=2 if i<49 else 1,state='failed'if i==0 else'admitted',acquisition_sha256=None if i==0 else'h')for i in range(137)]
def new():return {'new'+str(i):1 for i in range(59)}
def row():return dict(case_id='new0',report='new0',reserved_games=1,state='admitted',acquisition_sha256='h')
class Ledger(unittest.TestCase):
 def test_snapshot(self):self.assertEqual(validate_rows(old(),old(),new()),186)
 def test_complete(self):self.assertEqual(validate_rows(old()+[dict(row(),case_id=k,report=k)for k in new()],old(),new()),245)
 def test_refund(self):
  v=old();v[0]['reserved_games']=1
  with self.assertRaises(ValueError):validate_rows(v,old(),new())
 def test_promotion(self):
  v=old();v[0]['state']='admitted'
  with self.assertRaises(ValueError):validate_rows(v,old(),new())
 def test_omission(self):
  with self.assertRaises(ValueError):validate_rows(old()[1:],old(),new())
 def test_duplicate(self):
  with self.assertRaises(ValueError):validate_rows(old()+[old()[0]],old(),new())
 def test_failure(self):
  with self.assertRaises(ValueError):validate_rows(old()+[dict(row(),state='failed',acquisition_sha256=None)],old(),new())
 def test_unknown(self):
  with self.assertRaises(ValueError):validate_rows(old()+[dict(row(),case_id='extra',report='extra')],old(),new())
 def test_no_acquisition(self):
  with self.assertRaises(ValueError):validate_rows(old()+[dict(row(),acquisition_sha256=None)],old(),new())
 def test_charge(self):
  with self.assertRaises(ValueError):validate_rows(old()+[dict(row(),reserved_games=2)],old(),new())
 def test_requested(self):
  with self.assertRaises(ValueError):validate_rows(old()+[dict(row(),state='requested')],old(),new())
 def test_foreign_report(self):
  with self.assertRaises(ValueError):validate_rows(old()+[dict(row(),report='foreign')],old(),new())
if __name__=='__main__':unittest.main()
