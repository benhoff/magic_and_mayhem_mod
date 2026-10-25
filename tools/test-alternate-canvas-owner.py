#!/usr/bin/env python3
"""Bounded native alternate ownership/borrowed-dispatch integration evidence."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import evidence_digest, read_json, write_report

ROOT = Path(__file__).resolve().parents[1]
BEHAVIOR = 'NP.alternate-canvas-owned-generation'
SCENARIO = 'ACO2.owned-dispatch-headless'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.exists() or not output.is_relative_to(ROOT / 'working/tests'):
        parser.error('Require a new owned working/tests output directory')
    output.mkdir(parents=True)
    (output / '.retain').write_text('Native alternate owner: compiler dependencies, execution and sanitizer logs.\n')
    (output / 'tmp').mkdir()
    register = read_json(ROOT / 'research/runtime/coverage/register.json', root=ROOT)
    behavior = next(b for b in register['behaviors'] if b['id'] == BEHAVIOR)
    scenario = next(s for s in register['scenarios'] if s['id'] == SCENARIO)
    sources = {p: evidence_digest(ROOT / p, root=ROOT) for p in sorted(required_sources(behavior))}
    report = dict(schema=1, success=False, sources=sources,
                  claims=[dict(behavior=BEHAVIOR,
                               contract_sha256=behavior_contract(behavior, {}),
                               scenarios={SCENARIO: scenario_contract(scenario)})],
                  commands=[], scope=behavior['scope'], original_artifacts_consumed=False,
                  original_allocation_recovered=False, original_game_executed=False,
                  application_installed=False, live_replacement=False)
    env = os.environ | dict(TMPDIR=str(output / 'tmp'),
                            ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
                            UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')

    def run(label, command):
        print(label, flush=True)
        log = output / (label + '.log')
        with log.open('x') as stream:
            process = subprocess.run(list(map(str, command)), cwd=ROOT, env=env,
                                     stdout=stream, stderr=subprocess.STDOUT, timeout=120)
        report['commands'].append(dict(label=label, argv=list(map(str, command)),
                                       exit=process.returncode,
                                       log_sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
        if process.returncode:
            raise ValueError(label + ': ' + log.read_text()[-2500:])
        return log.read_text()

    try:
        run('configure', ['cmake', '-S', ROOT / 'renderer/alternate-canvas',
                          '-B', output / 'cmake', '-DBUILD_TESTING=ON', '-DCMAKE_BUILD_TYPE=Release'])
        run('build', ['cmake', '--build', output / 'cmake', '-j2'])
        run('ctest', ['ctest', '--test-dir', output / 'cmake', '--output-on-failure'])
        report['release'] = json.loads(run('release', [output / 'cmake/alternate-canvas-owner-test']))
        for name, flags in [('native64', []), ('native32', ['-m32'])]:
            run(name + '-build', ['g++', '-std=c++17', '-O1', '-g', '-Wall', '-Wextra', '-Werror',
                                  '-pedantic', '-fno-pie', '-no-pie', '-fno-omit-frame-pointer',
                                  '-fsanitize=address,undefined', *flags, '-MMD', '-MF',
                                  output / (name + '.d'), ROOT / 'tests/alternate-canvas-owner-test.cpp',
                                  '-o', output / name])
            dependencies = shlex.split((output / (name + '.d')).read_text().replace('\\\n', ' ').split(':', 1)[1])
            for token in dependencies:
                path = Path(token).resolve()
                if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:
                    raise ValueError('Undeclared compiler input: ' + token)
            report[name] = json.loads(run(name, [output / name]))
            if report[name] != report['release']:
                raise ValueError('Execution denominators differ: ' + name)
        if report['release']['frames'] != 24 or report['release']['refusals'] < 60:
            raise ValueError('Missing expected frame/refusal controls')
        report['success'] = True
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        report['sources_stable'] = all(evidence_digest(ROOT / p, root=ROOT) == sha for p, sha in sources.items())
        report['success'] &= report['sources_stable']
        report['binaries'] = {p.relative_to(output).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                              for p in [output / 'native64', output / 'native32',
                                        output / 'cmake/alternate-canvas-owner-test'] if p.is_file()}
        digest = write_report(ROOT, args.report, (json.dumps(report, indent=2) + '\n').encode())
        print(json.dumps(dict(success=report['success'], sha256=digest, error=report.get('error'))), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
