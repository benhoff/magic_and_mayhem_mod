#!/usr/bin/env python3
import copy,ctypes as c,hashlib,struct,sys,tempfile,unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock,patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_interaction_recent_sources import RecentSources,fresh_target,POLICY
from broader_interaction_recent_launch import recent_binding

def state(g=1):
    return dict(ordinal=g,thread=312,frame=dict(generation=g,nonce='42'),history=[10,20,3],position=[1,2,3],camera=[0]*7,mode=0,tracked=dict(slot=0,type=0,owner=0,active=1,health=10))

class Fresh(unittest.TestCase):
    def run_target(self,states):
        journey=SimpleNamespace(child=SimpleNamespace(poll=lambda:None),latest=Mock(side_effect=states),source_frames=SimpleNamespace(target=Mock(return_value=dict(point=[11,22]))))
        return journey,fresh_target(journey)
    def test_exact_current_source(self):
        j,(s,t)=self.run_target([state(1),state(1)]);self.assertEqual(t['recent_source_check']['age_frames'],0);j.source_frames.target.assert_called_once_with(1,[10,20,3])
    def test_two_frames_permitted_same_source_pose(self):
        j,(s,t)=self.run_target([state(1),state(3)]);self.assertEqual(t['recent_source_check']['age_frames'],2)
    def test_stale_source_is_replaced_before_pointer(self):
        j,(s,t)=self.run_target([state(1),state(57),state(57),state(58)]);self.assertEqual(s['frame']['generation'],57);self.assertEqual(j.source_frames.target.call_count,2)
    def test_changed_source_fields_are_refreshed(self):
        for key,value in [('history',[11,20,3]),('position',[2,2,3]),('camera',[1]*7),('mode',1)]:
            a=state(1);b=state(2);b[key]=value;j,(s,t)=self.run_target([a,b,b,b]);self.assertEqual(s,b)
    def test_hit_feedback_never_selects_target(self):
        a=state();b=state();a['hit']={'slot':99};b['hit']=None;j,(s,t)=self.run_target([a,b]);self.assertEqual(t['point'],[11,22]);self.assertFalse(t['recent_source_check']['hit_feedback_used'])
    def test_nonce_change_refuses(self):
        b=state(2);b['frame']['nonce']='43'
        with self.assertRaises(ValueError):self.run_target([state(),b])
    def test_generation_reversal_refuses(self):
        with self.assertRaises(ValueError):self.run_target([state(2),state(1)])
    def test_thread_change_refuses(self):
        b=state();b['thread']=313
        with self.assertRaises(ValueError):self.run_target([state(),b])
    def test_dead_wizard_refuses(self):
        a=state();a['tracked']['health']=0
        with self.assertRaises(ValueError):self.run_target([a])
    def test_wrong_owner_refuses(self):
        a=state();a['tracked']['owner']=1
        with self.assertRaises(ValueError):self.run_target([a])
    def test_original_deadline_is_preserved(self):
        j=SimpleNamespace(child=SimpleNamespace(poll=lambda:None),latest=Mock(),source_frames=Mock())
        with patch('time.monotonic',side_effect=[0,16]):
            with self.assertRaisesRegex(ValueError,'readiness deadline'):fresh_target(j)
        j.latest.assert_not_called()

class Stream(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.path=Path(self.temp.name)/'worlds.bin'
        self.api=c.CDLL('libzstd.so.1');self.api.ZSTD_compress.argtypes=[c.c_void_p,c.c_size_t,c.c_void_p,c.c_size_t,c.c_int];self.api.ZSTD_compress.restype=c.c_size_t
    def record(self,g,ordinal,chunk=0):
        body=bytes([g])*(64 if chunk else 128);out=c.create_string_buffer(1024);n=self.api.ZSTD_compress(out,1024,body,len(body),1);header=struct.pack('<QQIIIIQQ',g,chunk,64,0 if chunk else 64,n,int(ordinal!=1),ordinal,0 if ordinal==1 else ordinal-1)
        return header+hashlib.sha256(body).digest()+out.raw[:n]
    def reader(self,data):
        self.path.write_bytes(b'MNMPROF4'+data);s=RecentSources(self.path);s.module=SimpleNamespace(decode=Mock(return_value=(None,[])));self.addCleanup(s.close);return s
    def test_only_selected_World_semantics_materialized(self):
        s=self.reader(self.record(1,1)+self.record(2,2)+self.record(3,3));s.poll(3);self.assertEqual(s.module.decode.call_count,1);self.assertEqual(s.decoder.count,3);self.assertEqual(set(s.frames),{3})
    def test_source_chunks_not_counted_as_Worlds(self):
        s=self.reader(self.record(1,1,1)+self.record(1,2));s.poll(1);self.assertEqual((s.worlds,s.chunks,s.decoder.count),(1,1,2));self.assertEqual(s.module.decode.call_count,1)
    def test_corrupt_hash_on_skipped_World_refuses(self):
        data=bytearray(self.record(1,1));data[48]^=1;s=self.reader(data+self.record(2,2))
        with self.assertRaises(ValueError):s.poll(2)
    def test_bad_prefix_ordinal_on_skipped_World_refuses(self):
        s=self.reader(self.record(1,2)+self.record(2,3))
        with self.assertRaises(ValueError):s.poll(2)
    def test_noncontiguous_World_refuses(self):
        s=self.reader(self.record(2,1))
        with self.assertRaises(ValueError):s.poll(2)
    def test_noncontiguous_chunk_refuses(self):
        s=self.reader(self.record(1,1,2))
        with self.assertRaises(ValueError):s.poll(1)
    def test_partial_tail_waits_without_consuming(self):
        data=self.record(1,1);s=self.reader(data[:-1]);s.poll(1);self.assertEqual((s.at,s.worlds,s.decoder.count),(8,0,0));self.path.write_bytes(b'MNMPROF4'+data);s.poll(1);self.assertEqual(s.worlds,1)
    def test_backward_request_refuses(self):
        s=self.reader(self.record(1,1)+self.record(2,2));s.poll(2)
        with self.assertRaises(ValueError):s.poll(1)
    def test_repeated_request_reuses_verified_cache(self):
        s=self.reader(self.record(1,1));s.poll(1);s.poll(1);self.assertEqual(s.module.decode.call_count,1)
    def test_overflow_refuses(self):
        s=self.reader(b'')
        with self.assertRaises(ValueError):s.poll(8193)

class Binding(unittest.TestCase):
    def base(self):
        class Base:
            def __init__(self):self.plan=dict(family='interaction',recent_interaction_sources=dict(policy=POLICY))
            def patch(self,m):return m
            def receipt(self):return {}
        return Base
    def test_only_exact_controller_is_changed_once(self):
        b=recent_binding(self.base())();m=SimpleNamespace(__file__='run-rendering-interaction.py',InteractionJourney=type('Original',(),{}));b.patch(m);selected=m.InteractionJourney;b.patch(m);self.assertIs(m.InteractionJourney,selected);self.assertIs(selected.ready_target,fresh_target);self.assertFalse(b.receipt()['recent_interaction_sources']['action_schedule_changed'])
    def test_unrelated_module_is_preserved(self):
        b=recent_binding(self.base())();m=SimpleNamespace(__file__='unrelated.py');self.assertIs(b.patch(m),m)
        with self.assertRaises(ValueError):b.receipt()

if __name__=='__main__':unittest.main()
