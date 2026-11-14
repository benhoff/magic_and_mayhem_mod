#!/usr/bin/env python3
"""Create an authored native particle-storm replay from a validated parent case."""
import argparse
from contextlib import closing
import json
import os
from pathlib import Path
import runpy
import sqlite3
import struct
import subprocess
import tempfile

from native_world_replay import (MAX_SOURCE, check_assets, create_case, event_header,
                                 export_tape, open_case, sha_file, validate_case)
from resident_capture import PrefixDecoder

ROOT = Path(__file__).resolve().parents[1]


def signed(value):
    return value if value < 0x80000000 else value - 0x100000000


def amplify_source(source, request, kind, copies, spread):
    """Preserve source definitions; repair local references after insertion.

    This produces an authored closed native queue, never an original guarded
    acknowledgement or a claim that the original engine emitted extra draws.
    The CLI admits the complete parent with the native decoder before calling.
    """
    if copies not in (1, 2, 4, 8) or not 0 <= spread <= 128 or kind not in (1, 2):
        raise ValueError('Stress factor/spread/event kind')
    if len(source) < 64 or source[:8] != b'MNMPRO03':
        raise ValueError('Stress requires V3 World source')
    output = bytearray(source[:64]); struct.pack_into('<I', output, 20, 131072)
    sequence = entries = returns = rasters = particles = last_oracle = 0
    references = {}; entry = None; active = False; at = 64
    offsets = [(0, 0), (spread, 0), (-spread, 0), (0, spread),
               (0, -spread), (spread, spread), (-spread, spread), (spread, -spread)]

    def append(fields, payload=b''):
        nonlocal sequence
        sequence += 1
        if sequence > 131072 or len(output) + 96 + len(payload) > MAX_SOURCE:
            raise ValueError('Amplified World exceeds record/source byte budget')
        r = list(fields); r[0] = 96 + len(payload); r[1] = sequence; r[18] = len(payload)
        output.extend(struct.pack('<24I', *r)); output.extend(payload)
        return sequence

    while at < len(source):
        if at + 96 > len(source):
            raise ValueError('Truncated stress source')
        r = list(struct.unpack_from('<24I', source, at))
        if r[0] != 96 + r[18] or r[0] > len(source) - at or r[0] < 96:
            raise ValueError('Invalid stress record extent')
        payload = source[at + 96:at + r[0]]; at += r[0]
        if r[2] == 13:
            raise ValueError('Refused source cannot become a stress case')
        if r[2] in (8, 9) and r[4]:
            if r[4] not in references or payload:
                raise ValueError('Missing stress source definition')
            r[4] = references[r[4]]
        if r[2] == 11:
            entries += 1; entry = r[:]; active = True
            if entries != 1 or returns:
                raise ValueError('Stress needs exactly one World')
        if r[2] == 10:
            last_oracle = max(last_oracle, r[14])
        if active and r[2] == 9:
            rasters += 1
        if r[2] == 12:
            returns += 1; active = False; r[19] = rasters; r[15] = rasters
        old_sequence = r[1]
        definition = append(r, payload)
        if r[2] == 25:
            references[old_sequence] = definition
        if active and r[2] == 9 and r[14] == 8 and r[15] == 13:
            particles += 1
            for dx, dy in offsets[1:copies]:
                extra = r[:]
                for field, delta in [(8, dx), (9, dy)]:
                    value = signed(r[field]) + delta
                    if not -0x80000000 <= value <= 0x7fffffff:
                        raise ValueError('Stress anchor exceeds signed32 range')
                    extra[field] = value & 0xffffffff
                append(extra, payload); rasters += 1
    if entries != 1 or not rasters or rasters > 12320 or returns != (1 if kind == 2 else 0):
        raise ValueError('Stress World closure/raster count')
    if kind == 1:
        if len(request) < 64 or request[:8] != b'MNMWBQ03':
            raise ValueError('Stress parent request missing')
        checkpoint = [0] * 24
        for field in [3, 5, 6, 7]:
            checkpoint[field] = entry[field]
        checkpoint[2] = 10; checkpoint[14] = last_oracle + 1; checkpoint[15] = 4
        checkpoint[16] = sequence + 1; append(checkpoint)
        returned = checkpoint[:]; returned[2] = 12; returned[14] = 1; returned[15] = rasters
        returned[16] = 0; returned[17] = 1; returned[19] = rasters; returned[20] = 1
        append(returned)
    return bytes(output), dict(original_particles=particles, derived_particles=particles * copies,
                              raster_count=rasters, records=sequence)


