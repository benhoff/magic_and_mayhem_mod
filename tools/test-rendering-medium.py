#!/usr/bin/env python3
"""Run frozen native rendering controls; report Medium acceptance gaps separately."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
TARGETS = [
    'palette-row-cache-test', 'native-frame-recipe-test',
    'masked-displacement-test', 'native-prepared-world-test',
    'native-retained-world-test', 'world-gpu-resident-test',
    'world-gpu-ack-test', 'world-continuous-memory-test',
    'world-continuous-channel-test', 'world-continuous-hash-test',
    'world-continuous-normal-test', 'canvas-sequence-test',
    'canvas-world-test', 'other-writer-atomic-test', 'world-raster-batch-test',
    'minimap-producer-test', 'colour-rectangle-cache-test',
    'scene-renderer-test', 'scene-upload-test', 'scene-history-test',
    'sprite-atlas-test', 'resource-cache-test', 'render-command-consumer-test',
    'render-resident-world-test', 'render-copy-diagnostics-test',
    'resident-canvas-transition-test',
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--pe', type=Path, help='Pinned unchanged No-CD image for isolated leaf comparisons')
    parser.add_argument('--displacement-input', type=Path, help='Captured SPR frame plus row offsets; requires --pe')
    parser.add_argument('--bottom-edge-input', type=Path, help='Separate captured bottom-edge geometry; requires --pe')
    parser.add_argument('--snapshot-source', type=Path, help='Captured owned World source; requires --pe and --snapshot-request')
    parser.add_argument('--snapshot-request', type=Path)
    parser.add_argument('--prepared', type=Path, help='Reuse immutable compiled inputs/binaries; every control and original comparison executes again, including earlier failures')
    args = parser.parse_args()
    out = args.output.resolve()
    if not out.is_relative_to(ROOT / 'working') or out.exists():
        parser.error('Require a new output directory under working/')
    if not 1 <= args.jobs <= 8:
        parser.error('Use 1..8 build jobs')
    if bool(args.pe) != bool(args.displacement_input):
        parser.error('Supply both --pe and --displacement-input for original comparisons')
    if bool(args.snapshot_source) != bool(args.snapshot_request) or (args.snapshot_source and not args.pe):
        parser.error('Snapshot controls require both owned inputs and --pe for manifest bookends')
    if args.bottom_edge_input and not args.pe:
        parser.error('Bottom-edge comparison requires --pe')
    out.mkdir(parents=True)
    frozen, build = out / 'source', out / 'build'
    frozen.mkdir()
    report = dict(schema=1, success=False, scope=__doc__, controls=[],
                  original_artifacts_consumed=False, live_gameplay_executed=False,
                  performance_acceptance_executed=False, status_promotions=False)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    for key in ('MESA_LOADER_DRIVER_OVERRIDE', '__GLX_VENDOR_LIBRARY_NAME',
                '__EGL_VENDOR_LIBRARY_FILENAMES', 'QT_XCB_GL_INTEGRATION'):
        env.pop(key, None)
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1',
               GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4', QT_SCALE_FACTOR='1')
    (out / 'tmp').mkdir()
    env['TMPDIR'] = str(out / 'tmp')

    def run(label, command):
        with (out / (label + '.log')).open('x') as log:
            result = subprocess.run([str(x) for x in command], cwd=frozen,
                                    env=env, stdout=log, stderr=subprocess.STDOUT,
                                    timeout=900 if label in ('build', 'registered-ctests') else 120)
        return result.returncode

    try:
        names = subprocess.check_output(['git', 'ls-files', '-z'], cwd=ROOT).decode().split('\0')
        for name in names + ['tools/test-rendering-medium.py']:
            path = Path(name)
            if not name or path.parts[0] not in {
                    'apps', 'assets', 'audio', 'compat', 'reconstruction',
                    'renderer', 'runtime', 'protocols', 'tests', 'tools'}:
                continue
            if path.suffix not in {'.c', '.cpp', '.h', '.hpp', '.S', '.inc',
                                   '.cmake', '.json', '.py'} and path.name != 'CMakeLists.txt':
                continue
            target = frozen / path
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / path, target)
        register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
        importance = json.loads((ROOT / 'apps/coverage-ui/importance.json').read_text())
        medium = {e['behavior'] for e in importance['entries'] if e['weight'] == 3}
        report['medium_findings'] = [dict(id=b['id'], status=b['status'],
                                         remaining_gaps=b.get('remaining_gaps', []))
                                     for b in register['behaviors'] if b['id'] in medium]
        require(len(report['medium_findings']) == len(medium), 'Missing Medium behavior')
        spec = importlib.util.spec_from_file_location('medium_claims', ROOT / 'tools/draft-coverage-claims.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        behaviors = ['NR.masked-displacement-admission', 'NR.displacement-finite-edge']
        if args.pe:
            behaviors += ['RS.displacement-linear-stride', 'RS.raster-clipped-word-grayscale']
        if args.snapshot_source:
            behaviors += ['NR.world-owned-snapshot-delivery']
        declaration = module.declaration(ROOT, register, behaviors, [])
        report.update(claims=declaration['claims'], sources=declaration['sources'])
        # Freeze declaration-only research inputs as well; compiled inputs below
        # must independently be present in the recorded source closure.
        for name in declaration['sources']:
            target = frozen / name
            if not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / name, target)
            require(sha(target) == declaration['sources'][name], name)
        report['frozen_sources'] = {p.relative_to(frozen).as_posix(): sha(p)
                                    for p in sorted(frozen.rglob('*')) if p.is_file()}
        if args.prepared:
            prepared = args.prepared.resolve()
            require(prepared.is_relative_to(ROOT / 'working'), 'Use an owned prepared run')
            previous = json.loads((prepared / 'report.json').read_text())
            completed = {c['target'] for c in previous['controls']}
            require(set(TARGETS) <= completed and previous['frozen_sources_stable'] and
                    previous.get('compiled_sources'), 'Prepared build provenance is incomplete')
            for name, fingerprint in previous['compiled_sources'].items():
                require(sha(prepared / 'source' / name) == fingerprint == sha(frozen / name), name)
            for name, fingerprint in previous['frozen_sources'].items():
                if Path(name).name == 'CMakeLists.txt' or Path(name).suffix == '.cmake':
                    require(sha(prepared / 'source' / name) == fingerprint == sha(frozen / name), name)
            build = Path(previous.get('build_path', prepared / 'build'))
            for control in previous['controls']:
                if control['target'] not in TARGETS:
                    continue
                binary = next(build.rglob(control['target']))
                require(sha(binary) == control['executable_sha256'], 'Prepared binary changed')
            report['prepared_run'] = dict(path=str(prepared), report_sha256=sha(prepared / 'report.json'),
                aggregate_success=previous['success'], scope='Only verified compiled native inputs/binaries are reused. All controls and original comparisons execute again; no inherited acceptance.')
        for label, command in ([] if args.prepared else [
            ('configure', ['cmake', '-S', frozen / 'apps/qt-shell', '-B', build,
                           '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=ON']),
            ('build', ['cmake', '--build', build, '--target', *TARGETS,
                       *(['displacement-linear-test'] if args.pe else []),
                       *(['world-snapshot-queue-test'] if args.snapshot_source else []),
                       '--parallel', args.jobs])]):
            if run(label, command):
                raise RuntimeError(label + ' failed; inspect its log')
        compiled = set(previous['compiled_sources']) if args.prepared else set()
        for dependency in ([] if args.prepared else build.rglob('*.o.d')):
            for token in dependency.read_text().replace('\\\n', ' ').split():
                path = Path(token).resolve()
                if path.is_file() and path.is_relative_to(frozen):
                    compiled.add(path.relative_to(frozen).as_posix())
        require(compiled, 'Missing compiler dependency closure')
        report['compiled_sources'] = {n: sha(frozen / n) for n in sorted(compiled)}
        report['build_path'] = str(build)
        require(not compiled - report['frozen_sources'].keys(), 'Undeclared compiler inputs')
        # Project the pre-execution freeze onto the actual compiler and CMake
        # closure. Unrelated frozen files remain inventory, not execution proof.
        execution_inputs = set(compiled)
        makefile = build / 'CMakeFiles/Makefile.cmake'
        dependency_root = (prepared / 'source') if args.prepared else frozen
        if makefile.is_file():
            import re
            for token in re.findall(r'"([^"\n]+)"', makefile.read_text()):
                path = Path(token).resolve()
                if path.is_file() and path.is_relative_to(dependency_root):
                    execution_inputs.add(path.relative_to(dependency_root).as_posix())
        # Stubbed status and isolated reference translation units are compiled
        # below, outside CMake. Their complete local include closure is explicit.
        execution_inputs.update(['tests/native-draw-status-test.cpp', 'runtime/native-draw/status.h'])
        if args.pe:
            execution_inputs.update(['tests/displacement-linear-reference.cpp',
                'tests/clipped-word-grayscale-reference.cpp', 'tests/sprite-binary-reference.cpp'])
        for name in execution_inputs:
            require(name in report['frozen_sources'], 'Unfrozen execution input: ' + name)
            report['sources'][name] = report['frozen_sources'][name]
        for target in TARGETS:
            binaries = [p for p in build.rglob(target) if p.is_file() and os.access(p, os.X_OK)]
            require(len(binaries) == 1, target)
            binary = binaries[0]
            cpu_only = target in {'world-continuous-memory-test', 'world-gpu-ack-test',
                                  'world-continuous-channel-test', 'world-continuous-hash-test'}
            code = run(target, [binary] if cpu_only else ['xvfb-run', '-a', binary])
            report['controls'].append(dict(target=target, exit_code=code,
                                           executable_sha256=sha(binary)))
        expected_ctests = {'opengl-' + name for name in (
            'palette-row-cache', 'native-frame-recipe', 'masked-displacement',
            'native-prepared-world', 'native-retained-world', 'world-gpu-resident',
            'world-continuous-normal')}
        expected_ctests.update('native-' + name for name in (
            'world-continuous-memory', 'world-gpu-ack', 'world-continuous-channel',
            'world-continuous-hash'))
        inventory = subprocess.check_output(['ctest', '--test-dir', str(build),
                                              '--show-only=json-v1'], env=env)
        names = {test['name'] for test in json.loads(inventory)['tests']}
        require(expected_ctests <= names, 'Missing CTest registrations: ' + str(sorted(expected_ctests - names)))
        code = run('registered-ctests', ['ctest', '--test-dir', build, '--output-on-failure',
                    '-R', '^(' + '|'.join(sorted(expected_ctests)) + ')$',
                    '--output-junit', out / 'ctest-results.xml'])
        report['registered_ctests'] = dict(names=sorted(expected_ctests), exit_code=code)
        # This facade helper uses stubbed Win32 calls; exercise optimized and
        # sanitizer builds without loading any original image or Wine prefix.
        for mode, flags in [('optimized', ['-O2']),
                            ('asan-ubsan', ['-O1', '-g', '-fsanitize=address,undefined'])]:
            binary = out / ('status-' + mode)
            command = ['clang++', '-std=c++17', '-UNDEBUG', '-Wall', '-Wextra', '-Werror',
                       *flags, frozen / 'tests/native-draw-status-test.cpp', '-o', binary]
            if run('status-build-' + mode, command):
                raise RuntimeError('Status helper build failed')
            code = run('status-' + mode, [binary])
            report['controls'].append(dict(target='native-draw-status-' + mode,
                                           exit_code=code, executable_sha256=sha(binary)))
        if args.pe:
            pe, supplied = args.pe.resolve(), args.displacement_input.resolve()
            require(sha(pe) == '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168', 'Original PE hash')
            require(supplied.is_relative_to(ROOT / 'working'), 'Use an owned captured input')
            require(sha(supplied) == '894a738274d25b7eb223f58f56c66ab76584ba4fea43c4c55b4f2107b1064c81', 'Captured geometry hash')
            owned = out / 'displacement-input.bin'
            shutil.copyfile(supplied, owned)
            report['inputs'] = {str(pe): sha(pe), str(owned): sha(owned)}
            edge_input = None
            if args.bottom_edge_input:
                supplied_edge = args.bottom_edge_input.resolve()
                require(supplied_edge.is_relative_to(ROOT / 'working') and
                    sha(supplied_edge) == '6613b89309eec185004f80c3a3e57927990ad42ec0094d9812a2f507ede5839e', 'Bottom-edge geometry hash')
                edge_input = out / 'bottom-edge-input.bin'
                shutil.copyfile(supplied_edge, edge_input)
                report['inputs'][str(edge_input)] = sha(edge_input)
            require(run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify']) == 0, 'Original manifest before')
            try:
                report['original_artifacts_consumed'] = True
                report['references'] = []
                for name in ('displacement-linear', 'clipped-word-grayscale'):
                    binary = out / (name + '-reference')
                    require(run(name + '-build', ['g++', '-m32', '-std=c++17', '-O2',
                        '-Wall', '-Wextra', '-Werror', '-fno-pie', '-no-pie',
                        frozen / ('tests/' + name + '-reference.cpp'), '-o', binary]) == 0, name + ' reference build')
                    modes = (('linear', 'bottom-edge') if edge_input else ('linear',)) if name == 'displacement-linear' else ('grayscale',)
                    for mode in modes:
                        output = out / (mode + '-original')
                        command = [binary, pe, owned, output] if name == 'displacement-linear' else [binary, pe, output]
                        if mode == 'bottom-edge':
                            command[2] = edge_input
                            command.append('--bottom-edge')
                        require(run(mode + '-original', command) == 0, mode + ' original execution')
                        result = json.loads(output.read_text()) if name == 'displacement-linear' else json.loads((out / (mode + '-original.log')).read_text())
                        require(result['success'], str(result))
                        report['references'].append(dict(mode=mode, result=result,
                                                         executable_sha256=sha(binary)))
                native = next(build.rglob('displacement-linear-test'))
                pixels = out / 'linear-native.565'
                require(run('linear-native', ['xvfb-run', '-a', native, owned, pixels]) == 0, 'Native linear execution')
                require(pixels.read_bytes() == (out / 'linear-original.565').read_bytes(), 'Native linear words differ')
                report['linear_native_original_words_match'] = True
                report['bottom_edge_scope'] = 'Original reads authored backing padding; native finite-edge policy retains logical out-of-range destinations. No equivalence claim for those taps.'
                if args.snapshot_source:
                    inputs = []
                    for label, supplied, expected in [
                        ('snapshot-source', args.snapshot_source, 'f3ec2e4525696811b5412cce8f3302cad260235b85942d8b87db7bac254d4378'),
                        ('snapshot-request', args.snapshot_request, '310976726f7c2b69331ed9448b945bb6a1c75e260682680265916e17d9505456')]:
                        supplied = supplied.resolve()
                        require(supplied.is_relative_to(ROOT / 'working') and sha(supplied) == expected, label)
                        owned = out / (label + '.bin')
                        shutil.copyfile(supplied, owned)
                        report['inputs'][str(owned)] = sha(owned)
                        inputs.append(owned)
                    binary = next(build.rglob('world-snapshot-queue-test'))
                    require(run('snapshot-queue', [binary, *inputs]) == 0, 'Snapshot queue controls')
                    report['snapshot_controls'] = json.loads((out / 'snapshot-queue.log').read_text().splitlines()[-1])
            finally:
                require(run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify']) == 0, 'Original manifest after')
                report['original_manifest_verified_before_after'] = True
                report['inputs_stable'] = all(sha(Path(n)) == h for n, h in report['inputs'].items())
                require(report['inputs_stable'], 'Inputs changed')
        report['success'] = report['registered_ctests']['exit_code'] == 0 and all(c['exit_code'] == 0 for c in report['controls'])
    except (OSError, ValueError, AssertionError, RuntimeError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        sources = report.get('frozen_sources', {})
        report['frozen_sources_stable'] = bool(sources) and all(sha(frozen / n) == h for n, h in sources.items())
        report['workspace_drift'] = [n for n, h in sources.items() if not (ROOT / n).is_file() or sha(ROOT / n) != h]
        report['success'] = report['success'] and report['frozen_sources_stable']
        with (out / 'report.json').open('x') as stream:
            json.dump(report, stream, indent=2)
            stream.write('\n')
    print(json.dumps(dict(success=report['success'], controls=len(report['controls']),
                         report=str(out / 'report.json'))), flush=True)
    return int(not report['success'])


if __name__ == '__main__':
    raise SystemExit(main())
