#!/usr/bin/env python3
"""Record once, then inspect, view, verify or benchmark a native World sequence."""
import argparse
from contextlib import closing
import hashlib
import json
import os
from pathlib import Path
import sqlite3
import statistics
import subprocess
import sys
import tempfile
import time

from native_world_replay import (asset_manifest, check_assets, export_tape,
                                 import_capture, sha_file, validate_case)

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_RESULTS = ROOT / 'working/render-replays/results.sqlite3'


def bounded(value):
    number = int(value)
    if not 1 <= number <= 8192:
        raise argparse.ArgumentTypeError('Choose a frame in1..8192')
    return number


def build():
    directory = ROOT / 'working/build/render-replay'
    directory.mkdir(parents=True, exist_ok=True)
    log = directory / 'build-cli.log'
    with log.open('w') as output:
        for command in [
                ['cmake', '-S', str(ROOT / 'tools/render-replay'), '-B', str(directory), '-DCMAKE_BUILD_TYPE=Release'],
                ['cmake', '--build', str(directory), '--target', 'mnm-world-replay', '-j2']]:
            result = subprocess.run(command, cwd=ROOT, stdout=output, stderr=subprocess.STDOUT)
            if result.returncode:
                raise ValueError('Replay build failed; see ' + str(log))
    return directory / 'mnm-world-replay'


def append_result(database, report, executable=None):
    database = Path(database)
    database.parent.mkdir(parents=True, exist_ok=True)
    with closing(sqlite3.connect(database)) as db, db:
        db.execute('CREATE TABLE IF NOT EXISTS runs(id INTEGER PRIMARY KEY,created REAL,payload TEXT)')
        db.execute('CREATE TABLE IF NOT EXISTS inputs(path TEXT,sha256 TEXT,bytes BLOB,PRIMARY KEY(path,sha256))')
        paths = [Path(__file__), ROOT / 'tools/native_world_replay.py', ROOT / 'tools/render-replay/main.cpp',
                 ROOT / 'tools/render-replay/CMakeLists.txt']
        if executable:
            paths.append(executable)
        for path in paths:
            data = path.read_bytes()
            db.execute('INSERT OR IGNORE INTO inputs VALUES(?,?,?)',
                       (str(path), hashlib.sha256(data).hexdigest(), data))
        cursor = db.execute('INSERT INTO runs(created,payload) VALUES(?,?)', (time.time(), json.dumps(report)))
        return cursor.lastrowid


def distribution(values):
    values = sorted(values)
    if not values:
        return {}
    return dict(samples=len(values), median=statistics.median(values),
                mean=statistics.fmean(values), p95=values[int(.95 * (len(values) - 1))],
                maximum=max(values))


