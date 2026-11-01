#!/usr/bin/env python3
"""Mutation controls for EP05 semantic admission; fixtures are not live evidence."""
import copy
import importlib.util
import struct
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from rendering_progression import validate_flow, validate_callbacks


def flow(case='progression'):
    def row(event, screen, seq, ack, **extra):
        return dict(event=event, screen=screen, sequence=seq, ack=ack, thread=40, **extra)
    states = [row('main',3,2,0), row('region-entry',18,4,1,region=1),
              row('natural-report',6,8,3,outcome=int(case!='defeat'),title='Original report',texts=['Score']*21),
              row('result-return',4,10,4), row('region-entry',18,12,5,region=2 if case!='defeat' else 1)]
    return dict(case=case, size='800x600', success=True, normal_quit=True, native_command_fallback=False, native_error='',
                states=states, worlds=[dict(realm='Celtic',region=n,world=i+1,paint=i+10,frames=i+20,width=800,height=600)
                                      for i,n in enumerate([1,2] if case!='defeat' else [1,1])])


def trace(quit_first=False):
    values=[]
    for event,screen,arg,active in [(3,3,0,0),(28,2,0,0x6db967), (3,6,21,0), (3,3,4,0)]:
        if quit_first and not values:
            values.append([1,3,40,17,0,0,0,0,0,3,0,0,0,0,0,0])
        row=[0]*16;row[0]=len(values)+1;row[1]=event;row[2]=40;row[3]=screen;row[9]=arg;row[11]=active;values.append(row)
    return b'MNMMENU1'+struct.pack('<II',1,64)+b''.join(struct.pack('<16I',*r) for r in values)


class ProgressionTests(unittest.TestCase):
    def test_distinct_legitimate_routes_remain_preparation_not_pixel_acceptance(self):
        for case in ('progression','victory','defeat'):
            result=validate_flow(flow(case),case)
            self.assertTrue(result['semantic_route_verified'])
            self.assertFalse(result['pixel_history_accepted'])
            self.assertFalse(result['ownership_retirement_accepted'])
            self.assertFalse(result['supported_player_accepted'])

    def test_reload_repeated_map_wrong_outcome_quit_and_owner_drift_refuse(self):
        mutations=[lambda f:f['worlds'][1].update(width=640),
                   lambda f:f['worlds'][1].update(region=1),
                   lambda f:f['worlds'][1].update(world=1),
                   lambda f:f['states'][2].update(outcome=0),
                   lambda f:f['states'][2]['texts'].__setitem__(19,'You quit the battle.'),
                   lambda f:f['states'][3].update(ack=3),
                   lambda f:f['states'][3].update(sequence=6),
                   lambda f:f['states'][3].update(thread=44),
                   lambda f:f['states'].append(dict(event='load',thread=40)),
                   lambda f:f.update(success=False),
                   lambda f:f.update(normal_quit=False),
                   lambda f:f.update(native_command_fallback=True),
                   lambda f:f['worlds'][1].update(paint=0)]
        for mutation in mutations:
            f=flow();mutation(f)
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):validate_flow(f,'progression')

    def test_defeat_cannot_reuse_victory(self):
        f=flow('defeat');f['states'][2]['outcome']=1
        with self.assertRaises(ValueError):validate_flow(f,'defeat')

    def test_original_callback_proof_required_and_quit_cannot_supply_defeat(self):
        self.assertTrue(validate_callbacks(trace(),flow())['verified'])
        for data in (trace(True), trace()[:-1], trace().replace(struct.pack('<I',0x6db967),struct.pack('<I',0x6cbb78))):
            with self.assertRaises(ValueError):validate_callbacks(data,flow())

    def test_projection_bounded_for_remote_target_and_portal(self):
        spec=importlib.util.spec_from_file_location('outcome_input',ROOT/'tools/campaign-outcome-input.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        hero=dict(x=0,y=0,z=0)
        for target in [dict(x=100,y=-100,z=30),dict(x=1,y=2,z=0),hero]:
            x,y=module.target_point(hero,target)
            self.assertTrue(0<=x<800 and 0<=y<600)


if __name__=='__main__':unittest.main()
