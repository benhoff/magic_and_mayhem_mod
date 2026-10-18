#!/usr/bin/env python3
"""Fail closed on incomplete CPU-context proofs and collateral register changes."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
import hashlib
import json
import os
import runpy
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from raster_cpu_debug import decode, decode_rgb, wine_wrapper


class CpuProofTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / 'proof.bin'
        self.q = [0] * 62
        self.q[:14] = [1, 256, 1, 0, 0, 144, 148, 0x400000, 15, 1, 1, 1, 0x484001, 0x700000]
        self.q[14:22] = [0x50657, 17, 19, 23, 0x50f57, 17, 19, 23]
        self.q[22:30] = [0x5947b2, 0x595b47, 0x5968a4, 0x597086, 0, 15, 3, 1]

    def write(self):
        self.path.write_bytes(b'MNMCPU01' + struct.pack('<62I', *self.q))

    def test_context_is_explicitly_authored(self):
        self.write()
        proof = decode(self.path, 15)
        self.assertTrue(proof['verified'])
        self.assertEqual(proof['host_cpuid'][1:], proof['authored_cpuid'][1:])
        self.assertFalse(proof['dispatch_pointers_written_by_debugger'])

    def test_incomplete_error_exit_and_reserved_data_refuse(self):
        for index, value in ((2, 0), (3, 93), (4, 1), (8, 6), (9, 0), (10, 2), (11, 0),
                             (12, 0), (13, 0), (27, 6), (28, 2), (29, 0), (61, 1)):
            with self.subTest(index=index):
                old = self.q[index]; self.q[index] = value; self.write()
                with self.assertRaises(ValueError): decode(self.path, 15)
                self.q[index] = old

    def test_collateral_cpuid_changes_refuse(self):
        for index in range(18, 22):
            with self.subTest(index=index):
                self.q[index] ^= 1; self.write()
                with self.assertRaises(ValueError): decode(self.path, 15)
                self.q[index] ^= 1

    def test_wrong_original_dispatch_refuses(self):
        self.q[22] = 0x59521a; self.write()
        with self.assertRaises(ValueError): decode(self.path, 15)

    def test_original_override_remains_distinct(self):
        self.q[26] = 1; self.q[22] = 0x57de00; self.write()
        self.assertEqual(decode(self.path, 15)['override'], 1)

    def test_extent_and_magic_refuse(self):
        self.write(); valid = self.path.read_bytes()
        for data in (valid[:-1], valid + b'\0', b'INVALID!' + valid[8:]):
            self.path.write_bytes(data)
            with self.assertRaises(ValueError): decode(self.path, 15)

    def rgb_proof(self):
        self.write()
        cpu=decode(self.path,15)
        q=[0]*126
        q[:12]=[1,512,1,0,0,cpu['pid'],cpu['tid'],3,1,1,0x545ce9,8]
        q[62:70]=[2,4,0,0x700000,0x202,0,0,cpu['tid']]
        p=self.path.parent/'rgb.bin'
        return cpu,q,p

    def test_rgb_proof_is_bound_but_does_not_invent_native_admission(self):
        cpu,q,p=self.rgb_proof();p.write_bytes(b'MNMRGB01'+struct.pack('<126I',*q))
        r=decode_rgb(p,cpu)
        self.assertEqual((r['calls'],r['empty_calls']),(3,1))
        self.assertEqual(r['samples'][0]['sequence'],2)
        self.assertIs(r['native_admission_proved'],False)

    def test_rgb_incomplete_child_counts_and_reserved_data_refuse(self):
        cpu,q,p=self.rgb_proof()
        for at,value in ((2,0),(3,92),(4,1),(5,145),(6,149),(8,4),(9,0),(10,0),(11,9),(12,1),(125,1)):
            with self.subTest(at=at):
                old=q[at];q[at]=value;p.write_bytes(b'MNMRGB01'+struct.pack('<126I',*q))
                with self.assertRaises(ValueError):decode_rgb(p,cpu)
                q[at]=old

    def test_rgb_geometry_order_and_foreign_thread_refuse(self):
        cpu,q,p=self.rgb_proof()
        for at,value in ((62,0),(62,4),(63,0),(63,2049),(64,4),(65,0),(67,1),(68,2049),(69,999)):
            with self.subTest(at=at,value=value):
                old=q[at];q[at]=value;p.write_bytes(b'MNMRGB01'+struct.pack('<126I',*q))
                with self.assertRaises(ValueError):decode_rgb(p,cpu)
                q[at]=old

    def test_rgb_negative_right_and_empty_observation(self):
        cpu,q,p=self.rgb_proof();q[67]=0xffffffff;p.write_bytes(b'MNMRGB01'+struct.pack('<126I',*q))
        self.assertEqual(decode_rgb(p,cpu)['samples'][0]['right'],-1)
        q[8]=q[9]=0;q[62:70]=[0]*8;p.write_bytes(b'MNMRGB01'+struct.pack('<126I',*q))
        self.assertEqual(decode_rgb(p,cpu)['empty_calls'],0)

    def test_rgb_extent_magic_and_unverified_cpu_refuse(self):
        cpu,q,p=self.rgb_proof();valid=b'MNMRGB01'+struct.pack('<126I',*q)
        for data in (valid[:-1],valid+b'\0',b'INVALID!'+valid[8:]):
            p.write_bytes(data)
            with self.assertRaises(ValueError):decode_rgb(p,cpu)
        p.write_bytes(valid)
        with self.assertRaises(ValueError):decode_rgb(p,dict(cpu,verified=False))

    def test_wrapper_rejects_wrong_launch_changed_helper_and_reuse(self):
        # Every refusal occurs before Wine is invoked.
        directory = Path(self.directory.name)
        debugger = directory / 'debugger.exe'; debugger.write_bytes(b'fixture')
        wrapper = wine_wrapper(directory / 'bin', debugger, 15)
        result = subprocess.run([str(wrapper), 'Chaos.exe'], capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('unexpected game launch', result.stderr)
        game = directory / 'Chaos.exe'; game.write_bytes(b'game fixture')
        command = [str(wrapper), 'explorer', '/desktop=MagicMayhem,800x600', str(game)]
        debugger.write_bytes(b'changed')
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Debugger changed', result.stderr)
        debugger.write_bytes(b'fixture')
        launch = directory / 'debugger-launch.json'; launch.write_bytes(b'preserved')
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('FileExistsError', result.stderr)
        self.assertEqual(launch.read_bytes(), b'preserved')

    def test_batch_adapter_preserves_original_and_wait_tail(self):
        directory = Path(self.directory.name)
        debugger = directory / 'debugger.exe'; debugger.write_bytes(b'fixture')
        wrapper = wine_wrapper(directory / 'bin', debugger, 15)
        experiment = directory / 'experiment'; game = experiment / 'game/Chaos.exe'
        game.parent.mkdir(parents=True); game.write_bytes(b'game fixture')
        def winepath(path): return 'Z:' + str(path).replace('/', chr(92))
        batch = experiment / 'original-process-wait.cmd'
        original = ('@echo off\r\nstart "" /wait /b "' + winepath(game) + '" -fixture\r\n'
                    'set MNM_ORIGINAL_EXIT=%ERRORLEVEL%\r\nexit /b %MNM_ORIGINAL_EXIT%\r\n').encode()
        batch.write_bytes(original)
        manifest = experiment / 'manifest.json'
        manifest.write_text(json.dumps({'original_process_wait': {'batch_sha256': hashlib.sha256(original).hexdigest()}}))
        manifest_before = manifest.read_bytes()
        argv = [str(wrapper), 'explorer', '/desktop=MagicMayhem,800x600', 'cmd.exe', '/d', '/q', '/c', winepath(batch)]
        class Executed(Exception): pass
        with patch.object(sys, 'argv', argv), patch.dict(os.environ), patch('os.execv', side_effect=Executed) as launch:
            with self.assertRaises(Executed): runpy.run_path(str(wrapper), run_name='__main__')
            self.assertEqual(os.environ['MNM_RASTER_CPU_FAMILY'], '15')
        adapted = experiment / 'original-process-wait-cpu.cmd'
        self.assertEqual(batch.read_bytes(), original)
        self.assertEqual(manifest.read_bytes(), manifest_before)
        self.assertEqual(adapted.read_bytes(), original.replace(
            ('/b "' + winepath(game) + '"').encode(),
            ('/b "' + winepath(debugger) + '" "' + winepath(game) + '"').encode(), 1))
        self.assertEqual(launch.call_args.args[1][-1], winepath(adapted))
        record = json.loads((directory / 'debugger-launch.json').read_text())
        self.assertEqual(record['game'], str(game))
        self.assertEqual(record['batch']['sha256'], hashlib.sha256(original).hexdigest())

    def test_batch_adapter_rejects_unbound_input(self):
        directory = Path(self.directory.name)
        debugger = directory / 'debugger.exe'; debugger.write_bytes(b'fixture')
        wrapper = wine_wrapper(directory / 'bin', debugger, 15)
        batch = directory / 'original-process-wait.cmd'; batch.write_bytes(b'changed')
        (directory / 'manifest.json').write_text(json.dumps({'original_process_wait': {'batch_sha256': '0' * 64}}))
        command = [str(wrapper), 'explorer', '/desktop=MagicMayhem,800x600', 'cmd.exe', '/d', '/q', '/c',
                   'Z:' + str(batch).replace('/', chr(92))]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Wait batch changed', result.stderr)
        self.assertFalse((directory / 'original-process-wait-cpu.cmd').exists())

    def test_rgb_override_or_native_default_is_explicitly_recorded(self):
        for enabled in (False, True):
            directory = Path(self.directory.name) / str(enabled); directory.mkdir()
            debugger = directory / 'debugger.exe'; debugger.write_bytes(b'fixture')
            game = directory / 'Chaos.exe'; game.write_bytes(b'game fixture')
            wrapper = wine_wrapper(directory / 'bin', debugger, 15, empty_rgb=enabled)
            argv = [str(wrapper), 'explorer', '/desktop=MagicMayhem,800x600', str(game)]
            class Executed(Exception): pass
            with patch.object(sys, 'argv', argv), patch.dict(os.environ, {'MNM_PIXEL_EMPTY_RGB_NATIVE': '1', 'MNM_PIXEL_REFUSAL_DIAGNOSTICS': '1','MNM_RASTER_RGB_TRACE':'1'}), patch('os.execv', side_effect=Executed):
                with self.assertRaises(Executed): runpy.run_path(str(wrapper), run_name='__main__')
                self.assertEqual(os.environ.get('MNM_PIXEL_EMPTY_RGB_NATIVE'), '1' if enabled else None)
                self.assertNotIn('MNM_PIXEL_REFUSAL_DIAGNOSTICS', os.environ)
                self.assertNotIn('MNM_RASTER_RGB_TRACE',os.environ)
            record = json.loads((directory / 'debugger-launch.json').read_text())
            self.assertIs(record['empty_rgb'], enabled)
            self.assertEqual(record['empty_rgb_selection'],'enabled1' if enabled else 'default')

    def test_rgb_opt_out_overrides_ambient_enable_and_conflicts_refuse(self):
        directory=Path(self.directory.name)
        debugger=directory/'debugger.exe';debugger.write_bytes(b'fixture')
        game=directory/'Chaos.exe';game.write_bytes(b'game fixture')
        with self.assertRaises(ValueError):
            wine_wrapper(directory/'conflict',debugger,15,empty_rgb=True,disable_empty_rgb=True)
        self.assertFalse((directory/'conflict').exists())
        wrapper=wine_wrapper(directory/'bin',debugger,15,disable_empty_rgb=True)
        argv=[str(wrapper),'explorer','/desktop=MagicMayhem,800x600',str(game)]
        class Executed(Exception):pass
        with patch.object(sys,'argv',argv),patch.dict(os.environ,{'MNM_PIXEL_EMPTY_RGB_NATIVE':'1'}),patch('os.execv',side_effect=Executed):
            with self.assertRaises(Executed):runpy.run_path(str(wrapper),run_name='__main__')
            self.assertEqual(os.environ['MNM_PIXEL_EMPTY_RGB_NATIVE'],'0')
        self.assertEqual(json.loads((directory/'debugger-launch.json').read_text())['empty_rgb_selection'],'disabled0')

    def test_rgb_caller_trace_requires_explicit_selection(self):
        directory=Path(self.directory.name)
        debugger=directory/'debugger.exe';debugger.write_bytes(b'fixture')
        game=directory/'Chaos.exe';game.write_bytes(b'game fixture')
        wrapper=wine_wrapper(directory/'bin',debugger,15,rgb_caller_trace=True)
        argv=[str(wrapper),'explorer','/desktop=MagicMayhem,800x600',str(game)]
        class Executed(Exception):pass
        with patch.object(sys,'argv',argv),patch.dict(os.environ),patch('os.execv',side_effect=Executed):
            with self.assertRaises(Executed):runpy.run_path(str(wrapper),run_name='__main__')
            self.assertEqual(os.environ['MNM_RASTER_RGB_TRACE'],'1')
        self.assertIs(json.loads((directory/'debugger-launch.json').read_text())['rgb_caller_trace'],True)


if __name__ == '__main__':
    unittest.main()
