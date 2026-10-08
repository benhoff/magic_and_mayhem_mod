"""Scope identities for execution evidence; source bytes are pinned separately.

Evidence IDs/statuses are deliberately excluded so registering a new result does
not invalidate its own claim. Dependencies are explicit review inputs, not an
inferred compiler dependency graph.
"""
import hashlib
import json


ORIGINAL_KINDS = {'isolated_original', 'live_equivalence', 'live_replacement'}
EXECUTION_KINDS = ORIGINAL_KINDS | {'native_integration', 'live_observation'}


def fingerprint(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':'),
                                     ensure_ascii=True).encode()).hexdigest()


def required_sources(behavior):
    return (set(behavior['implementation']) | set(behavior['tests'])
            | set(behavior.get('validation_dependencies', [])))


def behavior_contract(behavior, builds):
    return fingerprint({
        'id': behavior['id'], 'kind': behavior['kind'], 'scope': behavior['scope'],
        'original': behavior['original'],
        'builds': {ref['build']: builds[ref['build']]['source_sha256'] for ref in behavior['original']},
        'implementation': sorted(behavior['implementation']),
        'tests': sorted(behavior['tests']),
        'dependencies': sorted(behavior.get('validation_dependencies', [])),
        'domain': behavior.get('validation_domain'),
    })


def scenario_contract(scenario):
    return fingerprint({k: scenario[k] for k in ('id', 'scope', 'level', 'behaviors', 'tests')})


def stage_kinds(stage, status):
    if stage == 'comparison':
        return ORIGINAL_KINDS
    if stage == 'replacement':
        return {'live_replacement'}
    if stage == 'integration':
        if status in {'headless', 'preview'}:
            return {'native_integration'}
        if status == 'live_equivalence':
            return {'live_equivalence', 'live_replacement'}
        return {status}
    return EXECUTION_KINDS


def validation_support(behavior, builds, evidence, sources, claims, scenarios, stale):
    """Current support per stage, with historical limitations retained explicitly."""
    result = {}
    contract = behavior_contract(behavior, builds)
    for stage in ('implementation', 'comparison', 'integration', 'replacement'):
        status = behavior['status'][stage]
        if status == 'none':
            result[stage] = {'state': 'none', 'evidence': []}
            continue
        candidates = [eid for eid in behavior['evidence']
                      if eid in evidence and evidence[eid]['kind'] in stage_kinds(stage, status)]
        current, historical, limitations = [], [], set()
        for eid in candidates:
            claim = claims.get(eid, {}).get(behavior['id'])
            if claim is None:
                limitations.add('unbound')
                continue
            if claim['contract_sha256'] != contract:
                limitations.add('scope_changed')
                continue
            needed = required_sources(behavior)
            if stage in {'integration', 'replacement'}:
                level = status if stage == 'integration' else 'live_equivalence'
                matching = [s for s in scenarios.values()
                            if behavior['id'] in s['behaviors'] and s['level'] == level
                            and eid in s['evidence']
                            and claim.get('scenarios', {}).get(s['id']) == scenario_contract(s)]
                # Each scenario proof must fingerprint its own test inputs too.
                matching = [s for s in matching if set(s['tests']) <= set(sources.get(eid, {}))]
                if not matching:
                    limitations.add('scenario_unbound')
                    continue
            if not needed or not needed <= set(sources.get(eid, {})):
                limitations.add('incomplete_sources')
                continue
            historical.append(eid)
            if eid in stale:
                limitations.add('stale')
                continue
            current.append(eid)
        result[stage] = {'state': 'current' if current else 'pending',
                         'evidence': current, 'historical_evidence': historical,
                         'limitations': sorted(limitations or ({'no_execution'} if not candidates else set()))}
    return result
