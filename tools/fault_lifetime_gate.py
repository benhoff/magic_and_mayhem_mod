"""Bounded retained-wire admission for the ordinary player's fault policy."""
import copy
import hashlib
import json
from pathlib import Path
import sqlite3
import struct

from rendering_acceptance import requirements, validate
from rendering_pilot import require, counter
from rendering_transition import TERMINAL_KEYS, fault_receipt
from rendering_pause import input_state, validate_dwell
from rendering_pause_assessment import read_samples
from retained_artifacts import read_artifact, recorded_repository

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/fault-lifetime-gate-results-20261109/'
BID = 'NP.fault-lifetime-gate-closeout'
SID = 'native.fault-lifetime-gate-closeout'
GATE = 'gate.fault-lifetime'
PROFILES = ('player-default', 'player-software')
LEAVES = frozenset('operation.'+v for v in ('transition-recovery', 'failure-cleanup', 'surface-loss-restore'))
LOSS = 'OpenGL offscreen target is unavailable'


def basis(plan, parent):
    validate(plan)
    definitions = {(r['configuration'], r['id']): r for r in requirements(plan)}
    rows = parent['requirements']
    require(parent['success'] is True and len(rows) == len(definitions) == 156
            and {(r['configuration'], r['id']) for r in rows} == set(definitions), 'Acceptance inventory differs')
    require(parent['passed_requirements'] == sum(r['state'] == 'passed' for r in rows)
            and parent['pending_requirements'] == sum(r['state'] == 'pending' for r in rows), 'Parent counts differ')
    for row in rows:
        if row['id'] in LEAVES | {GATE}:
            require(all(row[k] == v for k, v in definitions[row['configuration'], row['id']].items()),
                    'Fault acceptance definition changed')
            require(row['state'] == ('pending' if row['id'] == GATE else 'passed'), 'Fault prerequisite/gate state differs')
            if row['id'] in LEAVES: require(row['admitted_execution_evidence'], 'Missing fault execution receipts')
    require(parent['decisions'] == {'supported-player': 'pending', 'original-recovery': 'pending'}, 'Final decisions changed')
    return rows


def pixels(comparison):
    """Verify every row of the retained independent comparison, without replay."""
    require(comparison['success'] is True and comparison['sources_stable'] is True
            and comparison['comparison_processes_stopped'] is True
            and comparison['original_manifest_verified_before_after'] is True
            and comparison['original_pixels_used_as_native_inputs'] is False, 'Incomplete independent comparison')
    epochs = comparison['epochs']
    require(len(epochs) == 2 and [e['failed'] for e in epochs] == [True, False], 'Wrong compared epoch classes')
    count = worlds = total = 0
    for i, epoch in enumerate(epochs):
        p, ref = epoch['comparison'], epoch['original_reference']
        rows = p['comparisons']
        require(ref['native_player_pixels_read'] is False and ref['original_destination_capture_read'] is False,
                'Dependent original oracle')
        require(p['success'] is True and p['mismatching_pixels'] == 0 and rows
                and p['pixels_compared'] == sum(v['pixels'] for v in rows)
                and all(v['pixels'] > 0 and v['mismatching_pixels'] == 0 and v['actual_sha256'] == v['expected_sha256'] for v in rows)
                and p['world_presentations'] == sum(int(v['world']) > 0 for v in rows)
                and len(rows) == epoch['observations']['swaps'], 'Incomplete compared swap cohort')
        require(epoch['observations']['world_swaps'] == p['world_presentations'] > 0, 'World presentations differ')
        if i == 0:
            require(p['synthetic_end_added'] is False and p['normal_completion_claimed'] is False,
                    'Failed prefix received normal completion')
        else: require(p['remaining_surfaces'] == 0, 'Fresh normal END retained surfaces')
        count += len(rows); worlds += p['world_presentations']; total += p['pixels_compared']
    p = comparison['comparison']; part = comparison['partition']
    require(p['success'] is True and p['mismatching_pixels'] == 0 and p['pixels_compared'] == total
            and p['world_presentations'] == worlds and part['actual_native_swaps'] == count
            and part['distinct_producers'] is True and part['all_swaps_uniquely_bound'] is True,
            'Duplicate, omitted or unbound swap partition')
    return dict(pixels=total, swaps=count, world_presentations=worlds)


