import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_prelaunch_review import reviewed_with_setup_refusals
class Reader:
 def __init__(self,values):self.values=values
 def json(self,n):return self.values[n]
 def digest(self,n):return'hash-'+n
def fixture():
 a=dict(success=False,delegate_success=False,sources_stable=True,original_game_runs=0,charged_original_starts=0,actual_start_count_complete=True,observed_session_roots=[],error='Stop after unreviewed surface-loss acquisition/comparison',provider_adaptation=dict(host_receipts=[],actual_drivers=[],actual_qt_environments=[],capture_storage=dict(entries=[])))
 review=dict(kind='broader_zero_game_setup_refusal_review',success=True,case_accepted=False,original_game_runs=0,reserved_original_starts_retained=2,acquisition=dict(record='refused',sha256='hash-refused'),source_continuation_review=dict(record='continuation',sha256='hash-continuation'),reviewer_sources=dict(reviewer='hash-reviewer'))
 r=Reader(dict(refused=a,review=review));rows=[dict(case_id='passed',state='admitted',reserved_games=2),dict(case_id='zero',state='failed',report='refused',reserved_games=2,acquisition_sha256='hash-refused')];p=dict(source_continuation_review=review['source_continuation_review'],preserved_setup_refusals=dict(zero=dict(record='review',sha256='hash-review')));return r,rows,p
class PrelaunchReview(unittest.TestCase):
 def test_no_charge_refund_or_row_rewrite(self):
  r,rows,p=fixture();before=copy.deepcopy(rows);self.assertEqual(reviewed_with_setup_refusals(r,rows,p),4);self.assertEqual(rows,before)
 def test_unknown_or_nonzero_start_refuses(self):
  for field,value in[('actual_start_count_complete',False),('original_game_runs',1),('charged_original_starts',1)]:
   r,rows,p=fixture();r.values['refused'][field]=value
   with self.subTest(field=field),self.assertRaises(ValueError):reviewed_with_setup_refusals(r,rows,p)
 def test_host_or_player_execution_refuses(self):
  r,rows,p=fixture();r.values['refused']['provider_adaptation']['host_receipts']=[{}]
  with self.assertRaises(ValueError):reviewed_with_setup_refusals(r,rows,p)
 def test_acceptance_or_refund_refuses(self):
  for field,value in[('case_accepted',True),('reserved_original_starts_retained',0)]:
   r,rows,p=fixture();r.values['review'][field]=value
   with self.subTest(field=field),self.assertRaises(ValueError):reviewed_with_setup_refusals(r,rows,p)
 def test_running_or_wrong_acquisition_refuses(self):
  for kind in('running','identity'):
   r,rows,p=fixture()
   if kind=='running':rows[1]['state']='requested'
   else:rows[1]['acquisition_sha256']='other'
   with self.subTest(kind=kind),self.assertRaises(ValueError):reviewed_with_setup_refusals(r,rows,p)
 def test_unreviewed_still_stops(self):
  r,rows,p=fixture();p['preserved_setup_refusals']={}
  with self.assertRaises(ValueError):reviewed_with_setup_refusals(r,rows,p)
if __name__=='__main__':unittest.main()
