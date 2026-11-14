#!/usr/bin/env python3
"""Bounded source/time completion never selects observations by cursor success."""
from pathlib import Path
import copy,sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_cursor_completion import Completion

def row(n,point=(40,100)):
 return dict(ordinal=n,kind=1,tick=n*100,thread=7,frame=dict(generation=n,nonce='42',owner=7),
  cursor=dict(point=list(point),poll=n),camera=[2,26],mode=1,tracked=dict(slot=0,health=400,flag68=0),history=[400,271,0],position=[1,2,3])
class Tests(unittest.TestCase):
 def setUp(self):
  self.rows=[row(n)for n in range(1,8)];self.released=dict(frame_input_row=1,host_us='100',scaling=0);self.settled=dict(frame_input_row=4,host_us='400');self.paints=[dict(world=n,host_us=n*100,scaling=0)for n in range(2,8)]
 def call(self):
  c=Completion([(self.released['frame_input_row'],self.settled['frame_input_row'],self.released['host_us'],self.settled['host_us'])]);return c,c(self.rows,self.paints,self.released,self.settled)
 def test_existing_host_grace_preserved(self):
  c,(tail,shown)=self.call();self.assertEqual([r['ordinal']for r in tail],[5,6,7]);self.assertEqual(c.diagnostics[0]['host_grace_us'],500000)
 def test_no_cursor_result_filter(self):
  self.rows[5]['cursor']['point']=[999,999];c,(tail,_)=self.call();self.assertIn(self.rows[5],tail)
 def test_camera_change_ends_completion(self):
  self.rows[4]['camera']=[1,26];c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4]);self.assertEqual(c.diagnostics,[])
 def test_selection_change_ends_completion(self):
  self.rows[4]['tracked']['flag68']=1;c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_mode_change_ends_completion(self):
  self.rows[4]['mode']=0;c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_actor_change_ends_completion(self):
  self.rows[4]['tracked']['health']=399;c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_history_change_ends_completion(self):
  self.rows[4]['history']=[401,271,0];c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_position_change_ends_completion(self):
  self.rows[4]['position']=[2,2,3];c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_thread_change_ends_completion(self):
  self.rows[4]['thread']=8;c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_nonce_change_ends_completion(self):
  self.rows[4]['frame']['nonce']='43';c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_callback_ends_completion(self):
  self.rows[4]['kind']=2;c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_source_time_limit(self):
  self.rows[4]['tick']=901;c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_host_time_limit(self):
  for p in self.paints:
   if p['world']>4:p['host_us']=500401
  c,(tail,_)=self.call();self.assertEqual([r['ordinal']for r in tail],[2,3,4])
 def test_scaling_filter_still_required(self):
  for p in self.paints:p['scaling']=2
  with self.assertRaises(ValueError):self.call()
 def test_three_distinct_generations_still_required(self):
  self.paints=self.paints[:2]
  with self.assertRaises(ValueError):self.call()
 def test_no_input_mutation(self):
  before=copy.deepcopy((self.rows,self.paints,self.released,self.settled));self.call();self.assertEqual((self.rows,self.paints,self.released,self.settled),before)
 def test_undeclared_interval_is_original(self):
  c=Completion([]);tail,_=c(self.rows,self.paints,self.released,self.settled);self.assertEqual([r['ordinal']for r in tail],[2,3,4]);self.assertEqual(c.diagnostics,[])
 def test_complete_eight_away_rotation_schedule(self):
  from broader_cursor_completion import away_rotation_intervals
  marks=[]
  for mode in('sharp','integer'):
   for view in range(4):
    for name in(mode+'-view-'+str(view)+'-away-2',mode+'-rotate-'+str(view)):
     for phase in('begin','released','settled'):marks.append(dict(action=name,phase=phase,frame_input_row=len(marks)+1,host_us=str(len(marks)+1)))
  self.assertEqual(len(away_rotation_intervals(marks)),8)
  marks[3]['action']='foreign'
  with self.assertRaises(ValueError):away_rotation_intervals(marks)
 def test_partial_schedule_refuses(self):
  from broader_cursor_completion import away_rotation_intervals
  with self.assertRaises(ValueError):away_rotation_intervals([])
if __name__=='__main__':unittest.main()
