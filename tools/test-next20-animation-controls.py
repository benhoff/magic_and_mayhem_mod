#!/usr/bin/env python3
"""Compare owned animation operations with bounded unchanged-original fixtures."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SELECTED = ['AN.attachment', 'AN.direction', 'AN.forward', 'AN.placement', 'AN.world-facing']
POLICY = 'NP.animation-owned-selection-admission'
CONTROLLER = ['AN.forward', 'AN.direction', 'AN.reverse-controller']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--asset-root', type=Path, required=True,
                        help='Read-only installed asset tree with ANI files and effectani.cfg')
    args = parser.parse_args()
    out, asset_root = args.output.resolve(), args.asset_root.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or not asset_root.is_dir():
        parser.error('Require a new test output and an existing read-only asset root')
    out.mkdir(parents=True)
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    paths = set().union(*(required_sources(behaviors[bid]) for bid in SELECTED+[POLICY, 'AN.reverse-controller']))
    for directory in ('assets', 'reconstruction', 'game', 'apps/command-session', 'protocols'):
        for path in (ROOT/directory).rglob('*'):
            if path.is_file() and (path.suffix in ('.cpp', '.hpp', '.c', '.h', '.S', '.inc', '.cmake')
                                   or path.name == 'CMakeLists.txt'):
                paths.add(path.relative_to(ROOT).as_posix())
    paths.update(['tools/test-next20-animation-controls.py', 'tools/coverage_claims.py',
                  'tools/decode-cfg.py', 'tests/animation-body-placement-reference.cpp',
                  'tests/animation-placement-reference.cpp'])
    sources = {name: sha(ROOT/name) for name in sorted(paths)}

    def declaration(ids):
        return dict(sources=sources, claims=[dict(behavior=bid,
                    contract_sha256=behavior_contract(behaviors[bid], builds)) for bid in ids])

    for label, ids in [('native', SELECTED+[POLICY]), ('original', SELECTED[:-1]),
                       ('controller', CONTROLLER), ('body', ['AN.placement'])]:
        (out/(label+'-claims.json')).write_text(json.dumps(declaration(ids), indent=2)+'\n')
    report = dict(schema=1, success=False, **declaration(SELECTED+[POLICY]), commands=[],
                  original_game_executed=False, live_replacement=False,
                  scope='Owned forward display admission, phase switching, body null-display fallback, '
                        'mode-one attachment lifecycle and production cell-coordinate facing controls. '
                        'Selected original helper/controller comparisons retain their fixture boundaries; '
                        'camera/object/asset lifetime, full targeting, world draw ordering and live replacement are separate.')
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')

    def run(label, command, timeout=900):
        command = [str(v) for v in command]
        report['commands'].append(dict(label=label, argv=command))
        with (out/(label+'.log')).open('x') as log:
            subprocess.run(command, cwd=ROOT, env=env, stdout=log,
                           stderr=subprocess.STDOUT, timeout=timeout, check=True)

    inputs = {}
    original = None
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        executable = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(executable) != BUILD:
            raise ValueError('Unsupported original executable')
        for path in asset_root.rglob('*'):
            if path.is_file() and (path.suffix.lower()=='.ani' or path.name.lower()=='effectani.cfg'):
                inputs[str(path)] = sha(path)
        installed = ROOT/'working/game-clean'
        if not installed.exists():
            installed.symlink_to(asset_root, target_is_directory=True)
        elif installed.resolve() != asset_root:
            raise ValueError('Installed test asset root already belongs to another run')
        native = out/'native'
        run('native-configure', ['cmake', '-S', ROOT/'reconstruction/animation', '-B', native,
                                 '-DCMAKE_BUILD_TYPE=Debug'])
        run('native-build', ['cmake', '--build', native, '--parallel', '4'])
        run('native-tests', ['ctest', '--test-dir', native, '--output-on-failure'])
        for test in ('model', 'placement', 'body-placement', 'attachment'):
            binary = out/('sanitized-'+test)
            run('sanitized-'+test+'-build', ['g++', '-std=c++17', '-O1', '-g', '-Wall', '-Wextra', '-Werror',
                 '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie',
                 '-I'+str(ROOT/'assets'), '-I'+str(ROOT/'reconstruction/animation'),
                 ROOT/('tests/animation-'+test+'-test.cpp'),
                 *[ROOT/'reconstruction/animation'/name for name in
                   ('no_cd.cpp', 'placement.cpp', 'body_placement.cpp', 'attachment.cpp')], '-o', binary])
            run('sanitized-'+test, [binary])
        # Legacy corpus runners use this exact inspector location. Build it
        # from the current source rather than adopting an unbound old binary.
        assets_build = ROOT/'working/build/assets'
        run('inspector-configure', ['cmake', '-S', ROOT/'assets', '-B', assets_build,
                                    '-DBUILD_TESTING=OFF', '-DCMAKE_BUILD_TYPE=Debug'])
        run('inspector-build', ['cmake', '--build', assets_build, '--target', 'mnm-animation-inspect', '--parallel', '4'])
        legacy = {}
        for label, folder, script in [('forward', 'animation', 'test-animation-contract.py'),
                                      ('switch', 'animation-switch', 'test-animation-switch.py'),
                                      ('placement', 'animation-placement', 'test-animation-placement.py'),
                                      ('attachment', 'animation-attachment', 'test-animation-attachment.py')]:
            parent = ROOT/'working/tests'/folder
            before = set(parent.glob('run-*')) if parent.exists() else set()
            run(label, ['python3', ROOT/'tools'/script])
            created = set(parent.glob('run-*'))-before
            if len(created)!=1:
                raise ValueError('Ambiguous legacy output: '+label)
            path = created.pop()/'report.json'
            inputs[str(path)] = sha(path)
            legacy[label] = json.loads(path.read_text())
            summary = legacy[label]['summary']
            if not (summary.get('all_match') or summary.get('all_traces_match')):
                raise ValueError('Original corpus did not compare: '+label)
        run('body-reference-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
             '-I'+str(ROOT/'assets'), '-I'+str(ROOT/'reconstruction/animation'),
             ROOT/'tests/animation-body-placement-reference.cpp',
             *[ROOT/'reconstruction/animation'/name for name in ('no_cd.cpp', 'placement.cpp', 'body_placement.cpp')],
             '-o', out/'body-reference'])
        run('body-reference', [out/'body-reference', executable])
        body = json.loads((out/'body-reference.log').read_text())
        if body['cases']!=4608 or not body['controller_states_match'] or not body['original_targets_unchanged']:
            raise ValueError('Body fallback comparison incomplete')
        run('controller', ['python3', ROOT/'tools/test-animation-controller.py',
                           '--output', out/'controller', '--claims', out/'controller-claims.json'])
        controller = json.loads((out/'controller/report.json').read_text())
        if not controller['success']:
            raise ValueError('Full controller comparison did not pass')
        run('facing', ['python3', ROOT/'tools/test-native-facing-reconciliation.py', '--output', out/'facing'])
        facing = json.loads((out/'facing/report.json').read_text())
        if not facing['success']:
            raise ValueError('Production-facing control did not pass')
        if any(sha(ROOT/name)!=digest for name, digest in sources.items()):
            raise ValueError('Animation sources changed during execution')
        report.update(success=True, native_ctests=4, sanitizer_controls=4,
                      legacy=legacy, body=body, controller=controller, facing=facing,
                      inspector_sha256=sha(assets_build/'mnm-animation-inspect'))
        original = dict(schema=1, success=True, **declaration(SELECTED[:-1]),
                        executable_sha256=BUILD, legacy=legacy, body=body,
                        live_replacement=False, unsafe_negative_original_inputs_executed=False,
                        sources_stable=True)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        except (OSError, subprocess.SubprocessError) as error:
            report['cleanup_error'] = str(error)
            report['success'] = False
        report.update(sources_stable=all(sha(ROOT/p)==h for p,h in sources.items()),
                      inputs=inputs, inputs_stable=all(sha(Path(p))==h for p,h in inputs.items()))
        report['success'] &= report['sources_stable'] and report['inputs_stable']
        if report['success'] and original is not None:
            original.update(inputs_stable=True, original_manifest_verified_before_after=True)
            (out/'original-result.json').write_text(json.dumps(original, indent=2)+'\n')
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