def execute(args):
    meta = validate_case(args.case)
    case_hash = sha_file(args.case)
    check_assets(args.assets, meta['assets'])
    last = args.end or meta['worlds']
    if not 1 <= args.start <= last <= meta['worlds']:
        raise ValueError('Selected range is outside the captured prefix')
    repeat = args.repeat if args.repeat is not None else (2 if args.action == 'verify' else 1)
    if not 1 <= repeat <= 8 or (args.action == 'verify' and repeat < 2):
        raise ValueError('Repeat count must be1..8; verification needs at least2')
    baseline = None
    if args.against is not None:
        if args.action != 'verify':
            raise ValueError('--against requires verify')
        with closing(sqlite3.connect(Path(args.results).resolve().as_uri() + '?mode=ro', uri=True)) as db:
            row = db.execute('SELECT payload FROM runs WHERE id=?', (args.against,)).fetchone()
        if not row:
            raise ValueError('Unknown baseline run')
        baseline = json.loads(row[0])
        if (baseline.get('case_sha256') != case_hash or baseline.get('mode') != 'verify'
                or not baseline.get('success')):
            raise ValueError('Baseline must be a successful verification of this exact case')
    executable = args.executable.resolve() if args.executable else build()
    parent = ROOT / 'working/render-replays'
    parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix='interchange-', dir=parent))
    tape = temporary / 'worlds.bin'
    env = dict(os.environ, QT_QPA_PLATFORM='xcb')
    if args.backend == 'software':
        env.update(LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe')
    else:
        env.pop('LIBGL_ALWAYS_SOFTWARE', None)
        env.pop('GALLIUM_DRIVER', None)
    command = [str(executable), str(tape), str(args.assets.resolve()), args.action,
               str(args.start), str(last), str(repeat), str(args.fps)]
    if args.backend == 'software':
        command = ['xvfb-run', '-a', *command]
    report = dict(success=False, mode=args.action, case=str(args.case.resolve()),
                  case_sha256=case_hash, executable_sha256=sha_file(executable),
                  source_revision=subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip(),
                  backend=args.backend, command=command, selected_range=[args.start, last],
                  fps=args.fps, scope=meta['scope'], label=meta.get('label', ''))
    completed = False
    try:
        report['tape_sha256'] = export_tape(args.case, tape)
        # Interchange is reconstructible from the immutable SQLite case. Capture
        # files and the sole case are never removed here.
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True,
                                timeout=None if args.action == 'view' else 900)
        report.update(returncode=result.returncode, stdout=result.stdout, stderr=result.stderr)
        try:
            report['native'] = json.loads(result.stdout.splitlines()[-1])
        except (ValueError, IndexError):
            raise ValueError('Replay returned no native result')
        if result.returncode or not report['native'].get('success'):
            raise ValueError(report['native'].get('error', 'Native replay failed'))
        if baseline:
            prior = baseline['native']
            if [prior['first_frame'], prior['last_frame']] != [args.start, last]:
                raise ValueError('Baseline range differs')
            expected = [(x['generation'], x['sha256']) for x in prior['runs'][0]['frames']]
            for run in report['native']['runs']:
                actual = [(x['generation'], x['sha256']) for x in run['frames']]
                if actual != expected:
                    mismatch = next((a[0] for a, b in zip(actual, expected) if a != b), None)
                    raise ValueError('Baseline pixels differ at frame ' + str(mismatch))
            report['baseline_match'] = args.against
        # The case and asset inputs must still be identical after execution.
        if sha_file(args.case) != case_hash:
            raise ValueError('Replay case changed during execution')
        check_assets(args.assets, meta['assets'])
        report['success'] = completed = True
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
        report['retained_interchange'] = str(temporary)
    finally:
        run_id = append_result(args.results, report, executable)
        if completed:
            tape.unlink()
            temporary.rmdir()
    summary = dict(success=report['success'], run_id=run_id, results=str(args.results),
                   case=str(args.case), frames=[args.start, last], error=report.get('error'))
    rows = [row for run in report.get('native', {}).get('runs', []) for row in run['frames']]
    if rows:
        summary['native_completion_ms'] = distribution([row['native_completion_ms'] for row in rows])
        summary['timing_scope'] = 'Serial native CPU+GPU completion; decode/I/O/readback/hash excluded. Not live-game FPS or scanout.'
    print(json.dumps(summary))
    return 0 if report['success'] else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='action', required=True)
    for action in ['record', 'import', 'inspect', 'view', 'verify', 'benchmark']:
        sub = commands.add_parser(action)
        sub.add_argument('case', type=Path, help='Immutable SQLite replay case')
        if action != 'inspect':
            sub.add_argument('--assets', type=Path, default=ROOT / 'working/game-nocd')
        if action == 'record':
            sub.add_argument('--frames', type=bounded, default=3600, help='Stop capture I/O after this many Worlds; gameplay continues')
            sub.add_argument('--label', default='')
        elif action == 'import':
            sub.add_argument('--capture', type=Path, required=True)
            sub.add_argument('--prefix', type=bounded)
            sub.add_argument('--label', default='')
        elif action in ['view', 'verify', 'benchmark']:
            sub.add_argument('--start', type=bounded, default=1)
            sub.add_argument('--end', type=bounded)
            sub.add_argument('--repeat', type=int)
            sub.add_argument('--fps', type=float, default=20, help='Viewer rate; captured positions/actions remain unchanged')
            sub.add_argument('--backend', choices=['hardware', 'software'], default='hardware')
            sub.add_argument('--executable', type=Path)
            sub.add_argument('--results', type=Path, default=DEFAULT_RESULTS)
            sub.add_argument('--against', type=int, help='Compare frame hashes against a prior successful verify run ID')
    args = parser.parse_args()
    try:
        if args.action == 'inspect':
            meta = validate_case(args.case)
            print(json.dumps({key: value for key, value in meta.items() if key != 'assets'}, indent=2))
            return 0
        if args.action in ['view', 'verify', 'benchmark']:
            if not 1 <= args.fps <= 1000:
                raise ValueError('Viewer rate must be1..1000')
            return execute(args)
        if args.case.exists():
            raise ValueError('Case already exists; choose a new path')
        capture = args.capture if args.action == 'import' else None
        assets = asset_manifest(args.assets)
        if args.action == 'record':
            parent = ROOT / 'working/render-replays'
            parent.mkdir(parents=True, exist_ok=True)
            recording = Path(tempfile.mkdtemp(prefix='recording-', dir=parent))
            capture = recording / 'resident-source.bin'
            env = dict(os.environ, MNM_RESIDENT_STAGE_PROFILE='1',
                       MNM_RESIDENT_PROFILE_CAPTURE=str(capture),
                       MNM_RESIDENT_CAPTURE_PREFIX='1', MNM_RESIDENT_CAPTURE_MAX_WORLDS=str(args.frames))
            env.pop('MNM_RESIDENT_PROFILE_CAPTURE_DIRECTORY', None)
            print('Recording the first gameplay session. Load a prepared battle or perform the actions once.\n'
                  'Capture stops at the frame limit; save and quit normally to finish the case.\n'
                  'Recording is diagnostic and can affect live FPS. Capture directory: ' + str(recording), flush=True)
            code = subprocess.run([str(ROOT / 'run-native.sh')], cwd=ROOT, env=env).returncode
            if code:
                raise ValueError('Game did not exit cleanly; capture retained at ' + str(recording))
        meta = import_capture(capture, args.case, assets, label=args.label,
                              prefix=args.prefix if args.action == 'import' else None)
        check_assets(args.assets, assets)
        print(json.dumps(dict(success=True, case=str(args.case), worlds=meta['worlds'],
                              compressed_bytes=meta['compressed_bytes'], label=meta['label'])))
        return 0
    except (OSError, ValueError, TypeError, KeyError, sqlite3.Error, subprocess.SubprocessError) as error:
        print(json.dumps(dict(success=False, error=str(error))))
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
