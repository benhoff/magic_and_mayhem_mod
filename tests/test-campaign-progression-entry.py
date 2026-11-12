#!/usr/bin/env python3
import copy
import hashlib
import importlib.util
from pathlib import Path
import struct
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from campaign_progression_entry import input_reference,validate_entry_flow
from rendering_progression import validate_flow,validate_callbacks
spec=importlib.util.spec_from_file_location('progression_controls',ROOT/'tests/test-rendering-progression.py');fixtures=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixtures)


def loaded_flow():
    q=fixtures.flow('victory');q.update(entry_mode='original-checkpoint',entry_load_verified=True,entry_checkpoint_name='checkpoint-proof-01',entry_checkpoint_sha256='a'*64)
    q['states'][1].update(event='checkpoint-entry',screen=12,save_name='checkpoint-proof-01',save_count=1,parent=3,depth=3)
    q['states'][1:1]=[dict(event='display-configuration',screen=10,sequence=3,ack=0,thread=40),dict(event='configured-main',screen=3,sequence=4,ack=1,thread=40)]
    q['worlds'][0]['entry_mode']='original-checkpoint';return q


class Entry(unittest.TestCase):
    def test_initial_load_still_requires_natural_victory_and_real_region2(self):
        q=loaded_flow();self.assertTrue(validate_entry_flow(q)['verified']);self.assertTrue(validate_flow(q,'victory')['later_map_entered'])
        for changed in ('outcome','same-map','quit','no-second-world'):
            bad=copy.deepcopy(q)
            if changed=='outcome':next(r for r in bad['states'] if r['event']=='natural-report')['outcome']=0
            elif changed=='same-map':bad['worlds'][1]['region']=1
            elif changed=='quit':next(r for r in bad['states'] if r['event']=='natural-report')['texts'][0]='Quit the battle'
            else:bad['worlds'].pop()
            with self.assertRaises(ValueError):validate_flow(bad,'victory')

    def test_unbound_or_duplicate_load_inputs_are_refused(self):
        for key,value in (('entry_load_verified',False),('entry_checkpoint_name','other'),('entry_checkpoint_sha256','bad')):
            q=loaded_flow();q[key]=value
            with self.assertRaises(ValueError):validate_entry_flow(q)
        for change in (dict(save_count=2),dict(parent=2),dict(depth=4),dict(save_name='checkpoint-other')):
            q=loaded_flow();q['states'][3].update(change)
            with self.assertRaises(ValueError):validate_entry_flow(q)
        q=loaded_flow();q['states'].append(q['states'][3].copy())
        with self.assertRaises(ValueError):validate_entry_flow(q)

    def test_original_load_entry_return_and_success_must_precede_natural_transition(self):
        q=loaded_flow();raw=fixtures.trace();old=list(struct.iter_unpack('<16I',raw[16:]));new=[]
        for event in (2,3,4):
            r=[0]*16;r[1]=event;r[2]=40;r[3]=12;r[9]=0;r[8]=int(event==4);new.append(r)
        for r in old:new.append(list(r))
        for i,r in enumerate(new,1):r[0]=i
        trace=raw[:16]+b''.join(struct.pack('<16I',*r) for r in new)
        self.assertIsNotNone(validate_callbacks(trace,q)['initial_original_load'])
        # Successful Load state is also recorded while queued/active flags
        # change. This is one load callback, not three independent loads.
        repeated=[r.copy() for r in new];repeated.insert(3,repeated[2].copy());repeated.insert(4,repeated[2].copy())
        for i,r in enumerate(repeated,1):r[0]=i
        repeated_trace=raw[:16]+b''.join(struct.pack('<16I',*r) for r in repeated)
        self.assertEqual(len(validate_callbacks(repeated_trace,q)['initial_original_load']['success_snapshots']),3)
        repeated[4][2]=41
        with self.assertRaises(ValueError):validate_callbacks(raw[:16]+b''.join(struct.pack('<16I',*r) for r in repeated),q)
        new[2][8]=0
        wrong=raw[:16]+b''.join(struct.pack('<16I',*r) for r in new)
        with self.assertRaises(ValueError):validate_callbacks(wrong,q)
        with self.assertRaises(ValueError):validate_callbacks(raw,q)

    def test_original_save_input_is_exact_and_unique(self):
        raw=b'unchanged-original-save-fixture';sha=hashlib.sha256(raw).hexdigest()
        q=dict(kind='public_native_campaign_lightweight_diagnostic',scenario='checkpoint-capture',configuration=dict(checkpoint_mode='capture'),sources_stable=True,flow=dict(saved_name='checkpoint-proof-01'),success=True,private_wine_stopped=True,artifact_retention_complete=True,original_manifest_verified_before_after=True,checkpoint=dict(verified=True,save_sha256=sha,save_bytes=len(raw),saved_snapshot=dict(screen=13)),artifacts=[dict(path='working/run/game/Save/checkpoint-proof-01.sav',sha256=sha,record='save')])
        class Reader:
            def json(self,name):return q
            def digest(self,name):return 'b'*64
            def read_bytes(self,name):return raw
        self.assertEqual(input_reference(Reader(),'capture','checkpoint-proof-01')[0],raw)
        q['artifacts'].append(q['artifacts'][0].copy())
        with self.assertRaises(ValueError):input_reference(Reader(),'capture','checkpoint-proof-01')
        q['artifacts'].pop();q['checkpoint']['save_sha256']='c'*64
        with self.assertRaises(ValueError):input_reference(Reader(),'capture','checkpoint-proof-01')
        q['checkpoint']['save_sha256']=sha;q['checkpoint']['domain']='unknown'
        with self.assertRaises(ValueError):input_reference(Reader(),'capture','checkpoint-proof-01')
        q['checkpoint'].update(domain='transition-ready',portal_goal=dict(slot=4,type=24,owner=1,active=0,health=0,x=42,y=26,z=4))
        with self.assertRaises(ValueError):input_reference(Reader(),'capture','checkpoint-proof-01')
        q['configuration']['checkpoint_route']='transition-ready'
        self.assertEqual(input_reference(Reader(),'capture','checkpoint-proof-01')[1]['domain'],'transition-ready')
        q['checkpoint']['portal_goal']['active']=1
        with self.assertRaises(ValueError):input_reference(Reader(),'capture','checkpoint-proof-01')


if __name__=='__main__':unittest.main()
