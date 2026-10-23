import copy
import tempfile
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_session_contracts import witness_cohort,functional_admission,declared_source,delivery_admission


class Witnesses(unittest.TestCase):
    def setUp(self):
        self.catalog=dict(frames=[{},{}],witnesses=[dict(ordinal=1),dict(ordinal=2)])
        self.original=dict(egl_swap_completion_observation=dict(frames=[dict(ordinal=1),dict(ordinal=2)]),egl_physical_observation=dict(frames=[dict(ordinal=1),dict(ordinal=2)]))
    def test_complete_exact_order(self):
        self.assertEqual(len(witness_cohort(self.catalog,self.original)[0]),2)
    def test_truncated_or_extra_physical_plane_refuses_before_zip(self):
        for rows in ([dict(ordinal=1)],[dict(ordinal=1),dict(ordinal=2),dict(ordinal=3)]):
            self.original['egl_physical_observation']['frames']=rows
            with self.assertRaisesRegex(ValueError,'incomplete'):witness_cohort(self.catalog,self.original)
    def test_duplicate_reordered_boolean_and_missing_ordinals_refuse(self):
        for category in ('egl_swap_completion_observation','egl_physical_observation'):
            for value in (1,True,3,None):
                data=copy.deepcopy(self.original);data[category]['frames'][1]['ordinal']=value
                with self.subTest(category=category,value=value),self.assertRaises(ValueError):witness_cohort(self.catalog,data)
        self.catalog['witnesses'].reverse()
        with self.assertRaises(ValueError):witness_cohort(self.catalog,self.original)
    def test_empty_cohort_is_not_success(self):
        with self.assertRaises(ValueError):witness_cohort(dict(frames=[],witnesses=[]),dict(egl_swap_completion_observation=dict(frames=[]),egl_physical_observation=dict(frames=[])))
    def test_performance_default_and_negative_gate_stay_required(self):
        for negative in (False,True):self.assertTrue(functional_admission(functional_only=False,resident=True,impossible_budget=negative)['performance_required'])
    def test_functional_only_cannot_claim_performance_or_bypass_negative(self):
        self.assertFalse(functional_admission(functional_only=True,resident=True,impossible_budget=False)['performance_required'])
        for resident,negative in ((False,False),(False,True),(True,True)):
            with self.assertRaises(ValueError):functional_admission(functional_only=True,resident=resident,impossible_budget=negative)

    def test_functional_mode_retains_memory_cache_and_every_presentation(self):
        checks=dict(rss=True,caches=True,no_coalescing=True,all_presentations=True,warm_median=False)
        for key in ('rss','caches','no_coalescing','all_presentations'):
            broken={**checks,key:False}
            self.assertFalse(delivery_admission(broken,functional_only=True)['passed'])
        with self.assertRaises(ValueError):delivery_admission({'rss':True},functional_only=True)

    def test_timing_failure_is_separate_only_in_explicit_functional_mode(self):
        checks=dict(rss=True,caches=True,no_coalescing=True,all_presentations=True,warm_median=False)
        self.assertTrue(delivery_admission(checks,functional_only=True)['passed'])
        self.assertFalse(delivery_admission(checks,functional_only=False)['passed'])

    def test_declared_source_preserves_contained_parent_dependencies(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'renderer').mkdir();(root/'assets').mkdir();target=root/'assets/asset.hpp';target.write_text('source')
            self.assertEqual(declared_source(root,'renderer/../assets/asset.hpp'),target)
    def test_declared_source_refuses_absolute_escape_and_original_or_working(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            for value in ('../outside','/absolute','renderer/../original/game.exe','working/input','assets/../.git/index'):
                with self.subTest(value=value),self.assertRaises(ValueError):declared_source(root,value)
    def test_declared_source_refuses_symlink_escape(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'assets').mkdir();(root/'assets/alias').symlink_to('/tmp')
            with self.assertRaises(ValueError):declared_source(root,'assets/alias/file')


if __name__=='__main__':unittest.main()
