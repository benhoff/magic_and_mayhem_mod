import sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_counter_namespace import prospective_bindings
class Namespace(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name);self.old={str(self.root/'working/legacy.sqlite3'):'working/broader-player/bp16/world-attachment/attempts.sqlite3'}
 def test_declaration_changes_only_counter_target_and_preserves_old(self):
  p=self.root/next(iter(self.old.values()));p.parent.mkdir(parents=True);p.write_bytes(b'old exact bytes');before=dict(self.old);new=prospective_bindings(self.root,self.old);self.assertEqual(new,{next(iter(self.old)):'working/broader-player/bp20/world-attachment/attempts.sqlite3'});self.assertEqual(self.old,before);self.assertEqual(p.read_bytes(),b'old exact bytes');self.assertFalse((self.root/next(iter(new.values()))).exists())
 def test_existing_database_or_sidecar_refuses(self):
  for suffix in('','-wal','-shm','-journal'):
   p=self.root/('working/broader-player/bp20/world-attachment/attempts.sqlite3'+suffix);p.parent.mkdir(parents=True,exist_ok=True);p.touch()
   with self.assertRaises(ValueError):prospective_bindings(self.root,self.old)
   p.unlink()
 def test_unknown_or_traversing_old_namespace_refuses(self):
  for value in('working/other.sqlite3','working/broader-player/bp16/../escape.sqlite3','working/broader-player/bp16/wrong.txt'):
   with self.assertRaises(ValueError):prospective_bindings(self.root,{next(iter(self.old)):value})
 def test_unknown_legacy_key_refuses(self):
  with self.assertRaises(ValueError):prospective_bindings(self.root,{'outside':next(iter(self.old.values()))})
 def test_aliases_refuse(self):
  with self.assertRaises(ValueError):prospective_bindings(self.root,self.old|{str(self.root/'working/other.sqlite3'):next(iter(self.old.values()))})
if __name__=='__main__':unittest.main()
