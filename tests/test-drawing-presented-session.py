#!/usr/bin/env python3
"""Admission boundaries for source-owned, physically presented native sessions."""
import copy
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_presented_session import admit, sha, rgba565, Unsupported


def fixture(root):
    producer=b'owned producer; protocol decoded only by the native child'
    state=dict(schema=1,kind='mnm-drawing-history-v1',session='18446744073709551615',assets={},
        segments=[dict(stage='menu',producer='producer',frames=[dict(frame_index=0,
            boundary=dict(sequence=2,queues=0,canvas=7,width=8,height=6))])])
    session=dict(schema=1,session_identity='synthetic-session',inputs=[],
        actions=[dict(index=0,action='original-action-identity',before_presentation=12)],
        frames=[dict(identity=dict(session='synthetic-session',presentation_index=12,width=8,height=6,
                                  pixel_format='rgb565',actions_completed=1),input_ids=['state','producer'])],effects=[])
    def write(config=None,plan=None):
        config=copy.deepcopy(config or session);plan=copy.deepcopy(plan or state)
        state_bytes=json.dumps(plan).encode();(root/'state.json').write_bytes(state_bytes)
        (root/'producer.bin').write_bytes(producer)
        config['inputs']=[dict(id='state',path='state.json',role='owned-state',sha256=sha(state_bytes)),
                          dict(id='producer',path='producer.bin',role='operation-input',sha256=sha(producer))]
        (root/'session.json').write_text(json.dumps(config));return config,plan
    write();return session,state,write


