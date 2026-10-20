#!/usr/bin/env python3
"""Bounded canvas/source-permission/publication controls for the next twenty."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]
NATIVE = ['NR.world-producer-handoff', 'NR.world-source-region-epoch',
          'NR.world-startup-sequence', 'RS.canvas-bind-clip', 'RS.canvas-create-lock',
          'RS.canvas-producer-sequence', 'RS.canvas-startup-clear',
          'RS.drawing-publication', 'RS.world-producer-bypass',
          'RS.world-startup-palette', 'RS.world-startup-wave']
ORIGINAL = ['RS.canvas-bind-clip', 'RS.canvas-create-lock', 'RS.canvas-startup-clear',
            'RS.world-producer-bypass', 'RS.world-startup-palette', 'RS.world-startup-wave']
WAVE = ['NR.world-producer-handoff', 'NR.world-startup-sequence', 'RS.world-startup-wave']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--prefix-template', type=Path, required=True,
                        help='Stopped private synthetic-test Wine prefix under working/tests')
    args = parser.parse_args()
    out, prefix = args.output.resolve(), args.prefix_template.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require a new output under working/tests')
    if not prefix.is_dir() or not prefix.is_relative_to(ROOT/'working/tests'):
        parser.error('Require an existing private test prefix')
    out.mkdir(parents=True)
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    paths = set().union(*(required_sources(behaviors[bid]) for bid in NATIVE))
    # Include transitive repository compiler inputs, including assembly/.inc and
    # CMake files. Original media, generated builds and unrelated tools stay out.
    for directory in ('assets', 'audio', 'compat', 'reconstruction', 'renderer',
                      'runtime', 'protocols', 'apps', 'game'):
        for path in (ROOT/directory).rglob('*'):
            if path.is_file() and (path.suffix in ('.cpp', '.hpp', '.c', '.h', '.S', '.inc', '.cmake')
                                   or path.name == 'CMakeLists.txt'):
                paths.add(path.relative_to(ROOT).as_posix())
    paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'tests').glob('*.hpp'))
    paths.update(['tools/test-next20-canvas-controls.py', 'tools/coverage_claims.py',
                  'tools/test-canvas-startup.py', 'tools/test-canvas-producers.py',
                  'tools/test-rendering-tranche2-reference.py', 'tools/test-rendering-tranche2-wave.py',
                  'tools/inspect-canvas-startup.py', 'tools/inspect-canvas-producers.py',
                  'tools/build-shadow-bridge.py', 'tools/build-scene-observer.py',
                  'tests/test-drawing-publication.py', 'tests/world-source-regions-test.cpp',
                  'tests/canvas-world-test.cpp'])
    sources = {name: sha(ROOT/name) for name in sorted(paths)}

    def declaration(ids):
        return dict(sources=sources, claims=[dict(behavior=bid,
                    contract_sha256=behavior_contract(behaviors[bid], builds)) for bid in ids])

    for label, ids in [('native', NATIVE), ('original', ORIGINAL), ('wave', WAVE)]:
        (out/(label+'-claims.json')).write_text(json.dumps(declaration(ids), indent=2)+'\n')
    report = dict(schema=1, success=False, **declaration(NATIVE), commands=[],
                  scope='Synthetic owned canvas handoff and undefined-history/refusal controls, '
                        'six readable permission modes and unknown protection refusal, final '
                        'publication chronology, actual PE32 startup/producer forwarding, '
                        'and selected original startup branches. Full physical ownership, '
                        'all writers, original live initialization and replacement remain separate.',
                  live_replacement=False, original_pixels_used_as_native_inputs=False)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    for key in ('MESA_LOADER_DRIVER_OVERRIDE', '__GLX_VENDOR_LIBRARY_NAME',
                '__EGL_VENDOR_LIBRARY_FILENAMES', 'QT_XCB_GL_INTEGRATION'):
        env.pop(key, None)
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1',
               GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4',
               ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')

    def run(label, command, timeout=600):
        command = [str(v) for v in command]
        report['commands'].append(dict(label=label, argv=command))
        with (out/(label+'.log')).open('x') as log:
            subprocess.run(command, cwd=ROOT, env=env, stdout=log,
                           stderr=subprocess.STDOUT, timeout=timeout, check=True)

    try:
        build = out/'build'
        run('configure', ['cmake', '-S', ROOT/'compat/legacy/canvas-producers', '-B', build,
                          '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '--parallel', '4', '--target',
                      'canvas-world-test', 'rendering-tranche2-test', 'rendering-tranche2-gpu-test'])
        for label, flags in [('regions', ['-O2']),
                             ('regions-sanitized', ['-O1', '-g', '-fsanitize=address,undefined',
                                                    '-fno-omit-frame-pointer', '-fno-pie', '-no-pie'])]:
            run(label+'-build', ['g++', '-std=c++17', '-UNDEBUG', '-Wall', '-Wextra', '-Werror',
                                 *flags, ROOT/'tests/world-source-regions-test.cpp', '-o', out/label])
            run(label, [out/label])
        run('publication', ['python3', '-B', ROOT/'tests/test-drawing-publication.py'])
        # Each legacy runner owns its cloned prefix. The template is already
        # stopped by its owner; no shared game prefix is used or terminated.
        local = ROOT/'working/tests/scene-selftest-wine'
        if not local.exists():
            local.symlink_to(prefix, target_is_directory=True)
        elif local.resolve() != prefix:
            raise ValueError('Synthetic startup prefix already belongs to another test')
        run('startup-hooks', ['xvfb-run', '-a', 'python3', ROOT/'tools/test-canvas-startup.py'])
        run('producer-hooks', ['python3', ROOT/'tools/test-canvas-producers.py',
                              '--prefix-template', prefix, '--claims', out/'native-claims.json',
                              '--report', out/'producer-report.json'])
        run('original', ['python3', ROOT/'tools/test-rendering-tranche2-reference.py',
                         '--executable', ROOT/'original/Arcane_Nocd/Chaos.exe',
                         '--output', out/'original', '--claims', out/'original-claims.json',
                         *[v for case in ('create', 'bind', 'clear', 'wave', 'palette', 'uniform', 'caller')
                           for v in ('--case', case)]])
        run('wave', ['python3', ROOT/'tools/test-rendering-tranche2-wave.py',
                     '--reference-report', out/'original/report.json', '--build', build,
                     '--claims', out/'wave-claims.json', '--output', out/'wave'])
        run('handoff', ['xvfb-run', '-a', build/'canvas-world-test'])
        original = json.loads((out/'original/report.json').read_text())
        wave = json.loads((out/'wave/report.json').read_text())
        producer = json.loads((out/'producer-report.json').read_text())
        if not all(r['success'] for r in (original, wave, producer)):
            raise ValueError('A bounded component result did not pass')
        if any(sha(ROOT/p) != h for p, h in sources.items()):
            raise ValueError('Sources changed during bounded canvas controls')
        report.update(success=True, original=original, wave=wave, producer=producer,
                      publication_tests=23, unknown_protection_modes=3,
                      readable_protection_modes=6, native_regions_sanitized=True,
                      handoff_passed=True, binaries={p.name: sha(p) for p in
                          [build/'canvas-world-test', build/'rendering-tranche2-test',
                           build/'rendering-tranche2-gpu-test', out/'regions', out/'regions-sanitized']})
        wave_result = dict(schema=1, success=True, sources_stable=True, **declaration(WAVE),
                           original_sha256=original['original_sha256'], wave=wave,
                           original_pixels_used_as_native_inputs=False, live_replacement=False)
        (out/'wave-result.json').write_text(json.dumps(wave_result, indent=2)+'\n')
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
