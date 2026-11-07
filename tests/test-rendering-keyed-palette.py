#!/usr/bin/env python3
"""Adverse admission checks over actual retained keyed and selected-table proof."""
from pathlib import Path
import copy,importlib.util,json,os,sys,unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
s=importlib.util.spec_from_file_location('keypal',ROOT/'tools/close-rendering-keyed-palette.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class Checks(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  with Reader(ROOT) as r:
   cls.q=r.json(os.environ.get('MNM_KEYED_PALETTE_REPORT',m.PREFIX+'closeout-analysis-02.json'));assert cls.q['success'];cls.runs=cls.q['executions'];cls.parent=r.json(cls.q['parent']['record']);cls.plan=r.json('docs/native-rendering-closure.json')
 def keyed(self,change):
  rows=copy.deepcopy(self.runs[0]['keyed']['witnesses']);change(rows)
  with self.assertRaises(ValueError):m.check_keyed(rows)
 def palette(self,change):
  rows=copy.deepcopy(self.runs[0]['palette']['witnesses']);change(rows)
  with self.assertRaises(ValueError):m.check_palettes(rows)
 def test_all_actual_keyed_domains(self):
  self.assertEqual(len(self.runs),8)
  for r in self.runs:m.check_keyed(r['keyed']['witnesses'])
 def test_all_actual_selected_tables(self):
  self.assertEqual(sum(len(r['palette']['witnesses']) for r in self.runs),72)
  for r in self.runs:m.check_palettes(r['palette']['witnesses'])
 def test_repeated_redraw_is_not_source_progress(self):self.keyed(lambda r:r[1].update(generation=r[0]['generation']))
 def test_opaque_copy_cannot_supply_keyed_credit(self):self.keyed(lambda r:r[0]['copy']['words'].__setitem__(8,0))
 def test_wrong_original_key_refuses(self):self.keyed(lambda r:r[0]['copy']['caller'].__setitem__(22,0))
 def test_failed_original_call_refuses(self):self.keyed(lambda r:r[0]['copy']['caller'].__setitem__(13,0x88760096))
 def test_skipped_key_overwrite_refuses(self):self.keyed(lambda r:r[0]['copy'].update(after_skipped_sha256='0'*64))
 def test_zero_visible_change_refuses(self):self.keyed(lambda r:r[0]['copy'].update(changed_words=0))
 def test_missing_key_skip_refuses(self):self.keyed(lambda r:r[0]['copy'].update(skipped_words=0))
 def test_wrong_native_copy_output_refuses(self):self.keyed(lambda r:r[0].update(actual_rgba=[0,0,0,255]))
 def test_changed_selected_table_refuses(self):self.palette(lambda r:r[0]['source']['source'].update(table_hex='00'*512))
 def test_same_table_is_not_remapping_variation(self):self.palette(lambda r:r[1]['source']['source'].update(table_sha256=r[0]['source']['source']['table_sha256']))
 def test_wrong_shade_selection_refuses(self):self.palette(lambda r:r[0]['source']['fields'].__setitem__(16,0x7777))
 def test_wrong_frame_identity_refuses(self):self.palette(lambda r:r[0]['source']['source'].update(frame_sha256='0'*64))
 def test_wrong_native_palette_output_refuses(self):self.palette(lambda r:r[0].update(actual_rgba=[0,0,0,255]))
 def test_unowned_palette_output_refuses(self):self.palette(lambda r:r[0].update(lease=0))
 def test_missing_domain_does_not_close(self):
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.runs[:-1])
 def test_all_actual_route_boundaries(self):
  for r in self.runs:m.keyed_boundary(r['keyed']['route_boundary'],r['keyed']['historical_refused_calls'])
 def test_known_invalid_cursor_is_retained_without_success_credit(self):
  bad=[r for r in self.runs if r['keyed']['route_boundary']['failed_calls']];self.assertEqual(len(bad),2)
  for r in bad:self.assertEqual(r['domain'][1],[640,480]);self.assertEqual(r['scenario'],'normal-storage')
 def test_unknown_failure_cannot_be_excluded(self):
  r=copy.deepcopy(next(v for v in self.runs if v['keyed']['route_boundary']['failed_calls']));bad=next(v for v in r['keyed']['historical_refused_calls'] if v[12]&1);bad[1]+=1
  with self.assertRaises(ValueError):m.keyed_boundary(r['keyed']['route_boundary'],r['keyed']['historical_refused_calls'])
 def test_failed_copy_cannot_publish_work(self):
  r=copy.deepcopy(next(v for v in self.runs if v['keyed']['route_boundary']['failed_calls']));bad=next(v for v in r['keyed']['historical_refused_calls'] if v[12]&1);bad[40]=1
  with self.assertRaises(ValueError):m.keyed_boundary(r['keyed']['route_boundary'],r['keyed']['historical_refused_calls'])
 def test_exact_four_new_rows_preserve_every_parent(self):
  rows=m.close_rows(self.plan,self.parent,self.runs);self.assertEqual(sum(v['state']=='passed' for v in rows),60)
  for before,after in zip(self.parent['requirements'],rows):
   if before['id'] not in m.LEAVES:self.assertEqual(before,after)
if __name__=='__main__':unittest.main()
