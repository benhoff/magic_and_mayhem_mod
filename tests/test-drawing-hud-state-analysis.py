import importlib.util
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('analysis',ROOT/'tools/analyze-drawing-hud-state.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)


def fixture():
    hud=[];semantic=[]
    for i,screen in enumerate([2]+[17]*4+[10]*4,1):
        common=dict(ordinal=i,tick=i*500,thread=18,menu_screen=screen)
        semantic.append(dict(**common,top_screen=screen,validity=63,world_counter=40,accepted_sequence=40,mode=0,pacing_fields=[50,20],effect_counter=1,ani_fields=[1]*24))
        hud.append(dict(**common,validity=15,candidate_state=1,observed_pool_id=1,actor_slot=2,actor_type=14,actor_owner=1,camera_orientation=2,camera_whole=[1,2,0],camera_fraction=[0,0,0]))
    return dict(owner_thread=18,samples=hud),dict(owner_thread=18,samples=semantic)


class HUDAnalysis(unittest.TestCase):
    def test_camera_dwell_is_bounded_not_semantic_input_proof(self):
        r=module.assess(*fixture());self.assertTrue(r['selected_camera_pause_observed']);self.assertEqual(r['validated_hit_samples'],1);self.assertFalse(r['selection_verified']);self.assertFalse(r['input_to_state_causality_proved'])

    def test_camera_change_stays_visible(self):
        h,s=fixture();h['samples'][6]['camera_whole']=[3,2,0]
        self.assertFalse(module.assess(h,s)['selected_camera_pause_observed'])

    def test_wrong_owner_ambiguous_ticks_or_state_identity_refuses(self):
        for kind in ('owner','tick','identity'):
            h,s=fixture()
            if kind=='owner':h['owner_thread']=19
            elif kind=='tick':s['samples'][1]['tick']=s['samples'][0]['tick']
            else:h['samples'][0]['menu_screen']=3
            with self.subTest(kind=kind),self.assertRaises(ValueError):module.assess(h,s)

    def test_missing_exact_tick_is_excluded_not_interpolated(self):
        h,s=fixture();h['samples'][0]['tick']+=1;r=module.assess(h,s)
        self.assertEqual(r['unpaired_hud_samples'],1);self.assertEqual(r['validated_hit_samples'],0)


if __name__=='__main__':unittest.main()
