#!/usr/bin/env python3
"""Actual retained-data adversarial controls for the two input/media gates."""
import copy
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import Reader
import input_media_gate as gate
from rendering_pause import validate_dwell
from rendering_pause_assessment import read_samples
from rendering_menu_acceptance import artifact


class Proxy:
    def __init__(self, reader, name, change):
        self.reader, self.name, self.change = reader, name, change
    def digest(self, name):
        return self.reader.digest(name)
    def json(self, name):
        value = self.reader.json(name)
        if name == self.name:
            value = copy.deepcopy(value)
            self.change(value)
        return value


class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = Reader(ROOT)
        cls.parent = cls.reader.json(gate.PREFIX+'basis-01.json')
        cls.joined = cls.reader.json(gate.PREFIX+'probe-01.json')
        cls.plan = cls.reader.json('docs/native-rendering-closure.json')
        quick = next(v for v in cls.parent['requirements'] if v['configuration']=='player-default'
                     and v['id']=='journey.quick-battle-entry.setup-confirm')
        cls.pair = gate.Join(cls.reader).attachments(quick)[0]
        cls.a = cls.reader.json(cls.pair['acquisition']['record'])
        cls.c = cls.reader.json(cls.pair['comparison']['record'])
        cls.pause = cls.reader.json('research/runtime/rendering-pause-results-20261107/default-800-13-acquisition.json')
        rows, _ = read_samples(artifact(cls.reader, cls.pause, 'pause-samples.sqlite3'),
                              artifact(cls.reader, cls.pause, 'frame-input.bin'))
        cls.dwell = [v for v in rows if v['phase']=='mini-dwell-1']
        cls.movie = gate.module('close-rendering-movie-return')
        cls.intro = gate.module('close-rendering-enabled-intros')
        cls.expected, _ = cls.movie.references(cls.reader)
        collection = cls.reader.json(gate.INTRO)
        cls.intro_a = cls.reader.json(collection['cases'][0]['case']['record'])
        cls.media, cls.media_wire, cls.policy = cls.movie.media_inputs(cls.reader, cls.intro_a)

    @classmethod
    def tearDownClass(cls):
        cls.reader.close()

    def changed_parent(self, change):
        value = copy.deepcopy(self.parent)
        change(value)
        with self.assertRaises(ValueError): gate.basis(self.plan, value)

    def test_01_complete_named_prerequisites(self):
        self.assertEqual(len(gate.basis(self.plan, self.parent)),156)
        self.assertEqual(len(gate.LEAVES),18)

    def test_02_missing_picking(self):
        def change(q):
            row=next(v for v in q['requirements'] if v['id']=='journey.cursor-interaction.picking')
            row['state']='pending';q['passed_requirements']-=1;q['pending_requirements']+=1
        self.changed_parent(change)

    def test_03_missing_leaf_receipt(self):
        self.changed_parent(lambda q:next(v for v in q['requirements'] if v['id']=='journey.enabled-movies.movie-skip').update(admitted_execution_evidence=[]))

    def test_04_duplicate_requirement(self):
        self.changed_parent(lambda q:q['requirements'].__setitem__(1,copy.deepcopy(q['requirements'][0])))

    def test_05_wrong_parent_count(self):
        self.changed_parent(lambda q:q.update(passed_requirements=q['passed_requirements']+1))

    def test_06_changed_gate_condition(self):
        self.changed_parent(lambda q:next(v for v in q['requirements'] if v['id']==gate.GATE).update(pending_reason='No proof required'))

    def test_07_only_two_gate_cells_change(self):
        rows=gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),self.joined)
        self.assertEqual(sum(v['state']=='passed' for v in rows),self.parent['passed_requirements']+2)
        self.assertEqual([v for v in rows if v['id']!=gate.GATE],
                         [v for v in self.parent['requirements'] if v['id']!=gate.GATE])
        self.assertEqual(self.parent['decisions'],{'supported-player':'pending','original-recovery':'pending'})

    def test_08_actual_fullscreen_and_input(self):
        self.assertTrue(gate.fullscreen(self.a,self.c)['verified'])

    def bad_fullscreen(self, change):
        a,c=copy.deepcopy(self.a),copy.deepcopy(self.c)
        change(a,c)
        with self.assertRaises(ValueError):gate.fullscreen(a,c)

    def test_09_missing_f9_interval(self):
        self.bad_fullscreen(lambda a,c:c['fullscreen']['intervals'].pop())

    def test_10_uncompared_fullscreen_paint(self):
        self.bad_fullscreen(lambda a,c:c['fullscreen']['intervals'][0].update(presented_paints=[99999999]))

    def test_11_mixed_f9_action(self):
        self.bad_fullscreen(lambda a,c:a['flow']['fullscreen_transitions'][0].update(accepted=False))

    def test_12_missing_release(self):
        self.bad_fullscreen(lambda a,c:c['input_acknowledgements'].update(releases=0))

    def test_13_missing_original_ack_owner(self):
        self.bad_fullscreen(lambda a,c:c['input_acknowledgements']['cases'][0].update(original_thread=0))

    def test_14_dependent_pixel_oracle(self):
        self.bad_fullscreen(lambda a,c:c['original_reference'].update(native_player_pixels_read=True))

    def test_15_hidden_pixel_difference(self):
        self.bad_fullscreen(lambda a,c:c['comparison']['comparisons'][0].update(actual_sha256='0'*64))

    def test_16_changed_receipt_identity(self):
        ref=dict(self.pair['acquisition'],sha256='0'*64)
        with self.assertRaises(ValueError):gate.Join(self.reader).checked(ref)

    def test_17_wrong_requested_profile(self):
        with self.assertRaises(ValueError):gate.Join(self.reader).acquisition(self.pair['acquisition'],'player-software')

    def test_18_forced_stop_primary(self):
        reader=Proxy(self.reader,self.pair['acquisition']['record'],lambda q:q.update(forced_private_prefix_stop=True))
        with self.assertRaises(ValueError):gate.Join(reader).acquisition(self.pair['acquisition'],'player-default')

    def test_19_mixed_comparison_acquisition(self):
        reader=Proxy(self.reader,self.pair['comparison']['record'],lambda q:q['acquisition'].update(sha256='0'*64))
        with self.assertRaises(ValueError):gate.Join(reader).pair(self.pair,'player-default')

    def test_20_raw_pause_media_music(self):
        self.assertTrue(gate.pause_ownership(self.dwell)['media_live'])

    def bad_dwell(self, change):
        rows=copy.deepcopy(self.dwell);change(rows)
        with self.assertRaises(ValueError):gate.pause_ownership(rows)

    def test_21_unacknowledged_media(self):
        self.bad_dwell(lambda r:r[0]['media'].update(response=999))

    def test_22_stalled_music_owner(self):
        self.bad_dwell(lambda r:r[-1]['music'].update(heartbeat=r[0]['music']['heartbeat']))

    def test_23_held_gameplay_key(self):
        self.bad_dwell(lambda r:r[0]['input'].update(held=[38]))

    def test_24_wrong_world_nonce(self):
        self.bad_dwell(lambda r:r[-1]['world'].update(nonce='0'))

    def test_25_world_progress_during_mini(self):
        self.bad_dwell(lambda r:r[-1].update(original_world_return=r[0]['original_world_return']+1))

    def test_26_complete_enabled_media(self):
        self.assertTrue(self.movie.completed_media(self.intro_a,self.media,self.media_wire,self.policy,self.expected)['verified'])

    def bad_movie(self, change):
        media=copy.deepcopy(self.media);change(media)
        with self.assertRaises(ValueError):self.movie.completed_media(self.intro_a,media,self.media_wire,self.policy,self.expected)

    def test_27_silent_movie_fallback(self):
        self.bad_movie(lambda r:r[1].update(initial_silent_output=True))

    def test_28_movie_audio_device_missing(self):
        self.bad_movie(lambda r:r[1].update(audio_device_available=False))

    def test_29_incomplete_eof(self):
        self.bad_movie(lambda r:r[1].update(video_frames=r[1]['video_frames']-1))

    def test_30_lost_main_focus(self):
        a=copy.deepcopy(self.intro_a)
        next(v for v in a['screenshots'] if v['label']=='main-ready-initial')['host_focus']['host_focused']=False
        with self.assertRaises(ValueError):self.intro.main_return(self.reader,a)

    def test_31_surviving_graphics_owner(self):
        reader=Proxy(self.reader,self.pair['acquisition']['record'],lambda q:q['retirement']['graphics_retirement'].update(surviving_owners=['surface']))
        with self.assertRaises(ValueError):gate.Join(reader).acquisition(self.pair['acquisition'],'player-default')

    def test_32_unreturned_world_work(self):
        reader=Proxy(self.reader,self.pair['acquisition']['record'],lambda q:q['resident'].update(verified_returns='0'))
        with self.assertRaises(ValueError):gate.Join(reader).acquisition(self.pair['acquisition'],'player-default')

    def test_33_preexisting_music_failure(self):
        def change(rows):
            for row in rows:row['music']['failed']=1
        self.bad_dwell(change)

    def test_34_failed_media_ack(self):
        self.bad_dwell(lambda r:r[0]['media'].update(status=6))

    def test_35_unacknowledged_music_request(self):
        self.bad_dwell(lambda r:r[0]['music'].update(request=999))

    def test_36_missing_media_acceptance(self):
        self.bad_dwell(lambda r:r[0]['media'].update(accepted=999))

    def test_37_gate_cohort_requires_fullscreen(self):
        proof=copy.deepcopy(self.joined);proof['fullscreen']=[]
        with self.assertRaises(ValueError):gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),proof)

    def test_38_gate_cohort_requires_every_input_leaf(self):
        proof=copy.deepcopy(self.joined);proof['linked_requirements'].pop()
        with self.assertRaises(ValueError):gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),proof)


if __name__=='__main__':unittest.main()
