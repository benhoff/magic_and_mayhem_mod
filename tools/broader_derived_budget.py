"""Preserve the zero-Original startup failure and bound one reviewed replacement."""
from broader_secondary_budget import bound as parent_bound,predecessors as parent_predecessors
from broader_player_acceptance import checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
FAILED='bp44-world-interaction-default800-01'
HOVER_FAILED='bp46-world-interaction-default800-01'
DEATH_FAILED='bp47-world-interaction-default800-01'
PILOT='bp48-world-interaction-default800-01'


def validate_rows(rows,old,new):
    fixed={v['case_id']:v for v in old}
    require(len(old)==len(fixed)==156 and sum(v['reserved_games']for v in old)==205,'Changed156/205 historical domain')
    require(len(new)==sum(new.values())==45 and new.get(PILOT)==1 and FAILED not in new and HOVER_FAILED not in new and DEATH_FAILED not in new
            and set(fixed).isdisjoint(new)and all(type(v)is int and v==1 for v in new.values()),
            'Changed45/45 derived-admission continuation')
    minimal={'case_id','report','reserved_games','state','acquisition_sha256'};full=minimal|{'plan','started_ns','validation'}
    seen=set();total=0
    for row in rows:
        cid=row['case_id'];require(set(row)in(minimal,full)and cid not in seen,'Unknown/duplicate request');seen.add(cid)
        if cid in fixed:require(all(fixed[cid][k]==v for k,v in row.items()),'Historical request rewritten')
        else:require(cid in new and row['state']=='admitted'and row['reserved_games']==new[cid]
                     and row['acquisition_sha256']and cid in row['report'],'Stop after failed/incomplete new request')
        total+=row['reserved_games']
    require(set(fixed)<=seen and total<=250,'Historical row omitted or250ceiling exceeded')
    return total


