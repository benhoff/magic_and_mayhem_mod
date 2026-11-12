#!/usr/bin/env python3
"""Refuse stale, unrelated and misprojected picking witnesses."""
from copy import deepcopy
import ast
from types import SimpleNamespace
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_picking_resize import targets, ready, physical_point
from rendering_pilot import require


def case():
    actor = dict(slot=0, type=0, owner=0, active=1, health=100, pool=1)
    baseline = dict(ordinal=1, kind=1, tick=100, frame=dict(generation=10, nonce='123'),
                    cursor=dict(poll=20), tracked=actor)
    callback = dict(ordinal=2, kind=3, call=1, mask=16, mouse=[100,100,0,0,0,0,0,0,0,0])
    row = dict(ordinal=3, kind=1, tick=130, frame=dict(generation=11, nonce='123'),
               thread=7, world_return=101, cursor=dict(poll=21, tick=120, point=[100,100]),
               tracked=actor, hit=dict(actor), camera=[0])
    admission = dict(sequence=4, kind=1, thread=7, world_return_ordinal=101,
                     flags=168, input_active=1, x=100, y=100,
                     ground=[5,62,4], ground_bounds=[80,80,24])
    paint = dict(world='11', paint='5', native_gpu_frame=True, gpu_read_complete=True,
                 pixels_captured=True, presentation_owner='gui')
    return [baseline, callback, row], [admission], paint, dict(point=[100,100], expected_cell_xy=[5,62]), baseline


class Picking(unittest.TestCase):
    def test_exact_original_return_object(self):
        args = case()
        self.assertEqual(ready(*args, 'object')['world_return'], 101)

    def test_exact_original_return_cell(self):
        args = case()
        self.assertEqual(ready(*args, 'cell')['ground'], [5,62,4])

    def test_wrong_ground_cannot_pass(self):
        args = case(); args[1][0]['ground'][0] += 1
        self.assertIsNone(ready(*args, 'cell'))

    def test_stale_and_unrelated_observations_refuse(self):
        mutations = [lambda a:a[1][0].update(world_return_ordinal=100),
                     lambda a:a[1][0].update(thread=8),
                     lambda a:a[0][-1]['cursor'].update(poll=20),
                     lambda a:a[0][-1]['cursor'].update(point=[99,100]),
                     lambda a:a[0][-1]['frame'].update(nonce='456'),
                     lambda a:a[0][-1]['hit'].update(slot=1),
                     lambda a:a[1][0].update(input_active=0),
                     lambda a:a[1][0].update(ground=[80,62,4]),
                     lambda a:a[2].update(gpu_read_complete=False),
                     lambda a:a[2].update(presentation_owner='worker'),
                     lambda a:a[1].append(deepcopy(a[1][0])),
                     lambda a:a[0][1].update(mask=1),
                     lambda a:a[0][1].update(mouse=[100,100,0,1,0,0,0,0,0,0])]
        for n, mutate in enumerate(mutations):
            with self.subTest(n=n):
                args = case(); mutate(args)
                self.assertIsNone(ready(*args, 'object'))

    def test_foot_comes_from_actual_opaque_source(self):
        frame = bytearray(83)
        struct.pack_into('<5I', frame, 0, len(frame),6,3,0xfffffffc,0xfffffffd)
        for y in range(3):struct.pack_into('<II',frame,40+y*8,64+y*3,73)
        frame[64:73]=bytes([1,4,1,1,4,1,2,2,2])
        f=[0]*24;f[2]=9;f[8:10]=[100,200];f[15]=1;f[19]=len(frame);f[22]=0x500512
        body,foot=targets([(tuple(f),bytes(frame))],[100,200,0],[5,62,5])
        self.assertEqual(foot['point'],[106,205])
        self.assertEqual(foot['opaque_pixel'],[2,2])
        self.assertEqual(foot['expected_cell_xy'],[5,62])
        self.assertNotEqual(body['point'],foot['point'])

    def test_transparent_lower_row_is_not_a_foot(self):
        frame = bytearray(83)
        struct.pack_into('<5I',frame,0,len(frame),6,3,0xfffffffc,0xfffffffd)
        for y in range(3):struct.pack_into('<II',frame,40+y*8,64+y*3,73)
        frame[64:73]=bytes([1,4,1,1,4,1,6,0,0])
        f=[0]*24;f[2]=9;f[8:10]=[100,200];f[15]=1;f[19]=len(frame);f[22]=0x500512
        _,foot=targets([(tuple(f),bytes(frame))],[100,200,0],[5,62,5])
        self.assertEqual(foot['opaque_pixel'][1],1)

    def test_physical_display_pixel(self):
        p=dict(width=1100,height=795,frame_width=640,frame_height=480,
               left=20,top=0,image_width=1060,image_height=795)
        pointer=dict(viewport=[10,10,1100,795],screen=[196,176])
        self.assertEqual(physical_point(pointer,p,[100,100]),[100,100])
        with self.assertRaises(ValueError):physical_point(pointer,p,[101,100])

    def test_main_preferences_route_recreates_both_actual_sizes(self):
        # Execute the real controller method against a strict public-control
        # model. This catches the former Quick -> Mini assumption before launch.
        tree=ast.parse((Path(__file__).resolve().parents[1]/'tools/run-rendering-picking-resize.py').read_text())
        method=next(n for n in ast.walk(tree) if isinstance(n,ast.FunctionDef) and n.name=='rebuild')
        namespace=dict(base=SimpleNamespace(require=require),time=__import__('time'))
        exec(compile(ast.Module(body=[method],type_ignores=[]),'<actual rebuild controller>','exec'),namespace)
        for low in (True,False):
            with self.subTest(low=low):
                owner=SimpleNamespace(size=[800,600] if low else [640,480],rebuilds=[],last={'maps':['Celtic Forest One (small)']})
                events=[];states=iter([26,22,3,10,3,22,14,25,14,7]);fresh=[False]
                def wait(screen,**kw):
                    self.assertEqual(screen,next(states))
                    events.append(('wait',screen))
                    return dict(screen=screen,preferences=[0,0,int(not low)],setup=dict(map=1,map_name=owner.last['maps'][0]))
                def click(name):
                    events.append(('click',name))
                    if name=='spellboxOK':fresh[0]=True
                owner.wait=wait;owner.click=owner.click_id=click
                owner.held_key=lambda *a,**k:events.append(('key',a[0]))
                owner.key=lambda *a:events.append(('key',a[0]))
                owner.capture=lambda *a:None
                owner.child=SimpleNamespace(poll=lambda:None)
                owner.viewport=SimpleNamespace(poll=lambda:dict(world=30 if fresh[0] else 20,frame_width=640 if low else 800,frame_height=480 if low else 600))
                owner.latest=lambda:dict(frame=dict(generation=30 if fresh[0] else 20),high_resolution=int(not low) if fresh[0] else int(low))
                selected=SimpleNamespace(get_name=lambda:owner.last['maps'][0])
                node=SimpleNamespace(get_selection_iface=lambda:SimpleNamespace(get_selected_child=lambda i:selected))
                owner.access=SimpleNamespace(nodes=lambda:[(node,dict(role='list',rect=[0,0,100,100]))])
                owner.input=SimpleNamespace(click=lambda rect:None)
                namespace['rebuild'](owner,low)
                self.assertEqual(list(states),[])
                self.assertEqual(owner.size,[640,480] if low else [800,600])
                self.assertFalse(owner.rebuilds[0]['same_world'])
                self.assertIn(('click','preferencesRadio2' if low else 'preferencesRadio1'),events)
                self.assertNotIn(('wait',17),events)


if __name__ == '__main__':unittest.main()
