#!/usr/bin/env python3
"""Baseline guards: source inventory and historical proof cannot imply acceptance."""
from copy import deepcopy
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_baseline import revision_tree, summarize, write


def fixture():
    behavior = dict(id='NR.one', status=dict(understanding='scoped', implementation='scoped',
                    comparison='recorded', integration='headless', replacement='none'), evidence=['E.old', 'E.new'])
    register = dict(behaviors=[behavior], evidence=[dict(id=e, kind='native_integration', record=e+'.json',
                    sha256='a'*64, scope='Fixture only') for e in behavior['evidence']])
    reviewed = dict(audit_valid=True, behaviors=[behavior],
                    stale_evidence=[dict(id='E.old', changed_sources=['renderer/a.cpp'])])
    plan = dict(reviewed_commit='old-review', contracts=[dict(id='C1', behaviors=['NR.one']),
                dict(id='C2', behaviors=['OUTSIDE.other'])],
                journeys=[dict(id='active-world', contracts=['C1'], conditional_contracts=['C2'])],
                operations=[dict(id='copy', contracts=['C1'])], blockers=[dict(id='B1')], work_packages=[])
    return register, reviewed, plan, dict(groups=[])


class BaselineTests(unittest.TestCase):
    def test_scoped_code_and_matching_sources_do_not_accept_player(self):
        r, a, p, d = fixture()
        before = deepcopy(r)
        result = summarize(r, a, p, d)
        self.assertEqual(r, before)
        self.assertEqual(result['registered_status_counts']['implementation'], {'scoped': 1})
        self.assertFalse(result['complete_player_accepted'])
        self.assertFalse(result['original_recovery_complete'])
        self.assertIsNone(result['overall_completion_percent'])
        self.assertTrue(all(not row['acceptance_assessed'] for row in result['requirements']))

    def test_changed_and_matching_provenance_are_distinct(self):
        result = summarize(*fixture())
        self.assertEqual([r['source_binding_state'] for r in result['evidence']], ['changed', 'matching'])
        self.assertEqual(result['evidence'][0]['changed_sources'], ['renderer/a.cpp'])

    def test_audit_failure_suppresses_source_credit(self):
        r, a, p, d = fixture()
        a['audit_valid'] = False
        self.assertEqual({e['source_binding_state'] for e in summarize(r, a, p, d)['evidence']}, {'unknown'})

    def test_conditional_and_outside_dependencies_are_retained(self):
        result = summarize(*fixture())
        row = result['requirements'][0]
        self.assertEqual(row['conditional_behaviors'], ['OUTSIDE.other'])
        self.assertEqual(row['behaviors_outside_rendering_selection'], ['OUTSIDE.other'])
        self.assertEqual(result['authored_plan_reviewed_commit'], 'old-review')
        self.assertFalse(result['authored_plan_states_reassessed'])

    def test_invalid_outputs_are_refused_before_writer(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for path in ('original/a.json', 'working/a.json', 'research/runtime/a.txt'):
                with self.assertRaises(ValueError):
                    write(root, path, {})
            target = root/'research/runtime/existing.json'
            target.parent.mkdir(parents=True)
            target.write_text('old')
            with self.assertRaises(ValueError):
                write(root, target, {})

    def test_revision_export_excludes_staged_work_and_preserves_environment(self):
        # Fixture Git writes must never reach an invocation's private config,
        # index, worktree or command-scope overrides.
        environment = {k: v for k, v in os.environ.items() if not k.startswith('GIT_')}
        with patch.dict(os.environ, environment, clear=True), tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            def git(*args):
                return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE).decode().strip()
            git('init', '-q')
            git('config', 'user.name', 'Fixture')
            git('config', 'user.email', 'fixture@example.invalid')
            (root/'source.txt').write_text('committed')
            (root/'working').mkdir()
            (root/'working/.gitkeep').write_text('')
            git('add', 'source.txt')
            git('add', '-f', 'working/.gitkeep')
            git('commit', '-qm', 'fixture')
            expected = git('rev-parse', 'HEAD')
            (root/'source.txt').write_text('staged')
            git('add', 'source.txt')
            (root/'untracked.txt').write_text('excluded')
            (root/'original').mkdir()
            (root/'original/secret').write_text('excluded')
            missing = root/'working/missing.sqlite3'
            with patch.dict(os.environ, {'MM_EVIDENCE_EXTRA_DB': '["'+str(missing)+'"]'}):
                previous = {k: os.environ.get(k) for k in
                            ('MM_EVIDENCE_DB', 'MM_EVIDENCE_CACHE', 'MM_EVIDENCE_EXTRA_DB')}
                with revision_tree(root, 'HEAD') as (snapshot, identity):
                    self.assertEqual(identity['commit'], expected)
                    self.assertEqual((snapshot/'source.txt').read_text(), 'committed')
                    self.assertFalse((snapshot/'untracked.txt').exists())
                    self.assertFalse((snapshot/'original').exists())
                    self.assertFalse((snapshot/'working').exists())
                    self.assertIn(str(missing), identity['unavailable_local_stores'])
                self.assertEqual(previous, {k: os.environ.get(k) for k in previous})
            self.assertEqual((root/'source.txt').read_text(), 'staged')


if __name__ == '__main__':
    unittest.main()
