#!/usr/bin/env python3
"""Adverse guards for original selection effects and partial-colour provenance."""
import copy
import struct
import sys
import unittest
from pathlib import Path
import numpy as np
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from rendering_selection import (MATRIX,OUT,BytesSource,Lineage,PartialWorlds,decode,original_pair,
    marker_catalog,marker_request,source_state,contrast_pair,viewport_points,module,sha,compared_swaps,
    cursor_accounting,close_rows,PARENT,LEAF,blend_word,resolve_points)
from resident_capture import records


class OriginalSelection(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            m=r.json(MATRIX);cls.c=r.json(m['comparisons'][0]['record']);cls.a=r.json(cls.c['acquisition']['record'])
            cls.frames=decode(artifact(r,cls.a,'frame-input.bin'))
            cls.cursor=artifact(r,cls.a,'native-cursor-lifetime.bin')
            cls.original_selection_pair=cls.c['proof']['selection_pairs'][0]
            cls.catalog,cls.asset=marker_catalog(r,cls.a,cls.c)
            cls.sources={};decoder=module('inspect-canvas-producers')
            for g,ch,stream,request in records(BytesSource(artifact(r,cls.a,'worlds.bin'))):
                if not ch and g in (83,105):cls.sources[g]=(decoder.decode(stream,complete=False)[1],request)
                if len(cls.sources)==2:break

    def pair(self,rows=None,pair=None):
        return original_pair(pair or self.original_selection_pair,self.c['proof'],rows or self.frames['rows'],self.frames['nonce'])

    def test_real_original_toggle_pair_and_resource_catalog(self):
        p=self.pair();self.assertEqual([s['selected'] for s in p['states']],[0,1]);self.assertEqual(len(self.catalog),38)
        self.assertEqual(self.asset['path'],'Sprites/markers.spr')

    def changed(self,field,value):
        rows=copy.deepcopy(self.frames['rows']);g=self.c['proof']['cursor_bindings'][5]['source_frames'][0]
        row=next(r for r in rows if r['kind']==1 and r['frame'] and r['frame']['generation']==g)
        row['tracked'][field]=value;return rows

    def test_hover_is_not_selection(self):
        with self.assertRaises(ValueError):self.pair(self.changed('flag70',1))
    def test_unacknowledged_selection(self):
        p=copy.deepcopy(self.original_selection_pair);p['toggles'][0]['after']=1
        with self.assertRaises(ValueError):self.pair(pair=p)
    def test_dead_actor(self):
        with self.assertRaises(ValueError):self.pair(self.changed('health',0))
    def test_other_pool(self):
        with self.assertRaises(ValueError):self.pair(self.changed('pool',77))
    def test_wrong_flag_after_original_callback(self):
        with self.assertRaises(ValueError):self.pair(self.changed('flag68',1))
    def test_missing_callback_return(self):
        rows=copy.deepcopy(self.frames['rows']);rows[self.original_selection_pair['toggles'][0]['returned']-1]['kind']=2
        with self.assertRaises(ValueError):self.pair(rows)
    def test_wrong_callback_mouse(self):
        p=copy.deepcopy(self.original_selection_pair);p['toggles'][0]['mouse']=[0,0]
        with self.assertRaises(ValueError):self.pair(pair=p)
    def test_relabelled_source_nonce(self):
        with self.assertRaises(ValueError):original_pair(self.original_selection_pair,self.c['proof'],self.frames['rows'],'99')
    def test_marker_only_in_selected_actual_sources(self):
        for g,selected in ((83,0),(105,1)):
            rows,request=self.sources[g];corners,_,auxiliary=marker_request(rows,request,selected,self.catalog)
            self.assertEqual(len(corners),4*selected)
    def test_marker_without_original_selection_refuses(self):
        rows,request=self.sources[105]
        with self.assertRaises(ValueError):marker_request(rows,request,0,self.catalog)
    def test_source_or_descriptor_tampering_refuses(self):
        rows,request=self.sources[105];bad=bytearray(request);bad[-1]^=1
        with self.assertRaises(ValueError):marker_request(rows,bytes(bad),1,self.catalog)
    def test_unknown_actual_caller_is_not_marker_proof(self):
        rows,request=self.sources[105];bad=[]
        for f,b in rows:
            f=list(f)
            if f[22]==0x500845:f[22]+=1
            bad.append((tuple(f),b))
        with self.assertRaises(ValueError):marker_request(bad,request,1,self.catalog)
    def test_missing_corner_refuses(self):
        rows,request=self.sources[105]
        bad=[(f,b) for f,b in rows if not (f[22]==0x500845 and f[1]==1781)]
        with self.assertRaises(ValueError):marker_request(bad,request,1,self.catalog)
    def test_actor_motion_is_not_selection_contrast(self):
        rows=copy.deepcopy(self.frames['rows']);g=self.c['proof']['cursor_bindings'][5]['source_frames'][0]
        row=next(r for r in rows if r['kind']==1 and r['frame'] and r['frame']['generation']==g);row['position'][0]+=1
        with self.assertRaises(ValueError):self.pair(rows)

    def cursor_changed(self,change):
        rows=[list(r) for r in struct.iter_unpack('<16I',self.cursor)]
        change(rows)
        return b''.join(struct.pack('<16I',*r) for r in rows)

    def test_raw_cursor_refusal_remains_explicit(self):
        c=cursor_accounting(self.cursor)
        self.assertEqual(c['client_refusals'],1)
        self.assertEqual(c['refusals'][0]['slot'],3)
        self.assertTrue(c['all_refusals_accounted'])
    def test_foreground_cursor_refusal_cannot_be_teardown(self):
        def change(rows):
            next(r for r in rows[1:] if r[1]==6)[3]=0
        with self.assertRaises(ValueError):cursor_accounting(self.cursor_changed(change))
    def test_missing_outer_close_cannot_explain_refusal(self):
        def change(rows):
            next(r for r in rows[1:] if r[1]==9)[1]=8
        with self.assertRaises(ValueError):cursor_accounting(self.cursor_changed(change))
    def test_missing_raw_refusal_event_refuses(self):
        def change(rows):
            next(r for r in rows[1:] if r[1]==6)[1]=8
        with self.assertRaises(ValueError):cursor_accounting(self.cursor_changed(change))
    def test_client_after_outer_close_refuses(self):
        def change(rows):
            rows[-2][1]=7
        with self.assertRaises(ValueError):cursor_accounting(self.cursor_changed(change))
    def test_cursor_geometry_refusal_is_not_hidden(self):
        def change(rows):rows[-1][12]+=1
        with self.assertRaises(ValueError):cursor_accounting(self.cursor_changed(change))
    def test_cursor_queue_overflow_is_not_hidden(self):
        def change(rows):rows[-1][9]+=1
        with self.assertRaises(ValueError):cursor_accounting(self.cursor_changed(change))


class FrozenAdmission(unittest.TestCase):
    def setUp(self):
        with Reader(ROOT) as r:
            self.plan=r.json('docs/native-rendering-closure.json');self.parent=r.json(PARENT)
        self.cases=[dict(identity=str(i),domain=[p,list(s)],verified=True,selection_callbacks=20,
            settled_source_generations=60,presentations=[{}]*60,contrasts=[{}]*10,matched_viewport_pixels=1,
            cursor_journal=dict(all_refusals_accounted=True),cleanup=dict(verified=True,normal_quit=True),
            pair={},provider='synthetic-admission-control-only')
            for i,(p,s) in enumerate(( (p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600))))]
    def test_only_two_named_rows_change(self):
        rows=close_rows(self.plan,self.parent,self.cases)
        self.assertEqual(sum(r['state']=='passed' for r in rows),104)
        self.assertEqual([r for r in rows if r['id']!=LEAF],
                         [r for r in self.parent['requirements'] if r['id']!=LEAF])
    def test_missing_actual_domain_refuses(self):
        with self.assertRaises(ValueError):close_rows(self.plan,self.parent,self.cases[:3])
    def test_relabelled_configuration_refuses(self):
        self.cases[0]['domain']=self.cases[1]['domain']
        with self.assertRaises(ValueError):close_rows(self.plan,self.parent,self.cases)
    def test_repeated_acquisition_refuses(self):
        self.cases[0]['identity']=self.cases[1]['identity']
        with self.assertRaises(ValueError):close_rows(self.plan,self.parent,self.cases)
    def test_missing_settled_output_refuses(self):
        self.cases[0]['presentations'].pop()
        with self.assertRaises(ValueError):close_rows(self.plan,self.parent,self.cases)
    def test_unaccounted_cursor_refusal_blocks_credit(self):
        self.cases[0]['cursor_journal']['all_refusals_accounted']=False
        with self.assertRaises(ValueError):close_rows(self.plan,self.parent,self.cases)
    def test_unreviewed_parent_row_cannot_be_imported(self):
        self.parent['passed_requirements']=103
        with self.assertRaises(ValueError):close_rows(self.plan,self.parent,self.cases)


