"""Select an exact retained interaction successor while preserving older predicates."""
import inspect
from broader_player_discovery_matrix import P, BINARY, specs, requests
from broader_player_discovery_matrix import current as prior_current
from broader_player_copy_matrix import current as original_current
from broader_player_runtime_caller_matrix import patched_source
from broader_player_acceptance import checked
from rendering_pilot import require
RECOVERED = 'bp40-world-interaction-default640-01'


def coordinator_record(reader, spec):
    cid = spec['case']['id']
    require(cid == RECOVERED and spec['family'] == 'interaction' and spec['role'] == 'world',
            'Interaction successor outside exact acquired context')
    phase = dict(record=spec['plan'], sha256=reader.digest(spec['plan']))
    name = P + cid + '-coordinator-02.json'
    value = checked(reader, dict(record=name, sha256=reader.digest(name)))
    old = value['previous_failed_coordinator']
    require(old['record'] == P + cid + '-coordinator-01.json'
            and reader.digest(old['record']) == old['sha256'], 'Interaction refusal identity changed')
    refusal = reader.json(old['record'])
    require(refusal['success'] is False and refusal['case'] == cid and refusal['plan'] == phase,
            'Interaction failure replaced or promoted')
    require(value['kind'] == 'broader_domain_case_coordinator' and value['case'] == cid
            and value['plan'] == phase and value['original_game_runs_added'] == 0
            and value['new_acquisition_performed'] is False, 'Interaction successor widened execution')
    execution = checked(reader, value['retained_analysis_execution'])
    require(execution['original_game_runs'] == execution['new_public_player_executions']
            == execution['new_request_reservations'] == 0 and execution['request_rows_unchanged'] is True
            and execution['original_acquisition_preserved'] is True
            and execution['only_admission_fields_changed'] is True, 'Not an exact retained-only analysis')
    require(execution['validation'] == value['validation'] and execution['acquisition'] == value['acquisition'],
            'Retained execution belongs to another result')
    return name


_namespace = dict(original_current.__globals__, _caller_coordinator_record=coordinator_record)
exec(compile(patched_source(inspect.getsource(original_current)), __file__, 'exec'), _namespace)


def current(root, reader, spec, row):
    if spec['case']['id'] != RECOVERED:
        return prior_current(root, reader, spec, row)
    return _namespace['current'](root, reader, spec, row)
