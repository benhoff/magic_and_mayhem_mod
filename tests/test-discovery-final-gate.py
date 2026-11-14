#!/usr/bin/env python3
"""Verify exact ledger projection and reuse of every prior predicate code object."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_discovery_final_gate as g

def fixture():
 old=[dict(case_id=g.FAILED if i==0 else 'old'+str(i),report=g.FAILED if i==0 else 'old'+str(i),reserved_games=2 if 1<=i<=49 else 1,state='failed'if i==0 else 'admitted',acquisition_sha256='h')for i in range(142)]
 new={'new'+str(i):1 for i in range(55)}
 rows={v['case_id']:dict(v)for v in old};rows.update({k:dict(case_id=k,report=k,reserved_games=n,state='admitted',acquisition_sha256='h')for k,n in new.items()});return rows,old,new
class Projection(unittest.TestCase):
 def test_complete_only_excludes_failure(self):
  rows,old,new=fixture();before=copy.deepcopy(rows);p=g.project_rows(rows,old,new);self.assertEqual(rows,before);self.assertEqual(set(rows)-set(p),{g.FAILED});self.assertEqual(len(p),196);self.assertEqual(sum(v['reserved_games']for v in p.values()),245)
 def test_incomplete(self):
  rows,old,new=fixture();del rows['new0']
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_extra(self):
  rows,old,new=fixture();rows['extra']=dict(rows['new0'],case_id='extra')
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_failed_promoted(self):
  rows,old,new=fixture();rows[g.FAILED]['state']='admitted'
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_failed_refunded(self):
  rows,old,new=fixture();rows[g.FAILED]['reserved_games']=0
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_failed_fabricated_acquisition(self):
  rows,old,new=fixture();rows[g.FAILED]['acquisition_sha256']=None
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_negative_not_removed_from_authoritative_ledger(self):
  rows,old,new=fixture();del rows[g.FAILED]
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_other_history_unchanged(self):
  rows,old,new=fixture();rows['old2']['report']='changed'
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_new_failure_no_credit(self):
  rows,old,new=fixture();rows['new0']['state']='failed'
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_new_charge_exact(self):
  rows,old,new=fixture();rows['new0']['reserved_games']=2
  with self.assertRaises(ValueError):g.project_rows(rows,old,new)
 def test_prior_code_objects_identical(self):
  for n in ['_FADING','_CALLER','_PRIOR','_WITNESS','_REBUILD','_UNCOVERED','_PAN','_COLLECTION']:
   self.assertIs(getattr(g,n).__code__,getattr(g.previous,n).__code__)
  self.assertIs(g._STRICT.__code__,g.previous.supplement.__code__);self.assertIs(g._COLLECTION_PROJECT.__code__,g.previous._PROJECT.__code__);self.assertIs(g._STRICT_PROJECT.__code__,g.strict_previous.project_rows.__code__)
 def test_context_lineage_only_adds_catalog(self):
  original=g._lineage_source.replace('discovery-source-catalog-01.json','strict-binding-source-catalog-01.json').replace("names=['strict-binding-source-catalog-01.json','collection-source-catalog-01.json'","names=['collection-source-catalog-01.json'")
  self.assertEqual(original,g.strict_previous._lineage_source)
 def test_prior_reservation_alias_keeps_original(self):
  prospective={g.FAILED:1,'another':1};before=copy.deepcopy(prospective);self.assertEqual(g.prior_case_reservations(prospective),{g.PILOT:1,'another':1});self.assertEqual(prospective,before)
 def test_prior_reservation_rejects_changed_domain(self):
  with self.assertRaises(ValueError):g.prior_case_reservations({g.FAILED:2})
if __name__=='__main__':unittest.main()
