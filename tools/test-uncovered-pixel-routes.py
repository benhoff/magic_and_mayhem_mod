#!/usr/bin/env python3
"""Execute every pending optional pixel route against pinned original code."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

from pixel_route_matrix import comparison_matrix

ROOT = Path(__file__).resolve().parents[1]
TARGETS = 'research/runtime/native-pixel-targets-reviewed-20261015.json'
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RUNNERS = {'pixel': 'pixel', 'minimap': 'minimap', 'colour_rect': 'colour-rect', 'raster': 'raster'}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing to overwrite evidence')
    declaration = json.loads(a.claims.read_text())
    if not declaration['claims'] or any(sha(ROOT / n) != h for n, h in declaration['sources'].items()):
        p.error('Missing claims or changed prospective sources')
    if TARGETS not in declaration['sources']:
        p.error('Pending-route inventory must be prospectively fingerprinted')
    pending = [r['route'] for r in json.loads((ROOT / TARGETS).read_text())['remaining_route_reachability']]
    if len(pending) != 24:
        p.error('Pending-route inventory changed; review the new denominator')
    parent = ROOT / 'working/tests/uncovered-pixel-routes'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, claims=declaration['claims'], sources=declaration['sources'],
                  scope='All 24 previously unexercised optional routes through actual production splices versus pinned original wrappers. Full physical output and stated caller-state checks in RGB565/555. Supplied Win32/owned-plane adapters; isolated comparison, no new game-caller or complete replacement claim.',
                  original_sha256=BUILD, original_game_executed=False, original_work_bypassed=False,
                  complete_replacement=False, comparisons={}, commands=[])
    try:
        for family, stem in RUNNERS.items():
            command = [sys.executable, '-B', str(ROOT / ('tools/test-' + stem + '-splice.py')),
                       '--claims', str(a.claims.resolve())]
            result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=900)
            log = out / (family + '.log')
            log.write_text(result.stdout + result.stderr)
            report['commands'].append(dict(command=command, exit_code=result.returncode,
                                           log=str(log.relative_to(ROOT)), sha256=sha(log)))
            if result.returncode:
                raise ValueError(f'{family} comparison failed; inspect {log}')
            child = Path(result.stdout.strip().splitlines()[-1])
            if not child.resolve().is_relative_to(ROOT / 'working/tests') or child.name != 'report.json':
                raise ValueError('Unexpected comparison report location')
            record = json.loads(child.read_text())
            if record.get('success') is not True or record.get('sources_stable') is not True or record.get('inputs_stable') is not True:
                raise ValueError('Incomplete original comparison: ' + family)
            if record.get('original_sha256') != BUILD:
                raise ValueError('Original comparison build changed: ' + family)
            if record['sources'] != declaration['sources'] or record['claims'] != declaration['claims']:
                raise ValueError('Child comparison scope/provenance changed')
            report['comparisons'][family] = dict(path=str(child.relative_to(ROOT)), sha256=sha(child), original_sha256=BUILD,
                                                  comparison=record['comparison'])
            print(f'{family}: independently matched original output', flush=True)
        report['matrix'] = comparison_matrix({f: r['comparison'] for f, r in report['comparisons'].items()}, pending)
        report['success'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        report['sources_stable'] = all(sha(ROOT / n) == h for n, h in declaration['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        (out / '.retain').write_text('Per-route original/native comparison acquisition; ' + str(a.report) + '\n')
        a.report.parent.mkdir(parents=True, exist_ok=True)
        with a.report.open('x') as stream:
            json.dump(report, stream, indent=2)
            stream.write('\n')
        print(a.report, flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
