"""Reconcile required boundaries without turning bounded evidence into exclusions.

This is a retained-data review. It cannot admit player rows or widen support.
The old scope review and every execution fingerprint remain immutable inputs.
"""
import copy
import hashlib
import json
from pathlib import Path

from coverage_claims import fingerprint
from campaign_map_resources import assess as assess_resources

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/required-boundaries-results-20261110/'
BID = 'NP.required-boundaries-review'
SID = 'native.required-boundaries-review'
DOMAIN = 'docs/native-rendering-support-domain.json'
PARENT = 'research/runtime/pixels-history-gate-results-20261109/compatibility-02-admission.json'
INPUTS = {
    'scope': 'research/runtime/rendering-scope-results-20261030/decisions-01.json',
    'callers': 'research/runtime/rendering-scope-results-20261030/callers-03.json',
    'formats': 'research/runtime/rendering-scope-results-20261030/components-03.json',
    'alternate': 'research/runtime/original-alternate-allocation-results-20261026/controls09.json',
    'providers': 'research/runtime/screen-provider-master-results-20261026/providers01.json',
    'history': 'research/runtime/pixels-history-gate-results-20261109/admission-01-proof.json',
    'progression': 'research/runtime/campaign-progression-closeout-results-20261108/acceptance-01.json',
    'routing': 'research/runtime/copy-routing-safety-results-20261109/routing-03.json',
}
PROFILES = {'player-default', 'player-software'}
SIZES = {(640, 480), (800, 600)}
ROUTES = {'pixel/pcx_words': 0x54b5f0, 'pixel/brighten': 0x58d770,
          'pixel/row_copy': 0x58d3b0, 'raster/splice15': 0x57e1b0,
          'minimap/camera3': 0x5532f0}
WITNESSES = ('docs/native-rendering-closure.json', 'docs/native-rendering-closure.md',
             'docs/native-rendering-dependencies.json', 'runtime/native-draw/surfaces.h',
             'compat/legacy/world_frame.cpp', 'apps/qt-shell/viewport_presentation.cpp')


def require(ok, reason):
    if not ok:
        raise ValueError(reason)


def integer(value, minimum=0):
    require(type(value) is int and value >= minimum, 'Expected a typed bounded observation')
    return value


def basis(plan, parent):
    """Keep all 156 exact requirement definitions and both independent decisions."""
    rows = parent['requirements']
    definitions = {
        (c['id'], 'journey.' + j['id'] + '.' + leaf['id'])
        for c in plan['completion_contract']['configurations']
        for j in plan['required_gap_register']['journeys'] for leaf in j['subrequirements']
    } | {(c['id'], prefix + v['id'])
         for c in plan['completion_contract']['configurations']
         for prefix, group in [('operation.', 'operations'), ('gate.', 'gates')]
         for v in plan['required_gap_register'][group]}
    keys = [(v['configuration'], v['id']) for v in rows]
    require(parent['success'] is True and len(keys) == len(set(keys)) == len(definitions) == 156
            and set(keys) == definitions, 'Changed or duplicate acceptance inventory')
    require(parent['passed_requirements'] == sum(v['state'] == 'passed' for v in rows) == 148
            and parent['pending_requirements'] == sum(v['state'] == 'pending' for v in rows) == 8,
            'Acceptance counts or states differ from reviewed parent')
    require(parent['decisions'] == {'supported-player': 'pending', 'original-recovery': 'pending'},
            'Review cannot decide player or original recovery')
    require(parent['scope_amendments'] == plan['acceptance_scope_amendments'], 'Changed overlap amendment')
    amendment = next(v for v in parent['scope_amendments']
                     if v['id'] == 'ALIAS09.controlled-opaque-overlap-v1')
    require(amendment['remaining_obligation'] == dict(scope_decision='SC.natural-callers',
            gate='gate.required-boundaries', state='pending'), 'Controlled overlap erased natural obligation')
    for row in rows:
        if row['id'] == 'gate.required-boundaries':
            require(row['state'] == 'pending' and row['admitted_execution_evidence'] == [],
                    'Boundary review cannot inherit an already closed gate')
    return rows


