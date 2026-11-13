"""Keep the failed runtime phase and every earlier request beside fresh current proof."""
from pathlib import Path
from broader_player_acceptance import checked,native_phase
from broader_runtime_continuation_budget import review
from rendering_pilot import require

def exact_ledger(rows,values,q):
 require(len(rows)==183 and sum(v['reserved_games']for v in rows.values())==231
         and len(values)==90 and sum(v['spec']['case']['reserved_original_games']for v in values)==98,
         'Complete183/231 ledger and90/98 positive matrix required')
 fixed={v['case_id']:v for v in q['old_requests']};require(len(fixed)==93 and all(rows.get(k)==v for k,v in fixed.items()),'Historical93/133 requests changed')
 require(len({v['spec']['case']['id']for v in values})==90 and set(fixed).isdisjoint(v['spec']['case']['id']for v in values),'Historical or duplicate context received current credit')

def supplement(reader,root,rows,values,plan):
 q=review(reader,plan);exact_ledger(rows,values,q)
 from broader_player_copy_matrix import P,BINARY
 controls=checked(reader,q['controls']);require(controls['sources_stable']and controls['fading']['guard_controls']==14484 and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current fading guard controls missing')
 model=checked(reader,q['particle_model']);require(model['original_manifest_verified_before_after']and len(model['results'])==2 and all(v['cases']==77760 for v in model['results'])and all(reader.digest(n)==h for n,h in model['sources'].items()),'Current independent particle model comparison missing')
 diagnosis=checked(reader,q['failed_attachment_diagnosis']);failed=diagnosis['failed_request'];require(rows[failed['case_id']]==failed and not reader.json(diagnosis['acquisition']['record'])['success']and reader.digest(diagnosis['acquisition']['record'])==diagnosis['acquisition']['sha256'],'Failed World205 was rewritten or promoted')
 prior=[]
 for row in q['prior_completed_contexts']:
  a=reader.json(row['report']);require(reader.digest(row['report'])==row['acquisition_sha256']and a['success']and not native_phase(root,reader,a,BINARY)['current_native_source_candidate'],'Old B06runtime acquired current credit')
  prior.append(dict(case=row['case_id'],acquisition=dict(record=row['report'],sha256=row['acquisition_sha256']),validation=dict(record=row['validation'],sha256=reader.digest(row['validation'])),current_credit=False))
 require(len(prior)==20,'Historical20completed contexts lost')
 setup_name=P+'B06-attachment-setup-refusal-01.json';setup=checked(reader,dict(record=setup_name,sha256=reader.digest(setup_name)));closed=setup['failed_request'];require(rows[closed['case_id']]==closed and not(Path(root)/closed['report']).exists()and setup['actual_start_proof']['before_player_or_Original_launch'],'Zero-start setup negative changed')
 canonical_name=P+'B06-canonical-plan-guard-controls-02.json';canonical=checked(reader,dict(record=canonical_name,sha256=reader.digest(canonical_name)));require(canonical['tests']==33 and canonical['files']==3 and all(reader.digest(n)==h for n,h in canonical['sources'].items()),'Canonical-plan controls missing');repaired=[]
 for v in values:
  require(v['phase']['current_native_source_candidate'],'Noncurrent runtime value')
  if v['spec']['family']not in {'attachment','font','palette','picking'}:continue
  g=v['acquired']['provider_adaptation']['canonical_plan_guard_adaptation'];require(g['phase']==v['spec']['plan']and g['changed_guards']==1 and g['all_other_AST_nodes_unchanged']and g['actions_deadlines_budgets_Native_code_unchanged']and reader.digest(g['original_source']['record'])==g['original_source']['sha256']==g['source_sha256']and reader.digest(g['adapted_source']['record'])==g['adapted_source']['sha256']==g['adapted_source_sha256'],'Actual canonical-plan source adaptation missing');repaired.append(dict(case=v['spec']['case']['id'],validation=v['validation'],guard=g))
 require(len(repaired)==16,'All16guarded contexts required')
 pilot_name=P+'runtime29-attachment-single-pilot-result-01.json';pilot=checked(reader,dict(record=pilot_name,sha256=reader.digest(pilot_name)));matches=[v for v in values if v['spec']['case']['id']==q['first_pilot']];require(len(matches)==1 and pilot['acquisition']==matches[0]['acquisition']and pilot['validation']==matches[0]['validation']and pilot['current_native_source_candidate'],'Reviewed failed-context replacement pilot missing')
 negative='research/runtime/cursor-hotspot-results-20261108/bp27/world-hotspot/bp27-world-hotspot-default640-01-comparison-01.json';raw=reader.json(negative);require(not raw['success']and raw['comparison']['success']and raw['comparison']['mismatching_pixels']==0 and raw['error']=='Original cursor draw did not precede the actual readiness paint','Historical partial-paint negative changed')
 control_name=P+'runtime29-final-reader-controls-03.json';final=checked(reader,dict(record=control_name,sha256=reader.digest(control_name)));require(final['tests']==12 and final['sources_stable']and final['original_game_runs']==final['new_native_executions']==0 and all(reader.digest(n)==h for n,h in final['sources'].items()),'Current runtime final-reader refusal controls missing')
 return dict(final_reader_controls=dict(record=control_name,sha256=reader.digest(control_name)),verified=True,review=plan['runtime_continuation_review'],historical_rows_preserved=93,historical_charges_preserved=133,prior_completed_contexts_not_current=prior,zero_start_setup=dict(record=setup_name,sha256=reader.digest(setup_name)),failed_World205=q['failed_attachment_diagnosis'],failed_context_replacement=dict(record=pilot_name,sha256=reader.digest(pilot_name)),fading_controls=q['controls'],particle_model=q['particle_model'],actual_guarded_cases=repaired,partial_paint_negative=dict(record=negative,sha256=reader.digest(negative)),positive_contexts=90,positive_actual_starts=98,total_rows=183,total_charges=231,combined_with_WP10=239,Native_source_waivers=[],reservations_refunded=False,historical_failures_retired=False,universal_repair_claimed=False,scope='Every original93request/133charge andnegative remains exact,including20formerly current B06contexts andthe real World205producer refusal. All90newpositive contexts execute andretire at current full Native/runtime/compiler hashes. Only finite same-context nonreproduction is proved; no historical operand reconstruction,universal repair,Original recovery orunbounded domain is inferred.')
