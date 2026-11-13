import copy,hashlib,json,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_nominal_claims import current_claims,comparison_command,PUBLISHER,OLD,NEW,REVIEW
class R:
 def json(self,n):return self.review
 def digest(self,n):return self.hashes[n]
 def read_bytes(self,n):return self.payloads[n]
class Claims(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name);(self.root/'working').mkdir();self.old=dict(schema=1,claims=[dict(behavior='exact',contract_sha256='same',scenarios={'exact':'same'})],sources={PUBLISHER:OLD,'runtime/native.c':'native-exact'});self.a=self.root/'working/old.json';self.b=self.root/'working/new.json';self.a.write_text(json.dumps(self.old));self.b.write_text(json.dumps(current_claims(self.old)));self.r=R();self.r.hashes={REVIEW:'review',PUBLISHER:NEW,'runtime/native.c':'native-exact'};self.r.payloads={'old':self.a.read_bytes(),'new':self.b.read_bytes()};ref=lambda label:dict(record=label,sha256=hashlib.sha256(self.r.payloads[label]).hexdigest());self.r.review=dict(success=True,original_game_runs=0,new_request_reservations=0,changed_claim_source_paths=[PUBLISHER],sources={PUBLISHER:NEW},old_input_path=str(self.a),new_input_path=str(self.b),old_input=ref('old'),new_input=ref('new'));self.cmd=['python3','comparer','--claims',str(self.a),'--report','fresh']
 def test_only_input_selector_changes(self):
  old=copy.deepcopy(self.cmd);new,ref=comparison_command(self.r,self.root,dict(family='caller'),self.cmd);self.assertEqual(new,['python3','comparer','--claims',str(self.b),'--report','fresh']);self.assertEqual(old,self.cmd);self.assertEqual(ref['record'],REVIEW)
 def test_contracts_and_native_hashes_are_literal(self):
  before=copy.deepcopy(self.old);new=current_claims(self.old);self.assertEqual(new['claims'],before['claims']);self.assertEqual(new['sources']['runtime/native.c'],'native-exact');self.assertEqual(before,self.old)
 def test_native_drift_refuses(self):
  self.r.hashes['runtime/native.c']='changed'
  with self.assertRaises(ValueError):comparison_command(self.r,self.root,dict(family='caller'),self.cmd)
 def test_corrupt_or_foreign_input_refuses(self):
  self.a.write_text('{}')
  with self.assertRaises(ValueError):comparison_command(self.r,self.root,dict(family='caller'),self.cmd)
 def test_unknown_publisher_or_duplicate_selector_refuses(self):
  for value in('unknown',NEW):
   old=copy.deepcopy(self.old);old['sources'][PUBLISHER]=value
   with self.assertRaises(ValueError):current_claims(old)
  with self.assertRaises(ValueError):comparison_command(self.r,self.root,dict(family='caller'),self.cmd+['--claims',str(self.a)])
 def test_other_families_unchanged(self):self.assertEqual(comparison_command(self.r,self.root,dict(family='font'),self.cmd),(self.cmd,None))
if __name__=='__main__':unittest.main()
