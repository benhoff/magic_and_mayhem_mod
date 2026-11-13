"""Permit only the exact retained queue-refused pair under a new bounded review."""
from broader_guarded_continuation import bound as guarded_bound,predecessors as guarded_predecessors
from rendering_pilot import require

def review(reader,plan):
 ref=plan['capture_refusal_review'];require(reader.digest(ref['record'])==ref['sha256'],'Capture-refusal review changed');q=reader.json(ref['record'])
 require(q['kind']=='broader_capture_refusal_continuation_review'and q['success']is True and q['original_game_runs']==0 and q['case_accepted']is False and q['old_additional_ceiling']==113 and q['new_additional_ceiling']==115 and q['old_combined_ceiling']==121 and q['new_combined_ceiling']==123 and q['ceiling_extension']==2 and q['replacement_original_starts']==2 and q['reserved_failed_starts_retained']==2 and q['reservations_refunded']is False and q['agent_set_limit']is True and q['user_numeric_approval_claimed']is False and q['native_sources_actions_deadlines_or_capture_limits_changed']is False and q['automatic_repairs_remaining']==0,'Capture continuation cannot refund, weaken or grant acceptance')
 require(all(reader.digest(n)==h for n,h in q['reviewer_sources'].items()),'Capture continuation reviewer changed')
 maintenance=q['maintenance_checkpoint'];require(reader.digest(maintenance['record'])==maintenance['sha256'],'Maintenance checkpoint changed');m=reader.json(maintenance['record']);require(m['success']and m['total_completed_source_copies']==58 and m['source_checkouts_removed']==0 and m['result']['real_restore_verified']and m['result']['replicas_verified']==2,'Finish verified source maintenance before replacement')
 parent=q['parent_global_plan'];require(reader.digest(parent['record'])==parent['sha256']and guarded_bound(reader,reader.json(parent['record']))==113,'Changed prior bounded plan')
 ar=q['failed_acquisition'];cr=q['failed_comparison'];dr=q['capture_diagnosis'];hr=q['negative_custody']
 for v in [ar,cr,dr,hr]:require(reader.digest(v['record'])==v['sha256'],'Capture negative evidence changed')
 a,c,d,h=(reader.json(v['record'])for v in [ar,cr,dr,hr])
 require(a['success']is True and a['original_game_runs']==2 and a['private_wine_stopped']and a['prefix_pool']['returned']and len(a['proof']['epochs'])==2,'Unbounded or unstopped negative acquisition')
 require(c['success']is False and c['error']=='Capture incomplete or truncated'and c['comparison_processes_stopped']and c['sources_stable']and c['acquisition']==ar,'Only the exact stopped incomplete-capture comparison is reviewed')
 require(d['success']and d['case_accepted']is False and d['acquisition']==ar and d['failed_comparison']==cr and d['missing_command_bytes']==30089412 and d['stopped_ring_capacity']==1048576 and d['missing_tail_recoverable_from_retained_ring']is False and d['synthetic_END_or_replies_added']is False and d['raw_capture_complete_flags_preserved']and d['capture_limits_changed']is False,'Lost command bytes cannot be waived or fabricated')
 require(h['success']and h['case_accepted']is False and h['acquisition']==ar and h['failed_comparison']==cr and h['diagnosis']==dr and h['reservations_refunded']is False and len(h['Cold_handoffs'])==3 and all(v['real_restore_verified']and v['replicas_verified']==2 for v in h['Cold_handoffs'])and h['RAM_handoff']['real_restore_verified']and h['RAM_handoff']['replicas_verified']==2,'Negative raw custody is incomplete')
 require(plan['preserved_negative_predecessors'][q['failed_case_id']]==q['negative_predecessor_review'],'Incomplete negative request accounting')
 return q

def bound(reader,plan):
 if plan['maximum_additional_original_games']!=115:return guarded_bound(reader,plan)
 review(reader,plan);require(plan['maximum_combined_original_games']==123 and plan['maximum_combined_WP10_and_new_original_games']==123 and plan['minimum_final_additional_reservations']==115 and plan['maximum_manually_reviewed_additional_repairs']==0,'Unbounded capture repair')
 return 115

def predecessors(reader,rows,plan):
 q=review(reader,plan);matches=[v for v in rows if v['case_id']==q['failed_case_id']];require(len(matches)==1 and matches[0]['state']=='acquired'and matches[0]['reserved_games']==2 and dict(record=matches[0]['report'],sha256=matches[0]['acquisition_sha256'])==q['failed_acquisition'],'Capture failure reservation missing or altered')
 return guarded_predecessors(reader,rows,plan)
