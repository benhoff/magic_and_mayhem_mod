#!/usr/bin/env python3
"""Run bounded moving native effect workloads; retain inputs/results in SQLite."""
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
import time

ROOT = Path(__file__).resolve().parents[1]
CASES = ('wave', 'row-distortion', 'translucency', 'keyed-overlap')
SOURCES = ('tools/effect-stress.py', 'tools/render-replay/effect_stress.cpp',
           'tools/render-replay/CMakeLists.txt', 'tests/scene-resource-fixtures.hpp',
           'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/surface_copy.cpp',
           'renderer/surface_copy.hpp', 'renderer/scenes/scene_renderer.cpp',
           'renderer/scenes/scene_renderer.hpp', 'renderer/sprites/row_effect.hpp',
           'renderer/sprites/sprite.cpp', 'renderer/sprites/sprite.hpp',
           'renderer/resources/resource_cache.cpp', 'renderer/resources/resource_cache.hpp',
           'renderer/resources/sprite_atlas.cpp', 'renderer/resources/sprite_atlas.hpp')


def distribution(values):
    values = sorted(values)
    if not values:
        raise ValueError('No measured frames')
    return dict(samples=len(values), median=statistics.median(values),
                mean=statistics.fmean(values), p95=values[int(.95 * (len(values) - 1))],
                maximum=max(values))


def summarize(native, budget):
    if not native.get('success'):
        raise ValueError(native.get('error', 'Native effect run failed'))
    rows = native['samples']
    expected = [(repeat, frame) for repeat in range(native['repeats'])
                for frame in range(native['warmup'] + native['frames'])]
    if [(r['repeat'], r['frame']) for r in rows] != expected:
        raise ValueError('Missing, duplicated or reordered stress frames')
    if native.get('ordinary_readbacks') != 0 or not native.get('repeat_pixels_equal'):
        raise ValueError('Ordinary readback or repeat comparison failed')
    for row in rows:
        if (row['warmup'] != (row['frame'] < native['warmup']) or not row['pixels_equal']
                or row['pixel_sha256'] != row['expected_sha256']):
            raise ValueError('Pixel/warmup assertion differs')
        if not (0 <= row['submit_ms'] <= row['completion_ms']):
            raise ValueError('Native timing order differs')
    for repeat in range(1, native['repeats']):
        first = rows[:native['warmup'] + native['frames']]
        current = [r for r in rows if r['repeat'] == repeat]
        if [(r['input_sha256'], r['pixel_sha256']) for r in first] != [
                (r['input_sha256'], r['pixel_sha256']) for r in current]:
            raise ValueError('Repeat input/pixel hashes differ')
    first = rows[:native['warmup'] + native['frames']]
    if len(first) > 1 and len({r['input_sha256'] for r in first}) < 2:
        raise ValueError('Authored input did not move')
    measured = [r for r in rows if not r['warmup']]
    completion = distribution([r['completion_ms'] for r in measured])
    return dict(completion_ms=completion,
                submission_ms=distribution([r['submit_ms'] for r in measured]),
                over_budget_frames=sum(r['completion_ms'] > budget for r in measured),
                p95_within_budget=completion['p95'] <= budget,
                max_draw_calls=max(r['counters']['draw_calls'] for r in measured),
                max_snapshot_pixels=max(r['counters']['snapshot_pixels'] for r in measured),
                max_canvas_clones=max(r['counters']['canvas_clones'] for r in measured),
                max_upload_bytes=max(r['counters']['texture_upload_bytes'] for r in measured))


def compare(before, after):
    if not before['success'] or not after['success']:
        raise ValueError('Comparison needs successful native runs')
    a, b = before['native'], after['native']
    if (a['case'], a['level'], a['definition']) != (b['case'], b['level'], b['definition']):
        raise ValueError('Baseline effect definition differs')
    def inputs(native):
        return [(r['repeat'], r['frame'], r['input_sha256']) for r in native['samples']]
    if inputs(a) != inputs(b):
        raise ValueError('Baseline input frames differ')
    if [r['pixel_sha256'] for r in a['samples']] != [r['pixel_sha256'] for r in b['samples']]:
        raise ValueError('Baseline pixels differ')
    return dict(pixels_equal=True, same_driver=a['renderer'] == b['renderer'],
                before=before['summary']['completion_ms'], after=after['summary']['completion_ms'],
                scope='Only identical input/pixel sets. Compare performance with the same driver and isolated unprofiled runs; timing is not live FPS or GPU busy time.')


