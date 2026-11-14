#!/usr/bin/env python3
"""Concurrent admission readers never observe a partial published receipt."""
import io,json,os,sys,tempfile,threading,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import owned_session_journal as journal

class Publication(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name);self.path=self.root/'receipt.json';self.value={'session':'owned session','native_only':{'accepted':True},'unicode':'\u2603','payload':'x'*32768}
 def test_original_dump_bytes_are_preserved(self):
  out=io.StringIO();json.dump(self.value,out,indent=2);out.write('\n');journal.publish(self.path,self.value);self.assertEqual(self.path.read_bytes(),out.getvalue().encode());self.assertEqual(list(self.root.iterdir()),[self.path])
 def test_partial_temporary_write_is_not_a_visible_journal(self):
  ready=threading.Event();resume=threading.Event();original=os.fdopen;errors=[]
  class Partial:
   def __init__(self,fd,mode):self.file=original(fd,mode)
   def __enter__(self):return self
   def __exit__(self,*args):return self.file.__exit__(*args)
   def write(self,body):
    half=len(body)//2;self.file.write(body[:half]);self.file.flush();ready.set()
    if not resume.wait(5):raise RuntimeError('Test publication barrier expired')
    return half+self.file.write(body[half:])
   def __getattr__(self,n):return getattr(self.file,n)
  def writer():
   try:journal.publish(self.path,self.value)
   except BaseException as e:errors.append(e)
  with patch.object(journal.os,'fdopen',Partial):
   thread=threading.Thread(target=writer);thread.start()
   try:
    self.assertTrue(ready.wait(5));self.assertEqual(list(self.root.glob('*.json')),[]);self.assertEqual(len(list(self.root.glob('*.pending'))),1)
   finally:resume.set();thread.join(5)
  self.assertFalse(thread.is_alive());self.assertEqual(errors,[]);self.assertEqual(json.loads(self.path.read_bytes()),self.value)
 def test_existing_receipt_is_never_replaced(self):
  self.path.write_bytes(b'original');before=self.path.stat().st_ino
  with self.assertRaises(FileExistsError):journal.publish(self.path,self.value)
  self.assertEqual(self.path.read_bytes(),b'original');self.assertEqual(self.path.stat().st_ino,before);self.assertEqual(list(self.root.iterdir()),[self.path])
 def test_existing_symlink_is_never_followed_or_replaced(self):
  target=self.root/'target';target.write_bytes(b'original');self.path.symlink_to(target)
  with self.assertRaises(FileExistsError):journal.publish(self.path,self.value)
  self.assertTrue(self.path.is_symlink());self.assertEqual(target.read_bytes(),b'original')
 def test_competing_publishers_have_one_complete_winner(self):
  barrier=threading.Barrier(2);results=[];link=os.link
  def synchronized(*args,**kwargs):barrier.wait(5);return link(*args,**kwargs)
  def writer(value):
   try:journal.publish(self.path,value);results.append('published')
   except FileExistsError:results.append('exists')
  with patch.object(journal.os,'link',synchronized):
   threads=[threading.Thread(target=writer,args=({'writer':i},))for i in range(2)]
   for t in threads:t.start()
   for t in threads:t.join(5)
  self.assertTrue(all(not t.is_alive()for t in threads));self.assertCountEqual(results,['published','exists']);self.assertIn(json.loads(self.path.read_bytes())['writer'],(0,1));self.assertEqual(list(self.root.iterdir()),[self.path])
 def test_publish_failure_leaves_no_visible_or_partial_receipt(self):
  with patch.object(journal.os,'link',side_effect=OSError('injected publish refusal')):
   with self.assertRaisesRegex(OSError,'publish refusal'):journal.publish(self.path,self.value)
  self.assertEqual(list(self.root.iterdir()),[])
 def test_unserializable_value_is_refused_before_creating_files(self):
  with self.assertRaises(TypeError):journal.publish(self.path,{'unsupported':b'bytes'})
  self.assertEqual(list(self.root.iterdir()),[])
 def test_legacy_exclusive_open_exposes_the_recorded_empty_parse_error(self):
  with self.path.open('x')as output:
   with self.assertRaises(json.JSONDecodeError)as caught:json.loads(self.path.read_text())
   self.assertEqual(str(caught.exception),'Expecting value: line 1 column 1 (char 0)');json.dump(self.value,output,indent=2);output.write('\n')
  self.assertEqual(json.loads(self.path.read_bytes()),self.value)
if __name__=='__main__':unittest.main()
