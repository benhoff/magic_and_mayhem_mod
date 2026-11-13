"""Exact stopped-attempt accounting and the smallest reviewed repair extension."""
from broader_player_requests import cumulative_bound
from broader_prelaunch_review import reviewed_with_setup_refusals
from rendering_pilot import require

def review(reader,plan):
 ref=plan['guard_stopped_review'];require(reader.digest(ref['record'])==ref['sha256'],'Changed guard-stop continuation review');q=reader.json(ref['record'])
 require(q['kind']=='broader_guard_stopped_continuation_review'and q['success']is True and q['original_game_runs']==0 and q['case_accepted']is False and q['old_additional_ceiling']==112 and q['new_additional_ceiling']==113 and q['old_combined_ceiling']==120 and q['new_combined_ceiling']==121 and q['ceiling_extension']==1 and q['replacement_original_starts']==2 and q['prior_remaining_unallocated_starts']==1 and q['minimum_final_additional_reservations']==113 and q['reserved_failed_starts_retained']==2 and q['observed_failed_original_starts']==2 and q['reservations_refunded']is False and q['native_inputs_or_actions_changed']is False and q['user_numeric_approval_claimed']is False,'Guard-stop review cannot reset a cap, refund starts or accept the failed case')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Guard-stop reviewer source changed')
 ar=q['failed_acquisition'];require(reader.digest(ar['record'])==ar['sha256'],'Changed guard-stopped negative');a=reader.json(ar['record'])
 require(a['kind']=='broader_guard_stopped_unreported_acquisition'and a['success']is False and a['delegate_success']is False and a['case_accepted']is False and a['original_game_runs']==a['charged_original_starts']==2 and a['actual_start_count_complete']is True and len(a['observed_session_roots'])==2 and a['private_wine_stopped']is True and len(a['missing_unrecoverable_collector_observations'])==4,'Only the exact unaccepted fully charged two-start guard stop is reviewed')
 require(len(a['Cold_handoffs'])==3 and all(v['real_restore_verified']and v['replicas_verified']==2 for v in a['Cold_handoffs'])and a['RAM_handoff']['real_restore_verified']and a['RAM_handoff']['replicas_verified']==2,'Stopped raw evidence lacks verified custody')
 parent=q['parent_global_plan'];require(reader.digest(parent['record'])==parent['sha256'],'Changed parent start bound');old=reader.json(parent['record']);require(cumulative_bound(reader,old)==112,'Unknown original bound')
 return q

def bound(reader,plan):
 if plan['maximum_additional_original_games']!=113:return cumulative_bound(reader,plan)
 q=review(reader,plan);require(plan['success']is True and plan['previous_WP10_original_games']==8 and plan['maximum_combined_original_games']==121 and plan['maximum_combined_WP10_and_new_original_games']==121 and plan['minimum_final_additional_reservations']==113 and plan['maximum_manually_reviewed_additional_repairs']==0,'Unbounded repair continuation')
 return 113

def predecessors(reader,rows,plan):
 q=review(reader,plan);ordinary=[];charge=0;seen=False
 for row in rows:
  if row['case_id']!=q['failed_case_id']:ordinary.append(row);continue
  require(not seen and row['state']=='failed'and row['reserved_games']==2 and dict(record=row['report'],sha256=row['acquisition_sha256'])==q['failed_acquisition'],'Guard review cannot hide another failure or refund its reservation');seen=True;charge=2
 require(seen,'Reviewed guard-stopped request is missing')
 return charge+reviewed_with_setup_refusals(reader,ordinary,plan)
