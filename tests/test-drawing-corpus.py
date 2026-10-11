#!/usr/bin/env python3
"""DRAW-02 admission, identity, complete comparison and oracle isolation checks."""
import copy
import gzip
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import drawing_corpus as corpus


class CorpusTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads((corpus.ROOT / corpus.CATALOG).read_text())
        cls.register = json.loads((corpus.ROOT / 'research/runtime/coverage/register.json').read_text())
        cls.cases = corpus.admit(corpus.ROOT, cls.catalog, cls.register)

    def test_portable_full_corpus(self):
        self.assertEqual(len(self.cases), 9)
        self.assertEqual(sum(c[1]['width'] * c[1]['height'] for c in self.cases), 4320000)
        self.assertEqual([c[1]['sequence'] for c in self.cases if c[0]['kind'] == 'world_canvas'], [1, 2, 3, 4])
        self.assertEqual(len(self.catalog['scenarios']), 14)

    def test_surface_after_removed_from_native_input(self):
        surface = corpus.load_tool('check-surface-fixtures')
        for c, identity, raw, original in self.cases[:5]:
            decoded = surface.replay.decode(raw)
            self.assertEqual(decoded['after'], bytes(len(original)))
            self.assertEqual(corpus.sha(raw), identity['native_input_sha256'])
            self.assertTrue(corpus.compare(surface.replay.replay(decoded), original,
                                           identity['width'], identity['height'])['matching'])

    def test_source_frames_pack_without_destination(self):
        for _, identity, raw, _ in self.cases[5:]:
            metadata, frames = corpus.world_inputs(raw)
            self.assertEqual(metadata['sequence'], identity['sequence'])
            for indexed, group in frames.items():
                if not group:
                    continue
                sprite = corpus.sprite_files(frames)[f'captured-{indexed}.spr']
                base = 24 + indexed * 768 + len(group) * 4
                self.assertEqual(sprite[base:], b''.join(frame for _, frame in sorted(group.items())))

    def test_input_and_frame_identity_required(self):
        for key in ('sequence', 'width', 'native_input_sha256'):
            catalog = copy.deepcopy(self.catalog)
            catalog['cases'][5]['identity'][key] = 0
            with self.assertRaisesRegex(ValueError, 'identity mismatch'):
                corpus.admit(corpus.ROOT, catalog, self.register)

    def test_independent_provenance_required(self):
        catalog = copy.deepcopy(self.catalog)
        catalog['cases'][5]['evidence'] = 'NR.world-frame.native-20261007'
        with self.assertRaisesRegex(ValueError, 'independent original'):
            corpus.admit(corpus.ROOT, catalog, self.register)
        catalog = copy.deepcopy(self.catalog)
        catalog['cases'][5]['oracle']['historical_path'] = catalog['cases'][5]['input']['historical_path']
        with self.assertRaisesRegex(ValueError, 'provenance mismatch'):
            corpus.admit(corpus.ROOT, catalog, self.register)

    def test_changed_artifacts_refused(self):
        for role in ('input', 'oracle'):
            catalog = copy.deepcopy(self.catalog)
            catalog['cases'][5][role]['sha256'] = '0' * 64
            with self.assertRaisesRegex(ValueError, 'hash mismatch'):
                corpus.admit(corpus.ROOT, catalog, self.register)

    def test_duplicate_path_and_denominator_guards(self):
        for mutation in ('duplicate', 'escape', 'scenario'):
            catalog = copy.deepcopy(self.catalog)
            if mutation == 'duplicate':
                catalog['cases'][1]['id'] = catalog['cases'][0]['id']
            elif mutation == 'escape':
                catalog['cases'][1]['id'] = '../../original/output'
            else:
                catalog['scenarios'].pop()
            with self.assertRaises(ValueError):
                corpus.admit(corpus.ROOT, catalog, self.register)

    def test_symlink_and_bounded_gzip(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'outside').symlink_to(corpus.ROOT)
            with self.assertRaisesRegex(ValueError, 'escapes'):
                corpus.local(root, 'outside/AGENTS.md')
            packed = gzip.compress(bytes(200), mtime=0)
            (root / 'bomb.gz').write_bytes(packed)
            entry = {'path': 'bomb.gz', 'encoding': 'gzip', 'sha256': corpus.sha(packed), 'raw_sha256': corpus.sha(bytes(200))}
            with patch.object(corpus, 'MAX_BYTES', 100):
                with self.assertRaisesRegex(ValueError, 'extent/hash'):
                    corpus.artifact(root, entry)

    def test_mismatch_borders_and_extent(self):
        expected = bytes(24)
        actual = bytearray(expected)
        actual[0] = 1
        actual[-1] = 2
        result = corpus.compare(actual, expected, 4, 3)
        self.assertEqual(result['mismatching_pixels'], 2)
        self.assertEqual(result['bounds'], [0, 0, 3, 2])
        self.assertEqual(result['examples'][-1]['original'], 0)
        with self.assertRaisesRegex(ValueError, 'entire'):
            corpus.compare(expected[:-2], expected, 4, 3)

    def test_world_process_receives_only_owned_inputs(self):
        _, identity, raw, expected = self.cases[5]
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp)
            def execute(command, **kwargs):
                self.assertFalse(any('oracle' in str(p) for p in command))
                self.assertEqual((output / 'inputs/world.bin').read_bytes(), raw)
                self.assertFalse(any(p.name.endswith('.565') for p in (output / 'inputs').rglob('*')))
                (output / 'native.565').write_bytes(expected)
                (output / 'native.json').write_text(json.dumps({'success': True, 'sequence': identity['sequence'],
                    'width': identity['width'], 'height': identity['height'], 'records': identity['draws'],
                    'remaining_surfaces': 0, 'pixel_sha256': corpus.sha(expected)}))
                import subprocess
                return subprocess.CompletedProcess(command, 0, b'', b'')
            with patch.object(corpus.subprocess, 'run', side_effect=execute):
                actual, _ = corpus.run_world(raw, output, Path('/fake/native'), False)
            self.assertEqual(actual, expected)

    def test_complete_gate_and_report_immutability(self):
        checker = corpus.load_tool('check-drawing-corpus')
        with tempfile.TemporaryDirectory() as tmp, patch.object(checker, 'ROOT', Path(tmp)):
            report = Path(tmp) / 'report.json'
            result = {'success': True, 'pixels_compared': 1, 'mismatches': 0,
                      'draw02_complete': False, 'artifacts': 'working/tests/mock'}
            argv = ['checker', '--surface-executable', '/fake/surface', '--world-executable', '/fake/world',
                    '--report', str(report), '--require-complete']
            with patch.object(sys, 'argv', argv), patch.object(checker, 'sources', return_value={}), \
                    patch.object(checker, 'check', return_value=result):
                self.assertEqual(checker.main(), 1)
            self.assertFalse(json.loads(report.read_text())['draw02_complete'])
            with self.assertRaisesRegex(ValueError, 'new'):
                checker.output_path(report)
            with self.assertRaises(ValueError):
                checker.output_path(Path(tmp) / 'original/forbidden-report.json')

    def test_unknown_modes_and_pointer_refuse(self):
        raw = bytearray(self.cases[5][2])
        struct.pack_into('<I', raw, 84, 6)
        with self.assertRaisesRegex(ValueError, 'Unsupported'):
            corpus.world_inputs(raw)
        raw = bytearray(self.cases[5][2])
        raw[80 + 64 + 28] = 1
        with self.assertRaisesRegex(ValueError, 'pointer'):
            corpus.world_inputs(raw)


if __name__ == '__main__':
    unittest.main()
