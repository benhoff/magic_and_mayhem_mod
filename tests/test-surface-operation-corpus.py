"""Operation-specific admission, provenance binding and failed replay accounting."""
import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import surface_fixture_corpus as corpus

spec = importlib.util.spec_from_file_location('surface_checker', ROOT / 'tools/check-surface-fixtures.py')
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


class OperationCorpus(unittest.TestCase):
    def setUp(self):
        self.catalog = corpus.read_json(corpus.DEFAULT)
        matrix = corpus.read_json(ROOT / 'research/runtime/native-surface-operation-matrix.json')
        self.required = {r['id'] for r in matrix['operations'] if r['requirement'] == 'required'}

    def test_independent_retained_artifacts_admit_all_required_families(self):
        pinned = corpus.validate_catalog(self.catalog)
        self.assertEqual(len(pinned), 26)
        represented = set().union(*(a['matrix_ids'] for a in corpus.ADAPTERS.values()))
        self.assertEqual(represented, self.required)
        self.assertEqual(len(self.required), 12)

    def test_operation_claims_cannot_be_swapped_or_relabelled(self):
        for update in ({'matrix_ids': ['SURF.restore']}, {'oracle': 'original_game_pixels'},
                       {'id': 'run-any-program'}, {'args': ['--recovery-only']},
                       {'scope': ''}):
            catalog = copy.deepcopy(self.catalog)
            catalog['cohorts'][0].update(update)
            with self.subTest(update=update), self.assertRaises(ValueError):
                corpus.validate_catalog(catalog)

    def test_missing_duplicate_or_unknown_cohort_and_no_boundaries_refuse(self):
        for catalog in (dict(self.catalog, cohorts=[]),
                        dict(self.catalog, cohorts=self.catalog['cohorts'] * 2),
                        dict(self.catalog, remaining_contracts=[])):
            with self.assertRaises(ValueError):
                corpus.validate_catalog(catalog)
        # A deliberately bounded subset is admitted, but cannot complete the matrix.
        subset = dict(self.catalog, cohorts=[self.catalog['cohorts'][1]])
        corpus.validate_catalog(subset)
        report = corpus.summarize(subset, [dict(id='formats', success=True, returncode=0)], self.required)
        self.assertFalse(report['required_fixture_cells_complete'])
        self.assertIn('SURF.restore', report['missing_required_matrix_ids'])

    def test_artifact_replacement_hash_corruption_and_omission_refuse(self):
        for mode in ('hash', 'omit', 'path', 'invalid_hash'):
            catalog = copy.deepcopy(self.catalog)
            artifacts = catalog['cohorts'][0]['artifacts']
            first = next(iter(artifacts))
            if mode == 'hash':
                artifacts[first] = '0' * 64
            elif mode == 'invalid_hash':
                artifacts[first] = 'x' * 64
            elif mode == 'omit':
                artifacts.pop(first)
            else:
                artifacts['../../original/game.exe'] = artifacts.pop(first)
            with self.subTest(mode=mode), self.assertRaises(ValueError):
                corpus.validate_catalog(catalog)

    def test_paths_cannot_escape_or_consume_originals(self):
        for name in ('../AGENTS.md', '/etc/passwd', 'original/game.exe', '.git/config',
                     'working/fixture.json', 'tools//check-surface-fixtures.py'):
            with self.subTest(name=name), self.assertRaises(ValueError):
                corpus.source_path(name)

    def result(self, key):
        adapter = corpus.ADAPTERS[key]
        pinned = corpus.validate_catalog(self.catalog)
        sources = {n: pinned[n] for n in adapter['artifacts']}
        checker_path = 'tools/' + adapter['checker']
        sources[checker_path] = corpus.sha(ROOT / checker_path)
        binaries = {n: 'a' * 64 for n in ('libmnm-renderer.a', adapter['binary'],
                                         'mnm-render-commands')}
        result = dict(schema=1, success=True, sources=sources, **adapter['metrics'])
        if key == 'clipping':
            result['binaries'] = {n: binaries[n] for n in
                                  ('mnm-render-commands', adapter['binary'])}
        else:
            result.update(binary_sha256=binaries[adapter['binary']],
                          library_sha256=binaries['libmnm-renderer.a'])
        if key == 'backend':
            result['combined'] = True
        if key == 'ownership':
            result['ownership_driver_comparison'] = True
        return result, pinned, binaries

    def test_adapter_results_require_actual_operation_counts_and_native_identities(self):
        for key, adapter in corpus.ADAPTERS.items():
            result, pinned, binaries = self.result(key)
            self.assertEqual(corpus.admit_result(key, result, pinned, binaries), result['sources'])
            metric = next(iter(adapter['metrics']))
            for value in (0, True, adapter['metrics'][metric] - 1):
                bad = dict(result, **{metric: value})
                with self.subTest(key=key, value=value), self.assertRaisesRegex(ValueError, 'Incomplete'):
                    corpus.admit_result(key, bad, pinned, binaries)
            with self.assertRaises(ValueError):
                corpus.admit_result(key, dict(result, success=False), pinned, binaries)
            with self.assertRaises(ValueError):
                corpus.admit_result(key, dict(result, sources={}), pinned, binaries)
            wrong = dict(binaries, **{adapter['binary']: 'b' * 64})
            with self.assertRaisesRegex(ValueError, 'identity'):
                corpus.admit_result(key, result, pinned, wrong)

    def test_guard_only_and_noncombined_execution_cannot_claim_original_outputs(self):
        for key, field in (('backend', 'combined'), ('ownership', 'ownership_driver_comparison')):
            result, pinned, binaries = self.result(key)
            result[field] = False
            with self.assertRaises(ValueError):
                corpus.admit_result(key, result, pinned, binaries)

    def test_failed_missing_and_duplicate_results_do_not_supply_coverage(self):
        rows = [dict(id=k, success=True, returncode=0) for k in corpus.ADAPTERS]
        report = corpus.summarize(self.catalog, rows, self.required)
        self.assertTrue(report['success'])
        self.assertTrue(report['required_fixture_cells_complete'])
        self.assertFalse(report['milestone_complete'])
        self.assertFalse(report['full_surface_acceptance_complete'])
        rows[2]['returncode'] = 1
        report = corpus.summarize(self.catalog, rows, self.required)
        self.assertFalse(report['success'])
        self.assertEqual(set(report['missing_required_matrix_ids']), corpus.ADAPTERS['ownership']['matrix_ids'])
        rows[2]['returncode'] = 0
        del rows[2]['success']
        self.assertFalse(corpus.summarize(self.catalog, rows, self.required)['success'])
        for incomplete in (rows[:-1], rows + [rows[0]]):
            with self.assertRaises(ValueError):
                corpus.summarize(self.catalog, incomplete, self.required)

    def test_no_cpu_fallback_or_missing_build(self):
        with self.assertRaisesRegex(ValueError, 'opengl'):
            checker.check(corpus.DEFAULT)
        with self.assertRaisesRegex(ValueError, '--build'):
            checker.check(corpus.DEFAULT, backend='opengl')

    def test_unsafe_or_changed_source_fails_stability(self):
        self.assertFalse(corpus.stable_sources({'original/game.exe': '0' * 64}))
        self.assertFalse(corpus.stable_sources({'tools/check-surface-fixtures.py': '0' * 64}))

    def test_zero_exit_without_child_report_is_retained_as_failure(self):
        with tempfile.TemporaryDirectory(dir=ROOT / 'working', prefix='operation-control-') as name:
            build = Path(name)
            for filename in ('libmnm-renderer.a', 'mnm-render-commands',
                             *(a['binary'] for a in corpus.ADAPTERS.values())):
                (build / filename).write_bytes(b'not executed: subprocess is mocked')
            child = subprocess.CompletedProcess([], 0)
            with patch.object(corpus.subprocess, 'run', return_value=child) as run:
                report = corpus.check(corpus.DEFAULT, build)
            self.assertEqual(run.call_count, 4)
            self.assertFalse(report['success'])
            self.assertEqual(report['covered_matrix_ids'], [])
            self.assertEqual(set(report['missing_required_matrix_ids']), self.required)
            self.assertTrue(all('error' in r for r in report['cases']))
            self.assertTrue((ROOT / report['artifacts'] / 'report.json').is_file())


if __name__ == '__main__':
    unittest.main()