class AuxiliaryMarkers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            refs=r.json(MATRIX)['comparisons'];cls.cases=[]
            for index,g,selected in ((1,247,1),(2,149,0)):
                c=r.json(refs[index]['record']);a=r.json(c['acquisition']['record']);catalog,_=marker_catalog(r,a,c)
                for generation,ch,stream,request in records(BytesSource(artifact(r,a,'worlds.bin'))):
                    if not ch and generation==g:
                        rows=module('inspect-canvas-producers').decode(stream,complete=False)[1]
                        cls.cases.append((rows,request,selected,catalog));break
    def test_other_exact_frames_are_retained_without_corner_credit(self):
        for rows,request,selected,catalog in self.cases:
            corners,_,aux=marker_request(rows,request,selected,catalog)
            self.assertEqual(len(corners),4*selected)
            self.assertTrue(aux);self.assertTrue(all(a['index']>=4 for a in aux))
    def test_other_frames_cannot_replace_missing_corner(self):
        rows,request,selected,catalog=self.cases[0]
        bad=[(f,b) for f,b in rows if not (f[22]==0x500845 and catalog.get(sha(b[:f[19]]),{}).get('index')==0)]
        with self.assertRaises(ValueError):marker_request(bad,request,selected,catalog)
    def test_unknown_auxiliary_resource_refuses(self):
        rows,request,selected,catalog=self.cases[1];bad=[];changed=False
        for f,b in rows:
            if f[22]==0x500845 and not changed:
                body=bytearray(b);body[-1]^=1;b=bytes(body);changed=True
            bad.append((f,b))
        self.assertTrue(changed)
        with self.assertRaises(ValueError):marker_request(bad,request,selected,catalog)
    def test_auxiliary_resource_with_wrong_caller_refuses(self):
        rows,request,selected,catalog=self.cases[1];bad=[]
        for f,b in rows:
            f=list(f)
            if f[22]==0x500845:f[22]+=1
            bad.append((tuple(f),b))
        with self.assertRaises(ValueError):marker_request(bad,request,selected,catalog)


