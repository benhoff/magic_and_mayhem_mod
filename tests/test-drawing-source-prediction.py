#!/usr/bin/env python3
"""A diagnostic cohort must keep every unsupported/interfered witness explicit."""
import copy
import importlib.util
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
spec=importlib.util.spec_from_file_location('prediction',Path(__file__).resolve().parents[1]/'tools/compare-drawing-source-points.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


def fixture():
    p=[64,1,19,1,0,0,2,8,6,8,0,0,1,0,7,0]
    frame=dict(ordinal=1,entry=1,points=[dict(phase=i,source=copy.copy(p)) for i in (1,2,3)],producer_live_at_all_points=True,both_generations_known=True,competing_mutation_count=0)
    call=[0]*32;call[2]=7;call[21]=16;call[22:28]=[0,0,0,0,8,6]
    record=[0]*24;record[1]=1;record[2]=1
    return dict(frames=[frame]),{1:tuple(call)},[(tuple(record),b'')]


class Prediction(unittest.TestCase):
    def test_opaque_full_plane_candidate(self):
        selected,excluded=m.candidates(*fixture());self.assertEqual(len(selected),1);self.assertEqual(excluded,[])
    def test_each_missing_or_interfered_condition_keeps_original_identity(self):
        for key,value,reason in (('producer_live_at_all_points',False,'producer-stopped-or-failed'),('both_generations_known',False,'unknown-generation'),('competing_mutation_count',1,'observed-competing-mutation')):
            observation,api,records=fixture();observation['frames'][0][key]=value
            selected,excluded=m.candidates(observation,api,records);self.assertEqual(selected,[]);self.assertEqual(excluded,[dict(ordinal=1,entry=1,reason=reason)])
    def test_changed_point_transfer_geometry_and_active_world_refuse(self):
        observation,api,records=fixture();observation['frames'][0]['points'][1]['source'][10]=1
        self.assertEqual(m.candidates(observation,api,records)[1][0]['reason'],'source-point-state-changed')
        observation,api,records=fixture();call=list(api[1]);call[22]=1;api[1]=tuple(call)
        self.assertEqual(m.candidates(observation,api,records)[1][0]['reason'],'unsupported-transfer-geometry')
        observation,api,records=fixture();record=list(records[0][0]);record[2]=11;records[0]=(tuple(record),b'')
        self.assertEqual(m.candidates(observation,api,records)[1][0]['reason'],'world-queue-active')
    def test_repeated_prefix_is_explicit_not_a_new_derived_frame(self):
        observation,api,records=fixture();second=copy.deepcopy(observation['frames'][0]);second.update(ordinal=2,entry=2);observation['frames'].append(second);api[2]=api[1]
        selected,excluded=m.candidates(observation,api,records);self.assertEqual(len(selected),1);self.assertEqual(excluded[0],dict(ordinal=2,entry=2,reason='repeated-or-reordered-prefix'))

if __name__=='__main__':unittest.main()
