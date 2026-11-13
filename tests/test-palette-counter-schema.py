import sqlite3,sys,tempfile,unittest
from contextlib import closing
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_palette_counter_schema import SCHEMA,source_schema,initialize,namespace,verify
class Schema(unittest.TestCase):
 def setUp(self):self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name);self.relative='working/broader-player/bp34/world-palette/attempts.sqlite3'
 def test_real_source_schema(self):self.assertEqual(source_schema(Path(__file__).resolve().parents[1])[0],SCHEMA)
 def test_prepare_and_actual_sql_rollback(self):
  q=initialize(self.root,self.relative,SCHEMA);self.assertTrue(q['empty']and q['duplicate_report_refused']);self.assertEqual(q['actual_game_attempts_inserted'],0)
 def test_existing_counter_refused_without_mutation(self):
  initialize(self.root,self.relative,SCHEMA);before=(self.root/self.relative).read_bytes()
  with self.assertRaises(ValueError):initialize(self.root,self.relative,SCHEMA)
  self.assertEqual(before,(self.root/self.relative).read_bytes())
 def test_nonempty_counter_refused(self):
  initialize(self.root,self.relative,SCHEMA)
  with closing(sqlite3.connect(self.root/self.relative))as db,db:db.execute('insert into attempts(report)values(?)',('real-request',))
  with self.assertRaises(ValueError):verify(self.root,self.relative)
 def test_unknown_schema_refused(self):
  with self.assertRaises(ValueError):initialize(self.root,self.relative,'CREATE TABLE attempts(x)')
 def test_wrong_namespace_refused(self):
  for p in ['/tmp/attempts.sqlite3','working/palette-cycling/attempts.sqlite3','working/broader-player/bp33/world-palette/attempts.sqlite3','working/broader-player/bp34/world-font/attempts.sqlite3','working/broader-player/bp34/world-palette/../attempts.sqlite3']:
   with self.subTest(p=p),self.assertRaises(ValueError):namespace(self.root,p)
 def test_symlink_parent_refused(self):
  (self.root/'elsewhere').mkdir();(self.root/'working').symlink_to(self.root/'elsewhere',target_is_directory=True)
  with self.assertRaises(ValueError):initialize(self.root,self.relative,SCHEMA)
 def test_sidecar_refused(self):
  p=self.root/self.relative;p.parent.mkdir(parents=True);Path(str(p)+'-journal').write_bytes(b'held')
  with self.assertRaises(ValueError):initialize(self.root,self.relative,SCHEMA)
 def test_incorrect_existing_table_refused(self):
  p=self.root/self.relative;p.parent.mkdir(parents=True)
  with closing(sqlite3.connect(p))as db,db:db.execute('CREATE TABLE attempts(report TEXT)')
  with self.assertRaises(ValueError):verify(self.root,self.relative)
if __name__=='__main__':unittest.main()
