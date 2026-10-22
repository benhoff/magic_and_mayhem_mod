#!/usr/bin/env python3
"""Execute the ten bounded native rendering policies with prospective claims."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

from drawing_fixture_prefix import prepare

ROOT = Path(__file__).resolve().parents[1]
BEHAVIORS = [
    'NP.surface-final-word-read', 'NR.native-draw-status-session-file',
    'NR.native-status-outside-drawing', 'NR.minimap-continuous-read-window',
    'NR.world-source-region-epoch', 'NR.readonly-description-continuity',
    'NR.masked-update-scratch-reuse', 'NR.native-colour-rectangle-cache',
    'NR.native-palette-row-cache', 'NR.displacement-finite-edge',
]
SCENARIO = 'NR.ten-closeouts.owned-controls-20261020'
TARGETS = ['native-frame-recipe-test', 'colour-rectangle-cache-test',
           'palette-row-cache-test', 'masked-displacement-test',
           'world-continuous-memory-test', 'world-source-regions-test',
           'scene-renderer-test', 'scene-upload-test', 'sprite-atlas-test',
           'live-render-channel-test']
MUTATIONS = ['cross-desc', 'desc-timeout', 'desc-tracker', 'desc-gate',
             'meta-change', 'copy-timeout', 'lock-timeout', 'status-flush']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(name, relative):
    spec = importlib.util.spec_from_file_location(name, ROOT / relative)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--prepared-run', type=Path,
                        help='Verify and reuse compiled inputs/binaries; every control executes again')
    parser.add_argument('--wine-prefix', type=Path, required=True,
                        help='Idle, marked drawing-fixture-prefix-suite prefix; exclusively locked')
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT / 'working/tests'):
        parser.error('Require a new output directory under working/tests')
    out.mkdir(parents=True)
    (out / 'tmp').mkdir()
    (out / 'qt-runtime').mkdir(mode=0o700)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    for key in ('MESA_LOADER_DRIVER_OVERRIDE', '__GLX_VENDOR_LIBRARY_NAME',
                '__EGL_VENDOR_LIBRARY_FILENAMES', 'QT_XCB_GL_INTEGRATION'):
        env.pop(key, None)
    env.update(TMPDIR=str(out / 'tmp'), XDG_RUNTIME_DIR=str(out / 'qt-runtime'),
               QT_FORCE_STDERR_LOGGING='1', QT_QPA_PLATFORM='xcb',
               LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4',
               __GLX_VENDOR_LIBRARY_NAME='mesa',
               __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',
               WINEDEBUG='-all', WINEPREFIX=str(args.wine_prefix.resolve()),
               ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    declaration = load('ten_claims', 'tools/draft-coverage-claims.py').declaration(
        ROOT, register, BEHAVIORS, [SCENARIO])
    # Record candidate bytes before compiling, then retain the exact dependency
    # closure. No full source/install snapshot or historical evidence rewrite.
    # Python report reads resolve SQLite payloads; bind the actual reader too.
    declaration['sources']['tools/coverage_evidence.py'] = sha(ROOT / 'tools/coverage_evidence.py')
    candidates = dict(declaration['sources'])
    for directory in ('apps', 'assets', 'audio', 'compat', 'reconstruction',
                      'renderer', 'runtime', 'protocols', 'tests', 'tools'):
        for path in (ROOT / directory).rglob('*'):
            if path.is_file() and (path.suffix in {'.c', '.cpp', '.h', '.hpp', '.S', '.inc', '.cmake', '.py', '.json'}
                                   or path.name == 'CMakeLists.txt'):
                candidates.setdefault(path.relative_to(ROOT).as_posix(), sha(path))
    report = dict(schema=1, success=False, scope=__doc__, claims=declaration['claims'],
                  sources=declaration['sources'], commands=[], controls={},
                  original_artifacts_consumed=False, original_game_executed=False,
                  live_replacement=False, performance_acceptance=False,
                  reviewed_head=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip())
    (out / 'claims.json').write_text(json.dumps(declaration, indent=2) + '\n')

    def run(label, command, *, timeout=180, extra=None, cwd=ROOT, expected=0):
        command = [str(v) for v in command]
        if command[0] == 'xvfb-run':
            command[1:1] = ['-e', str(out / (label + '-xserver.log'))]
        print(label, flush=True)
        with (out / (label + '.log')).open('x') as log:
            result = subprocess.run(command, cwd=cwd, env={**env, **(extra or {})},
                                    stdout=log, stderr=subprocess.STDOUT, timeout=timeout)
        report['commands'].append(dict(label=label, argv=command, exit_code=result.returncode,
                                       log_sha256=sha(out / (label + '.log'))))
        if result.returncode != expected:
            raise RuntimeError(label + ' failed: ' + str(result.returncode))
        return (out / (label + '.log')).read_text()

    def compiler_inputs(dependency):
        for token in dependency.read_text().replace('\\\n', ' ').split():
            path = Path(token).resolve()
            if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT / 'working'):
                name = path.relative_to(ROOT).as_posix()
                if name not in candidates or sha(path) != candidates[name]:
                    raise ValueError('Unpinned or changed compiler input: ' + name)
                report['sources'][name] = candidates[name]

    prefix_lock = None
    previous = None
    try:
        prefix, prefix_lock = prepare(ROOT, out, args.wine_prefix, args.wine_prefix)
        report['prefix'] = dict(path=str(prefix), exclusively_locked=True, stopped_before_use=True)
        build = out / 'build'
        if args.prepared_run:
            prepared = args.prepared_run.resolve()
            if not prepared.is_relative_to(ROOT / 'working/tests'):
                raise ValueError('Prepared run must be an owned test output')
            previous = json.loads((prepared / 'report.json').read_text())
            build = Path(previous.get('build_path', prepared / 'build'))
            report['prepared_run'] = dict(path=str(prepared), sha256=sha(prepared / 'report.json'),
                                           prior_success=previous['success'])
        else:
            run('configure', ['cmake', '-S', ROOT / 'tests/native-world-rolling', '-B', build,
                              '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=ON'])
            run('build', ['cmake', '--build', build, '--parallel', '4', '--target', *TARGETS], timeout=900)
        for dependency in build.rglob('*.o.d'):
            compiler_inputs(dependency)
        for token in re.findall(r'"([^"\n]+)"', (build / 'CMakeFiles/Makefile.cmake').read_text()):
            path = Path(token).resolve()
            if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT / 'working'):
                name = path.relative_to(ROOT).as_posix()
                if name not in candidates or sha(path) != candidates[name]:
                    raise ValueError('Unpinned build input: ' + name)
                report['sources'][name] = candidates[name]
        binaries = {target: next(p for p in build.rglob(target) if p.is_file()) for target in TARGETS}
        if previous:
            for target, binary in binaries.items():
                if previous['binaries'][target] != sha(binary):
                    raise ValueError('Prepared binary changed: ' + target)
            for name in set(report['sources']) - set(declaration['sources']):
                if previous['sources'].get(name) != report['sources'][name]:
                    raise ValueError('Prepared compiler/CMake input changed: ' + name)
            # Declared inputs that also appeared in compiler depfiles must match
            # too; only the orchestration script itself may have been repaired.
            for dependency in build.rglob('*.o.d'):
                for token in dependency.read_text().replace('\\\n', ' ').split():
                    path = Path(token).resolve()
                    if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT / 'working'):
                        name = path.relative_to(ROOT).as_posix()
                        if previous['sources'].get(name) != candidates[name]:
                            raise ValueError('Prepared compiler input changed: ' + name)
        report['build_path'] = str(build)
        report['binaries'] = {k: sha(v) for k, v in binaries.items()}
        for target in TARGETS:
            if target == 'live-render-channel-test':
                continue  # exercised by the actual PE32 schedules below
            command = [binaries[target]]
            if target not in ('world-continuous-memory-test', 'world-source-regions-test'):
                command = ['xvfb-run', '-a', *command]
            text = run(target, command)
            rows = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
            report['controls'][target] = rows[-1] if rows else dict(passed=True)
        text = run('palette-baseline', ['xvfb-run', '-a', binaries['palette-row-cache-test']],
                   extra={'MNM_PALETTE_ROW_CACHE_BASELINE': '1'})
        report['controls']['palette-baseline'] = json.loads(text.splitlines()[-1])
        for name in ('native-draw-status-test', 'world-continuous-memory-test', 'world-source-regions-test'):
            exe = out / (name + '-sanitized')
            run(name + '-sanitized-build', ['clang++', '-std=c++17', '-O1', '-g', '-UNDEBUG',
                '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined',
                '-fno-omit-frame-pointer', '-MD', '-MF', exe.with_suffix('.d'), ROOT / ('tests/' + name + '.cpp'), '-o', exe])
            compiler_inputs(exe.with_suffix('.d'))
            run(name + '-sanitized', [exe])
        status = out / 'status-optimized'
        run('status-optimized-build', ['clang++', '-std=c++17', '-O2', '-UNDEBUG',
            '-Wall', '-Wextra', '-Werror', ROOT / 'tests/native-draw-status-test.cpp', '-o', status])
        run('status-optimized', [status])
        # Actual native facade allocation: no original DLL/image is loaded.
        builder = load('ten_native_builder', 'tools/build-native-draw.py')
        with (out / 'native-build.log').open('x') as log:
            result = subprocess.run([sys.executable, '-c',
                'import importlib.util,sys; s=importlib.util.spec_from_file_location("builder",sys.argv[1]); '
                'm=importlib.util.module_from_spec(s); s.loader.exec_module(m); m.build(sys.argv[2])',
                ROOT / 'tools/build-native-draw.py', out / 'native-build'], env=env, cwd=ROOT,
                stdout=log, stderr=subprocess.STDOUT, timeout=180)
        if result.returncode:
            raise RuntimeError('Native DLL build failed')
        manifest = json.loads((out / 'native-build/manifest.json').read_text())
        for name, digest in manifest['sources'].items():
            if candidates.get(name) != digest:
                raise ValueError('Changed native DLL input: ' + name)
            report['sources'][name] = digest
        shutil.copyfile(out / 'native-build/MnmDraw.dll', out / 'MnmDraw.dll')
        definition = out / 'user32.def'
        definition.write_text('LIBRARY USER32.dll\nEXPORTS\nGetDesktopWindow@0\n')
        run('user-imports', ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', out / 'user32.lib'])
        imports = {**builder.IMPORTS['kernel32'], 'LoadLibraryA': 4, 'GetProcAddress': 8}
        definition = out / 'probe-kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{name}@{size}\n' for name, size in imports.items()))
        run('probe-imports', ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', out / 'probe-kernel32.lib'])
        probe = out / 'surface-probe.obj'
        run('surface-build', [*builder.FLAGS, '-MD', '-MF', out / 'surface.d', '-c',
                             ROOT / 'tests/native-draw-surface-probe.c', '-o', probe])
        compiler_inputs(out / 'surface.d')
        run('surface-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console',
                            '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:' + str(out / 'surface.exe'),
                            probe, out / 'probe-kernel32.lib', out / 'user32.lib'])
        run('surface-policy', ['xvfb-run', '-a', 'wine', out / 'surface.exe'],
            extra={'MNM_FACTORY_NATIVE': '1', 'WINEDLLOVERRIDES': 'ddraw=',
                   'MNM_NATIVE_DRAW_STATUS': 'Z:' + str(out / 'native-status.bin').replace('/', '\\')}, cwd=out)
        policy = struct.unpack('<7I', (out / 'surface-policy.bin').read_bytes())
        if policy != (15, 117, 512, 0, 4, 367200, 12):
            raise ValueError('Incomplete surface allocation/refusal/tail policy')
        report['controls']['surface-policy'] = dict(final_pixel_dword_cases=12,
            refused_outside_row_locks=12, invalid_size_refusals=5, remaining_surfaces=0,
            native_ddraw_disabled=True, policy_sha256=sha(out / 'surface-policy.bin'))
        status_words = struct.unpack('<24I', (out / 'native-status.bin').read_bytes())
        if status_words[:5] != (0x444d4e4d, 0x31305752, 2, 96, 0) or any(status_words[5:9]) or status_words[15] != 512:
            raise ValueError('Real mapped status did not retire resources or retain its peak')
        run('status-startup-refusal', ['xvfb-run', '-a', 'wine', out / 'surface.exe'],
            extra={'MNM_FACTORY_NATIVE': '1', 'WINEDLLOVERRIDES': 'ddraw=',
                   'MNM_NATIVE_DRAW_STATUS': 'Z:' + str(out / 'absent/status.bin').replace('/', '\\')},
            cwd=out, expected=100)
        report['controls']['mapped-status'] = dict(bytes=96, remaining_resources=0,
            peak_surfaces=512, snapshots=status_words[14], startup_failure_exit=100,
            sha256=sha(out / 'native-status.bin'))
        # The existing runner retains original-once/LastError and independent
        # full GPU pixels, plus genuine mutation/refusal counterexamples.
        text = run('pe32-schedules', ['xvfb-run', '-a', sys.executable, '-B', ROOT / 'tools/test-render-mutations.py',
            binaries['live-render-channel-test'].parent, '--wine-prefix', prefix,
            '--claims', out / 'claims.json', *[v for case in MUTATIONS for v in ('--case', case)]], timeout=600)
        nested_path = Path(text.strip().splitlines()[-1])
        nested = json.loads(nested_path.read_text())
        if not nested['success'] or {c['mode'] for c in nested['cases']} != set(MUTATIONS):
            raise ValueError('Incomplete PE32 schedules')
        for name, digest in nested['sources'].items():
            if candidates.get(name) != digest:
                raise ValueError('Unpinned nested runner input: ' + name)
            report['sources'][name] = digest
        report['controls']['pe32-schedules'] = dict(path=str(nested_path), sha256=sha(nested_path),
            full_frame_comparisons=nested['full_frame_comparisons'],
            valid_sessions=nested['valid_sessions'], refused_sessions=nested['refused_sessions'],
            cases=[c['mode'] for c in nested['cases']])
        report['success'] = True
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if prefix_lock is not None:
            subprocess.run(['wineserver', '-k'], env=env, timeout=15, check=False,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            subprocess.run(['wineserver', '-w'], env=env, timeout=15, check=False,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            prefix_lock.close()
        report['changed_sources'] = [p for p, digest in report['sources'].items() if sha(ROOT / p) != digest]
        report['sources_stable'] = not report['changed_sources']
        report['success'] &= report['sources_stable']
        # Compare only actual provenance with immutable Git blobs. A worktree
        # clean-filter scan can recurse through report readers and stall IPC.
        report['unique_sources'] = {}
        objects = subprocess.Popen(['git', 'cat-file', '--batch'], cwd=ROOT,
                                   stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        try:
            for name, fingerprint in sorted(report['sources'].items()):
                objects.stdin.write(('HEAD:' + name + '\n').encode())
                objects.stdin.flush()
                header = objects.stdout.readline().decode().strip().split()
                original = None
                if len(header) == 3 and header[1] == 'blob':
                    original = objects.stdout.read(int(header[2]))
                    if objects.stdout.read(1) != b'\n':
                        raise ValueError('Incomplete Git source response')
                elif not header or header[-1] != 'missing':
                    raise ValueError('Unexpected Git source response')
                if name not in report['changed_sources'] and (
                        original is None or hashlib.sha256(original).hexdigest() != fingerprint):
                    target = out / 'unique-sources' / name
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(ROOT / name, target)
                    report['unique_sources'][name] = sha(target)
            objects.stdin.close()
            if objects.wait(timeout=15):
                raise ValueError('Git source lookup failed')
        finally:
            if objects.poll() is None:
                objects.kill()
                objects.wait()
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(out / 'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
