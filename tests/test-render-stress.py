#!/usr/bin/env python3
"""Structural authoring controls and optional real native stress replay checks."""
import argparse
from contextlib import closing
import importlib.util
import json
import os
from pathlib import Path
import sqlite3
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from native_world_replay import create_case, validate_case, asset_manifest, open_case, event_header
from resident_capture import PrefixDecoder
spec = importlib.util.spec_from_file_location('render_stress', ROOT / 'tools/render-stress.py')
stress = importlib.util.module_from_spec(spec); spec.loader.exec_module(stress)
OPTIONS = None


def fixture(generation=1, particles=True, create=True):
    header = bytearray(64); header[:8] = b'MNMPRO03'
    struct.pack_into('<5I', header, 8, 3, 64, 0x40209ca7, 131072, 32)
    records = []
    def add(op, fields=None, payload=b''):
        r = [0] * 24; r[:4] = [96 + len(payload), len(records) + 1, op, 7]
        r[5:8] = [64, 48, 64]; r[18] = len(payload)
        for k, v in (fields or {}).items():
            r[k] = v & 0xffffffff
        records.append((r, payload)); return r[1]
    if create:
        add(1)
    add(5, {12: 64, 13: 48, 14: 0x1234})
    add(11, {14: 1})
    frame = bytearray(53)
    for at, value in [(0, 53), (4, 3), (8, 1), (12, 1), (16, 0xffffffff), (40, 48), (44, 51)]:
        struct.pack_into('<I', frame, at, value)
    frame[48:53] = b'\0\2\1\0\1'
    for index in range(2):
        palette = b''.join(struct.pack('<H', (i * 1733 + 0x321 + index * 0x0421) & 0xffff) for i in range(256))
        definition = add(25, {14: 9, 17: 1, 19: 53, 20: 512}, bytes(frame) + palette)
        if particles:
            add(9, {4: definition, 8: 10 + generation + index * 3, 9: 12 + generation,
                    12: 64, 13: 48, 14: 8, 15: 13, 17: 1, 19: 53, 20: 512})
    # An ordinary indexed copy and a blend must each stay single/order barriers.
    add(9, {4: definition, 8: 9, 9: 10, 12: 64, 13: 48, 14: 0, 15: 2, 17: 1, 19: 53, 20: 512})
    add(26, {8: 7, 9: 8, 12: 64, 13: 48, 15: 2, 19: 2, 20: 2}, b'\xe0\7' * 4)
    add(10, {14: 1, 15: 4}); add(12, {14: 1, 19: 3 if particles else 1})
    return bytes(header) + b''.join(struct.pack('<24I', *r) + p for r, p in records), bytes(frame)


def unpack(source):
    at = 64; rows = []
    while at < len(source):
        r = list(struct.unpack_from('<24I', source, at)); rows.append((r, source[at + 96:at + r[0]])); at += r[0]
    return rows


def asset(root):
    _, frame = fixture(); data = bytearray(24 + 768 + 4)
    for at, value in [(0, 0x00525053), (4, len(data) + len(frame)), (8, 4), (12, 1), (16, 1)]:
        struct.pack_into('<I', data, at, value)
    (root / 'particle.spr').write_bytes(data + frame)


