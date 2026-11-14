import copy,sys,types,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_pan_ack import acknowledged,perform
class Clock:
 def __init__(self):self.now=0
 def monotonic(self):return self.now
 def monotonic_ns(self):return round(self.now*1e9)
 def sleep(self,seconds):self.now+=seconds
class Journey:
 def __init__(self,clock,change=.35):
  self.clock=clock;self.change=change;self.pan_receipts=[];self.actions=[];self.events=[];self.marks=[];self.progress=[];self.error=False;self.release_ok=True
  self.before=dict(kind=1,ordinal=1,thread=7,frame=dict(generation=5,nonce='123'),camera=[0,23,55,6,0,0,0]);self.after=copy.deepcopy(self.before);self.after.update(ordinal=2,frame=dict(generation=6,nonce='123'),camera=[0,22,54,6,0,0,0])
  self.input=types.SimpleNamespace(display=1,x=types.SimpleNamespace(XKeysymToKeycode=lambda *_:111,XFlush=lambda *_:None),xt=types.SimpleNamespace(XTestFakeKeyEvent=self.event),focus=lambda:dict(host_focused=True))
 def event(self,display,code,down,delay):self.events.append((down,self.clock.now));return 1 if down else int(self.release_ok)
 def latest(self):
  if self.error and self.clock.now>=.1:raise ValueError('retained decode refusal')
  return self.after if self.clock.now>=self.change else self.before
 def mark(self,action,phase):self.marks.append(phase);return dict(world='6'if self.clock.now>=self.change else'5')
 def wait_progress(self,after,minimum):self.progress.append((after,minimum,self.clock.now))
class Controls(unittest.TestCase):
 def setUp(self):self.clock=Clock();self.j=Journey(self.clock)
 def run_pan(self,**kw):
  args=dict(action='high-initial-integer-pan',symbol=0xff52,hold=.22,settle=.8,advance=True);args.update(kw);return perform(self.j,clock=self.clock,**args)
 def test_delayed_source_ack_fits_old_total_budget(self):
  self.run_pan();self.assertGreaterEqual(self.j.events[1][1],.35);self.assertLessEqual(self.j.events[1][1],1.02);self.assertAlmostEqual(self.clock.now,1.02);self.assertEqual(self.j.progress[0][:2],(6,5));self.assertEqual(self.j.marks,['begin','released','settled'])
 def test_old_short_pulse_has_no_source_change(self):
  self.clock.now=.22;self.assertEqual(self.j.latest(),self.j.before);self.clock.now=0;self.run_pan();self.assertIsNotNone(self.j.pan_receipts[0]['acknowledged_after'])
 def test_early_ack_keeps_minimum_hold(self):
  self.j.change=.05;self.run_pan();self.assertGreaterEqual(self.j.events[1][1],.22);self.assertAlmostEqual(self.clock.now,1.02)
 def test_no_ack_refuses_and_releases(self):
  self.j.change=2
  with self.assertRaisesRegex(ValueError,'existing1.02'):self.run_pan()
  self.assertEqual([v[0]for v in self.j.events],[1,0]);self.assertFalse(self.j.progress);self.assertIsNone(self.j.pan_receipts[0]['acknowledged_after'])
 def test_after_deadline_ack_never_accepted(self):
  self.j.change=1.03
  with self.assertRaises(ValueError):self.run_pan()
 def test_latest_decode_failure_still_releases(self):
  self.j.error=True
  with self.assertRaisesRegex(ValueError,'decode'):self.run_pan()
  self.assertEqual([v[0]for v in self.j.events],[1,0]);self.assertEqual(len(self.j.pan_receipts),1)
 def test_release_failure_is_retained(self):
  self.j.release_ok=False
  with self.assertRaisesRegex(ValueError,'release refused'):self.run_pan()
  self.assertFalse(self.j.pan_receipts[0]['physical_release_accepted'])
 def test_view_change_is_not_pan_ack(self):self.j.after['camera'][0]=1;self.assertFalse(acknowledged(self.j.before,self.j.after))
 def test_same_camera_is_not_pan_ack(self):self.j.after['camera']=self.j.before['camera'];self.assertFalse(acknowledged(self.j.before,self.j.after))
 def test_only_height_change_is_not_pan_ack(self):self.j.after['camera']=[0,23,55,7,0,0,0];self.assertFalse(acknowledged(self.j.before,self.j.after))
 def test_stale_generation_is_not_ack(self):self.j.after['frame']['generation']=5;self.assertFalse(acknowledged(self.j.before,self.j.after))
 def test_other_nonce_is_not_ack(self):self.j.after['frame']['nonce']='456';self.assertFalse(acknowledged(self.j.before,self.j.after))
 def test_other_thread_is_not_ack(self):self.j.after['thread']=8;self.assertFalse(acknowledged(self.j.before,self.j.after))
 def test_callback_row_is_not_World_ack(self):self.j.after['kind']=3;self.assertFalse(acknowledged(self.j.before,self.j.after))
 def test_wrong_action_is_refused_before_input(self):
  with self.assertRaises(ValueError):self.run_pan(action='other-pan')
  self.assertFalse(self.j.events)
 def test_changed_hold_budget_refused(self):
  with self.assertRaises(ValueError):self.run_pan(hold=.3)
  self.assertFalse(self.j.events)
 def test_changed_settle_budget_refused(self):
  with self.assertRaises(ValueError):self.run_pan(settle=1)
 def test_wrong_key_refused(self):
  with self.assertRaises(ValueError):self.run_pan(symbol=0xff54)
if __name__=='__main__':unittest.main()
