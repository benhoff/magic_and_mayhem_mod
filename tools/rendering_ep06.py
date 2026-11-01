"""Requirement-scoped EP06 reconciliation; observations never imply completion."""
from collections import Counter
import hashlib
import importlib.util
from pathlib import Path

from rendering_acceptance import requirements, validate, verify_evidence
from rendering_normal_acceptance import admit, receipt
from rendering_pilot import require
from resident_capture import records

CONFIGURATIONS = ('player-default', 'player-software')
OPERATIONS = (
    'copy-opaque', 'copy-keyed', 'copy-clipped', 'copy-stretched',
    'copy-alias-overlap', 'fill-full', 'fill-clipped', 'partial-write',
    'cpu-write-readback', 'text-font-dc-lifetime', 'image-cropping',
    'sprite-indexed', 'sprite-direct', 'sprite-auxiliary', 'palette-remap',
    'palette-cycling', 'destination-dependency', 'flags-and-return-values')

# These are specific missing proof obligations, not proposed product exclusions.
NEXT = {
 'copy-opaque': 'Bind every required opaque caller and source/destination authority to complete output across the required scene scope.',
 'copy-keyed': 'Keep CPU-keyed proof; obtain an actual keyed-GPU caller or review a precise supported-domain boundary. Zero observations prove no global absence.',
 'copy-clipped': 'Join actual clip rectangles and attached clipper pieces to unchanged exterior words and caller-visible results.',
 'copy-stretched': 'Capture actual required stretch/format/key arguments and independently verify their complete destinations; unscaled COPY proof cannot substitute.',
 'copy-alias-overlap': 'Obtain an actual required alias/overlap caller and frozen-history output; supplied self-copy controls do not establish natural reachability.',
 'fill-full': 'Join actual fill colour, full destination extent and caller result to complete output. The COPY validator explicitly excludes fills.',
 'fill-clipped': 'Bind actual fill colour/clip and preserved exterior, including valid-prefix/error policy. Whole-frame equality alone is insufficient.',
 'partial-write': 'Verify dirty-byte/WORD preservation, valid prefixes and rollback/error effects at actual writer boundaries.',
 'cpu-write-readback': 'Identify required CPU readers/writers and show their ordered authority restoration; enumerate readers outside the native reply channel.',
 'text-font-dc-lifetime': 'Observe font/DC acquire/select/restore/reload/teardown, tied to glyph output and the same resource generations.',
 'image-cropping': 'Join actual whole/cropped image geometry and caller state to palette/history and complete output in the required scene scope.',
 'sprite-indexed': 'Bind actual indexed callers, shade/palette/clip/order and owned generations across required scenes; raster counts alone are insufficient.',
 'sprite-direct': 'Bind actual direct callers and owned generations to complete presented composition across required scenes.',
 'sprite-auxiliary': 'Identify required attachment/auxiliary/effect producers and join their order and ownership to complete output.',
 'palette-remap': 'Bind actual runtime table/provider selection and resource revisions to retained builder proof and remapped output.',
 'palette-cycling': 'Observe actual refresh/input production, selection/delivery, ordering and retirement through required transitions.',
 'destination-dependency': 'Extend complete owned-initialization/intervening-history proof to required callers and scenes; retain any unknown writer or initialization boundary.',
 'flags-and-return-values': 'Check actual return/AX/flags/register/state effects alongside pixels. COPY HRESULTs do not establish all caller ABI effects.'}


