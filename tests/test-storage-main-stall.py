#!/usr/bin/env python3
"""Storage-targeted diagnostics cannot consume the early one-shot or gain credit."""
from pathlib import Path
import hashlib
import subprocess
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from rendering_stall_sampler import StorageLoadArm


class ArmTests(unittest.TestCase):
    def test_missing_or_partial_load_witness_does_not_arm(self):
        with tempfile.TemporaryDirectory() as tmp:
            arm=StorageLoadArm(Path(tmp)/'sampler')
            self.assertFalse(arm.ready())
            for raw in (b'',b'\x89PNG\r\n\x1a\n',b'\x89PNG\r\n\x1a\n'+b'x'*40):
                arm.path.write_bytes(raw)
                self.assertFalse(arm.ready())
                self.assertIsNone(arm.receipt)

    def test_arm_binds_completed_owned_screenshot_once(self):
        with tempfile.TemporaryDirectory() as tmp:
            arm=StorageLoadArm(Path(tmp)/'sampler')
            raw=b'\x89PNG\r\n\x1a\n'+b'x'*40+b'\0\0\0\0IEND\xaeB`\x82'
            arm.path.write_bytes(raw)
            self.assertTrue(arm.ready())
            self.assertEqual(arm.receipt['sha256'],hashlib.sha256(raw).hexdigest())
            self.assertFalse(arm.receipt['original_load_completion_inferred'])
            arm.path.unlink()
            self.assertTrue(arm.ready())

    def test_symlinked_witness_refuses(self):
        with tempfile.TemporaryDirectory() as tmp:
            arm=StorageLoadArm(Path(tmp)/'sampler');arm.path.symlink_to(Path(tmp)/'other')
            with self.assertRaises(ValueError):arm.ready()

    def test_unsupported_routes_and_nominal_census_refuse_before_execution(self):
        variants=[[],['--scenario','normal-quick'],['--scenario','normal-storage','--skip-intros'],
                  ['--scenario','normal-storage','--operations-provenance'],['--scenario','normal-storage','--stall-intro-host'],
                  ['--scenario','normal-storage','--stall-world-host'],['--scenario','normal-storage','--operation-census']]
        for args in variants:
            with self.subTest(args=args):
                p=subprocess.run([sys.executable,'-B',str(ROOT/'tools/run-rendering-acceptance-pilot.py'),
                    '--sample-storage-main','--report','research/runtime/rendering-normal-results-20261031/not-executed-storage-stall.json',*args],
                    capture_output=True,text=True,timeout=10)
                self.assertEqual(p.returncode,2)


if __name__=='__main__':unittest.main()
