#!/usr/bin/env python3
"""Hermes readiness and refusal controls; no original/live execution credit."""
import copy
import hashlib
import importlib.util
import struct
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_camera_dialogue import CameraDialogueGuard,dialogue_crop,validate_camera_dialogue,validate_camera_mini
from campaign_pixel_routes import camera_scenarios,validate_camera_progress


def item(native, original):
    return dict(name='test',visible=dict(native=native,original=original),dismissed=False)


class CameraCrop(unittest.TestCase):
    def test_crop_retains_heading_at_both_admitted_extents(self):
        self.assertEqual(dialogue_crop(800,600),(225,90,600,270))
        self.assertEqual(dialogue_crop(640,480),(180,72,480,216))
        # The observed640heading begins above row90and needs its complete H.
        self.assertLess(dialogue_crop(640,480)[1],86)

    def test_other_extents_refuse_before_quiet_classification(self):
        for size in [(640,600),(800,480),(641,480),(0,0),(1280,960)]:
            with self.subTest(size=size),self.assertRaisesRegex(ValueError,'extent'):
                dialogue_crop(*size)


class CameraReadiness(unittest.TestCase):
    def guard(self,resident=True):
        clicks=[];events=[]
        guard=CameraDialogueGuard(Path('/tmp/input-0.json'),lambda *args:clicks.append(args),
                                  lambda action,**fields:events.append(dict(action=action,**fields)),resident)
        return guard,clicks,events

    def test_stale_cpu_hermes_cannot_trigger_resident_click(self):
        guard,clicks,_=self.guard()
        with patch.object(guard,'capture',side_effect=[item(False,True),item(False,True)]),patch('campaign_camera_dialogue.time.sleep'):
            self.assertFalse(guard.ensure_ready())
        self.assertEqual(clicks,[])

    def test_witnessed_hermes_requires_two_quiet_native_captures(self):
        guard,clicks,_=self.guard()
        with patch.object(guard,'capture',side_effect=[item(True,False),item(False,True),item(False,True)]),patch('campaign_camera_dialogue.time.sleep'):
            self.assertTrue(guard.ensure_ready())
        self.assertEqual(clicks,[(400,300)]);self.assertEqual(guard.dismissals,1)
        self.assertTrue(guard.checks[0]['dismissed'])

    def test_dual_mode_disagreement_cannot_trigger_click(self):
        guard,clicks,_=self.guard(False)
        with patch.object(guard,'capture',side_effect=[item(True,False),item(True,True),item(False,False),item(False,False)]),patch('campaign_camera_dialogue.time.sleep'):
            self.assertTrue(guard.ensure_ready())
        self.assertEqual(clicks,[(400,300)])

    def test_persistent_dialogue_and_disagreement_are_bounded(self):
        for resident,source,expected in [(True,item(True,False),12),(False,item(True,False),0)]:
            guard,clicks,_=self.guard(resident)
            with patch.object(guard,'capture',side_effect=lambda:copy.deepcopy(source)),patch('campaign_camera_dialogue.time.sleep'),self.assertRaises(ValueError):
                guard.ensure_ready()
            self.assertEqual(len(clicks),expected);self.assertLessEqual(len(guard.checks),16)

    def test_capture_budget_refuses_before_request(self):
        guard,_,_=self.guard();guard.checks=[None]*32
        with self.assertRaisesRegex(ValueError,'budget'):guard.capture()

    def fixture(self,root):
        checks=[]
        for i in range(3):
            name=f'camera-input-0-{i:02d}';hashes={}
            for route in ('native','original'):
                path=root/(name+'-'+route+'.png');path.write_bytes((name+route).encode());hashes[route]=hashlib.sha256(path.read_bytes()).hexdigest()
            checks.append(dict(name=name,metadata=dict(fallback=False,native_saved=True,original_saved=True),image_sha256=hashes,
                               texts=dict(native='Hermes' if i==0 else 'Forest',original='Hermes'),
                               visible=dict(native=i==0,original=True),dismissed=i==0))
        return dict(camera_dialogue=dict(oracle='resident-native',checks=checks,dismissals=1,overhead_seconds=1.,scope='Known Hermes'))

    def test_acceptance_refuses_wrong_oracle_fallback_claims_and_unclosed_dialogue(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);fixture=self.fixture(root)
            self.assertEqual(validate_camera_dialogue(fixture,root,True)['dismissals'],1)
            mutations=[lambda g:g.update(oracle='dual-draw'),lambda g:g.update(dismissals=0),
                       lambda g:g.update(overhead_seconds=float('nan')),lambda g:g.update(overhead_seconds=True),
                       lambda g:g['checks'][-1].update(visible=dict(native=True,original=True)),
                       lambda g:g['checks'][0]['metadata'].update(fallback=True),
                       lambda g:g['checks'][0]['metadata'].update(native_saved=False),
                       lambda g:g['checks'][0].update(dismissed=False),
                       lambda g:g['checks'][-1].update(dismissed=True),
                       lambda g:g['checks'][0].update(name='../escape'),
                       lambda g:g['checks'].pop()]
            for mutate in mutations:
                bad=copy.deepcopy(fixture);mutate(bad['camera_dialogue'])
                with self.subTest(mutate=mutate),self.assertRaises(ValueError):validate_camera_dialogue(bad,root,True)
            path=root/'camera-input-0-01-native.png';path.write_bytes(b'changed')
            with self.assertRaises(ValueError):validate_camera_dialogue(fixture,root,True)

    def test_modal_reset_requires_four_fresh_turns_and_all_three_bodies(self):
        counts=[0]*4;calls=[];actions=[];readiness=0
        def counters(game):return dict(forwarded=0,history_refused=False,families=dict(minimap=[dict(route=f'camera{i}',native=n) for i,n in enumerate(counts)]))
        def key(*args):
            calls.append(args);index=len(calls)-1;counts[0 if index<4 else (0,2,0,1)[index-4]]+=1
        def ready():
            nonlocal readiness
            readiness+=1;return readiness==2
        with patch('campaign_pixel_routes.read_counters',side_effect=counters),patch('campaign_pixel_routes.time.sleep'):
            result=camera_scenarios(Path('.'),key,lambda action,**fields:actions.append(dict(action=action,**fields)),ready)
        self.assertEqual(result['turns'],8);self.assertEqual(result['ready_epoch_turns'],4)
        self.assertEqual(result['dialogue_interruptions'],1)
        inputs=dict(actions=actions);self.assertTrue(validate_camera_progress(inputs)['verified'])
        bad=copy.deepcopy(inputs);bad['actions'][-1]['ready_epoch_initial']['camera1']=bad['actions'][-1]['final']['camera1']
        with self.assertRaises(ValueError):validate_camera_progress(bad)
        bad=copy.deepcopy(inputs);bad['actions'][-1]['ready_epoch_turns']=3
        with self.assertRaises(ValueError):validate_camera_progress(bad)


