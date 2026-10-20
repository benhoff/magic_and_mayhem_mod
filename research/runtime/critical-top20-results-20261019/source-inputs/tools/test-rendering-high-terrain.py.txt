#!/usr/bin/env python3
"""Run the 17 bounded High terrain comparisons against prospectively pinned sources.

Build apps/terrain-preview normally and with ASan/UBSan first. The existing
component runners retain their own raw outputs and original-manifest bookends.
This wrapper records failures without treating a report's existence as a pass.
Run under tools/working-retention.py; graphical cases require Xvfb.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
from contextlib import ExitStack

from terrain_validation_display import private_display

ROOT = Path(__file__).resolve().parents[1]
BUILD_HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
CASES = [
    ('RE.terrain_map', 'terrain-map', 'test-terrain-map.py', 'preview'),
    ('RE.terrain_sections', 'terrain-sections', 'test-terrain-sections.py', 'preview'),
    ('RE.terrain_region', 'terrain-region', 'test-terrain-region.py', 'region'),
    ('RE.terrain_catalog', 'terrain-catalog', 'test-terrain-catalog.py', 'plain'),
    ('RE.terrain_selection', 'terrain-selection', 'test-terrain-selection.py', 'plain'),
    ('RE.terrain_constraints', 'terrain-constraints', 'test-terrain-constraints.py', 'plain'),
    ('RE.terrain_solver', 'terrain-solver', 'test-terrain-solver.py', 'plain'),
    ('RE.terrain_specific', 'terrain-specific', 'test-terrain-specific.py', 'plain'),
    ('RE.terrain_generation', 'terrain-generation', 'test-terrain-generation.py', 'plain'),
    ('RE.terrain_generated', 'terrain-generated', 'test-terrain-generated.py', 'plain'),
    ('RE.terrain_camera', 'terrain-camera', 'test-terrain-camera.py', 'preview'),
    ('RE.terrain_traversal', 'world-terrain', 'test-world-terrain-preview.py', 'world'),
    ('RE.terrain_submission', 'terrain-submission', 'test-terrain-submission.py', 'main'),
    ('RE.palette_preferences', 'terrain-preferences', 'test-terrain-palette-preferences.py', 'shaded'),
    ('RE.palette_shading', 'terrain-palette-shading', 'test-terrain-palette-shading.py', 'shaded'),
    ('RE.terrain_lighting', 'terrain-light-fields', 'test-terrain-light-fields.py', 'shaded'),
    ('RE.terrain_creature_lighting', 'terrain-creature-lights', 'test-terrain-creature-lights.py', 'plain'),
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--executable', type=Path, required=True)
    p.add_argument('--source-root', type=Path, required=True)
    p.add_argument('--normal-build', type=Path, required=True)
    p.add_argument('--sanitized-build', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--behavior', action='append', choices=[c[0] for c in CASES])
    p.add_argument('--jobs', type=int, choices=range(1, 4), default=1)
    a = p.parse_args()
    source = a.source_root.resolve()
    normal, sanitized = a.normal_build.resolve(), a.sanitized_build.resolve()
    out = a.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').write_text('High terrain validation: retain failures and raw component reports.\n')
    declaration = json.loads(a.claims.read_text())
    claims = {c['behavior']: c for c in declaration['claims']}
    selected = [c for c in CASES if not a.behavior or c[0] in a.behavior]
    if a.behavior and len(a.behavior) != len(set(a.behavior)):
        raise ValueError('Duplicate behavior selection')
    if not {c[0] for c in selected} <= claims.keys():
        raise ValueError('Selected behaviors need prospective claims')
    for name, expected in declaration['sources'].items():
        if sha(ROOT / name) != expected or sha(source / name) != expected:
            raise ValueError('Declared main/frozen source differs: ' + name)
    exe = a.executable.resolve()
    if sha(exe) != BUILD_HASH:
        raise ValueError('Unsupported original executable')
    sources = dict(declaration['sources'])
    sources['tools/test-rendering-high-terrain.py'] = sha(Path(__file__))
    sources['tools/terrain_validation_inputs.py'] = sha(ROOT / 'tools/terrain_validation_inputs.py')
    sources['tools/terrain_validation_pixels.py'] = sha(ROOT / 'tools/terrain_validation_pixels.py')
    sources['tools/terrain_validation_display.py'] = sha(ROOT / 'tools/terrain_validation_display.py')
    for _, _, runner, _ in selected:
        sources['tools/' + runner] = sha(ROOT / 'tools' / runner)
    binaries = [normal / 'mnm-terrain-preview', sanitized / 'mnm-terrain-preview',
                normal / 'sprites/assets/mnm-region-recipe-inspect',
                sanitized / 'sprites/assets/mnm-region-recipe-inspect']
    needed_builds = []
    if any(c[3] in {'preview', 'region', 'world', 'shaded'} for c in selected):
        needed_builds += binaries[:2]
    if any(c[3] == 'region' for c in selected):
        needed_builds += binaries[2:]
    build_hashes = {str(b): sha(b) for b in needed_builds}
    if needed_builds:
        # Pin the files actually compiled, including transitive project headers.
        # Other targets may exist in compile_commands without being built.
        for build in (normal, sanitized):
            for entry in json.loads((build / 'compile_commands.json').read_text()):
                path = Path(entry['file']).resolve()
                if Path(entry['output']).is_file() and path.is_relative_to(source):
                    sources[str(path.relative_to(source))] = sha(path)
            dependencies = subprocess.check_output(['ninja', '-C', str(build), '-t', 'deps'], text=True)
            for line in dependencies.splitlines():
                path = Path(line.strip())
                if line.startswith('    ') and path.is_absolute() and path.is_relative_to(source):
                    sources[str(path.relative_to(source))] = sha(path)
        for name in ('apps/terrain-preview/CMakeLists.txt', 'assets/CMakeLists.txt',
                     'renderer/CMakeLists.txt', 'renderer/sprites/CMakeLists.txt',
                     'reconstruction/rendering/CMakeLists.txt'):
            sources[name] = sha(source / name)
    for name in ('tools/decode-cfg.py', 'tools/test-terrain-preview.py',
                 'tools/test-terrain-region.py', 'tools/test-terrain-map.py'):
        sources[name] = sha(source / name)
    report = dict(schema=1, success=False, executable_sha256=BUILD_HASH,
                  sources=sources, claims=[claims[c[0]] for c in selected],
                  tool_claims=[claims['TOOL.rendering-high-terrain-regressions']]
                  if 'TOOL.rendering-high-terrain-regressions' in claims else [],
                  frozen_source=str(source), builds=build_hashes, results=[],
                  presentation_policy='RGB565 bit replication (surface-dib-ddraw.md); '
                                      'native policy checked separately after exact original-word equality',
                  scope='Selected offline terrain contracts and software-rendered owned scenes. '
                        'No original allocation/file-error recovery, entities, whole-world '
                        'production, hardware performance or live replacement.')
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1',
               ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1',
               MNM_TERRAIN_EXECUTABLE=str(exe))

    def run_case(case):
        bid, folder, runner, mode = case
        parent = ROOT / 'working/tests' / folder
        before = set(parent.glob('run-*'))
        command = [sys.executable, '-B', str(ROOT / 'tools' / runner)]
        if mode != 'main':
            command += ['--source-root', str(source)]
        if mode in {'preview', 'region'}:
            command += ['--preview', str(binaries[0]), '--sanitized', str(binaries[1])]
        elif mode == 'shaded':
            command += ['--preview', str(binaries[0]), '--sanitized-preview', str(binaries[1])]
        elif mode == 'world':
            command += [str(binaries[0]), '--sanitized', str(binaries[1])]
        if mode == 'region':
            command += ['--inspect', str(binaries[2]), '--inspect-sanitized', str(binaries[3])]
        if mode in {'world', 'shaded'}:
            command += ['--inner']
        row = dict(behavior=bid, command=command, success=False)
        started = time.monotonic()
        with (out / (bid + '.log')).open('x') as log, ExitStack() as stack:
            try:
                job_env = dict(env)
                if mode not in {'plain', 'main'}:
                    display_env = stack.enter_context(private_display(out / (bid + '.xvfb.log')))
                    job_env['DISPLAY'] = display_env['DISPLAY']
                    job_env.pop('XAUTHORITY', None)
                    row['display'] = display_env['DISPLAY']
                completed = subprocess.run(command, cwd=ROOT, stdout=log,
                                           stderr=subprocess.STDOUT, env=job_env, timeout=2400)
                row['returncode'] = completed.returncode
            except subprocess.TimeoutExpired:
                row['error'] = 'Component exceeded its 2400-second bound'
            except (RuntimeError, OSError) as error:
                row['error'] = str(error)
        generated = sorted(set(parent.glob('run-*')) - before)
        # Each component has its own folder; ambiguous concurrent runs refuse.
        if len(generated) == 1:
            path = generated[0] / 'report.json'
            row['artifacts'] = str(generated[0].relative_to(ROOT))
            if path.is_file():
                data = json.loads(path.read_text())
                row.update(report=str(path.relative_to(ROOT)), report_sha256=sha(path))
                negative = any(data.get(k) is False for k in ('all_match', 'all_match_required', 'all_matches', 'success'))
                row['success'] = row.get('returncode') == 0 and not negative
            else:
                row.setdefault('error', 'Component did not produce its report')
        else:
            row.setdefault('error', 'Missing or ambiguous component output directory')
        row['seconds'] = round(time.monotonic() - started, 3)
        print(bid, 'PASS' if row['success'] else 'FAIL', row.get('artifacts', ''), flush=True)
        return row

    def verify(phase):
        with (out / ('original-' + phase + '.log')).open('x') as log:
            subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'],
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=120)

    verify('before')
    try:
        with ThreadPoolExecutor(max_workers=a.jobs) as pool:
            report['results'] = list(pool.map(run_case, selected))
        report['source_changes'] = [n for n, h in sources.items() if sha(ROOT / n) != h]
        report['builds_stable'] = all(sha(Path(b)) == h for b, h in build_hashes.items())
        report['sources_stable'] = not report['source_changes']
        report['success'] = (len(report['results']) == len(selected)
                             and all(r['success'] for r in report['results'])
                             and report['sources_stable'] and report['builds_stable'])
    finally:
        verify('after')
        report['original_manifest_verified_before_after'] = True
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
