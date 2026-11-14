"""Negative custody is distinct from acceptance and cannot reset a start cap."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_player_requests import cumulative_bound,reviewed_predecessors

class Reader:
    def __init__(self,objects):self.objects=objects
    def json(self,n):return self.objects[n]
    def digest(self,n):return 'hash-'+n

class Accounting(unittest.TestCase):
    def inputs(self):
        continuation=dict(record='continuation',sha256='hash-continuation')
        a=dict(record='acquisition',sha256='a-hash')
        review=dict(kind='broader_player_negative_predecessor_review',success=True,case_accepted=False,
            original_game_runs=0,reserved_original_starts_retained=2,acquisition=a,
            source_continuation_review=continuation,failed_comparison=dict(record='failed',sha256='hash-failed'),reviewer_sources={'reviewer':'hash-reviewer'})
        objects=dict(review=review,failed=dict(success=False,comparison_processes_stopped=True,acquisition=a),
            continuation=dict(success=True,original_game_runs=0,reserved_original_starts_retained=12,
                previous_WP10_starts_retained=8,old_cases_current_runtime_credit=False))
        plan=dict(success=True,maximum_additional_original_games=112,previous_WP10_original_games=8,
            maximum_combined_original_games=120,source_continuation_review=continuation,
            preserved_negative_predecessors={'negative':dict(record='review',sha256='hash-review')})
        rows=[dict(case_id='passed',reserved_games=2,state='admitted'),dict(case_id='negative',report='acquisition',reserved_games=2,state='acquired',acquisition_sha256='a-hash')]
        return Reader(objects),plan,rows
    def test_exact_negative_retains_charge_and_state(self):
        r,p,rows=self.inputs();before=copy.deepcopy(rows)
        self.assertEqual(cumulative_bound(r,p),112);self.assertEqual(reviewed_predecessors(r,rows,p),4);self.assertEqual(rows,before)
    def test_unreviewed_predecessor_stops(self):
        r,p,rows=self.inputs();p['preserved_negative_predecessors']={}
        with self.assertRaises(ValueError):reviewed_predecessors(r,rows,p)
    def test_acceptance_and_charge_refund_refuse(self):
        for key,value in [('case_accepted',True),('reserved_original_starts_retained',0),('original_game_runs',2)]:
            r,p,rows=self.inputs();r.objects['review'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):reviewed_predecessors(r,rows,p)
    def test_wrong_execution_or_source_phase_refuses(self):
        for key in ('acquisition','source_continuation_review'):
            r,p,rows=self.inputs();r.objects['review'][key]=dict(record='other',sha256='other')
            with self.subTest(key=key),self.assertRaises(ValueError):reviewed_predecessors(r,rows,p)
    def test_running_or_successful_comparison_refuses(self):
        for key in ('success','comparison_processes_stopped'):
            r,p,rows=self.inputs();r.objects['failed'][key]=not r.objects['failed'][key]
            with self.subTest(key=key),self.assertRaises(ValueError):reviewed_predecessors(r,rows,p)
    def test_unknown_start_or_source_change_refuses(self):
        for kind in ('unknown-start','source-change','comparison-identity'):
            r,p,rows=self.inputs()
            if kind=='unknown-start':rows[1]['state']='requested'
            if kind=='source-change':r.objects['review']['reviewer_sources']['reviewer']='old'
            if kind=='comparison-identity':r.objects['review']['failed_comparison']['sha256']='old'
            with self.subTest(kind=kind),self.assertRaises(ValueError):reviewed_predecessors(r,rows,p)
    def test_cap_change_and_old_credit_refuse(self):
        for kind in ('cap','combined','refund','old-credit'):
            r,p,rows=self.inputs()
            if kind=='cap':p['maximum_additional_original_games']=113
            if kind=='combined':p['maximum_combined_original_games']=112
            if kind=='refund':r.objects['continuation']['reserved_original_starts_retained']=0
            if kind=='old-credit':r.objects['continuation']['old_cases_current_runtime_credit']=True
            with self.subTest(kind=kind),self.assertRaises(ValueError):cumulative_bound(r,p)
    def test_original96_basis_is_preserved(self):
        r,p,rows=self.inputs();p['maximum_additional_original_games']=96
        self.assertEqual(cumulative_bound(r,p),96)

if __name__=='__main__':unittest.main()