class CameraMiniReadiness(unittest.TestCase):
    fixture=CameraReadiness.fixture
    def helper(self):
        spec=importlib.util.spec_from_file_location('camera_menu',Path(__file__).resolve().parents[1]/'tools/campaign-camera-menu-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

    def test_transition_race_retries_only_after_witnessed_hermes(self):
        module=self.helper();ticks=[0.];escapes=[];events=[];readiness=iter([False,True])
        def sleep(seconds):ticks[0]+=seconds
        witness=[1]*16
        result=module.open_mini(lambda:next(readiness),lambda:escapes.append(1),
            lambda:witness if len(escapes)==2 else None,lambda action,**kw:events.append(action),sleep,lambda:ticks[0])
        self.assertEqual(result,witness);self.assertEqual(len(escapes),2)
        ticks[0]=0;escapes.clear()
        with self.assertRaisesRegex(ValueError,'no recognized'):
            module.open_mini(lambda:False,lambda:escapes.append(1),lambda:None,lambda *a,**k:None,sleep,lambda:ticks[0])
        self.assertEqual(len(escapes),1)

    def test_transition_witness_is_bound_to_original_journal(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);fixture=self.fixture(root)
            row=[0]*16;row[0]=1;row[1]=3;row[3]=17;row[9]=4
            journal=root/'events.bin';journal.write_bytes(b'MNMMENU1'+struct.pack('<II',1,64)+struct.pack('<16I',*row))
            fixture.update(success=True,before_sequence=0,mini_callback=row,
                actions=[dict(action='camera-hermes-dismiss'),dict(action='camera-mini-escape',attempt=0)])
            self.assertTrue(validate_camera_mini(fixture,root,True,journal,4)['verified'])
            module=self.helper();self.assertEqual(module.new_mini(journal,0),row);self.assertIsNone(module.new_mini(journal,1))
            for mutate in (lambda f:f['mini_callback'].__setitem__(9,3),lambda f:f.update(before_sequence=1),
                           lambda f:f['actions'].append(dict(action='camera-mini-escape',attempt=1)),
                           lambda f:f['actions'].clear()):
                bad=copy.deepcopy(fixture);mutate(bad)
                with self.assertRaises(ValueError):validate_camera_mini(bad,root,True,journal,4)


if __name__=='__main__':unittest.main()
