"""Retained pixel/history gate: completed output, defined history and caller state.

This rechecks wire identities and existing independent comparisons. It neither
executes a raster oracle nor turns historical native sources into current ones.
"""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys
import tempfile

from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_pilot import counter, number, require
from retained_artifacts import read_artifact, scoped_artifacts, session_artifact_root
from rendering_operations_history import selected_members, pinned_member
from rendering_capture import capture
from drawing_caller_state import decode, decode_bindings, decode_tail
from resident_capture import records
from rendering_caller_state_join import (profile_operations, join_native, retained_census,
    join_retained_sources, validate_model_results, caller_inventory)

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/pixels-history-gate-results-20261109/'
BID = 'NP.pixels-history-gate-closeout'
SID = 'native.pixels-history-gate-closeout'
GATE = 'gate.pixels-history'
PROFILES = ('player-default', 'player-software')
LEAVES = frozenset(('operation.destination-dependency', 'operation.cpu-write-readback',
                    'operation.flags-and-return-values', 'journey.enabled-movies.both-intros'))
SUPPORT = {
    'destination': 'research/runtime/rendering-destination-results-20261106/current-closeout-binding-01.json',
    'cpu': 'research/runtime/rendering-operations-results-20261107/cpu-transport-binding-01.json',
    'state': 'research/runtime/caller-state-installed-results-20261108/admission-01.json',
    'movies': 'research/runtime/rendering-enabled-intros-results-20261107/closeout-analysis-01.json',
}


