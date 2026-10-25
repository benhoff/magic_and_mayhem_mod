import copy
import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from critical_rendering_acceptance import PROFILES, assess
from coverage_evidence import evidence_digest, identity, read_json, write_report

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('acceptance_cli', ROOT/'tools/check-critical-rendering-acceptance.py')
cli = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cli)


def fixture():
    reports = {}
    for backend in ('software', 'hardware'):
        reports[backend] = {}
        for name, count in PROFILES.items():
            rows = [dict(ordinal=n, gpu=dict(pixels=480000, mismatching_pixels=0),
                         physical=dict(pixels=480000, mismatching_pixels=0))
                    for n in range(1, count+1)]
            if name == 'legacy_storage':
                for plane in ('gpu', 'physical'):
                    rows[169][plane]['mismatching_pixels'] = 126
            if name == 'corrected_storage':
                for row in rows:
                    row.update(source=dict(mismatching_pixels=0), destination=dict(
                        native_destination_mismatching_words=0,
                        source_destination_mismatching_words=0,
                        destination_physical_mismatching_pixels=0))
            reports[backend][name] = dict(
                sources_stable=True, original_reference_admitted=True,
                original_pixels_used_as_native_inputs=False,
                original_oracles_read_by_native=False, new_original_execution=False,
                channel_tolerance=1, alpha_compared=False, frames=count,
                physical_generation_frames=count, painted_before_swap_frames=count,
                hardware_requested=backend == 'hardware',
                renderer_identities=['NVIDIA', 'NVIDIA'] if backend == 'hardware'
                                    else ['llvmpipe', 'llvmpipe'],
                hardware_admission=dict(verified=backend == 'hardware'),
                comparisons=rows, mismatches=252 if name == 'legacy_storage' else 0,
                pixels_compared=960000*count, witness_mismatches=0,
                success=name != 'legacy_storage',
                outcome='mismatch' if name == 'legacy_storage' else 'matching',
                reference_profile=dict(name='stale_upload_crop', policies=dict(
                    egl_correct_stale_upload_policy=True)),
                witness_frames=count if name == 'corrected_storage' else 0)
    return reports