def check_callers(scan, routing):
    rows = scan['routes']
    require(scan['success'] is True and scan['global_unreachable_proved'] is False
            and len(rows) == len(ROUTES)
            and {v['route']: v['start'] for v in rows} == ROUTES,
            'Caller investigation identity or uncertainty changed')
    require(all(v['natural_caller_recovered'] is False and v['classification'] == 'unused_candidate'
                for v in rows), 'New caller evidence needs a new requiredness review')
    require(scan['dispatch_witnesses']['five_entries_selected'] is False
            and scan['dispatch_witnesses']['other_dispatches_closed'] is False,
            'Local selector cannot establish all indirect targets')
    require(routing['success'] is True and routing['completed_obligation'] == 'no_unsafe_overlap_in_owned_copy_routing'
            and routing['global_game_overlap_absence'] is False
            and routing['natural_overlap_witness'] is False
            and routing['acceptance_rows_promoted'] == 0,
            'Owned memory safety cannot supply natural output/reachability')
    return dict(routes=copy.deepcopy(rows), installed_route_denominator=43,
                observed_cross_context_union=38, global_unreachable_proved=False,
                owned_copy_memory_safety=True, natural_overlap_proved=False)


def check_formats(controls, source):
    require(controls['success'] is True and controls['strict_player_executed'] is False
            and controls['physical_device_reset'] is False, 'Component evidence was widened')
    rows = controls['format_rows']
    require([v['masks'] for v in rows] == [[0xf800, 0x7e0, 31], [0xf800, 0x7e0, 31], [0x7c00, 0x3e0, 31]]
            and all(v['remaining_surfaces'] == v['remaining_devices'] == 0 for v in rows),
            'Missing RGB565/RGB555 descriptor and lease controls')
    require('policy!=WorldInputPolicy::rgb565&&policy!=WorldInputPolicy::colourRgb555' in source
            and 'frame.colourRgb555&&op!=MNM_WORLD_COLOUR_RECT' in source
            and 'Noncanonical RGB555 World source' in source, 'World format admission changed')
    return dict(surface_storage=['RGB565', 'explicit RGB555'], strict_world_transport='RGB565',
                offline_rgb555_world='canonical colour rectangles only',
                live_rgb555_production_proved=False, component_is_strict_player=False)


def check_history(proof, parent):
    refs = {v['gate_proof']['record']: v['gate_proof']['sha256']
            for row in parent['requirements'] if row['id'] == 'gate.pixels-history'
            for v in row['admitted_execution_evidence']}
    require(INPUTS['history'] in refs, 'History proof is not admitted by the parent')
    require(proof['success'] is True and proof['kind'] == 'retained_pixels_history_gate_proof'
            and proof['current_whole_player_proved'] is False
            and proof['new_pixel_comparisons'] == 0, 'Wrong history evidence role')
    cases = proof['cases']
    require(len(cases) == 12 and {v['configuration'] for v in cases} == PROFILES
            and {(v['configuration'], tuple(v['size'])) for v in cases} ==
            {(p, s) for p in PROFILES for s in SIZES}, 'Incomplete owned-history domain')
    require(all(v['source_generations'] > 0 and v['command_history']['imports'] == v['source_generations']
                and v['command_history']['command_end'] > 0 and v['history_index']['sha256']
                and v['completed_output']['pixels'] > 0 and v['current_native_execution'] is False
                for v in cases), 'Missing ordered source/command/reference history')
    return dict(accepted_cohort_history_resolved=True, whole_viewport_cases=proof['whole_viewport_cases'],
                actual_swaps=proof['actual_native_swaps'], historical_compared_pixels=proof['independently_compared_pixels'],
                cpu_replies=proof['cpu_reads'], caller_states=proof['caller_state_cases'],
                historical_eight_frame_counterexample_retired=False, current_whole_player_proved=False)