def derive(parent, destination, first, last, copies, spread, assets, label=''):
    meta = validate_case(parent); parent_hash = sha_file(parent)
    if not 1 <= first <= last <= meta['worlds'] or copies not in (1, 2, 4, 8) or not 0 <= spread <= 128:
        raise ValueError('Stress range/factor/spread bounds')
    stats = []
    def events():
        with open_case(parent) as db, closing(PrefixDecoder()) as decoder:
            for o, g, c, k, s, q, h, packed in db.execute('SELECT * FROM events ORDER BY ordinal'):
                body = decoder.decode(event_header(o, g, c, s, q, h, packed), packed, s + q)
                if g > last:
                    continue
                if k == 0:
                    yield g, c, body[:s], b'', 0
                else:
                    transformed, count = amplify_source(body[:s], body[s:], k,
                                                       copies if g >= first else 1, spread)
                    if g >= first:
                        stats.append(dict(generation=g, **count))
                    yield g, 0, transformed, b'', 2
        if sha_file(parent) != parent_hash:
            raise ValueError('Stress parent changed during generation')
        if not any(x['original_particles'] for x in stats):
            raise ValueError('Selected range has no additive particles')
    provenance = dict(kind='authored_particle_amplification', parent_case_sha256=parent_hash,
                      original_engine_execution=False, copies=copies, spread=spread,
                      amplified_range=[first, last], scope='Drawing workload only; no added simulated creatures/AI.')
    result = create_case(destination, events(), assets, label=label or f'Authored {copies}x captured particles', provenance=provenance)
    return result, stats


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('case', type=Path)
    parser.add_argument('--from-case', dest='parent', type=Path, required=True)
    parser.add_argument('--start', type=int, required=True); parser.add_argument('--end', type=int, required=True)
    parser.add_argument('--copies', type=int, choices=[1, 2, 4, 8], default=4)
    parser.add_argument('--spread', type=int, default=24)
    parser.add_argument('--assets', type=Path, default=ROOT / 'working/game-nocd')
    parser.add_argument('--executable', type=Path)
    parser.add_argument('--results', type=Path, default=ROOT / 'working/render-replays/results.sqlite3')
    parser.add_argument('--label', default='')
    args = parser.parse_args(); replay = runpy.run_path(str(ROOT / 'tools/render-replay.py'))
    report = dict(success=False, mode='stress', case=str(args.case.resolve()), parent=str(args.parent.resolve()))
    executable = None; temporary = None; created = False; commands = []
    report['commands'] = commands
    try:
        if args.case.exists():
            raise ValueError('Case already exists; choose a new path')
        meta = validate_case(args.parent); parent_hash = sha_file(args.parent)
        check_assets(args.assets, meta['assets'])
        executable = args.executable.resolve() if args.executable else replay['build']()
        directory = ROOT / 'working/render-replays'; directory.mkdir(parents=True, exist_ok=True)
        temporary = Path(tempfile.mkdtemp(prefix='stress-interchange-', dir=directory))
        def admit(case, name):
            tape = temporary / name; export_tape(case, tape)
            command = [str(executable), str(tape), str(args.assets.resolve()), 'validate', '1', '1', '1', '20']
            result = subprocess.run(command, env=dict(os.environ, QT_QPA_PLATFORM='offscreen'), capture_output=True, text=True, timeout=900)
            commands.append(dict(command=command, returncode=result.returncode, stdout=result.stdout, stderr=result.stderr))
            native = json.loads(result.stdout.splitlines()[-1])
            if result.returncode or not native.get('success') or native.get('renderer_started') is not False:
                raise ValueError('Native stress admission failed: ' + native.get('error', result.stderr))
        admit(args.parent, 'parent.bin')
        case, counts = derive(args.parent, args.case, args.start, args.end, args.copies, args.spread, meta['assets'], args.label)
        created = True
        admit(args.case, 'derived.bin')
        check_assets(args.assets, meta['assets'])
        if sha_file(args.parent) != parent_hash:
            raise ValueError('Stress parent changed')
        report.update(success=True, case_sha256=sha_file(args.case), parent_case_sha256=parent_hash,
                      provenance=case['provenance'], worlds=case['worlds'], counts=counts,
                      commands=commands, executable_sha256=sha_file(executable))
    except (OSError, ValueError, KeyError, TypeError, IndexError, sqlite3.Error, subprocess.SubprocessError) as error:
        report.update(error=str(error), retained_interchange=str(temporary))
        if created:
            with closing(sqlite3.connect(args.case)) as db, db:
                db.execute('INSERT OR REPLACE INTO metadata VALUES(?,?)', ('failure', json.dumps(str(error))))
    run = replay['append_result'](args.results, report, executable)
    with closing(sqlite3.connect(args.results)) as db, db:
        raw = Path(__file__).read_bytes()
        db.execute('INSERT OR IGNORE INTO inputs VALUES(?,?,?)', (str(Path(__file__).resolve()), sha_file(__file__), raw))
    if report['success']:
        for path in temporary.iterdir():
            path.unlink()
        temporary.rmdir()
    print(json.dumps(dict(success=report['success'], run_id=run, case=str(args.case),
                          worlds=report.get('worlds'), error=report.get('error'), results=str(args.results))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