def connect(path):
    path.parent.mkdir(parents=True, exist_ok=True)
    db = sqlite3.connect(path)
    for table in ('runs', 'batches'):
        db.execute(f'CREATE TABLE IF NOT EXISTS {table}(id INTEGER PRIMARY KEY,created REAL,payload TEXT)')
    db.execute('CREATE TABLE IF NOT EXISTS inputs(path TEXT,sha256 TEXT,bytes BLOB,PRIMARY KEY(path,sha256))')
    return db


def retain(db, executable):
    hashes = {}
    for path in [*(ROOT / p for p in SOURCES), executable]:
        raw = path.read_bytes(); sha = hashlib.sha256(raw).hexdigest()
        name = str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)
        db.execute('INSERT OR IGNORE INTO inputs VALUES(?,?,?)', (name, sha, raw)); hashes[name] = sha
    return hashes


def build():
    directory = ROOT / 'working/build/render-replay'; directory.mkdir(parents=True, exist_ok=True)
    log = directory / 'effect-stress-build.log'
    with log.open('w') as stream:
        for command in (["cmake", "-S", str(ROOT / 'tools/render-replay'), "-B", str(directory), "-DCMAKE_BUILD_TYPE=Release"],
                        ["cmake", "--build", str(directory), "--target", "mnm-effect-stress", "-j2"]):
            subprocess.run(command, cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT, check=True)
    return directory / 'mnm-effect-stress'


def execute(args):
    if not (1 <= args.frames <= 120 and 1 <= args.repeat <= 5 and 0 <= args.warmup <= 8
            and (args.frames + args.warmup) * args.repeat <= 256):
        raise ValueError('Choose1..120frames,1..5repeats,0..8warmup; at most256total frames per case')
    if not 0 < args.budget_ms <= 10000:
        raise ValueError('Budget must be positive and at most10000ms')
    executable = args.executable.resolve() if args.executable else build()
    cases = list(CASES) if args.case == ['all'] else args.case
    if 'all' in cases or len(set(cases)) != len(cases) or len(set(args.levels)) != len(args.levels):
        raise ValueError('Choose unique cases/levels; all must stand alone')
    config = dict(cases=cases, levels=args.levels, frames=args.frames, repeats=args.repeat,
                  warmup=args.warmup, gpu_timing=args.gpu_timing, backend=args.backend)
    env = dict(os.environ, QT_QPA_PLATFORM='xcb')
    env.pop('LIBGL_ALWAYS_SOFTWARE', None); env.pop('GALLIUM_DRIVER', None)
    if args.backend == 'software':
        env.update(LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe')
    baseline = {}
    with closing(connect(args.results)) as db, db:
        if args.against is not None:
            row = db.execute('SELECT payload FROM batches WHERE id=?', (args.against,)).fetchone()
            if not row:
                raise ValueError('Unknown baseline batch')
            prior = json.loads(row[0])
            if not prior['success']:
                raise ValueError('Baseline batch did not pass pixel/contract checks')
            for ident in prior['run_ids']:
                report = json.loads(db.execute('SELECT payload FROM runs WHERE id=?', (ident,)).fetchone()[0])
                baseline[(report['case'], report['level'])] = report
            if set(baseline) != {(c, level) for c in cases for level in args.levels}:
                raise ValueError('Baseline case/level set differs')
        hashes = retain(db, executable); db.commit()
        reports = []; ids = []
        for case in cases:
            for level in args.levels:
                command = [str(executable), case, str(level), str(args.frames), str(args.repeat), str(args.warmup), str(int(args.gpu_timing))]
                if args.backend == 'software':
                    command = ['xvfb-run', '-a', *command]
                report = dict(case=case, level=level, command=command, success=False,
                              source_revision=subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip(),
                              source_hashes=hashes, budget_ms=args.budget_ms, backend=args.backend)
                try:
                    result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True, timeout=180)
                    report.update(returncode=result.returncode, stdout=result.stdout, stderr=result.stderr)
                    native = json.loads(result.stdout.splitlines()[-1]); report['native'] = native
                    report['summary'] = summarize(native, args.budget_ms)
                    if result.returncode or (native['case'], native['level']) != (case, level):
                        raise ValueError('Native return/configuration differs')
                    software = any(n in native['renderer'].lower() for n in ('llvmpipe', 'softpipe', 'swrast', 'software rasterizer'))
                    if software != (args.backend == 'software'):
                        raise ValueError('Actual renderer does not match requested backend: ' + native['renderer'])
                    report['success'] = True
                    if baseline:
                        report['comparison'] = compare(baseline[(case, level)], report)
                except (OSError, ValueError, KeyError, IndexError, subprocess.SubprocessError) as error:
                    report.update(success=False, error=str(error))
                    if isinstance(error, subprocess.TimeoutExpired):
                        report.update(stdout=str(error.stdout or ''), stderr=str(error.stderr or ''))
                ident = db.execute('INSERT INTO runs(created,payload) VALUES(?,?)', (time.time(), json.dumps(report))).lastrowid
                db.commit(); reports.append(report); ids.append(ident)
                print(json.dumps(dict(run_id=ident, case=case, level=level, success=report['success'],
                                      summary=report.get('summary'), error=report.get('error'))), flush=True)
        batch = dict(success=all(r['success'] for r in reports), config=config, run_ids=ids,
                     budget_ms=args.budget_ms, against=args.against,
                     budget_met=all(r.get('summary', {}).get('p95_within_budget', False) for r in reports),
                     scope='Authored native rendering stress only. Performance overruns are observations, separate from pixel/contract failures. No base engine execution or live FPS claim.')
        ident = db.execute('INSERT INTO batches(created,payload) VALUES(?,?)', (time.time(), json.dumps(batch))).lastrowid
    print(json.dumps(dict(batch_id=ident, results=str(args.results), **batch)), flush=True)
    return 0 if batch['success'] and (batch['budget_met'] or not args.require_budget) else 1


