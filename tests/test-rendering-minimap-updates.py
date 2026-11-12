#!/usr/bin/env python3
"""Adverse checks over actual retained minimap input/output updates."""
from pathlib import Path
import copy,importlib.util,os,sys,unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
s=importlib.util.spec_from_file_location('minimap_updates',ROOT/'tools/close-rendering-minimap-updates.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class Checks(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  with Reader(ROOT) as r:
   cls.q=r.json(os.environ.get('MNM_MINIMAP_UPDATE_REPORT',m.PREFIX+'closeout-analysis-04.json'));assert cls.q['success'];cls.runs=cls.q['executions'];cls.parent=r.json(cls.q['parent']['record']);cls.plan=r.json('docs/native-rendering-closure.json')
 def damage(self,change):
  rows=copy.deepcopy(self.runs[0]['updates']);change(rows)
  with self.assertRaises(ValueError):m.check_updates(rows)
 def test_all_actual_contexts(self):
  self.assertEqual(len(self.runs),4)
  for r in self.runs:m.check_updates(r['updates'])
 def test_repeated_redraw_is_not_source_progress(self):self.damage(lambda r:r[1]['source'].update(generation=r[0]['source']['generation']))
 def test_missing_original_order_refuses(self):self.damage(lambda r:r[0]['source']['context']['cells'][0]['fields'].__setitem__(1,0))
 def test_mixed_source_surface_refuses(self):self.damage(lambda r:r[0]['source']['context']['creatures']['fields'].__setitem__(3,999))
 def test_mixed_map_center_refuses(self):self.damage(lambda r:r[0]['source']['context']['cells'][0]['metadata'].__setitem__(3,0))
 def test_dropped_cell_request_refuses(self):self.damage(lambda r:r[0]['source']['context']['cells'].pop())
 def test_wrong_cell_return_refuses(self):self.damage(lambda r:r[0]['source']['context']['cells'][0]['fields'].__setitem__(21,0))
 def test_unobserved_view_refuses(self):self.damage(lambda r:r[0]['source']['context']['camera']['metadata'].__setitem__(9,1))
 def test_wrong_camera_return_state_refuses(self):self.damage(lambda r:r[0]['source']['context']['camera']['fields'].__setitem__(19,0))
 def test_wrong_creature_refresh_refuses(self):self.damage(lambda r:r[0]['source']['context']['creatures']['fields'].__setitem__(19,0))
 def test_wrong_native_pixels_refuse(self):self.damage(lambda r:r[0]['presentation'].update(actual_map_sha256='0'*64))
 def test_no_input_change_is_not_update(self):self.damage(lambda r:r[1]['source']['context'].update(render_input_sha256=r[0]['source']['context']['render_input_sha256']))
 def test_no_visible_change_is_not_update(self):self.damage(lambda r:r[1]['presentation'].update(changed_visible_words=0))
 def test_lost_lease_refuses(self):self.damage(lambda r:r[0]['presentation'].update(lease=0))
 def test_unmatched_original_output_refuses(self):self.damage(lambda r:r[0]['presentation'].update(independently_matched=False))
 def test_sparse_map_pixel_substitution_refuses(self):self.damage(lambda r:r[0]['presentation'].update(mapped_unique_words=1))
 def test_missing_source_derived_checks_refuses(self):self.damage(lambda r:r[0]['source'].update(known_direct_words=0))
 def test_source_copy_after_scene_refuses(self):self.damage(lambda r:r[0]['source']['context']['display_copy'].__setitem__(1,r[0]['source']['context']['entry'][1]+1))
 def test_copy_outside_source_interval_refuses(self):self.damage(lambda r:r[0]['presentation']['phase'].update(copy_sequence=0))
 def test_wrong_source_expected_words_refuses(self):self.damage(lambda r:r[0]['presentation']['phase'].update(source_direct_sha256='0'*64))
 def test_failed_copy_caller_refuses(self):self.damage(lambda r:r[0]['presentation']['phase']['caller'].__setitem__(13,1))
 def test_reused_source_incarnation_refuses(self):self.damage(lambda r:r[0]['presentation']['phase'].update(source_created=r[0]['presentation']['phase']['next_import_sequence']))
 def test_unrelated_scene_generation_refuses(self):self.damage(lambda r:r[0]['presentation'].update(presentation_world=99999))
 def test_missing_context_cannot_close(self):
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.runs[:-1])
 def test_exact_two_new_rows_preserve_every_parent(self):
  rows=m.close_rows(self.plan,self.parent,self.runs);self.assertEqual(sum(v['state']=='passed' for v in rows),62)
  for before,after in zip(self.parent['requirements'],rows):
   if before['id']!=m.LEAF:self.assertEqual(before,after)
if __name__=='__main__':unittest.main()