class Admission(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.root=Path(self.temp.name)
        self.session,self.state,self.write=fixture(self.root)
    def tearDown(self):self.temp.cleanup()
    def test_complete_binding_and_exact_u64(self):
        s,p,_,_,segments,mapped=admit(self.root)
        self.assertEqual(p['session'],'18446744073709551615')
        self.assertEqual(mapped,[(1,self.state['segments'][0]['frames'][0]['boundary'])])
        self.assertEqual(s['frames'],self.session['frames']);self.assertEqual(segments[0]['stage'],'menu')
    def test_original_results_are_not_admitted_as_effects(self):
        self.session['effects']=[dict(id='draw',operation='flags-and-return-values',presentation_index=12)]
        self.write(self.session)
        with self.assertRaises(Unsupported):admit(self.root)
    def test_original_oracle_fields_are_refused(self):
        for object_name in ('session','frame','identity','action','state','segment','binding'):
            s,p=copy.deepcopy(self.session),copy.deepcopy(self.state)
            target={'session':s,'frame':s['frames'][0],'identity':s['frames'][0]['identity'],
                    'action':s['actions'][0],'state':p,'segment':p['segments'][0]}.get(object_name)
            if target is not None:target['original_oracle_sha256']='a'*64
            config,_=self.write(s,p)
            if object_name=='binding':
                config['inputs'][0]['original_oracle_sha256']='a'*64
                (self.root/'session.json').write_text(json.dumps(config))
            with self.subTest(object_name=object_name),self.assertRaises(ValueError):admit(self.root)
    def test_typed_identities(self):
        for key in ('presentation_index','width','height','actions_completed'):
            for bad in (True,False,1.5,-1,'1'):
                s=copy.deepcopy(self.session);s['frames'][0]['identity'][key]=bad;self.write(s)
                with self.subTest(key=key,bad=bad),self.assertRaises(ValueError):admit(self.root)
    def test_missing_duplicate_or_extra_frame_dependency(self):
        for ids in ([],['state'],['state','producer','producer'],['state','producer','oracle']):
            s=copy.deepcopy(self.session);s['frames'][0]['input_ids']=ids;self.write(s)
            with self.subTest(ids=ids),self.assertRaises(ValueError):admit(self.root)
    def test_reordered_source_identity(self):
        self.state['segments'][0]['frames'][0]['frame_index']=1;self.write(plan=self.state)
        with self.assertRaises(ValueError):admit(self.root)
    def test_exact_extent_and_queue_limit(self):
        for key,bad in (('width',9),('height',True),('queues',17),('sequence',0)):
            p=copy.deepcopy(self.state);p['segments'][0]['frames'][0]['boundary'][key]=bad;self.write(plan=p)
            with self.subTest(key=key),self.assertRaises(ValueError):admit(self.root)
    def test_action_count_and_boundary(self):
        for bad in (11,13,True):
            s=copy.deepcopy(self.session);s['actions'][0]['before_presentation']=bad;self.write(s)
            with self.subTest(bad=bad),self.assertRaises(ValueError):admit(self.root)
        s=copy.deepcopy(self.session);s['frames'][0]['identity']['actions_completed']=0;self.write(s)
        with self.assertRaises(ValueError):admit(self.root)
    def test_unknown_state_and_stage_are_unsupported(self):
        p=copy.deepcopy(self.state);p['kind']='original-pixels';self.write(plan=p)
        with self.assertRaises(Unsupported):admit(self.root)
        p=copy.deepcopy(self.state);p['segments'][0]['stage']='movie';self.write(plan=p)
        with self.assertRaises(Unsupported):admit(self.root)
    def test_state_session_range(self):
        for bad in ('0','01',str(2**64),1,True):
            p=copy.deepcopy(self.state);p['session']=bad;self.write(plan=p)
            with self.subTest(bad=bad),self.assertRaises(ValueError):admit(self.root)
    def test_symlink_and_parent_paths(self):
        config,_=self.write();(self.root/'alias.bin').symlink_to('producer.bin')
        for bad in ('alias.bin','../producer.bin','/producer.bin','a\\b'):
            s=copy.deepcopy(config);s['inputs'][1]['path']=bad;(self.root/'session.json').write_text(json.dumps(s))
            with self.subTest(path=bad),self.assertRaises(ValueError):admit(self.root)
    def test_changed_hash_duplicate_paths_and_unused_asset(self):
        config,_=self.write();config['inputs'][1]['sha256']='a'*64
        (self.root/'session.json').write_text(json.dumps(config))
        with self.assertRaises(ValueError):admit(self.root)
        config,_=self.write();config['inputs'][1]['path']='state.json'
        (self.root/'session.json').write_text(json.dumps(config))
        with self.assertRaises(ValueError):admit(self.root)
        config,_=self.write();(self.root/'unused').write_bytes(b'encoded')
        config['inputs'].append(dict(id='unused',path='unused',role='encoded-asset',sha256=sha(b'encoded')))
        (self.root/'session.json').write_text(json.dumps(config))
        with self.assertRaises(ValueError):admit(self.root)
    def test_rgb565_presentation_definition(self):
        self.assertEqual(rgba565(struct.pack('<4H',0,0xffff,0xf800,0xabcd)),
                         bytes([0,0,0,255,255,255,255,255,255,0,0,255,173,121,107,255]))
    def test_normalized_rgb565_display_policy(self):
        self.assertEqual(rgba565(struct.pack('<H',0x18e3),'normalized-nearest'),bytes([25,28,25,255]))
        with self.assertRaises(ValueError):rgba565(b'','unknown')
    def test_native_asset_aliases_and_transport_collisions(self):
        for names in (['Interface\\Image.bmp'], ['Interface\\Image.bmp','interface/image.bmp'],
                      ['drawing-session.json'],['producer-0001.bin'],['../image.bmp'],['C:/image.bmp']):
            p=copy.deepcopy(self.state);p['assets']={name:'asset' for name in names}
            s=copy.deepcopy(self.session);s['frames'][0]['input_ids'].append('asset')
            config,_=self.write(s,p);(self.root/'asset.bin').write_bytes(b'encoded')
            config['inputs'].append(dict(id='asset',path='asset.bin',role='encoded-asset',sha256=sha(b'encoded')))
            (self.root/'session.json').write_text(json.dumps(config))
            with self.subTest(names=names):
                if len(names)==1 and names[0].startswith('Interface'):admit(self.root)
                else:
                    with self.assertRaises(ValueError):admit(self.root)

if __name__=='__main__':unittest.main()
