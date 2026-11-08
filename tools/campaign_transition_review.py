"""Small retained-data proofs for map exit and next-map loading.

Read existing SQLite objects; never hydrate, duplicate or recollect a capture.
Successful analysis does not admit a configuration-wide acceptance row.
"""
import hashlib
import json
import struct

from rendering_capture import capture
from rendering_pilot import commands, require, sha
from rendering_progression import validate_callbacks, validate_flow

BID = 'NP.campaign-transition-retained-review'
SID = 'native.campaign-transition-retained-review'
OUT = 'research/runtime/campaign-transition-lean-results-20261108/'
PARENT = 'research/runtime/rendering-recovery-four-results-20261108/master-admission-01.json'
INPUT = 'research/runtime/rendering-progression-results-20261101/'
CASES = (
    ('target-nominal-victory-full-01', 'target-nominal-victory-full-01-comparison'),
    ('completion-progression-travel-diagnostic-20261102-28',
     'completion-progression-travel-comparison-20261102-28-retry01'),
)
LEAVES = ('journey.map-transitions.leave-map', 'journey.map-transitions.loading')
REPORT_LIMIT = 64 * 1024
COMPRESSED_LIMIT = 128 * 1024**2
DECODED_LIMIT = 384 * 1024**2
SCOPE = ('Retained-data review of natural Celtic region1 exit and region2 loading, '
         'original report/World-pop/next-World callback ordering, fresh bridge ACKs, '
         'native World surface incarnation retirement/recreation and referenced independent '
         'presented-output comparisons. Nominal software640 and separate diagnostic default800 '
         'source phases only. No new game, pixel/oracle execution, current native runtime '
         'equivalence, complete external-object lifetime or acceptance-row promotion.')


def read_reference(reader, ref, budget=32 * 1024**2):
    raw = reader.read_bytes(ref['record'])
    require(len(raw) <= budget and sha(raw) == ref['sha256'], 'Changed or oversized retained report')
    return json.loads(raw)


def bound_artifact(reader, acquisition, suffix, limit):
    matches = [v for v in acquisition['artifacts'] if v['path'].endswith('/' + suffix)]
    require(len(matches) == 1, 'Missing or ambiguous transition input: ' + suffix)
    ref = matches[0]
    require(type(ref['bytes']) is int and 0 < ref['bytes'] <= limit,
            'Transition input exceeds its read budget')
    body = reader.read_bytes(ref['record'])
    require(len(body) == ref['bytes'] and sha(body) == ref['sha256'], 'Changed retained transition input')
    return body, {k: ref[k] for k in ('record', 'sha256', 'bytes')}


def command_bytes(envelope, session):
    # Preflight the qCompress sizes before the existing strict decoder allocates.
    require(envelope[:8] == b'MNMACE01' and len(envelope) <= COMPRESSED_LIMIT,
            'Command envelope exceeds compressed read budget')
    at, decoded = 8, 0
    while at < len(envelope):
        require(len(envelope) - at >= 8, 'Truncated command envelope')
        m, n = struct.unpack_from('<II', envelope, at)
        at += 8
        require(0 < m <= 65536 and n <= 33 * 1024**2 and m+n <= len(envelope)-at,
                'Invalid command envelope extent')
        at += m
        require(not n or n >= 4, 'Truncated compressed command body')
        decoded += struct.unpack_from('>I', envelope, at)[0] if n else 0
        require(decoded <= DECODED_LIMIT, 'Command envelope exceeds decoded RAM budget')
        at += n
    rows = capture(envelope)
    require(rows[0][0]['session'] == session and rows[0][0]['role'] == 'commands'
            and rows[-2][0]['kind'] == 'ended'
            and all(h['kind'] in ('fragment', 'cpu-read', 'ended') for h, _ in rows[1:-1]),
            'Mixed or incomplete command session')
    raw = b''.join(body for header, body in rows if header['kind'] == 'fragment')
    return raw, decoded


