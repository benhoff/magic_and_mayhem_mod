#!/usr/bin/env python3
"""Adverse actor/event ownership guards on a retained real original execution."""
from copy import deepcopy
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest
import numpy as np

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from scene_actor_observation import decode
import scene_actor_output as output
from scene_actor_output import melee_windows,unique_body,split_normal_source,regular_sources,combat_pose_change,visible_native_points,scoped_artifacts
from rendering_pilot import viewport


class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        name=os.environ.get('MNM_SCENE_ACTOR_ACQUISITION','research/runtime/rendering-normal-results-20261031/ac07-combat-default800-03.json')
        with Reader(ROOT)as reader:
            q=reader.json(name);assert q['success'] is True
            observed=decode(artifact(reader,q,'scene-actors-0001.bin'));raw=artifact(reader,q,'gameplay-events.bin')
            source=artifact(reader,q,'worlds.bin')
        cls.frames=observed['frames'];cls.events=[struct.unpack_from('<16I',raw,i)for i in range(16,len(raw),64)]
        windows=melee_windows(cls.frames,cls.events);assert windows
        cls.window=windows[0];cls.pair=[cls.frames[cls.window[k]-1]for k in('before','after')]
        cls.event=tuple(cls.window['event'])
        cls.impact=None
        for w in windows:
            states=[]
            for key in('before','after'):
                f=cls.frames[w[key]-1];sources={};actors={}
                for slot in(w['event'][4],w['event'][8]):
                    actors[str(slot)]=list(next(a for a in f['actors']if a[0]==slot))
                    draws=[d for d in f['draws']if d[0:2]==(slot,0)and d[7]==31]
                    if len(draws)==1:sources['actor-'+str(slot)]=dict(raw_queue=list(draws[0]))
                states.append(dict(sources=sources,actors=actors))
            if combat_pose_change(w,states):cls.impact=(w,states);break
        assert cls.impact is not None
        with tempfile.TemporaryDirectory(prefix='mnm-actor-controls-',dir='/tmp')as temporary:
            path=Path(temporary)/'source.bin';path.write_bytes(source)
            for g,chunk,raw,request in output.records(path):
                if chunk or g!=cls.window['before']:continue
                rows=output.module('inspect-canvas-producers').decode(raw,complete=False)[1]
                cls.rasters=output.base.checked_request(rows,request)[1]
                cls.draw=next(d for d in cls.pair[0]['draws']if d[0:2]==(0,0)and d[7]==31)
                cls.halves=[row for row in cls.rasters if row[0][14]==0 and row[0][15]==1
                    and row[0][8:10]==cls.draw[5:7] and row[0][19]==cls.draw[11]
                    and output.crc32c(row[1][:row[0][19]])==cls.draw[12]]
                assert len(cls.halves)==2
                break

    def rejected(self,change):
        pair=deepcopy(self.pair);events=[self.event];change(pair,events)
        self.assertEqual(melee_windows(pair,events),[])

    def test_real_adjacent_original_melee_health_window(self):
        self.assertTrue(melee_windows(self.pair,[self.event]))
        self.assertGreater(self.event[11],self.event[12])

    def test_missing_original_event(self):self.rejected(lambda f,e:e.clear())
    def test_foreign_original_thread(self):
        def change(f,e):e[0]=tuple(v+1 if i==2 else v for i,v in enumerate(e[0]))
        self.rejected(change)
    def test_foreign_frame_thread(self):self.rejected(lambda f,e:f[0].update(thread=f[0]['thread']+1))
    def test_different_source_session(self):self.rejected(lambda f,e:f[0].update(nonce=f[0]['nonce']+1))
    def test_skipped_source_generation(self):self.rejected(lambda f,e:f[1].update(generation=f[1]['generation']+1))
    def test_changed_pool_epoch(self):self.rejected(lambda f,e:f[1].update(pool_epoch=f[1]['pool_epoch']+1))
    def test_late_original_event_is_not_in_frame(self):self.rejected(lambda f,e:f[1].update(gameplay_sequence=e[0][0]-1))
    def test_already_observed_event_is_not_a_new_transition(self):self.rejected(lambda f,e:f[0].update(gameplay_sequence=e[0][0]))
    def test_changed_health_does_not_bind_original_damage(self):
        def change(f,e):
            slot=e[0][8];rows=[]
            for a in f[1]['actors']:
                a=list(a)
                if a[0]==slot:a[4]+=1
                rows.append(tuple(a))
            f[1]['actors']=rows
        self.rejected(change)
    def test_foreign_target_owner(self):
        def change(f,e):e[0]=tuple(v+1 if i==10 else v for i,v in enumerate(e[0]))
        self.rejected(change)
    def test_duplicate_melee_in_bracket_needs_separate_analysis(self):self.rejected(lambda f,e:e.append(e[0]))
    def test_event_clock_outside_source_bracket(self):
        def change(f,e):e[0]=tuple((f[1]['tick']+10000)&0xffffffff if i==3 else v for i,v in enumerate(e[0]))
        self.rejected(change)
    def test_shared_body_token_cannot_assign_actor_ownership(self):
        frame=deepcopy(self.pair[0]);hero=next(a for a in frame['actors']if a[0]==0)
        other=next(i for i,a in enumerate(frame['actors'])if a[0]!=0);a=list(frame['actors'][other]);a[16]=hero[16];frame['actors'][other]=tuple(a)
        self.assertIsNone(unique_body(frame,{(0,0):[object()]},0))

    def changed_half(self,index,value):
        halves=deepcopy(self.halves);f=list(halves[1][0]);f[index]=value
        halves[1]=(tuple(f),*halves[1][1:]);return halves

    def test_real_original_split_clip_body_has_unique_queue_owner(self):
        selected=split_normal_source(self.draw,self.halves)
        self.assertIsNotNone(selected)
        joined=regular_sources(self.pair[0],self.rasters,{})
        self.assertIsNotNone(unique_body(self.pair[0],joined,0))
    def test_duplicate_raster_is_not_two_clip_halves(self):
        self.assertIsNone(split_normal_source(self.draw,[self.halves[0]]*2))
    def test_unknown_split_caller_refused(self):
        self.assertIsNone(split_normal_source(self.draw,self.changed_half(22,0x500513)))
    def test_split_clip_gap_refused(self):
        self.assertIsNone(split_normal_source(self.draw,self.changed_half(11,self.halves[1][0][11]+1)))
    def test_split_clip_overlap_refused(self):
        self.assertIsNone(split_normal_source(self.draw,self.changed_half(11,self.halves[1][0][11]-1)))
    def test_changed_queue_flag_argument_refused(self):
        self.assertIsNone(split_normal_source(self.draw,self.changed_half(16,self.halves[1][0][16]^1)))
    def test_three_matches_are_ambiguous(self):
        self.assertIsNone(split_normal_source(self.draw,self.halves+[self.halves[0]]))
    def test_two_identical_queue_candidates_have_no_unique_actor_source(self):
        frame=deepcopy(self.pair[0]);frame['draws'].append(self.draw)
        self.assertIsNone(unique_body(frame,regular_sources(frame,self.rasters,{}),0))
    def test_original_impact_changes_a_rendered_body_frame(self):
        self.assertTrue(combat_pose_change(*self.impact))
    def test_health_only_change_does_not_prove_visual_impact(self):
        window,states=deepcopy(self.impact)
        states[1]['sources']=deepcopy(states[0]['sources'])
        self.assertFalse(combat_pose_change(window,states))
    def test_foreign_engagement_target_does_not_bind_impact(self):
        window,states=deepcopy(self.impact);states[0]['actors'][str(window['event'][4])][11]+=1
        self.assertFalse(combat_pose_change(window,states))
    def test_nonmelee_action_does_not_bind_impact(self):
        window,states=deepcopy(self.impact);states[0]['actors'][str(window['event'][4])][10]=15
        self.assertFalse(combat_pose_change(window,states))
    def test_nonmelee_behavior_does_not_bind_impact(self):
        window,states=deepcopy(self.impact);states[0]['actors'][str(window['event'][4])][9]=1
        self.assertFalse(combat_pose_change(window,states))
    def test_source_health_must_match_original_hit_context(self):
        def change(f,e):
            rows=[]
            for a in f[0]['actors']:
                a=list(a)
                if a[0]==e[0][4]:a[4]+=1
                rows.append(tuple(a))
            f[0]['actors']=rows
        self.rejected(change)


