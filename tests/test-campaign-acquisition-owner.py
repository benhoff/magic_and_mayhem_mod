#!/usr/bin/env python3
"""Ownership adversaries for concurrent campaign evidence collectors; no game."""
import hashlib,json,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_acquisition_owner import admitted_sessions,single_session

class Ownership(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name)
  self.owner=self.root/'working/pilot/case/owner';self.owner.mkdir(parents=True)
  self.prefix=self.owner.parent/'wineprefix';self.prefix.mkdir()
 def add(self,owner=None,prefix=None,name='run-first'):
  owner=owner or self.owner;prefix=prefix or self.prefix
  session=self.root/'working/experiments/menu-observer'/name;session.mkdir(parents=True,exist_ok=True)
  directory=owner/'sessions';directory.mkdir(parents=True,exist_ok=True)
  p=directory/(hashlib.sha256(str(session).encode()).hexdigest()+'.json');p.write_text(json.dumps(dict(prefix=str(prefix),session=str(session))))
  return session,p
 def test_exact_owner_ignores_concurrent_foreign_directories(self):
  session,_=self.add();self.add(self.root/'working/tests/native-output-owner/run-other',self.root/'working/pilot/other/wineprefix','run-other')
  self.assertEqual(single_session(self.root,self.owner,self.prefix),session)
 def test_missing_setup_owner_and_journal_have_no_admitted_sessions(self):
  self.assertEqual(admitted_sessions(self.root,self.owner,self.prefix),[])
  self.assertEqual(admitted_sessions(self.root,self.owner/'missing',self.prefix),[])
  with self.assertRaises(ValueError):single_session(self.root,self.owner,self.prefix)
 def test_foreign_prefix_inside_assigned_owner_refuses(self):
  self.add(prefix=self.root/'working/pilot/foreign/wineprefix')
  with self.assertRaises(ValueError):single_session(self.root,self.owner,self.prefix)
 def test_multiple_actual_generations_cannot_supply_nominal_capture(self):
  self.add();self.add(name='run-second')
  with self.assertRaises(ValueError):single_session(self.root,self.owner,self.prefix)
 def test_escaped_or_rewritten_session_identity_refuses(self):
  _,p=self.add();q=json.loads(p.read_text());q['session']=str(self.root/'unowned/run-first');p.write_text(json.dumps(q))
  with self.assertRaises(ValueError):single_session(self.root,self.owner,self.prefix)
 def test_aliased_journal_and_receipt_refuse(self):
  session,p=self.add();p.rename(p.with_suffix('.held'));p.symlink_to(p.with_suffix('.held'))
  with self.assertRaises(ValueError):single_session(self.root,self.owner,self.prefix)
  p.unlink();p.with_suffix('.held').rename(p);journal=p.parent;journal.rename(journal.with_name('held'));journal.symlink_to(journal.with_name('held'))
  with self.assertRaises(ValueError):single_session(self.root,self.owner,self.prefix)
 def test_session_alias_refuses_even_with_matching_filename(self):
  session,p=self.add();actual=session.with_name('run-held');session.rename(actual);session.symlink_to(actual)
  with self.assertRaises(ValueError):single_session(self.root,self.owner,self.prefix)
 def test_journal_generation_limit_is_bounded(self):
  for i in range(9):self.add(name='run-'+str(i))
  with self.assertRaises(ValueError):admitted_sessions(self.root,self.owner,self.prefix)
 def test_receipt_name_binds_the_exact_admitted_session(self):
  _,p=self.add();p.rename(p.with_name('wrong.json'))
  with self.assertRaises(ValueError):single_session(self.root,self.owner,self.prefix)
if __name__=='__main__':unittest.main()