class Join:
    def __init__(self, reader):
        self.reader, self.records, self.phases = reader, {}, {}

    def checked(self, ref):
        require(isinstance(ref, dict) and {'record', 'sha256'} <= ref.keys()
                and ref['record'].startswith('research/runtime/') and '..' not in Path(ref['record']).parts
                and self.reader.digest(ref['record']) == ref['sha256'], 'Retained evidence identity differs')
        value = self.reader.json(ref['record'])
        require(value['success'] is True, 'Unsuccessful retained execution')
        self.records[ref['record']] = dict(record=ref['record'], sha256=ref['sha256'])
        self.phases[ref['record']] = dict(recorded_sources=value.get('sources', {}),
            current_drift={n: dict(recorded=h, current=self.reader.digest(n) if (ROOT/n).is_file() else None)
                for n, h in value.get('sources', {}).items() if not (ROOT/n).is_file() or self.reader.digest(n) != h},
            compiler_sources=value.get('compiler_sources', {}), player_binary=value.get('player_binary'),
            current_native_execution_claimed=False)
        return value

    def raw(self, a, path):
        limit = 32 if Path(path).name == 'epoch-0-samples.sqlite3' else 16
        return read_artifact(self.reader, a, path, max_bytes=limit*1024*1024, label='fault gate')

    def at(self, a, root, name):
        return str((Path(root)/name).relative_to(recorded_repository(a)))

    def js(self, a, root, name):
        return json.loads(self.raw(a, self.at(a, root, name)))

    def case(self, pair, profile, leaf):
        a, c = self.checked(pair['acquisition']), self.checked(pair['comparison'])
        require(c['acquisition'] == pair['acquisition'], 'Comparison belongs to another acquisition')
        cfg = a['configuration']; loss = leaf == 'operation.surface-loss-restore'
        require(type(cfg['software_flag']) is bool and cfg['software_flag'] is (profile == PROFILES[1])
                and ('--software-rendering' in a['command']) is cfg['software_flag']
                and cfg['actual_size'] == ([800, 600] if cfg['high_resolution'] else [640, 480])
                and cfg['skip_intros'] is False and cfg['terrain_lighting'] == 'native'
                and cfg['compiled_test_driver_invoked'] is False
                and cfg['declared_fault'] == ('quiescent-native-offscreen-target-loss' if loss else 'drained-Mini-producer-invalid'),
                'Wrong fault profile, extent or policy')
        require(a['kind'] == ('public_native_surface_loss_acquisition' if loss else 'public_native_transition_recovery_acquisition')
                and a['sources_stable'] is True and a['player_binary_stable'] is True and a['original_game_runs'] == 2
                and a['original_manifest_verified_before_after'] is True and a['original_pixels_used_as_native_inputs'] is False
                and a['private_wine_stopped'] is True and a['prefix_pool']['returned'] is True
                and a['exit_code'] == 0 and not a.get('forced_private_prefix_stop', False), 'Incomplete two-generation execution')
        if cfg['software_flag']: require(a['resident']['renderer'].lower().startswith(('llvmpipe', 'softpipe')), 'Wrong software provider')
        p = a['proof']; epochs = p['epochs']; proof = c['proof']
        require(len(epochs) == 2 and epochs[0]['root'] != epochs[1]['root'] and p['controlled_faults'] == 1
                and p['physical_restart'] is True and p['no_compiled_test_driver'] is True
                and p['failed_epoch_normal_credit'] is False and proof['verified'] is True
                and proof['old_rendering_accepted'] is False and proof['failed_epoch_normal_completion_claimed'] is False,
                'Wrong failed/fresh generation classes')
        pixel = pixels(c); headers = []; runners = []; nonces = []; bindings = []; sampled = []
        for i, e in enumerate(epochs):
            require(c['epochs'][i]['root'] == e['root'], 'Comparison epoch belongs to another root')
            ring = self.raw(a, self.at(a, e['root'], 'render-commands.bin'))
            require(len(ring) == 1048640, 'Stopped producer ring extent differs')
            h = struct.unpack_from('<16I', ring)
            require(struct.pack('<2I', *h[:2]) == b'MNMRDC03' and h[2] == 3 and h[3] == 1048640
                    and h[4] > 0 and h[8] == 1 and not any(h[10:]), 'Malformed stopped producer')
            if i: require(h[6:8] == (2, 0) and h[5] == h[9], 'Fresh producer did not reach drained END')
            elif loss: require(h[6:8] in ((1, 0), (3, 3), (3, 4)) and ((h[5]-h[9]) & 0xffffffff) <= h[3]-64,
                               'Unexpected surface-loss producer failure')
            else: require(h[6:8] == (3, 5), 'Expected INVALID failure missing')
            admission = self.js(a, e['root'], 'native-output-admission.json')
            require(admission == e['admission'], 'Original runner admission changed')
            require(self.js(a, e['root'], 'resident-world-report.json') == e['resident'], 'Resident World report changed')
            samples, frames = read_samples(self.raw(a, 'epoch-'+str(i)+'-samples.sqlite3'),
                                           self.raw(a, self.at(a, e['root'], 'frame-input.bin')))
            require(frames['nonce'] == e['resident']['nonce']
                    and all(v['input']['size'] == cfg['actual_size'] for v in samples), 'Raw original nonce or extent differs')
            sampled.append((samples, frames))
            runners.append(admission['runner_identity']); nonces.append(e['resident']['nonce']); headers.append(h)
            candidates = [json.loads(self.raw(a, v['path'])) for v in a['artifacts'] if v['path'].endswith('.bin.producer.json')]
            match = [b for b in candidates if b['channel'] == str(Path(e['root'])/'render-commands.bin')]
            require(len(match) == 1 and match[0]['producer'] == h[4] and match[0]['transport_version'] == 3,
                    'Wrong command producer/channel binding')
            bindings.append(match[0])
            domain = c['epochs'][i]['world_domain']; resident = e['resident']
            require(domain['extents'] == [cfg['actual_size']] and domain['worlds']
                    and domain['worlds'] == [dict(generation=g, width=cfg['actual_size'][0], height=cfg['actual_size'][1])
                        for g in range(1, len(domain['worlds'])+1)]
                    and 0 < len(domain['worlds']) <= int(resident['completed'])
                    and c['epochs'][i]['original_reference']['worlds'] == int(resident['completed'])
                    and (i == 0 or len(domain['worlds']) == int(resident['completed'])), 'Source generations were replayed, omitted or mixed')
        require(headers[0][4] != headers[1][4] and bindings[0]['channel'] != bindings[1]['channel']
                and len({(v['pid'], v['start_ticks']) for v in runners}) == 2 and len(set(nonces)) == 2,
                'Fresh generation reused producer, channel, process or World identity')
        old, fresh = epochs; terminal = self.js(a, old['root'], 'native-terminal-state.json')
        parked = [v for v in sampled[0][0] if v['phase'] == 'mini-dwell-1']
        validate_dwell(parked)
        boundary = next(v for v in sampled[0][0] if v['phase'] == 'mini-fault-boundary')
        require(boundary['menu'] == parked[-1]['menu'] and boundary['menu']['screen'] == 17
                and boundary['host_ns'] > parked[-1]['host_ns']
                and boundary['original_world_return'] == parked[-1]['original_world_return']
                and not boundary['input']['active'] and not boundary['input']['held']
                and boundary['world']['pending_world_jobs'] == 0, 'Fault was not at the parked Mini boundary')
        require(terminal == old['terminal'] and terminal['session'] == old['root'] and terminal['schema'] == 1
                and set(terminal['state']) == TERMINAL_KEYS and all(v is True for v in terminal['state'].values()),
                'Incomplete exact terminal retirement')
        fault = json.loads(self.raw(a, 'fault-injection.json'))
        require(fault == old['fault'], 'Fault controller handoff changed')
        if loss:
            from rendering_surface_loss import loss_receipt
            fault_summary = loss_receipt(fault, headers[0][4],
                self.raw(a, self.at(a, old['root'], 'surface-loss-request-v1.json')),
                self.js(a, old['root'], 'surface-loss-observation-v1.json'),
                self.js(a, old['root'], 'surface-loss-retirement-v1.json'))
            require(terminal['error'] == LOSS and counter(terminal['host_us'])*1000 >= counter(fault_summary['retirement_host_ns'])
                    and fault_summary['provider_owner'] == 'gui-resident', 'Wrong native target-loss error/retirement')
        else:
            fault_summary = fault_receipt(fault, headers[0][4])
            require(terminal['error'] == 'Resident World session requires a fresh producer after failure: Native command producer refused session '
                    +str(headers[0][4])+': INVALID (reason 5)', 'Wrong expected producer error')
        require(counter(terminal['host_us'])*1000 >= counter(fault_summary['begin_host_ns']) > boundary['host_ns'],
                'Fault/terminal predates parked boundary')
        with sqlite3.connect(':memory:') as db:
            db.deserialize(self.raw(a, 'terminal-samples.sqlite3'))
            require(db.execute('PRAGMA integrity_check').fetchone() == ('ok',), 'Damaged terminal samples')
            samples = list(db.execute('SELECT * FROM terminal ORDER BY ordinal'))
        require(len(samples) >= 8 and samples[-1][1]-samples[0][1] >= 600_000_000, 'Unbounded terminal stability')
        for i, (ordinal, at, inp, native, paint) in enumerate(samples):
            state, keys = json.loads(native), input_state(inp)
            require(ordinal == i+1 and at >= counter(terminal['host_us'])*1000 and (i == 0 or samples[i-1][1] < at)
                    and state['session'] == old['root'] and state['state'] == 'failed'
                    and not keys['active'] and not keys['held'] and paint == samples[0][4]
                    and json.loads(paint) == old['terminal_samples'][i]['last_world_paint'], 'Terminal World or input advanced')
        stopped = self.js(a, old['root'], 'native-only-session.json')
        require(stopped['state'] == 'restarted' and stopped['normal_quit'] is False
                and all(stopped[k] is True for k in ('restart_requested', 'force_stop_verified', 'renderer_retired'))
                and old['outcome']['rendering_accepted'] is False, 'Failed owner gained normal Quit or survived')
        restart = [v for v in a['actions'] if v['action'] == 'physical-hard-restart']
        require(len(restart) == 1 and restart[0]['host_ns'] > samples[-1][1]
                and restart[0]['host_focus']['host_focused'] is True
                and restart[0]['symbols'] == [0xffe3, 0xffe1, ord('r')]
                and counter(bindings[1]['host_us'])*1000 >= restart[0]['host_ns'], 'Missing physical public fresh restart')
        for phase, pressed in (('before', False), ('pressed', True), ('released', False)):
            bits = bytes.fromhex(restart[0]['keymaps'][phase])
            require(len(bits) == 32 and all(bool(bits[k//8] & (1 << (k%8))) is pressed for k in restart[0]['keycodes']),
                    'Public restart chord was not pressed/released')
        rows, frame = sampled[1]
        before = next(v for v in rows if v['phase'] == 'fresh-before-gameplay-control')
        after = next(v for v in rows if v['phase'] == 'fresh-after-gameplay-control')
        require(frame['nonce'] == nonces[1] and before['host_ns'] < after['host_ns']
                and before['input']['active'] == after['input']['active'] == 1
                and not before['input']['held'] and not after['input']['held']
                and before['input']['keys'][0x26] != after['input']['keys'][0x26]
                and before['original_world_return'] < after['original_world_return']
                and before['original_generation'] < after['original_generation'], 'Fresh work/input did not progress')
        import importlib.util
        spec = importlib.util.spec_from_file_location('fault_cleanup', ROOT/'tools/join-rendering-normal-acceptance.py')
        cleanup_module = importlib.util.module_from_spec(spec); spec.loader.exec_module(cleanup_module)
        root = str(Path(fresh['root']).relative_to(recorded_repository(a)))+'/'
        scoped = dict(a, session=fresh['root'], resident=fresh['resident'], artifacts=[v for v in a['artifacts'] if v['path'].startswith(root)])
        cleanup = cleanup_module.cleanup_proof(self.reader, scoped)
        require(cleanup == proof['cleanup'] and cleanup['verified'] is True and cleanup['normal_quit'] is True,
                'Fresh normal cleanup differs from raw owners')
        return dict(identity=a['session_identity'], configuration=profile, size=cfg['actual_size'],
            fault='native-target-loss' if loss else 'producer-invalid', pair=pair, pixels=pixel, terminal_samples=len(samples),
            terminal_stability_ns=samples[-1][1]-samples[0][1], failed_error=terminal['error'], fault_receipt=fault_summary,
            fresh_cleanup=cleanup, runner_identities=runners, world_nonces=nonces,
            producer_identities=[h[4] for h in headers], channels=[v['channel'] for v in bindings],
            failed_normal_completion=False, physical_hardware_reset=False, current_native_execution=False)


def analyze(reader, parent):
    rows = basis(reader.json('docs/native-rendering-closure.json'), parent)
    join = Join(reader); cases = {}; links = []
    for row in rows:
        if row['id'] not in LEAVES: continue
        attachments = row['admitted_execution_evidence']; domains = set()
        require(len(attachments) == 2, 'Fault leaf must cover both actual extents')
        refs = []
        for attachment in attachments:
            pair = {k: attachment[k] for k in ('acquisition', 'comparison')}; key = pair['acquisition']['record']
            if key not in cases: cases[key] = join.case(pair, row['configuration'], row['id'])
            case = cases[key]
            require(case['pair'] == pair and case['configuration'] == row['configuration']
                    and case['fault'] == ('native-target-loss' if row['id'] == 'operation.surface-loss-restore' else 'producer-invalid')
                    and attachment['actual_world_extents'] == [case['size']], 'Mixed fault leaf primary execution')
            domains.add(tuple(case['size'])); refs.append(pair)
        require(domains == {(640,480), (800,600)}, 'Missing actual fault size')
        links.append(dict(configuration=row['configuration'], requirement=row['id'], executions=refs))
    values = list(cases.values())
    require(len(values) == 8 and len({v['identity'] for v in values}) == 8
            and len({(v['configuration'], v['fault'], tuple(v['size'])) for v in values}) == 8,
            'Incomplete or duplicate fault/profile/extent cohort')
    require(len({n for v in values for n in v['world_nonces']}) == 16
            and len({(r['pid'], r['start_ticks']) for v in values for r in v['runner_identities']}) == 16,
            'Duplicated original work across cases')
    return dict(schema=1, success=True, kind='retained_fault_lifetime_gate_proof', cases=values, linked_requirements=links,
        retained_records=[join.records[k] for k in sorted(join.records)], retained_execution_source_phases=join.phases,
        original_game_runs=0, new_pixel_comparisons=0, policy='terminal-failure/public-fresh-generation',
        physical_hardware_reset=False, current_whole_player_proved=False,
        scope='Exactly eight retained expected-negative/fresh-generation pairs for default/software at actual640/800. Raw failure, terminal input/output stability, complete retirement, distinct identities and fresh positive work/normal Quit are rechecked. Independent presented pixel proofs stay in their original execution phases. No physical reset, transparent old-generation or isolated continuation, arbitrary fault/provider, current combined-player or final-decision claim.')


def close_rows(parent, proof_ref, proof):
    rows = copy.deepcopy(parent['requirements'])
    for row in rows:
        if row['id'] != GATE: continue
        require(row['state'] == 'pending', 'Fault gate already admitted')
        selected = [v for v in proof['cases'] if v['configuration'] == row['configuration']]
        require(len(selected) == 4 and {(v['fault'], tuple(v['size'])) for v in selected} ==
                {(f, s) for f in ('native-target-loss','producer-invalid') for s in ((640,480),(800,600))}, 'Gate case domain differs')
        row.update(state='passed', admitted_execution_evidence=[dict(execution=v['pair'], gate_proof=proof_ref,
            actual_world_extents=[v['size']], fault=v['fault'], reason='Expected negative, exact terminal retirement and stable input/output; physical public fresh restart with distinct original/World/channel identities and normal fresh Quit. Failed epoch receives no normal credit. Historical pixel/native execution phases remain exact.') for v in selected])
    require(sum(r['state'] == 'passed' for r in rows) == parent['passed_requirements']+2
            and [v for v in rows if v['id'] != GATE] == [v for v in parent['requirements'] if v['id'] != GATE],
            'Fault admission changed unrelated cells')
    return rows
