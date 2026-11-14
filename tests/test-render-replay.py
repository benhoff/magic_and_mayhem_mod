#!/usr/bin/env python3
"""Replay data admission plus optional actual native moving-crowd controls."""
import argparse
from contextlib import closing
import json
import os
from pathlib import Path
import shutil
import sqlite3
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from native_world_replay import (create_case, validate_case, export_tape, open_case,
                                 event_header, check_assets, asset_manifest,
                                 import_capture)
from resident_capture import PrefixDecoder

OPTIONS = None


def authored_world(generation, actors=32):
    """Small actual owned producer packets; no original assets or simulation."""
    operations = []
    def add(op, fields=None, payload=b''):
        r = [0] * 24
        r[0:4] = [96 + len(payload), len(operations) + 1, op, 7]
        r[5:8] = [64, 48, 64]
        r[18] = len(payload)
        for key, value in (fields or {}).items():
            r[key] = value
        operations.append(struct.pack('<24I', *r) + payload)
    if generation == 1:
        add(1)
    add(5, {12: 64, 13: 48, 14: 0x1234})
    add(11, {14: 1})
    sprite = bytearray(55)
    for at, value in [(0, 55), (4, 3), (8, 1), (12, 1), (16, 0xffffffff), (40, 48), (44, 51)]:
        struct.pack_into('<I', sprite, at, value)
    sprite[48:51] = b'\0\2\1'
    sprite[53:55] = b'\0\xf8'
    for actor in range(actors):
        add(9, {8: (actor * 7 + generation) % 60 + 1,
                9: (actor * 3 + generation // 10) % 43 + 1,
                12: 64, 13: 48, 15: 8, 19: 55}, bytes(sprite))
    add(26, {8: generation % 60, 9: 2, 12: 64, 13: 48, 15: 2, 19: 2, 20: 2}, b'\xe0\7' * 4)
    add(10, {14: 1, 15: 4})
    add(12, {14: 1})
    header = bytearray(64)
    header[:8] = b'MNMPRO03'
    struct.pack_into('<5I', header, 8, 3, 64, 0x40209ca7, 131072, 32)
    return bytes(header) + b''.join(operations)


def fixture_assets(root):
    body = bytearray(36 + 55 * 2 + 40)
    def put(at, value):
        struct.pack_into('<I', body, at, value)
    for at, value in [(0, 0x00525053), (4, len(body)), (8, 4), (12, 3)]:
        put(at, value)
    for index in range(3):
        offset = 55 * index if index < 2 else 110
        at = 36 + offset
        put(24 + index * 4, offset)
        put(at, 55 if index < 2 else 40)
        put(at + 28, 0xffffffff)
        if index == 2:
            continue
        for pos, value in [(4, 3), (8, 1), (12, 1), (16, 0xffffffff), (40, 48), (44, 51)]:
            put(at + pos, value)
        body[at + 48:at + 51] = b'\0\2\1'
        body[at + 53:at + 55] = b'\0\xf8'
    (root / 'body.spr').write_bytes(body)


def authored_case(path, frames=64, assets=None):
    return create_case(path, ((g, 0, authored_world(g), b'', 2) for g in range(1, frames + 1)),
                       assets or {}, label='Authored32moving SPR instances and destination-dependent rectangles',
                       provenance=dict(kind='authored_native_controls', original_engine_execution=False))


class DataTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.case = self.root / 'case.sqlite3'

    def tearDown(self):
        self.temp.cleanup()

    def test_prefix_codec_keyframes_and_exact_bytes(self):
        meta = authored_case(self.case, 260)
        self.assertEqual(validate_case(self.case)['worlds'], 260)
        decoder = PrefixDecoder()
        try:
            with open_case(self.case) as db:
                for ordinal, generation, chunk, kind, source, request, digest, packed in db.execute('SELECT * FROM events ORDER BY ordinal'):
                    body = decoder.decode(event_header(ordinal, generation, chunk, source, request, digest, packed), packed, source + request)
                    self.assertEqual(body, authored_world(generation))
            self.assertLess(meta['compressed_bytes'], meta['source_raw_bytes'] // 3)
        finally:
            decoder.close()

    def test_corrupt_tail_is_not_accepted_as_prefix(self):
        capture = self.root / 'capture.bin'
        capture.write_bytes(b'MNMPROF1' + struct.pack('<QII', 1, 16, 64) + b'x' * 80 + b'bad-tail')
        with self.assertRaises(ValueError):
            import_capture(capture, self.case, {}, prefix=1)
        with self.assertRaises(ValueError):
            validate_case(self.case)

    def test_existing_case_not_overwritten(self):
        authored_case(self.case)
        initial = self.case.read_bytes()
        with self.assertRaises(FileExistsError):
            authored_case(self.case)
        self.assertEqual(self.case.read_bytes(), initial)

    def test_corruption_and_generation_gap_refused(self):
        for column, expression in [('generation', 'generation+1'), ('sha256', "'0'"),
                                   ('packed', "CAST(packed||x'00' AS BLOB)"), ('kind', '9')]:
            case = self.root / (column + '.sqlite3')
            authored_case(case)
            with closing(sqlite3.connect(case)) as db, db:
                db.execute('UPDATE events SET ' + column + '=' + expression + ' WHERE ordinal=20')
            with self.assertRaises(ValueError):
                validate_case(case)

    def test_source_history_preserved_and_dangling_history_refused(self):
        events = [(1, 1, b'x' * 16, b'', 0), (1, 0, b'y' * 16, b'q' * 64, 1),
                  (2, 2, b'z' * 16, b'', 0), (2, 0, b'a' * 16, b'q' * 64, 1)]
        create_case(self.case, iter(events), {})
        self.assertEqual(validate_case(self.case)['events'], 4)
        with self.assertRaises(ValueError):
            create_case(self.root / 'dangling.sqlite3', iter(events[:-1]), {})
        with self.assertRaises(ValueError):
            create_case(self.root / 'skip.sqlite3', iter(events[1:]), {})
        with self.assertRaises(ValueError):
            create_case(self.root / 'history-gap.sqlite3', iter([(1, 2, b'x' * 16, b'', 0)]), {})

    def test_asset_changes_and_escapes_refused(self):
        assets = self.root / 'assets'; assets.mkdir(); (assets / 'sprite').write_bytes(b'render-input')
        manifest = asset_manifest(assets)
        check_assets(assets, manifest)
        (assets / 'sprite').write_bytes(b'changed')
        with self.assertRaises(ValueError):
            check_assets(assets, manifest)
        with self.assertRaises(ValueError):
            check_assets(assets, {'../outside': [1, 'bad']})


class NativeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not OPTIONS.renderer:
            raise unittest.SkipTest('Pass --renderer for actual OpenGL controls')
        cls.temp = None if OPTIONS.output else tempfile.TemporaryDirectory()
        cls.root = OPTIONS.output if OPTIONS.output else Path(cls.temp.name)
        if OPTIONS.output:
            cls.root.mkdir(parents=True, exist_ok=False)
            (cls.root / '.retain').write_text('Authored native replay cases, per-frame results, native ELF/source identities and failure controls; preserve for this test batch.\n')
        cls.assets = cls.root / 'assets'; cls.assets.mkdir()
        fixture_assets(cls.assets)
        cls.case = cls.root / 'crowd.sqlite3'; authored_case(cls.case, assets=asset_manifest(cls.assets))
        cls.results = cls.root / 'results.sqlite3'

    @classmethod
    def tearDownClass(cls):
        if cls.temp:
            cls.temp.cleanup()

    def replay(self, mode, *extra):
        cmd = [sys.executable, str(ROOT / 'tools/render-replay.py'), mode, str(self.case),
               '--assets', str(self.assets), '--executable', str(OPTIONS.renderer),
               '--backend', 'hardware' if OPTIONS.hardware else 'software',
               '--results', str(self.results), *extra]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        summary = json.loads(result.stdout)
        with closing(sqlite3.connect(self.results)) as db:
            report = json.loads(db.execute('SELECT payload FROM runs WHERE id=?', (summary['run_id'],)).fetchone()[0])
        return summary, report

    def test_moving_frames_repeat_and_baseline(self):
        summary, report = self.replay('verify', '--repeat', '3')
        frames = report['native']['runs'][0]['frames']
        self.assertGreater(len({x['sha256'] for x in frames}), 40)
        self.assertEqual(report['native']['ordinary_readbacks'], 0)
        self.replay('verify', '--against', str(summary['run_id']))

    def test_selected_range_restores_earlier_state(self):
        _, full = self.replay('verify')
        _, partial = self.replay('verify', '--start', '33', '--end', '48')
        self.assertEqual([x['sha256'] for x in full['native']['runs'][0]['frames'][32:48]],
                         [x['sha256'] for x in partial['native']['runs'][0]['frames']])
        _, benchmark = self.replay('benchmark', '--start', '33', '--end', '48')
        self.assertEqual(benchmark['native']['runs'][0]['final_sha256'],
                         partial['native']['runs'][0]['final_sha256'])
        self.assertEqual(benchmark['native']['runs'][0]['diagnostic_readbacks'], 1)

    def test_opt_in_owner_profile_preserves_readback_contract(self):
        previous = os.environ.get('MNM_WORLD_OWNER_PROFILE')
        try:
            os.environ['MNM_WORLD_OWNER_PROFILE'] = '1'
            _, report = self.replay('benchmark', '--start', '33', '--end', '36')
        finally:
            if previous is None:
                os.environ.pop('MNM_WORLD_OWNER_PROFILE', None)
            else:
                os.environ['MNM_WORLD_OWNER_PROFILE'] = previous
        native = report['native']
        self.assertEqual(native['ordinary_readbacks'], 0)
        self.assertEqual(native['runs'][0]['diagnostic_readbacks'], 1)
        for frame in native['runs'][0]['frames']:
            owner = frame['owner_profile']
            self.assertEqual(len(owner['producer_ms']), 28)
            self.assertEqual(len(owner['producer_calls']), 28)
            self.assertGreater(owner['total_ms'], 0)
            self.assertGreater(sum(owner['producer_calls']), 0)
            for key in ['prepare_ms', 'scene_admission_ms', 'scene_drawing_ms', 'identities_ms']:
                self.assertGreaterEqual(owner[key], 0)
            additive = frame['additive_profile']
            for key in ['batches', 'sprites', 'visited_pixels', 'upload_pixels',
                        'planning_ms', 'combination_ms', 'upload_ms', 'composite_ms']:
                self.assertGreaterEqual(additive[key], 0)

    def test_real_source_history_is_applied_before_world(self):
        first = authored_world(1)
        # Move CREATE/FILL into a source-only prefix. Renumber the remaining
        # closed World without changing its draws or captured final pixels.
        prefix = first[:64 + 192]
        closed = bytearray(first[:64] + first[64 + 192:])
        at = 64; sequence = 1
        while at < len(closed):
            struct.pack_into('<I', closed, at + 4, sequence)
            at += struct.unpack_from('<I', closed, at)[0]
            sequence += 1
        case = self.root / 'source-history.sqlite3'
        events = [(1, 1, prefix, b'', 0), (1, 0, bytes(closed), b'', 2)]
        events.extend((g, 0, authored_world(g), b'', 2) for g in range(2, 65))
        create_case(case, iter(events), asset_manifest(self.assets), provenance=dict(kind='authored_source_history'))
        _, baseline = self.replay('verify')
        original = self.case
        try:
            self.case = case
            _, actual = self.replay('verify')
        finally:
            self.case = original
        self.assertEqual([x['sha256'] for x in baseline['native']['runs'][0]['frames']],
                         [x['sha256'] for x in actual['native']['runs'][0]['frames']])

    def test_actual_viewer_rewinds_without_global_input(self):
        tape = self.root / 'viewer.bin'
        export_tape(self.case, tape)
        cmd = [str(OPTIONS.renderer), str(tape), str(self.assets), 'view', '1', '20', '1', '1000', '2']
        env = dict(os.environ, QT_QPA_PLATFORM='xcb')
        if not OPTIONS.hardware:
            cmd = ['xvfb-run', '-a', *cmd]
            env['LIBGL_ALWAYS_SOFTWARE'] = '1'
        result = subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        report = json.loads(result.stdout)
        self.assertTrue(report['success'])
        self.assertEqual(report['seek_restarts'], 1)
        self.assertEqual(report['diagnostic_readbacks'], 2)
        with closing(sqlite3.connect(self.results)) as db, db:
            db.execute('CREATE TABLE IF NOT EXISTS controls(kind TEXT,payload TEXT)')
            db.execute('INSERT INTO controls VALUES(?,?)', ('viewer-rewind', json.dumps(dict(command=cmd,returncode=result.returncode,stdout=result.stdout,stderr=result.stderr))))

    def test_baseline_pixel_difference_is_a_failure(self):
        summary, report = self.replay('verify')
        damaged = self.root / 'bad-baseline-for-refusal.sqlite3'
        with closing(sqlite3.connect(self.results)) as source, closing(sqlite3.connect(damaged)) as target:
            source.backup(target)
        report['native']['runs'][0]['frames'][7]['sha256'] = '0' * 64
        with closing(sqlite3.connect(damaged)) as db, db:
            db.execute('UPDATE runs SET payload=? WHERE id=?', (json.dumps(report), summary['run_id']))
        cmd = [sys.executable, str(ROOT / 'tools/render-replay.py'), 'verify', str(self.case),
               '--assets', str(self.assets), '--executable', str(OPTIONS.renderer),
               '--backend', 'hardware' if OPTIONS.hardware else 'software',
               '--results', str(damaged), '--against', str(summary['run_id'])]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('Baseline pixels differ at frame 8', result.stdout)
        with closing(sqlite3.connect(self.results)) as db, db:
            db.execute('CREATE TABLE IF NOT EXISTS controls(kind TEXT,payload TEXT)')
            db.execute('INSERT INTO controls VALUES(?,?)', ('baseline-refusal', json.dumps(dict(command=cmd,returncode=result.returncode,stdout=result.stdout,stderr=result.stderr))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--renderer', type=Path)
    parser.add_argument('--hardware', action='store_true')
    parser.add_argument('--output', type=Path, help='Retain native cases/results in this new directory')
    OPTIONS, remainder = parser.parse_known_args()
    unittest.main(argv=[sys.argv[0], *remainder])