def sha(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def current_sources(reader, sources):
    require(sources and all(reader.digest(n) == h for n, h in sources.items()),
            'Execution sources are no longer current')


def bind_artifacts(root, reader, acquired, paths):
    """Validate physical parser inputs against retained immutable acquisition bytes."""
    root = Path(root).resolve()
    artifacts = {r['path']: r for r in acquired['artifacts']}
    require(len(artifacts) == len(acquired['artifacts']), 'Duplicate acquired artifact')
    bound = []
    for path in sorted(set(map(Path, paths))):
        require(path.is_file() and not path.is_symlink(), 'Missing/aliased EP06 parser input')
        name = path.resolve().relative_to(root).as_posix()
        require(name in artifacts, 'Unretained EP06 parser input: ' + name)
        row = artifacts[name]
        require(sha(path) == row['sha256'] == reader.digest(row['record']),
                'Changed EP06 parser input: ' + name)
        bound.append(dict(path=name, record=row['record'], sha256=row['sha256']))
    require(bound, 'No bound EP06 parser inputs')
    return bound


def nominal_facts(root, reader, configuration, pair):
    run = admit(reader, pair, configuration)
    current_sources(reader, run['sources'])
    acquired, compared = [receipt(reader, pair[k]) for k in ('acquisition', 'comparison')]
    cfg = acquired['configuration']
    census = cfg.get('operation_census')
    require(type(census) is dict and set(census) == {'copy_scope', 'coordinate_exercise', 'changes_actions', 'changes_admission'}
            and census['copy_scope'] == 'all'
            and all(census[k] is False for k in ('coordinate_exercise', 'changes_actions', 'changes_admission')),
            'Nominal read-only operation census is unproved')
    require('operations_provenance' not in cfg, 'Manufactured operation actions are ineligible')
    session = Path(acquired['session'])
    directory = Path(acquired['temporary_root'])/'copy-diagnostics'
    profile = Path(acquired['temporary_root'])/'worlds.bin'
    paths = [session/'lock-capture/copy-routes-v2.bin', session/'lock-capture/gpu-copy-census-v1.bin', profile]
    # Validators also open CPU streams and retained pixel witnesses.
    paths += [p for p in directory.rglob('*') if p.is_file()]
    paths += [p for p in (session/'lock-capture').glob('*') if p.is_file()]
    inputs = bind_artifacts(root, reader, acquired, paths)
    from natural_gpu_copy_branches import analyze
    copy = analyze(session, directory)
    require(copy['application_observation_complete'] is True, 'Incomplete application operation census')
    # A missing per-copy closure stays a negative facet, even if every presented
    # frame matched. It does not invalidate the independently admitted journey.
    spec = importlib.util.spec_from_file_location('ep06_producers', Path(root)/'tools/inspect-canvas-producers.py')
    decoder = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(decoder)
    kinds, raster_callers, raster_modes = Counter(), Counter(), Counter()
    world_count = chunks = 0
    for generation, chunk, stream, request in records(profile):
        world_count += not bool(chunk)
        chunks += bool(chunk)
        _, rows = decoder.decode(stream, complete=False)
        for fields, payload in rows:
            kinds[decoder.NAMES[fields[2]]] += 1
            if fields[2] == 9:
                raster_callers[('indexed' if fields[17] else 'direct')+':'+hex(fields[22])] += 1
                raster_modes[str((fields[14], fields[15], fields[17]))] += 1
    require(chunks == compared['original_reference']['source_chunks'], 'Producer source history count differs')
    return dict(configuration=configuration, admission=run, bound_inputs=inputs,
                copy=copy, producer=dict(worlds=world_count, source_chunks=chunks,
                    operations=dict(kinds), raster_callers=dict(raster_callers), raster_modes=dict(raster_modes)),
                command_operations=compared['comparison']['command_operations'],
                cpu_reads=compared['comparison']['cpu_reads'],
                complete_owned_history=True, normal_graphics_retirement=True,
                whole_operation_acceptance=False)


def scoped_facets(operation, facts):
    """Return checked facets with their execution binding, never whole-op passes."""
    out = []
    for fact in facts:
        copy = fact['copy']; producers = fact['producer']['operations']
        checks = []
        if operation.startswith('copy-'):
            if copy['all_application_output_closed']:
                checks.append('Every emitted CPU/GPU COPY in this session has caller-bound complete destination output; fills, no-command plans and retired tails remain separate.')
            branch = {'copy-keyed':'keyed_gpu', 'copy-clipped':'geometric_clipping', 'copy-alias-overlap':'self_copy'}.get(operation)
            if branch and copy['branches'][branch]['proven']:
                checks.append('Actual original GPU branch has checked command intervals: '+branch)
        if operation in ('destination-dependency', 'cpu-write-readback'):
            checks.append('Every presented viewport matches the independent original replay continuously from owned initial history, without original destination or native player pixels as oracle input.')
            if operation == 'cpu-write-readback' and fact['cpu_reads']:
                checks.append('The comparison checked actual native CPU read replies against the same ordered independent history.')
        kind = {'text-font-dc-lifetime':'font', 'image-cropping':'jpeg'}.get(operation)
        if kind and producers.get(kind, 0):
            checks.append('Actual '+kind+' producer records occur within the completely compared history; this does not prove caller state or resource-specific lifetime.')
        raster = {'sprite-indexed':'indexed:', 'sprite-direct':'direct:'}.get(operation)
        if raster and any(n.startswith(raster) for n in fact['producer']['raster_callers']):
            checks.append('Actual '+raster[:-1]+' raster caller records occur within completely compared history; scene adequacy and caller-state joins remain outstanding.')
        if checks:
            out.append(dict(execution=fact['admission']['pair'], world_extents=fact['admission']['world_extents'],
                            provider=fact['admission']['provider'], verified_facets=checks))
    return out


def failed_facts(root, reader, reference):
    """Preserve individual checked calls from a failed journey without admission."""
    acquired = reader.json(reference['record'])
    require(reader.digest(reference['record']) == reference['sha256']
            and acquired['kind'] == 'public_native_normal_acquisition'
            and acquired['success'] is False
            and acquired['sources_stable'] is True
            and acquired['original_manifest_verified_before_after'] is True,
            'Not a source-bound failed nominal acquisition')
    current_sources(reader, acquired['sources'])
    cfg = acquired['configuration']
    require(cfg.get('diagnostic_only', False) is False and not cfg.get('skip_intros')
            and 'injected_fault' not in cfg and 'operation_census' in cfg,
            'Failed execution differs from nominal census scope')
    artifacts = acquired['artifacts']
    sessions = {str(Path(row['path']).parent) for row in artifacts if row['path'].endswith('/resident-world-report.json')}
    if not sessions:
        require(acquired.get('exit_code') is None and 'session' not in acquired,
                'Missing evidence for a launched native session')
        return dict(execution=reference, scenario=acquired['scenario'],
                    configuration='player-software' if cfg['software_flag'] else 'player-default',
                    sources=acquired['sources'], error=acquired.get('error'),
                    launch_preparation_failed=True, actual_provider_unobserved=True,
                    acceptance_credit=False, nominal_journey_admitted=False,
                    boundary='No player launch or rendering observation; this is a retained preparation failure, not a renderer result.')
    require(len(sessions) == 1, 'Ambiguous failed-session generation')
    session = Path(root)/sessions.pop()
    directory = Path(acquired['temporary_root'])/'copy-diagnostics'
    output = Path(acquired['temporary_root'])
    paths = [session/'resident-world-report.json', session/'lock-capture/copy-routes-v2.bin',
             session/'lock-capture/gpu-copy-census-v1.bin', session/'lock-capture/lifecycle.log',
             output/'flow.json', output/'public-launch.log']
    paths += [p for p in directory.rglob('*') if p.is_file()]
    inputs = bind_artifacts(root, reader, acquired, paths)
    from natural_gpu_copy_branches import analyze
    result = analyze(session, directory)
    import json
    flow = json.loads((output/'flow.json').read_text())
    return dict(execution=reference, scenario=acquired['scenario'],
                configuration='player-software' if cfg['software_flag'] else 'player-default',
                sources=acquired['sources'], bound_inputs=inputs, copy=result,
                flow_summary={k:flow.get(k) for k in ('stage','error','save_load','preferences_cancel_verified','display_rebuilds','reloaded_from_main','normal_quit','renderer')},
                acceptance_credit=False, nominal_journey_admitted=False,
                boundary='Only individually complete original-module GPU COPY intervals are proved. Missing output, failed END, normal Quit and specific ownership remain unresolved; no nominal operation or journey credit.')


def reconcile(plan, candidates, facts, negatives=()):
    validate(plan)
    all_rows = requirements(plan)
    selected = [r for r in all_rows if r['owner_task'] == 'EP06' and r['kind'] == 'operation']
    require(len(selected) == 36 and {r['requirement'] for r in selected} == set(OPERATIONS)
            and {r['configuration'] for r in selected} == set(CONFIGURATIONS), 'EP06 denominator changed')
    require(len({(r['configuration'], r['id']) for r in selected}) == 36, 'Duplicate EP06 obligation')
    byid = {r['id']: r for r in candidates}
    require(len(byid) == len(candidates), 'Duplicate candidate evidence')
    rows = []
    for row in selected:
        attached = [byid[eid] for eid in row['candidate_evidence']]
        actual = [f for f in facts if f['configuration'] == row['configuration']]
        facets = scoped_facets(row['requirement'], actual)
        rows.append(dict(configuration=row['configuration'], requirement=row['id'],
                         state='pending', whole_operation_accepted=False,
                         candidate_evidence=attached, current_scoped_proofs=facets,
                         reusable_component_evidence=[e for e in attached if e['current_source_binding_state']=='matching_recorded_sources'],
                         failed_session_scoped_proofs=[dict(execution=n['execution'],
                              checked_original_gpu_calls=n['copy']['checked_original_gpu_calls'],
                              missing_output_calls=n['copy']['missing_output_calls'],
                              nominal_journey_admitted=False, whole_operation_accepted=False)
                              for n in negatives if n.get('configuration')==row['configuration']
                              and row['requirement']=='copy-opaque' and n.get('copy',{}).get('checked_original_gpu_calls',0)>0],
                         contract_remaining=row['pending_reason'], next_proof=NEXT[row['requirement']]))
    return dict(requirements_total=len(all_rows), ep06_obligations=36, ep06_operations=18,
                whole_operations_accepted=0, rows_with_current_scoped_proof=sum(bool(r['current_scoped_proofs']) for r in rows),
                rows=rows, executions=facts, retained_failures=list(negatives),
                ep09_obligations_untouched=6, ep06_gate_obligations_untouched=2, player_accepted=False, original_recovery_accepted=False,
                boundary='Scoped output proof is reusable. Required scene/caller adequacy, absent natural branches, specific lifetime and ABI effects remain explicit; no source review, attachment, count or zero census closes an operation.')


def candidates(root, reader, register, plan):
    ids = sorted({e for r in requirements(plan) if r['owner_task'] == 'EP06' and r['kind'] == 'operation' for e in r['candidate_evidence']})
    return [verify_evidence(root, reader, register, eid) for eid in ids]