class DataTests(unittest.TestCase):
    def test_amplifies_only_particles_and_repairs_later_definitions(self):
        source, _ = fixture(); result, counts = stress.amplify_source(source, b'', 2, 4, 24)
        rows = unpack(result); self.assertEqual(counts['derived_particles'], 8)
        self.assertEqual([r[1] for r, _ in rows], list(range(1, len(rows) + 1)))
        definitions = {r[1]: p for r, p in rows if r[2] == 25}
        rasters = [r for r, _ in rows if r[2] == 9]
        self.assertEqual(len(rasters), 9)
        self.assertEqual(sum(r[14] == 0 for r in rasters), 1)
        self.assertEqual(sum(r[2] == 26 for r, _ in rows), 1)
        for r in rasters:
            self.assertIn(r[4], definitions); self.assertEqual(r[10:14], [0, 0, 64, 48])
        self.assertNotEqual(rasters[4][4], unpack(source)[-5][0][4])
        self.assertEqual([stress.signed(r[8]) for r in rasters[:4]], [11, 35, -13, 11])

    def test_control_factor_preserves_existing_record_bytes(self):
        source, _ = fixture(); result, counts = stress.amplify_source(source, b'', 2, 1, 24)
        before, after = unpack(source), unpack(result)
        self.assertEqual(len(before), len(after)); self.assertEqual(counts['derived_particles'], 2)
        for (a, ap), (b, bp) in zip(before, after):
            if a[2] == 12:
                a[15] = b[15]
            self.assertEqual((a, ap), (b, bp))

    def test_unreturned_source_gets_authored_closure(self):
        source, _ = fixture(); rows = unpack(source)[:-2]
        source = source[:64] + b''.join(struct.pack('<24I', *r) + p for r, p in rows)
        request = b'MNMWBQ03' + b'\0' * 56
        result, counts = stress.amplify_source(source, request, 1, 2, 24)
        rows = unpack(result); self.assertEqual([r[2] for r, _ in rows[-2:]], [10, 12])
        self.assertEqual(rows[-1][0][19], counts['raster_count'])
        self.assertEqual(rows[-1][0][14], 1)

    def test_bad_bounds_shapes_and_missing_sources_refuse(self):
        source, _ = fixture()
        for copies, spread, kind in [(3, 24, 2), (4, 129, 2), (4, -1, 2), (4, 24, 3)]:
            with self.assertRaises(ValueError):
                stress.amplify_source(source, b'', kind, copies, spread)
        with self.assertRaises(ValueError):
            stress.amplify_source(source[:-1], b'', 2, 4, 24)
        at = 64
        for r, payload in unpack(source):
            if r[2] == 9:
                break
            at += r[0]
        for field, value in [(4, 999), (8, 0x7fffffff), (2, 13)]:
            damaged = bytearray(source)
            struct.pack_into('<I', damaged, at + field * 4, value)
            with self.assertRaises(ValueError):
                stress.amplify_source(bytes(damaged), b'', 2, 4, 24)

    def test_raster_budget_refuses_without_relaxing_native_limit(self):
        source, _ = fixture(); rows = unpack(source)
        many = rows[:4] + [rows[4]] * 5000 + rows[-2:]
        packed = []
        for sequence, (fields, payload) in enumerate(many, 1):
            r = fields[:]; r[1] = sequence
            if r[2] == 12:
                r[19] = r[15] = 5000
            packed.append(struct.pack('<24I', *r) + payload)
        raw = source[:64] + b''.join(packed)
        with self.assertRaisesRegex(ValueError, 'raster count'):
            stress.amplify_source(raw, b'', 2, 4, 24)

    def test_history_parent_identity_and_no_particle_refusal(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); parent = root / 'parent.sqlite3'; child = root / 'child.sqlite3'
            source, _ = fixture(create=False)
            complete, _ = fixture(); history = complete[:64] + struct.pack('<24I', *unpack(complete)[0][0])
            create_case(parent, [(1, 1, history, b'', 0), (1, 0, source, b'', 2)], {})
            old_hash = stress.sha_file(parent); meta, _ = stress.derive(parent, child, 1, 1, 2, 24, {})
            self.assertEqual(stress.sha_file(parent), old_hash)
            self.assertEqual(meta['provenance']['parent_case_sha256'], old_hash)
            self.assertFalse(meta['provenance']['original_engine_execution'])
            with open_case(child) as db, closing(PrefixDecoder()) as decoder:
                o, g, c, k, s, q, h, p = db.execute('SELECT * FROM events ORDER BY ordinal LIMIT 1').fetchone()
                self.assertEqual(decoder.decode(event_header(o, g, c, s, q, h, p), p, s + q), history)
            before = child.read_bytes()
            with self.assertRaises(FileExistsError):
                stress.derive(parent, child, 1, 1, 2, 24, {})
            self.assertEqual(child.read_bytes(), before)
            empty = root / 'empty.sqlite3'; source, _ = fixture(particles=False)
            create_case(empty, [(1, 0, source, b'', 2)], {})
            failed = root / 'failed.sqlite3'
            with self.assertRaises(ValueError):
                stress.derive(empty, failed, 1, 1, 4, 24, {})
            with self.assertRaises(ValueError):
                validate_case(failed)


