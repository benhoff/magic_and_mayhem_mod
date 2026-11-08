"""Batch retained proofs through their existing, requirement-specific admission.

No collectors, game processes, historical hash rewriting or cached gate passes.
One Reader and one immutable result serve the whole bounded batch. Accounting
and the normal finish-work commit gate remain separate, required operations.
"""
from copy import deepcopy
import importlib.util
from pathlib import Path
from time import perf_counter

from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_pilot import require

PLAN = 'docs/native-rendering-closure.json'
ADAPTERS = {
    'cpu': ('join-rendering-operations-acceptance', 'operation.cpu-write-readback'),
    'full-fill': ('join-rendering-full-fill-acceptance', 'operation.fill-full'),
    'pan': ('join-rendering-pan-acceptance', 'journey.camera-motion.pan'),
    'markers': ('join-rendering-markers', 'journey.hud-minimap-tooltips.markers'),
}
CONTROLS = ('tools/rendering_closeout.py', 'tools/close-rendering-batch.py',
            'tests/test-rendering-closeout-batch.py', 'tools/rendering_acceptance.py',
            'tools/coverage_claims.py', PLAN)
BID = 'NP.rendering-closeout-batch'
SID = 'native.rendering-closeout-batch'


def control_sources(reader):
    from coverage_claims import required_sources
    register = reader.json('research/runtime/coverage/register.json')
    behaviors = [b for b in register['behaviors'] if b['id'] == BID]
    names = set(CONTROLS)
    if behaviors:
        names.update(required_sources(behaviors[0]))
    return {n: reader.digest(n) for n in sorted(names)}


