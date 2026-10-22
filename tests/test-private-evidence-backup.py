#!/usr/bin/env python3
"""Exercise actual remote verification locally, interruption/retry and restoration."""
from contextlib import closing
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import sqlite3
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Store, digest_file
from evidence_local import backup_sqlite
spec=importlib.util.spec_from_file_location('private_backup',ROOT/'tools/backup-evidence.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

class BackupTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)/'repo';self.root.mkdir()
        self.remote=Path(self.tmp.name)/'remote'
        self.run=subprocess.run;self.transfers=0
        self.git('init','-q');self.git('config','user.name','Fixture');self.git('config','user.email','fixture@example.invalid')
        (self.root/'.evidence-store.json').write_text('{"schema":1,"archives":[]}')
        self.git('add','.');self.git('commit','-qm','Fixture')
        self.git('config','mm.evidenceBackupHost','arch')
        self.git('config','mm.evidenceBackupDirectory',str(self.remote))
        self.source=self.root/'working/coverage/evidence-pending.sqlite3'
        store=Store(self.source)
        with store.db:store.put(io.BytesIO(b'{"restored":true}'),17)
        store.close()
        self.backup=backup_sqlite(self.source,self.root/'working/snapshots')

    def git(self,*args):return subprocess.check_output(['git','-C',str(self.root),*args]).decode().strip()

    def local_remote(self,host,directory,request):
        self.assertEqual(host,'arch')
        p=self.run([sys.executable,'-c',m.REMOTE,directory],input=json.dumps(request),text=True,capture_output=True)
        if p.returncode:raise ValueError(p.stderr)
        return json.loads(p.stdout)

    def transport(self,argv,**kwargs):
        if argv[0]!='rsync':return self.run(argv,**kwargs)
        self.assertEqual(argv[argv.index('--')+1],argv[-2])
        dest=Path(argv[-1].split(':',1)[1]);shutil.copyfile(argv[-2],dest)
        self.transfers+=1
        return subprocess.CompletedProcess(argv,0)

    def upload(self,sid):
        with patch.object(m,'remote',self.local_remote),patch.object(m.subprocess,'run',self.transport):
            return m.upload(self.root,sid)

    def test_round_trip_restores_verified_evidence_and_commit_manifest(self):
        sid=m.prepare(self.root,'HEAD',[self.backup]);result=self.upload(sid)
        self.assertEqual(result['state'],'verified')
        path=self.remote/'objects'/(self.backup['sha256']+'.sqlite3')
        self.assertEqual(digest_file(path),self.backup['sha256'])
        restored=Store(path,readonly=True)
        try:self.assertEqual(restored.verify(),1)
        finally:restored.close()
        with closing(sqlite3.connect(self.remote/'sets'/result['manifest'])) as db:
            self.assertEqual(db.execute('SELECT commit_id FROM backup').fetchone()[0],self.git('rev-parse','HEAD'))
        self.assertEqual(path.stat().st_mode&0o777,0o600)
        self.assertEqual(self.remote.stat().st_mode&0o777,0o700)

    def test_second_set_skips_existing_immutable_database(self):
        self.upload(m.prepare(self.root,'HEAD',[self.backup]));self.transfers=0
        result=self.upload(m.prepare(self.root,'HEAD',[self.backup]))
        self.assertEqual(result['uploaded_databases'],0);self.assertEqual(self.transfers,1)

    def test_failed_upload_preserves_commit_snapshot_and_pending_retry(self):
        head=self.git('rev-parse','HEAD');sid=m.prepare(self.root,'HEAD',[self.backup])
        with patch.object(m,'remote',side_effect=OSError('offline')):
            with self.assertRaises(OSError):m.upload(self.root,sid)
        with closing(m.catalog(self.root)) as db:self.assertEqual(db.execute('SELECT state FROM sets').fetchone()[0],'pending')
        self.assertEqual(self.git('rev-parse','HEAD'),head)
        self.assertEqual(digest_file(self.backup['path']),self.backup['sha256'])
        self.assertEqual(self.upload(sid)['state'],'verified')

    def test_corrupt_remote_object_is_not_overwritten_or_marked_verified(self):
        self.upload(m.prepare(self.root,'HEAD',[self.backup]))
        path=self.remote/'objects'/(self.backup['sha256']+'.sqlite3');path.write_bytes(b'corrupt')
        sid=m.prepare(self.root,'HEAD',[self.backup])
        with self.assertRaises(ValueError):self.upload(sid)
        self.assertEqual(path.read_bytes(),b'corrupt')
        with closing(m.catalog(self.root)) as db:self.assertEqual(db.execute('SELECT state FROM sets WHERE id=?',(sid,)).fetchone()[0],'pending')

    def test_changed_local_snapshot_refuses_before_network(self):
        sid=m.prepare(self.root,'HEAD',[self.backup]);Path(self.backup['path']).write_bytes(b'changed')
        with patch.object(m,'remote',side_effect=AssertionError('Network forbidden')):
            with self.assertRaises(ValueError):m.upload(self.root,sid)

    def test_missing_pinned_archive_refuses_without_network(self):
        (self.root/'.evidence-store.json').write_text(json.dumps({'schema':1,'archives':[{'sha256':'a'*64}]}))
        with patch.object(m,'cache_root',return_value=self.root/'empty'):
            with self.assertRaisesRegex(ValueError,'unavailable locally'):m.prepare(self.root,'HEAD',[self.backup])

    def test_unsafe_host_and_directory_refuse(self):
        self.git('config','mm.evidenceBackupHost','-oProxyCommand=bad')
        with self.assertRaises(ValueError):m.destination(self.root)
        self.git('config','mm.evidenceBackupHost','arch');self.git('config','mm.evidenceBackupDirectory','/home/hoff/../escape')
        with self.assertRaises(ValueError):m.destination(self.root)

    def test_destination_change_cannot_redirect_existing_set(self):
        sid=m.prepare(self.root,'HEAD',[self.backup]);self.git('config','mm.evidenceBackupHost','other')
        with patch.object(m,'remote',side_effect=AssertionError('Network forbidden')):
            with self.assertRaisesRegex(ValueError,'different destination'):m.upload(self.root,sid)

    def test_live_wal_is_included_before_remote_transfer(self):
        with closing(sqlite3.connect(self.source)) as live:
            live.execute('PRAGMA journal_mode=WAL');live.execute('CREATE TABLE live(value TEXT)');live.execute("INSERT INTO live VALUES ('present')");live.commit()
            backup=backup_sqlite(self.source,self.root/'working/live-backups')
            self.upload(m.prepare(self.root,'HEAD',[backup]))
        with closing(sqlite3.connect(self.remote/'objects'/(backup['sha256']+'.sqlite3'))) as db:
            self.assertEqual(db.execute('SELECT value FROM live').fetchone(),('present',))

if __name__=='__main__':unittest.main()
