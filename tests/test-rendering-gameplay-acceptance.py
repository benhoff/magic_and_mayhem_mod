#!/usr/bin/env python3
"""Reject input-only, stale, mismatched-owner and incomplete gameplay witnesses."""
import copy
from pathlib import Path
import sys
import struct
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_gameplay_acceptance import ACTIONS, assess, windows, minimap, bind_markers


def fixture():
    rows=[];hs=[];ss=[];ls=[];calls=[];paints=[]
    rotations={'center-second':0,'rotate-1':1,'rotate-2':2,'rotate-3':3,'rotate-4':0}
    for i,action in enumerate(ACTIONS):
        begin=1000+i*3000
        for phase,delta in [('begin',0),('released',200),('settled',2500)]:
            rows.append(dict(action=action,phase=phase,accepted=True,host_us=str((begin+delta)*1000),original_tick=str(begin+delta),world=str(i*20+1)))
        for j in range(10):
            tick=begin+300+j*200
            hs.append(dict(tick=tick,thread=7,menu_screen=7,validity=8,camera_orientation=rotations.get(action,0),camera_whole=[int(i>=14),0,0]))
            ss.append(dict(tick=tick,thread=7,menu_screen=7,validity=4,top_screen=2,world_counter=42 if action=='pause' else tick,mode=int(action=='pause')))
            ls.append(dict(tick=tick,flags=1,pool_id=1,raw_68_slots=[0] if action=='pointer-away-selected' else []))
            paints.append(dict(host_us=str(tick*1000),world=str(i*20+j+1)))
        if action in ('select-creature','deselect-creature'):
            after=int(action=='select-creature')
            common=dict(actor_state=1,slot_plus_one=1,pool_id=1,actor_type=0,actor_owner=0,actor_active=1,actor_health=20)
            calls.append(dict(call=len(calls)+1,mouse_record=[400,265],entry=dict(common,tick=begin+100,actor_flag_68=1-after),returned=dict(common,tick=begin+150,actor_flag_68=after)))
    return [rows,dict(owner_thread=7,samples=hs),dict(owner_thread=7,samples=ss),dict(owner=7,samples=ls),dict(owner=7,calls=calls),paints]


