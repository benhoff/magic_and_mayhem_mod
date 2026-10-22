#!/usr/bin/env python3
"""Check the reviewed resident supplier map; never infer runtime validation."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT = ROOT/'research/runtime/coverage/high-resident-supplier-crosswalk-current-20261020.json'
DOMAINS = {'world', 'commands_hud', 'cpu_reply', 'bindings_retirement'}


def validate(root, register, crosswalk):
    if (crosswalk.get('schema') != 1 or crosswalk.get('static_review_only') is not True
            or crosswalk.get('status_promotions') != []):
        raise ValueError('Supplier review cannot promote execution status')
    by_id = {b['id']: b for b in register['behaviors']}
    aggregate = by_id[crosswalk['behavior']]
    if (aggregate['id'] != 'NR.public-resident-world' or
            crosswalk['status_at_review'] != aggregate['status']):
        raise ValueError('Changed aggregate or engine milestone')
    seen = set()
    if (not crosswalk.get('aggregate_glue_tests') or
            not set(crosswalk['aggregate_glue_tests']) <= set(aggregate['tests'])):
        raise ValueError('Unlinked aggregate glue test')
    used = set(crosswalk['aggregate_glue_tests'])
    for row in crosswalk['domains']:
        if row['id'] not in DOMAINS or row['id'] in seen:
            raise ValueError('Unknown or duplicate resident domain')
        seen.add(row['id'])
        suppliers = row['suppliers']
        if (not suppliers or len(set(suppliers)) != len(suppliers) or
                any(s not in by_id or s == aggregate['id'] for s in suppliers)):
            raise ValueError('Missing independent registered supplier')
        if not row.get('scope') or not row.get('remaining_boundary'):
            raise ValueError('Supplier scope and remaining boundary required')
        implementations = {p for s in suppliers for p in by_id[s]['implementation']}
        tests = {p for s in suppliers for p in by_id[s]['tests']}
        if not row['tests'] or not set(row['tests']) <= tests:
            raise ValueError('Unlinked supplier test')
        used.update(row['tests'])
        if not row['witnesses']:
            raise ValueError('Missing code witness')
        for witness in row['witnesses']:
            p = witness['path']
            allowed = (implementations if witness.get('role') == 'supplier' else
                       set(aggregate['implementation']) if witness.get('role') == 'aggregate_glue' else set())
            if p not in allowed:
                raise ValueError('Witness is outside its supplier implementation')
            target = (root/p).resolve()
            if not target.is_relative_to(root.resolve()):
                raise ValueError('Escaping witness path')
            lines = evidence_text(target).splitlines()
            at = witness['line']
            if not 1 <= at <= len(lines) or lines[at-1].strip() != witness['text']:
                raise ValueError('Code witness moved or changed')
            used.add(p)
    if seen != DOMAINS:
        raise ValueError('Missing resident integration domain')
    sources = crosswalk['source_sha256']
    if not used <= sources.keys():
        raise ValueError('Incomplete witness/test source closure')
    for p, expected in sources.items():
        target = (root/p).resolve()
        if (Path(p).is_absolute() or not target.is_relative_to(root.resolve()) or
                target.relative_to(root.resolve()).parts[0] in {'working', 'original', '.git'} or
                hashlib.sha256(evidence_bytes(target)).hexdigest() != expected):
            raise ValueError('Unsafe or changed reviewed source: '+p)
    return {'success': True, 'domains': sorted(seen), 'static_review_only': True,
            'implementation_supplied': not crosswalk['concrete_missing_required_code'],
            'engine_validation_granted': False, 'status_promotions': []}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--crosswalk', type=Path, default=DEFAULT)
    a = p.parse_args()
    result = validate(ROOT, json.loads(evidence_text(ROOT/'research/runtime/coverage/register.json')),
                      json.loads(evidence_text(a.crosswalk)))
    print(json.dumps(result))


if __name__ == '__main__':
    main()
