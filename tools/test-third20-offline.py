#!/usr/bin/env python3
"""Bounded third-twenty capture/storage controls and retained normalization replay."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]
IDS = ['NR.drawing-backing-provenance', 'NR.drawing-canvas-storage-transitions',
       'NR.drawing-copy-failure-provenance', 'NR.drawing-storage-query-lock-return',
       'NR.storage-session-corpus']
TESTS = ['test-drawing-backing-provenance.py', 'test-drawing-storage-transitions.py',
         'test-drawing-copy-failure-provenance.py', 'test-drawing-storage-query-returns.py',
         'test-drawing-copy-order.py', 'test-drawing-storage-retention.py']


def sha(path):
    return hashlib.sha256(evidence_bytes(path)).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require a new output under working/tests')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Third-twenty offline controls and immutable-input replay.\n')
    register = json.loads(evidence_text(ROOT/'research/runtime/coverage/register.json'))
    by_id = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    names = set().union(*(required_sources(by_id[bid]) for bid in IDS))
    names.update('tests/'+name for name in TESTS)
    names.update(['tools/test-third20-offline.py', 'tools/coverage_claims.py'])
    sources = {name: sha(ROOT/name) for name in sorted(names)}
    claims = [dict(behavior=bid, contract_sha256=behavior_contract(by_id[bid], builds)) for bid in IDS]
    (out/'claims.json').write_text(json.dumps(dict(sources=sources, claims=claims), indent=2)+'\n')
    report = dict(schema=1, success=False, sources=sources, claims=claims, controls=[],
                  original_game_executed=False, native_pixels_compared=False,
                  physical_backing_owned=False, live_replacement=False,
                  scope='Offline parser/exit/barrier controls and retained corrected storage normalization. '
                        'No renewed original execution, all-writer/physical ownership or full journey acceptance.')

    def run(label, command):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(v) for v in command], cwd=ROOT, stdout=log,
                           stderr=subprocess.STDOUT, check=True, timeout=600)

    inputs = {}
    verified = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        for name in TESTS:
            run(name, [sys.executable, '-B', ROOT/'tests'/name])
            text = evidence_text(out/(name+'.log'))
            count = int(re.search(r'Ran (\d+) tests?', text).group(1))
            report['controls'].append(dict(test=name, tests=count, success=True))
        from drawing_recorded_session import admit
        from drawing_corpus import artifact
        from drawing_copy_order import normalize
        catalog_path = ROOT/'tests/fixtures/drawing-storage-corrected-session/session.json'
        catalog = json.loads(evidence_text(catalog_path))
        raw, records, frames, _ = admit(ROOT, catalog, register)
        original_path = ROOT/next(e['record'] for e in register['evidence'] if e['id']==catalog['evidence'])
        expected_path = ROOT/'research/runtime/egl-destination-native-corrected-20261016.json'
        original = json.loads(evidence_text(original_path))
        expected = json.loads(evidence_text(expected_path))['copy_order']
        reads = [f for f in frames if f['fill'] is None and not f.get('background')]
        reads += [f['background'] for f in frames if 'sequence' in f.get('background', {})]
        normalized, proof = normalize(raw, records, artifact(ROOT, catalog['traces']['points']), original, reads)
        if proof['normalized_sha256'] != expected['normalized_sha256']:
            raise ValueError('Retained corrected storage normalization changed')
        if len(frames)!=222 or proof['bound_driver_copies']!=expected['bound_driver_copies']:
            raise ValueError('Retained corrected storage cohort changed')
        for path in (catalog_path, original_path, expected_path):
            inputs[path.relative_to(ROOT).as_posix()] = sha(path)
        report.update(retained_normalization=dict(frames=len(frames), proof=proof,
                      output_bytes=len(normalized), historical_normalization_matches=True),
                      unit_tests=sum(c['tests'] for c in report['controls']), success=True)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            verified = True
        except (OSError, subprocess.SubprocessError) as error:
            report['cleanup_error'] = str(error)
            report['success'] = False
        report.update(sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()),
                      inputs=inputs, inputs_stable=all(sha(ROOT/n)==h for n,h in inputs.items()),
                      original_manifest_verified_before_after=verified)
        report['success'] &= report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(json.dumps({k: report.get(k) for k in ('success', 'unit_tests', 'error')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