def semantic_handoff(flow, events):
    semantic = validate_flow(flow, flow['case'])
    require(flow['case'] in ('victory', 'progression'), 'Defeat is not next-map progression')
    callbacks = validate_callbacks(events, flow)
    states = flow['states']
    expected = ['main', 'display-configuration', 'configured-main', 'region-entry',
                'natural-report', 'result-return', 'region-travel-entry', 'campaign-travel',
                'region-entry', 'cleanup-mini', 'cleanup-report', 'cleanup-realm', 'cleanup-main']
    require([s['event'] for s in states] == expected, 'Unexpected progression route or reload')
    report, returned, travel_entry, travel, entry = states[4:9]
    require(report['outcome'] == 1 and 'victory' in report['title'].lower(), 'No natural original victory')
    require(all(b['sequence'] > a['sequence'] and b['generation'] > a['generation']
                and b['ack'] == a['ack'] + 1
                for a, b in zip(states[4:8], states[5:9])), 'Stale progression state/ACK/generation')
    require(returned['screen'] == travel['screen'] == 4
            and travel_entry['screen'] == entry['screen'] == 18
            and travel_entry['region'] == entry['region'] == travel['location'] == travel['destination'] == 2
            and travel_entry['realm'] == entry['realm'] == 'Celtic'
            and travel_entry['enter_handoff'] == 4 and entry['enter_handoff'] == 3,
            'Missing actual original campaign travel/loading handoff')
    old, new = flow['worlds']
    require(new['world'] == new['after_world'] + 1 > old['world'], 'Stale or ambiguous first later World')
    rows = list(struct.iter_unpack('<16I', events[16:]))
    first, later = callbacks['original_world_epochs']
    ok = callbacks['original_result_ok_sequence']
    pops = [r for r in rows if ok < r[0] < later['first_sequence'] and r[1] == 28 and r[3] == 2]
    require(len(pops) == 1 and pops[0][2] == report['thread']
            and pops[0][4:6] == (0x6cbb78, 0x5c5dd8)
            and pops[0][6] == 0 and pops[0][11:13] == (0x659408, 3)
            and first['last_sequence'] < ok < pops[0][0] < later['first_sequence'],
            'Missing owner-thread inactive original World pop before later World ticks')
    enters = [r for r in rows if pops[0][0] < r[0] < later['first_sequence']
              and r[1] in (2, 3) and r[3] == 18]
    require([(r[1], r[9]) for r in enters] == [(2, 0), (3, 0)] * 2
            and all(r[2] == report['thread'] and r[4:6] == (0x6578c0, 0x5c667c)
                    and r[11:13] == (0x6578c0, 4) and r[6] == 1 for r in enters),
            'Missing original later-region entry callback pairs')
    return dict(verified=True, semantic=semantic, original_world_pop=list(pops[0]),
                original_entry_callbacks=[list(r) for r in enters],
                original_epoch_order=callbacks['original_world_epochs'],
                report_ok=ok, later_entry=entry, later_world=new,
                scope='Original callback-file order and separate bridge-state seqlock order; '
                      'these sequence domains are not compared with each other. '
                      'Build/session addresses do not establish cross-launch pointer stability.')


