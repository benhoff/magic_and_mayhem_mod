#!/usr/bin/env python3
from pathlib import Path
import sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_quit_completion import wait_for_quit
class Process:
    def __init__(self):self.code=None
    def poll(self):return self.code
class Quit(unittest.TestCase):
    def test_exit_before_ack(self):
        p=Process();x=wait_for_quit(p,lambda:None,lambda:setattr(p,'code',0),11);self.assertFalse(x['ack_observed']);self.assertEqual(x['exit_code'],0)
    def test_ack_then_exit(self):
        p=Process();row=[0,3,0,11,1];x=wait_for_quit(p,lambda:(row,),lambda:setattr(p,'code',0),11);self.assertTrue(x['ack_observed'])
    def test_refused_failed_and_timeout(self):
        with self.assertRaises(ValueError):wait_for_quit(Process(),lambda:([0,3,0,11,2],),lambda:None,11)
        p=Process();p.code=99
        with self.assertRaises(ValueError):wait_for_quit(p,lambda:None,lambda:None,11)
        tick=[0]
        with self.assertRaises(ValueError):wait_for_quit(Process(),lambda:None,lambda:None,11,timeout=.1,monotonic=lambda:tick[0],sleep=lambda n:tick.__setitem__(0,tick[0]+n))
if __name__=='__main__':unittest.main()
