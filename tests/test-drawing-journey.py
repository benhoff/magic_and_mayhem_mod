#!/usr/bin/env python3
"""Multi-frame retained-history admission and native-command separation."""
import copy
import json
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import drawing_scene_corpus as corpus

CATALOG = 'tests/fixtures/drawing-journey/scene.json'
RETENTION = 'NR.drawing-journey.retention-20261010'


class JourneyTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads((ROOT / CATALOG).read_text())
        cls.register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())

    def admit_structural(self, catalog):
        # Independently pinned original evidence remains real. Substitute only
        # the static receipt to exercise structural guards beneath authenticity.
        from rendering_coverage import evidence_record
        def evidence(root, entry, builds):
            return catalog if entry['id'] == RETENTION else evidence_record(root, entry, builds)
        with patch('rendering_coverage.evidence_record', side_effect=evidence):
            return corpus.admit(ROOT, catalog, self.register, RETENTION)

    def test_full_contiguous_history(self):
        raw, assets, oracles = corpus.admit(ROOT, self.catalog, self.register, RETENTION)
        self.assertGreater(len(raw), 32 * 1024 * 1024)
        self.assertEqual(len(assets), 16)
        self.assertEqual(len(oracles), 17)
        self.assertEqual(self.catalog['frames'][0]['queues'], 0)
        self.assertEqual(self.catalog['frames'][-1]['queues'], 15)
        self.assertEqual([f['queues'] for f in self.catalog['frames'][2:]], list(range(1, 16)))

    def test_chunk_order_and_concatenation_hash(self):
        catalog = copy.deepcopy(self.catalog)
        catalog['input']['chunks'].reverse()
        with self.assertRaisesRegex(ValueError, 'concatenation'):
            self.admit_structural(catalog)

    def test_frame_order_and_queue_identity(self):
        for change in ('duplicate', 'queue', 'canvas', 'terminal', 'oracle'):
            with self.subTest(change=change):
                c = copy.deepcopy(self.catalog)
                if change == 'duplicate':
                    c['frames'][2] = c['frames'][1]
                elif change == 'queue':
                    c['frames'][2]['queues'] += 1
                elif change == 'canvas':
                    c['frames'][2]['canvas'] += 1
                elif change == 'terminal':
                    c['frames'].pop()
                else:
                    c['checkpoints'].pop(2)
                with self.assertRaises(ValueError):
                    self.admit_structural(c)

    def test_native_inputs_exclude_independent_oracles(self):
        import shutil
        import tempfile
        from subprocess import CompletedProcess
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            for p in ('tests/fixtures/drawing-journey', 'research/runtime/coverage/register.json',
                      'research/runtime/native-world-producer-handoff-gpu-current-comparison-20261008.json'):
                source, target = ROOT / p, root / p
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copytree(source, target) if source.is_dir() else shutil.copyfile(source, target)
            binary = root / 'renderer';binary.write_bytes(b'test-command-isolation-only')
            def execute(command, **kwargs):
                declaration = json.loads(Path(command[-2]).read_text())
                inputs = Path(command[-2]).parent
                self.assertEqual(set(declaration), {'schema', 'boundary', 'assets', 'retain_checkpoints', 'frames'})
                self.assertFalse(any('oracle' in p.name for p in inputs.rglob('*')))
                self.assertEqual(declaration['frames'], self.catalog['frames'])
                source_raw=(inputs/'producers.bin').read_bytes()
                self.assertTrue(all(r[21]==0 for r,_ in corpus.load_tool('inspect-canvas-producers').decode(source_raw,False)[1] if r[2]==9))
                output = Path(command[-1]);output.mkdir()
                checkpoints = []
                by_sequence = {}
                for c in self.catalog['checkpoints']:
                    data = corpus.artifact(ROOT, c['oracle'])
                    # These simulated outputs test argument separation only.
                    (output / f"checkpoint-{c['ordinal']:04}.565").write_bytes(data)
                    checkpoints.append({k: c[k] for k in ('ordinal', 'sequence', 'canvas', 'width', 'height')} | {'sha256': corpus.sha(data)})
                    by_sequence[c['sequence']] = data
                frames = []
                for i, frame in enumerate(declaration['frames'], 1):
                    data = by_sequence[frame['sequence']]
                    (output / f'frame-{i:04}.565').write_bytes(data)
                    frames.append(frame | {'ordinal': i, 'sha256': corpus.sha(data)})
                (output / 'scene.565').write_bytes(data)
                report = dict(success=True, boundary=declaration['boundary'], remaining_surfaces=0,
                    original_pixels_used_as_native_inputs=False, original_oracles_read=False,
                    native_gpu_presentation_ready=True, checkpoints=checkpoints, frames=frames,
                    frames_published=len(frames), scene_sha256=corpus.sha(data))
                original=corpus.input_bytes(ROOT,self.catalog)
                historical=json.loads((ROOT / 'research/runtime/native-world-producer-handoff-gpu-current-comparison-20261008.json').read_text())
                results=corpus.original_raster_results(original,historical)
                result_data=''.join('\t'.join(map(str,row))+'\n' for row in results).encode()
                (output/'raster-results.tsv').write_bytes(result_data)
                report.update(original_raster_results_used_as_native_inputs=False,raster_result_count=len(results),
                              unknown_raster_results=0,raster_results_sha256=corpus.sha(result_data))
                (output / 'scene-report.json').write_text(json.dumps(report))
                return CompletedProcess(command, 0, b'', b'')
            with patch.object(corpus.subprocess, 'run', side_effect=execute):
                result = corpus.check(root=root, catalog_path=CATALOG, executable=binary, retention=RETENTION)
            self.assertEqual(len(result['frame_comparisons']), 17)
            self.assertTrue(result['declared_canvas_complete'])
            self.assertFalse(result['draw03_complete'])


if __name__ == '__main__':
    unittest.main()
