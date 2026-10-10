#!/usr/bin/env python3
"""Guard reviewed denominators and historical/current replacement distinctions."""
import copy
import hashlib
import json
import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from rendering_coverage import ROOT, reconcile, read

spec = importlib.util.spec_from_file_location('rendering_report_cli', ROOT / 'tools/report-rendering-coverage.py')
cli = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cli)


class RenderingCoverageTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.record = {'success': True, 'original': 'pinned', 'equivalent': True, 'bypassed': True,
                       'actual_entry_counts': {'0x10': 3}, 'complete_raster_queues': [1], 'caller_ax_equal': 3}
        data = json.dumps(self.record).encode()
        (self.root / 'proof.json').write_bytes(data)
        e = {'id': 'proof', 'scope': 'One bounded draw', 'record': 'proof.json', 'sha256': hashlib.sha256(data).hexdigest(),
             'kind': 'live_replacement', 'build': 'build', 'original_hash_pointer': '/original',
             'equivalence_pointer': '/equivalent', 'bypass_pointer': '/bypassed',
             'assertions': [{'pointer': '/success', 'equals': True}]}
        b = {'id': 'RI.fixture', 'kind': 'recovered', 'scope': 'One bounded draw',
             'implementation': ['native.cpp'], 'evidence': ['proof'],
             'status': {'implementation': 'scoped', 'comparison': 'recorded', 'replacement': 'scoped_live'}}
        release = {'id': 'NR.live-drawing-replacement', 'scope': 'Complete session', 'status': {'implementation': 'none'}}
        self.register = {'builds': [{'id': 'build', 'source_sha256': 'pinned'}],
                         'behaviors': [b, release], 'evidence': [e], 'dispatch_tables': []}
        self.matrix = {'build': 'build', 'source_sha256': 'pinned', 'operations': [
            {'id': 'required', 'requirement': 'required', 'remaining_gap': 'All other cases'},
            {'id': 'conditional', 'requirement': 'conditional'}]}
        row = {'id': 'required', 'implementation': ['RI.fixture'], 'scope': 'One case', 'remaining': 'All other cases',
               'validation': [{'behavior': 'RI.fixture', 'evidence': 'proof', 'domain': 'original_executable'}],
               'replacement': [{'behavior': 'RI.fixture', 'evidence': 'proof'}]}
        route = {**row, 'id': 'RI.fixture'}
        self.definition = {'schema': 1, 'build': 'build', 'scope': 'Bounded subsets only',
                           'surface_operations': [row], 'drawing_routes': [route],
                           'world_takeovers': [{'behavior': 'RI.fixture', 'evidence': 'proof', 'admitted_entries': ['0x10', '0x20']}]}
        self.audit = {'errors': [], 'warnings': [], 'findings': {'stale_evidence': []},
                      'behaviors': [{'id': 'RI.fixture', 'current_validation': {
                          stage: {'state': 'current', 'evidence': ['proof']} for stage in ('implementation', 'comparison', 'replacement')}}]}

    def report(self):
        return reconcile(self.root, self.register, self.matrix, self.definition, self.audit)

    def test_scoped_support_does_not_become_overall_completion(self):
        r = self.report()
        self.assertEqual(r['surface_operations']['scoped_code_support']['percent'], 100)
        self.assertIsNone(r['overall_completion_percent'])
        self.assertEqual(r['release_contract']['status']['implementation'], 'none')
        self.assertEqual(len(r['conditional_surface_operations']), 1)
        self.assertEqual(r['world_takeovers'][0]['exercised_entries']['percent'], 50)
        self.assertEqual(r['world_takeovers'][0]['unobserved_entries'], ['0x20'])

    def test_audit_errors_preserve_history_but_suppress_current_claims(self):
        self.audit['errors'] = ['Unrelated incomplete scenario']
        r = self.report()
        self.assertEqual(r['drawing_routes']['scoped_live_replacement_support']['count'], 1)
        self.assertIsNone(r['drawing_routes']['current_support']['replacement'])
        self.assertFalse(r['world_takeovers'][0]['current'])

    def test_stale_and_changed_scope_do_not_count_current(self):
        self.audit['findings']['stale_evidence'] = [{'id': 'proof'}]
        self.audit['behaviors'][0]['current_validation']['replacement'] = {'state': 'pending', 'evidence': []}
        r = self.report()
        self.assertEqual(r['world_takeovers'][0]['source_freshness'], 'stale')
        self.assertFalse(r['world_takeovers'][0]['current'])

    def test_required_denominator_changes_require_review(self):
        self.matrix['operations'].append({'id': 'new', 'requirement': 'required'})
        with self.assertRaisesRegex(ValueError, 'denominator'):
            self.report()

    def test_route_denominator_changes_require_review(self):
        self.register['behaviors'].append({'id': 'RI.new'})
        with self.assertRaisesRegex(ValueError, 'denominator'):
            self.report()

    def test_empty_render_dispatch_remains_visible(self):
        self.register['dispatch_tables'] = [
            {'id': 'RI.unassigned', 'complete': False, 'entries': []},
            {'id': 'kind8-unknown', 'complete': False, 'entries': [{'behaviors': []}]},
            {'id': 'minimap-unknown', 'complete': False, 'entries': []}]
        r = self.report()
        self.assertEqual(len(r['unresolved_render_dispatch']), 3)

    def test_native_policy_or_observation_cannot_count_as_replacement(self):
        for change in ('policy', 'observation'):
            with self.subTest(change=change):
                original = copy.deepcopy(self.register)
                if change == 'policy':
                    self.register['behaviors'][0]['kind'] = 'native_policy'
                else:
                    self.register['evidence'][0]['kind'] = 'live_observation'
                with self.assertRaises(ValueError):
                    self.report()
                self.register = original

    def test_driver_fixture_does_not_promote_original_comparison(self):
        self.register['evidence'][0]['kind'] = 'native_integration'
        self.register['behaviors'][0]['kind'] = 'native_policy'
        self.register['behaviors'][0]['status']['comparison'] = 'none'
        for row in self.definition['surface_operations'] + self.definition['drawing_routes']:
            row['validation'][0]['domain'] = 'standalone_driver'
            row['replacement'] = []
        self.definition['world_takeovers'] = []
        r = self.report()
        self.assertEqual(r['surface_operations']['independent_validation_support']['count'], 1)
        self.assertEqual(r['surface_operations']['original_executable_comparison_support']['count'], 0)

    def test_unreviewed_validation_domain_refuses(self):
        self.definition['surface_operations'][0]['validation'][0]['domain'] = 'guessed'
        with self.assertRaisesRegex(ValueError, 'Unreviewed'):
            self.report()

    def test_changed_evidence_and_failed_assertions_refuse(self):
        (self.root / 'proof.json').write_text('{}')
        with self.assertRaisesRegex(ValueError, 'changed evidence'):
            self.report()
        self.register['evidence'][0]['sha256'] = hashlib.sha256(b'{}').hexdigest()
        with self.assertRaises(KeyError):
            self.report()

    def test_unknown_or_duplicate_admitted_entry_refuses(self):
        for entries in (['0x20'], ['0x10', '0x10']):
            self.definition['world_takeovers'][0]['admitted_entries'] = entries
            with self.assertRaisesRegex(ValueError, 'entry inventory'):
                self.report()

    def test_unlinked_proof_and_undocumented_gaps_refuse(self):
        self.register['behaviors'][0]['evidence'] = []
        with self.assertRaisesRegex(ValueError, 'not linked'):
            self.report()
        self.register['behaviors'][0]['evidence'] = ['proof']
        self.definition['surface_operations'][0]['remaining'] = ''
        with self.assertRaisesRegex(ValueError, 'remaining boundary'):
            self.report()

    def test_original_working_and_escaping_paths_never_read(self):
        for path in ('original/game.exe', 'working/capture.json', '../proof.json', '/tmp/proof.json'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                read(self.root, path)

    def test_reports_never_overwrite_or_write_originals(self):
        for paths in ((self.root / 'proof.json',), (self.root / 'new', self.root / 'new'),
                      (ROOT / 'original/new-rendering-report.json',), (ROOT / '.git/new-report.json',)):
            with self.subTest(paths=paths), self.assertRaises(ValueError):
                cli.output_paths(paths)

    def test_symlinked_original_and_git_roots_are_protected(self):
        checkout = self.root / 'checkout'
        checkout.mkdir()
        for name in ('original', '.git'):
            outside = self.root / ('external-' + name)
            outside.mkdir()
            (checkout / name).symlink_to(outside, target_is_directory=True)
            with patch.object(cli, 'ROOT', checkout):
                for target in (checkout / name / 'new.json', outside / 'new.json'):
                    with self.subTest(target=target), self.assertRaises(ValueError):
                        cli.output_paths((target,))
        self.assertFalse(any(self.root.rglob('new.json')))

    def test_repository_crosswalk_has_exact_denominators_and_linked_evidence(self):
        register = read(ROOT, 'research/runtime/coverage/register.json')
        matrix = read(ROOT, 'research/runtime/native-surface-operation-matrix.json')
        definition = read(ROOT, 'research/runtime/rendering-coverage-scope.json')
        behavior_ids = {b['id'] for b in register['behaviors']}
        self.assertEqual({r['id'] for r in definition['surface_operations']},
                         {o['id'] for o in matrix['operations'] if o['requirement'] == 'required'})
        self.assertEqual({r['id'] for r in definition['drawing_routes']},
                         {bid for bid in behavior_ids if bid.startswith('RI.')})
        by_id = {b['id']: b for b in register['behaviors']}
        for row in definition['surface_operations'] + definition['drawing_routes']:
            self.assertTrue(set(row['implementation']) <= behavior_ids)
            for p in row['validation'] + row['replacement']:
                self.assertIn(p['evidence'], by_id[p['behavior']]['evidence'])


if __name__ == '__main__':
    unittest.main()
