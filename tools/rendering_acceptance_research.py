"""Research diagnostics for artifact 5; this module cannot admit acceptance.

Current-source hashes, execution-contract identities and complete provenance
are separate checks. None establishes requirement adequacy or player closure.
"""
from coverage_claims import behavior_contract, required_sources, EXECUTION_KINDS


def claim_diagnostics(claims, behaviors, builds, recorded_sources):
    result=[]
    seen=set()
    for claim in claims:
        bid=claim['behavior']
        if bid in seen:
            raise ValueError('Duplicate execution behavior claim')
        seen.add(bid)
        if bid not in behaviors:
            result.append(dict(behavior=bid,contract_state='retired_behavior',missing_sources=[]))
            continue
        current=behavior_contract(behaviors[bid],builds)
        result.append(dict(behavior=bid,recorded_contract=claim['contract_sha256'],current_contract=current,
            contract_state='matching' if claim['contract_sha256']==current else 'changed',
            missing_sources=sorted(required_sources(behaviors[bid])-set(recorded_sources)),
            scoped_scenario_bindings=claim.get('scenarios',{})))
    return result


def candidate_diagnostics(evidence, payload, verified, behaviors, builds):
    from coverage_audit import pointer, source_hashes
    recorded=source_hashes(pointer(payload,evidence['sources_pointer']))
    bindings=evidence.get('source_bindings',{n:n for n in recorded})
    sources={name:recorded[key] for key,name in bindings.items()
             if key not in evidence.get('recorded_inputs',[])}
    claims=pointer(payload,evidence['claims_pointer']) if 'claims_pointer' in evidence else []
    details=claim_diagnostics(claims,behaviors,builds,sources)
    blockers=[]
    if payload.get('success') is not True:
        blockers.append('execution_success_not_true')
    if evidence['kind'] not in EXECUTION_KINDS:
        blockers.append('not_execution_evidence')
    if verified['current_source_binding_state']!='matching_recorded_sources':
        blockers.append('recorded_source_dependencies_changed')
    if not details:
        blockers.append('no_scope_bound_execution_claim')
    if any(c['contract_state']!='matching' for c in details):
        blockers.append('execution_contract_changed_or_retired')
    if any(c['missing_sources'] for c in details):
        blockers.append('execution_claim_source_closure_incomplete')
    # This is a research inventory. No collector-specific public-player receipt
    # exists in the opening protocol, even if all preceding checks match.
    blockers.append('public_player_positive_receipt_admission_not_established')
    return dict(id=evidence['id'],record=evidence['record'],sha256=evidence['sha256'],
        kind=evidence['kind'],scope=evidence['scope'],
        payload_and_assertions_verified=verified['payload_and_assertions_verified'],
        recorded_success=payload.get('success'),
        current_source_binding_state=verified['current_source_binding_state'],
        changed_sources=verified['changed_sources'],claims=details,blockers=blockers,
        admitted_as_player_receipt=False,closes_requirement=False)


GATE_RESEARCH={
    'identity': 'Bind a receipt to exact contract/gap/plan revisions, requirement/configuration, original build, as-built compiler/binary closure, asset/save/generated-World identities, actual backend and selected media. A request for software is not its observed provider.',
    'pixels-history': 'Join actual painted/swapped native-owned generation/lease and completion to full composed output, contiguous prerequisite history, independent reference and caller effects. Queued frame callbacks and successful COPY samples cannot replace this join.',
    'journeys': 'Require each of the47 named leaves in the same exact configuration/domain. Semantic Qt UI checks and original World pixel proof have distinct roles. Derive from admitted receipts, never a supplied coverage summary.',
    'operations': 'Join all21 required operation/failure scopes to adequate independent comparisons and required caller/input/state domains. Component proof remains useful without becoming whole-journey proof.',
    'transitions': 'Require640x480/800x600 rebuild/rebind, actual next-map progression, and distinct legitimate victory/defeat results. Save reload, resize and Quit-created defeat cannot replace omitted outcomes.',
    'input-media': 'Bind actual action request/ack/effect and owned input/presentation, enabled movie completion/skip/return and selected music/audio state. Keep muted-input failure expectations and audible controls separate.',
    'normal-lifetime': 'Require original normal Quit, balanced World import/completion/return, no pending work, and process/channel/cursor/graphics retirement for the same healthy session. A forced stop or expected fault supplies no normal-quit credit.',
    'fault-lifetime': 'Use a distinct expected-negative receipt with exact error/state and cleanup, fresh-generation restart or declared isolated continuation and no duplicated work. Do not turn safe refusal into normal-path success or synthetic loss into physical reset.',
    'required-boundaries': 'Join each applicable scope condition to its evidence-backed resolution. Required unknowns remain blocking for their affected claim. Preserve the current bounded platform policy; absence of observations is not unreachability.',
    'separate-decisions': 'Record two independent verdicts with evidence and remaining limits. This gate requires separate decisions, not two positive decisions: player acceptance can coexist with pending original recovery. Neither verdict is derived from a majority or percentage.'
}


JOIN_RULES=[
    dict(id='typed_identity',rule='Use exact typed identities for requirement/configuration/source/domain/caller/reference policy; True is not1. Reject unknown or duplicate identities.'),
    dict(id='admission_before_projection',rule='Verify payload hash, registered assertions, collector identity/schema, complete source closure, execution and scenario contract, actual configuration and reference/ownership boundary before projecting any result.'),
    dict(id='authority',rule='A digest detects changed bytes; a self-authored dictionary with a freshly recomputed digest is not proof of trusted collection. Derived summaries cannot authenticate themselves.'),
    dict(id='failure_history',rule='Retain failed/pending/unsupported attempts. A fresh required normal-path failure blocks its affected row. Retained historical mismatches stay historical unless source/caller/configuration relevance is established.'),
    dict(id='supersession',rule='No last-timestamp-wins rule. A later repair needs a new receipt, explicit affected input/scope/source relationship, rerun proof and named superseded failure; keep both records.'),
    dict(id='reuse',rule='Cross-configuration reuse needs an explicit per-requirement proof of unchanged provider-sensitive dependencies and compatible inputs/caller/reference scope. Never copy all78 outcomes because one software probe works.'),
    dict(id='separate_decisions',rule='Supported-player closure consumes all156 requirements, applicable scope conditions and admitted gates. Original recovery consumes15 contract-group recovery exits; native implementation, original comparison, live integration and replacement stages remain independent.'),
    dict(id='versioned_migration',rule='Introduce a new execution-capable dossier version with frozen input bindings and audited migration. Restore the immutable opening events as candidates/work observations, not as execution results. Preserve original IDs/times/bytes and back up both states.'),
    dict(id='transactional_validation',rule='Apply the same event/metadata validation on append, restore and read; reject malformed batches atomically. Verify candidate registry/payload assertions in the public restore path; a lower-level workspace restore is not positive evidence admission.'),
    dict(id='invalidation',rule='Source/domain/provider/reference-policy changes invalidate affected adequacy claims; propagate through the dependency join and rerun the pilot after shared capture/history changes. Preserve previous fingerprints.'),
    dict(id='recoverability',rule='Pin every referenced local payload and verified SQLite backup/configuration, test restoration and retain failures. Publication is separate from local completion and cannot alter an engine verdict.')
]
