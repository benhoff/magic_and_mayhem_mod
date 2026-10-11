#!/usr/bin/env python3
"""Replay DRAW-02 native inputs and compare complete surfaces/World canvases."""
import argparse
import json
from pathlib import Path
import subprocess

from drawing_corpus import CATALOG, ROOT, check, sha


def output_path(path):
    path = path.resolve()
    if path.exists() or not path.is_relative_to(ROOT):
        raise ValueError('Report must be new and within the repository')
    if any(path.is_relative_to((ROOT / d).resolve()) for d in ('original', '.git')):
        raise ValueError('Cannot write reports into original/ or .git/')
    return path


def sources():
    # Include reviewed compiler inputs, shared headers, and portable data. Do not
    # fingerprint installed game/media files or rewrite historical evidence.
    paths = set()
    for directory in ('renderer', 'assets', 'compat/legacy', 'protocols/include'):
        for p in (ROOT / directory).rglob('*'):
            if p.is_file() and (p.suffix in {'.cpp', '.hpp', '.c', '.h', '.S', '.cmake'} or p.name == 'CMakeLists.txt'):
                paths.add(p)
    for directory in ('tests/fixtures/drawing', 'tests/fixtures/surfaces'):
        paths.update(p for p in (ROOT / directory).rglob('*') if p.is_file())
    paths.update(ROOT / p for p in ('tools/drawing_corpus.py', 'tools/check-drawing-corpus.py',
                                   'tools/retain-drawing-corpus.py', 'tools/check-surface-fixtures.py',
                                   'tools/replay-render-capture.py', 'tools/rendering_coverage.py',
                                   'tools/coverage_audit.py', 'tests/test-drawing-corpus.py',
                                   'research/runtime/draw02-independent-comparison.md'))
    return {str(p.relative_to(ROOT)): sha(p.read_bytes()) for p in sorted(paths)}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--corpus', default=CATALOG)
    p.add_argument('--surface-executable', type=Path, required=True)
    p.add_argument('--world-executable', type=Path, required=True)
    p.add_argument('--headless', action='store_true')
    p.add_argument('--report', type=Path, required=True)
    p.add_argument('--claims', type=Path)
    p.add_argument('--require-complete', action='store_true', help='Fail while broader required scenarios remain pending')
    args = p.parse_args()
    try:
        report_path = output_path(args.report)
        fingerprints = sources()
        claims = json.loads(args.claims.read_text()) if args.claims else None
        if claims:
            for name, digest in claims['sources'].items():
                if sha((ROOT / name).read_bytes()) != digest:
                    raise ValueError('Prospective claim source changed: ' + name)
                fingerprints[name] = digest
        result = check(catalog_path=args.corpus, surface_executable=args.surface_executable,
                       world_executable=args.world_executable, headless=args.headless)
        if any(sha((ROOT / name).read_bytes()) != digest for name, digest in fingerprints.items()):
            raise ValueError('Comparison sources changed during replay')
        result.update(sources=fingerprints, sources_stable=True, inputs_stable=True)
        if claims:
            result['claims'] = claims['claims']
        report_path.parent.mkdir(parents=True, exist_ok=True)
        with report_path.open('x') as out:
            json.dump(result, out, indent=2)
            out.write('\n')
        print(json.dumps({k: result[k] for k in ('success', 'pixels_compared', 'mismatches', 'draw02_complete', 'artifacts')}))
        return int(not result['success'] or args.require_complete and not result['draw02_complete'])
    except (ValueError, OSError, KeyError, TypeError, StopIteration, subprocess.TimeoutExpired) as error:
        p.exit(2, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
