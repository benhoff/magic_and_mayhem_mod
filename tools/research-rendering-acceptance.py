#!/usr/bin/env python3
"""Research artifact 5 admission, joins, recovery and real SQLite restoration.

Exercises the unchanged opening APIs on isolated copies. Counterexamples are
synthetic bookkeeping experiments, not engine executions or accepted receipts.
"""
import argparse
from collections import Counter
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import shutil
import sqlite3
import sys

from coverage_claims import fingerprint
from coverage_evidence import Reader, write_report
from drawing_journeys import coverage, REQUIREMENTS, OPERATIONS
from rendering_acceptance import Workspace, assemble, requirements, verify_evidence
from rendering_acceptance_research import candidate_diagnostics, GATE_RESEARCH, JOIN_RULES
from rendering_execution import review

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-acceptance-research-results-20261031/'
PARENT='research/runtime/rendering-execution-research-results-20261031/'
WITNESSES=['tools/research-rendering-acceptance.py','tools/rendering_acceptance_research.py',
    'tools/rendering_acceptance.py','tools/rendering_execution.py','tools/rendering_scope.py',
    'tools/rendering_completion.py','tools/coverage_claims.py','tools/coverage_evidence.py',
    'tools/evidence_local.py','tools/drawing_journeys.py','tools/drawing_session.py',
    'tools/check-drawing-journeys.py','tools/drawing_recorded_session.py',
    'tools/drawing_action_session.py','tools/retain-drawing-session.py',
    'tools/native_only_contract.py','tools/native_output_owner.py',
    'apps/drawing-session/main.cpp','apps/drawing-session/CMakeLists.txt',
    'apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/gl_viewport.cpp',
    'docs/native-rendering-closure.json','docs/native-rendering-acceptance.md',
    'tests/test-rendering-acceptance.py','tests/test-drawing-session.py',
    'tests/test-rendering-acceptance-research.py']


def synthetic_summaries():
    observations=[dict(id='synthetic-'+name,scenario=name,boundary='presented_frame',
        independent_reference=True,contiguous=True,actions_bound=True,frames=1,
        requirements=list(leaves),operations=list(OPERATIONS) if i==0 else [],outcome='matching')
        for i,(name,leaves) in enumerate(REQUIREMENTS.items())]
    summary=coverage(observations)
    copied=deepcopy(observations)
    for row in copied:row['configuration']='player-software'
    failure=dict(observations[0],id='synthetic-current-failure',outcome='mismatch')
    mixed=coverage([*observations,failure])
    return dict(trusted_summary_can_report_complete=summary['draw02_complete'],
        configuration_metadata_ignored_by_legacy_summary=coverage(copied)==summary,
        passing_leaf_sets_do_not_erase_global_failure=not mixed['draw02_complete'],
        declared_reference_flags_are_caller_trust_boundary=True,
        actual_acquisition_performed=False,admitted_evidence_created=False,
        scope='Pure trusted aggregation API given synthetic dictionaries. The production runner first admits artifacts; this is a warning against treating its summary function as an independent receipt verifier, not a demonstrated current player bypass.')


