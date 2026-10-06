#!/usr/bin/env python3
"""Strict scene admission; optional installed pixel/reproducibility integration."""
import copy
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


loader = module('bounded_scene', ROOT/'tools/load-bounded-scene.py')
SPEC = ROOT/'tests/fixtures/forest-redcap-scene.json'


class Admission(unittest.TestCase):
    def setUp(self):
        self.spec = json.loads(SPEC.read_text())

    def test_valid(self):
        loader.validate(self.spec)

    def test_unsupported(self):
        cases = [('schema', 2), ('policy', 'full-world'), ('crop', [0, 0, 17, 8]),
                 ('creature', {'type': 11, 'start': [1, 1, 4], 'goal': [2, 6, 4]}),
                 ('presentation', {'view': 4, 'ticks': 0, 'frames': 1})]
        for key, value in cases:
            with self.subTest(key=key):
                changed = copy.deepcopy(self.spec);changed[key] = value
                with self.assertRaises(ValueError):loader.validate(changed)
        for section in (None, 'creature', 'resources', 'presentation'):
            changed = copy.deepcopy(self.spec)
            (changed if section is None else changed[section])['unsupported'] = True
            with self.assertRaises(ValueError):loader.validate(changed)
        self.spec['crop'][0] = True
        with self.assertRaises(ValueError):loader.validate(self.spec)

    def test_duplicate_and_oversize(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp)/'scene.json'
            path.write_text('{"schema":1,"schema":1}')
            with self.assertRaises(ValueError):loader.read_spec(path)
            path.write_bytes(b' '*65537)
            with self.assertRaises(ValueError):loader.read_spec(path)

    def test_resources(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for request in loader.REQUESTS:
                payload = request.encode()
                loader.write_new(root/request, payload)
                self.spec['resources'][request] = loader.sha(payload)
            self.assertEqual(len(loader.admit(root, self.spec)), 6)
            # Same installed request with a case variant resolves uniquely.
            path = root/loader.REQUESTS[-1];path.rename(path.with_name('RedCap.spr'))
            loader.admit(root, self.spec)
            loader.write_new(path, b'ambiguous')
            with self.assertRaisesRegex(ValueError, 'ambiguous'):loader.admit(root, self.spec)
            path.unlink();path = path.with_name('RedCap.spr');path.write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'hash mismatch'):loader.admit(root, self.spec)
            path.unlink();path.symlink_to(root/loader.REQUESTS[-2])
            with self.assertRaisesRegex(ValueError, 'symlink'):loader.admit(root, self.spec)


def installed(build, output):
    os.environ.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    root = ROOT/'working/game-clean'
    for name in ('first', 'repeat'):
        loader.load(SPEC, root, build, output/name)
    first, repeat = output/'first', output/'repeat'
    for path in first.iterdir():
        if path.suffix in ('.565', '.png', '.geometry', '.frozen'):
            assert path.read_bytes() == (repeat/path.name).read_bytes(), path
    frames = json.loads((first/'frame.json').read_text())['frames']
    assert frames == json.loads((repeat/'frame.json').read_text())['frames']
    goal = json.loads(SPEC.read_text())['creature']['goal']
    geometry = (first/'world.geometry').read_bytes()
    width, height, _ = struct.unpack_from('<3I', geometry, 4)
    x, y, z = goal
    definition = struct.unpack_from('<H', geometry, 76+12*((z*height+y)*width+x))[0]
    ttd = (first/'resources'/loader.REQUESTS[1]).read_bytes()
    offset = struct.unpack_from('<i', ttd, 16+356*definition+0x94)[0] if definition else 0
    assert frames[-1]['actors'][0]['fine'] == [x*32, y*32, z*16+offset], frames[-1]
    pixels = module('scene_pixels', ROOT/'tools/test-terrain-preview.py')
    terrain = {'sprite': (first/'resources'/loader.REQUESTS[2]).read_bytes(), 'frames': {}}
    creature = {'sprite': (first/'resources'/loader.REQUESTS[5]).read_bytes(), 'frames': {}}
    for i, frame in enumerate(frames):
        expected = [0x2124]*(512*256)
        for draw in frame['queue']:
            ox, oy, opaque = pixels.frame(creature if draw['creature'] else terrain, draw['frame'])
            for x, y, value in opaque:
                x += draw['x']-ox;y += draw['y']-oy
                if 0 <= x < 512 and 0 <= y < 256:expected[y*512+x] = value
        assert (first/f'frame-{i:03}.565').read_bytes() == struct.pack('<'+'H'*len(expected), *expected)
    changed = json.loads(SPEC.read_text());changed['resources'][loader.REQUESTS[2]] = '0'*64
    path = output/'mismatch.json';path.write_text(json.dumps(changed))
    try:
        loader.load(path, root, build, output/'refused')
    except ValueError as error:
        assert 'hash mismatch' in str(error)
    else:raise AssertionError('Changed terrain sprite accepted')
    assert not (output/'refused').exists()
    sources = [ROOT/'tools/load-bounded-scene.py', Path(__file__).resolve(), SPEC]
    report = {'success': True, 'live_validated': False, 'pixel_comparisons': len(frames),
              'repeat_pixels_and_geometry': True, 'arrival': True, 'hash_mismatch_refused_before_output': True,
              'source_sha256': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},
              'scope': 'Strict specification admission and installed Forest/Redcap ordinary scene; independent SPR composition, repeated native outputs and arrival. No original world equivalence.',
              'result_sha256': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                                for p in (first/'result.json', repeat/'result.json')}}
    loader.write_new(output/'report.json', (json.dumps(report, indent=2)+'\n').encode())
    print(output/'report.json')


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--installed':
        if len(sys.argv) != 4:raise ValueError('Usage: --installed BUILD NEW_OUTPUT_DIRECTORY')
        installed(Path(sys.argv[2]), Path(sys.argv[3]))
    else:
        unittest.main()
