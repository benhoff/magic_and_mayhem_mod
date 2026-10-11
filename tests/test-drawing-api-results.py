#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_results import compare_calls,compare_fault_traces
class ResultsTest(unittest.TestCase):
    def setUp(self):self.original=[dict(ordinal=1,entry=1,method=5,object=1,related=2,caller=0x58c05d,entry_error=9,flags=16,hresult=-2005532222,last_error=17)]
    def test_exact_caller_result(self):self.assertTrue(compare_calls(copy.deepcopy(self.original),self.original)['matching'])
    def test_pixel_equality_cannot_hide_wrong_hresult_or_error(self):
        for key in ('hresult','last_error'):
            a=copy.deepcopy(self.original);a[0][key]=0;self.assertFalse(compare_calls(a,self.original)['matching'])
    def test_unknown_result_refuses(self):
        for value in (None,True,'unknown'):
            a=copy.deepcopy(self.original);a[0]['hresult']=value
            with self.assertRaises(ValueError):compare_calls(a,self.original)
    def test_invalid_original_result_refuses(self):
        for key,value in [('hresult',2**31),('last_error',-1)]:
            original=copy.deepcopy(self.original);original[0][key]=value
            with self.assertRaises(ValueError):compare_calls(self.original,original)
    def test_reordered_or_missing_call_refuses(self):
        with self.assertRaises(ValueError):compare_calls([],self.original)
        for key in ('entry','method','object','related','flags'):
            a=copy.deepcopy(self.original);a[0][key]+=1
            with self.assertRaises(ValueError):compare_calls(a,self.original)
    def test_exhaustion_cannot_be_original_success(self):
        reference=[{'original':{'id':0,'draws_consumed':3,'events':[[1,2,1,16,1,0],[1,2,1,16,1,0],[1,2,0,16,1,0]]}}]
        actual=[dict(id=0,draws_consumed=2,events=reference[0]['original']['events'][:2],returned=False,budget_exhausted=True)]
        self.assertEqual(compare_fault_traces(actual,reference,2)['budget_exhaustions'],1)
        actual[0]['returned']=True
        with self.assertRaises(ValueError):compare_fault_traces(actual,reference,2)
    def test_wrong_restore_outcome_refuses(self):
        reference=[{'original':{'id':0,'draws_consumed':1,'events':[[3,2,0,0,0,0]]}}]
        actual=[dict(id=0,draws_consumed=1,events=[[3,2,0x80004005,0,0,0]],returned=True,budget_exhausted=False)]
        with self.assertRaises(ValueError):compare_fault_traces(actual,reference)
    def test_boolean_fault_identity_or_count_refuses(self):
        reference=[{'original':{'id':1,'draws_consumed':1,'events':[]}}]
        for key in ('id','draws_consumed'):
            actual=[dict(id=1,draws_consumed=1,events=[],returned=True,budget_exhausted=False)]
            actual[0][key]=True
            with self.assertRaises(ValueError):compare_fault_traces(actual,reference)
if __name__=='__main__':unittest.main()
