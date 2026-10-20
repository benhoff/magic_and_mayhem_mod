#!/usr/bin/env python3
"""Prospective native scene lifecycle checks; generated inputs, no game/playtest.

Use working-retention.py. Private Debug and sanitizer builds exercise the CPU
owner, actual production Qt window, bounded GL presentation and native snapshots.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shlex
import struct
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

from coverage_claims import required_sources

ROOT = Path(__file__).resolve().parents[1]
BEHAVIORS = ['WORLD.owned', 'WORLD.sandbox']
TESTS = ['native-world-lifecycle', 'native-world-wire', 'native-scene-lifecycle',
         'native-world-controls', 'native-world-stop', 'native-world-playback',
         'native-world-picking', 'native-world-spawning', 'native-world-scene']
TARGETS = ['native-world-test', 'mnm-world-sandbox', 'world-session-lifecycle-test',
           'world-scene-window-test', 'mnm-world-scene-preview', 'world-controls-test',
           'world-stop-test', 'world-playback-test', 'world-picking-test',
           'world-spawning-test', 'world-scene-test']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def sources(register):
    roots = ['apps/world-scene', 'apps/world-sandbox', 'game', 'assets', 'renderer',
             'reconstruction/pathfinding', 'reconstruction/motion',
             'reconstruction/animation', 'reconstruction/rendering', 'protocols/include']
    paths = {'tools/test-scene-lifecycle.py', 'tools/draft-coverage-claims.py',
             'tools/coverage_claims.py', 'tools/create-movement-fixture.py',
             'tools/run-scene-display.py', 'tools/terrain_validation_display.py',
             'tests/native-world-test.cpp', 'tests/test-native-world.py',
             'tests/world-session-lifecycle-test.cpp', 'tests/test-scene-lifecycle.py',
             'tests/scene-window-driver.cpp', 'tests/scene-window-driver.hpp',
             'tests/test-world-spawning.py'}
    paths.update('tests/' + name + '-test.cpp' for name in
                 ['world-controls', 'world-stop', 'world-playback', 'world-picking',
                  'world-spawning', 'world-scene'])
    for behavior in register['behaviors']:
        if behavior['id'] in BEHAVIORS:
            paths.update(required_sources(behavior))
    for directory in roots:
        paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT / directory).rglob('*')
                     if p.is_file() and (p.suffix in {'.cpp', '.hpp', '.h', '.c', '.S', '.inc', '.cmake'}
                                        or p.name == 'CMakeLists.txt'))
    pending = list(paths)
    while pending:
        name = pending.pop()
        for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"', (ROOT / name).read_text(), re.M):
            for base in [(ROOT / name).parent, ROOT / 'tests', ROOT / 'protocols/include',
                         *(ROOT / directory for directory in roots)]:
                candidate = (base / include).resolve()
                if candidate.is_file() and candidate.is_relative_to(ROOT):
                    relative = candidate.relative_to(ROOT).as_posix()
                    if relative not in paths:
                        paths.add(relative)
                        pending.append(relative)
                    break
    return {p: sha(ROOT / p) for p in sorted(paths)}


def sprite(width, height, colour):
    # Independent generated SPR v4: two direct RGB565 opaque rectangular frames.
    frames = []
    for _ in range(2):
        header = bytearray(40 + 8 * height)
        delta = bytes([0, width]) * height
        pixels = struct.pack('<H', colour) * (width * height)
        struct.pack_into('<3I2i', header, 0, len(header) + len(delta) + len(pixels),
                         width, height, width // 2, height - 1)
        struct.pack_into('<I', header, 28, 0xffffffff)
        for row in range(height):
            struct.pack_into('<2I', header, 40 + 8 * row,
                             len(header) + 2 * row,
                             len(header) + len(delta) + width * 2 * row)
        frames.append(bytes(header) + delta + pixels)
    return (b'SPR\0' + struct.pack('<5I', 32 + sum(map(len, frames)), 4, 2, 0, 0)
            + struct.pack('<2I', 0, len(frames[0])) + b''.join(frames))


def animation():
    program = [(1, 2), (0, 1), (0, 1), (5, 2), (6, -1)]
    records = program * 8
    starts = [i * len(program) for i in range(9)]
    body = b''.join(struct.pack('<Ii9I', op, value, *([0] * 9)) for op, value in records)
    return (b'ANI\0' + struct.pack('<5I', 44 + 4 * len(starts) + len(body), len(records), 5, 0, len(starts))
            + b'creature.spr\0'.ljust(20, b'\0') + struct.pack('<9I', *starts) + body)


def fnv(data, value=14695981039346656037):
    for byte in data:
        value = ((value ^ byte) * 1099511628211) & ((1 << 64) - 1)
    return value


def checkpoint_variant(data, mode=None, missing_map=False, wrong_binding=False):
    assert data[:8] == b'MNMNWLD\0' and struct.unpack_from('<I', data, 12)[0] == len(data) - 24
    assert struct.unpack_from('<Q', data, 16)[0] == fnv(data[24:])
    changed = bytearray(data)
    cursor = 24 + 20
    for index in range(3):
        size, = struct.unpack_from('<I', changed, cursor)
        if missing_map and index == 0:
            changed[cursor + 4 + size - 1] ^= 1
        cursor += 4 + size
    assert changed[cursor] == 1
    at = cursor + 1 + 12
    value, = struct.unpack_from('<Q', changed, at)
    if mode in {'stationary', 'multi'}:
        value = fnv(b'stationary-occupancy-v1', value)
    if mode == 'multi':
        value = fnv(b'multi-creature-occupancy-v1', value)
    if wrong_binding:
        value ^= 0x4000000000000000
    struct.pack_into('<Q', changed, at, value)
    struct.pack_into('<Q', changed, 16, fnv(changed[24:]))
    return changed


def pixels(frame):
    expected = [0x2124] * (512 * 256)
    owners = {}
    for draw in frame['queue']:
        width, height, colour = (7, 13, 0xf800) if draw['creature'] else (31, 15, 0x07e0)
        for row in range(height):
            for column in range(width):
                x = draw['x'] - width // 2 + column
                y = draw['y'] - (height - 1) + row
                if 0 <= x < 512 and 0 <= y < 256:
                    expected[y * 512 + x] = colour
                    owners[(x, y)] = draw.get('slot') if draw['creature'] else None
    return struct.pack('<' + 'H' * len(expected), *expected), owners


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    parent = ROOT / 'working/tests/scene-lifecycle'
    parent.mkdir(parents=True, exist_ok=True)
    out = args.output or Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    if args.output:
        out.mkdir(parents=True, exist_ok=False)
    print(out, flush=True)
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    declaration = module('scene_claims', ROOT / 'tools/draft-coverage-claims.py').declaration(
        ROOT, register, BEHAVIORS, ['scene.owned-lifecycle-preview'])
    declaration['sources'].update(sources(register))
    (out / 'declaration.json').write_text(json.dumps(declaration, indent=2) + '\n')
    report = dict(schema=1, success=False, sources=declaration['sources'], claims=declaration['claims'],
                  scope='Native scene lifetime/Qt controls and production-window startup, shutdown and fresh-window continuation on generated frozen/SPR/TTD/ANI inputs. No original game, installed assets, original comparison or live replacement.',
                  original_execution=False, game_assets_read=False, playtesting=False,
                  commands=[], windows=[], refusals=[])
    env = dict(os.environ, QT_QPA_PLATFORM='offscreen', LIBGL_ALWAYS_SOFTWARE='1',
               ASAN_OPTIONS='detect_leaks=0:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1',
               MNM_SCENE_LIFECYCLE_EVIDENCE=str(out / 'lifecycle'),
               MNM_SCENE_DISPLAY_EVIDENCE=str(out / 'displays'))

    def run(label, arguments, expected=0, timeout=240, display=False):
        arguments = list(map(str, arguments))
        if display:
            arguments = [sys.executable, str(ROOT / 'tools/run-scene-display.py'), *arguments]
        with (out / (label + '.stdout')).open('x') as stdout, (out / (label + '.stderr')).open('x') as stderr:
            result = subprocess.run(arguments, cwd=ROOT, env=env, stdout=stdout, stderr=stderr, timeout=timeout)
        report['commands'].append(dict(label=label, argv=arguments, exit_code=result.returncode, expected=expected))
        error = (out / (label + '.stderr')).read_text()
        if result.returncode != expected or 'AddressSanitizer' in error or 'runtime error:' in error:
            raise RuntimeError(f'{label}: exit {result.returncode}; see {out / (label + ".stderr")}')
        return (out / (label + '.stdout')).read_text()

    def closure(build):
        result = set()
        for path in build.rglob('*.o.d'):
            for token in shlex.split(path.read_text().replace('\\\n', ' '))[1:]:
                candidate = Path(token)
                if not candidate.is_absolute():
                    candidate = build / candidate
                candidate = candidate.resolve()
                if candidate.is_relative_to(ROOT) and candidate.is_file():
                    name = candidate.relative_to(ROOT).as_posix()
                    if not name.startswith('working/'):
                        result.add(name)
        missing = result - set(declaration['sources'])
        if not result or missing:
            raise RuntimeError('Undeclared compiler dependencies: ' + repr(sorted(missing)))
        return sorted(result)

    try:
        build = out / 'build'
        run('configure', ['cmake', '-S', ROOT / 'apps/world-scene', '-B', build, '-DCMAKE_BUILD_TYPE=Debug',
                          '-DXVFB_RUN=' + str(ROOT / 'tools/run-scene-display.py')])
        run('build', ['cmake', '--build', build, '--target', *TARGETS, '-j4'], timeout=600)
        report['compiler_dependencies'] = closure(build)
        run('ctest', ['ctest', '--test-dir', build, '--output-on-failure', '--no-tests=error',
                      '-R', '^(' + '|'.join(TESTS) + ')$', '--output-junit', out / 'ctest.xml'])
        cases = ET.parse(out / 'ctest.xml').findall('.//testcase')
        assert sorted(c.attrib['name'] for c in cases) == sorted(TESTS)
        assert not any(c.find('failure') is not None or c.find('skipped') is not None for c in cases)
        report['ctests'] = TESTS
        print('Native ownership, controls, playback, spawning and renderer controls passed', flush=True)
        fixture = out / 'inputs'
        fixture.mkdir()
        (fixture / 'Terrain.spr').write_bytes(sprite(31, 15, 0x07e0))
        (fixture / 'creature.spr').write_bytes(sprite(7, 13, 0xf800))
        (fixture / 'movement.ani').write_bytes(animation())
        catalog = bytearray(16 + 356 * 2)
        struct.pack_into('<4I', catalog, 0, 0x00445454, len(catalog), 4, 2)
        struct.pack_into('<4I', catalog, 16 + 356 + 0x84, 1, 1, 1, 1)
        (fixture / 'Terrain.ttd').write_bytes(catalog)
        (fixture / 'map.bin').write_bytes(module('scene_fixture', ROOT / 'tools/create-movement-fixture.py').fixture())
        sandbox = build / 'world/mnm-world-sandbox'
        initial = fixture / 'initial.mnw'
        run('create-scene', [sandbox, 'move-terrain-ani', fixture / 'map.bin', fixture / 'movement.ani',
                             0, initial, 1, 1, 1, 5, 1, 1, 2])
        for mode in ['stationary', 'multi']:
            (fixture / (mode + '.mnw')).write_bytes(checkpoint_variant(initial.read_bytes(), mode))
        report['inputs'] = {p.relative_to(out).as_posix(): sha(p) for p in fixture.iterdir()}
        preview, window = build / 'mnm-world-scene-preview', build / 'world-scene-window-test'
        checks = 0
        for view, mode in enumerate(['base', 'stationary', 'multi', 'base']):
            checkpoint = initial if mode == 'base' else fixture / (mode + '.mnw')
            options = ['--root', fixture, '--realm', '.', '--sprite', 'creature.spr',
                       '--region', '0,0,1,4,3', '--definition', 1, '--view', view]
            prefix = out / f'v{view}-reference'
            run(f'reference-{view}', [preview, '--checkpoint', checkpoint, *options, '--output', prefix], display=True)
            frame = json.loads(Path(str(prefix) + '.json').read_text())['frames'][0]
            expected, owners = pixels(frame)
            assert Path(str(prefix) + '-000.565').read_bytes() == expected
            candidates = [point for point, actor in owners.items() if actor == 0]
            assert candidates, 'Generated actor must be pickable'
            x, y = candidates[len(candidates) // 2]
            script = [dict(op='capture', name='initial'), dict(op='mouse', button='left', x=x, y=y),
                      dict(op='capture', name='selected'), dict(op='button', button='queueStop'),
                      dict(op='capture', name='pending'), dict(op='save', name='saved-pending'),
                      dict(op='ticks', count=1), dict(op='capture', name='stopped'), dict(op='close', play=True)]
            script_path = out / f'v{view}-script.json'
            script_path.write_text(json.dumps(script, indent=2) + '\n')
            whole = out / f'v{view}-whole'
            run(f'window-{view}', [window, '--checkpoint', checkpoint, *options,
                                   '--window-script', script_path, '--window-output', whole], display=True)
            summary = json.loads((whole / 'window-report.json').read_text())
            assert summary['all_match'] and summary['closing_while_playing']
            assert summary['captures'][0]['selected_index'] == 0 and summary['captures'][1]['selected_index'] == 1
            assert len(summary['captures'][2]['pending']) == 1 and not summary['captures'][3]['pending']
            assert (whole / 'saved-pending.mnms').read_bytes() == (whole / 'pending.mnw').read_bytes()
            for name in ['initial', 'selected', 'pending', 'stopped']:
                rendered = json.loads((whole / (name + '.json')).read_text())
                assert (whole / (name + '.565')).read_bytes() == pixels(rendered)[0]
                checks += 1
            resumed_script = out / f'v{view}-resumed-script.json'
            resumed_script.write_text(json.dumps([dict(op='capture', name='loaded'), dict(op='ticks', count=1),
                                                   dict(op='capture', name='stopped'), dict(op='close')]) + '\n')
            resumed = out / f'v{view}-resumed'
            run(f'window-resumed-{view}', [window, '--checkpoint', whole / 'saved-pending.mnms', *options,
                                          '--window-script', resumed_script, '--window-output', resumed], display=True)
            resumed_summary = json.loads((resumed / 'window-report.json').read_text())
            assert resumed_summary['all_match'] and not resumed_summary['closing_while_playing']
            assert resumed_summary['captures'][0]['selected_index'] == 0
            for current, old in [('loaded', 'pending'), ('stopped', 'stopped')]:
                for suffix in ['mnw', 'json', '565', 'png']:
                    assert (resumed / (current + '.' + suffix)).read_bytes() == (whole / (old + '.' + suffix)).read_bytes()
                checks += 1
            for folder in [whole, resumed]:
                lifecycle = json.loads((folder / 'lifecycle.json').read_text())
                assert lifecycle == dict(closed_on_application_quit=True, playback_retired=True,
                                          playing_after_quit=False, selection_cleared=True, placement_cleared=True)
                report['windows'].append(dict(view=view, mode=mode, output=str(folder.relative_to(out)), lifecycle=lifecycle))
        report['pixel_comparisons'] = checks + 4
        report['fresh_window_continuations'] = 4
        print('Eight production-entry-point windows closed cleanly; four fresh-window continuations match', flush=True)
        negative_inputs = dict(corrupt=initial.read_bytes()[:-1] + bytes([initial.read_bytes()[-1] ^ 1]),
                               missing_map=checkpoint_variant(initial.read_bytes(), missing_map=True),
                               wrong_binding=checkpoint_variant(initial.read_bytes(), wrong_binding=True))
        for name, data in negative_inputs.items():
            path = out / (name + '.mnw')
            path.write_bytes(data)
            prefix = out / ('refused-' + name)
            run(name, [preview, '--checkpoint', path, '--root', fixture, '--output', prefix], expected=1, display=True)
            assert {'corrupt': 'snapshot checksum mismatch', 'missing_map': 'cannot open frozen movement map',
                    'wrong_binding': 'movement map identity mismatch'}[name] in (out / (name + '.stderr')).read_text().lower()
            assert not list(out.glob('refused-' + name + '*'))
            report['refusals'].append(name)
        sanitized = out / 'sanitized'
        run('sanitize-configure', ['cmake', '-S', ROOT / 'apps/world-scene', '-B', sanitized,
                                   '-DCMAKE_BUILD_TYPE=Debug',
                                   '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie',
                                   '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])
        run('sanitize-build', ['cmake', '--build', sanitized, '--target', 'world-session-lifecycle-test', '-j4'], timeout=300)
        report['sanitizer_compiler_dependencies'] = closure(sanitized)
        run('sanitizers', [sys.executable, ROOT / 'tests/test-scene-lifecycle.py', sanitized / 'world-session-lifecycle-test'])
        report['sanitizers_passed'] = True
        report['leak_sanitizer'] = 'Disabled under tracing; no leak-sanitizer claim.'
        report['inputs_stable'] = all(sha(out / path) == digest for path, digest in report['inputs'].items())
        assert report['inputs_stable']
        report['binaries'] = {str(p.relative_to(out)): sha(p) for base in [build, sanitized]
                              for p in base.rglob('*') if p.is_file() and not p.suffix and os.access(p, os.X_OK)}
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
        raise
    finally:
        report['changed_sources'] = [p for p, digest in declaration['sources'].items() if sha(ROOT / p) != digest]
        report['sources_stable'] = not report['changed_sources']
        report['success'] = report['success'] and report['sources_stable']
        report['artifacts'] = {p.relative_to(out).as_posix(): sha(p) for p in out.rglob('*')
                               if p.is_file() and p.relative_to(out).parts[0] not in {'build', 'sanitized'}
                               and p.name != 'report.json'}
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print('Report: ' + str(out / 'report.json'), flush=True)
    if not report['success']:
        raise RuntimeError('Scene lifecycle sources changed during execution')


if __name__ == '__main__':
    main()
