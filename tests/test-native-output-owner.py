"""Admission and namespace-boundary controls; no original game inputs."""
import copy
import importlib.util
import hashlib
import json
import os
from pathlib import Path
from types import SimpleNamespace
import stat
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
import native_output_owner as owner

spec=importlib.util.spec_from_file_location('owned_run',Path(__file__).resolve().parents[1]/'tools/run-native-owned.py')
owned_run=importlib.util.module_from_spec(spec);spec.loader.exec_module(owned_run)


class DisplayLine(unittest.TestCase):
    def number(self,chunks,server_code=None,ready=True):
        server=SimpleNamespace(poll=lambda:server_code)
        with patch.object(owned_run.select,'select',return_value=([7],[],[]) if ready else ([],[],[])),patch.object(owned_run.os,'read',side_effect=chunks) as read:
            value=owned_run._display_number(7,server)
            return value,read.call_count

    def test_complete_line(self):
        self.assertEqual(self.number([b'17\n']),('17',1))

    def test_split_digits_and_newline_must_all_be_consumed(self):
        self.assertEqual(self.number([b'1',b'7',b'\n']),('17',3))

    def test_digits_without_newline_refuse_eof(self):
        with self.assertRaisesRegex(RuntimeError,'before newline'):self.number([b'17',b''])

    def test_malformed_and_overlong_lines_refuse(self):
        for line in (b'\n',b' 17\n',b'17x\n',b'17\n4\n',b'\xff\n',b'9'*32):
            with self.subTest(line=line),self.assertRaises(RuntimeError):self.number([line])

    def test_timeout_or_exited_server_refuse(self):
        with self.assertRaisesRegex(RuntimeError,'complete display line'):self.number([],ready=False)
        with self.assertRaisesRegex(RuntimeError,'initialization'):self.number([b'17\n'],server_code=1)


