#!/usr/bin/env python3
"""Independent geometry/bitmap refusal controls; synthetic rows earn no game credit."""
import copy
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from cursor_hotspot import descriptors,geometry,points,acceptance_points,bitmap_words,original_geometry
from cursor_hotspot_analysis import check_copy,digest,cursor_draws,require_copy_observation,bind_original_copy,readiness_states,cursor_viewport
from rendering_pilot import rgba
from rendering_interaction_budget import admit
import numpy as np


def config():
    text='[HEADER]\nBitmapWidth=400\nBitmapHeight=280\n'
    special={6:([19,0],3),7:([39,19],5),9:([0,19],4)}
    for n in range(22):
        hot,frame=special.get(n,([0,0],0))
        text+=f'[CURSOR_{n}]\nHotspotx={hot[0]}\nHotspoty={hot[1]}\nFrames={frame}\nDelay=0; comment\n'
    return text.encode()


def bitmap(bits=24,topdown=False,padding=0):
    stride=((400*bits+31)//32)*4;offset=54+(1024 if bits==8 else 0)
    raw=bytearray(offset+stride*280+padding);raw[:2]=b'BM'
    struct.pack_into('<I',raw,2,len(raw));struct.pack_into('<I',raw,10,offset)
    struct.pack_into('<IiiHHIIiiII',raw,14,40,400,-280 if topdown else 280,1,bits,0,stride*280,0,0,0,0)
    y=0 if topdown else 279
    if bits==24:raw[offset+y*stride:offset+y*stride+3]=b'\x00\x00\xff'
    else:
        raw[58:62]=b'\x00\x00\xff\x00';raw[offset+y*stride]=1
    return bytes(raw)


class Hotspot(unittest.TestCase):
    def setUp(self):self.d=descriptors(config())
    def test_explicit_nonzero_original_descriptors(self):
        self.assertEqual(self.d[7],dict(hotspot=[39,19],frames=[5],delay=0))
    def test_inline_comments_without_whitespace(self):
        self.assertEqual(self.d[6]['delay'],0)
    def test_wrong_nonzero_descriptor_refuses(self):
        with self.assertRaises(ValueError):descriptors(config().replace(b'Hotspotx=39',b'Hotspotx=38'))
    def test_missing_required_descriptor_refuses(self):
        with self.assertRaises(KeyError):descriptors(config().replace(b'[CURSOR_7]',b'[CURSOR_77]'))
    def test_hotspot_is_subtracted_from_pointer(self):
        self.assertEqual(geometry([639,240],self.d[7],[640,480]),dict(point=[639,240],origin=[600,221],
            saved=[600,221,640,261],destination=[600,221,640,261],source=[200,0,240,40],frame=5))
    def test_negative_origin_clips_source_and_destination_together(self):
        g=geometry([0,0],self.d[7],[640,480])
        self.assertEqual(g['origin'],[-39,-19]);self.assertEqual(g['destination'],[0,0,1,21])
        self.assertEqual(g['source'],[239,19,240,40])
    def test_bottom_right_crop(self):
        g=geometry([639,479],self.d[6],[640,480])
        self.assertEqual(g['source'],[120,0,140,1]);self.assertEqual(g['saved'],[620,479,640,480])
    def test_outside_pointer_refuses(self):
        for p in ([-1,0],[640,0],[0,480]):
            with self.assertRaises(ValueError):geometry(p,self.d[7],[640,480])
    def test_animated_frame_cannot_be_guessed(self):
        d=dict(hotspot=[0,0],frames=[1,2],delay=2)
        with self.assertRaises(ValueError):geometry([3,4],d,[640,480])
        self.assertEqual(geometry([3,4],d,[640,480],2)['frame'],2)
    def test_unselected_frame_refuses(self):
        with self.assertRaises(ValueError):geometry([3,4],self.d[7],[640,480],4)
    def row(self):
        return dict(kind=1,frame=dict(generation=8,nonce='123',owner=7),cursor=dict(point=[639,240],poll=9),
                    saved_cursor=[1,600,221],cursor_type=7,high_resolution=0,ordinal=10)
    def test_actual_state_must_agree_with_descriptor_geometry(self):
        self.assertEqual(original_geometry(self.row(),self.d,[640,480])['origin'],[600,221])
    def test_wrong_saved_origin_refuses(self):
        row=self.row();row['saved_cursor'][1]+=1
        with self.assertRaises(ValueError):original_geometry(row,self.d,[640,480])
    def test_missing_original_save_or_point_refuses(self):
        for key in ('saved_cursor','cursor','frame'):
            row=self.row();row[key]=None
            with self.assertRaises(ValueError):original_geometry(row,self.d,[640,480])
    def test_wrong_resolution_refuses(self):
        with self.assertRaises(ValueError):original_geometry(self.row(),self.d,[800,600])
    def test_prescribed_points_are_bounded_for_both_sizes(self):
        for size in ([640,480],[800,600]):
            self.assertEqual(len(points(size)),17)
            self.assertTrue(all(0<=p[i]<size[i] for name,p in points(size) for i in (0,1)))


class Bitmap(unittest.TestCase):
    def test_24bit_bottom_up_and_top_down(self):
        for topdown in (False,True):self.assertEqual(int(bitmap_words(bitmap(topdown=topdown))[0,0]),0xf800)
    def test_indexed_palette_and_black_are_independent_bytes(self):
        image=bitmap_words(bitmap(8));self.assertEqual(int(image[0,0]),0xf800);self.assertEqual(int(image[0,1]),0)
    def test_original_zero_file_alignment_padding(self):
        self.assertEqual(int(bitmap_words(bitmap(padding=2))[0,0]),0xf800)
    def test_nonzero_trailing_data_refuses(self):
        raw=bytearray(bitmap(padding=2));raw[-1]=1
        with self.assertRaises(ValueError):bitmap_words(bytes(raw))
    def test_truncated_plane_refuses(self):
        with self.assertRaises(ValueError):bitmap_words(bitmap()[:-1])
    def test_compressed_bitmap_refuses(self):
        raw=bytearray(bitmap());struct.pack_into('<I',raw,30,1)
        with self.assertRaises(ValueError):bitmap_words(bytes(raw))
    def test_wrong_atlas_dimensions_refuse(self):
        raw=bytearray(bitmap());struct.pack_into('<i',raw,18,399)
        with self.assertRaises(ValueError):bitmap_words(bytes(raw))


class AttemptBudget(unittest.TestCase):
    def case(self,name,repair=None,games=0,proved=False):
        return dict(report=name,repair_of=repair,result=dict(original_game_runs=games,success=proved),
            proved=proved,comparison_failed=False,preparation_fixed=False)
    def test_every_zero_game_request_remains_in_chain(self):
        cases=[self.case('a'),self.case('b','a'),self.case('c','b')]
        r=admit(cases,'c',8,6)
        self.assertEqual(r['requests_before'],3);self.assertEqual(r['original_games_before'],0)
        self.assertEqual(r['repair_ancestors'],['a','b','c'])
    def test_failure_blocks_unrelated_expansion(self):
        with self.assertRaises(ValueError):admit([self.case('a')],None,8,6)
    def test_unknown_start_cannot_refund_a_failure(self):
        with self.assertRaises(ValueError):admit([self.case('a',games=None)],'a',8,6)
    def test_eight_requests_exhaust_even_without_games(self):
        with self.assertRaises(ValueError):admit([self.case(str(n),str(n-1) if n else None) for n in range(8)],'7',8,6)
    def test_six_starts_exhaust_even_with_success(self):
        with self.assertRaises(ValueError):admit([self.case(str(n),games=1,proved=True) for n in range(6)],None,8,6)
    def test_missing_failure_ancestor_refuses(self):
        with self.assertRaises(ValueError):admit([self.case('b','a')],'b',8,6)


class CopyHistory(unittest.TestCase):
    def setUp(self):
        self.s=np.array([[1,0],[2,3]],dtype='uint32')
        self.b=np.array([[9,9],[9,9]],dtype='uint32')
        self.a=np.array([[1,9],[2,3]],dtype='uint32')
        self.w=[4,5,0,0,2,2,0,0,1,0]
        self.r=dict(words=self.w+[0,0,0],mismatches=0,source_width=2,source_height=2,
            target_width=2,target_height=2,pixels=4,source_sha256=digest(self.s),
            before_sha256=digest(self.b),actual_sha256=digest(self.a),expected_sha256=digest(self.a))
    def check(self):check_copy(self.r,self.w,self.s,self.b,self.a)
    def test_independent_keyed_output_includes_preserved_background(self):self.check()
    def test_native_self_agreement_does_not_replace_original_reference(self):
        self.r['actual_sha256']=self.r['expected_sha256']='0'*64
        with self.assertRaises(ValueError):self.check()
    def test_wrong_reference_source_refuses(self):
        self.s[0,0]+=1
        with self.assertRaises(ValueError):self.check()
    def test_wrong_saved_background_refuses(self):
        self.b[0,1]+=1
        with self.assertRaises(ValueError):self.check()
    def test_opaque_zero_overwrite_refuses(self):
        self.a[0,1]=0
        with self.assertRaises(ValueError):self.check()
    def test_wrong_command_key_refuses(self):
        self.r['words'][8]=0
        with self.assertRaises(ValueError):self.check()
    def test_partial_destination_comparison_refuses(self):
        self.r['pixels']=3
        with self.assertRaises(ValueError):self.check()
    def test_wrong_source_extent_refuses(self):
        self.r['source_width']=3
        with self.assertRaises(ValueError):self.check()
    def test_mismatch_counter_cannot_be_ignored(self):
        self.r['mismatches']=1
        with self.assertRaises(ValueError):self.check()


class OriginalAnimatedCursor(unittest.TestCase):
    def setUp(self):
        self.d=descriptors(config());self.d[1]=dict(hotspot=[0,0],frames=list(range(24,36)),delay=2)
        self.row=dict(kind=1,frame=dict(generation=8,nonce='123',owner=7),cursor=dict(point=[300,275],poll=9),
            saved_cursor=[1,300,275],cursor_type=1,high_resolution=0,ordinal=10)
        self.owner=[0]*44;self.owner[24:28]=[360,80,400,120];self.owner[28:32]=[300,275,340,315]
        self.owner[42:44]=[10,11]
        self.copy=dict(sequence=3,world=8,atlas=True,words=[10,11,360,80,400,120,300,275,1,0])
    def check(self):return cursor_draws(self.row,self.d,[640,480],[self.copy],{3:self.owner})
    def test_actual_move_class_uses_original_requested_animation_frame(self):
        self.assertEqual(self.check()[0]['frame'],29)
    def test_wrong_native_tile_refuses_even_with_valid_original_request(self):
        self.copy['words'][2]=320
        with self.assertRaises(ValueError):self.check()
    def test_frame_outside_actual_class_descriptor_refuses(self):
        self.d[1]['frames']=[24]
        with self.assertRaises(ValueError):self.check()
    def test_wrong_original_saved_origin_refuses(self):
        self.row['saved_cursor'][1]+=1
        with self.assertRaises(ValueError):self.check()
    def test_wrong_original_source_rectangle_refuses(self):
        self.owner[24]+=1
        with self.assertRaises(ValueError):self.check()
    def test_native_atlas_shape_cannot_replace_independent_bitmap_identity(self):
        self.copy['atlas']=False
        with self.assertRaises(ValueError):self.check()
    def test_another_world_cannot_supply_the_cursor(self):
        self.copy['world']=7
        with self.assertRaises(ValueError):self.check()
    def test_correct_frame_has_no_implicit_guess_without_original_copy(self):
        with self.assertRaises(ValueError):original_geometry(self.row,self.d,[640,480])
    def test_wrong_original_destination_refuses(self):
        self.owner[28]+=1
        with self.assertRaises(ValueError):self.check()
    def test_reduced_schedule_preserves_all_three_required_edge_classes(self):
        self.assertEqual([n for n,p in acceptance_points([640,480])],['center','frame-top','frame-right','frame-left'])


class ScopedCopyOutput(unittest.TestCase):
    def test_missing_cursor_output_cannot_be_hidden_by_viewport_agreement(self):
        with self.assertRaises(ValueError):require_copy_observation(11,{10,12},(10,12))
    def test_observation_interval_is_inclusive(self):
        for seq in [10,12]:
            with self.assertRaises(ValueError):require_copy_observation(seq,set(),(10,12))
    def test_missing_menu_output_is_outside_the_cursor_claim(self):
        for seq in [9,13]:require_copy_observation(seq,set(),(10,12))
    def setUp(self):
        self.v=dict(words=[10,11,200,0,240,40,600,221,1,0],source_extent=[400,280],target_extent=[640,480])
        self.o=[0]*44;self.o[10]=64;self.o[12]=0x8000;self.o[14]=1;self.o[15]=1
        self.o[16:18]=[400,280];self.o[19:21]=[640,480];self.o[24:28]=[200,0,240,40]
        self.o[28:32]=[600,221,640,261];self.o[42:44]=[10,11]
    def check(self):bind_original_copy(self.v,self.o)
    def test_original_input_geometry_binds_complete_independent_output(self):self.check()
    def test_wrong_original_key_refuses(self):
        self.o[22]=1
        with self.assertRaises(ValueError):self.check()
    def test_missing_original_key_authority_refuses(self):
        self.o[10]=0
        with self.assertRaises(ValueError):self.check()
    def test_wrong_original_resource_refuses(self):
        self.o[42]+=1
        with self.assertRaises(ValueError):self.check()
    def test_shifted_native_geometry_cannot_define_original_expectation(self):
        self.v['words'][6]+=1
        with self.assertRaises(ValueError):self.check()
    def test_refused_original_copy_earns_no_output_credit(self):
        self.o[13]=0x88760096
        with self.assertRaises(ValueError):self.check()
    def test_unproved_alias_cannot_enter_hotspot_scope(self):
        self.o[10]|=128
        with self.assertRaises(ValueError):self.check()


class Readiness(unittest.TestCase):
    def setUp(self):
        self.rows=[dict(kind=1,ordinal=n,frame=dict(generation=n),cursor=dict(poll=n),
            cursor_type=6,saved_cursor=[1,301,0]) for n in (1,2,3)]
        entries=[dict(generation=n,ordinal=n,poll=n,cursor_class=6,saved=[1,301,0],paint=str(n)) for n in (1,2,3)]
        self.a=dict(actions=[dict(action='hotspot-readiness',name='top',host_ns=350000,observations=entries)])
        self.w=dict(released=dict(host_us='100',frame_input_row=0),settled=dict(host_us='400',frame_input_row=3))
        self.o=dict(swaps=[1,2,3],paints={n:(dict(world=str(n),host_us=str(100+50*n)),b'') for n in (1,2,3)})
    def check(self):return readiness_states(self.a,self.rows,self.o,'top',self.w)
    def test_exact_displayed_live_states_are_retained(self):self.assertEqual(len(self.check()),3)
    def test_later_action_paint_cannot_replace_held_pointer_state(self):
        self.o['paints'][3][0]['host_us']='351'
        with self.assertRaises(ValueError):self.check()
    def test_readiness_requires_an_actual_swap(self):
        self.o['swaps'].remove(3)
        with self.assertRaises(ValueError):self.check()
    def test_stale_poll_identity_refuses(self):
        self.rows[-1]['cursor']['poll']+=1
        with self.assertRaises(ValueError):self.check()
    def test_duplicate_source_generation_refuses(self):
        self.a['actions'][0]['observations'][-1]['generation']=2
        with self.assertRaises(ValueError):self.check()
    def test_changed_original_saved_origin_refuses(self):
        self.rows[-1]['saved_cursor'][1]+=1
        with self.assertRaises(ValueError):self.check()


class CursorStencil(unittest.TestCase):
    def setUp(self):
        d=descriptors(config());self.g=geometry([320,0],d[6],[640,480])
        self.atlas=np.zeros((280,400),dtype='uint32')
        tile=np.arange(1,1601,dtype='uint32').reshape(40,40);self.atlas[:40,120:160]=tile
        plane=np.zeros((480,640),dtype='uint32');dest=plane[:40,301:341];dest[tile!=31]=tile[tile!=31]
        self.raw=rgba(plane,16,(0xf800,0x7e0,31),None).tobytes()
        self.p=dict(width=640,height=480,frame_width=640,frame_height=480,image_width=640,image_height=480,left=0,top=0)
    def check(self):return cursor_viewport(self.raw,self.p,self.g,self.atlas,31)
    def test_full_visible_opaque_stencil_matches_independent_atlas(self):self.assertEqual(self.check()['viewport_pixels'],1599)
    def test_shifted_hotspot_refuses(self):
        self.g['destination'][0]+=1;self.g['destination'][2]+=1
        with self.assertRaises(ValueError):self.check()
    def test_wrong_original_key_refuses(self):
        with self.assertRaises(ValueError):cursor_viewport(self.raw,self.p,self.g,self.atlas,0)
    def test_wrong_frame_cannot_be_chosen_from_actual_pixels(self):
        self.g['source']=[160,0,200,40]
        with self.assertRaises(ValueError):self.check()
    def test_changed_visible_cursor_pixel_refuses(self):
        b=bytearray(self.raw);b[(301+2)*4]^=1;self.raw=bytes(b)
        with self.assertRaises(ValueError):self.check()


if __name__=='__main__':unittest.main()
