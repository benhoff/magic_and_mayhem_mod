from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_campaign_guidance import open_mini


class Guidance(unittest.TestCase):
    def test_three_pages_require_actual_mini_before_success(self):
        actions=[];waits=[]
        def wait(screen,timeout):
            self.assertEqual((screen,timeout),(17,2));waits.append(screen)
            if len(waits)<4:raise ValueError('Original menu state timed out')
        r=open_mini(lambda name,action:actions.append((name,action)),wait)
        self.assertEqual(len(r['attempts']),4);self.assertEqual(sum(a['kind']=='click' for n,a in actions),3)
        self.assertTrue(r['mini_verified']);self.assertFalse(r['guidance_lifecycle_recovered'])

    def test_unknown_route_stops_at_budget_and_other_failures_propagate(self):
        actions=[]
        def timeout(*args,**kwargs):raise ValueError('Original menu state timed out')
        with self.assertRaisesRegex(ValueError,'budget exhausted'):open_mini(lambda n,a:actions.append(a),timeout)
        self.assertEqual(sum(a['kind']=='click' for a in actions),6)
        def exited(*args,**kwargs):raise ValueError('Original exited before required state')
        with self.assertRaisesRegex(ValueError,'Original exited'):open_mini(lambda *args:None,exited)

    def test_no_dialogue_needs_no_click_and_budget_cannot_grow(self):
        actions=[];r=open_mini(lambda n,a:actions.append(a),lambda *args,**kwargs:None)
        self.assertEqual(len(actions),1);self.assertTrue(r['mini_verified'])
        with self.assertRaises(ValueError):open_mini(None,None,7)


    def test_noninteger_page_budget_and_wrong_wait_state_refuse(self):
        for value in (True,False,1.5,'1',0):
            with self.assertRaises(ValueError):open_mini(None,None,value)
        for state in (False,(),((1,2,1),),((1,17,0),)):
            with self.assertRaises(ValueError):open_mini(lambda *a:None,lambda *a,**k:state)
        self.assertTrue(open_mini(lambda *a:None,lambda *a,**k:((1,17,1),))['mini_verified'])

    def test_failed_physical_action_does_not_continue(self):
        actions=[]
        def failed(name,action):actions.append(name);raise RuntimeError('input child failed')
        with self.assertRaises(RuntimeError):open_mini(failed,lambda *a,**k:None)
        self.assertEqual(len(actions),1)

if __name__=='__main__':unittest.main()
