import hashlib,importlib.util,os,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from evidence_objects import Pair
from evidence_workspace import materialize
spec=importlib.util.spec_from_file_location('held_top',ROOT/'tools/retire-broader-held-top-files.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class HeldTopFiles(unittest.TestCase):
 def test_exact_retention_restore_keeps_pin_hardlinks_and_children(self):
  with tempfile.TemporaryDirectory(dir='/tmp')as directory:
   base=Path(directory);root=base/'execution';root.mkdir();(root/'.retain').write_text('original pin');f=root/'raw.bin';data=b'raw observation\0'*123;f.write_bytes(data)
   (root/'surface-loss-request-v1.json').write_text('original publication');os.link(root/'surface-loss-request-v1.json',root/'surface-loss-request-v1.json.pending');inode=(root/'surface-loss-request-v1.json').stat().st_ino
   child=root/'child';child.mkdir();(child/'unknown.txt').write_text('keep physically')
   row=m.row(f,f.name);row.update(size=len(data),sha256=hashlib.sha256(data).hexdigest());pin=m.row(root/'.retain','.retain');pin.update(size=(root/'.retain').stat().st_size,sha256=hashlib.sha256((root/'.retain').read_bytes()).hexdigest());plan=m.prepare(root,[row,pin])
   self.assertEqual([v['path']for v in plan['members']],['.','raw.bin'])
   with Pair(base/'a.sqlite3',base/'b.sqlite3',require_independent=False)as pair:
    result=m.transfer(pair,plan,dict(record='completed',sha256='a'*64),base);materialize(pair,result['manifest'],base/'restored')
   self.assertEqual((base/'restored/raw.bin').read_bytes(),data);self.assertFalse(f.exists());self.assertEqual((root/'.retain').read_text(),'original pin');self.assertEqual((child/'unknown.txt').read_text(),'keep physically')
   self.assertEqual((root/'surface-loss-request-v1.json').stat().st_nlink,2);self.assertEqual((root/'surface-loss-request-v1.json.pending').stat().st_ino,inode);self.assertTrue(result['real_restore_verified'])
 def test_unknown_or_changed_file_refuses(self):
  with tempfile.TemporaryDirectory(dir='/tmp')as directory:
   root=Path(directory);(root/'.retain').touch();f=root/'raw.bin';f.write_bytes(b'raw');row=m.row(f,f.name);row.update(size=3,sha256=hashlib.sha256(b'raw').hexdigest())
   (root/'unknown.bin').touch()
   with self.assertRaises(ValueError):m.prepare(root,[row,dict(path='.retain',sha256=hashlib.sha256(b'').hexdigest())])
   (root/'unknown.bin').unlink();f.write_bytes(b'changed')
   with self.assertRaises(ValueError):m.prepare(root,[row,dict(path='.retain',sha256=hashlib.sha256(b'').hexdigest())])
if __name__=='__main__':unittest.main()