class NativeTests(unittest.TestCase):
    def test_native_controls_and_serial_pixel_equivalence(self):
        if not OPTIONS.renderer:
            self.skipTest('Pass --renderer for native admission/rendering controls')
        temporary = None if OPTIONS.output else tempfile.TemporaryDirectory()
        root = OPTIONS.output if OPTIONS.output else Path(temporary.name)
        root.mkdir(exist_ok=temporary is not None, parents=True)
        (root / '.retain').write_text('Authored particle stress admission and native pixel controls; exact results/ELF and generation failures retained.\n')
        assets = root / 'assets'; assets.mkdir(); asset(assets)
        parent = root / 'parent.sqlite3'; create_case(parent, ((g, 0, fixture(g, create=g == 1)[0], b'', 2) for g in range(1, 17)), asset_manifest(assets))
        results = root / 'results.sqlite3'
        def command(args, env=None):
            result = subprocess.run([sys.executable, *map(str, args)], capture_output=True, text=True, env=env, timeout=120)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            summary = json.loads(result.stdout)
            with closing(sqlite3.connect(results)) as db:
                report = json.loads(db.execute('SELECT payload FROM runs WHERE id=?', (summary['run_id'],)).fetchone()[0])
            return summary['run_id'], report
        def verify(case, *extra, serial=False):
            return command([ROOT / 'tools/render-replay.py', 'verify', case, '--assets', assets, '--executable', OPTIONS.renderer,
                            '--backend', 'hardware' if OPTIONS.hardware else 'software', '--results', results, *extra],
                           dict(os.environ, MNM_WORLD_ADDITIVE_BATCH_BASELINE='1' if serial else '0'))
        _, baseline = verify(parent)
        for copies in [1, 4]:
            child = root / f'particles-{copies}x.sqlite3'
            _, generated = command([ROOT / 'tools/render-stress.py', child, '--from-case', parent, '--start', 1, '--end', 16,
                                    '--copies', copies, '--assets', assets, '--executable', OPTIONS.renderer, '--results', results])
            self.assertTrue(all('"renderer_started":false' in c['stdout'] for c in generated['commands']))
            if copies == 1:
                _, actual = verify(child)
                self.assertEqual([f['sha256'] for f in actual['native']['runs'][0]['frames']],
                                 [f['sha256'] for f in baseline['native']['runs'][0]['frames']])
            else:
                serial_id, _ = verify(child, serial=True)
                _, actual = verify(child, '--against', str(serial_id))
                self.assertEqual(actual['baseline_match'], serial_id)
        bad_source, _ = fixture(); bad_source = bytearray(bad_source); at = 64
        for r, payload in unpack(bad_source):
            if r[2] == 9:
                break
            at += r[0]
        struct.pack_into('<I', bad_source, at + 16, 999)
        bad_parent = root / 'malformed-parent.sqlite3'
        create_case(bad_parent, [(1, 0, bytes(bad_source), b'', 2)], asset_manifest(assets))
        refused = root / 'refused.sqlite3'
        result = subprocess.run([sys.executable, str(ROOT / 'tools/render-stress.py'), str(refused),
                                 '--from-case', str(bad_parent), '--start', '1', '--end', '1',
                                 '--assets', str(assets), '--executable', str(OPTIONS.renderer), '--results', str(results)],
                                capture_output=True, text=True, timeout=120)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertFalse(refused.exists())
        summary = json.loads(result.stdout)
        with closing(sqlite3.connect(results)) as db:
            failure = json.loads(db.execute('SELECT payload FROM runs WHERE id=?', (summary['run_id'],)).fetchone()[0])
        self.assertFalse(failure['success']); self.assertEqual(failure['commands'][0]['returncode'], 1)
        if temporary:
            temporary.cleanup()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(); parser.add_argument('--renderer', type=Path)
    parser.add_argument('--hardware', action='store_true'); parser.add_argument('--output', type=Path)
    OPTIONS, remainder = parser.parse_known_args()
    unittest.main(argv=[sys.argv[0], *remainder])
