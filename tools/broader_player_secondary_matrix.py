"""Bind the reviewed secondary scene continuation while preserving retained cases."""
import sqlite3
from pathlib import Path
from broader_player_current_interaction_matrix import P, BINARY
from broader_player_current_interaction_matrix import current as prior_current
from broader_player_discovery_matrix import replacements, replacement
from broader_secondary_budget import bound, predecessors
from broader_secondary_staged_scene import verify_staged
from broader_secondary_subprocess import verify_selection
from broader_player_acceptance import checked
from rendering_pilot import require
CATALOG = P + 'secondary-source-catalog-02.json'


def specs(reader):
    catalog = checked(reader, dict(record=CATALOG, sha256=reader.digest(CATALOG)))
    require(catalog['kind'] == 'broader_secondary_caller_continuation_catalog'
            and catalog['total_positive_cases'] == 90 and catalog['total_positive_original_starts'] == 98
            and catalog['retained_current_cases'] == catalog['remaining_positive_cases'] == 45,
            'Changed90/98 secondary runtime domain')
    out = []
    for binding in catalog['case_bindings']:
        phase = checked(reader, binding['plan'])
        cases = [v for v in phase['broader_cases'] if v['id'] == binding['case_id']]
        require(len(cases) == 1, 'Missing unique secondary phase case')
        if binding['role'] == 'strict':
            name = phase['actual_context_analysis_plan']
            analysis = checked(reader, dict(record=name, sha256=reader.digest(name)))
            require(analysis['strict_phase'] == binding['plan'], 'Strict analysis belongs to another phase')
        out.append(dict(plan=binding['plan']['record'], case=cases[0], role=binding['role'], family=binding['family']))
    require(len(out) == len({v['case']['id'] for v in out}) == 90
            and sum(v['case']['reserved_original_games'] for v in out) == 98, 'Changed positive denominator')
    return out


def requests(root, reader):
    with sqlite3.connect('file:' + str(Path(root) / 'working/broader-player/requests.sqlite3') + '?mode=ro', uri=True) as db:
        db.row_factory = sqlite3.Row
        rows = [dict(v) for v in db.execute('select *from requests order by started_ns')]
    plan = reader.json(P + 'plan-45.json')
    limit = bound(reader, plan)
    charge = predecessors(reader, rows, plan)
    return {v['case_id']: v for v in rows}, dict(reserved_additional_original_starts=charge,
        maximum_additional_original_starts=limit, maximum_combined_original_starts=255,
        prior_WP10_original_starts=8, all_reservations_retained=True, rows=rows)


def current(root, reader, spec, row):
    value = prior_current(root, reader, spec, row)
    phase = reader.json(spec['plan'])
    if 'scene_runtime_variant' not in phase:
        return value
    phase_ref = dict(record=spec['plan'], sha256=reader.digest(spec['plan']))
    name = P + spec['case']['id'] + '-runtime-variant-verification-01.json'
    recorded = checked(reader, dict(record=name, sha256=reader.digest(name)))
    require(recorded['kind'] == 'current_secondary_runtime_pre_admission_verification'
            and recorded['phase'] == phase_ref and recorded['acquisition'] == value['acquisition']
            and recorded['validation'] == value['validation']
            and recorded['actual_ledger_rows_renamed'] is False
            and all(reader.digest(n) == h for n, h in recorded['sources'].items()),
            'Missing exact pre-admission runtime variant verification')
    actual = verify_staged(reader, root, phase_ref, value['acquisition'], value['validation'])
    require(actual['verified'] and recorded['scene_sha256'] == actual['scene_sha256']
            and recorded['actual_compiled_header_sha256'] == actual['actual_compiled_header_sha256']
            and recorded['staged_compiler_manifest'] == actual['staged_compiler_manifest'],
            'Current staged runtime variant no longer matches admission')
    selection,_=verify_selection(reader,root,phase_ref,spec['case']['report'])
    require(recorded['actual_subprocess_selection']==selection,'Actual preparer selection changed')
    value['actual_subprocess_selection']=selection
    value['runtime_variant'] = dict(record=name, sha256=reader.digest(name))
    value['runtime_variant_current'] = actual
    value['phase'] = dict(value['phase'], derived_runtime_variant_verified=True,
                         actual_compiled_header_sha256=actual['actual_compiled_header_sha256'],
                         actual_scene_sha256=actual['scene_sha256'], physical_and_compiled_sources_distinct=True)
    return value
