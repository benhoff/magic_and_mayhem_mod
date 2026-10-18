#!/usr/bin/env python3
"""Keep short census observations separate from ordinary campaign acceptance."""
import importlib.util
from pathlib import Path
import subprocess
import sys
import unittest
import struct
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('census_campaign',ROOT/'tools/test-native-campaign.py')
campaign=importlib.util.module_from_spec(spec)
spec.loader.exec_module(campaign)

class CensusPolicy(unittest.TestCase):
    def resident(self,**changes):
        value=dict(completed='8',imports='8',verified_returns='8',engine_fps=0,error='',
                   world_readbacks=0,native_readbacks=0,rgba_readbacks=0,
                   normal_quit_retired=True,pending=False)
        value.update(changes)
        return value

    def test_ordinary_count_floor_remains(self):
        with self.assertRaisesRegex(ValueError,'completion/import'):
            campaign.validate_resident_cpu_census(self.resident(engine_fps=30),20)

    def test_ordinary_fps_floor_remains(self):
        with self.assertRaisesRegex(ValueError,'FPS below'):
            campaign.validate_resident_cpu_census(self.resident(completed=64,imports=64,verified_returns=64),20)

    def test_ordinary_acceptance(self):
        gates=campaign.validate_resident_cpu_census(self.resident(completed=64,imports=64,verified_returns=64,engine_fps=30),20)
        self.assertTrue(gates['worlds_passed'] and gates['fps_passed'])

    def test_census_retains_failed_ordinary_gates(self):
        gates=campaign.validate_resident_cpu_census(self.resident(),20,True)
        self.assertEqual(gates,dict(minimum_worlds=64,worlds_passed=False,minimum_fps=20,fps_passed=False))

    def test_census_rejects_incomplete_or_readback(self):
        for changes in (dict(completed=0),dict(imports=7),dict(verified_returns=7),
                        dict(pending=True),dict(normal_quit_retired=False),dict(error='failed'),
                        dict(world_readbacks=1),dict(native_readbacks=1),dict(rgba_readbacks=1)):
            with self.subTest(changes=changes),self.assertRaises(ValueError):
                campaign.validate_resident_cpu_census(self.resident(**changes),20,True)

    def test_explicit_mode_and_claims_required_before_launch(self):
        result=subprocess.run([sys.executable,str(ROOT/'tools/test-native-campaign.py'),
                               '--cpu-census-only','--cpu-guard','1'],capture_output=True,text=True)
        self.assertEqual(result.returncode,2)
        self.assertIn('CPU census-only requires',result.stderr)

    def test_coherent_mode_has_exclusive_claims(self):
        claims=[dict(behavior='NP.active-lock-reader-restoration'),dict(behavior='NP.protected-cpu-lease-storage')]
        campaign.validate_cpu_census_claims(3,claims)
        for mode,items in [(2,claims),(3,claims+[dict(behavior='HOST.other')]),(3,[dict(behavior='NP.active-lock-reader-census')]),(1,claims)]:
            with self.subTest(mode=mode,items=items),self.assertRaises(ValueError):campaign.validate_cpu_census_claims(mode,items)
        campaign.validate_cpu_census_claims(2,[dict(behavior='NP.active-lock-reader-census')])

    def test_reader_kind_counts(self):
        words=[0x524d4e4d,0x31304344,1,48,16,0,15,1,0,0,0,0]
        raw=struct.pack('<12I',*words)
        self.assertEqual(campaign.parse_cpu_reader_counts(raw,16),dict(getdc=0,typed_lock=15,byte_unlock=1,typed_unborrowed=0))
        for data,ticket in [(raw[:-1],16),(raw+b'\0',16),(raw,15),(raw,0)]:
            with self.subTest(ticket=ticket,size=len(data)),self.assertRaises(ValueError):campaign.parse_cpu_reader_counts(data,ticket)
        for i in range(12):
            changed=words[:];changed[i]^=1
            with self.subTest(word=i),self.assertRaises(ValueError):campaign.parse_cpu_reader_counts(struct.pack('<12I',*changed),16)

if __name__=='__main__':unittest.main()