class Acceptance(unittest.TestCase):
    def setUp(self):
        self.reports = fixture()

    def test_both_complete_correct_output_profiles_pass_without_legacy_or_global_credit(self):
        before = copy.deepcopy(self.reports)
        result = assess(self.reports)
        self.assertTrue(result['success'])
        self.assertFalse(result['whole_renderer_accepted'])
        self.assertFalse(result['all_critical_behaviors_accepted'])
        self.assertFalse(result['legacy_diagnostics']['hardware']['blocking'])
        self.assertEqual(self.reports, before)

    def test_both_backends_and_every_profile_are_required(self):
        for backend in ('software', 'hardware'):
            changed = copy.deepcopy(self.reports); del changed[backend]
            with self.subTest(backend=backend), self.assertRaises(ValueError):assess(changed)
            changed = copy.deepcopy(self.reports); del changed[backend]['quick_battle']
            with self.assertRaises(ValueError):assess(changed)

    def test_a_new_correct_output_failure_remains_blocking(self):
        for backend in ('software', 'hardware'):
            changed = copy.deepcopy(self.reports); row = changed[backend]['corrected_storage']
            row['comparisons'][0]['physical']['mismatching_pixels'] = 1
            row.update(mismatches=1, success=False, outcome='mismatch')
            with self.subTest(backend=backend), self.assertRaises(ValueError):assess(changed)

    def test_legacy_cannot_hide_new_failures_or_move_the_bad_ordinal(self):
        for change in ('extra', 'move', 'pass'):
            changed = copy.deepcopy(self.reports); row = changed['software']['legacy_storage']
            if change == 'extra':row['comparisons'][0]['gpu']['mismatching_pixels'] = 1; row['mismatches'] += 1
            if change == 'move':row['comparisons'][168]['gpu']['mismatching_pixels'] = 126; row['comparisons'][169]['gpu']['mismatching_pixels'] = 0
            if change == 'pass':row['success'] = True; row['outcome'] = 'matching'
            with self.subTest(change=change), self.assertRaises(ValueError):assess(changed)

    def test_summary_cannot_override_frame_mismatches_or_denominator(self):
        for key, value in [('mismatches', 0), ('pixels_compared', 1), ('frames', 237),
                           ('physical_generation_frames', 237), ('channel_tolerance', 2)]:
            changed = copy.deepcopy(self.reports); changed['software']['legacy_storage'][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):assess(changed)

    def test_missing_paint_receipts_or_oracle_inputs_never_supply_acceptance(self):
        for key, value in [('painted_before_swap_frames', 0), ('original_reference_admitted', False),
                           ('original_pixels_used_as_native_inputs', True),
                           ('original_oracles_read_by_native', True), ('sources_stable', False)]:
            changed = copy.deepcopy(self.reports); changed['hardware']['corrected_storage'][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):assess(changed)

    def test_hardware_fallback_and_missing_second_driver_refuse(self):
        for drivers in (['NVIDIA', 'llvmpipe'], ['NVIDIA'], ['NVIDIA', 'NVIDIA software']):
            changed = copy.deepcopy(self.reports); changed['hardware']['corrected_storage']['renderer_identities'] = drivers
            with self.subTest(drivers=drivers), self.assertRaises(ValueError):assess(changed)

    def test_corrected_profile_and_all_independent_witnesses_are_required(self):
        for change in ('profile', 'policy', 'count', 'source', 'destination'):
            changed = copy.deepcopy(self.reports); row = changed['software']['corrected_storage']
            if change == 'profile':row['reference_profile']['name'] = 'unmodified_upload'
            if change == 'policy':row['reference_profile']['policies']['egl_correct_stale_upload_policy'] = False
            if change == 'count':row['witness_frames'] -= 1
            if change == 'source':row['comparisons'][0]['source']['mismatching_pixels'] = 1
            if change == 'destination':row['comparisons'][0]['destination']['native_destination_mismatching_words'] = 1
            with self.subTest(change=change), self.assertRaises(ValueError):assess(changed)

    def test_missing_duplicated_and_reordered_frames_refuse(self):
        for change in ('missing', 'duplicate', 'reorder'):
            changed = copy.deepcopy(self.reports); rows = changed['software']['quick_battle']['comparisons']
            if change == 'missing':rows.pop()
            if change == 'duplicate':rows[0]['ordinal'] = 2
            if change == 'reorder':rows[0], rows[1] = rows[1], rows[0]
            with self.subTest(change=change), self.assertRaises(ValueError):assess(changed)

    def cli_inputs(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        root = Path(directory.name)
        (root/'.evidence-store.json').write_text('{"schema":1,"archives":[]}\n')
        for name in ['tools/critical_rendering_acceptance.py', 'tools/check-critical-rendering-acceptance.py',
                     'tools/native_render_driver.py', 'tools/coverage_evidence.py',
                     'tools/rendering_session_contracts.py', 'tests/test-critical-rendering-acceptance.py']:
            target = root/name; target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT/name, target)
        source = root/'native.cpp'; source.write_text('fixed execution source\n')
        digest = hashlib.sha256(source.read_bytes()).hexdigest()
        component = dict(behavior='component', contract_sha256='a'*64)
        policy = dict(behavior='policy', contract_sha256='b'*64)
        bundles = {}
        for backend, profiles in self.reports.items():
            bundle = dict(sources_stable=True, profiles={})
            for name, value in profiles.items():
                record = dict(value, sources={'native.cpp':digest}, claims=[component])
                child = root/'research/runtime'/f'{backend}-{name}.json'
                write_report(root, child, json.dumps(record).encode())
                bundle['profiles'][name] = dict(record=child.relative_to(root).as_posix(),
                                                sha256=evidence_digest(child, root=root))
            path = root/'research/runtime'/f'{backend}.json'
            write_report(root, path, json.dumps(bundle).encode()); bundles[backend] = path
        claims = root/'working/claims.json'; claims.parent.mkdir(exist_ok=True)
        claims.write_text(json.dumps(dict(sources={'native.cpp':digest}, claims=[policy])))
        output = root/'research/runtime/result.json'
        argv = ['--policy', 'critical-rendering-correct-output-v1', '--software-report', str(bundles['software']),
                '--hardware-report', str(bundles['hardware']), '--claims', str(claims), '--report', str(output)]
        return root, source, policy, output, argv

    def test_sqlite_cohorts_bind_only_the_prospective_acceptance_policy(self):
        root, _, policy, output, argv = self.cli_inputs()
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(cli.main(argv, root=root), 0)
        self.assertIsNotNone(identity(output.read_bytes()))
        result = read_json(output, root=root)
        self.assertEqual(result['claims'], [policy])
        self.assertTrue(result['inputs_stable'])
        self.assertFalse(result['whole_renderer_accepted'])
        self.assertEqual(result['legacy_diagnostics']['software']['mismatches'], 252)

    def test_changed_prospective_source_cannot_gain_acceptance(self):
        root, source, _, output, argv = self.cli_inputs()
        source.write_text('different source after declaration\n')
        with self.assertRaisesRegex(ValueError, 'Prospective acceptance source changed'):
            cli.main(argv, root=root)
        self.assertFalse(output.exists())


if __name__ == '__main__':unittest.main()
