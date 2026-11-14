"""Require the exact retained interaction conjunction before the unchanged final gate."""
import broader_discovery_final_gate as previous
from broader_player_acceptance import checked
from rendering_pilot import require
P = previous.P
same_context = previous.same_context
guarded_contexts = previous.guarded_contexts
caller_binding = previous.caller_binding
prior_context = previous.prior_context
RECOVERED = 'bp40-world-interaction-default640-01'
CONJUNCTION = P + 'current-interaction-retained-analysis-conjunction-01.json'


def reviewed_interaction(reader, values):
    ref = dict(record=CONJUNCTION, sha256=reader.digest(CONJUNCTION))
    value = checked(reader, ref)
    require(value['kind'] == 'reviewed_current_interaction_retained_analysis_conjunction'
            and value['case'] == RECOVERED and value['qualified_current_contexts'] == 45
            and value['request_rows'] == 152 and value['conservative_charges'] == 201,
            'Different retained interaction scope')
    require(value['original_game_runs'] == value['new_native_executions'] == value['new_request_reservations'] == 0
            and value['Native_runtime_compiler_player_controller_sources_changed'] is False
            and value['source_waivers'] == [] and value['old_reports_or_hashes_overwritten'] is False
            and value['historical_failures_retired'] is False, 'Retained interaction execution/history changed')
    require(value['total_positive_contexts'] == 90 and value['total_positive_original_starts'] == 98
            and value['final_request_rows'] == 197 and value['final_conservative_charges'] == 246
            and value['source_chunks_credited_as_Worlds'] == value['source_chunks_credited_as_action_witnesses'] == 0
            and value['pixels'] == 3202419000 and value['mismatching_pixels'] == 0,
            'Retained interaction output/domain changed')
    controls = [checked(reader, v) for v in value['controls']]
    require([v['tests'] for v in controls] == [12, 16, 8]
            and all(all(reader.digest(n) == h for n, h in v['sources'].items()) for v in controls),
            'Retained interaction controls stale')
    matches = [v for v in values if v['spec']['case']['id'] == RECOVERED]
    require(len(matches) == 1, 'Missing unique retained interaction case')
    actual = matches[0]
    require(actual['validation'] == value['validation'] and actual['acquisition'] == value['acquisition']
            and actual['coordinator'] == value['coordinator'], 'Different retained interaction result')
    typed = checked(reader, value['typed_bookkeeping'])
    require(len(typed['results']) == 2 and all(v['replicas_verified'] == 2
            and v['logical_restore_verified'] and v['real_restore_performed'] for v in typed['results']),
            'Retained interaction bookkeeping not restored')
    return ref


def supplement(reader, root, rows, values, plan):
    binding = reviewed_interaction(reader, values)
    result = previous.supplement(reader, root, rows, values, plan)
    result['retained_current_interaction_analysis'] = dict(verified=True, conjunction=binding,
        additional_game_starts=0, additional_reservations=0, historical_failures_retired=False,
        old_final_predicates_changed=False, actual_ledger_rows_renamed=False)
    return result
