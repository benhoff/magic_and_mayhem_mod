"""Retained input/media gate joins; original/native execution phases stay exact."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path

from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_menu_acceptance import artifact
from rendering_pause import validate_dwell
from rendering_pause_assessment import read_samples
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/input-media-gate-results-20261109/'
BID = 'NP.input-media-gate-closeout'
SID = 'native.input-media-gate-closeout'
GATE = 'gate.input-media'
PROFILES = ('player-default', 'player-software')
LEAVES = frozenset('journey.' + group + '.' + name for group, names in {
    'enabled-movies': ('both-intros', 'movie-skip', 'return-to-menu'),
    'camera-motion': ('four-orientations', 'pan', 'viewport-boundaries'),
    'cursor-interaction': ('cursor-motion', 'hotspot', 'picking', 'selection'),
    'hud-minimap-tooltips': ('hud-update', 'minimap-update', 'markers', 'tooltip-open-close'),
    'pause-preferences': ('pause', 'preferences-change', 'cancel', 'resume'),
}.items() for name in names)
INTRO = 'research/runtime/rendering-enabled-intros-results-20261107/closeout-analysis-01.json'


def module(name):
    spec = importlib.util.spec_from_file_location('input_media_' + name.replace('-', '_'), ROOT/'tools'/(name+'.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def positive(value, message):
    require(type(value) is int and value > 0, message)


def pause_ownership(rows):
    proof = validate_dwell(rows)
    require(all(v['media']['status'] in (3, 4)
                and v['media']['request'] == v['media']['response'] == v['media']['accepted']
                and v['music']['request'] == v['music']['response']
                and v['music']['failed'] == v['music']['losses'] == 0 for v in rows),
            'Failed or unacknowledged nominal media/music baseline')
    return proof


def pixel_proof(c):
    p = c['comparison']
    ref = c['original_reference']
    require(c['success'] is True and c['sources_stable'] is True
            and c['comparison_processes_stopped'] is True
            and c['original_manifest_verified_before_after'] is True
            and c['original_pixels_used_as_native_inputs'] is False,
            'Incomplete historical independent comparison')
    require(ref['continuous_from_owned_initial_history'] is True
            and ref['native_player_pixels_read'] is False
            and ref['original_destination_capture_read'] is False,
            'Independent owned history is missing')
    rows = p['comparisons']
    require(p['success'] is True and p['mismatching_pixels'] == 0
            and p['remaining_surfaces'] == 0 and rows
            and p['pixels_compared'] == sum(v['pixels'] for v in rows)
            and all(v['pixels'] > 0 and v['mismatching_pixels'] == 0
                    and v['actual_sha256'] == v['expected_sha256'] for v in rows),
            'Incomplete or mismatching presented output')
    return p


def fullscreen(a, c):
    pixel_proof(c)
    flow = a['flow']['fullscreen_transitions']
    proof = c['fullscreen']
    require(proof['verified'] is True and len(flow) == 4
            and [v['fullscreen'] for v in flow] == [True, False, True, False]
            and all(v['accepted'] is True for v in flow)
            and len(proof['intervals']) == 2, 'Required F9 entry/exit missing')
    compared = c['comparison']['comparisons']
    paints = {v['paint'] for v in compared if v['world'] > 0}
    for index, interval in enumerate(proof['intervals']):
        begin, end = flow[2*index:2*index+2]
        require(interval['begin'] == begin and interval['end'] == end,
                'F9 interval belongs to another action')
        shown = interval['presented_paints']
        lo, hi = int(begin['paint_before']), int(end['paint_before'])
        require(lo < hi and shown and set(shown) <= paints
                and all(lo < v <= hi for v in shown),
                'Fullscreen lacks independently compared World presentation')
        require(any(v['paint'] in shown and v['pixels'] == begin['screen_width']*begin['screen_height']
                    for v in compared), 'Fullscreen actual screen extent is unproved')
    ack = c['input_acknowledgements']
    require(ack['verified'] is True and ack['right_downs'] == ack['releases'] == 2
            and len(ack['cases']) == 2, 'Qt input delivery/release missing')
    for v in ack['cases']:
        require(v['channel_sequence'] >= v['published_sequence'] > 0
                and v['world_before_action'] > 0 and v['paint_before_action'] > 0
                and v['original_thread'] > 0 and v['result'] & 0x8000,
                'Original input acknowledgement is unbound')
    return dict(verified=True, intervals=copy.deepcopy(proof['intervals']), input=copy.deepcopy(ack))


def basis(plan, parent):
    validate(plan)
    definitions = requirements(plan)
    rows = parent['requirements']
    require(parent['success'] is True and len(rows) == len(definitions) == 156,
            'Wrong acceptance inventory')
    keys = [(v['configuration'], v['id']) for v in rows]
    require(len(set(keys)) == 156 and set(keys) == {(v['configuration'], v['id']) for v in definitions},
            'Duplicate or missing acceptance identity')
    require(parent['passed_requirements'] == sum(v['state'] == 'passed' for v in rows)
            and parent['pending_requirements'] == sum(v['state'] == 'pending' for v in rows),
            'Parent acceptance counts differ')
    by_key = {(v['configuration'], v['id']): v for v in definitions}
    for row in rows:
        if row['id'] not in LEAVES | {GATE}:
            continue
        definition = by_key[(row['configuration'], row['id'])]
        require(all(row[k] == v for k, v in definition.items()),
                'Input/media acceptance definition changed')
        if row['id'] in LEAVES:
            require(row['state'] == 'passed' and row.get('admitted_execution_evidence'),
                    'Required input/media leaf is not admitted: ' + row['id'])
        else:
            require(row['state'] == 'pending', 'Input/media gate is already passed')
    return rows


class Join:
    def __init__(self, reader):
        self.reader = reader
        self.records = {}
        self.phases = {}
        self.cases = {}

    def checked(self, ref):
        require(isinstance(ref, dict) and {'record', 'sha256'} <= ref.keys()
                and ref['record'].startswith('research/runtime/') and '..' not in Path(ref['record']).parts
                and self.reader.digest(ref['record']) == ref['sha256'], 'Changed retained evidence identity')
        q = self.reader.json(ref['record'])
        require(q['success'] is True, 'Unsuccessful retained evidence')
        self.records[ref['record']] = dict(record=ref['record'], sha256=ref['sha256'])
        if q.get('sources') and ref['record'] not in self.phases:
            self.phases[ref['record']] = dict(recorded_sources=q['sources'],
                current_drift={n: dict(recorded=h, current=self.reader.digest(n) if (ROOT/n).is_file() else None)
                    for n, h in q['sources'].items() if not (ROOT/n).is_file() or self.reader.digest(n) != h},
                current_native_execution_claimed=False)
        return q

    def ref(self, path):
        return dict(record=path, sha256=self.reader.digest(path))

    def acquisition(self, ref, profile):
        a = self.checked(ref)
        cfg = a['configuration']
        require(type(cfg['software_flag']) is bool and cfg['software_flag'] is (profile == PROFILES[1])
                and cfg['skip_intros'] is False and cfg['terrain_lighting'] == 'native'
                and ('--software-rendering' in a['command']) is cfg['software_flag'],
                'Actual input/media profile differs')
        require(a['sources_stable'] is True and a['private_wine_stopped'] is True
                and a['original_manifest_verified_before_after'] is True
                and a['exit_code'] == 0 and a['retirement']['state'] == 'completed'
                and not a.get('forced_private_prefix_stop', False)
                and a['original_pixels_used_as_native_inputs'] is False,
                'Unretired or diagnostic primary execution')
        require(a['prefix_pool']['returned'] is True, 'Private prefix has not returned')
        retired = a['retirement']['graphics_retirement']
        require(retired['process_exit'] is True and retired['resources_released'] is True
                and not retired['surviving_owners'] and all(v == 0 for v in retired['counts'].values())
                and a['resident']['normal_quit_retired'] is True and a['resident']['pending'] is False
                and int(a['resident']['imports']) == int(a['resident']['completed']) == int(a['resident']['verified_returns']),
                'Primary input/media owners or World work survived normal retirement')
        renderer = a['resident']['renderer']
        require(renderer and (not cfg['software_flag'] or renderer.lower().startswith(('llvmpipe', 'softpipe'))),
                'Actual software provider missing')
        identity = a.get('session_identity', a['session'])
        require(identity not in self.cases or self.cases[identity] == (ref, profile),
                'Mixed acquisition/session identity')
        self.cases[identity] = (ref, profile)
        return a

    def pair(self, pair, profile):
        require(set(pair) == {'acquisition', 'comparison'}, 'Malformed execution pair')
        a = self.acquisition(pair['acquisition'], profile)
        c = self.checked(pair['comparison'])
        require(c['acquisition'] == pair['acquisition'], 'Comparison belongs to another acquisition')
        pixel_proof(c)
        return a, c

    def attachments(self, row):
        selected = []
        for value in row['admitted_execution_evidence']:
            if 'matrix' in value:
                m = self.checked(value['matrix'])
                require(m['mismatching_pixels'] == 0, 'Unaccepted input matrix')
                for ref in m['comparisons']:
                    c = self.checked(ref)
                    a = self.checked(c['acquisition'])
                    if a['configuration']['software_flag'] is (row['configuration'] == PROFILES[1]) and a['configuration']['actual_size'] in value['actual_world_extents']:
                        selected.append(dict(acquisition=c['acquisition'], comparison=ref))
            else:
                value = value.get('execution', value)
                if 'acquisition' in value:
                    selected.append({k: value[k] for k in ('acquisition', 'comparison')})
                else:
                    require({'record', 'sha256'} <= value.keys(), 'Unknown input/media attachment')
                    selected.append({k: value[k] for k in ('record', 'sha256')})
        require(selected and len({fingerprint(v) for v in selected}) == len(selected),
                'Empty or duplicate input/media attachment domain')
        return selected

    def pause(self, a, c):
        rows, frames = read_samples(artifact(self.reader, a, 'pause-samples.sqlite3'),
                                    artifact(self.reader, a, 'frame-input.bin'))
        require(str(frames['nonce']) == str(a['resident']['nonce']), 'Pause belongs to another World nonce')
        result = []
        for n in (1, 2):
            picked = [v for v in rows if v['phase'] == 'mini-dwell-' + str(n)]
            proof = pause_ownership(picked)
            old = c['proof']['dwells'][n-1]
            require(all(old[k] == v for k, v in proof.items()) and old['resume_ack'] == 5+n
                    and old['compared_resumed_paints'] > 0, 'Pause raw/media/input proof differs')
            result.append(dict(proof, number=n, resume_ack=old['resume_ack'],
                compared_resumed_paints=old['compared_resumed_paints'],
                selected_music_operations=sorted({v['music']['operation'] for v in picked}),
                music_requests=sorted({v['music']['request'] for v in picked})))
        return result


def gate_executions(proof, profile):
    links = [v for v in proof['linked_requirements'] if v['configuration'] == profile]
    require(len(links) == 18 and {v['requirement'] for v in links} == LEAVES,
            'Incomplete gate primary-execution links')
    selected = {}
    def add(ref, role):
        key = ref['record']
        value = selected.setdefault(key, dict(execution=ref, roles=[]))
        require(value['execution'] == ref, 'Conflicting gate execution identity')
        if role not in value['roles']:value['roles'].append(role)
    for link in links:
        for ref in link['executions']:add(ref.get('acquisition', ref), link['requirement'])
    full = [v for v in proof['fullscreen'] if v['configuration'] == profile]
    require(len(full) == 2 and {tuple(s) for v in full for s in v['extents']} == {(640,480),(800,600)},
            'Gate execution cohort omitted actual fullscreen domains')
    for v in full:add(v['pair']['acquisition'], 'actual-fullscreen-and-input-ack')
    return [selected[k] for k in sorted(selected)]


def close_rows(parent, proof_reference, proof):
    rows = copy.deepcopy(parent['requirements'])
    for row in rows:
        if row['id'] == GATE:
            require(row['state'] == 'pending', 'Gate already admitted')
            row.update(state='passed', admitted_execution_evidence=[dict(
                value, gate_proof=proof_reference, configuration=row['configuration'],
                reason='Exact 18-leaf input/media cohort, independently compared F9 and original input ACKs, raw pause/media/music ownership and enabled EOF/skip/focused Main return. Historical execution source phases are preserved.')
                for value in gate_executions(proof, row['configuration'])])
    require(sum(v['state'] == 'passed' for v in rows) == parent['passed_requirements']+2,
            'Admission widened beyond the two input/media cells')
    require([v for v in rows if v['id'] != GATE] == [v for v in parent['requirements'] if v['id'] != GATE],
            'Unrelated requirement changed')
    return rows


def analyze(reader, parent):
    """Recheck raw critical joins and retain the accepted independent pixel phases."""
    rows = basis(reader.json('docs/native-rendering-closure.json'), parent)
    join = Join(reader)
    catalog = {(v['configuration'], v['id']): v for v in rows}
    linked = []
    pairs = {}
    single = {}
    for profile in PROFILES:
        for leaf in sorted(LEAVES):
            attachments = join.attachments(catalog[profile, leaf])
            for ref in attachments:
                if 'acquisition' in ref:
                    a, c = join.pair(ref, profile)
                    pairs.setdefault((profile, leaf), []).append((a, c, ref))
                    semantic = c.get('proof', c.get('ui_proof', c.get('original_menu_actions')))
                    require(isinstance(semantic, dict) and semantic.get('verified') is True,
                            'Unverified retained input presentation')
                else:
                    a = join.acquisition(ref, profile)
                    single.setdefault((profile, leaf), []).append((a, ref))
            linked.append(dict(configuration=profile, requirement=leaf, executions=attachments))
    require(len(linked) == 36, 'Incomplete input/media leaf join')

    # These are independent expected movie bytes, never native output inputs.
    movie = module('close-rendering-movie-return')
    intro = module('close-rendering-enabled-intros')
    skip = module('close-rendering-movie-skip')
    menus = module('test-menu-observer')
    expected, expected_ref = movie.references(reader)
    accepted_intros = join.checked(join.ref(INTRO))
    completed = []
    for old in accepted_intros['cases']:
        profile = PROFILES[int(old['configuration']['software_flag'])]
        a = join.acquisition(old['case'], profile)
        require(old['case'] in [ref for _, ref in single[profile, 'journey.enabled-movies.both-intros']]
                and old['identity'] == a['session'] and old['configuration'] == a['configuration']
                and old['sources'] == a['sources'], 'Movie pixel proof belongs to another primary')
        out = old['output']
        require(old['success'] is True and out['verified'] is True and out['mismatching_pixels'] == 0,
                'Complete intro output is not independently proved')
        positive(out['movie_swaps'], 'No actual intro swaps')
        positive(out['compared_pixels'], 'No independently compared intro viewport')
        media = movie.completed_media(a, *movie.media_inputs(reader, a), expected)
        main = intro.main_return(reader, a)
        require(media == old['media'] and main == old['main'], 'Retained EOF/Main proof differs from raw bytes')
        completed.append(dict(configuration=profile, high_resolution=a['configuration']['high_resolution'],
            acquisition=old['case'], output=out, media=media, main=main))
    require(len(completed) == 4 and {(v['configuration'], v['high_resolution']) for v in completed} ==
            {(p, h) for p in PROFILES for h in (False, True)}, 'Missing natural EOF profile/size context')

    skipped, returned, f9, paused, picking = [], [], [], [], []
    for profile in PROFILES:
        for a, ref in single[profile, 'journey.enabled-movies.movie-skip']:
            proof = skip.validate_artifacts(reader, copy.deepcopy(a), expected)
            skipped.append(dict(configuration=profile, high_resolution=a['configuration']['high_resolution'],
                                acquisition=ref, proof=proof))
        for a, ref in single[profile, 'journey.enabled-movies.return-to-menu']:
            from rendering_menu_acceptance import validate_artifacts
            calls = [[v['menu_id'], v['argument']] for v in menus.decode(artifact(reader, a, 'events.bin')) if v['event'] == 3]
            menu = validate_artifacts(reader, a, calls)
            media = movie.completed_media(a, *movie.media_inputs(reader, a), expected)
            returned.append(dict(configuration=profile, high_resolution=a['configuration']['high_resolution'],
                                 acquisition=ref, proof=movie.return_state(a, menu, media)))
        # F9 is an additional required gate case, not inferred from the edge row.
        quick = catalog[profile, 'journey.quick-battle-entry.setup-confirm']
        require(quick['state'] == 'passed', 'Fullscreen supplier is not an admitted normal execution')
        for pair in join.attachments(quick):
            a, c = join.pair(pair, profile)
            require(a['scenario'] == 'normal-quick', 'F9 supplier is not the actual Quick player')
            f9.append(dict(configuration=profile, extents=c['world_domain']['extents'],
                           pair=pair, proof=fullscreen(a, c)))
        require({tuple(size) for v in f9 if v['configuration'] == profile for size in v['extents']} ==
                {(640, 480), (800, 600)}, 'Fullscreen lacks both actual frame sizes')
        for a, c, pair in pairs[profile, 'journey.pause-preferences.pause']:
            dwells = join.pause(a, c)
            paused.append(dict(configuration=profile, size=a['configuration']['actual_size'],
                               pair=pair, dwells=dwells))
        for a, c, pair in pairs[profile, 'journey.cursor-interaction.picking']:
            proof = c['proof']
            require(proof['verified'] is True and proof['normal_quit'] is True
                    and proof['software'] is (profile == PROFILES[1])
                    and proof['windows'] == 48 and proof['object_windows'] == proof['cell_windows'] == 24
                    and proof['displayed_source_witnesses'] == 144 and proof['live_display_rebuilds'] == 2
                    and proof['same_world_continuity'] is False, 'Incomplete live picking/rebuild domain')
            require(a['configuration']['actual_size_schedule'] == [[800, 600], [640, 480], [800, 600]],
                    'Picking actual size schedule differs')
            picking.append(dict(configuration=profile, pair=pair, proof=proof))
    for label, values in [('skip', skipped), ('return', returned)]:
        require(len(values) == 4 and {(v['configuration'], v['high_resolution']) for v in values} ==
                {(p, h) for p in PROFILES for h in (False, True)}, 'Missing ' + label + ' context')
    require(len(f9) == len(paused) == 4 and
            {(v['configuration'], tuple(v['size'])) for v in paused} ==
            {(p, s) for p in PROFILES for s in ((640, 480), (800, 600))}
            and len(picking) == 2, 'Incomplete input/media gate domains')
    return dict(schema=1, kind='retained_input_media_gate_proof', success=True,
        linked_requirements=linked, completed_intros=completed, skipped_intros=skipped,
        natural_menu_returns=returned, fullscreen=f9, pause_media_music=paused, picking=picking,
        independent_movie_reference=expected_ref, retained_records=sorted(join.records.values(), key=lambda v:v['record']),
        execution_source_phases=join.phases, primary_executions=len(join.cases),
        original_game_runs=0, new_pixel_comparisons=0, new_native_code=False,
        historical_failures_retired=False, current_whole_player_proved=False,
        physical_audio_or_driver_equivalence_claimed=False,
        scope='Exactly the default/software input-media gates over their admitted finite action/owner/presentation cohorts. Raw enabled EOF/skip, focused Main, F9/input ACK and Mini input/media/music ownership are rechecked; accepted independent complete pixel proofs retain their original full source phases. Original simulation, picking/ray and layout suppliers, native UI/AA policy, historical failures and all other gates/decisions remain separate. No current combined-native execution or physical sound/driver equivalence.')