class MovingOriginalAnchors(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            ref=r.json(MATRIX)['comparisons'][1];c=r.json(ref['record']);a=r.json(c['acquisition']['record'])
            catalog,_=marker_catalog(r,a,c);frames=decode(artifact(r,a,'frame-input.bin'))
            pair=next(p for p in c['proof']['selection_pairs'] if p['mode']=='sharp' and p['case']=='view-1')
            cls.pair=original_pair(pair,c['proof'],frames['rows'],frames['nonce'])
            wanted={row['frame']['generation']:row for s in cls.pair['states'] for row in s['observations']}
            cls.packets={}
            for g,ch,stream,request in records(BytesSource(artifact(r,a,'worlds.bin'))):
                if not ch and g in wanted:
                    rows=module('inspect-canvas-producers').decode(stream,complete=False)[1]
                    cls.packets[g]=source_state(rows,request,wanted[g],catalog)
                if len(cls.packets)==6:break
    def test_actual_changed_anchors_have_each_frames_four_corner_contrasts(self):
        selected=next(s for s in self.pair['states'] if s['selected'])['generations']
        self.assertGreater(len({tuple(self.packets[g]['geometry']) for g in selected}),1)
        words,proof=contrast_pair(self.pair,self.packets,[640,480])
        self.assertEqual(len(words),6);self.assertEqual(len(proof['corners']),12)
        self.assertTrue(all(len(c['samples'])>=4 for c in proof['corners']))
    def test_opaque_black_source_words_can_be_visible_selection_contrast(self):
        words,proof=contrast_pair(self.pair,self.packets,[640,480])
        self.assertTrue(any(word==0 for points in words.values() for word in points.values()))
    def test_one_unproven_selected_generation_blocks_pair(self):
        packets=copy.deepcopy(self.packets);g=next(s for s in self.pair['states'] if s['selected'])['generations'][-1]
        packets[g]['writers'][:]=0
        with self.assertRaises(ValueError):contrast_pair(self.pair,packets,[640,480])
    def test_one_unproven_unselected_generation_blocks_pair(self):
        packets=copy.deepcopy(self.packets);g=next(s for s in self.pair['states'] if not s['selected'])['generations'][-1]
        packets[g]['writers'][:]=0
        with self.assertRaises(ValueError):contrast_pair(self.pair,packets,[640,480])
    def test_dynamic_anchor_does_not_allow_camera_change(self):
        packets=copy.deepcopy(self.packets);next(iter(packets.values()))['camera']['view']=99
        with self.assertRaises(ValueError):contrast_pair(self.pair,packets,[640,480])
    def test_missing_one_frames_corner_blocks_pair(self):
        packets=copy.deepcopy(self.packets);g=next(s for s in self.pair['states'] if s['selected'])['generations'][-1]
        packets[g]['corners'].pop()
        with self.assertRaises(ValueError):contrast_pair(self.pair,packets,[640,480])


class BlendedOriginalBackground(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            ref=r.json(MATRIX)['comparisons'][1];c=r.json(ref['record']);a=r.json(c['acquisition']['record'])
            catalog,_=marker_catalog(r,a,c);frames=decode(artifact(r,a,'frame-input.bin'))
            pair=next(p for p in c['proof']['selection_pairs'] if p['mode']=='integer' and p['case']=='view-0')
            cls.pair=original_pair(pair,c['proof'],frames['rows'],frames['nonce'])
            wanted={row['frame']['generation']:row for s in cls.pair['states'] for row in s['observations']};cls.packets={}
            for g,ch,stream,request in records(BytesSource(artifact(r,a,'worlds.bin'))):
                if not ch and g in wanted:
                    rows=module('inspect-canvas-producers').decode(stream,complete=False)[1]
                    cls.packets[g]=source_state(rows,request,wanted[g],catalog)
                if len(cls.packets)==6:break
    def test_actual_blended_off_background_supplies_all_four_corner_contrasts(self):
        words,proof=contrast_pair(self.pair,self.packets,[640,480])
        self.assertEqual(len(proof['corners']),12)
        self.assertTrue(all(len(c['samples'])>=4 for c in proof['corners']))
        self.assertTrue(any(p['history'][-1]['mode']==2 and len(p['history'])>1
            for g in proof['source_histories'] for p in g['points']))
    def test_missing_opaque_predecessor_cannot_assume_black(self):
        words,proof=contrast_pair(self.pair,self.packets,[640,480])
        item=next((g,p) for g in proof['source_histories'] for p in g['points'] if p['history'][-1]['mode']==2)
        g,p=item;packet=copy.copy(self.packets[g['generation']])
        packet['rasters']=[(f,b) for f,b in packet['rasters'] if f[14] not in (0,7)]
        self.assertNotIn(p['point'],resolve_points(packet,{p['point']},[640,480]))
    def test_recovered_word_rounding_and_unknown_history(self):
        self.assertEqual(blend_word(1,0xffff,0),0x7bef)
        self.assertEqual(blend_word(1,0xffff,0xffff),0xf7de)
        self.assertEqual(blend_word(2,0xffff,0),0x39e7)
        self.assertEqual(blend_word(2,0,0xffff),0xb5d6)
        self.assertEqual(blend_word(4,0xffff,0),0xb5d6)
        self.assertEqual(blend_word(5,None,0xffff),0x7bef)
        for mode in (1,2,4,5):self.assertIsNone(blend_word(mode,0xffff,None))
        self.assertIsNone(blend_word(3,0xffff,0xffff))


class CoordinateProvenance(unittest.TestCase):
    def history(self):
        h=Lineage(PartialWorlds(2,[4,3],{1:{5:0xffff},2:{5:0x07e0}}));self.seq=0
        self.apply(h,1,struct.pack('<7I',1,4,3,16,0xf800,0x7e0,31)+bytes(24))
        self.apply(h,1,struct.pack('<7I',2,4,3,16,0xf800,0x7e0,31)+bytes(24))
        self.apply(h,19,struct.pack('<5I',1,1,0,7,0))
        self.apply(h,3,struct.pack('<10I',1,2,0,0,4,3,0,0,0,0))
        return h
    def apply(self,h,op,body):
        self.seq+=1;h.apply(op,self.seq,body)
    def paint(self,world=1):
        return dict(world=str(world),paint='1',lease='7',mode=0,surface=2,sealed_world='0',
            frame_width=4,frame_height=3,width=4,height=3,image_width=4,image_height=3,left=0,top=0,scaling=2)
    def pixels(self):
        raw=np.zeros((3,4,4),dtype='uint8');raw[:,:,3]=255;raw[1,1]=255;return raw.tobytes()
    def test_named_source_word_maps_to_exact_viewport_pixel(self):
        h=self.history();self.apply(h,6,struct.pack('<I',2))
        self.assertEqual(viewport_points(h,self.paint(),{5:0xffff},self.pixels())['viewport_pixels'],1)
    def test_known_opaque_black_is_not_unknown_placeholder_zero(self):
        h=self.history();h.worlds.expected[2][5]=0
        self.apply(h,19,struct.pack('<5I',1,2,0,7,0))
        self.apply(h,3,struct.pack('<10I',1,2,0,0,4,3,0,0,0,0));self.apply(h,6,struct.pack('<I',2))
        raw=np.zeros((3,4,4),dtype='uint8');raw[:,:,3]=255
        self.assertEqual(viewport_points(h,self.paint(2),{5:0},raw.tobytes())['viewport_pixels'],1)
    def test_native_pixel_mismatch_always_refuses(self):
        h=self.history();self.apply(h,6,struct.pack('<I',2));raw=bytearray(self.pixels());raw[20]=0
        with self.assertRaises(ValueError):viewport_points(h,self.paint(),{5:0xffff},bytes(raw))
    def test_presentation_snapshot_outlives_mutable_world_source(self):
        h=self.history();self.apply(h,6,struct.pack('<I',2));self.apply(h,7,struct.pack('<I',1))
        self.assertIsNone(h.sealed)
        self.assertEqual(viewport_points(h,self.paint(),{5:0xffff},self.pixels())['generation'],1)
    def test_new_unpresented_world_does_not_relabel_old_snapshot(self):
        h=self.history();self.apply(h,6,struct.pack('<I',2));self.apply(h,19,struct.pack('<5I',1,2,0,7,0))
        self.assertEqual(viewport_points(h,self.paint(),{5:0xffff},self.pixels())['generation'],1)
        with self.assertRaises(ValueError):viewport_points(h,self.paint(2),{5:0x7e0},self.pixels())
    def test_unknown_keyed_copy_cannot_preserve_provenance_by_placeholder_zero(self):
        h=self.history()
        self.apply(h,3,struct.pack('<10I',1,2,0,1,1,2,1,1,1,0));self.apply(h,6,struct.pack('<I',2))
        with self.assertRaises(ValueError):viewport_points(h,self.paint(),{5:0xffff},self.pixels())
    def test_known_transparent_key_preserves_actual_previous_world(self):
        h=self.history();self.apply(h,1,struct.pack('<7I',3,4,3,16,0xf800,0x7e0,31)+bytes(24))
        self.apply(h,3,struct.pack('<10I',3,2,0,0,4,3,0,0,1,0));self.apply(h,6,struct.pack('<I',2))
        self.assertEqual(viewport_points(h,self.paint(),{5:0xffff},self.pixels())['viewport_pixels'],1)
    def test_equal_colour_native_upload_does_not_gain_original_provenance(self):
        h=self.history();self.apply(h,2,struct.pack('<5I',2,1,1,1,1)+b'\xff\xff');self.apply(h,6,struct.pack('<I',2))
        with self.assertRaises(ValueError):viewport_points(h,self.paint(),{5:0xffff},self.pixels())
    def test_masked_noop_preserves_source_but_masked_write_erases_it(self):
        h=self.history();self.apply(h,20,struct.pack('<5I',2,1,1,1,1)+b'\xff\xff\x00');self.apply(h,6,struct.pack('<I',2))
        self.assertEqual(viewport_points(h,self.paint(),{5:0xffff},self.pixels())['viewport_pixels'],1)
        self.apply(h,20,struct.pack('<5I',2,1,1,1,1)+b'\xff\xff\x01');self.apply(h,6,struct.pack('<I',2))
        with self.assertRaises(ValueError):viewport_points(h,self.paint(),{5:0xffff},self.pixels())
    def test_shifted_surface_does_not_match_source_point_without_lineage(self):
        h=self.history();self.apply(h,3,struct.pack('<10I',1,2,0,0,3,3,1,0,0,0));self.apply(h,6,struct.pack('<I',2))
        raw=np.zeros((3,4,4),dtype='uint8');raw[:,:,3]=255;raw[1,2]=255
        self.assertTrue(viewport_points(h,self.paint(),{5:0xffff},raw.tobytes()))
    def test_wrong_surface_identity_refuses(self):
        h=self.history();self.apply(h,6,struct.pack('<I',2));p=self.paint();p['surface']=1
        with self.assertRaises(ValueError):viewport_points(h,p,{5:0xffff},self.pixels())
    def test_sharp_nearest_projection_preserves_named_pixel(self):
        h=self.history();self.apply(h,6,struct.pack('<I',2));p=self.paint();p.update(width=8,height=6,image_width=8,image_height=6,scaling=0)
        raw=np.zeros((6,8,4),dtype='uint8');raw[:,:,3]=255;raw[2:4,2:4]=255
        self.assertEqual(viewport_points(h,p,{5:0xffff},raw.tobytes())['viewport_pixels'],4)


if __name__=='__main__':unittest.main()
