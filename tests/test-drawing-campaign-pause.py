import importlib.util
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('pause',ROOT/'tools/analyze-drawing-campaign-pause.py')
import sys
sys.path.insert(0,str(ROOT/'tools'))
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)


def fixture():
    samples=[]
    for i,(screen,counter) in enumerate([(17,40)]*3+[(10,40)]*3+[(2,41),(2,42)]+[(17,42)]*3,1):
        samples.append(dict(ordinal=i,tick=i*500,top_screen=screen,validity=63,world_counter=counter,accepted_sequence=counter,mode=0,camera=[1,2],effect_counter=3,ani_fields=[4]*24))
    actions=[dict(screen=s,action=a,status=1) for s,a in [(3,22),(18,23),(18,25),(17,14),(10,20),(17,15),(6,26),(3,3)]]
    return dict(samples=samples),actions


class CampaignPause(unittest.TestCase):
    def test_selected_pause_and_resume_are_bounded(self):
        r=module.assess(*fixture());self.assertTrue(r['selected_state_pause_observed'])
        self.assertFalse(r['allanimation_pause_equivalence'])

    def test_changed_menu_counter_or_missing_resume_refuses(self):
        o,a=fixture();o['samples'][4]['world_counter']=41
        with self.assertRaises(ValueError):module.assess(o,a)
        o,a=fixture()
        for s in o['samples'][6:8]:s['world_counter']=s['accepted_sequence']=40
        with self.assertRaises(ValueError):module.assess(o,a)

    def test_failed_or_missing_action_refuses(self):
        o,a=fixture();a[3]['status']=2
        with self.assertRaises(ValueError):module.assess(o,a)
        o,a=fixture()
        with self.assertRaises(ValueError):module.assess(o,a[:-1])


if __name__=='__main__':unittest.main()
