#!/usr/bin/env python3
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from campaign_retained_policies import POLICIES,ADAPTED_POLICIES,policy_script
from coverage_evidence import Reader


class Policies(unittest.TestCase):
    def test_current_has_no_redirect(self):self.assertIsNone(policy_script(ROOT,'current'))

    def test_retained_sources_match_the_exact_successful_acquisition_hashes(self):
        with Reader(ROOT) as r:
            for policy,(name,expected,record) in POLICIES.items():
                self.assertEqual(r.json(record)['sources']['tools/campaign-outcome-input.py'],expected)
                self.assertEqual(policy_script(ROOT,policy),ROOT/'tools'/name)

    def test_unknown_policy_cannot_name_an_executable(self):
        for name in ('','../v34','/bin/sh','retained-v35'):
            with self.assertRaises(ValueError):policy_script(ROOT,name)

    def test_modified_or_missing_source_refuses(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'tools').mkdir()
            for policy,(name,_,_) in POLICIES.items():
                with self.assertRaises(ValueError):policy_script(root,policy)
                (root/'tools'/name).write_bytes(b'changed')
                with self.assertRaises(ValueError):policy_script(root,policy)

    def test_redirected_source_refuses_even_with_correct_bytes(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'tools').mkdir()
            for policy,(name,_,_) in POLICIES.items():
                (root/'tools'/name).symlink_to(ROOT/'tools'/name)
                with self.assertRaises(ValueError):policy_script(root,policy)

    def test_explicit_entry_dispatch_preserves_help_arguments_without_game_input(self):
        for policy in POLICIES|ADAPTED_POLICIES:
            p=subprocess.run([sys.executable,str(ROOT/'tools/campaign-outcome-input.py'),'--help'],
                cwd=ROOT,env=dict(os.environ,MNM_CAMPAIGN_OUTCOME_POLICY=policy),capture_output=True,text=True)
            self.assertEqual(p.returncode,0,p.stderr);self.assertIn('--experiment',p.stdout);self.assertIn('--frame-size',p.stdout)


if __name__=='__main__':unittest.main()