class GameplayTests(unittest.TestCase):
    def test_finite_witness_keeps_exact_frame_and_whole_requirements_pending(self):
        result=assess(*fixture())
        self.assertTrue(result['success'])
        self.assertFalse(result['exact_scalar_to_source_frame_binding_proved'])
        self.assertFalse(result['tooltip_text_and_restoration_proved'])
        self.assertFalse(result['complete_camera_picking_hud_requirement_proved'])

    def test_schedule_missing_refused_reordered_and_missing_world(self):
        for mutation in (lambda x:x.pop(),lambda x:x[0].update(accepted=False),lambda x:x[1].update(phase='settled'),lambda x:x[2].update(host_us='1'),lambda x:x[0].update(world='0'),lambda x:x[0].pop('original_tick')):
            args=fixture();mutation(args[0])
            with self.assertRaises((ValueError,KeyError)):assess(*args)

    def test_original_owner_and_exact_tick_identity(self):
        for mutation in (lambda x:x[1].update(owner_thread=8),lambda x:x[2]['samples'][0].update(thread=9),lambda x:x[2]['samples'].append(copy.deepcopy(x[2]['samples'][0]))):
            args=fixture();mutation(args)
            with self.assertRaises(ValueError):assess(*args)

    def test_requested_rotations_without_original_state_do_not_pass(self):
        args=fixture()
        for sample in args[1]['samples']:sample['camera_orientation']=0
        self.assertFalse(assess(*args)['facts']['four_orientations']['verified'])

    def test_missing_stable_sample_and_out_of_range_orientation(self):
        for replacement in (None,4):
            args=fixture();start=int(windows(args[0])['rotate-2']['begin']['original_tick'])
            for sample in args[1]['samples']:
                if start<sample['tick']<start+2500:sample['camera_orientation']=replacement
            self.assertFalse(assess(*args)['success'])

    def test_pan_key_without_camera_change_does_not_pass(self):
        args=fixture()
        for sample in args[1]['samples']:sample['camera_whole']=[0,0,0]
        self.assertFalse(assess(*args)['facts']['pan']['verified'])

    def test_pause_requires_stopped_counter_and_live_presentations(self):
        for kind in ('moving','stopped-render','stopped-resume'):
            args=fixture();w=windows(args[0]);start=int(w['pause' if kind!='stopped-resume' else 'resume']['begin']['original_tick'])
            if kind=='moving':
                for s in args[2]['samples']:
                    if start<s['tick']<start+2500:s['world_counter']=s['tick']
            elif kind=='stopped-render':args[-1]=[p for p in args[-1] if not start*1000<int(p['host_us'])<(start+2500)*1000]
            else:
                for s in args[2]['samples']:
                    if start<s['tick']<start+2500:s['world_counter']=42
            self.assertFalse(assess(*args)['facts']['pause_resume']['verified'])

    def test_click_candidate_toggle_and_pool_continuity_required(self):
        for mutation in (lambda x:x[4]['calls'].clear(),lambda x:x[4]['calls'][0]['entry'].update(slot_plus_one=2),lambda x:x[4]['calls'][0]['returned'].update(actor_owner=1),lambda x:x[4]['calls'][0]['returned'].update(actor_health=0),lambda x:x[4]['calls'][0]['entry'].update(actor_active=0),lambda x:x[4]['calls'][0]['returned'].update(actor_flag_68=0),lambda x:x[3]['samples'].clear(),lambda x:x[3]['samples'][40].update(pool_id=2)):
            args=fixture();mutation(args)
            self.assertFalse(assess(*args)['facts']['wizard_latch_toggle']['verified'])

    def test_active_pause_preamble_requires_the_actual_mode_transition(self):
        args=fixture();w=windows(args[0]);start=int(w['pause']['begin']['original_tick'])
        selected=[s for s in args[2]['samples'] if start<s['tick']<start+2500]
        for s in selected[:2]:s.update(mode=0,world_counter=s['tick'])
        self.assertTrue(assess(*args)['facts']['pause_resume']['verified'])
        selected[-1]['mode']=0
        self.assertFalse(assess(*args)['facts']['pause_resume']['verified'])
        args=fixture()
        for s in args[2]['samples']:s['mode']=0
        self.assertFalse(assess(*args)['facts']['pause_resume']['verified'])

    def test_marker_matches_exact_original_row_tick_and_owner(self):
        marker=[dict(original_row=1,original_tick='1000'),dict(original_row=2,original_tick='1050')]
        observation=dict(owner_thread=7,rows=[dict(sequence=1,tick=1000,thread=7),dict(sequence=2,tick=1050,thread=7)])
        self.assertTrue(bind_markers(marker,observation)['verified'])
        for replacement in (dict(original_row=0,original_tick='1000'),dict(original_row=3,original_tick='1000'),dict(original_row=1,original_tick='999'),dict(original_row=True,original_tick='1000')):
            with self.assertRaises(ValueError):bind_markers([replacement],observation)
        observation['rows'][0]['thread']=8
        with self.assertRaises(ValueError):bind_markers(marker,observation)

    def test_minimap_four_cases_use_three_bodies_and_never_forward(self):
        words=[0]*580;words[:4]=[0x50504e4d,2,2320,6]
        for i in range(7):words[4+(3*32+i)*3:7+(3*32+i)*3]=[1,0 if i==6 else 10,0]
        raw=lambda:struct.pack('<580I',*words)
        self.assertTrue(minimap(raw())['verified'])
        for index,value in [(4+(3*32+4)*3+1,0),(4+(3*32+6)*3+1,1),(4+(3*32+1)*3+2,1),(2,2319)]:
            old=words[index];words[index]=value
            with self.assertRaises(ValueError):minimap(raw())
            words[index]=old

    def test_each_action_requires_actual_world_presentation(self):
        args=fixture();w=windows(args[0]);start=int(w['hover-tooltip']['begin']['host_us']);end=int(w['hover-tooltip']['settled']['host_us'])
        args[-1]=[p for p in args[-1] if not start<int(p['host_us'])<=end]
        self.assertFalse(assess(*args)['facts']['presented_all_action_windows'])

if __name__=='__main__':unittest.main()
