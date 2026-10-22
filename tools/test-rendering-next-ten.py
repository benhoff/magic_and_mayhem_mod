#!/usr/bin/env python3
"""Execute the next ten native rendering contracts with prospective claims."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

from drawing_fixture_prefix import prepare

ROOT = Path(__file__).resolve().parents[1]
BEHAVIORS = [
    'NR.masked-displacement-admission', 'RS.raster-clipped-word-grayscale',
    'NR.native-retained-world-image', 'NR.native-prepared-world-replay',
    'NR.resident-prepared-shared-budget', 'NR.native-prepared-command-frame',
    'NP.native-gpu-copy-plan', 'NR.resident-bind-admission',
    'NR.resident-command-pressure', 'NR.world-owned-snapshot-delivery',
]
SCENARIO = 'NR.next-ten.owned-boundaries-20261020'
TARGETS = ['native-frame-recipe-test', 'masked-displacement-test',
           'native-prepared-world-test', 'native-retained-world-test',
           'world-snapshot-queue-test', 'world-gpu-ack-test',
           'canvas-sequence-test', 'canvas-world-test',
           'resident-canvas-transition-test', 'render-command-consumer-test',
           'render-resident-world-test', 'mnm-render-commands']


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--wine-prefix', type=Path, required=True)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--request', type=Path, required=True)
    parser.add_argument('--pe', type=Path, required=True)
    parser.add_argument('--game', type=Path, required=True, help='Existing read-only native asset root with input-fingerprints.json')
    parser.add_argument('--build-dir', type=Path, help='Owned incremental build; reconfigure and rebuild every selected target')
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT / 'working/tests'):
        parser.error('Require a new working/tests output directory')
    out.mkdir(parents=True)
    (out / 'tmp').mkdir()
    (out / 'runtime').mkdir(mode=0o700)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(TMPDIR=str(out / 'tmp'), XDG_RUNTIME_DIR=str(out / 'runtime'),
               QT_QPA_PLATFORM='xcb', QT_SCALE_FACTOR='1', LIBGL_ALWAYS_SOFTWARE='1',
               GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4',
               __GLX_VENDOR_LIBRARY_NAME='mesa',
               __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',
               WINEDEBUG='-all', WINEPREFIX=str(args.wine_prefix.resolve()),
               ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    for key in ('MESA_LOADER_DRIVER_OVERRIDE', 'QT_XCB_GL_INTEGRATION'):
        env.pop(key, None)
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    declaration = load('next_ten_claims', 'tools/draft-coverage-claims.py').declaration(
        ROOT, register, BEHAVIORS, [SCENARIO])
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
                  sources=declaration['sources'], inputs={}, commands=[], controls={},
                  original_game_executed=False, original_code_modified=False,
                  original_pixels_used_as_native_inputs=False, live_replacement=False,
                  performance_acceptance=False)
    claims = out / 'claims.json'
    claims.write_text(json.dumps(declaration, indent=2) + '\n')
    (out / 'candidate-sources.json').write_text(json.dumps(candidates, indent=2) + '\n')

    def run(label, command, timeout=180):
        print(label, flush=True)
        command = [str(x) for x in command]
        with (out / (label + '.log')).open('x') as log:
            result = subprocess.run(command, cwd=ROOT, env=env, stdout=log,
                                    stderr=subprocess.STDOUT, timeout=timeout)
        report['commands'].append(dict(label=label, argv=command,
            exit_code=result.returncode, log_sha256=sha(out / (label + '.log'))))
        if result.returncode:
            raise RuntimeError(label + ': exit ' + str(result.returncode))
        return (out / (label + '.log')).read_text()

    def pin(path):
        name = path.relative_to(ROOT).as_posix()
        if name not in candidates or sha(path) != candidates[name]:
            raise ValueError('Unpinned or changed compiler input: ' + name)
        report['sources'][name] = candidates[name]

    def dependencies(path):
        for token in path.read_text().replace('\\\n', ' ').split():
            item = Path(token).resolve()
            if item.is_file() and item.is_relative_to(ROOT) and not item.is_relative_to(ROOT / 'working'):
                pin(item)

    prefix_lock = None
    verified_before = False
    try:
        # Reuse a marked, exclusively locked, idle prefix; never clone an install.
        prefix, prefix_lock = prepare(ROOT, out, args.wine_prefix, args.wine_prefix)
        report['prefix'] = dict(path=str(prefix), exclusively_locked=True, stopped_before_use=True)
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        verified_before = True
        for label, supplied, expected in [
            ('snapshot-source', args.source, 'f3ec2e4525696811b5412cce8f3302cad260235b85942d8b87db7bac254d4378'),
            ('snapshot-request', args.request, '310976726f7c2b69331ed9448b945bb6a1c75e260682680265916e17d9505456'),
            ('pe', args.pe, '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168')]:
            if sha(supplied) != expected:
                raise ValueError('Input hash: ' + label)
            report['inputs'][str(supplied.resolve())] = expected
        report['original_pe_sha256'] = sha(args.pe)
        game = args.game.resolve()
        manifest = game.parent / 'input-fingerprints.json'
        report['inputs'][str(manifest)] = sha(manifest)
        # World replay resolves SPR catalogues and native bitmap producers.
        # ANI/TTD simulation inputs are not re-executed by prepared rendering.
        image_suffixes = {'.spr', '.bmp', '.pcx', '.jpg', '.jpeg'}
        asset_inputs = json.loads(manifest.read_text())
        for name, fingerprint in asset_inputs.items():
            asset = game / name
            if asset.suffix.lower() not in image_suffixes:
                continue
            if not asset.resolve().is_relative_to(game) or not asset.is_file() or sha(asset) != fingerprint:
                raise ValueError('Installed native asset changed or archived: ' + name)
            report['inputs'][str(asset)] = fingerprint
        # Catalogue traversal can consume every SPR, not only the requested
        # frame. Reject extra image files absent from the preserved manifest.
        for asset in game.rglob('*'):
            if asset.is_file() and asset.suffix.lower() in image_suffixes and str(asset) not in report['inputs']:
                raise ValueError('Unmanifested native asset: ' + str(asset))
        report['native_asset_files'] = sum(Path(p).suffix.lower() in image_suffixes for p in report['inputs'])
        build = args.build_dir.resolve() if args.build_dir else out / 'build'
        if not build.is_relative_to(ROOT / 'working/tests/rendering-next-ten-20261020'):
            raise ValueError('Build directory must belong to this bounded fixture batch')
        run('configure', ['cmake', '-S', ROOT / 'tests/native-world-rolling', '-B', build,
                          '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=ON'])
        run('build', ['cmake', '--build', build, '--parallel', '4', '--target', *TARGETS], timeout=1200)
        for path in build.rglob('*.o.d'):
            dependencies(path)
        for token in re.findall(r'"([^"\n]+)"', (build / 'CMakeFiles/Makefile.cmake').read_text()):
            path = Path(token).resolve()
            if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT / 'working'):
                pin(path)
        binaries = {name: next(p for p in build.rglob(name) if p.is_file()) for name in TARGETS}
        report['binaries'] = {name: sha(path) for name, path in binaries.items()}
        for target in TARGETS:
            if target == 'mnm-render-commands':
                continue
            command = [binaries[target]]
            if target == 'world-snapshot-queue-test':
                command += [args.source, args.request]
            elif target in ('native-prepared-world-test', 'native-retained-world-test'):
                command += [args.source, args.request, game]
            elif target == 'canvas-sequence-test':
                command += [out / 'grayscale']
            asset_trace = target in ('native-prepared-world-test', 'native-retained-world-test')
            if asset_trace:
                command = ['strace', '-f', '-s', '4096', '-e', 'trace=openat', '-o', out / (target + '-assets.trace'), *command]
            if target not in ('world-snapshot-queue-test', 'world-gpu-ack-test', 'canvas-sequence-test'):
                command = ['xvfb-run', '-a', *command]
            output = run(target, command)
            records = [json.loads(line) for line in output.splitlines() if line.startswith('{')]
            report['controls'][target] = records[-1] if records else dict(passed=True)
            if asset_trace:
                opened = set()
                for token in re.findall(r'openat\([^,]+, ("(?:\\.|[^"\\])*")', (out / (target + '-assets.trace')).read_text()):
                    path = Path(json.loads(token))
                    if path.is_absolute() and path.is_relative_to(game) and path.is_file():
                        if str(path) not in report['inputs']:
                            raise ValueError('Unpinned native asset read: ' + str(path))
                        opened.add(str(path))
                if not opened:
                    raise ValueError('Missing captured replay native asset reads')
                report['controls'][target]['verified_asset_reads'] = sorted(opened)
        for target in ('render-command-pressure-test', 'native-gpu-copy-commands-test'):
            for mode, flags in [('optimized', ['-O2']), ('sanitized', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])]:
                exe = out / (target + '-' + mode)
                run(target + '-' + mode + '-build', ['clang', '-std=c11', '-Wall', '-Wextra', '-Werror',
                    '-UNDEBUG', *flags, '-MD', '-MF', exe.with_suffix('.d'), ROOT / ('tests/' + target + '.c'), '-o', exe])
                dependencies(exe.with_suffix('.d'))
                text = run(target + '-' + mode, [exe])
                report['controls'][target + '-' + mode] = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
        reference = out / 'grayscale-reference'
        run('grayscale-reference-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
            '-fno-pie', '-no-pie', '-MD', '-MF', reference.with_suffix('.d'),
            ROOT / 'tests/clipped-word-grayscale-reference.cpp', '-o', reference])
        dependencies(reference.with_suffix('.d'))
        report['controls']['grayscale-original'] = json.loads(run('grayscale-original', [reference, args.pe, out / 'grayscale']))
        pairs = []
        for index in range(16):
            original = out / ('grayscale-case-' + str(index) + '.original.565')
            native = out / ('grayscale-case-' + str(index) + '.native.565')
            if original.read_bytes() != native.read_bytes():
                raise ValueError('Native/original grayscale differs: ' + str(index))
            pairs.append(dict(case=index, words=original.stat().st_size // 2, sha256=sha(native)))
        report['controls']['grayscale-native-original'] = dict(cases=pairs, compared_words=sum(p['words'] for p in pairs), different_words=0)
        # The GPU harness has its own narrow marker/lock in addition to the
        # outer fixture-suite lease. Add that marker only after suite admission.
        marker = prefix / '.mnm-gpu-copy-plan-owner.json'
        if not marker.exists():
            marker.write_text(json.dumps(dict(owner='native-gpu-copy-plan-fixture-v1')) + '\n')
        run('gpu-copy-plans', ['xvfb-run', '-a', sys.executable, '-B', ROOT / 'tools/test-native-gpu-copy-plan.py',
            '--claims', claims, '--report', out / 'gpu-copy-plans.json', '--output-parent', out / 'gpu-copy-runs',
            '--wine-prefix', prefix, '--renderer', binaries['mnm-render-commands']], timeout=600)
        gpu = json.loads((out / 'gpu-copy-plans.json').read_text())
        report['controls']['gpu-copy-plans'] = dict(success=gpu['success'], cases=len(gpu['cases']), command_guards=gpu['command_guards'],
            report_sha256=sha(out / 'gpu-copy-plans.json'))
        for case in ('baseline', 'tracker', 'reentrant', 'timeout'):
            text = run('bind-' + case, ['xvfb-run', '-a', sys.executable, '-B', ROOT / 'tools/test-resident-bind-admission.py',
                '--claims', claims, '--case', case, '--consume-after-exit', '--wine-prefix', prefix], timeout=240)
            result = Path(text.splitlines()[-1])
            data = json.loads(result.read_text())
            if not data['success']:
                raise ValueError('Bind schedule did not pass: ' + case)
            report['controls']['bind-' + case] = {key: data[key] for key in
                ('success', 'bind_succeeded', 'complete_shutdown', 'fifo_archive_equal', 'source_pixels_owned_before_poison',
                 'original_locks', 'original_unlocks', 'zero_identity_controls', 'post_shutdown_bind_refused')}
            report['controls']['bind-' + case]['report'] = str(result.relative_to(ROOT))
            report['controls']['bind-' + case]['report_sha256'] = sha(result)
            for name, fingerprint in {**data['compiler_manifest']['compiled_sources'],
                                      **data['compiler_manifest']['protocol_sources']}.items():
                if candidates.get(name) != fingerprint:
                    raise ValueError('Unpinned PE32 input: ' + name)
                report['sources'][name] = fingerprint
        run('owned-snapshot-cli', [sys.executable, '-B', ROOT / 'tests/test-world-owned-snapshots-cli.py'])
        report['success'] = True
    except (OSError, ValueError, RuntimeError, StopIteration, KeyError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if prefix_lock:
            for command in (['wineserver', '-k'], ['wineserver', '-w']):
                subprocess.run(command, env=env, timeout=20, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            prefix_lock.close()
        if verified_before:
            try:
                run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
                report['original_manifest_verified_before_after'] = True
            except (OSError, RuntimeError, subprocess.SubprocessError) as error:
                report.update(success=False, integrity_error=str(error))
        report['changed_sources'] = [name for name, fingerprint in report['sources'].items() if sha(ROOT / name) != fingerprint]
        report['sources_stable'] = not report['changed_sources']
        report['inputs_stable'] = all(sha(path) == fingerprint for path, fingerprint in report['inputs'].items())
        report['success'] &= report['sources_stable'] and report['inputs_stable']
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(dict(success=report['success'], report=str(out / 'report.json'), error=report.get('error'))), flush=True)
    return int(not report['success'])


if __name__ == '__main__':
    raise SystemExit(main())
