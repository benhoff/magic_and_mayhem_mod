#!/usr/bin/env python3
"""Retired sources retain exact Git blobs, paths and restored metadata."""
from pathlib import Path
import hashlib,os,sqlite3,sys,tempfile,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from source_archive import connect,add,restore

class SourceArchives(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name);self.source=self.root/"source";self.source.mkdir()
        self.files=[]
        for name,data in (("tools/one.py",b"unique source\n"),("research/two.md",b"retained evidence\n")):
            p=self.source/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data);s=p.stat()
            blob=hashlib.sha1(b"blob "+str(len(data)).encode()+b"\0"+data).hexdigest()
            self.files.append((name,blob,[s.st_dev,s.st_ino,s.st_size,s.st_mtime_ns,s.st_ctime_ns]))
        self.db=connect(self.root/"cold.sqlite3");self.addCleanup(self.db.close)
    def test_exact_restore_and_shared_objects(self):
        snapshot,files=add(self.db,self.source,"a"*40,self.files,0)
        output=self.root/"restored"
        self.assertEqual(restore(self.db,snapshot,output),2)
        for name,blob,identity in self.files:
            self.assertEqual((output/name).read_bytes(),(self.source/name).read_bytes())
            self.assertEqual((output/name).stat().st_mtime_ns,identity[3])
        add(self.db,self.source,"b"*40,self.files,0)
        self.assertEqual(self.db.execute("select count(*) from blobs").fetchone()[0],2)
    def test_changed_source_refuses(self):
        (self.source/self.files[0][0]).write_bytes(b"changed")
        with self.assertRaises(ValueError):add(self.db,self.source,"a"*40,self.files,0)
    def test_unexpected_git_bytes_refuse(self):
        self.files[0]=(self.files[0][0],"0"*40,self.files[0][2])
        with self.assertRaisesRegex(ValueError,"Git blob"):add(self.db,self.source,"a"*40,self.files,0)
    def test_manifest_omission_and_unsafe_paths_refuse(self):
        snapshot,_=add(self.db,self.source,"a"*40,self.files,0)
        self.db.execute("drop trigger files_no_delete");self.db.execute("delete from files where path='tools/one.py'");self.db.commit()
        with self.assertRaisesRegex(ValueError,"manifest mismatch"):restore(self.db,snapshot,self.root/"missing")
        self.files[0]=("../outside",self.files[0][1],self.files[0][2])
        with self.assertRaises(ValueError):add(self.db,self.source,"a"*40,self.files,0)
    def test_corrupted_archive_refuses(self):
        snapshot,_=add(self.db,self.source,"a"*40,self.files,0)
        self.db.execute("drop trigger blobs_no_update");self.db.execute("update blobs set payload=x'00'");self.db.commit()
        with self.assertRaises(Exception):restore(self.db,snapshot,self.root/"corrupt")
    def test_reserve_refusal_keeps_source(self):
        with self.assertRaisesRegex(ValueError,"reserve"):add(self.db,self.source,"a"*40,self.files,10**20)
        self.assertTrue((self.source/self.files[0][0]).exists())
if __name__=="__main__":unittest.main()
