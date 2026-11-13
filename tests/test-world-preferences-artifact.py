import hashlib,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_preferences_artifact import preferences_artifact
class Reader:
 def read_bytes(self,n):return b'IsHighRes=FALSE\n'
class Preferences(unittest.TestCase):
 def setUp(self):
  self.root=Path('/recorded/repo');self.relative='working/experiments/owned/game/CFG/prefs.cfg';raw=Reader().read_bytes('blob');self.entry=dict(path=self.relative,record='blob',sha256=hashlib.sha256(raw).hexdigest(),bytes=len(raw));self.a=dict(session='/recorded/repo/working/experiments/owned',artifacts=[self.entry])
 def test_relative_producer_identity_reads_verified_bytes(self):self.assertEqual(preferences_artifact(Reader(),self.a,self.root),b'IsHighRes=FALSE\n')
 def test_absolute_producer_identity_reads_same_verified_bytes(self):
  self.entry['path']='/recorded/repo/'+self.relative;self.assertEqual(preferences_artifact(Reader(),self.a,self.root),b'IsHighRes=FALSE\n')
 def test_duplicate_absolute_relative_identity_refuses(self):
  self.a['artifacts'].append(dict(self.entry,path='/recorded/repo/'+self.relative))
  with self.assertRaises(ValueError):preferences_artifact(Reader(),self.a,self.root)
 def test_other_session_clone_does_not_satisfy_missing_input(self):
  self.entry['path']=self.relative.replace('/owned/','/other/')
  with self.assertRaises(ValueError):preferences_artifact(Reader(),self.a,self.root)
 def test_outside_or_traversing_session_refuses(self):
  for n in('/other/work/owned','/recorded/repo/working/../escape','/recorded/repo'):
   self.a['session']=n
   with self.assertRaises(ValueError):preferences_artifact(Reader(),self.a,self.root)
 def test_corrupt_payload_refuses(self):
  self.entry['sha256']='0'*64
  with self.assertRaises(ValueError):preferences_artifact(Reader(),self.a,self.root)
 def test_missing_record_refuses(self):
  self.a['artifacts']=[]
  with self.assertRaises(ValueError):preferences_artifact(Reader(),self.a,self.root)
if __name__=='__main__':unittest.main()