class VisibilitySelectionControls(unittest.TestCase):
    """Synthetic geometry guards only; actual executions supply scene credit."""
    def fixture(self,scaling=0):
        iw,ih=(13,7)if scaling==0 else(16,8)
        paint=dict(frame_width=8,frame_height=4,image_width=iw,image_height=ih,
                   left=2,top=1,width=iw+4,height=ih+2,scaling=scaling)
        tags=np.arange(32,dtype='int64').reshape(4,8);tags.flat[:8]=-1
        wanted={'actor':[dict(world_pixel=i,original_word=i+1)for i in range(16)]}
        return tags,paint,wanted
    def test_cursor_occlusion_can_leave_eight_other_points_in_same_paint(self):
        for scaling in(0,2):
            with self.subTest(scaling=scaling):
                tags,paint,wanted=self.fixture(scaling);selected=visible_native_points(tags,paint,wanted)
                self.assertEqual([v['world_pixel']for k,v,p in selected],list(range(8,16)))
                frame=np.zeros((4,8,4),dtype='uint8');frame[:,:,0]=np.arange(32).reshape(4,8);frame[:,:,3]=255
                expected=np.frombuffer(viewport(frame,paint),dtype='uint8').reshape(paint['height'],paint['width'],4)
                for kind,source,(x,y,vx,vy)in selected:
                    self.assertEqual(int(tags[y,x]),source['world_pixel'])
                    self.assertEqual(int(expected[vy,vx,0]),source['world_pixel'])
    def test_seven_visible_points_are_insufficient(self):
        tags,paint,wanted=self.fixture();tags.flat[8]=-1
        self.assertIsNone(visible_native_points(tags,paint,wanted))
    def test_one_body_cannot_substitute_for_another_occluded_body(self):
        tags,paint,wanted=self.fixture();wanted['child']=[dict(world_pixel=i,original_word=i+1)for i in range(16,24)];tags.flat[16]=-1
        self.assertIsNone(visible_native_points(tags,paint,wanted))
    def test_repeated_source_pixel_does_not_increase_coverage(self):
        tags,paint,wanted=self.fixture();wanted['actor'].append(wanted['actor'][-1])
        with self.assertRaisesRegex(ValueError,'Duplicate actor source point'):visible_native_points(tags,paint,wanted)
    def test_unvalidated_smooth_mapping_is_refused(self):
        tags,paint,wanted=self.fixture();paint['scaling']=1
        with self.assertRaisesRegex(ValueError,'Unsupported actor viewport'):visible_native_points(tags,paint,wanted)