def check_progression(report):
    cases = report['cases']
    require(report['success'] is True and len(cases) == 4
            and len({v['identity'] for v in cases}) == 4
            and {(v['configuration'], tuple(v['extent'])) for v in cases} ==
            {(p, s) for p in PROFILES for s in SIZES}, 'Missing actual later-map domain')
    result = []
    for c in cases:
        o, r, n = c['original'], c['resources'], c['native']
        require(c['verified'] is True and o['verified'] is True and r['verified'] is True and n['verified'] is True,
                'Unverified route/resource case')
        require(integer(o['semantic']['natural_outcome']) == 1 and o['semantic']['later_map_entered'] is True
                and o['later_entry']['realm'] == 'Celtic' and integer(o['later_entry']['region']) == 2
                and o['report_ok'] > 0 and o['original_entry_callbacks'], 'No natural Report/Realm/later-World route')
        require(integer(r['world_epochs']) == 2 and [integer(v['epoch']) for v in r['retirements']] == [1, 2],
                'Missing World retirement epochs')
        initial, later = r['initial_resources'], r['later_resources']
        for v in r['retirements']:
            before, after = v['before'], v['after']
            require(integer(before['active']) == 1 and integer(after['active']) == 0
                    and before['epoch'] == after['epoch'] == v['epoch']
                    and before['thread'] == after['thread'] == initial['thread']
                    and before['ordinal'] < after['ordinal'], 'Wrong retirement phase/thread/order')
            require(before['canvas_interfaces'] == after['canvas_interfaces'] == initial['canvas_interfaces']
                    and all(before['canvas_interfaces']), 'Persistent canvases were relabeled detached')
            require(all(after['globals'][str(a)] == 0 for a in (0x6de558, 0x6a28f8, 0x6e266c, 0x6903e8)),
                    'Required original bitmap/temporary/selection stores did not retire')
        require(initial['active'] == later['active'] == 1 and initial['epoch'] == 1 and later['epoch'] == 2
                and r['retirements'][0]['after']['ordinal'] < later['ordinal']
                and initial['globals'][str(0x6de558)] and later['globals'][str(0x6de558)],
                'Later populated resource acquisition precedes retirement')
        require(n['surviving_command_surfaces'] == 0
                and n['previous_world']['deleted'] < n['first_later_world']['created']
                and n['first_later_world']['generation'] > n['previous_world']['generation'],
                'Native World generation overlaps or survives retirement')
        result.append(dict(configuration=c['configuration'], extent=c['extent'], identity=c['identity'],
            acquisition=c['acquisition'], comparison=c['comparison'], original_world_epochs=2,
            original_retirements=2, persistent_canvas_interfaces=initial['canvas_interfaces'],
            canvas_identity_scope='within this execution only', realm='Celtic', later_region=2,
            all_screen_helpers_recovered=False))
    return result


def resource_journal(reader, acquired, expected):
    """Reanalyze at most 15,424 retained bytes, rather than acquire a new game."""
    candidates = [v for v in acquired['artifacts'] if v['path'].endswith('/map-resource-boundaries.bin')]
    require(len(candidates) == 1, 'Missing or ambiguous bounded resource journal')
    ref = candidates[0]
    require(type(ref['bytes']) is int and 384 <= ref['bytes'] <= 64+48*320,
            'Resource journal exceeds its declared capture bound')
    body = reader.read_bytes(ref['record'])
    require(len(body) == ref['bytes'] and hashlib.sha256(body).hexdigest() == ref['sha256'],
            'Resource journal payload differs')
    observed = json.loads(json.dumps(assess_resources(body, acquired['flow'])))
    require(observed == expected, 'Raw resource journal differs from the admitted original lifetime')
    return {k: ref[k] for k in ('record', 'sha256', 'bytes')}


def check_domain(policy, plan):
    """The owner's finite support choice does not amend technical proof rules."""
    require(policy['schema'] == 1 and policy['id'] == 'demonstrated-single-player-domain-v1'
            and policy['kind'] == 'native_rendering_support_policy'
            and policy['authorization']['answer'] == 'Use demonstrated finite domains',
            'Unsupported or unselected support policy')
    require(policy['evidence_cohort'] == dict(record=PARENT,
            sha256='f69a5984e90c5b9a42e287b165029e7b8a3e7504777785840588f20eed58ff67'),
            'Support cohort changed without a scope review')
    require(policy['original_build_sha256'] == plan['original_sha256']
            and len(policy['configurations']) == 2 and set(policy['configurations']) == PROFILES,
            'Changed original build or support profiles')
    require(policy['input_domain'] == 'exact_demonstrated_acquisition_and_scenario_identities'
            and policy['installed_assets_are_supported_maps'] is False
            and policy['arbitrary_maps_saves_duration_supported'] is False,
            'Installed assets or finite cases cannot grant arbitrary input support')
    gaps = plan['required_gap_register']
    require(policy['preserve_all_journeys_and_operations'] is True
            and policy['journeys'] == len(gaps['journeys']) == 14
            and policy['journey_leaves'] == sum(len(v['subrequirements']) for v in gaps['journeys']) == 47
            and policy['operations'] == len(gaps['operations']) == 21
            and policy['required_outcomes'] == ['victory', 'defeat', 'actual-later-map'],
            'Support choice dropped a journey, operation or outcome')
    require(policy['technical_unknowns_waived'] is False and policy['source_phase_boundaries_preserved'] is True
            and policy['expansion_requires_affected_input_provider_output_and_lifetime_evidence'] is True,
            'Finite support cannot waive unknowns, source phases or future validation')
    return copy.deepcopy(policy)