def load(root, name):
    require(name in ADAPTERS, 'Unknown closeout adapter: ' + name)
    path = root / 'tools' / (ADAPTERS[name][0] + '.py')
    spec = importlib.util.spec_from_file_location('closeout_' + name.replace('-', '_'), path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def inventory(plan, report):
    """Check complete identities/definitions and totals before any promotion."""
    validate(plan)
    definitions = requirements(plan)
    require(report.get('success') is True and report['dossier_identity'] == fingerprint(definitions),
            'Unaccepted or different dossier')
    expected = {(r['configuration'], r['id']): r for r in definitions}
    rows = report['requirements']
    keys = [(r['configuration'], r['id']) for r in rows]
    require(len(keys) == len(set(keys)) == len(expected) and set(keys) == set(expected),
            'Missing, duplicate or unknown requirement')
    for row, key in zip(rows, keys):
        require(all(row.get(k) == v for k, v in expected[key].items()),
                'Requirement definition changed: ' + str(key))
        require(row['state'] in ('passed', 'pending', 'failed'), 'Unknown requirement state')
        require(row['state'] != 'passed' or row.get('admitted_execution_evidence'),
                'Passed row lacks admitted execution')
    require(report['passed_requirements'] == sum(r['state'] == 'passed' for r in rows)
            and report['pending_requirements'] == sum(r['state'] == 'pending' for r in rows),
            'Inventory counters differ from rows')
    require(report.get('sources') and report.get('scope') and report.get('decisions'),
            'Missing source phases, scope or separate decisions')
    return keys


def preserve(plan, parent, rows, leaf):
    """An adapter may only admit its two pending configuration rows."""
    require(len(rows) == len(parent['requirements']), 'Adapter changed row count')
    wanted = {(cfg['id'], leaf) for cfg in plan['completion_contract']['configurations']}
    changed = set()
    for old, new in zip(parent['requirements'], rows):
        key = old['configuration'], old['id']
        require((new['configuration'], new['id']) == key, 'Adapter reordered identities')
        if old == new:
            continue
        require(key in wanted and old['state'] == 'pending' and new['state'] == 'passed',
                'Adapter changed an unrelated, failed or accepted row')
        require({k: v for k, v in old.items() if k not in ('state', 'admitted_execution_evidence')}
                == {k: v for k, v in new.items() if k not in ('state', 'admitted_execution_evidence')},
                'Adapter changed requirement metadata')
        require(new.get('admitted_execution_evidence'), 'Missing positive admission')
        changed.add(key)
    require(changed == wanted, 'Adapter must admit exactly its pending configuration pair')


def batch(root, reader, parent_name, components):
    require(components and len(components) <= len(ADAPTERS), 'Choose one to four retained components')
    names = [name for name, _ in components]
    require(len(set(names)) == len(names) and set(names) <= set(ADAPTERS),
            'Duplicate or unknown component')
    started = perf_counter()
    plan = reader.json(PLAN)
    parent_hash = reader.digest(parent_name)
    original = reader.json(parent_name)
    inventory(plan, original)
    controls = control_sources(reader)
    result = deepcopy(original)
    steps = []
    for name, record in components:
        tick = perf_counter()
        component_hash = reader.digest(record)
        analysis = reader.json(record)
        sources = analysis['analysis_sources']
        require(sources and all(reader.digest(n) == h for n, h in sources.items()),
                'Retained component source changed: ' + name)
        adapter = load(root, name)
        adapter_name = 'tools/' + ADAPTERS[name][0] + '.py'
        adapter_hash = reader.digest(adapter_name)
        snapshot = deepcopy(result)
        # Existing admission owns every caller/output/lifetime requirement.
        if name in ('cpu', 'full-fill'):
            joined = adapter.join(reader, deepcopy(analysis), deepcopy(snapshot), plan)
            rows = joined['requirements']
            require(joined['decisions'] == snapshot['decisions'], 'Adapter changed decisions')
        else:
            rows = adapter.join(plan, deepcopy(snapshot), deepcopy(analysis))
        leaf = ADAPTERS[name][1]
        preserve(plan, snapshot, rows, leaf)
        require(all(reader.digest(n) == h for n, h in sources.items())
                and reader.digest(record) == component_hash
                and reader.digest(adapter_name) == adapter_hash, 'Component changed during admission')
        result.update(requirements=rows,
                      passed_requirements=sum(r['state'] == 'passed' for r in rows),
                      pending_requirements=sum(r['state'] == 'pending' for r in rows),
                      closed_leaf_ids=sorted(set(result['closed_leaf_ids']) | {leaf}))
        # Keep conflicting historical source hashes in their original phases.
        result['sources'].update({n: h for n, h in analysis['sources'].items()
                                  if n not in result['sources']})
        steps.append(dict(adapter=name, record=record, sha256=component_hash,
                          adapter_source=dict(path=adapter_name, sha256=adapter_hash),
                          component_analysis_sources=sources, admitted_leaf=leaf,
                          seconds=perf_counter() - tick))
    require(reader.digest(parent_name) == parent_hash
            and all(reader.digest(n) == h for n, h in controls.items()),
            'Parent or runner controls changed during batch')
    # Recheck all earlier components too; a later admission cannot mask drift.
    for step in steps:
        require(reader.digest(step['record']) == step['sha256']
                and reader.digest(step['adapter_source']['path']) == step['adapter_source']['sha256']
                and all(reader.digest(n) == h for n, h in step['component_analysis_sources'].items()),
                'Earlier component changed during batch')
    inventory(plan, result)
    return dict(schema=1, success=True, kind='retained_rendering_closeout_batch',
                **{key: result[key] for key in ('requirements', 'dossier_identity', 'decisions',
                    'sources', 'closed_leaf_ids', 'passed_requirements', 'pending_requirements')},
                scope='Retained component admissions only: ' + ', '.join(names)
                      + '. Preserve all other rows, historical source phases, failures and both decisions. '
                        'No game, native execution, pixel replay, census, audit or commit gate is supplied by this join.',
                parent=dict(record=parent_name, sha256=parent_hash), components=steps,
                analysis_sources=controls, analysis_sources_stable=True,
                newly_passed_requirements=result['passed_requirements'] - original['passed_requirements'],
                original_game_runs=0, current_native_engine_execution=False,
                new_native_executions=0, new_original_oracle_runs=0, new_pixel_comparisons=0,
                case_execution_repeated=False, historical_failures_retired=False,
                timings_seconds=dict(total=perf_counter() - started,
                                     components=sum(s['seconds'] for s in steps)))
