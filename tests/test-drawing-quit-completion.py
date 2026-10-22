#!/usr/bin/env python3
from pathlib import Path
import sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_quit_completion import wait_for_quit
class Process:
    def __init__(self):self.code=None
    def poll(self):return self.code
class Quit(unittest.TestCase):
    def test_untyped_bounds_and_fabricated_exit_identities_refuse(self):
        for request,timeout in ((True,1),(1.5,1),(2**32,1),(1,True),(1,float('nan')),(1,float('inf'))):
            with self.assertRaises(ValueError):wait_for_quit(Process(),lambda:None,lambda:None,request,timeout=timeout)
        for key,value in [('thread',True),('process',-1),('attached_alive',1),('exit_code',False),('exit_code',1.5)]:
            child=dict(complete=True,attached_alive=True,thread=44,process=88,exit_code=0);child[key]=value
            with self.assertRaises(ValueError):wait_for_quit(Process(),lambda:None,lambda:None,11,target_exit=lambda:child)
    def test_exit_before_ack(self):
        p=Process();x=wait_for_quit(p,lambda:None,lambda:setattr(p,'code',0),11);self.assertFalse(x['ack_observed']);self.assertEqual(x['exit_code'],0)
    def test_ack_then_exit(self):
        p=Process();row=[0,3,0,11,1];x=wait_for_quit(p,lambda:(row,),lambda:setattr(p,'code',0),11);self.assertTrue(x['ack_observed'])
        self.assertEqual(x['launcher_exit_code'],0);self.assertFalse(x['original_child_exit_code_verified'])
    def test_already_exited_zero_cannot_prove_quit(self):
        p=Process();p.code=0
        with self.assertRaisesRegex(ValueError,'already exited'):wait_for_quit(p,lambda:None,lambda:None,11)
    def test_fast_exit_after_live_dispatch_retains_launcher_scope(self):
        p=Process();p.code=0;x=wait_for_quit(p,lambda:None,lambda:None,11,alive_before_dispatch=True)
        self.assertFalse(x['ack_observed']);self.assertFalse(x['original_child_exit_code_verified'])
    def test_observed_game_exit_does_not_require_desktop_launcher_exit(self):
        p=Process();child={'complete':True,'attached_alive':True,'thread':44,'process':88,'exit_code':0}
        x=wait_for_quit(p,lambda:None,lambda:None,11,target_exit=lambda:child)
        self.assertEqual(x['exit_code'],0);self.assertIsNone(x['launcher_exit_code']);self.assertFalse(x['launcher_exit_observed']);self.assertTrue(x['original_child_exit_code_verified']);self.assertEqual(x['exit_boundary'],'observed_original_process')
    def test_original_process_nonzero_and_partial_exit_refuse(self):
        for child in [{'complete':True,'attached_alive':True,'thread':44,'process':88,'exit_code':37},{'complete':False,'attached_alive':True,'thread':44,'process':88,'exit_code':0}]:
            with self.assertRaises(ValueError):wait_for_quit(Process(),lambda:None,lambda:None,11,target_exit=lambda:child)
    def test_refused_failed_and_timeout(self):
        with self.assertRaises(ValueError):wait_for_quit(Process(),lambda:([0,3,0,11,2],),lambda:None,11)
        p=Process();p.code=99
        with self.assertRaises(ValueError):wait_for_quit(p,lambda:None,lambda:None,11)
        tick=[0]
        with self.assertRaises(ValueError):wait_for_quit(Process(),lambda:None,lambda:None,11,timeout=.1,monotonic=lambda:tick[0],sleep=lambda n:tick.__setitem__(0,tick[0]+n))
if __name__=='__main__':unittest.main()
