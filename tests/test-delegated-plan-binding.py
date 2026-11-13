import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_delegated_plan import bind_arguments,capture_backing,LEGACY
OLD='research/runtime/old.json';NEW='research/runtime/current.json'
class Binding(unittest.TestCase):
 def test_exact_old_outer_identity_changes_only_value(self):
  args=['--report','same','--plan',OLD,'--low'];before=list(args);out,p=bind_arguments(args,OLD,NEW,'ui');self.assertEqual(out,['--report','same','--plan',NEW,'--low']);self.assertEqual(args,before);self.assertTrue(p['only_plan_identity_changed'])
 def test_reviewed_legacy_three_families_are_rebound(self):
  for family,n in LEGACY.items():self.assertEqual(bind_arguments(['--plan',n],OLD,NEW,family)[0],['--plan',NEW])
 def test_already_adapted_identity_is_idempotent(self):self.assertEqual(bind_arguments(['--plan',NEW],OLD,NEW,'caller')[0],['--plan',NEW])
 def test_no_explicit_plan_preserves_every_argument(self):self.assertEqual(bind_arguments(['--report','x','--scene-effects'],OLD,NEW,'combat'),(['--report','x','--scene-effects'],None))
 def test_unknown_foreign_or_ambiguous_selector_refuses(self):
  for args in [['--plan','research/runtime/foreign.json'],['--plan'],['--plan',OLD,'--plan',OLD],['--input-plan',OLD],['--plan',LEGACY['font']]]:
   with self.assertRaises(ValueError):bind_arguments(args,OLD,NEW,'caller')
 def test_escaping_phase_declaration_refuses(self):
  with self.assertRaises(ValueError):bind_arguments([],OLD,'research/runtime/../escape.json','caller')
 def test_only_two_existing_controller_RAM_policies_disable_outer_adapter(self):
  old=dict(enabled=True,policy='exact-old');before=copy.deepcopy(old)
  for family in('caller','picking'):
   value=capture_backing(family,old);self.assertFalse(value['enabled']);self.assertTrue(value['paired_real_restore_required']);self.assertTrue(value['raw_SQLite_import_required'])
  self.assertEqual(old,before);self.assertEqual(capture_backing('font',old),old)
if __name__=='__main__':unittest.main()