class RetainedSessionScopeControls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT)as reader:
            cls.acquired=reader.json('research/runtime/attachment-combat-results-20261107/attachment-default800-04.json')
        assert cls.acquired['success']is True
    def test_actual_overlapping_run_census_cannot_supply_scene_journal(self):
        acquired,excluded=scoped_artifacts(self.acquired)
        self.assertTrue(any(p.endswith('/gameplay-events.bin')for p in excluded))
        with Reader(ROOT)as reader:
            raw=artifact(reader,acquired,'gameplay-events.bin')
        self.assertEqual(raw[:8],b'MNMGP001')
        traces=[v['path']for v in acquired['artifacts']if v['path'].endswith('/gameplay-events.bin')]
        self.assertEqual(traces,[(Path(acquired['session'])/'gameplay-events.bin').relative_to(ROOT).as_posix()])
    def test_foreign_journal_cannot_replace_missing_exact_session_journal(self):
        q=deepcopy(self.acquired);name=(Path(q['session'])/'gameplay-events.bin').relative_to(ROOT).as_posix()
        q['artifacts']=[v for v in q['artifacts']if v['path']!=name]
        acquired,excluded=scoped_artifacts(q)
        with Reader(ROOT)as reader:
            with self.assertRaisesRegex(ValueError,'Missing/ambiguous'):artifact(reader,acquired,'gameplay-events.bin')
    def test_session_name_prefix_is_not_an_owner_root(self):
        q=deepcopy(self.acquired);v=deepcopy(next(v for v in q['artifacts']if v['path'].endswith('/gameplay-events.bin')))
        v['path']=Path(q['session']).relative_to(ROOT).as_posix()+'-other/gameplay-events.bin';q['artifacts'].append(v)
        acquired,excluded=scoped_artifacts(q);self.assertIn(v['path'],excluded)
    def test_duplicate_exact_artifact_path_refused(self):
        q=deepcopy(self.acquired);name=(Path(q['session'])/'gameplay-events.bin').relative_to(ROOT).as_posix()
        q['artifacts'].append(deepcopy(next(v for v in q['artifacts']if v['path']==name)))
        with self.assertRaisesRegex(ValueError,'Duplicate exact scene'):scoped_artifacts(q)


if __name__=='__main__':unittest.main()
