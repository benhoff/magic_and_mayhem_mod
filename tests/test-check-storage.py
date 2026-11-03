#!/usr/bin/env python3
"""Storage controls: diagnostic boundaries, shared inputs and comparison custody."""
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from check_capture_policy import policy,require_full,stop_private_prefix
from check_inputs import provision,paths
from comparison_process import Processes
from coverage_evidence import write_report
from evidence_workspace import register_completed,finish_completed,MARKER


class Capture(unittest.TestCase):
    def test_default_has_no_pixel_proof_and_full_is_explicit(self):
        self.assertFalse(policy()['full_capture']);self.assertEqual(policy()['deadline_seconds'],120)
        self.assertTrue(policy('full')['full_capture']);self.assertEqual(policy('full')['deadline_seconds'],900)
        with self.assertRaisesRegex(ValueError,'Lightweight'):
            require_full(dict(configuration=dict(evidence_mode='diagnostic')))
        require_full(dict(configuration=dict(evidence_mode='full')))
        require_full(dict(configuration={})) # Historical acquisitions always captured.

    def test_bounds_refuse_invalid_modes_and_unbounded_deadlines(self):
        for mode,seconds in [('unknown',120),('full',0),('diagnostic',901),('full',True)]:
            with self.subTest(mode=mode,seconds=seconds),self.assertRaises(ValueError):policy(mode,seconds)

    def test_prefix_timeout_only_stops_the_owned_prefix(self):
        calls=[]
        def run(command,**kwargs):
            calls.append((command,kwargs['env']['WINEPREFIX']))
            if len(calls)==1:raise subprocess.TimeoutExpired(command,5)
            return subprocess.CompletedProcess(command,0)
        with patch('check_capture_policy.subprocess.run',side_effect=run):
            self.assertTrue(stop_private_prefix(Path('/owned/loan'),{})['forced'])
        self.assertEqual([c[0][-1] for c in calls],['-w','-k','-w'])
        self.assertEqual({c[1] for c in calls},{'/owned/loan'})

    def test_owned_processes_stop_after_success_failure_and_timeout(self):
        for code,timeout in [('pass',5),('raise SystemExit(7)',5),('import time;time.sleep(30)',.05)]:
            p=Processes()
            with tempfile.TemporaryFile() as log:
                try:p.run([sys.executable,'-c',code],cwd=ROOT,stdout=log,timeout=timeout)
                except (subprocess.CalledProcessError,subprocess.TimeoutExpired):pass
            self.assertTrue(p.stopped())