def native_handoff(raw, first_new_generation, extent):
    live, retired, imports = {}, {}, []
    end = None
    nonce = None
    for op, sequence, payload in commands(raw):
        if op == 1:
            require(len(payload) >= 28, 'Truncated surface creation')
            sid, width, height, bits, *_ = struct.unpack_from('<7I', payload)
            require(sid not in live, 'Duplicate native surface owner')
            live[sid] = dict(surface=sid, created=sequence, size=[width, height], bits=bits)
        elif op == 7:
            require(len(payload) == 4, 'Malformed surface deletion')
            sid, = struct.unpack('<I', payload)
            require(sid in live, 'Unowned surface deletion')
            record = live.pop(sid)
            retired[(sid, record['created'])] = sequence
        elif op == 19:
            require(len(payload) == 20, 'Malformed World import')
            sid, lo, hi, nlo, nhi = struct.unpack('<5I', payload)
            generation, current_nonce = lo + (hi << 32), nlo + (nhi << 32)
            require(generation == len(imports)+1 and sid in live and current_nonce > 0,
                    'Unowned or discontinuous World import')
            require(nonce is None or nonce == current_nonce, 'Mixed native source generations')
            nonce = current_nonce
            require(live[sid]['size'] == extent and live[sid]['bits'] == 16,
                    'Wrong native World format/extent')
            imports.append(dict(live[sid], generation=generation, imported=sequence))
        elif op == 8:
            require(not live and not payload, 'Native owners survive END')
            end = sequence
    require(end is not None and 1 < first_new_generation <= len(imports), 'Missing later World or command END')
    old, new = imports[first_new_generation-2:first_new_generation]
    for record in imports:
        deletion = retired.get((record['surface'], record['created']))
        require(deletion is not None and record['imported'] < deletion < end,
                'Imported World incarnation survives or retires out of order')
    old_deleted = retired[(old['surface'], old['created'])]
    require((old['surface'], old['created']) != (new['surface'], new['created'])
            and old['imported'] < old_deleted < new['created'] < new['imported'],
            'Old native World storage survives next-map surface creation')
    return dict(verified=True, previous_world=old | dict(deleted=old_deleted),
                first_later_world=new | dict(deleted=retired[(new['surface'], new['created'])]),
                imports=len(imports), source_nonce=nonce, command_end=end,
                surviving_command_surfaces=0,
                scope='Native command-surface incarnations and source generation continuity only; '
                      'all original entity/vptr/allocator lifetimes remain outside this proof.')


def referenced_output(comparison, flow, source_sha):
    require(comparison['success'] is comparison['sources_stable']
            is comparison['asset_inputs_stable'] is True,
            'Incomplete retained independent comparison')
    require(comparison['captured_command_stream_sha256'] == source_sha, 'Independent comparison used another command history')
    reference = comparison['original_reference']
    require(reference['continuous_from_owned_initial_history'] is True
            and reference['native_player_pixels_read'] is reference['original_destination_capture_read'] is False,
            'Dependent or discontinuous original reference')
    output = comparison['comparison']
    require(output['success'] is True and output['mismatching_pixels'] == output['remaining_surfaces'] == 0
            and output['pixels_compared'] == sum(r['pixels'] for r in output['comparisons']) > 0
            and all(r['pixels'] > 0 and r['mismatching_pixels'] == 0
                    and r['actual_sha256'] == r['expected_sha256'] for r in output['comparisons']),
            'Incomplete or mismatching retained output')
    witnesses = []
    for world, transition in zip(flow['worlds'], comparison['transition_worlds'], strict=True):
        require(all(world[k] == transition[k] for k in ('region', 'realm', 'world'))
                and world['paint'] in transition['swaps'], 'Unbound transition World paint')
        matches = [r for r in output['comparisons'] if r['paint'] == world['paint']]
        require(matches and all(r['world'] == world['world'] for r in matches), 'Missing independently compared transition paint')
        witnesses.append(dict(region=world['region'], world=world['world'], paint=world['paint'],
                              occurrences=len(matches), pixels=sum(r['pixels'] for r in matches)))
    return dict(verified=True, transition_witnesses=witnesses,
                historical_compared_pixels=output['pixels_compared'],
                historical_comparison_processes_stopped=comparison.get('comparison_processes_stopped'),
                scope='Hash-verified retained comparison and its exact source-command stream; '
                      'no fresh pixel comparison or complete capture reanalysis.')


def nominal_candidate(acquired, comparison):
    cfg = acquired['configuration']
    return (acquired['kind'] == 'public_native_progression_acquisition'
            and comparison['kind'] == 'public_native_progression_comparison'
            and cfg.get('diagnostic_only') is False and cfg.get('evidence_mode') == 'full'
            and cfg['skip_intros'] is False
            and comparison.get('comparison_processes_stopped') is True
            and acquired.get('compiler_closure_stable') is True
            and acquired.get('player_binary_stable') is True)


