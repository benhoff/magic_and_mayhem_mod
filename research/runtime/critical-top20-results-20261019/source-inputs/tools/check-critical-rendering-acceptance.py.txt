#!/usr/bin/env python3
"""Assess complete software/hardware cohorts against an explicit output policy."""
import argparse
import hashlib
import json
from pathlib import Path

from critical_rendering_acceptance import POLICY, PROFILES, assess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--policy', choices=[POLICY], required=True)
    parser.add_argument('--software-report', type=Path, required=True)
    parser.add_argument('--hardware-report', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    sources = {name: hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in
               ('tools/critical_rendering_acceptance.py', 'tools/check-critical-rendering-acceptance.py',
                'tools/native_render_driver.py', 'tests/test-critical-rendering-acceptance.py')}
    inputs, reports, claims = {}, {}, []
    for backend in ('software', 'hardware'):
        path = getattr(args, backend+'_report').resolve()
        raw = path.read_bytes()
        inputs[str(path)] = hashlib.sha256(raw).hexdigest()
        bundle = json.loads(raw)
        if bundle.get('sources_stable') is not True or set(bundle.get('profiles', {})) != set(PROFILES):
            raise ValueError('Incomplete source-bound backend bundle: '+backend)
        reports[backend] = {}
        for name, entry in bundle['profiles'].items():
            child = Path(entry['record']).resolve()
            data = child.read_bytes()
            if hashlib.sha256(data).hexdigest() != entry['sha256']:
                raise ValueError('Changed complete profile report: '+name)
            inputs[str(child)] = entry['sha256']
            record = json.loads(data)
            for source, digest in record['sources'].items():
                target = (ROOT/source).resolve()
                if (Path(source).is_absolute() or not target.is_relative_to(ROOT) or
                        source.split('/')[0] in ('original', 'working', '.git') or
                        hashlib.sha256(target.read_bytes()).hexdigest() != digest or
                        source in sources and sources[source] != digest):
                    raise ValueError('Changed or conflicting execution source: '+source)
                sources[source] = digest
            reports[backend][name] = record
            for claim in record.get('claims', []):
                if claim not in claims:
                    claims.append(claim)
    result = assess(reports)
    result.update(sources=sources, claims=claims, inputs=inputs,
                  inputs_stable=all(hashlib.sha256(Path(p).read_bytes()).hexdigest() == h
                                    for p, h in inputs.items()),
                  sources_stable=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest() == h
                                     for p, h in sources.items()))
    if not result['inputs_stable'] or not result['sources_stable']:
        raise ValueError('Acceptance inputs changed during assessment')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    with args.report.open('x') as output:
        json.dump(result, output, indent=2)
        output.write('\n')
    print(json.dumps({key: result[key] for key in ('success', 'policy', 'accepted_profiles',
                                                  'legacy_diagnostics', 'whole_renderer_accepted')}))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