class Ownership(unittest.TestCase):
    def setUp(self):
        self.temporary=tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root=Path(self.temporary.name)
        self.authority=self.root/'authority';self.authority.write_bytes(b'private-cookie')
        self.authority.chmod(0o600)
        self.metadata=dict(origin='menu_action_bridge',**{k:True for k in owner.REQUIRED})
        self.environment=dict(DISPLAY=':1',MNM_NATIVE_CPU_GUARD='3')
        self.receipt=dict(display=':63001',authority=str(self.authority),socket_identity=[7,8],
            authority_sha256=hashlib.sha256(self.authority.read_bytes()).hexdigest(),disabled_extensions=['MIT-SHM','DRI3','GLX'])
        self.socket=Path('/tmp/.X11-unix/X63001')
        original=Path.stat
        def query(path,*args,**kwargs):
            if path==self.socket:return SimpleNamespace(st_dev=7,st_ino=8,st_mode=stat.S_IFSOCK|0o600,st_uid=os.getuid())
            return original(path,*args,**kwargs)
        self.addCleanup(patch.stopall)
        patch.object(Path,'stat',query).start()

    def test_complete_native_policy_admits(self):
        self.assertEqual(owner.admit(self.metadata,self.environment,self.receipt),(self.socket,self.authority))

    def test_each_missing_or_untyped_capability_refuses(self):
        for key in owner.REQUIRED:
            for value in (False,1,None):
                metadata=dict(self.metadata);metadata[key]=value
                with self.subTest(key=key,value=value),self.assertRaises(ValueError):owner.admit(metadata,self.environment,self.receipt)
        with self.assertRaises(ValueError):owner.admit(dict(self.metadata,origin='other'),self.environment,self.receipt)

    def test_diagnostic_backing_modes_refuse(self):
        for mode in (None,'0','1','2','true'):
            with self.subTest(mode=mode),self.assertRaises(ValueError):owner.admit(self.metadata,dict(self.environment,MNM_NATIVE_CPU_GUARD=mode),self.receipt)

    def test_server_without_closed_output_extensions_refuses(self):
        for extensions in (None,[],['MIT-SHM','DRI3'],['GLX']):
            with self.subTest(extensions=extensions),self.assertRaises(ValueError):
                owner.admit(self.metadata,self.environment,dict(self.receipt,disabled_extensions=extensions))

    def test_foreign_noncanonical_or_host_display_refuses(self):
        for display in (None,':1','localhost:3',':063001',':-1',':65536',':3.0'):
            with self.subTest(display=display),self.assertRaises(ValueError):owner.admit(self.metadata,self.environment,dict(self.receipt,display=display))

    def test_replaced_socket_and_authority_refuse(self):
        with self.assertRaises(ValueError):owner.admit(self.metadata,self.environment,dict(self.receipt,socket_identity=[7,9]))
        self.authority.write_bytes(b'replaced-cookie')
        with self.assertRaises(ValueError):owner.admit(self.metadata,self.environment,self.receipt)

    def test_host_screen_and_unix_aliases_refuse(self):
        for display in (':63001.0','unix/:63001','unix:63001','host/unix:63001'):
            with self.subTest(display=display),self.assertRaises(ValueError):
                owner.admit(self.metadata,dict(self.environment,DISPLAY=display),self.receipt)

    def test_public_or_redirected_authority_refuses(self):
        self.authority.chmod(0o644)
        with self.assertRaises(ValueError):owner.admit(self.metadata,self.environment,self.receipt)
        self.authority.chmod(0o600)
        alias=self.root/'alias';alias.symlink_to(self.authority)
        with self.assertRaises(ValueError):owner.admit(self.metadata,self.environment,dict(self.receipt,authority=str(alias)))

    def test_receipt_requires_live_ancestor(self):
        path=self.root/'owner.json'
        receipt=dict(self.receipt,schema=1,policy='native-only-private-x11-v1',owner_pid=os.getppid())
        path.write_text(json.dumps(receipt));self.assertEqual(owner.load_receipt(path),receipt)
        for identity in (True,1,os.getpid(),2147483647):
            path.write_text(json.dumps(dict(receipt,owner_pid=identity)))
            with self.subTest(identity=identity),self.assertRaises((ValueError,OSError)):owner.load_receipt(path)

    def test_namespace_masks_all_host_display_routes(self):
        session=self.root/'session';prefix=self.root/'prefix';session.mkdir();prefix.mkdir()
        # Real production paths are not under /tmp; admission is tested above.
        with patch.object(owner,'admit',return_value=(self.socket,self.authority)):
            with patch.object(Path,'is_relative_to',return_value=False):
                command=owner.command(session,prefix,self.metadata,self.environment,self.receipt)
        for flag in ('--unshare-pid','--unshare-net','--unshare-ipc','--unshare-user','--cap-drop'):
            self.assertIn(flag,command)
        self.assertIn(['--dev','/dev'],[command[i:i+2] for i in range(len(command)-1)])
        for path in ('/tmp','/var/tmp'):
            self.assertIn(['--tmpfs',path],[command[i:i+2] for i in range(len(command)-1)])
        for key in ('WAYLAND_DISPLAY','LD_PRELOAD','LD_AUDIT','LD_LIBRARY_PATH','MNM_LEGACY_XCOOKIE'):
            self.assertIn(['--unsetenv',key],[command[i:i+2] for i in range(len(command)-1)])
        self.assertEqual(command[-1],'--')

    def test_verified_authority_is_explicitly_bound_after_masking_tmp(self):
        session=self.root/'session';prefix=self.root/'prefix';session.mkdir();prefix.mkdir()
        with patch.object(owner,'admit',return_value=(self.socket,self.authority)):
            with patch.object(Path,'is_relative_to',return_value=False):
                command=owner.command(session,prefix,self.metadata,self.environment,self.receipt)
        mount=['--ro-bind',str(self.authority),'/tmp/mnm-private-xauthority']
        triples=[command[i:i+3] for i in range(len(command)-2)]
        self.assertEqual(triples.count(mount),1)
        self.assertIn(['--setenv','XAUTHORITY','/tmp/mnm-private-xauthority'],triples)
        self.assertLess(command.index('--tmpfs'),triples.index(mount))
        self.assertNotIn(['--bind',str(self.authority),'/tmp/mnm-private-xauthority'],triples)

    def test_ram_checkout_is_explicitly_read_only_with_only_private_writable_children(self):
        repo=self.root/'repo';repo.mkdir();(repo/'.git').write_text('gitdir: retained')
        (repo/'AGENTS.md').write_text('owned');(repo/'tools').mkdir();(repo/'tools/menu-game-runner.py').write_text('pass')
        session=repo/'working/session';prefix=repo/'working/prefix';session.mkdir(parents=True);prefix.mkdir()
        with patch.object(owner,'admit',return_value=(self.socket,self.authority)):
            args=owner.command(session,prefix,self.metadata,self.environment,self.receipt,repository=repo)
        triples=[args[i:i+3] for i in range(len(args)-2)]
        self.assertIn(['--ro-bind',str(repo),str(repo)],triples)
        self.assertNotIn(['--bind',str(repo),str(repo)],triples)
        self.assertEqual([v for v in triples if v[0]=='--bind'],[['--bind',str(session),str(session)],['--bind',str(prefix),str(prefix)]])
        self.assertLess(triples.index(['--ro-bind',str(repo),str(repo)]),triples.index(['--bind',str(session),str(session)]))
        self.assertIn(['--tmpfs','/tmp'],[args[i:i+2] for i in range(len(args)-1)])

    def test_masked_targets_still_refuse_an_unbound_or_foreign_repository(self):
        session=self.root/'session';prefix=self.root/'prefix';session.mkdir();prefix.mkdir()
        with patch.object(owner,'admit',return_value=(self.socket,self.authority)):
            for repo in (None,self.root/'absent',Path('/tmp')):
                with self.subTest(repo=repo),self.assertRaises((ValueError,OSError)):
                    owner.command(session,prefix,self.metadata,self.environment,self.receipt,repository=repo)


if __name__=='__main__':unittest.main()
