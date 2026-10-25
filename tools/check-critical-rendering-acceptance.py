#!/usr/bin/env python3
"""Assess complete software/hardware cohorts against an explicit output policy."""
import argparse
import hashlib
import json
from pathlib import Path

from critical_rendering_acceptance import POLICY, PROFILES, assess
from coverage_evidence import evidence_digest, read_bytes, write_report
from rendering_session_contracts import declared_source

ROOT = Path(__file__).resolve().parents[1]


def main(argv=None, root=ROOT):
    root = Path(root).resolve()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--policy', choices=[POLICY], required=True)
    parser.add_argument('--software-report', type=Path, required=True)
    parser.add_argument('--hardware-report', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--claims', type=Path, help='Prospective acceptance-policy declaration')
    args = parser.parse_args(argv)
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    sources = {name: evidence_digest(root/name, root=root) for name in
               ('tools/critical_rendering_acceptance.py', 'tools/check-critical-rendering-acceptance.py',
                'tools/native_render_driver.py', 'tools/coverage_evidence.py',
                'tools/rendering_session_contracts.py', 'tests/test-critical-rendering-acceptance.py')}
    inputs, reports, claims = {}, {}, []
    if args.claims:
        raw = read_bytes(args.claims)
        declaration = json.loads(raw)
        if not declaration.get('claims') or not declaration.get('sources'):
            raise ValueError('Acceptance requires nonempty prospective claims and sources')
        claims = declaration['claims']
        inputs[str(args.claims.resolve())] = hashlib.sha256(raw).hexdigest()
        for name, digest in declaration['sources'].items():
            if (evidence_digest(declared_source(root, name), root=root) != digest or
                    name in sources and sources[name] != digest):
                raise ValueError('Prospective acceptance source changed: '+name)
            sources[name] = digest
    for backend in ('software', 'hardware'):
        path = getattr(args, backend+'_report').resolve()
        raw = read_bytes(path)
        inputs[str(path)] = hashlib.sha256(raw).hexdigest()
        bundle = json.loads(raw)
        if bundle.get('sources_stable') is not True or set(bundle.get('profiles', {})) != set(PROFILES):
            raise ValueError('Incomplete source-bound backend bundle: '+backend)
        reports[backend] = {}
        for name, entry in bundle['profiles'].items():
            child = (root/entry['record']).resolve()
            data = read_bytes(child)
            if hashlib.sha256(data).hexdigest() != entry['sha256']:
                raise ValueError('Changed complete profile report: '+name)
            inputs[str(child)] = entry['sha256']
            record = json.loads(data)
            for source, digest in record['sources'].items():
                target = declared_source(root, source)
                if (evidence_digest(target, root=root) != digest or
                        source in sources and sources[source] != digest):
                    raise ValueError('Changed or conflicting execution source: '+source)
                sources[source] = digest
            reports[backend][name] = record
            if not args.claims:
                for claim in record.get('claims', []):
                    if claim not in claims:
                        claims.append(claim)
    result = assess(reports)
    result.update(sources=sources, claims=claims, inputs=inputs,
                  inputs_stable=all(hashlib.sha256(read_bytes(Path(p))).hexdigest() == h
                                    for p, h in inputs.items()),
                  sources_stable=all(evidence_digest(root/p, root=root) == h
                                     for p, h in sources.items()))
    if not result['inputs_stable'] or not result['sources_stable']:
        raise ValueError('Acceptance inputs changed during assessment')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    write_report(root, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps({key: result[key] for key in ('success', 'policy', 'accepted_profiles',
                                                  'legacy_diagnostics', 'whole_renderer_accepted')}))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
