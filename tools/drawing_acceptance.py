"""Prospective DRAW-02 acceptance plan; planning never contributes coverage."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import json
from drawing_corpus import BUILD, ROOT, local, sha
from drawing_journeys import REQUIREMENTS, OPERATIONS, admit_suite, CATALOG

PLAN = 'tests/fixtures/drawing-journeys/acceptance.json'
FIELDS = {'id', 'actions', 'observe', 'owned_inputs', 'dependencies'}


def admit(root, plan):
    if set(plan) != {'schema', 'source_sha256', 'journeys', 'operations'}:
        raise ValueError('Acceptance plan fields changed')
    if type(plan['schema']) is not int or plan['schema'] != 1 or plan['source_sha256'] != BUILD:
        raise ValueError('Acceptance plan schema/build mismatch')
    if set(plan['journeys']) != set(REQUIREMENTS) or set(plan['operations']) != set(OPERATIONS):
        raise ValueError('Acceptance denominator changed')
    rows = []
    for scenario, required in REQUIREMENTS.items():
        entries = plan['journeys'][scenario]
        if [r.get('id') for r in entries] != list(required):
            raise ValueError('Journey requirements changed: ' + scenario)
        rows.extend(entries)
    for name in OPERATIONS:
        row = plan['operations'][name]
        if row.get('id') != name:
            raise ValueError('Operation identity changed')
        rows.append(row)
    for row in rows:
        if set(row) != FIELDS:
            raise ValueError('Requirement fields changed; evidence/pass labels are not a plan')
        for field in ('actions', 'observe', 'owned_inputs', 'dependencies'):
            value = row[field]
            if (not isinstance(value, list) or not value or
                    any(not isinstance(s, str) or not s.strip() for s in value) or
                    len(value) != len(set(value))):
                raise ValueError('Empty/duplicate acceptance specification: ' + field)
    suite = json.loads(evidence_text(local(root, CATALOG)))
    admit_suite(root, suite)
    return plan


def inventory(root=ROOT, path=PLAN):
    data = evidence_bytes(local(root, path))
    plan = admit(root, json.loads(data))
    return {
        'schema': 1, 'plan_sha256': sha(data), 'source_executable_sha256': BUILD,
        'journeys': plan['journeys'], 'operations': plan['operations'],
        'journey_count': len(REQUIREMENTS),
        'journey_requirement_count': sum(map(len, REQUIREMENTS.values())),
        'operation_count': len(OPERATIONS),
        'execution_performed': False, 'reference_complete': False,
        'draw02_complete': False,
        'scope': 'Prospective actions and observations only. Closure is derived from independently admitted execution by check-drawing-journeys.py.',
    }