def module(name):
    spec = importlib.util.spec_from_file_location('ph_' + name.replace('-', '_'), ROOT/'tools'/(name+'.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def basis(plan, parent):
    validate(plan)
    expected = {(v['configuration'], v['id']): v for v in requirements(plan)}
    rows = parent['requirements']
    require(parent['success'] is True and len(rows) == len(expected) == 156
            and len({(r['configuration'], r['id']) for r in rows}) == 156
            and {(r['configuration'], r['id']) for r in rows} == set(expected), 'Wrong acceptance inventory')
    require(parent['scope_amendments'] == plan['acceptance_scope_amendments']
            and parent['passed_requirements'] == sum(r['state'] == 'passed' for r in rows)
            and parent['pending_requirements'] == sum(r['state'] == 'pending' for r in rows), 'Wrong parent counts/contract')
    for row in rows:
        if row['id'] not in LEAVES | {GATE}: continue
        require(all(row[k] == v for k, v in expected[row['configuration'], row['id']].items()), 'Changed pixel/history definition')
        require(row['state'] == ('pending' if row['id'] == GATE else 'passed'), 'Unaccepted prerequisite or completed gate')
        if row['id'] in LEAVES: require(row['admitted_execution_evidence'], 'Missing primary execution')
    require(parent['decisions'] == {'supported-player': 'pending', 'original-recovery': 'pending'}, 'Final decisions changed')
    return rows


def viewport_index(raw, session):
    """Read bounded metadata, not inflated pixels; the enclosing object is pinned.

    qCompress sizes and the complete journal are checked. Pixel decoding and the
    independent full-plane comparison remain the historical oracle's work.
    """
    require(raw[:8] == b'MNMACE01' and len(raw) <= 2*1024**3+8, 'Wrong viewport envelope')
    at = 8; rows = []; total = clock = 0
    while at < len(raw):
        require(len(rows) < 131072 and len(raw)-at >= 8, 'Viewport row bound/header')
        m, n = struct.unpack_from('<II', raw, at); at += 8
        require(1 <= m <= 65536 and n <= 33*1024**2 and m+n <= len(raw)-at, 'Viewport record extent')
        row = json.loads(raw[at:at+m]); at += m
        require(type(row['index']) is int and row['index'] == len(rows), 'Viewport row gap')
        now = counter(row['host_us']); require(now >= clock, 'Viewport clock regressed'); clock = now
        size = struct.unpack_from('>I', raw, at)[0] if n >= 5 else 0
        require(not n or n >= 5 and 0 < size <= 32*1024**2, 'Compressed plane extent')
        rows.append((row, size)); total += size; at += n
    require(len(rows) >= 2, 'Empty viewport')
    first, last = rows[0][0], rows[-1][0]
    require(first['kind'] == 'open' and first['role'] == 'viewport' and first['session'] == session
            and type(first['version']) is int and first['version'] in (1, 2, 3), 'Foreign viewport session/policy')
    if first['version'] > 1:
        require(first['policy'] == ('campaign-lossless-viewport-v1' if first['version']==2 else 'movie-lossless-viewport-v1')
                and first['keyframe_paints']==128 and first['raw_limit_bytes']==64*1024**3
                and first['row_limit']==131072, 'Unknown lossless viewport policy')
    require(last['kind'] == 'close' and last['complete'] is True and last['error'] == ''
            and last['prior_rows'] == len(rows)-1 and counter(last['raw_bytes']) == total, 'Incomplete viewport history')
    paints = {}; swaps = []; latest = 0
    for row, size in rows[1:-1]:
        if row['kind'] == 'swap':
            ordinal = counter(row['paint'])
            require(ordinal == latest and size == 0, 'Swap lacks most recent completed paint')
            if ordinal: swaps.append(ordinal)
            continue
        require(row['kind'] == 'paint', 'Unknown viewport event')
        ordinal = counter(row['paint']); require(ordinal == latest+1, 'Paint history gap'); latest = ordinal
        width, height = number(row['width'], 1, 4096), number(row['height'], 1, 4096)
        lease, world = counter(row['lease']), counter(row['world'])
        if lease:
            require(size == width*height*4 and row['pixels_captured'] is True
                    and row['native_gpu_frame'] is True and row['gpu_read_complete'] is True
                    and row['transport'] == 'opengl', 'Native output is partial or GPU read incomplete')
            require(row['presentation_owner'] == 'gui' and row['immutable_lease'] is False,
                    'Unreviewed presentation owner policy')
            number(row['commands'], 1, 2**32-1); counter(row['sealed_world'])
        else: require(world == 0 and row['native_gpu_frame'] is False, 'World output lacks native lease')
        paints[ordinal] = row
    require(swaps, 'No actual swaps')
    return dict(paints=paints, swaps=swaps, native=[paints[p] for p in swaps if counter(paints[p]['lease'])])


def pixel_rows(c, observed):
    require(c['success'] is True and c['sources_stable'] is True and c['comparison_processes_stopped'] is True
            and c['original_manifest_verified_before_after'] is True
            and c['original_pixels_used_as_native_inputs'] is False, 'Incomplete independent comparison')
    ref, p = c['original_reference'], c['comparison']
    require(ref['native_player_pixels_read'] is False and ref['original_destination_capture_read'] is False,
            'Dependent pixel oracle')
    require(p['success'] is True and p['mismatching_pixels'] == p['remaining_surfaces'] == 0, 'Mismatched/live comparison')
    native = observed['native']; compared = p['comparisons']
    require(native and len(native) == len(compared), 'Native swap omitted or duplicated')
    require(all(all(type(v[k]) is int for k in ('paint','lease','world','pixels','mismatching_pixels'))
                and v['immutable_lease'] is False and v['reference_binding'] == v['paint'] for v in compared),
            'Untyped or foreign GUI comparison binding')
    # Comparison iterates command bindings; repeated swaps can interleave them.
    actual = sorted((counter(v['paint']), counter(v['lease']), counter(v['world']), v['width']*v['height']) for v in native)
    matches = sorted((v['paint'], v['lease'], v['world'], v['pixels']) for v in compared)
    require(actual == matches, 'Compared paint/lease/generation/extent partition differs')
    require(all(type(v['pixels']) is int and v['pixels'] > 0 and v['mismatching_pixels'] == 0
                and len(v['actual_sha256']) == 64 and v['actual_sha256'] == v['expected_sha256'] for v in compared)
            and p['pixels_compared'] == sum(v['pixels'] for v in compared)
            and p['world_presentations'] == sum(v['world'] > 0 for v in compared)
            and c['observations']['world_swaps'] == p['world_presentations']
            and c['observations']['swaps'] == len(observed['swaps'])
            and c['observations']['paints'] == len(observed['paints']), 'Incomplete full viewport comparison')
    return dict(swaps=len(native), world_swaps=p['world_presentations'], pixels=p['pixels_compared'])


class Join:
    def __init__(self, reader):
        self.reader, self.refs, self.phases = reader, {}, {}

    def checked(self, ref, *, pixels_only=False, protected_pixel_sources=False):
        require(isinstance(ref, dict) and {'record', 'sha256'} <= ref.keys()
                and ref['record'].startswith('research/runtime/') and '..' not in Path(ref['record']).parts
                and self.reader.digest(ref['record']) == ref['sha256'], 'Retained report identity differs')
        value = self.reader.json(ref['record'])
        if pixels_only:
            require(value['comparison']['success'] is True and value['comparison']['mismatching_pixels']==0
                    and (value['sources_stable'] is True or protected_pixel_sources is True)
                    and value['comparison_processes_stopped'] is True
                    and value['original_manifest_verified_before_after'] is True, 'Unsuccessful historical pixel component')
        else: require(value['success'] is True, 'Unsuccessful retained proof')
        self.refs[ref['record']] = dict(ref, whole_report_success=value['success'],
            consumed_scope='independent-pixel-component-only' if pixels_only else 'successful-retained-proof')
        sources = value.get('sources', value.get('analysis_sources', {}))
        self.phases[ref['record']] = dict(recorded_sources=sources,
            recorded_sources_stable=value.get('sources_stable'), historical_failure=value.get('error'),
            protected_pixel_sources_review=protected_pixel_sources,
            source_commit=value.get('source_commit'),
            compiler_sources=value.get('compiler_sources',value.get('player_build',{}).get('compiler_sources')),
            player_binary=value.get('player_binary',value.get('player_build',{}).get('binary')),
            current_drift={n: dict(recorded=h, current=self.reader.digest(n) if (ROOT/n).is_file() else None)
                for n, h in sources.items() if not (ROOT/n).is_file() or self.reader.digest(n) != h},
            current_native_execution_claimed=False)
        return value

    def report(self, name): return self.checked(dict(record=name, sha256=self.reader.digest(name)))

    def pixel_origin(self, c):
        seen=set()
        while True:
            ref=c.get('retained_pixel_comparison',c.get('retained_pixel_phase'))
            if ref is None:return c
            require(ref['record'] not in seen and len(seen)<8,'Cyclic/overlong retained pixel ancestry')
            seen.add(ref['record']);old=self.checked(ref,pixels_only=True,
                protected_pixel_sources=ref.get('protected_pixel_sources_exact') is True)
            require(all(c[k]==old[k] for k in ('acquisition','comparison','observations','captured_command_stream_sha256')),
                    'Retained pixel ancestor is another output/input cohort')
            changes=ref.get('source_phase_changes',ref.get('reviewed_analysis_source_changes',{}))
            require(all(n.startswith(('tools/','tests/')) and n.endswith('.py') for n in changes)
                    and all(c['sources'].get(n)==v['after'] and old['sources'].get(n)==v['before'] for n,v in changes.items())
                    and all(n in changes or old['sources'][n]==h for n,h in c['sources'].items() if n in old['sources']),
                    'Unreviewed native/oracle change across pixel ancestry')
            c=old

    def pair(self, pair, profile):
        print('Rechecking completed output/history:', pair['acquisition']['record'], file=sys.stderr, flush=True)
        a, c = (self.checked(pair[k]) for k in ('acquisition', 'comparison'))
        require(c['acquisition'] == pair['acquisition'], 'Foreign comparison acquisition')
        a,excluded=scoped_artifacts(a,label='pixel/history')
        cfg = a['configuration']
        require(type(cfg['software_flag']) is bool and cfg['software_flag'] is (profile == PROFILES[1])
                and ('--software-rendering' in a['command']) is cfg['software_flag']
                and cfg['terrain_lighting'] == 'native' and not cfg.get('compiled_test_driver_invoked', False)
                and not cfg.get('diagnostic_only', False), 'Wrong profile/native input policy')
        require(a['sources_stable'] is True and a['original_pixels_used_as_native_inputs'] is False
                and a['private_wine_stopped'] is True and a['exit_code'] == 0
                and a['retirement']['state'] == 'completed' and not a.get('forced_private_prefix_stop', False), 'Unretired primary')
        if cfg['software_flag']: require(a['resident']['renderer'].lower().startswith(('llvmpipe', 'softpipe')), 'Software provider missing')
        admission_path=session_artifact_root(a,'session',label='pixel/history')+'/native-output-admission.json'
        admission=json.loads(read_artifact(self.reader,a,admission_path,max_bytes=1024**2))
        require(admission['native_only']['policy']=='native-only-rendering-v1'
                and admission['required_backing_mode']==3 and admission['legacy_embedding'] is False,
                'Original pixels/embedding are not an admitted native output source')
        streams = [v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('viewport') and v['path'].endswith('.bin')]
        require(len(streams) == 1, 'Missing/ambiguous whole viewport capture')
        observed = viewport_index(read_artifact(self.reader, a, streams[0]['path'], max_bytes=2*1024**3+8), a['session_identity'])
        pixels = pixel_rows(c, observed)
        # Historical reference index is stronger than a count/boolean summary.
        origin=self.pixel_origin(c)
        indices = [v for v in origin['artifacts'] if v.get('path') == 'reference/worlds.json']
        if indices:
            require(len(indices) == 1, 'Ambiguous reference history index')
            index_ref = indices[0]; index = self.checked(index_ref)
        else:
            archives = [v for v in origin['artifacts'] if v.get('path') in ('outputs.tar.gz','comparison-outputs.tar.gz')
                        or v.get('encoding') == 'tar-gzip']
            require(len(archives) == 1, 'Missing independent continuous history index')
            archive = archives[0]
            raw = self.reader.read_bytes(archive['record'])
            require(len(raw) == archive['bytes'] and hashlib.sha256(raw).hexdigest() == archive['sha256'], 'Changed oracle archive')
            members = selected_members(raw, lambda n: n if n == 'reference/worlds.json' else None, max_bytes=1024**2)
            expected = [v for v in archive.get('members',[]) if v['path'] == 'reference/worlds.json']
            require(len(expected) <= 1, 'Ambiguous reference index descriptor')
            body=members['reference/worlds.json']
            if expected:body=pinned_member(body,expected[0])
            index=json.loads(body)
            index_ref=dict(archive=archive['record'],archive_sha256=archive['sha256'],path='reference/worlds.json',
                           sha256=hashlib.sha256(body).hexdigest(),bytes=len(body))
        require(index['reference_history'] == 'Owned CPU producer inputs and original raster results chained continuously from initial creation'
                and index['native_player_pixels_read'] is index['original_destination_capture_read'] is False,
                'Undefined, reset or dependent original history')
        require(isinstance(index['worlds'],list) and index['worlds'], 'Missing original World plane index')
        worlds = len(index['worlds']); domain = c['world_domain']
        require([counter(v['generation']) for v in index['worlds']] == list(range(1,worlds+1))
                and [dict(generation=counter(v['generation']),width=v['width'],height=v['height']) for v in index['worlds']]
                == domain['worlds'], 'Independent source plane generation/extent index differs')
        size=[domain['worlds'][0]['width'],domain['worlds'][0]['height']]
        require('actual_size' not in cfg or cfg['actual_size']==size,'Configured/observed initial extent differs')
        require(size in ([640, 480], [800, 600]) and domain['verified'] is True and domain['format'] == 'RGB565'
                and domain['extents'] and all(v in ([640,480], [800,600]) for v in domain['extents'])
                and [v['generation'] for v in domain['worlds']] == list(range(1,worlds+1))
                and sorted({(v['width'],v['height']) for v in domain['worlds']}) == sorted(map(tuple,domain['extents']))
                and worlds == c['original_reference']['worlds'] == int(a['resident']['completed'])
                == int(a['resident']['imports']) == int(a['resident']['verified_returns'])
                and all(counter(v['world']) <= worlds for v in observed['native']), 'Noncontiguous source generation domain')
        streams = [v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('commands') and v['path'].endswith('.bin')]
        require(len(streams) == 1, 'Ambiguous full command history')
        command_rows = capture(read_artifact(self.reader, a, streams[0]['path'], max_bytes=1024**3+8))
        require(command_rows[0][0]['role'] == 'commands' and command_rows[0][0]['session'] == a['session_identity'], 'Foreign command history')
        stream = b''.join(body for row, body in command_rows if row['kind'] == 'fragment')
        require(hashlib.sha256(stream).hexdigest() == c['captured_command_stream_sha256'], 'Different compared command stream')
        life = module('close-rendering-world-exit').imported_lifetimes(stream)
        require(life['nonce']==counter(a['resident']['nonce']), 'Command/source nonce belongs to another original session')
        measured=module('rendering_normal_acceptance').world_domain(dict(commands=stream))
        require(measured==domain,'Reported World domain differs from raw command imports')
        require(len(life['imports']) == worlds and command_rows[-2][0]['kind'] == 'ended'
                and command_rows[-2][0]['commands'] == life['command_end'], 'Command/source history denominator differs')
        commands = dict(nonce=life['nonce'], imports=worlds, command_end=life['command_end'], sha256=c['captured_command_stream_sha256'])
        return dict(pair=pair, configuration=profile, size=size, identity=a['session_identity'], source_generations=worlds,
                    actual_world_extents=domain['extents'], completed_output=pixels, command_history=commands,
                    history_index=index_ref, excluded_other_session_artifacts=excluded, current_native_execution=False)


def attachments(row):
    result = []
    for value in row['admitted_execution_evidence']:
        value = value.get('execution', value)
        require('acquisition' in value and 'comparison' in value, 'Missing exact primary pair')
        result.append({k: value[k] for k in ('acquisition', 'comparison')})
    require(len({fingerprint(p) for p in result}) == len(result), 'Duplicate primary pair')
    return result


def primary_cohort(join, rows):
    """Visit every accepted leaf, including matrix and approved mechanism forms.

    Component destinations and semantic widgets have no whole-viewport credit.
    Their exact existing acceptance scope remains attached to the gate join.
    """
    pairs = {}; singles = {}; mechanisms = []
    for row in rows:
        if row['state'] != 'passed' or row['id'].startswith('gate.'): continue
        if not row.get('admitted_execution_evidence'):
            require(row['id'] == 'operation.copy-alias-overlap' and row.get('admitted_mechanism_evidence'), 'Unreviewed leaf without primary execution')
            for ref in row['admitted_mechanism_evidence']: join.checked(ref)
            mechanisms.append(dict(configuration=row['configuration'], requirement=row['id'], proof=row['admitted_mechanism_evidence']))
            continue
        for item in row['admitted_execution_evidence']:
            value = item.get('execution', item)
            if 'matrix' in item:
                matrix = join.checked(item['matrix']); require(matrix['mismatching_pixels'] == 0, 'Unaccepted matrix')
                values = []
                for ref in matrix['comparisons']:
                    c = join.checked(ref); a = join.checked(c['acquisition'])
                    if a['configuration']['software_flag'] is (row['configuration'] == PROFILES[1]) and a['configuration']['actual_size'] in item['actual_world_extents']:
                        values.append(dict(acquisition=c['acquisition'], comparison=ref))
                require(len(values) == 1, 'Matrix domain omitted or duplicated')
            else: values = [value]
            for value in values:
                if 'acquisition' in value:
                    pair = {k:value[k] for k in ('acquisition','comparison')}; name=pair['acquisition']['record']
                    a=join.checked(pair['acquisition']);c=join.checked(pair['comparison'])
                    flag=a['configuration']['software'] if a['kind']=='native_grimoire_copy_branch_acquisition' else a['configuration']['software_flag']
                    require(c['acquisition']==pair['acquisition'] and type(flag) is bool
                            and flag is (row['configuration']==PROFILES[1]), 'Mixed component/fault/viewport primary identity')
                    old=pairs.setdefault(name,dict(pair=pair,configuration=row['configuration'],requirements=[]))
                    require(old['pair'] == pair and old['configuration'] == row['configuration'], 'Conflicting primary comparison or profile')
                    if row['id'] not in old['requirements']: old['requirements'].append(row['id'])
                else:
                    a=join.checked(value); require(a['configuration']['software_flag'] is (row['configuration']==PROFILES[1]), 'Mixed semantic primary profile')
                    require(a['sources_stable'] is True and a['private_wine_stopped'] is True and a['exit_code']==0, 'Unretired semantic primary')
                    singles[value['record']]=dict(execution=value,configuration=row['configuration'],kind=a['kind'],whole_viewport_pixel_credit=False)
    return pairs, singles, mechanisms


def caller_state(join, case):
    """Current decoding of raw historical publications and independent bytes.

    Unlike the old execution collector this does not require current native
    hashes: it is an explicitly historical-data analysis, with drift retained.
    """
    print('Rechecking retained caller state:', case['acquisition']['record'], file=sys.stderr, flush=True)
    a, c = (join.checked(case[k]) for k in ('acquisition','comparison'))
    require(c['kind'] == 'public_native_caller_state_comparison' and not c.get('supplied_diagnostic_contexts'), 'Diagnostic caller-state input')
    req = join.checked(c['requirements']); inventory = caller_inventory(req)
    require(inventory == c['caller_inventory'] and inventory['sites'] == 63, 'Incomplete caller continuation inventory')
    cps = module('inspect-canvas-producers')
    state = decode(read_artifact(join.reader,a,'caller-state-0000.bin'))
    require(state['version'] == 5, 'Actual V5 completion required')
    bindings = decode_bindings(read_artifact(join.reader,a,'caller-bindings-0000.bin'),state)
    tail = decode_tail(read_artifact(join.reader,a,'caller-source-tail-0000.bin'),cps.decode)
    with tempfile.TemporaryDirectory(prefix='pixels-history-source-') as scratch:
        path=Path(scratch)/'worlds.bin';path.write_bytes(read_artifact(join.reader,a,'worlds.bin'))
        blocks=list(records(path))
    require(tail['generation'] == sum(not ch for g,ch,s,q in blocks)+1
            and tail['source'] == sum(bool(ch) for g,ch,s,q in blocks)+1, 'Incomplete final caller source tail')
    if tail['records']: blocks.append((tail['generation'],tail['source'],tail['stream'],b''))
    ops=profile_operations(blocks,cps.decode)
    native=join_native(state['rows'],ops,req,{r['id']:r for r in bindings['rows']},tail['nonce'])
    census=retained_census(state['rows'])
    retained=join_retained_sources(state['rows'],ops,native['used'],census,tail['nonce'])
    require(fingerprint(census)==fingerprint(c['retained_census']) and fingerprint(retained)==fingerprint(c['retained_source_join']), 'Raw source/caller accounting differs')
    archives=[v for v in c['artifacts'] if v['path']=='outputs.tar.gz'];require(len(archives)==1,'Missing caller expected-byte archive')
    archive=archives[0];raw=join.reader.read_bytes(archive['record'])
    require(len(raw)==archive['bytes'] and hashlib.sha256(raw).hexdigest()==archive['sha256'],'Changed caller model bytes')
    name='state-history/model-results.bin'
    selected=selected_members(raw,lambda n:n if n==name else None,max_bytes=300*1024**2)
    descriptor=next(v for v in archive['members']if v['path']==name)
    fresh=validate_model_results(pinned_member(selected[name],descriptor),native['joined'])
    require(all(c['caller_state'][k]==v for k,v in fresh.items()) and c['caller_state']['normal_retirement'] is True
            and c['caller_state']['history']['model_mismatches']==0 and case['state_cases']==fresh['cases'], 'Caller expected-byte result differs')
    return dict(identity=a['session_identity'],cases=fresh['cases'],models=fresh['models'],caller_sites=inventory['sites'],
                source_accounted=True,independent_expected_bytes=True,current_native_execution=False)


def analyze(reader, parent):
    rows = basis(reader.json('docs/native-rendering-closure.json'), parent); join = Join(reader)
    support = {k: join.report(n) for k, n in SUPPORT.items()}
    destination = join.checked(support['destination']['inputs']['closeout-analysis-05.json'])
    cpu = join.checked(support['cpu']['analysis'])
    for case in cpu['cases']: module('close-rendering-operations').check_case(case)
    require(len(cpu['cases']) == 4, 'Incomplete typed CPU history cases')
    state = support['state']['cases']; module('close-rendering-caller-state').validate_cases(state)
    # Recheck actual saved native completions against retained independent bytes;
    # no model binary/original game runs in this analysis.
    state_fresh = [caller_state(join, case) for case in state]
    cases = {}; linked = []
    for row in rows:
        if row['id'] not in LEAVES - {'journey.enabled-movies.both-intros'}: continue
        refs = attachments(row); domains = set()
        for pair in refs:
            name = pair['acquisition']['record']
            if name not in cases: cases[name] = join.pair(pair, row['configuration'])
            case = cases[name]
            require(case['pair'] == pair and case['configuration'] == row['configuration'], 'Mixed primary/profile pair')
            domains.add(tuple(case['size']))
            collection = state if row['id'] == 'operation.flags-and-return-values' else cpu['cases'] if row['id'] == 'operation.cpu-write-readback' else destination['executions']
            require(any((v.get('pair') or {k: v[k] for k in ('acquisition','comparison')}) == pair for v in collection), 'Primary missing from semantic/history proof')
        require(domains == {(640,480),(800,600)}, 'Missing actual profile/size history domain')
        linked.append(dict(configuration=row['configuration'], requirement=row['id'], executions=refs))
    require(len(cases) == 12 and len({v['identity'] for v in cases.values()}) == 12, 'Duplicated/missing World cohort')
    primary, singles, mechanisms = primary_cohort(join, rows)
    additional = []; components = []; faults = []
    for name, item in primary.items():
        if name in cases: continue
        c = join.checked(item['pair']['comparison'])
        if 'comparisons' in c.get('comparison', {}):
            additional.append(join.pair(item['pair'], item['configuration']))
        elif 'epochs' in c:
            pixels = module('fault_lifetime_gate').pixels(c)
            faults.append(dict(**item, completed_output=pixels, failed_prefix_normal_credit=False))
        elif c.get('kind') == 'public_native_clipping_comparison':
            require(c['complete_output_verified'] is True and c['analysis_completed'] is True
                    and c['positive_clipped_calls'] > 0 and c['native_clip_regions_verified'] is True, 'Incomplete clipped component planes')
            components.append(dict(**item, scope='Complete selected actual CPU/GPU destination planes; no whole viewport credit.'))
        elif c.get('kind') == 'natural_stretch_complete_output_proof':
            require(c['mismatching_pixels'] == 0 and c['compared_pixels'] > 0 and c['natural_stretch_calls'] > 0
                    and all(v['gpu_checkpoint_verified'] is True for v in c['proofs']), 'Incomplete natural stretch component')
            components.append(dict(**item, scope='Complete actual CPU destination and following GPU source/before planes; no whole viewport credit.'))
        else: raise ValueError('Unreviewed primary pixel/history family: '+str(c.get('kind')))
    all_cases = list(cases.values()) + additional
    require(len({v['identity'] for v in all_cases}) == len(all_cases), 'Duplicate World execution identity')
    movies = support['movies']['cases']
    require(len(movies) == 4 and {(v['configuration']['software_flag'],v['configuration']['high_resolution']) for v in movies}
            == {(s,h) for s in (False,True) for h in (False,True)}, 'Incomplete movie profile domain')
    for v in movies:
        output = v['output']; require(v['success'] is True and output['verified'] is True and output['movie_swaps'] > 0
            and output['compared_pixels'] > 0 and output['mismatching_pixels'] == 0
            and set(output['presented_frame_ordinals']) == {'intro0.avi','intro1.avi'}, 'Missing actual independent movie output')
        row = next(r for r in rows if r['configuration'] == PROFILES[int(v['configuration']['software_flag'])] and r['id'] == 'journey.enabled-movies.both-intros')
        require(v['case'] in [e['execution'] for e in row['admitted_execution_evidence']], 'Movie belongs to another accepted leaf')
    return dict(schema=1, success=True, kind='retained_pixels_history_gate_proof', cases=list(cases.values()),
        additional_presented_cases=additional, component_cases=components, fault_cases=faults,
        semantic_primary_cases=list(singles.values()), approved_mechanism_cases=mechanisms,
        primary_comparisons=len(primary), whole_viewport_cases=len(all_cases),
        actual_native_swaps=sum(v['completed_output']['swaps'] for v in all_cases),
        independently_compared_pixels=sum(v['completed_output']['pixels'] for v in all_cases),
        linked_requirements=linked, movie_cases=[dict(execution=v['case'], output=v['output']) for v in movies],
        caller_state_cases=sum(v['cases'] for v in state_fresh), caller_state_reanalysis=state_fresh,
        cpu_reads=sum(len(v['reads']) for v in cpu['cases']),
        retained_records=list(join.refs.values()), retained_execution_source_phases=join.phases,
        original_game_runs=0, new_pixel_comparisons=0, current_whole_player_proved=False,
        scope='Retained bounded default/software pixel/history gate over the exact accepted leaf cohort. Every healthy full-viewport primary joins all actual native leased swaps to completed GUI-owned GPU frames, full independent viewport comparisons and contiguous source/command/reference history. Twelve distinct Quick/Campaign storage and caller-state sessions at actual640/800 bind typed CPU return/write and caller effects; four enabled-intro cases retain their separate independent movie oracle. Expected-negative/fresh fault partitions, selected complete component planes, approved controlled overlap and semantic native widget layout keep separate roles and receive no invented whole-viewport credit. Historical native/original/compiler/oracle phases remain exact; no fresh current-player, physical scanout, arbitrary inputs/providers, other gate or final-decision claim.')


def close_rows(parent, proof_ref, proof):
    rows = copy.deepcopy(parent['requirements'])
    for row in rows:
        if row['id'] != GATE: continue
        selected = [v for v in proof['cases'] if v['configuration'] == row['configuration']]
        require(row['state'] == 'pending' and len(selected) == 6, 'Wrong gate profile domain')
        row.update(state='passed', admitted_execution_evidence=[dict(execution=v['pair'], actual_world_extents=[v['size']],
            gate_proof=proof_ref, reason='Actual native-owned completed GPU paint/swap, complete independently compared viewport, contiguous source/command/reference history and joined CPU/caller effects. Enabled movie output has a separate independent proof; native widget layout is semantic policy. Exact historical execution phases retained.') for v in selected])
    require([v for v in rows if v['id'] != GATE] == [v for v in parent['requirements'] if v['id'] != GATE]
            and sum(v['state']=='passed' for v in rows) == parent['passed_requirements']+2, 'Unrelated acceptance changed')
    return rows
