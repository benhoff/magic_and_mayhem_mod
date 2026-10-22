#!/usr/bin/env python3
"""Execute the nine reconciled bounded rendering scopes with prospective claims.

Selected original wrappers use scripted external APIs. Retained capture replay
is offline comparison, never a new live capture or complete driver recovery.
"""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]
SCOPES = ['RS.canvas-create-lock', 'RS.canvas-bind-clip', 'RS.canvas-startup-clear',
          'RS.world-startup-palette', 'RS.world-startup-wave', 'NR.world-startup-sequence',
          'NR.canvas-producer-replay', 'NR.world-producer-handoff', 'RS.world-producer-bypass']
BRANCHES = [bid for bid in SCOPES if bid.startswith('RS.')]
TOOL = 'TOOL.critical-scope-reconciliation'


def sha(path):
    return hashlib.sha256(evidence_bytes(path)).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--producer-capture', type=Path, required=True)
    p.add_argument('--startup-capture', type=Path, required=True)
    p.add_argument('--prefix-template', type=Path, required=True)
    a = p.parse_args()
    out = a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT / 'working/tests'):
        p.error('Require a fresh output under this checkout working/tests')
    out.mkdir(parents=True)
    register = json.loads(evidence_text(ROOT / 'research/runtime/coverage/register.json'))
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    scenarios = {s['id']: s for s in register['scenarios']}
    selected_scenarios = ['critical-scope-branches-20261020', 'canvas-producers-native-20261008',
                          'world-startup-sequence-native-20261008', 'world-producer-handoff-offline-20261008']
    paths = set().union(*(required_sources(behaviors[bid]) for bid in SCOPES + [TOOL]))
    paths.update(p for sid in selected_scenarios for p in scenarios[sid]['tests'])
    # Reviewed compiler dependency output and the actual PE32 builder closure
    # avoid unrelated application sources in this execution's provenance.
    manifest_path = 'research/runtime/coverage/critical-scope-compiler-inputs-20261020.json'
    compiler_inputs = json.loads(evidence_text(ROOT / manifest_path))
    if compiler_inputs.get('schema') != 1 or compiler_inputs.get('reviewed') is not True:
        raise ValueError('Require the reviewed native/PE32 compiler input manifest')
    paths.add(manifest_path)
    for name in compiler_inputs['paths']:
        target = (ROOT / name).resolve()
        if (Path(name).is_absolute() or not target.is_relative_to(ROOT) or
                target.relative_to(ROOT).parts[0] in {'working', 'original', '.git'}):
            raise ValueError('Unsafe reviewed compiler input: ' + name)
        paths.add(name)
    paths.update(['tools/test-critical-scope-reconciliation.py', 'tools/coverage_claims.py',
                  'tools/test-rendering-tranche2-reference.py', 'tools/test-rendering-tranche2-wave.py',
                  'tools/test-native-canvas-producers.py', 'tools/test-native-startup-sequence.py',
                  'tools/test-world-producer-handoff.py', 'tools/test-canvas-startup.py',
                  'tools/inspect-canvas-startup.py', 'tools/inspect-canvas-producers.py',
                  'tools/test-world-frames.py',
                  'tools/build-scene-observer.py', 'tools/build-shadow-bridge.py'])
    sources = {name: sha(ROOT / name) for name in sorted(paths)}

    def declaration(ids):
        return dict(schema=1, sources=sources, claims=[dict(
            behavior=bid, contract_sha256=behavior_contract(behaviors[bid], builds),
            scenarios={sid: scenario_contract(scenarios[sid]) for sid in selected_scenarios
                       if bid in scenarios[sid]['behaviors']}) for bid in ids])

    for label, ids in [('branches', BRANCHES), ('all', SCOPES + [TOOL]),
                        ('producer', ['NR.canvas-producer-replay']),
                        ('startup', ['NR.world-startup-sequence', 'RS.world-startup-wave', 'RS.world-startup-palette']),
                        ('handoff', ['NR.world-producer-handoff'])]:
        (out / (label + '-claims.json')).write_text(json.dumps(declaration(ids), indent=2) + '\n')
    report = dict(success=False, **declaration(SCOPES + [TOOL]), commands=[],
                  scope='Nine bounded scope controls: scripted original create/bind/clear/palette/wave/caller branches, '
                        'native CPU/GPU and PE32 hook controls, default producer checkpoints, sixteen zero-policy '
                        'startup completions and source-only finite GPU handoff. Wider scope is separately retained.',
                  live_replacement=False, new_live_capture=False,
                  physical_driver_recovery_proved=False, original_pixels_used_as_native_inputs=False)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    for key in ('MESA_LOADER_DRIVER_OVERRIDE', '__GLX_VENDOR_LIBRARY_NAME',
                '__EGL_VENDOR_LIBRARY_FILENAMES', 'QT_XCB_GL_INTEGRATION'):
        env.pop(key, None)
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe',
               LP_NUM_THREADS='4', WINEDEBUG='-all')

    def run(label, command, timeout=900):
        argv = [str(x) for x in command]
        print(label, flush=True)
        report['commands'].append(dict(label=label, argv=argv))
        with (out / (label + '.log')).open('x') as log:
            subprocess.run(argv, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT,
                           timeout=timeout, check=True)

    def saved_report(label):
        lines = evidence_text(out / (label + '.log')).splitlines()
        paths = [Path(line) for line in lines if line.endswith('/report.json')]
        if not paths or not paths[-1].is_relative_to(ROOT / 'working/tests'):
            raise ValueError('Missing isolated executed report: ' + label)
        return paths[-1], json.loads(evidence_text(paths[-1]))

    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        original = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        expected_original = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
        if sha(original) != expected_original:
            raise ValueError('Pinned original executable changed')
        owned_original = out / 'original-reference.exe'
        shutil.copyfile(original, owned_original)
        if sha(owned_original) != expected_original:
            raise ValueError('Isolated original reference copy differs')
        build = out / 'build'
        run('configure', ['cmake', '-S', ROOT / 'compat/legacy/canvas-producers', '-B', build,
                          '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '--parallel', '4', '--target',
                      'rendering-tranche2-test', 'rendering-tranche2-gpu-test', 'canvas-world-test',
                      'canvas-sequence-test'])
        run('native-controls', ['ctest', '--test-dir', build, '--output-on-failure',
                                '-R', '^(native-rendering-tranche2|native-rendering-tranche2-gpu|native-canvas-world-handoff|native-canvas-sequence)$'])
        prefix = ROOT / 'working/tests/scene-selftest-wine'
        if not prefix.exists():
            prefix.symlink_to(a.prefix_template.resolve(), target_is_directory=True)
        elif prefix.resolve() != a.prefix_template.resolve():
            raise ValueError('Private startup template already has another owner')
        run('startup-hooks', ['xvfb-run', '-a', 'python3', '-B', ROOT / 'tools/test-canvas-startup.py'])
        run('branches', ['python3', '-B', ROOT / 'tools/test-rendering-tranche2-reference.py',
                         '--executable', ROOT / 'original/Arcane_Nocd/Chaos.exe',
                         '--output', out / 'branches', '--claims', out / 'branches-claims.json',
                         *[v for case in ('create', 'bind', 'clear', 'wave', 'palette', 'caller') for v in ('--case', case)]])
        branch = json.loads(evidence_text(out / 'branches/report.json'))
        palette = next(row for row in branch['results'] if row['case'] == 'palette-selection')
        if not branch['success'] or palette['cases'] != 14280 or palette['palette_ordinals'] != 8:
            raise ValueError('Expanded palette/reference matrix is incomplete')
        run('wave', ['python3', '-B', ROOT / 'tools/test-rendering-tranche2-wave.py',
                     '--reference-report', out / 'branches/report.json', '--build', build,
                     '--claims', out / 'startup-claims.json', '--output', out / 'wave'])
        run('producer', ['python3', '-B', ROOT / 'tools/test-native-canvas-producers.py',
                         '--capture-report', a.producer_capture.resolve(), '--claims', out / 'producer-claims.json'])
        producer_path, producer = saved_report('producer')
        if not producer['success'] or producer['completed'] != 1061 or producer['equal'] != 1061 or producer['mismatches']:
            raise ValueError('Incomplete default source-only checkpoint comparison')
        run('startup', ['xvfb-run', '-a', 'python3', '-B', ROOT / 'tools/test-native-startup-sequence.py',
                        '--capture-report', a.startup_capture.resolve(), '--claims', out / 'startup-claims.json',
                        '--executable', owned_original,
                        '--prefix-template', a.prefix_template.resolve()])
        startup_path, startup = saved_report('startup')
        if not (startup['success'] and startup['completed_canvases_compared'] == 16 and
                startup['all_native_original_same_initial_state_match'] and
                startup['all_original_before_replays_match_live'] and startup['all_native_live_canvases_match']):
            raise ValueError('Incomplete or differing sixteen-queue startup comparison')
        run('handoff', ['python3', '-B', ROOT / 'tools/test-world-producer-handoff.py',
                        '--capture-report', a.producer_capture.resolve(), '--claims', out / 'handoff-claims.json',
                        '--output-parent', out / 'handoff'])
        handoff_path, handoff = saved_report('handoff')
        if not (handoff['success'] and handoff['completed'] == 1061 and handoff['equal'] == 1061 and
                handoff['original_world_equal'] == 16 and handoff['bypass_equal'] == 0):
            raise ValueError('Incomplete finite source-only handoff or unexpected bypass selection')
        wave = json.loads(evidence_text(out / 'wave/report.json'))
        if not wave['success']:
            raise ValueError('Full CPU/GPU wave comparison failed')
        inputs = {str(original): expected_original}
        for path, child in [(out / 'branches/report.json', branch), (out / 'wave/report.json', wave),
                            (producer_path, producer), (startup_path, startup), (handoff_path, handoff)]:
            if child.get('inputs_stable') is False or child.get('sources_stable') is not True:
                raise ValueError('Component provenance changed: ' + str(path))
            inputs[str(path)] = sha(path)
            for name, value in child.get('inputs', {}).items():
                absolute = str(Path(name) if Path(name).is_absolute() else ROOT / name)
                if absolute in inputs and inputs[absolute] != value:
                    raise ValueError('Component input identity disagrees: ' + name)
                inputs[absolute] = value
        report['inputs'] = inputs
        report['inputs_stable'] = all(sha(Path(name)) == value for name, value in inputs.items())
        if not report['inputs_stable']:
            raise ValueError('Bounded input changed')
        report.update(success=True, palette_cases=palette['cases'], palette_words=palette['palette_words_compared'],
                      default_checkpoints=1061, startup_completions=16, handoff_checkpoints=1061,
                      handoff_original_world_completions=16, wave_cases=768,
                      reports={name: dict(path=str(path.relative_to(ROOT)), sha256=sha(path))
                               for name, path in [('branches', out / 'branches/report.json'),
                                                   ('wave', out / 'wave/report.json'), ('producer', producer_path),
                                                   ('startup', startup_path), ('handoff', handoff_path)]})
    except (OSError, ValueError, KeyError, StopIteration, subprocess.SubprocessError) as exc:
        report['error'] = str(exc)
    finally:
        try:
            run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        except (OSError, subprocess.SubprocessError) as exc:
            report['success'] = False
            report['original_preservation_error'] = str(exc)
        report['sources_stable'] = all(sha(ROOT / name) == value for name, value in sources.items())
        report['success'] &= report['sources_stable']
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(out / 'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