def review(reader,plan):
    q=checked(reader,plan['derived_admission_review'])
    require(q['kind']=='reviewed_explicit_derived_compiler_admission_continuation'
            and q['original_game_runs']==q['new_native_executions']==0,'Wrong derived-admission continuation review')
    require(q['retained_current_cases']==q['remaining_positive_cases']==q['remaining_positive_original_starts']==45
            and q['new_additional_ceiling']==250 and q['new_combined_ceiling']==258
            and q['positive_contexts']==90 and q['positive_original_starts']==98 and q['first_pilot']==PILOT,
            'Changed finite derived-admission domain')
    require(q['reservations_refunded']is False and q['automatic_retries']is False and q['source_waivers']==[]
            and q['physical_Native_runtime_compiler_files_changed']is False
            and q['default_native_only_guard_changed']is False,'Unreviewed source/charge waiver')
    parent=checked(reader,q['parent_global_plan']);require(parent_bound(reader,parent)==247,'Prior247ceiling changed')
    diagnosis=checked(reader,q['startup_diagnosis']);row=diagnosis['failed_request']
    require(row in q['old_requests']and row['case_id']==FAILED and row['state']=='failed'
            and row['reserved_games']==1 and row['acquisition_sha256']==diagnosis['acquisition']['sha256']
            and diagnosis['before_Wine_proof']['Original_event_trace_absent']
            and diagnosis['before_Wine_proof']['native_output_admission_absent']
            and diagnosis['before_Wine_proof']['completed_Worlds']==0
            and diagnosis['primary_failure']=='Native-only build source changed: runtime/scene/world_raster_callers.h',
            'Zero-Original startup negative changed/promoted')
    require(parent_predecessors(reader,[v for v in q['old_requests']if v['case_id']not in(FAILED,HOVER_FAILED,DEATH_FAILED)],parent)==202,
            'Prior actual history changed')
    custody=checked(reader,q['negative_custody'])
    require(custody['case']==FAILED and not custody['case_accepted']and custody['requests_unchanged']
            and not custody['reservations_refunded']and len(custody['typed_bookkeeping'])==2
            and len(custody['results'])==3 and all(v['replicas_verified']==2 and v['real_restore_verified']for v in custody['results']),
            'Zero-Original failed setup custody missing')
    hover=checked(reader,q['hover_diagnosis']);hover_row=hover['failed_request']
    require(hover_row in q['old_requests']and hover_row['case_id']==HOVER_FAILED and hover_row['state']=='failed'
            and hover_row['reserved_games']==1 and hover_row['acquisition_sha256']==hover['acquisition']['sha256']
            and hover['actual_resident']['completed']==hover['actual_resident']['verified_returns']==704
            and not hover['case_accepted']and not hover['normal_quit_credit'],'Failed current hover changed/promoted')
    hover_custody=checked(reader,q['hover_negative_custody'])
    require(hover_custody['case']==HOVER_FAILED and not hover_custody['case_accepted']and hover_custody['requests_unchanged']
            and not hover_custody['reservations_refunded']and len(hover_custody['typed_bookkeeping'])==2
            and len(hover_custody['results'])==3 and all(v['replicas_verified']==2 and v['real_restore_verified']for v in hover_custody['results']),
            'Failed hover negative custody missing')
    recent=checked(reader,q['recent_source_controls'])
    require(recent['tests']==37 and recent['source_only_refresh']and recent['opaque_pixel_formula_unchanged']
            and not recent['hit_feedback_search']and not recent['action_schedule_changed']and not recent['deadlines_changed']
            and all(reader.digest(p)==h for p,h in recent['sources'].items()),'Recent source-target controls stale')
    death=checked(reader,q['death_diagnosis']);death_row=death['failed_request']
    require(death_row in q['old_requests']and death_row['case_id']==DEATH_FAILED and death_row['state']=='failed'
            and death_row['reserved_games']==1 and death_row['acquisition_sha256']==death['acquisition']['sha256']
            and death['actual_resident']['completed']==death['actual_resident']['verified_returns']==965
            and death['dead_before_Escape']and death['living_wizard_predicate_not_waived']and not death['case_accepted'],
            'Actual natural death failure changed/promoted')
    death_custody=checked(reader,q['death_negative_custody'])
    require(death_custody['case']==DEATH_FAILED and not death_custody['case_accepted']and death_custody['requests_unchanged']
            and not death_custody['reservations_refunded']and len(death_custody['typed_bookkeeping'])==2
            and len(death_custody['results'])==3 and all(v['replicas_verified']==2 and v['real_restore_verified']for v in death_custody['results']),
            'Natural death negative custody missing')
    relevance=checked(reader,q['terminal_relevance'])
    require(relevance['success']and relevance['negative_diagnosis']==q['death_diagnosis']
            and relevance['negative_custody']==q['death_negative_custody']and len(relevance['fresh_existing_natural_Campaign_defeat_proofs'])==4
            and relevance['new_required_row_excluded']is False and relevance['failed_case_accepted']is False
            and relevance['explicit_known_unsupported_boundary']and all(reader.digest(p)==h for p,h in relevance['source_review'].items()),
            'Natural Quick terminal relevance/known boundary review missing')
    pause=checked(reader,q['public_pause_controls'])
    require(pause['tests']==32 and pause['unchanged_outer_assessor_code_verified']and pause['every_other_interaction_proof_AST_node_preserved']
            and pause['original_Space_down_up_owned_sequence_required']and pause['paused_simulation_counter_must_be_constant']
            and pause['at_least_eight_resumed_simulation_counters_required']and not pause['current_acceptance_granted']
            and all(reader.digest(p)==h for p,h in(pause['sources']|pause['unchanged_dependencies']).items()),
            'Public pause collection controls stale')
    guards=checked(reader,q['derived_admission_controls'])
    require(guards['focused_tests']==51 and guards['actual_pre_Wine_guard_verified']
            and not guards['new_positive_case_executed']and guards['source_hash_waivers']==[]
            and all(reader.digest(p)==h for p,h in guards['sources'].items()),'Derived admission controls stale')
    actual=checked(reader,guards['actual_owned_runner_probe'])
    require(actual['actual_owned_runner_body_executed']and actual['stopped_immediately_after_successful_native_only_admission']
            and not actual['bwrap_or_Original_processes_started']and actual['original_game_runs']==actual['new_native_executions']==0,
            'Actual pre-Wine owned-runner control missing')
    retained=checked(reader,q['retained_current_check'])
    require(retained['qualified_contexts']==45 and len(retained['results'])==45
            and retained['new_native_source_waivers']==[],'Retained45current cases changed')
    controls=checked(reader,q['controls'])
    require(controls['success']and controls['tests']>=12 and controls['original_game_runs']==0
            and all(reader.digest(p)==h for p,h in controls['sources'].items()),'Derived continuation controls stale')
    require(all(reader.digest(p)==h for p,h in q['reviewer_sources'].items()),'Derived continuation reviewer changed')
    validate_rows(q['old_requests'],q['old_requests'],q['new_case_reservations'])
    return q


def bound(reader,plan):
    review(reader,plan)
    require(plan['success']and plan['previous_WP10_original_games']==8
            and plan['maximum_additional_original_games']==plan['minimum_final_additional_reservations']==250
            and plan['maximum_combined_original_games']==plan['maximum_combined_WP10_and_new_original_games']==258,
            'Changed250/258ceiling')
    return 250


def predecessors(reader,rows,plan):
    q=review(reader,plan);return validate_rows(rows,q['old_requests'],q['new_case_reservations'])
