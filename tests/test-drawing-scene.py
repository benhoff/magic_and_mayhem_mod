#!/usr/bin/env python3
"""Portable scene identity, provenance, path and oracle-isolation guards."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text, shared_cache
import drawing_scene_corpus as corpus


class SceneCorpusTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads(evidence_text(ROOT / corpus.CATALOG))
        cls.register = json.loads(evidence_text(ROOT / 'research/runtime/coverage/register.json'))

    def test_admit_full_owned_startup(self):
        raw, assets, oracles = corpus.admit(ROOT, self.catalog, self.register)
        self.assertEqual(len(oracles), 6)
        self.assertEqual(len(assets), 16)
        self.assertTrue(raw.startswith(b'MNMPRO01'))
        self.assertEqual(self.catalog['boundary']['sequence'], 20458)
        self.assertEqual(sum(len(p) // 2 for _, p in oracles), 2880000)

    def test_declared_identity_cannot_be_relabelled(self):
        catalog = copy.deepcopy(self.catalog)
        catalog['boundary']['canvas'] += 1
        with self.assertRaises(ValueError):
            corpus.admit(ROOT, catalog, self.register)

    def test_catalog_is_bound_to_retention_receipt(self):
        catalog = copy.deepcopy(self.catalog)
        catalog['input']['raw_sha256'] = '0' * 64
        with self.assertRaises(ValueError):
            corpus.admit(ROOT, catalog, self.register)

    def test_historical_original_record_cannot_be_rewritten(self):
        register = copy.deepcopy(self.register)
        next(e for e in register['evidence'] if e['id'] == self.catalog['evidence'])['sha256'] = '0' * 64
        with self.assertRaises(ValueError):
            corpus.admit(ROOT, self.catalog, register)

    def test_assets_refuse_paths_and_case_aliases(self):
        for name in ('../oracle.565', '/absolute.jpg', 'C:\\oracle.bmp', 'a/../b', 'a//b', '.\\a'):
            with self.subTest(name=name), self.assertRaises(ValueError):
                corpus.asset_name(name)
        self.assertEqual(corpus.asset_name('Sprites\\Cursor.spr'), 'Sprites/Cursor.spr')

    def test_artifact_hash_before_decode(self):
        entry = copy.deepcopy(self.catalog['input'])
        entry['sha256'] = '0' * 64
        with self.assertRaises(ValueError):
            corpus.artifact(ROOT, entry)

    def test_oracle_is_full_canvas(self):
        c = self.catalog['checkpoints'][-1]
        raw = corpus.artifact(ROOT, c['oracle'])
        with self.assertRaises(ValueError):
            corpus.compare(raw[:-2], raw, c['width'], c['height'])
        changed = bytearray(raw)
        changed[-1] ^= 1
        result = corpus.compare(bytes(changed), raw, c['width'], c['height'])
        self.assertEqual(result['bounds'], [799, 599, 799, 599])
        self.assertEqual(result['mismatching_pixels'], 1)

    def test_renderer_gets_only_owned_sources(self):
        # Keep test output under a disposable root, with no installation or
        # working directory. Do not mock admission/provenance checks.
        import shutil
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            for relative in ('tests/fixtures/drawing-scene', 'research/runtime/coverage/register.json',
                             'research/runtime/native-world-producer-handoff-gpu-current-comparison-20261008.json'):
                source, target = ROOT / relative, root / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                if source.is_dir():
                    shutil.copytree(source, target)
                else:
                    target.write_bytes(evidence_bytes(source))
            binary = root / 'renderer'
            binary.write_bytes(b'test-binary')
            seen = []
            def execute(command, **kwargs):
                scene_path = Path(command[-2])
                declaration = json.loads(evidence_text(scene_path))
                native_input = scene_path.parent
                self.assertEqual(set(declaration), {'schema', 'boundary', 'assets', 'retain_checkpoints'})
                self.assertFalse(any('oracle' in p.name for p in native_input.rglob('*')))
                self.assertEqual(len(declaration['assets']), 16)
                self.assertTrue((native_input / 'assets/interface').is_dir())
                self.assertFalse((native_input / 'assets/Interface').exists())
                self.assertEqual(len(list((native_input / 'assets').rglob('*.spr'))), 4)
                source_raw=evidence_bytes(native_input / 'producers.bin')
                self.assertTrue(all(r[21]==0 for r,_ in corpus.load_tool('inspect-canvas-producers').decode(source_raw,False)[1] if r[2]==9))
                seen.append(command)
                output = Path(command[-1]);output.mkdir()
                checkpoints = []
                for c in self.catalog['checkpoints']:
                    # Fake renderer output comes from this test's fixture; the
                    # command/input isolation is the assertion under test.
                    pixels = corpus.artifact(ROOT, c['oracle'])
                    (output / f"checkpoint-{c['ordinal']:04}.565").write_bytes(pixels)
                    checkpoints.append({k: c[k] for k in ('ordinal', 'sequence', 'canvas', 'width', 'height')} | {'sha256': corpus.sha(pixels)})
                (output / 'scene.565').write_bytes(pixels)
                report = {'success': True, 'boundary': declaration['boundary'], 'remaining_surfaces': 0,
                          'original_pixels_used_as_native_inputs': False, 'original_oracles_read': False,
                          'native_gpu_presentation_ready': True, 'checkpoints': checkpoints, 'scene_sha256': corpus.sha(pixels)}
                original=corpus.input_bytes(ROOT,self.catalog)
                historical=json.loads(evidence_text(ROOT / 'research/runtime/native-world-producer-handoff-gpu-current-comparison-20261008.json'))
                results=corpus.original_raster_results(original,historical)
                result_data=''.join('\t'.join(map(str,row))+'\n' for row in results).encode()
                (output/'raster-results.tsv').write_bytes(result_data)
                report.update(original_raster_results_used_as_native_inputs=False,raster_result_count=len(results),
                              unknown_raster_results=0,raster_results_sha256=corpus.sha(result_data))
                (output / 'scene-report.json').write_text(json.dumps(report))
                from subprocess import CompletedProcess
                return CompletedProcess(command, 0, b'', b'')
            with shared_cache(ROOT), patch.object(corpus.subprocess, 'run', side_effect=execute):
                result = corpus.check(root=root, executable=binary)
            self.assertEqual(len(seen), 1)
            self.assertTrue(result['declared_canvas_complete'])
            self.assertFalse(result['draw03_complete'])
            self.assertFalse(result['live_replacement'])


if __name__ == '__main__':
    unittest.main()
