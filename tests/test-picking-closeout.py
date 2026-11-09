#!/usr/bin/env python3
"""Adverse profile/domain/row admission controls; no execution credit."""
import ast
from copy import deepcopy
import importlib.util
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from picking_closeout import complete_cases, preserved_pilot, quiet_sources, LEAF
from rendering_picking_resize_assessment import source_chunk, picked_cell
from rendering_pilot import require

spec=importlib.util.spec_from_file_location('close_picking',ROOT/'tools/close-rendering-picking-closeout.py')
close=importlib.util.module_from_spec(spec);spec.loader.exec_module(close)


def cases():
    return [dict(success=True,sources_stable=True,comparison_processes_stopped=True,
        original_manifest_verified_before_after=True,
        comparison=dict(success=True,mismatching_pixels=0),software=s,
        acquisition=dict(record='synthetic-only',sha256='0'*64),
        binding=dict(record='synthetic-only',sha256='0'*64),
        proof=dict(verified=True,normal_quit=True,software=s,windows=48,
            object_windows=24,cell_windows=24,displayed_source_witnesses=144,
            live_display_rebuilds=2,same_world_continuity=False)) for s in (False,True)]


class Picking(unittest.TestCase):
    def test_complete_distinct_profiles(self):
        self.assertTrue(complete_cases(cases()))

    def test_missing_duplicate_or_partial_profiles(self):
        for data in (cases()[:1],cases()+cases()[:1],[cases()[0],cases()[0]]):
            with self.assertRaises(ValueError): complete_cases(data)

    def test_profile_boolean_is_not_an_integer(self):
        data=cases();data[1]['software']=1
        with self.assertRaises(ValueError):complete_cases(data)

    def test_every_proof_guard_refuses(self):
        for path,value in [(['success'],False),(['sources_stable'],False),
            (['comparison_processes_stopped'],False),
            (['original_manifest_verified_before_after'],False),
            (['comparison','success'],False),(['comparison','mismatching_pixels'],1),
            (['proof','verified'],False),(['proof','normal_quit'],False),
            (['proof','software'],True),(['proof','windows'],47),
            (['proof','object_windows'],23),(['proof','cell_windows'],23),
            (['proof','displayed_source_witnesses'],143),
            (['proof','live_display_rebuilds'],1),(['proof','same_world_continuity'],True)]:
            with self.subTest(path=path):
                data=cases();target=data[0]
                for key in path[:-1]: target=target[key]
                target[path[-1]]=value
                with self.assertRaises(ValueError):complete_cases(data)

    def test_source_chunk_camera_metadata_is_not_a_world(self):
        f=[0]*24;f[2]=24
        source_chunk([(f,b'')],dict(view=0),True)
        with self.assertRaises(ValueError):source_chunk([(f,b'')],dict(view=0))
        for kind in (11,12,13,27):
            f[2]=kind
            with self.assertRaises(ValueError):source_chunk([(f,b'')],None,True)

    def test_quiet_target_needs_three_distinct_source_poses(self):
        samples=[dict(generation=g,nonce='same',position=[78,65,5],anchor=[286,153],point=[357,243],view=3)for g in (1,2,3)]
        self.assertTrue(quiet_sources(samples))
        for key,value in [('generation',2),('nonce','foreign'),('position',[79,65,5]),('anchor',[297,160]),('point',[358,243]),('view',0)]:
            data=deepcopy(samples);data[-1][key]=value
            with self.subTest(key=key):self.assertFalse(quiet_sources(data))
        self.assertFalse(quiet_sources(samples[:2]))

    def test_scaling_focus_sends_no_scene_button(self):
        tree=ast.parse((ROOT/'tools/run-rendering-picking-closeout.py').read_text())
        method=next(n for n in ast.walk(tree)if isinstance(n,ast.FunctionDef)and n.name=='set_scaling')
        self.assertNotIn('XTestFakeButtonEvent',ast.unparse(method))
        self.assertIn('self.focus_viewport()',ast.unparse(method))

    def test_cells_follow_current_pose_without_hit_feedback(self):
        for point in ([49,28],[49,27]):
            picked_cell(dict(position=[*point,7]),dict(expected_cell_xy=point),dict(ground=[*point,6]))
        with self.assertRaises(ValueError):
            picked_cell(dict(position=[49,27,7]),dict(expected_cell_xy=[49,28]),dict(ground=[49,28,6]))
        with self.assertRaises(ValueError):
            picked_cell(dict(position=[49,28,7]),dict(expected_cell_xy=[49,28]),dict(ground=[49,27,6]))

    def test_retained_pilot_allows_only_exact_host_changes(self):
        name='tools/check-rendering-picking-closeout.py';native='runtime/scene/observer.c'
        old={name:'1'*64,native:'3'*64};current={name:'2'*64,native:'3'*64}
        changes={name:dict(before='1'*64,after='2'*64)}
        c=cases()[0];c['retained_pixel_comparison']=dict(new_original_reference_execution=False,new_pixel_comparison=False)
        self.assertTrue(preserved_pilot(old,current,changes,c))
        for value in (dict(current,**{native:'4'*64}),dict(current,**{'tools/foreign.py':'4'*64})):
            with self.assertRaises(ValueError):preserved_pilot(old,value,changes,c)
        with self.assertRaises(ValueError):preserved_pilot(old,current,{},c)
        c['proof']['normal_quit']=False
        with self.assertRaises(ValueError):preserved_pilot(old,current,changes,c)

    def test_only_two_requested_rows_change(self):
        with Reader(ROOT) as r:
            plan=r.json('docs/native-rendering-closure.json')
            parent=r.json('research/runtime/caller-state-master-results-20261108/admission-01.json')
        before=deepcopy(parent)
        rows=close.close_rows(plan,parent,cases())
        changed=[(a,b) for a,b in zip(parent['requirements'],rows) if a!=b]
        self.assertEqual(parent,before)
        self.assertEqual(len(changed),2)
        self.assertEqual({b['configuration'] for _,b in changed},{'player-default','player-software'})
        self.assertTrue(all(b['id']==LEAF and b['state']=='passed' for _,b in changed))

    def test_rebuild_uses_public_main_preferences_and_declared_map(self):
        tree=ast.parse((ROOT/'tools/run-rendering-picking-closeout.py').read_text())
        method=next(n for n in ast.walk(tree) if isinstance(n,ast.FunctionDef) and n.name=='rebuild')
        ns=dict(base=SimpleNamespace(require=require),time=__import__('time'))
        exec(compile(ast.Module(body=[method],type_ignores=[]),'<actual picking rebuild>','exec'),ns)
        for low in (True,False):
            owner=SimpleNamespace(size=[800,600] if low else [640,480],rebuilds=[])
            order=iter([26,22,3,10,3,22,14,7]);events=[];done=[False]
            def wait(screen,**kw):
                self.assertEqual(screen,next(order))
                return dict(screen=screen,preferences=[0,0,int(not low)],
                    setup=dict(map=1,map_name='Celtic Forest One (small)'))
            def click(name):
                events.append(name)
                if name=='spellboxOK':done[0]=True
            owner.wait=wait;owner.click=owner.click_id=click
            owner.held_key=owner.key=lambda *a,**k:None
            owner.capture=lambda *a,**k:None
            owner.child=SimpleNamespace(poll=lambda:None)
            owner.latest=lambda:dict(high_resolution=int(not low))
            owner.viewport=SimpleNamespace(poll=lambda:dict(world=20 if done[0] else 1,
                frame_width=640 if low else 800,frame_height=480 if low else 600))
            ns['rebuild'](owner,low)
            self.assertNotIn('Map',events)
            self.assertEqual(len(owner.rebuilds),1)
            self.assertFalse(owner.rebuilds[0]['same_world'])


if __name__=='__main__': unittest.main()