def assess(plan, parent, inputs, world_source, support=None):
    basis(plan, parent)
    old = inputs['scope']
    conditions = plan['completion_contract']['scope_decisions']
    require({v['id'] for v in conditions} == {v['id'] for v in old['decisions']}
            and all(next(d for d in old['decisions'] if d['id'] == v['id'])['close_when'] == v['close_when']
                    for v in conditions), 'Historical closing conditions changed')
    require(inputs['alternate']['original_allocation_recovered'] is False
            and inputs['alternate']['provider_lifecycle_authored'] is True,
            'Compatibility owner was relabeled historical allocation recovery')
    require(inputs['providers']['platform_file_profile_heap_com_codec_endpoints_supplied'] is True
            and inputs['providers']['windows_seh_dispatch'] is False
            and inputs['providers']['physical_driver_equivalence'] is False,
            'Supplied endpoints were relabeled installed provider recovery')
    callers = check_callers(inputs['callers'], inputs['routing'])
    formats = check_formats(inputs['formats'], world_source)
    history = check_history(inputs['history'], parent)
    progression = check_progression(inputs['progression'])
    answers = {
        'SC.alternate-ownership': ('pending', 'Native owners and the opt-in original compatibility owner have separate evidence. The historical allocator/publisher and a precise required-player exclusion remain unrecovered.',
            'Bind DEBUG/padding, allocation/publication/writer/retirement and actual required caller configurations; an authored owner or zero watches cannot close the chain.'),
        'SC.padded-fullscreen': ('pending', 'Qt toolbar/F9 fullscreen is required. Synthetic fullscreen controls and actual800/640/800 rebuilds cover different transitions.',
            'Observe actual toolbar/F9 entry/exit, output, input/focus/media and retirement in both profiles and frame sizes. Keep original exclusive/padded DirectDraw separate.'),
        'SC.natural-callers': ('pending', 'Five unused candidates remain in43 installed routes. Owned copy memory safety and controlled opaque overlap do not prove natural caller output or absence.',
            'Recover required indirect/raw-reader/writer/alias callers or an exact evidenced caller/configuration boundary, retaining the ALIAS09 natural obligation and all handlers.'),
        'SC.prior-history': ('resolved_for_admitted_cohort', 'PH09 joins owned origin/contiguous history and completed independently compared output for its exact accepted cohort. The older late-interval counterexample remains historical.',
            'A new input/source/provider claim must join its own prerequisite history; no current combined-player or arbitrary earlier prefix is inferred.'),
        'SC.map-save-domain': ('pending', 'Actual Celtic region1 victory/region2 output and exact original Save inputs now supplement Quick Battle and storage evidence. Demonstrated inputs are not an arbitrary support promise.',
            'Select the explicit supported domain, pin actual maps/saves and observed durations, and retain all14 journeys/21 operations. A900second deadline is not900seconds of observed success.'),
        'SC.screen-provider-lifetime': ('pending', 'Four real route-driven populated two-World lifetimes now replace manual ordering as evidence leads. Persistent display interfaces survive map retirement; selected original stores and native incarnations retire.',
            'Complete required Main/Realm/World/Report helper/vtable/callback ownership and failure effects. Neither route success nor process exit proves every provider or Windows exception path.'),
        'SC.rgb555-live': ('pending', 'Explicit RGB555 offscreen storage/retained WORDs are supported by a non-strict facade control. Strict RGB565 World input and offline colour-only RGB555 are distinct contracts.',
            'Recover actual required source/sprite/effect format production and its complete path, or establish an exact justified supported-format boundary; strict rejection is not natural unreachability.'),
        'SC.physical-compatibility': ('resolved_bounded_policy', 'The existing target is x86 Linux/Wine/Qt/X11/OpenGL with actual provider identity recorded per execution; software results name llvmpipe. No Windows driver identity or physical reset promise.',
            'Keep actual backend admission and logical fault/cleanup/fresh-generation checks; platform expansion needs separate evidence.'),
    }
    if support is not None:
        check_domain(support, plan)
        answers['SC.map-save-domain'] = ('resolved_bounded_policy',
            'Owner-selected exact demonstrated acquisition/scenario domains. Include actual accepted Quick selectors and Celtic region1/2, exact original Save inputs and observed durations. All14 journeys/47leaves/21operations remain required.',
            'New maps/saves/durations require affected input/provider/output/lifetime proof. Installed asset availability and900second deadlines do not widen the support promise. Technical unknowns and both final decisions remain pending.')
    decisions = [dict(id=v['id'], blocking_condition=v['blocking_condition'], close_when=v['close_when'],
        state=answers[v['id']][0], answer=answers[v['id']][1], next_required_work=answers[v['id']][2],
        original_recovery_state='unresolved') for v in conditions]
    return dict(decisions=decisions, callers=callers, formats=formats, history=history,
                actual_provider_routes=progression, gate_ready=False,
                pending_scope_ids=[v['id'] for v in decisions if v['state'] == 'pending'],
                acceptance_rows_preserved=True, supported_player_accepted=False, original_recovery_complete=False)


