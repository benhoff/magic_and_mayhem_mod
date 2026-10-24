#!/usr/bin/env python3
"""Renew three bounded rendering contracts against native controls and original x86."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
SELECTED = ['RE.sprite_queue', 'RE.sprite_visibility', 'AN.direction']
POLICY = 'NP.sprite-visibility-atomic-admission'
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True, help='New ignored execution directory')
    parser.add_argument('--report', type=Path, required=True, help='New SQLite-backed evidence pointer')
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or args.report.exists():
        parser.error('Require new execution and report paths')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain exact eighteenth rendering native/original controls and failures.\n')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    controls = {
        'queue': ['tests/sprite-queue-test.cpp', 'reconstruction/rendering/sprite_queue.cpp'],
        'visibility': ['tests/sprite-visibility-test.cpp', 'reconstruction/rendering/sprite_visibility.cpp'],
        'direction': ['tests/animation-model-test.cpp', 'reconstruction/animation/no_cd.cpp'],
    }
    flags = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-Ireconstruction/rendering',
             '-Ireconstruction/animation', '-Iassets']
    paths = set().union(*(required_sources(behaviors[b]) for b in SELECTED+[POLICY]))
    paths.update(['tools/coverage_claims.py', 'tools/original-manifest.sh',
                  'reconstruction/animation/controller.cpp', 'reconstruction/animation/controller.hpp'])
    # Pin actual compiler headers as well as every declared contract input.
    dep_sources = list(dict.fromkeys(p for v in controls.values() for p in v))
    dep_sources += ['tests/animation-binary-reference.cpp', 'tests/animation-controller-reference.cpp',
                    'tests/sprite-queue-reference.cpp', 'tests/sprite-visibility-reference.cpp',
                    'reconstruction/animation/controller.cpp']
    deps = subprocess.check_output(flags+['-MM']+dep_sources, cwd=ROOT, text=True)
    (out/'dependencies.txt').write_text(deps)
    for token in shlex.split(deps.replace('\\\n', ' ')):
        p = ROOT/token
        if p.is_file():
            paths.add(p.resolve().relative_to(ROOT).as_posix())
    sources = {p: sha(ROOT/p) for p in sorted(paths)}
    claims = [dict(behavior=b, contract_sha256=behavior_contract(behaviors[b], builds))
              for b in SELECTED+[POLICY]]
    report = dict(schema=1, success=False, sources=sources, claims=claims,
                  original_claims=[c for c in claims if c['behavior'] in SELECTED],
                  original_sha256=BUILD, commands=[], controls={}, comparisons={},
                  live_replacement=False, complete_directdraw_retirement=False,
                  scope='Selected offline depth-key/exact unstable sorting, visibility mask/reverse traversal '
                        'and forward phase switching only. Native malformed-input refusal is a separate policy. '
                        'No whole-world producers, original allocations, full targeting, live ownership, '
                        'performance, integration or replacement credit.')
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')

    def run(label, argv, timeout=300):
        command = [str(a) for a in argv]
        with (out/(label+'.log')).open('x') as log:
            result = subprocess.run(command, cwd=ROOT, env=env, stdout=log,
                                    stderr=subprocess.STDOUT, timeout=timeout)
        report['commands'].append(dict(label=label, argv=command, exit_code=result.returncode,
                                       log_sha256=sha(out/(label+'.log'))))
        result.check_returncode()

    inputs = {}
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        exe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(exe) != BUILD:
            raise ValueError('Unsupported original executable')
        inputs[str(exe)] = BUILD
        for mode, options in [('native', ['-O2']), ('sanitized', ['-O1', '-g', '-fsanitize=address,undefined',
                                                               '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'])]:
            for label, src in controls.items():
                binary = out/(label+'-'+mode)
                run(label+'-'+mode+'-build', flags+options+src+['-o', binary])
                run(label+'-'+mode, [binary])
                report['controls'][label+'-'+mode] = dict(success=True, binary_sha256=sha(binary))
        for label, folder, script in [('queue', 'sprite-queue', 'test-sprite-queue.py'),
                                      ('visibility', 'sprite-visibility', 'test-sprite-visibility.py'),
                                      ('direction', 'animation-switch', 'test-animation-switch.py')]:
            parent = ROOT/'working/tests'/folder
            before = set(parent.glob('run-*')) if parent.exists() else set()
            run(label+'-original', [sys.executable, '-B', ROOT/'tools'/script], timeout=900)
            created = set(parent.glob('run-*'))-before
            if len(created) != 1:
                raise ValueError('Ambiguous comparison output: '+label)
            path = created.pop()/'report.json'
            value = json.loads(path.read_bytes())
            if not (value.get('all_match') or value.get('summary', {}).get('all_match')):
                raise ValueError('Original comparison did not pass: '+label)
            pins = value.get('source_and_input_sha256', value.get('sources', {}))
            for name, expected in pins.items():
                if name not in sources:
                    inputs[str(ROOT/name)] = expected
            report['comparisons'][label] = dict(result=value, report_sha256=sha(path))
        declaration = out/'controller-claims.json'
        declaration.write_text(json.dumps(dict(sources=sources, claims=report['original_claims'])))
        run('controller-original', [sys.executable, '-B', ROOT/'tools/test-animation-controller.py',
                                   '--output', out/'controller', '--claims', declaration])
        controller = json.loads((out/'controller/report.json').read_bytes())
        if not controller['success'] or not controller['original_target_text_unchanged']:
            raise ValueError('Controller comparison failed')
        report['comparisons']['controller'] = controller
        report['native_all_passed'] = all(c['success'] for c in report['controls'].values())
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['inputs_stable'] = all(sha(Path(p)) == h for p, h in inputs.items())
        if not report['sources_stable'] or not report['inputs_stable']:
            raise ValueError('Inputs changed during execution')
        report['inputs'] = inputs
        report['success'] = True
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        except BaseException:
            report['success'] = False
            raise
        finally:
            write_report(ROOT, args.report.resolve(), (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'], controls=len(report['controls']),
                          comparisons={k: v.get('result', v).get('summary', v.get('cases'))
                                       for k, v in report['comparisons'].items()}), indent=2))


if __name__ == '__main__':
    main()
