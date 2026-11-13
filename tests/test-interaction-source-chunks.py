#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import struct
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from broader_interaction_source_chunks import contexts_from_records, bind_assessor
spec = importlib.util.spec_from_file_location('original_camera_matrix', ROOT / 'tools/close-rendering-camera-matrix.py')
CAMERA = importlib.util.module_from_spec(spec)
spec.loader.exec_module(CAMERA)


def row(kind, sequence=1, tag=None):
    f = [0] * 23
    f[1], f[2], f[3] = sequence, kind, 7
    if kind == 24:
        m = [0] * 37
        m[0], m[3], m[4], m[9], m[17] = tag, 10, 11, 0, 1
        return tuple(f), struct.pack('<37I', *m)
    if kind == 22: f[10], f[11], f[16] = 10, 11, 0
    if kind == 9: f[14], f[15], f[17], f[22] = 1, 1, 1, 0x1234
    return tuple(f), b''


def packet():
    return [row(24, 1, 2), row(24, 2, 0), row(24, 3, 1), row(22, 4), row(9, 5)]


class Chunks(unittest.TestCase):
    def setUp(self): self.data = {b'camera': packet(), b'empty': []}
    def decode(self, stream, complete=False): return {}, self.data[stream]
    def parse(self, entries): return contexts_from_records(entries, self.decode, CAMERA.context_many_cells)
    def world(self, g=1, stream=b'camera'): return g, 0, stream, b'x' * 64
    def chunk(self, c=1, g=1, stream=b'empty'): return g, c, stream, b''

    def test_complete_camera_chunk_is_validated_but_never_World_credit(self):
        contexts, chunks, evidence = self.parse([self.chunk(stream=b'camera'), self.world()])
        self.assertEqual((len(contexts), chunks), (1, 1))
        self.assertTrue(evidence[0]['complete_camera_packet'])
        self.assertFalse(evidence[0]['credited_as_World'])
        self.assertFalse(evidence[0]['credited_as_action_witness'])
        self.assertEqual(contexts[1]['camera']['view'], 0)

    def test_startup_chunk_preserves_old_behavior(self):
        contexts, chunks, evidence = self.parse([self.chunk(), self.world()])
        self.assertEqual(len(contexts), 1)
        self.assertFalse(evidence[0]['complete_camera_packet'])

    def test_partial_camera_chunk_refuses(self):
        self.data[b'partial'] = packet()[:-3] + [row(9, 5)]
        with self.assertRaises(ValueError): self.parse([self.chunk(stream=b'partial'), self.world()])

    def test_wrong_camera_identity_still_refuses(self):
        values = packet(); fields, body = values[1]; f = list(fields); f[3] = 8; values[1] = tuple(f), body
        self.data[b'wrong'] = values
        with self.assertRaisesRegex(ValueError, 'source identity'): self.parse([self.chunk(stream=b'wrong'), self.world()])

    def test_World_operation_in_chunk_refuses(self):
        self.data[b'world-op'] = packet() + [row(11)]
        with self.assertRaisesRegex(ValueError, 'World/failure'): self.parse([self.chunk(stream=b'world-op'), self.world()])

    def test_failure_operation_in_chunk_refuses(self):
        self.data[b'failure'] = [row(27)]
        with self.assertRaisesRegex(ValueError, 'World/failure'): self.parse([self.chunk(stream=b'failure'), self.world()])

    def test_missing_generation_refuses(self):
        with self.assertRaisesRegex(ValueError, 'generation'): self.parse([self.chunk(), self.world(2)])

    def test_duplicate_generation_refuses(self):
        with self.assertRaisesRegex(ValueError, 'generation'): self.parse([self.chunk(), self.world(), self.world()])

    def test_reordered_chunk_refuses(self):
        with self.assertRaisesRegex(ValueError, 'chunk identity'): self.parse([self.chunk(c=2), self.world()])

    def test_chunk_for_wrong_next_generation_refuses(self):
        with self.assertRaisesRegex(ValueError, 'generation'): self.parse([self.chunk(g=2), self.world()])

    def test_chunk_with_request_refuses(self):
        with self.assertRaisesRegex(ValueError, 'chunk identity'): self.parse([(1, 1, b'empty', b'request'), self.world()])

    def test_empty_World_request_refuses(self):
        with self.assertRaisesRegex(ValueError, 'request extent'): self.parse([self.chunk(), (1, 0, b'camera', b'')])

    def test_missing_chunks_refuses(self):
        with self.assertRaisesRegex(ValueError, 'prefix/history'): self.parse([self.world()])

    def test_terrain_without_camera_refuses(self):
        self.data[b'terrain'] = [row(22)]
        with self.assertRaisesRegex(ValueError, 'Terrain without'): self.parse([self.chunk(stream=b'terrain'), self.world()])


    def test_semantic_function_code_and_original_globals_are_unchanged(self):
        def original(x): return source_contexts(x) + 1
        before = dict(original.__globals__)
        bound = bind_assessor(original, lambda value: value * 2)
        self.assertIs(bound.__code__, original.__code__)
        self.assertEqual(bound(4), 9)
        self.assertEqual(before, original.__globals__)

    def test_existing_semantic_guard_still_refuses(self):
        def original(x):
            if x < 0: raise ValueError('existing semantic guard')
            return source_contexts(x)
        bound = bind_assessor(original, lambda value: value)
        with self.assertRaisesRegex(ValueError, 'existing semantic guard'): bound(-1)


if __name__ == '__main__': unittest.main()