class Inputs(unittest.TestCase):
    def test_two_worktrees_share_one_copy_and_drift_refuses(self):
        with tempfile.TemporaryDirectory() as directory:
            owner=Path(directory);source=owner/'working/source';source.mkdir(parents=True)
            (source/'data.bin').write_bytes(b'immutable fixture')
            roots=[owner/'working/first',owner/'working/second']
            with patch('check_inputs._owner',return_value=owner):
                first=provision(roots[0],dict(game=source,assets=source))
                second=provision(roots[1],dict(game=source,assets=source))
                self.assertEqual(first['game']['path'],second['game']['path'])
                self.assertTrue((roots[0]/'working/game-nocd').is_symlink())
                self.assertEqual(paths(roots[0])['assets'],paths(roots[1])['assets'])
                self.assertTrue((roots[0]/'working/assets/native-cd-audio-20261012').is_symlink())
                self.assertEqual((source/'data.bin').read_bytes(),b'immutable fixture')
                target=paths(roots[0])['game'];(target/'data.bin').write_bytes(b'changed')
                with self.assertRaisesRegex(ValueError,'changed'):paths(roots[0])
                with self.assertRaisesRegex(ValueError,'differs'):provision(roots[1],dict(game=source))

    def test_game_alias_always_becomes_a_private_writable_runtime(self):
        self.runtime_alias_control(False)

    def test_preexisting_runtime_alias_is_refused(self):
        self.runtime_alias_control(True)

    def runtime_alias_control(self,existing):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);working=root/'working';working.mkdir()
            source=working/'pristine';source.mkdir();(source/'Chaos.exe').write_bytes(b'original')
            (working/'game-nocd').symlink_to(source)
            runtime=working/'runtime';runtime.mkdir()
            if existing:(runtime/'game-nocd').symlink_to(source)
            text=(ROOT/'tools/run-game.sh').read_text()
            select=text.split('select_game() {',1)[1].split('verify_game() {',1)[0]
            stage=text.split('select_runtime_tree() {',1)[1].split('resolve_runtime() {',1)[0]
            script=root/'probe.sh';script.write_text('set -eu\ndie(){ echo "$*" >&2; exit 7; }\nselect_game() {'+select+'select_runtime_tree() {'+stage+'select_game\nselect_runtime_tree\nprintf changed >"$GAME_DIR/Chaos.exe"\n')
            result=subprocess.run(['bash',str(script)],capture_output=True,env=dict(os.environ,WORKING_DIR=str(working),RUNTIME_ROOT=str(runtime),REPO_DIR=str(root),VARIANT='nocd',SOURCE_GAME_OVERRIDE=''))
            self.assertEqual(result.returncode,7 if existing else 0,result.stderr)
            self.assertEqual((source/'Chaos.exe').read_bytes(),b'original')
            if not existing:
                self.assertFalse((runtime/'game-nocd').is_symlink())
                self.assertEqual((runtime/'game-nocd/Chaos.exe').read_bytes(),b'changed')

    def test_conflicting_alias_is_preserved(self):
        with tempfile.TemporaryDirectory() as directory:
            owner=Path(directory);source=owner/'working/source';source.mkdir(parents=True)
            (source/'data').write_bytes(b'input');root=owner/'working/task'
            old=root/'working/game-nocd';old.mkdir(parents=True);(old/'save').write_bytes(b'keep')
            with patch('check_inputs._owner',return_value=owner),self.assertRaisesRegex(ValueError,'preserved'):
                provision(root,dict(game=source))
            self.assertEqual((old/'save').read_bytes(),b'keep')

    def test_catalog_only_keeps_existing_private_input_directories(self):
        with tempfile.TemporaryDirectory() as directory:
            owner=Path(directory);source=owner/'working/source';source.mkdir(parents=True)
            (source/'data').write_bytes(b'input');root=owner/'working/task'
            old=root/'working/game-nocd';old.mkdir(parents=True);(old/'save').write_bytes(b'keep')
            with patch('check_inputs._owner',return_value=owner):
                provision(root,dict(game=source),aliases=False)
                self.assertNotEqual(paths(root)['game'],old)
            self.assertFalse(old.is_symlink());self.assertEqual((old/'save').read_bytes(),b'keep')


spec=importlib.util.spec_from_file_location('object_controls',ROOT/'tests/test-evidence-objects.py')
fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)


class Comparisons(unittest.TestCase):
    setUp=fixture.Objects.setUp

    def report(self,*,stopped=True):
        output=self.root/'working/pilot/comparison';output.mkdir(parents=True)
        (output/'.retain').write_text('original comparison pin')
        (output/'words.bin').write_bytes(b'exact original comparison words')
        artifacts=[]
        for f in output.iterdir():
            name='research/runtime/raw-'+f.name+'.bin';sha=write_report(self.root,name,f.read_bytes())
            artifacts.append(dict(path=f.name,record=name,sha256=sha,bytes=f.stat().st_size))
        source=self.root/'tools/driver.py';source.parent.mkdir();source.write_bytes(b'executed original phase')
        data=dict(kind='public_native_checkpoint_diagnostic_comparison',success=False,
                  output_directory=str(output),comparison_processes_stopped=stopped,
                  artifact_retention_complete=True,artifacts=artifacts,
                  sources={'tools/driver.py':hashlib.sha256(source.read_bytes()).hexdigest()})
        name='research/runtime/comparison.json';sha=write_report(self.root,name,(json.dumps(data)+'\n').encode())
        return output,dict(record=name,sha256=sha)

    def test_comparison_queue_does_not_delete_until_verified_finish(self):
        output,ref=self.report();register_completed(self.root,[output],ref,producer='comparison')
        self.assertEqual((output/'words.bin').read_bytes(),b'exact original comparison words')
        results=finish_completed(self.root)
        self.assertTrue(results[0]['real_restore_verified']);self.assertTrue((output/MARKER).is_file())

    def test_unstopped_or_changed_comparison_cannot_queue(self):
        output,ref=self.report(stopped=False)
        with self.assertRaisesRegex(ValueError,'stopped producer'):register_completed(self.root,[output],ref,producer='comparison')
        self.assertTrue((output/'words.bin').is_file())

    def test_unretained_or_changed_bytes_refuse(self):
        output,ref=self.report();(output/'words.bin').write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError,'artifact differs'):register_completed(self.root,[output],ref,producer='comparison')
        self.assertTrue((output/'words.bin').is_file())

    def test_wine_proof_is_still_required_for_wine_queue(self):
        output,ref=self.report()
        with self.assertRaisesRegex(ValueError,'stopped producer'):register_completed(self.root,[output],ref)


if __name__=='__main__':unittest.main()