def show(args):
    with closing(sqlite3.connect(args.results.resolve().as_uri() + '?mode=ro', uri=True)) as db:
        row = db.execute('SELECT id,payload FROM batches ORDER BY id DESC LIMIT 1').fetchone() if args.batch is None else db.execute('SELECT id,payload FROM batches WHERE id=?', (args.batch,)).fetchone()
        if not row:
            raise ValueError('No requested batch')
        batch = json.loads(row[1]); print('Batch', row[0], 'pixel/contract success:', batch['success'], 'budget met:', batch['budget_met'])
        print('case                 level  p95 ms  max ms  >budget  draws  clones')
        for ident in batch['run_ids']:
            r = json.loads(db.execute('SELECT payload FROM runs WHERE id=?', (ident,)).fetchone()[0]); s = r.get('summary')
            if not s:
                print(r['case'], r['level'], 'FAILED:', r.get('error')); continue
            print(f"{r['case']:21} {r['level']:5} {s['completion_ms']['p95']:7.2f} {s['completion_ms']['maximum']:7.2f} {s['over_budget_frames']:8} {s['max_draw_calls']:6} {s['max_canvas_clones']:7}")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='action', required=True)
    run = sub.add_parser('run', help='Run all four families, three levels by default')
    run.add_argument('--case', nargs='+', choices=('all', *CASES), default=['all'])
    run.add_argument('--levels', nargs='+', choices=(1, 2, 3), type=int, default=[1, 2, 3])
    run.add_argument('--frames', type=int, default=8); run.add_argument('--repeat', type=int, default=3)
    run.add_argument('--warmup', type=int, default=2); run.add_argument('--gpu-timing', action='store_true')
    run.add_argument('--backend', choices=('hardware', 'software'), default='hardware')
    run.add_argument('--budget-ms', type=float, default=50); run.add_argument('--require-budget', action='store_true')
    run.add_argument('--executable', type=Path); run.add_argument('--against', type=int)
    view = sub.add_parser('show'); view.add_argument('--batch', type=int)
    for command in (run, view):
        command.add_argument('--results', type=Path, default=ROOT / 'working/effect-stress/results.sqlite3')
    args = parser.parse_args()
    try:
        return execute(args) if args.action == 'run' else show(args)
    except (OSError, ValueError, sqlite3.Error, subprocess.SubprocessError) as error:
        print(str(error), file=sys.stderr); return 1


if __name__ == '__main__':
    raise SystemExit(main())
