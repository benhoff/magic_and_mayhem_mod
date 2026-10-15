#!/usr/bin/env python3
"""Save/Load evidence rejects cached paints and missing original resumptions."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('storage', ROOT/'tools/campaign_storage.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class StorageTests(unittest.TestCase):
    def flow(self):
        return dict(success=True, save_load=True, stage=12, normal_quit=True,
                    resident_world_requested=True, native_command_fallback=False,
                    native_recoveries=0, native_error='', native_command_frames=120,
                    checkpoints=[dict(phase=p, source_generation=str(100+i*100),
                                      source_base=str(i*100), frames=100+i*100,
                                      frame_base=i*100, native_active=True,
                                      fallback=False, error='') for i, p in enumerate(
                                          ['before-save', 'after-save', 'after-load'])])

    def test_fresh_worlds(self):
        self.assertEqual(len(m.validate_storage_flow(self.flow())), 3)

    def test_cached_frame_and_mode_rejected(self):
        for key, value in [('source_generation', '100'), ('native_active', False), ('fallback', True)]:
            flow = self.flow()
            flow['checkpoints'][2][key] = value
            with self.assertRaises(ValueError): m.validate_storage_flow(flow)
        flow = self.flow(); flow['resident_world_requested'] = False
        with self.assertRaises(ValueError): m.validate_storage_flow(flow)

    def fixture(self, root, difficulty=1, recreate=False):
        flow = self.flow(); flow['saved_name'] = 'native-render-123456abcdef'
        saved = root/'game/Save'/ (flow['saved_name']+'.sav')
        saved.parent.mkdir(parents=True); saved.write_bytes(b'original serialized fixture')
        flow['saved_sha256'] = m.hashlib.sha256(saved.read_bytes()).hexdigest()
        owned = root/'owned'; owned.mkdir(); (owned/saved.name).write_bytes(saved.read_bytes())
        (root/'manifest.json').write_text(json.dumps(dict(save_store=dict(store=str(owned)))))
        (root/'save-store-result.json').write_text('{"merged":true}')
        (root/'lock-capture').mkdir();(root/'lock-capture/lifecycle.log').write_text('resident_storage_materialized 0 17 0 1 1\nresident_storage_materialized 0 17 0 0 1\n')
        rows=[]
        def add(event, screen, argument=0):
            r=[0]*16;r[0]=len(rows)+1;r[1]=event;r[2]=23;r[3]=screen;r[6]=1;r[8]=1;r[9]=argument;rows.append(r)
        actions=[(3,0)]+([(18,65537)] if difficulty else [])+[(18,0),(17,1),(13,0),(17,0),(12,0),(17,3),(6,21),(3,4)]
        if recreate:
            flow.update(recreate_world=True,display_rebuilds=2,reloaded_from_main=True)
            flow['states']=[dict(stage=8,screen=12,reloaded=True,parent=3,depth=3)]
            checkpoint=dict(flow['checkpoints'][-1],phase='after-recreate-load',source_generation='400',source_base='300',frames=400,frame_base=300)
            flow['checkpoints'].append(checkpoint)
            actions=actions[:-1]+[(3,3),(10,0),(3,3),(10,0),(3,1),(12,0),(17,3),(6,21),(3,4)]
        rebuild=0
        for index,(screen, arg) in enumerate(actions):
            add(3,screen,arg)
            main_load=recreate and index==len(actions)-4
            if main_load:rows[-1][12]=3
            if screen in (12,13):
                add(4,screen)
                if main_load:
                    add(13,17,2);rows[-1][4]=rows[-1][11]=0x6a5088;rows[-1][5]=0x5c6644;rows[-1][10]=rows[-1][12]=5
                else:add(20,2)
            if (screen,arg)==(17,3):add(22,screen,0)
            if recreate and screen==10:
                add(6,screen);add(7,screen,640 if not rebuild else 800)
                rows[-1][10]=480 if not rebuild else 600
                add(8,screen);rebuild+=1
        def write(): (root/'events.bin').write_bytes(b'MNMMENU1'+struct.pack('<II',1,64)+b''.join(struct.pack('<16I',*r) for r in rows))
        write()
        if difficulty==0:
            order=['select-spell','summon','select-creature','move-creature']
            (root/'storage-input-0.json').write_text(json.dumps(dict(success=True,mode='mini',tutorial_actions=[dict(action=name) for name in order],opening_progress=order,escape_sent=True)))
        return flow,rows,write

    def test_original_callbacks_and_owned_save(self):
        with tempfile.TemporaryDirectory() as d:
            flow,_,_=self.fixture(Path(d))
            self.assertTrue(m.validate_storage(Path(d),flow)['verified'])

    def test_original_display_rebuild_and_main_load(self):
        with tempfile.TemporaryDirectory() as d:
            flow,_,_=self.fixture(Path(d),recreate=True)
            self.assertEqual(len(m.validate_storage(Path(d),flow)['checkpoints']),4)

    def test_recreated_flow_cannot_reuse_old_checkpoint(self):
        with tempfile.TemporaryDirectory() as d:
            flow,_,_=self.fixture(Path(d),recreate=True)
            flow['checkpoints'][-1]['source_generation']='300'
            with self.assertRaises(ValueError):m.validate_storage(Path(d),flow)

    def test_display_dimensions_and_second_quit_are_required(self):
        for mutate in ('display','quit','rebuilds','reload','mini','main_parent'):
            with tempfile.TemporaryDirectory() as d:
                flow,rows,write=self.fixture(Path(d),recreate=True)
                if mutate=='display':next(r for r in rows if r[1]==7)[9]=800
                elif mutate=='quit':next(r for r in reversed(rows) if r[1]==22)[9]=1
                elif mutate=='rebuilds':flow['display_rebuilds']=1
                elif mutate=='reload':flow['reloaded_from_main']=False
                elif mutate=='mini':next(r for r in rows if r[1]==13)[10]=4
                else:flow['states'][0]['parent']=17
                write()
                with self.assertRaises(ValueError):m.validate_storage(Path(d),flow)

    def test_requested_difficulty_is_authoritative(self):
        for actual in (0,1):
            with tempfile.TemporaryDirectory() as d:
                flow,_,_=self.fixture(Path(d),actual)
                self.assertEqual(m.validate_storage(Path(d),flow,actual,Path(d))['difficulty'],actual)
                with self.assertRaises(ValueError):m.validate_storage(Path(d),flow,1-actual,Path(d))
                with self.assertRaises(ValueError):m.validate_storage(Path(d),flow,True)

    def test_initiate_requires_ordered_opening_input_proof(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);flow,_,_=self.fixture(root,0)
            with self.assertRaises(ValueError):m.validate_storage(root,flow,0)
            path=root/'storage-input-0.json';record=json.loads(path.read_text())
            record['tutorial_actions'].reverse();path.write_text(json.dumps(record))
            with self.assertRaises(ValueError):m.validate_storage(root,flow,0,root)

    def test_only_calibrated_tutorial_titles_admit_input(self):
        spec=importlib.util.spec_from_file_location('storage_input',ROOT/'tools/menu-storage-input.py')
        helper=importlib.util.module_from_spec(spec);spec.loader.exec_module(helper)
        self.assertEqual(helper.tutorial_action(' Select Zombie Spell\n'),('select-spell',(523,575),1))
        self.assertEqual(helper.tutorial_action('Summon Zombie'),('summon',(400,280),3))
        self.assertEqual(helper.tutorial_action('Select Zombie'),('select-creature',(400,280),1))
        self.assertEqual(helper.tutorial_action('Move Zombie'),('move-creature',(400,280),3))
        self.assertEqual(helper.tutorial_action('Hermes'),('dismiss',(400,300),1))
        for text in ('Zombe Spell','Select Zombie Spell tutorial is absent',''):
            self.assertIsNone(helper.tutorial_action(text))

    def test_ocr_failure_cannot_supply_a_tutorial_action(self):
        spec=importlib.util.spec_from_file_location('storage_input_failure',ROOT/'tools/menu-storage-input.py')
        helper=importlib.util.module_from_spec(spec);spec.loader.exec_module(helper)
        crash=helper.subprocess.CompletedProcess([], -8, 'Select Zombie Spell', 'failed OCR')
        with patch.object(helper.subprocess,'run',return_value=crash):
            result=helper.recognize_title(Path('unknown.png'))
        self.assertEqual(result['ocr_returncode'],-8)
        self.assertIsNone(helper.tutorial_action(result['text']))
        with patch.object(helper.subprocess,'run',side_effect=helper.subprocess.TimeoutExpired('tesseract',5)):
            result=helper.recognize_title(Path('unknown.png'))
        self.assertIsNone(helper.tutorial_action(result['text']))

    def test_original_resume_missing_or_outside_interval(self):
        for outside in (False,True):
            with tempfile.TemporaryDirectory() as d:
                flow,rows,write=self.fixture(Path(d))
                for r in rows:
                    if r[1]==20:r[1]=0
                if outside:rows[0][1]=20;rows[0][3]=2
                write()
                with self.assertRaises(ValueError):m.validate_storage(Path(d),flow)

    def test_save_mutation_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            flow,_,_=self.fixture(Path(d));flow['saved_sha256']='0'*64
            with self.assertRaises(ValueError):m.validate_storage(Path(d),flow)


if __name__ == '__main__': unittest.main()
