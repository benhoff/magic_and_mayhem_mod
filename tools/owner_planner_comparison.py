#!/usr/bin/env python3
"""Shared source-bound, isolated original/native owner planner comparison runner."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SUITES = {'selection': {'ranges': [(4359392, 4360663)],
               'endpoints': [],
               'scope': 'Whole4284e0 taskless old-plan projection, count-based budgets, finite '
                        'authored evaluator candidates and ordered supplied prepare/next/save '
                        'methods. Real original RNG and group iterators; no text redirects. '
                        'Weighted/maximum selection, exact-one rejection, bias and read-only '
                        'selection. Evaluator generation, task execution, task replacement gates '
                        'and World scheduling excluded.',
               'suite': 'owner-planner-selection',
               'sources': ['tests/owner-planner-selection-test.cpp',
                           'reconstruction/creature-ai/owner_planner.cpp',
                           'reconstruction/combat/melee.cpp'],
               'wrapper': 'tools/test-owner-planner.py'},
 'brain': {'ranges': [(4679344, 4680499)],
           'endpoints': [5863504, 5863536],
           'scope': 'Whole4766b0 with existing supplied DAT models, signed integer feature '
                    'vectors, real lookup/temporary vector storage and explicit Linux allocators. '
                    'Native inference admission is separate. Model loading/training, '
                    'missing/corrupt-model recovery, task feature production and World integration '
                    'excluded.',
           'suite': 'owner-brain-inference',
           'sources': ['tests/owner-brain-inference-test.cpp',
                       'reconstruction/creature-ai/brain_inference.cpp',
                       'assets/dat.cpp'],
           'wrapper': 'tools/test-owner-brain.py'},
 'features': {'ranges': [(4240416, 4251595)],
              'endpoints': [],
              'scope': 'Whole40b420 with its exact kind6 ordered67 feature IDs, one ordinary '
                       'noncasting actor per source group, empty resources/inventories/observation '
                       'points and no block7c. Source CFG/value fields, flags and wrapped '
                       'positions vary. No text redirects. Other features, group aggregates and '
                       'task path suffix excluded.',
              'suite': 'owner-task-features',
              'sources': ['tests/owner-task-features-test.cpp',
                          'reconstruction/creature-ai/owner_planner_features.cpp',
                          'reconstruction/creature-ai/brain_inference.cpp',
                          'assets/dat.cpp'],
              'wrapper': 'tools/test-owner-task-features.py'},
 'candidate': {'ranges': [(4398304, 4401360), (4280304, 4280360), (4255152, 4255372)],
               'endpoints': [4679344, 5863504, 5863536],
               'scope': 'Whole431ce0 candidate-completion projection after prepared successful '
                        'per-actor probes, one ordinary creature per group, exact67+5 feature '
                        'order, explicit radius/path summaries, three supplied inference outputs '
                        'and real414ff0/40edb0 scoring. Region parameter1, empty resources/spells, '
                        'finite coefficient/percentage inputs and strategic RNG. Probe production, '
                        'grouping/eligibility, model inference (compared separately), full task '
                        'dispatch/replacement and live execution excluded.',
               'suite': 'owner-task-candidate',
               'sources': ['tests/owner-task-candidate-test.cpp',
                           'reconstruction/creature-ai/owner_planner_features.cpp',
                           'reconstruction/creature-ai/brain_inference.cpp',
                           'reconstruction/combat/melee.cpp'],
               'wrapper': 'tools/test-owner-task-candidate.py'},
 'task': {'ranges': [(4401408, 4403120), (4226064, 4226531)],
          'endpoints': [5863504, 5863536, 5287696, 5307728, 5308144],
          'scope': 'Whole432900 single ordinary melee actor with empty resources, no wizard '
                   'formations, zero regional danger and empty opposite-brain assignments; actual407c10 resolves memory '
                   'keys/markers into live pool slots. Target/move/stop command and allocator '
                   'services are explicit recorders. '
                   'Complete/stale/missing/dead/allied/engaged/dirty branches; full owner '
                   'dispatch/task replacement/path scoring and live integration excluded.',
          'suite': 'owner-hostile-task',
          'sources': ['tests/owner-hostile-task-test.cpp',
                      'reconstruction/creature-ai/owner_hostile_task.cpp'],
          'wrapper': 'tools/test-owner-hostile-task.py'}}



def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main(suite):
    config = SUITES[suite]
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--claims', type=Path)
    p.add_argument('--brain', type=Path)
    p.add_argument('--trace', type=Path)
    p.add_argument('--trace-origin', type=Path)
    args = p.parse_args()
    args.output = args.output.resolve()
    if args.output.exists():
        p.error('Refusing existing evidence output')
    ranges, endpoints = config['ranges'], config['endpoints']
    if suite == 'brain' and not args.brain:
        p.error('Brain inference requires --brain')
    claims = json.loads(args.claims.read_text()) if args.claims else {'sources': {}, 'claims': []}
    if any(sha(ROOT / path) != digest for path, digest in claims['sources'].items()):
        raise ValueError('Declared source changed before execution')
    args.output.mkdir(parents=True)
    report = dict(success=False, claims=claims['claims'], source_sha256=claims['sources'],
                  source_executable_sha256=HASH, original_target_text_unchanged=False,
                  scope=config['scope'],
                  suite=config['suite'],
                  endpoint_addresses=[hex(a) for a in endpoints], wine_required=False,
                  live_replacement=False, commands=[])

    def run(name, command, env=None):
        report['commands'].append([str(x) for x in command])
        try:
            result = subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                                    capture_output=True, text=True, timeout=120)
        except subprocess.TimeoutExpired as exc:
            for stream in ['stdout', 'stderr']:
                value = getattr(exc, stream) or b''
                (args.output / (name + '.' + stream)).write_text(value.decode(errors='replace') if isinstance(value, bytes) else value)
            raise
        (args.output / (name + '.stdout')).write_text(result.stdout)
        (args.output / (name + '.stderr')).write_text(result.stderr)
        if result.returncode or 'ERROR: AddressSanitizer' in result.stderr or 'runtime error:' in result.stderr:
            raise RuntimeError(f'{name} failed ({result.returncode}): {result.stdout[-3000:]}{result.stderr[-3000:]}')
        return result.stdout

    run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
    try:
        executable = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        raw = executable.read_bytes()
        if sha(executable) != HASH:
            raise ValueError('Unsupported immutable PE; no execution/redirects permitted')
        pe = struct.unpack_from('<I', raw, 60)[0]
        sections = pe + 24 + struct.unpack_from('<H', raw, pe + 20)[0]

        def image_bytes(va, size):
            for i in range(struct.unpack_from('<H', raw, pe + 6)[0]):
                rva, n, offset = struct.unpack_from('<3I', raw, sections + 40 * i + 12)
                if 0x400000 + rva <= va and va + size <= 0x400000 + rva + n:
                    return raw[offset + va - 0x400000 - rva:offset + va - 0x400000 - rva + size]
            raise ValueError('Unbacked original address')
        report['original_entry_anchors'] = {hex(a): image_bytes(a, 16).hex() for a, _ in ranges}
        report['original_target_sha256'] = {hex(a): hashlib.sha256(image_bytes(a, b-a)).hexdigest() for a, b in ranges}
        report['redirect_original_bytes'] = {hex(a): image_bytes(a, 5).hex() for a in endpoints}
        flags = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-Ireconstruction/combat', '-Iassets', '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections']
        sources = config['sources']
        # Development runs also need a stable transitive source closure. A
        # declaration remains mandatory for scope-bound evidence, but omitted
        # claims must never permit a mixed-version build to look successful.
        dependency_text = run('pre-build-dependencies', flags + ['-m32', '-DMNM_PLANNER_REFERENCE' if suite == 'selection' else '-DMNM_BRAIN_REFERENCE', '-MM'] + sources)
        initial_dependencies = {'tools/owner_planner_comparison.py', config['wrapper'], *sources}
        for token in shlex.split(dependency_text.replace('\\\n', ' ')):
            path = Path(token)
            if not path.is_absolute() and (ROOT / path).is_file():
                initial_dependencies.add((ROOT / path).resolve().relative_to(ROOT).as_posix())
        report['build_inputs_sha256'] = {path: sha(ROOT / path) for path in sorted(initial_dependencies)}
        def build(name, options):
            objects=[]
            for i, source in enumerate(sources):
                obj=args.output/f'{name}-{i}.o'
                run(f'build-{name}-{i}',flags+options+['-MMD','-MF',args.output/f'{name}-{i}.d','-c',source,'-o',obj])
                objects.append(obj)
            run(f'link-{name}',flags+options+objects+['-o',args.output/name])
        build('native',['-O2'])
        build('reference',['-m32','-O1','-g','-no-pie','-DMNM_PLANNER_REFERENCE' if suite == 'selection' else '-DMNM_BRAIN_REFERENCE'])
        build('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])
        dependencies = {'tools/owner_planner_comparison.py', config['wrapper']}
        for dependency in args.output.glob('*.d'):
            for token in shlex.split(dependency.read_text().replace('\\\n', ' ')):
                file = Path(token)
                if file.is_absolute():
                    if file.is_relative_to(ROOT):
                        file = file.relative_to(ROOT)
                    else:
                        continue
                if (ROOT / file).is_file() and not str(file).startswith('working/'):
                    dependencies.add((ROOT / file).resolve().relative_to(ROOT).as_posix())
        report['compiler_dependencies'] = sorted(dependencies)
        if dependencies - report['build_inputs_sha256'].keys():
            raise ValueError('Source closure changed during compilation: ' + str(sorted(dependencies - report['build_inputs_sha256'].keys())))
        if any(sha(ROOT / path) != digest for path, digest in report['build_inputs_sha256'].items()):
            raise ValueError('Source edited during compilation; refusing mixed-version execution')
        if args.claims and dependencies - claims['sources'].keys():
            raise ValueError('Unbound compiler inputs: ' + str(sorted(dependencies - claims['sources'].keys())))
        if suite == 'brain': report['brain_sha256'] = sha(args.brain)
        inputs = [args.brain] if suite == 'brain' else []
        if args.trace:
            report['trace_sha256'] = sha(args.trace)
            report['trace_path'] = str(args.trace.resolve())
            if args.trace_origin:
                origin=json.loads(args.trace_origin.read_text())
                if not all(origin.get(k) for k in ['success','sources_unchanged','build_inputs_unchanged','native_passed','sanitizer_passed']):
                    raise ValueError('Task trace requires a successful stable native producer')
                if not origin.get('claims') or not origin.get('source_sha256'):
                    raise ValueError('Task trace producer lacks prospective bindings')
                if any(sha(ROOT/k)!=v for k,v in origin['source_sha256'].items()):
                    raise ValueError('Task trace producer sources changed')
                matches=[v for v in origin.get('task_traces',{}).values() if v['sha256']==report['trace_sha256'] and (args.trace_origin.parent/v['path']).resolve()==args.trace.resolve()]
                if not matches:raise ValueError('Task trace missing from producer artifacts')
                report['trace_origin_sha256']=sha(args.trace_origin)
                report['trace_origin_path']=str(args.trace_origin.resolve())
                report['trace_origin_bound']=True
                report['producer_sources_current']=True
            inputs.append(args.trace)
        native = json.loads(run('native', [args.output / 'native', *inputs]))
        original = json.loads(run('original', [args.output / 'reference', executable, *inputs]))
        sanitized = json.loads(run('sanitized', [args.output / 'sanitized', *inputs], env={**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1', 'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1'}))
        assert native == original == sanitized
        assert executable.read_bytes() == raw
        if suite == 'brain': assert sha(args.brain) == report['brain_sha256']
        if args.trace: assert sha(args.trace) == report['trace_sha256']
        if args.trace_origin:
            assert sha(args.trace_origin)==report['trace_origin_sha256']
            assert all(sha(ROOT/k)==v for k,v in origin['source_sha256'].items())
        report.update(**original,
                      original_target_text_unchanged=True,
                      native_passed=True, sanitizer_passed=True, input_unchanged=True,
                      binaries={name: sha(args.output / name) for name in ['native', 'reference', 'sanitized']},
                      compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0])
    finally:
        run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['sources_unchanged'] = all(sha(ROOT / path) == digest for path, digest in claims['sources'].items())
        report['build_inputs_unchanged'] = all(sha(ROOT / path) == digest for path, digest in report.get('build_inputs_sha256', {}).items())
        (args.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    if not report['sources_unchanged'] or not report['build_inputs_unchanged']:
        raise ValueError('Source changed during comparison')
    print(json.dumps({k: report[k] for k in ['success', 'checks', 'cases', 'slices', 'sanitizer_passed'] if k in report}))
