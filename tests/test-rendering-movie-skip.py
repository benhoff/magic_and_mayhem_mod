#!/usr/bin/env python3
"""Actual public playback cancellation and focused-menu closeout guards."""
import copy,importlib.util,json,struct,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
s=importlib.util.spec_from_file_location('skip',ROOT/'tools/close-rendering-movie-skip.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)


class SkipTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.r=Reader(ROOT);cls.report=cls.r.json(m.PREFIX+'closeout-analysis-01.json');cls.parent=cls.r.json(cls.report['input_parent']['record']);cls.plan=cls.r.json('docs/native-rendering-closure.json')
  cls.a=cls.r.json(cls.report['executions'][0]['case']['record']);cls.expected=cls.report['expected_movies'];m.validate_artifacts(cls.r,cls.a,cls.expected)
  cls.media=json.loads(artifact(cls.r,cls.a,'native-media-report.json'));cls.policy=json.loads(artifact(cls.r,cls.a,'native-audio-launch-policy.json'));cls.raw=artifact(cls.r,cls.a,'native-media.bin');cls.final=m.snapshot(cls.raw);cls.callbacks=[[3,3],[10,1],[3,2],[22,3],[3,4]]
 @classmethod
 def tearDownClass(cls):cls.r.close()
 def assess(self,a=None,media=None,final=None,policy=None,expected=None,callbacks=None):
  return m.assess(a or self.a,media or self.media,final or self.final,policy or self.policy,expected or self.expected,callbacks or self.callbacks)
 def test_actual_live_skip_and_original_return_pass(self):self.assertTrue(self.assess()['verified'])
 def test_only_two_rows_close(self):
  self.assertEqual(m.close_rows(self.plan,self.parent,self.report['executions']),self.report['requirements']);self.assertEqual((self.report['passed_requirements'],self.report['pending_requirements']),(52,104))
 def test_all50parent_rows_and_decisions_preserved(self):
  for a,b in zip(self.parent['requirements'],self.report['requirements']):
   if a['id']==m.LEAF:self.assertEqual((a['state'],b['state']),('pending','passed'))
   else:self.assertEqual(a,b)
  self.assertEqual(self.parent['decisions'],self.report['decisions'])
 def test_four_real_domains_and_both_skip_paths(self):
  self.assertEqual(self.report['original_game_runs'],4);self.assertEqual({(v['configuration']['software_flag'],v['configuration']['high_resolution']) for v in self.report['executions']},{(s,h) for s in (False,True) for h in (False,True)})
 def test_missing_duplicate_or_unproved_domain_refuses(self):
  for kind in ('missing','duplicate','proof'):
   runs=copy.deepcopy(self.report['executions'])
   if kind=='missing':runs.pop()
   elif kind=='duplicate':runs[-1]=runs[0]
   else:runs[0]['proof']['verified']=False
   with self.subTest(kind=kind),self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,runs)
 def test_no_game_force_stop_or_surviving_owner_refuses(self):
  for kind in ('no-game','force','owner'):
   a=copy.deepcopy(self.a)
   if kind=='no-game':a['original_game_runs']=0
   elif kind=='force':a['forced_private_prefix_stop']=True
   else:a['retirement']['graphics_retirement']['counts']['surfaces']=1
   with self.subTest(kind=kind),self.assertRaises(ValueError):self.assess(a=a)
 def test_eof_not_skip_and_zero_delivery_refuse(self):
  for kind in ('eof','frames','hidden'):
   media=copy.deepcopy(self.media)
   if kind=='eof':media[1]['status']=media[1]['result']=3
   elif kind=='frames':media[1]['video_frames']=media[1]['presented_video_frames']=0
   else:media[1]['video_presentation_requested']=False
   with self.subTest(kind=kind),self.assertRaises(ValueError):self.assess(media=media)
 def test_audio_device_loss_or_legacy_policy_refuses(self):
  media=copy.deepcopy(self.media);media[1]['output_losses']=1
  with self.assertRaises(ValueError):self.assess(media=media)
  policy=copy.deepcopy(self.policy);policy['native_media']=False
  with self.assertRaises(ValueError):self.assess(policy=policy)
 def test_physical_escape_focus_and_order_are_required(self):
  for kind in ('key','focus','order'):
   a=copy.deepcopy(self.a);v=a['proof']['movie_skips'][0]
   if kind=='key':v['physical']['physical']['symbol']=0x20
   elif kind=='focus':v['physical']['physical']['host_focus']['host_focused']=False
   else:v['before']['host_ns']=v['after']['host_ns']
   with self.subTest(kind=kind),self.assertRaises(ValueError):self.assess(a=a)
 def test_wrong_request_timeout_cancel_and_ack_refuse(self):
  for kind in ('request','timeout','ack'):
   a=copy.deepcopy(self.a);v=a['proof']['movie_skips'][0]
   if kind=='request':v['decoded_before']['accepted']+=1
   elif kind=='timeout':v['decoded_after']['cancelled']=v['request']
   else:v['decoded_after']['status']=2
   with self.subTest(kind=kind),self.assertRaises(ValueError):self.assess(a=a)
 def test_duplicate_or_unattributed_cancellation_refuses(self):
  a=copy.deepcopy(self.a);a['proof']['movie_skips'].append(copy.deepcopy(a['proof']['movie_skips'][0]))
  with self.assertRaises(ValueError):self.assess(a=a)
 def test_changed_asset_and_wrong_original_preferences_refuse(self):
  a=copy.deepcopy(self.a);a['installation_inputs']['FMV/Intro0.avi']['sha256']='0'*64
  with self.assertRaises(ValueError):self.assess(a=a)
  a=copy.deepcopy(self.a);a['states'][1]['preferences'][2]^=1
  with self.assertRaises(ValueError):self.assess(a=a)
 def test_menu_ack_callback_and_world_mutations_refuse(self):
  a=copy.deepcopy(self.a);a['states'][3]['ack']+=1
  with self.assertRaises(ValueError):self.assess(a=a)
  with self.assertRaises(ValueError):self.assess(callbacks=self.callbacks[:-1])
  a=copy.deepcopy(self.a);a['resident']['imports']='1'
  with self.assertRaises(ValueError):self.assess(a=a)
 def test_snapshot_odd_sequence_or_changed_hash_refuses(self):
  raw=bytearray(self.raw);struct.pack_into('<I',raw,m.wire.RESPONSE_SEQUENCE_OFFSET,1)
  with self.assertRaises(ValueError):m.snapshot(bytes(raw))
  before=self.a['proof']['movie_skips'][0]['before'];bad=copy.deepcopy(before);bad['sha256']='0'*64
  with self.assertRaises(ValueError):m.snapshot(artifact(self.r,self.a,before['path']),bad)
 def test_visible_main_cannot_be_replaced_by_accessibility_alone(self):
  a=copy.deepcopy(self.a);v=next(v for v in a['screenshots'] if v['label']=='main-ready-initial');node=next(v for v in v['nodes'] if v['name']=='Quick Battle');node['rect']=[0,0,1,1]
  with self.assertRaises(ValueError):m.validate_artifacts(self.r,a,self.expected)
 def test_current_sources_and_no_whole_movie_or_fixture_claim(self):
  self.assertTrue(all(self.r.digest(n)==h for n,h in self.report['sources'].items()));self.assertFalse(self.report['movie_gpu_output_proved']);self.assertFalse(self.report['new_native_code']);self.assertFalse(self.report['new_game_fixture']);self.assertFalse(self.report['historical_failures_retired'])


if __name__=='__main__':unittest.main()