def sqlite_probes(plan,prerequisite,latest,report_name,reader):
    parent=ROOT/'working/acceptance-research-probes'/Path(report_name).stem
    parent.mkdir(parents=True,exist_ok=False)
    results=[]
    retained=[]
    def retain(label,path):
        name=str(Path(report_name).with_suffix(''))+'-'+label+'.bin'
        value=dict(record=name,sha256=write_report(ROOT,name,path.read_bytes()),bytes=path.stat().st_size)
        retained.append(value);return value
    def one(name,action):
        w=Workspace(parent/name,plan)
        before=w.events()
        result=dict(id=name,accepted_by_current_api=False)
        try:
            action(w)
            result['accepted_by_current_api']=True
        except (ValueError,KeyError,TypeError,sqlite3.DatabaseError) as error:
            result['error']=str(error)
        after=w.events()
        projection=assemble(plan,prerequisite,w,[],dict(scope='Synthetic research SQLite experiment'))
        result.update(events_before=len(before),events_after=len(after),
            source_review_projected=projection['ep01_review']['source_and_lead_review_performed'],
            passed_rows=projection['counts']['passed_rows'],
            supported_player_accepted=projection['supported_player_accepted'],
            original_recovery_complete=projection['original_recovery_complete'])
        database=w.path;w.close();result['database']=retain(name,database)
        results.append(result)
        return result
    def snapshot(w):
        return deepcopy(assemble(plan,prerequisite,w,[],dict(scope='Synthetic opening snapshot')))
    def event(w,kind,payload,date='2026-10-31T00:00:00+00:00'):
        return dict(id=fingerprint(dict(dossier=w.identity,kind=kind,payload=payload)),
                    kind=kind,payload=payload,observed_at=date)
    def candidate(w):
        return dict(configuration=w.rows[0]['configuration'],requirement=w.rows[0]['id'],
            evidence=dict(id='SYNTHETIC.only',record='research/runtime/synthetic.json',sha256='a'*64),
            reason='Synthetic candidate; no execution/admission.')
    one('ordinary-candidate',lambda w:w.append('candidate_attachment',candidate(w)))
    one('append-extra-pass-field',lambda w:w.append('candidate_attachment',dict(candidate(w),accepted=True)))
    def wrong_scope(w):
        value=candidate(w);value['configuration']='invented-config'
        w.append('candidate_attachment',value)
    one('append-unknown-configuration',wrong_scope)
    def restore_accepted(w):
        value=snapshot(w);value['requirements'][0]['state']='passed';w.restore_snapshot(value)
    one('restore-passed-row',restore_accepted)
    def restore_wrong_dossier(w):
        value=snapshot(w);value['dossier_identity']='0'*64;w.restore_snapshot(value)
    one('restore-other-dossier',restore_wrong_dossier)
    def restore_missing_row(w):
        value=snapshot(w);value['requirements'].pop();w.restore_snapshot(value)
    one('restore-missing-row',restore_missing_row)
    def restore_source_shape(w):
        value=snapshot(w);value['events']=[event(w,'source_applicability_review',{})];w.restore_snapshot(value)
    one('restore-empty-source-observation',restore_source_shape)
    def restore_reason(w):
        value=snapshot(w);payload=candidate(w);payload['reason']=None
        value['events']=[event(w,'candidate_attachment',payload)];w.restore_snapshot(value)
    one('restore-null-candidate-reason',restore_reason)
    def restore_time(w):
        value=snapshot(w);value['events']=[event(w,'candidate_attachment',candidate(w),'not-a-timestamp')];w.restore_snapshot(value)
    one('restore-unparsed-timestamp',restore_time)
    def restore_bindings(w):
        value=snapshot(w);value['bindings']['completion_contract']['sha256']='0'*64;w.restore_snapshot(value)
    one('restore-conflicting-summary-bindings',restore_bindings)
    def atomic_batch(w):
        value=snapshot(w);good=event(w,'candidate_attachment',candidate(w))
        bad=event(w,'candidate_attachment',dict(candidate(w),accepted=True))
        value['events']=[good,bad];w.restore_snapshot(value)
    one('restore-batch-rolls-back',atomic_batch)
    def update(w):
        w.append('candidate_attachment',candidate(w))
        with w.db:w.db.execute("UPDATE events SET observed_at='rewritten'")
    one('sql-update-trigger',update)
    def delete(w):
        w.append('candidate_attachment',candidate(w))
        with w.db:w.db.execute('DELETE FROM events')
    one('sql-delete-trigger',delete)
    def fake_candidate(w):
        value=snapshot(w);value['events']=[event(w,'candidate_attachment',candidate(w))];w.restore_snapshot(value)
    one('workspace-restore-does-not-resolve-evidence',fake_candidate)
    def public_verification(w):
        verify_evidence(ROOT,reader,dict(evidence=[]),'SYNTHETIC.only')
    one('public-evidence-verification-refuses-unregistered',public_verification)
    def changed_hash(w):
        value=snapshot(w);item=event(w,'candidate_attachment',candidate(w));item['id']='0'*64
        value['events']=[item];w.restore_snapshot(value)
    one('restore-changed-event-hash',changed_hash)
    expected_accept={'ordinary-candidate','restore-empty-source-observation','restore-null-candidate-reason',
        'restore-unparsed-timestamp','restore-conflicting-summary-bindings',
        'workspace-restore-does-not-resolve-evidence'}
    if {r['id'] for r in results if r['accepted_by_current_api']}!=expected_accept:
        raise ValueError('Opening API behavior differs from the reviewed research findings')
    if any(r['passed_rows'] or r['supported_player_accepted'] or r['original_recovery_complete'] for r in results):
        raise ValueError('Synthetic probe escaped opening pending projection')
    rollback=next(r for r in results if r['id']=='restore-batch-rolls-back')
    if rollback['events_after']!=0:raise ValueError('Malformed restore was not atomic')
    original=Workspace(parent/'actual-snapshot',plan)
    original.restore_snapshot(latest);original_events=original.events();backup=original.backup()
    restore_root=parent/'actual-backup-restore';restore_path=restore_root/plan['acceptance_record']['workspace']
    restore_path.parent.mkdir(parents=True);shutil.copy2(backup['path'],restore_path)
    restored=Workspace(restore_root,plan)
    actual=restored.events()
    if actual!=original_events or {e['id'] for e in actual}!={e['id'] for e in latest['events']}:
        raise ValueError('Real SQLite backup/restore changed event identities or timestamps')
    restored.close();original.close()
    restoration=dict(success=True,events=len(actual),ids_and_times_unchanged=True,
        original_ledger=retain('actual-ledger',original.path),
        verified_backup=retain('verified-backup',Path(backup['path'])),
        restored_ledger=retain('actual-restored-ledger',restore_path),
        backup_sha256=backup['sha256'],engine_validation_performed=False)
    return dict(probes=results,actual_backup_restore=restoration,retained=retained,
        observed_validation_asymmetries=['restore_empty_source_payload','restore_null_reason',
            'restore_nonempty_unparsed_timestamp','snapshot_summary_bindings_not_revalidated'],
        lower_level_restore_trust_boundary='Workspace.restore_snapshot assumes externally verified candidate evidence; the public restore CLI/review rechecks registry/payload assertions. Do not expose the workspace method as positive receipt admission.',
        all_projected_acceptance_pending=True)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    args=parser.parse_args()
    if not args.report.startswith(PREFIX) or (ROOT/args.report).exists():
        parser.error('Require a new owned immutable research report')
    with Reader(ROOT) as reader:
        sources={n:reader.digest(n) for n in WITNESSES}
        report=dict(schema=1,kind='rendering_acceptance_research',success=False,sources=sources,
            engine_execution_performed=False,original_media_consumed=False,
            supported_player_accepted=False,original_recovery_complete=False,
            positive_receipt_profile_established=False,status_promotions=[])
        try:
            prerequisite=review(ROOT);plan=reader.json('docs/native-rendering-closure.json')
            register=reader.json('research/runtime/coverage/register.json')
            latest=reader.json(PARENT+'acceptance-candidates-01.json')
            entries={e['id']:e for e in register['evidence']}
            behaviors={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
            identifiers=sorted({e['evidence']['id'] for e in latest['attached_candidates']}
                | {e['id'] for e in prerequisite['evidence_applicability_seed']}
                | {eid for eid in entries if eid.startswith('RER31.')})
            diagnostics=[]
            for eid in identifiers:
                entry=entries[eid];verified=verify_evidence(ROOT,reader,register,eid)
                diagnostics.append(candidate_diagnostics(entry,reader.json(entry['record']),verified,behaviors,builds))
            build_record=reader.json(entries['RER31.build']['record'])
            compiled=build_record['compiled_sources']
            compiled_changes=[n for n,h in compiled.items() if reader.digest(n)!=h]
            # Execute a type counterexample against the unchanged real candidate
            # verifier, using a locally supplied synthetic registry only.
            probe_name=PREFIX+'assertion-type-probe-01.json'
            probe=dict(schema=1,success=1,synthetic_bookkeeping_probe=True,
                sources={'tools/rendering_acceptance.py':reader.digest('tools/rendering_acceptance.py')})
            digest=write_report(ROOT,probe_name,(json.dumps(probe,indent=2)+'\n').encode())
            entry=dict(id='SYNTHETIC.integer-success',record=probe_name,sha256=digest,kind='static',
                scope='Research counterexample only.',sources_pointer='/sources',
                assertions=[dict(pointer='/success',equals=True)])
            # The pending database may have been created by this first write.
            # An older invocation-scoped Reader must not be expected to discover
            # a database that did not exist when its resolver was opened.
            with Reader(ROOT) as probe_reader:
                observed=verify_evidence(ROOT,probe_reader,dict(evidence=[entry]),entry['id'])
            strict=candidate_diagnostics(entry,probe,observed,behaviors,builds)
            if 'execution_success_not_true' not in strict['blockers']:
                raise ValueError('Typed diagnostic accepted integer success')
            ledger=sqlite_probes(plan,prerequisite,latest,args.report,reader)
            matrix=[]
            for row in requirements(plan):
                attachments=[e['evidence']['id'] for e in latest['attached_candidates']
                    if (e['configuration'],e['requirement'])==(row['configuration'],row['id'])]
                matrix.append(dict(row,state='pending',admitted_execution_evidence=[],
                    reviewed_attached_candidates=attachments,
                    necessary_join=GATE_RESEARCH[row['requirement']] if row['kind']=='gate' else
                        ('Admitted exact journey leaf/action/configuration with its required presented output, reference/semantic policy and lifetime.' if row['kind']=='journey' else
                         'Admitted required operation caller/input domain, independent output and typed side effects; ordinary/fault/recovery outcome classified.'),
                    missing_execution_detail=row['pending_reason']))
            groups=[]
            for group in latest['original_recovery']:
                linked=[]
                for bid in group['behaviors']:
                    b=behaviors[bid]
                    linked.append(dict(id=bid,kind=b['kind'],scope=b['scope'],confidence=b['confidence'],
                        status=b['status'],original=b['original'],evidence=b['evidence'],
                        remaining_gaps=b.get('remaining_gaps',[]),
                        native_policy_cannot_recover_original=b['kind']=='native_policy'))
                groups.append(dict(group,reviewed_behaviors=linked,closure_inferred=False,
                    required_next_proof=group['recovery_exit'],
                    installed_native_replacement_is_automatically_required=False))
            report.update(success=True,records=[dict(record=n,sha256=reader.digest(n)) for n in
                [PARENT+'acceptance-candidates-01.json',PARENT+'join-01.json',PARENT+'anchor-accounting-correction-01.json']],
                acceptance_bindings=plan['acceptance_record']['bindings'],
                dossier_identity=latest['dossier_identity'],requirements=matrix,
                candidate_diagnostics=diagnostics,original_recovery_groups=groups,
                scope_conditions=latest['scope_decisions'],supplier_policy=plan['completion_contract']['supplier_policy'],
                sqlite_research=ledger,synthetic_aggregation=synthetic_summaries(),
                assertion_type_research=dict(synthetic_record=probe_name,sha256=digest,
                    actual_candidate_verifier_accepts_integer_one_for_true=observed['payload_and_assertions_verified'],
                    candidate_still_cannot_pass=not observed['closes_requirement'],
                    strict_diagnostic_blocks_integer_success='execution_success_not_true' in strict['blockers']),
                gate_research=GATE_RESEARCH,join_rules=JOIN_RULES,
                provenance_import_research=dict(
                    as_built_compiler_inputs=len(compiled),changed_compiler_sources=compiled_changes,
                    available_recorded_identity_evidence=[dict(id=eid,record=entries[eid]['record'],sha256=entries[eid]['sha256'])
                        for eid in ['RER31.inputs','RER31.build','RER31.host','RER31.mapped-libraries','RER31.catalog','RER31.music-inputs','RER31.save-domain']],
                    opening_projection='assemble() hardcodes pending actual-player/input/provider fields; those labels do not erase the exact identities subsequently retained by artifact4 research.',
                    required_migration='Import verified source/build/input/provider identity observations into a typed work/provenance projection. Keep recorded provenance, current source/contract support, physical input availability, per-requirement adequacy and acceptance verdicts independent.',
                    current_installation_or_provider_admission_inferred=False),
                backend_frontier=dict(generic_session_cases=sum(c['adapter']=='session' for c in reader.json(plan['scope']['journey_catalog'])['cases']),
                    generic_abi='tools/drawing_session.py reads owned session.json, then expects exact session-report.json identities/effects.',
                    current_presenter='apps/drawing-session/main.cpp consumes presentation.json and writes presentation-report.json; recorded composition is a separate established adapter.',
                    handoff='Connect a real collector/backend/case at EP03; do not relabel a recorded adapter, assert an arbitrary binary implements the generic ABI, or promote its self-reported oracle separation alone.',
                    generic_checker_os_sandbox_enforced=False,
                    trust_boundary='Generic checker omits oracle inputs/paths/results from its ABI and pins/reviews the backend; subprocess execution inherits filesystem/environment access. Strong independence needs trusted source/binary closure and independently verified acquisition/input-role separation, or appropriate enforced access isolation.'),
                migration_handoff=dict(current_mode='opening_pending_record',
                    required_owner_tasks=['EP03','EP10','EP11'],
                    next_steps=['Prove a real EP03 public-player receipt and independent collector validation.',
                        'Version execution-capable event/receipt schema without mutating opening records; validate fields identically on append/read/restore.',
                        'Bind complete requirement/configuration/source/domain/caller/reference identity, typed assertions and independently checked observations.',
                        'Implement dependency-aware reuse, invalidation, current-failure resolution and exact supersession; preserve historical failures.',
                        'Migrate immutable candidates/work events with original IDs/times and verified backups; candidates remain candidates.',
                        'Derive all156 outcomes, applicable scope resolutions and independent15-group recovery verdict; retain explicit pending/fail with reasons.'],
                    user_judgment_required_now=False,
                    product_choice_when='Only a proposed broader/different support domain or compatibility promise needs an explicit product choice after engineering coverage/costs are supplied.'),
                counts=dict(requirement_configuration_rows=len(matrix),candidate_evidence_reviewed=len(diagnostics),
                    source_states=dict(Counter(d['current_source_binding_state'] for d in diagnostics)),
                    candidate_claim_contract_states=dict(Counter(c['contract_state'] for d in diagnostics for c in d['claims'])),
                    original_groups=len(groups),group_behavior_links=sum(len(g['reviewed_behaviors']) for g in groups),
                    sqlite_experiments=len(ledger['probes']),accepted_requirements=0,
                    unresolved_player_scope=latest['counts']['unresolved_scope_decisions'],
                    unresolved_original_scope=latest['counts']['unresolved_original_recovery_scope_decisions']),
                scope='Completed research of existing artifact5 trust boundaries, typed/provenance/aggregation requirements, actual SQLite restore probes and every current requirement/group. No positive public-player receipt, engine comparison or recovery closure is manufactured.')
        except Exception as error:
            report['error']=str(error)
        finally:
            report['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
            report['success'] &= report['sources_stable']
            digest=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
            print(json.dumps(dict(success=report['success'],error=report.get('error'),sha256=digest,counts=report.get('counts'))))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
