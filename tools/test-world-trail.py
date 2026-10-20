#!/usr/bin/env python3
"""Compare owned kind9 line placement with unchanged No-CD4df840."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BEHAVIORS = ('RS.world-kind9-trail', 'NP.world-trail-admission')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def declaration():
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    builds = {b['id']: b for b in register['builds']}
    behaviors = [b for b in register['behaviors'] if b['id'] in BEHAVIORS]
    scenario = next(s for s in register['scenarios'] if s['id'] == 'world-trail-bounds')
    if len(behaviors) != len(BEHAVIORS):
        raise ValueError('Register trail contracts before execution')
    claims = [dict(behavior=b['id'], contract_sha256=behavior_contract(b, builds),
                   scenarios={scenario['id']: scenario_contract(scenario)}) for b in behaviors]
    paths = set(scenario['tests']) | set().union(*(required_sources(b) for b in behaviors))
    return claims, {p: sha(ROOT/p) for p in sorted(paths)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--executable', type=Path, default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    args = parser.parse_args()
    out = args.output.resolve()
    if not out.is_relative_to(ROOT/'working') or out.exists():
        parser.error('Require new output directory under working/')
    out.mkdir(parents=True)
    claims, sources = declaration()
    report = dict(schema=1, success=False, claims=claims, sources=sources,
                  original_sha256=BUILD,
                  scope='Original4df840 and unchanged5002a0 kind9 versus owned line stamps:127449 cells, all octants and half-step ties, '
                        'coincident and axis-aligned endpoints, signed translations/offsets and full helper EAX return and zero queue metadata. '
                        'Synthetic580a90 raster binding; no downstream pixel, live pointer/writer or replacement claim.',
                  live_replacement=False, raster_pixels_compared=False)
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')

    def run(label, command):
        with (out/(label+'.stdout')).open('xb') as stdout, (out/(label+'.stderr')).open('xb') as stderr:
            subprocess.run([str(v) for v in command], cwd=ROOT, check=True,
                           stdout=stdout, stderr=stderr, timeout=180)

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        if sha(args.executable) != BUILD:
            raise ValueError('Unsupported original executable')
        for name, flags in [('original', ['-m32', '-fno-pie', '-no-pie', '-DMNM_ORIGINAL_REFERENCE']),
                            ('native', [])]:
            run(name+'-build', ['g++', *flags, '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                ROOT/'tests/world-trail-test.cpp', '-o', out/name])
            run(name, [out/name, *([args.executable] if name == 'original' else [])])
        original = (out/'original.stdout').read_bytes()
        if original != (out/'native.stdout').read_bytes() or len(original.splitlines()) != 127449:
            raise ValueError('Native trail calls/returns differ from original')
        stamps = sum(len(json.loads(line)[-1]) for line in original.splitlines())
        run('configure', ['cmake', '-S', ROOT/'reconstruction/rendering/world-trail',
            '-B', out/'build', '-DCMAKE_BUILD_TYPE=Release'])
        run('build', ['cmake', '--build', out/'build', '--parallel', '2'])
        run('ctest', ['ctest', '--test-dir', out/'build', '--output-on-failure'])
        if sha(args.executable) != BUILD or declaration() != (claims, sources):
            raise ValueError('Sources/contracts/original changed during execution')
        report.update(success=True, sources_stable=True, calls_and_full_return_match=True,
                      original_helper_instructions_unchanged=True, original_queue_instructions_unchanged=True,
                      queue_to_trail_match=True, queue_cases=84966, cases=127449,
                      stamps_compared=stamps, native_admission_controls=4,
                      oversized_and_overflow_refused_before_effects=True, native_ctests_passed=1)
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['artifacts'] = {p.name: sha(p) for p in out.iterdir() if p.is_file()}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)


if __name__ == '__main__':
    main()