def coverage_frontier(cases):
    required = {(p, tuple(s)) for p in ('player-default', 'player-software')
                for s in ([640, 480], [800, 600])}
    eligible = {(c['configuration'], tuple(c['extent'])) for c in cases if c['nominal_candidate']}
    return [dict(id=leaf, state='pending', required_domains=4,
                 nominal_retained_domains=[dict(configuration=p, extent=list(s)) for p, s in sorted(eligible)],
                 missing_nominal_domains=[dict(configuration=p, extent=list(s)) for p, s in sorted(required-eligible)],
                 remaining=['Per-domain current applicability, independent complete output and transition ownership admission.',
                            'Original populated object/vptr/generation acquire/release evidence is not supplied by final zero owners.'])
            for leaf in LEAVES]


def review_case(reader, acquisition_name, comparison_name):
    refs = {k: dict(record=INPUT+n+'.json', sha256=reader.digest(INPUT+n+'.json'))
            for k, n in (('acquisition', acquisition_name), ('comparison', comparison_name))}
    acquired, comparison = [read_reference(reader, refs[k]) for k in ('acquisition', 'comparison')]
    require(comparison['acquisition'] == refs['acquisition'], 'Mixed acquisition/comparison pair')
    require(acquired['success'] is acquired['sources_stable'] is acquired['private_wine_stopped']
            is acquired['original_manifest_verified_before_after'] is acquired['artifact_retention_complete'] is True
            and acquired['exit_code'] == 0 and acquired['original_pixels_used_as_native_inputs'] is False,
            'Incomplete retained progression acquisition')
    events, event_ref = bound_artifact(reader, acquired, 'events.bin', 4*1024**2)
    semantic = semantic_handoff(acquired['flow'], events)
    require(validate_callbacks(events, acquired['flow']) == comparison['original_menu_actions'],
            'Current callback analysis differs from historical comparison')
    command_refs = [v for v in acquired['artifacts'] if '/capture/commands-' in v['path']]
    require(len(command_refs) == 1, 'Missing or ambiguous command capture')
    envelope, command_ref = bound_artifact(reader, acquired, command_refs[0]['path'].split('/')[-1], COMPRESSED_LIMIT)
    raw, decoded = command_bytes(envelope, acquired['session_identity'])
    source_sha = sha(raw)
    extent = [acquired['flow']['worlds'][0]['width'], acquired['flow']['worlds'][0]['height']]
    native = native_handoff(raw, semantic['later_world']['world'], extent)
    output = referenced_output(comparison, acquired['flow'], source_sha)
    require(native['imports'] == comparison['original_reference']['worlds']
            == int(acquired['resident']['imports']) == int(acquired['resident']['verified_returns']),
            'Native imports and returned original histories differ')
    cfg = acquired['configuration']
    nominal = nominal_candidate(acquired, comparison)
    require(cfg['id'] in ('player-default', 'player-software')
            and cfg['software_flag'] is (cfg['id'] == 'player-software'), 'Mixed provider configuration')
    if cfg['software_flag']:
        require(acquired['resident']['renderer'].lower().startswith(('llvmpipe', 'softpipe')), 'Unproved software provider')
    drift = [p for p, h in acquired['sources'].items()
             if not (reader.root/p).is_file() or hashlib.sha256((reader.root/p).read_bytes()).hexdigest() != h]
    return dict(configuration=cfg['id'], extent=extent, identity=acquired['session_identity'],
                nominal_candidate=nominal, acceptance_eligible=False,
                historical_source_phase=True, raw_changed_source_count=len(drift),
                refs=refs, raw_inputs=[event_ref, command_ref], original=semantic, native=native,
                referenced_output=output, compressed_bytes_read=len(events)+len(envelope),
                decoded_command_envelope_bytes=decoded,
                original_lifetimes=comparison['populated_handoff_lifetime_closure'],
                scope='Finite retained boundary proof only; diagnostic cases remain diagnostic and '
                      'nominal candidates do not certify an entire profile or current native sources.')
