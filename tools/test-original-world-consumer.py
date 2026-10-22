#!/usr/bin/env python3
"""Compare the owned World consumer envelope with unchanged pinned PE32 code."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import subprocess

from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['RS.world-consumer-envelope']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--executable', type=Path, default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    a = p.parse_args()
    out = a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        p.error('Require a new retained output under working/tests')
    out.mkdir(parents=True)
    r = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bs = {b['id']: b for b in r['behaviors']}; builds = {b['id']: b for b in r['builds']}
    paths = set().union(*(required_sources(bs[i]) for i in IDS))
    paths.update(['tools/coverage_claims.py', 'tests/sprite-binary-reference.cpp',
                  'reconstruction/rendering/world_dispatch.hpp'])
    sources = {n: sha(ROOT/n) for n in sorted(paths)}
    claims = [{'behavior': i, 'contract_sha256': behavior_contract(bs[i], builds)} for i in IDS]
    report = {'schema': 1, 'success': False, 'claims': claims, 'sources': sources,
              'original_sha256': BUILD, 'scope': __doc__, 'live_replacement': False,
              'raster_pixels_compared': False, 'original_consumer_instructions_unchanged': False,
              'original_clip_setter_body_copied_unchanged': True,
              'helper_bodies_recovered': False}
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')

    def run(label, args, output=None):
        with (out/(label+'.log')).open('x') as log:
            if output:
                with (out/output).open('x') as target:
                    subprocess.run([str(x) for x in args], cwd=ROOT, stdout=target,
                                   stderr=log, timeout=180, check=True)
            else:
                subprocess.run([str(x) for x in args], cwd=ROOT, stdout=log,
                               stderr=subprocess.STDOUT, timeout=180, check=True)
    before = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify']); before = True
        if sha(a.executable) != BUILD:
            raise ValueError('Require unchanged pinned executable')
        for name, flags in [('native', []), ('original', ['-m32', '-DMNM_ORIGINAL_REFERENCE',
                                                       '-fno-pie', '-no-pie'])]:
            run(name+'-build', ['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                               *flags, ROOT/'tests/world-consumer-test.cpp', '-o', out/name])
            run(name, [out/name, *([a.executable] if flags else [])], name+'.jsonl')
        count = events = clip = final = 0
        with (out/'native.jsonl').open() as native, (out/'original.jsonl').open() as original:
            for n, o in itertools.zip_longest(native, original):
                if not n or not o:
                    raise ValueError('Missing native/original matrix row')
                actual, expected = json.loads(n), json.loads(o)
                if actual != expected:
                    (out/'mismatch.json').write_text(json.dumps(
                        {'native': actual, 'original': expected}, indent=2)+'\n')
                    raise ValueError('Consumer state differs; inspect retained mismatch')
                if actual['id'] != count:
                    raise ValueError('Duplicated or unordered matrix case')
                count += 1; events += len(actual['events'])
                clip += sum(e[0] == 0x57dc60 for e in actual['events'])
                final += sum(e[0] == 0x4d8ab0 for e in actual['events'])
        if count != 49248 or not clip or not final:
            raise ValueError('Incomplete declared consumer domain')
        if sha(a.executable) != BUILD:
            raise ValueError('Original input changed')
        report.update(success=True, cases=count, events_compared=events,
                      clip_calls_compared=clip, finalizer_calls_compared=final,
                      helper_receiver_identity_compared=True, clip_setup_compared=True,
                      row_y_and_metadata_compared=True, queue_traversal_compared=True,
                      original_consumer_instructions_unchanged=True,
                      binary_sha256={n: sha(out/n) for n in ['native', 'original']},
                      output_sha256={n: sha(out/(n+'.jsonl')) for n in ['native', 'original']})
    except (OSError, ValueError, subprocess.SubprocessError) as e:
        report['error'] = str(e)
    finally:
        if before:
            try:
                run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
                report['original_manifest_verified_before_after'] = True
            except subprocess.SubprocessError as e:
                report.update(success=False, preservation_error=str(e))
        report['sources_stable'] = all(sha(ROOT/n) == h for n, h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