def analyze(reader):
    """Hash-bind every consumed retained result and keep its original source phase."""
    plan = reader.json('docs/native-rendering-closure.json')
    parent = reader.json(PARENT)
    payloads, refs, phases = {}, {}, {}
    for key, name in INPUTS.items():
        value = reader.json(name)
        require(value['success'] is True, 'Unsuccessful retained input: ' + name)
        payloads[key] = value
        refs[key] = dict(record=name, sha256=reader.digest(name))
        sources = value.get('sources', value.get('analysis_sources', {}))
        phases[key] = dict(recorded_sources=copy.deepcopy(sources),
            current_drift={n: dict(recorded=h, current=reader.digest(n) if (ROOT/n).is_file() else None)
                           for n, h in sources.items() if not (ROOT/n).is_file() or reader.digest(n) != h},
            current_execution_claimed=False)
    history_refs = {v['gate_proof']['record']: v['gate_proof']['sha256']
                    for row in parent['requirements'] if row['id'] == 'gate.pixels-history'
                    for v in row['admitted_execution_evidence']}
    require(history_refs.get(INPUTS['history']) == refs['history']['sha256'], 'Accepted history digest differs')
    policy = check_domain(reader.json(DOMAIN), plan)
    require(reader.digest(PARENT) == policy['evidence_cohort']['sha256'], 'Support cohort payload changed')
    result = assess(plan, parent, payloads, reader.read_text('compat/legacy/world_frame.cpp'), policy)
    domains = []
    journals = []
    for c, admitted in zip(result['actual_provider_routes'], payloads['progression']['cases'], strict=True):
        ref = c['acquisition']
        require(reader.digest(ref['record']) == ref['sha256'], 'Progression acquisition digest differs')
        a = reader.json(ref['record'])
        require(a['success'] is True and a['session_identity'] == c['identity']
                and a['configuration']['id'] == c['configuration']
                and a['original_pixels_used_as_native_inputs'] is False
                and a['map_inputs_stable'] is True, 'Wrong actual progression input/session domain')
        journals.append(resource_journal(reader, a, admitted['resources']))
        checkpoint = a['entry_checkpoint']['imported']
        require(checkpoint['sha256'] == '8862488014bfb67c34c4e080adb46601d4cd1a55f64042e349f73aef95150818'
                and checkpoint['bytes'] == 357718, 'Different original transition Save needs review')
        domains.append(dict(configuration=c['configuration'], extent=c['extent'], acquisition=ref,
            original_save={k: checkpoint[k] for k in ('name', 'sha256', 'bytes')},
            installation_manifest_sha256=fingerprint(a['installation_inputs']),
            installation_entries=len(a['installation_inputs']),
            deadline_ms=a['configuration']['deadline_ms'],
            observed_flow_elapsed_ms=integer(a['flow']['elapsed_ms'], 1),
            deadline_is_observed_duration=False, arbitrary_maps_saves_duration_supported=False))
    return dict(schema=1, kind='rendering_required_boundaries_review', success=True, **result,
        parent=dict(record=PARENT, sha256=reader.digest(PARENT)),
        completion_contract_sha256=fingerprint(plan['completion_contract']),
        scope_amendments=copy.deepcopy(parent['scope_amendments']), retained_inputs=refs,
        retained_source_phases=phases, demonstrated_campaign_inputs=domains, raw_resource_journals=journals,
        preserved_acceptance_counts=dict(passed=148, pending=8, total=156),
        original_game_runs=0, new_native_executions=0, new_pixel_comparisons=0,
        support_promise_selected=True, support_policy=dict(record=DOMAIN, sha256=reader.digest(DOMAIN), policy=policy),
        historical_failures_retired=False,
        scope='Evidence-bound required-boundary review. Three bounded conclusions (accepted-cohort history, owner-selected demonstrated input domain and existing platform policy); five technical conditions remain pending. No acceptance-row, final-decision, original-recovery or current combined-player promotion.')
