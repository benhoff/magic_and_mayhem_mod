#!/usr/bin/env python3
"""Compare owned canvas startup against the pinned original initializer."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    with path.open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require a new retained output under working/tests')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Pinned canvas initializer comparison.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'))
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == 'RS.canvas-startup-initializer')
    sources = {p: sha(ROOT/p) for p in sorted(required_sources(behavior))}
    report = dict(schema=1, success=False, scope=behavior['scope'], sources=sources,
                  original_sha256=BUILD, claims=[dict(behavior=behavior['id'],
                  contract_sha256=behavior_contract(behavior, {b['id']: b for b in register['builds']}))],
                  live_replacement=False, physical_driver_equivalence=False,
                  original_pixels_used_as_native_inputs=False)
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')

    def run(label, command, stream=None):
        print(label, flush=True)
        with (out/(label+'.log')).open('xb') as log:
            subprocess.run(list(map(str, command)), cwd=ROOT, env=env, check=True,
                           stdout=stream if stream is not None else log, stderr=log, timeout=180)

    before = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        before = True
        executable = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(executable) != BUILD:
            raise ValueError('Unsupported original executable')
        fixture = ROOT/'tests/canvas-initializer-reference.cpp'
        cohorts = [('native', 'g++', []),
                   ('original', 'g++', ['-m32', '-fno-pie', '-no-pie', '-DMNM_ORIGINAL_REFERENCE']),
                   ('sanitized', 'clang++', ['-fsanitize=address,undefined', '-fno-omit-frame-pointer'])]
        hashes = {}
        for label, compiler, flags in cohorts:
            run(label+'-build', [compiler, '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                                *flags, '-MD', '-MF', out/(label+'.d'), fixture, '-o', out/label])
            for name in (out/(label+'.d')).read_text().replace('\\\n', ' ').split()[1:]:
                path = Path(name)
                if path.is_file() and path.resolve().is_relative_to(ROOT):
                    relative = path.resolve().relative_to(ROOT).as_posix()
                    if relative not in sources:
                        raise ValueError('Undeclared compiler input: '+relative)
            output = out/(label+'.bin')
            with output.open('xb') as stream:
                run(label+'-execute', [out/label, *([executable] if label == 'original' else [])], stream)
            if (out/(label+'-execute.log')).read_text().strip() != 'cases=46656':
                raise ValueError('Unexpected case denominator: '+label)
            hashes[label] = sha(output)
        sizes = {label: (out/(label+'.bin')).stat().st_size for label in hashes}
        if len(set(hashes.values())) != 1 or set(sizes.values()) != {18475776}:
            raise ValueError('Original/native/sanitized outputs differ')
        report.update(success=True, cases=46656, bytes_compared=18475776,
                      output_sha256=hashes, selected_original_bodies_unchanged=True)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if before:
            try:
                run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
                report['original_manifest_verified_before_after'] = True
            except (OSError, subprocess.SubprocessError) as error:
                report.update(success=False, preservation_error=str(error))
        report['sources_stable'] = all(sha(ROOT/p) == digest for p, digest in sources.items())
        report['success'] = report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
