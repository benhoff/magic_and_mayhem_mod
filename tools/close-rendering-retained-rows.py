#!/usr/bin/env python3
"""Join retained software creature output and the accepted normal-lifetime cohort."""
import argparse
from collections import Counter
import importlib.util
import json
from pathlib import Path
import struct

import numpy as np

from coverage_claims import fingerprint
from coverage_evidence import Reader, write_report
from evidence_objects import configured
from native_cursor_journal import decode as cursor_decode
from rendering_acceptance import requirements, validate
from rendering_menu_acceptance import artifact
from rendering_pilot import History, acquisition, commands, counter, require
from rendering_capture import capture

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-retained-results-20261107/'
ANIMATION = 'research/runtime/rendering-five-results-20261107/closeout-analysis-03.json'
BEHAVIORS = {'NP.software-creature-closeout', 'NP.normal-lifetime-gate-closeout'}
ROWS = {('player-software', 'journey.creatures-effects.creature-animation'),
        ('player-default', 'gate.normal-lifetime'), ('player-software', 'gate.normal-lifetime')}
KINDS = {'public_native_normal_acquisition', 'native_public_main_setup_menu_acquisition',
         'native_public_movie_skip_acquisition', 'public_results_return_acquisition',
         'public_native_interaction_acquisition'}


def module(name):
    spec = importlib.util.spec_from_file_location('retained_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


aa = module('close-rendering-animation-auxiliary')
world_exit = module('close-rendering-world-exit')
menu_events = module('test-menu-observer')
sha = aa.sha


def integer(value):
    if type(value) is int:
        require(value >= 0, 'Negative counter')
        return value
    return counter(value)


def checked(reader, ref):
    require(isinstance(ref, dict) and {'record', 'sha256'} <= set(ref)
            and reader.digest(ref['record']) == ref['sha256'], 'Changed accepted input reference')
    return reader.json(ref['record'])


def primary_cohort(reader, parent):
    """Expand every passed row's execution or matrix; unknown attachment forms refuse."""
    result = {}

    def add(ref, profile, row_id, compared=None):
        a = checked(reader, ref)
        require(a['kind'] in KINDS, 'Unreviewed primary execution family')
        require(a['configuration']['software_flag'] is (profile == 'player-software'),
                'Primary acquisition does not match its accepted configuration')
        value = result.setdefault(ref['record'], dict(reference=ref, profile=profile, rows=[], comparisons=[]))
        require(value['reference'] == ref and value['profile'] == profile, 'Mixed primary acquisition binding')
        if row_id not in value['rows']:
            value['rows'].append(row_id)
        if compared and compared not in value['comparisons']:
            value['comparisons'].append(compared)

    for row in parent['requirements']:
        if row['state'] != 'passed':
            continue
        require(row['admitted_execution_evidence'], 'A passed row lacks primary evidence')
        for evidence in row['admitted_execution_evidence']:
            profile = row['configuration']
            if 'execution' in evidence:
                ref = evidence['execution']
                if 'acquisition' in ref:
                    add(ref['acquisition'], profile, row['id'], ref['comparison'])
                else:
                    add(ref, profile, row['id'])
            elif 'matrix' in evidence:
                matrix = checked(reader, evidence['matrix'])
                require(matrix['success'] and matrix['mismatching_pixels'] == 0, 'Unaccepted matrix')
                selected = 0
                for compared in matrix['comparisons']:
                    c = checked(reader, compared)
                    a = checked(reader, c['acquisition'])
                    if (a['configuration']['software_flag'] is (profile == 'player-software')
                            and a['configuration']['actual_size'] in evidence['actual_world_extents']):
                        add(c['acquisition'], profile, row['id'], compared)
                        selected += 1
                require(selected == 1, 'Missing/duplicate matrix primary domain')
            else:
                raise ValueError('Unreviewed accepted-row evidence attachment')
    require(result, 'Empty accepted normal cohort')
    return list(result.values())


def terminal_state(a, shell, admission, process, input_raw, ring_raw, control_raw, cursor_raw, graphics_raw, callbacks):
    """No process-exit-only inference: verify the retained terminal owners directly."""
    require(a['success'] and a['exit_code'] == 0 and a['private_wine_stopped']
            and a['original_manifest_verified_before_after'] and a['sources_stable']
            and not a.get('forced_private_prefix_stop', False), 'Incomplete normal execution')
    require(not a['configuration'].get('diagnostic_only', False)
            and 'injected_fault' not in a['configuration'], 'Diagnostic execution cannot supply normal credit')
    require(shell['schema'] == 1 and shell['policy'] == 'native-only-rendering-v1'
            and shell['session'] == admission['session'] == a['session']
            and admission['native_only']['policy'] == shell['policy'], 'Mixed cleanup session/policy')
    require(shell['state'] == 'completed' and shell['normal_quit'] and shell['renderer_retired']
            and shell['fallback'] is False and not shell['error'] and shell['launcher_exit'] == 0
            and shell['restart_requested'] is False and shell['force_stop_verified'] is False,
            'Force-stop/restart/fallback cannot supply normal retirement')
    require(type(process['original_exit_code']) is int and process['original_exit_code'] == 0
            and process['cleanup_after_original_exit'] and process['prefix'] == admission['prefix'],
            'Wrong original process/prefix retirement')
    require(callbacks and callbacks[-1] == [3, 4] and callbacks.count([3, 4]) == 1,
            'Missing unique original Main Quit callback')
    require(len(input_raw) == 1088 and input_raw[:8] == b'MNMINK01', 'Wrong terminal input')
    keys = struct.unpack('<272I', input_raw)
    require(keys[2:4] == (1, 1088) and keys[4] > 0 and keys[4] % 2 == 0 and keys[5] == 0
            and not any(v & 0x80000000 for v in keys[16:]), 'Active/held/unstable terminal input')
    require(len(ring_raw) == 1048640 and ring_raw[:8] == b'MNMRDC03', 'Wrong terminal command channel')
    ring = struct.unpack_from('<16I', ring_raw)
    require(ring[2:4] == (3, 1048640) and ring[4] > 0 and ring[5] == ring[9] > 0
            and ring[6:9] == (2, 0, 1) and not any(ring[10:]), 'Command channel is not drained/ended')
    require(len(control_raw) == 576 and control_raw[:8] == b'MNMRCV01', 'Wrong terminal control channel')
    control = struct.unpack_from('<16I', control_raw)
    require(control[2:5] == (1, 576, ring[4]) and control[5] == control[8] > 0
            and control[6:8] == (3, 0) and control[9:13] == (1, 1, 0, 2)
            and not any(control[13:]) and not any(control_raw[64:]), 'Stop is pending/refused/foreign')
    cursor = cursor_decode(cursor_raw)
    require(cursor['closed'] and cursor['active_clients'] == cursor['pending_jobs'] == 0
            and cursor['queue_accepted'] == cursor['queue_completed'] + cursor['queue_cancelled'],
            'Cursor clients/jobs survive')
    require(len(graphics_raw) == 64, 'Wrong graphics inventory extent')
    graphics = struct.unpack('<16I', graphics_raw)
    require(graphics[:5] == (0x444d4e4d, 0x31544552, 1, 64, 1) and not any(graphics[5:11])
            and graphics[11] > 0 and graphics[12] > 0 and not any(graphics[13:]), 'Graphics owners survive')
    resident, retirement = a['resident'], a['retirement']
    count = integer(resident['imports'])
    require(count == integer(resident['completed']) == integer(resident['verified_returns'])
            and resident['pending'] is False and resident['normal_quit_retired'] is True
            and not resident['error'], 'Source work is pending or unreturned')
    require(retirement['state'] == 'completed' and retirement['forwarded'] == 0
            and retirement['worlds'] == count and retirement['graphics_retirement']['resources_released']
            and retirement['graphics_retirement']['process_exit']
            and not retirement['graphics_retirement']['surviving_owners']
            and all(v == 0 for v in retirement['graphics_retirement']['counts'].values()),
            'Declared retirement disagrees with raw zero-owner inventory')
    states = a.get('states', a.get('flow', {}).get('states', []))
    threads = {v['thread'] for v in states if 'thread' in v and 'event' not in v}
    require(len(threads) == 1 and next(iter(threads)) > 0, 'Ambiguous original owner thread')
    return dict(verified=True,original_thread=next(iter(threads)), source_imports=count,
                command_session=ring[4], published=ring[5], acknowledged=ring[9],
                stop_requested=control[5], stop_acknowledged=control[8],
                command_ended=True, producer_stopped=True, renderer_retired=True,
                normal_quit=True, original_exit=0, held_keys=0, input_active=0,
                graphics_owners=0, pending_jobs=0, cursor=cursor,
                original_callbacks=callbacks, private_wine_stopped=True)


def full_command_history(reader, a, compared):
    streams = [v for v in a['artifacts'] if '/capture/' in v['path']
               and Path(v['path']).name.startswith('commands') and v['path'].endswith('.bin')]
    if not streams:
        require(a['kind'] not in ('public_native_normal_acquisition', 'public_native_interaction_acquisition'),
                'Complete recorded command history missing')
        return None
    require(len(streams) == 1, 'Ambiguous command capture')
    rows = capture(artifact(reader, a, Path(streams[0]['path']).name))
    require(rows[0][0]['role'] == 'commands' and rows[0][0]['session'] == a['session_identity'],
            'Wrong command capture/session')
    raw = b''.join(body for row, body in rows if row['kind'] == 'fragment')
    c = checked(reader, compared)
    require(c['success'] and c['acquisition']['sha256'] and sha(raw) == c['captured_command_stream_sha256'],
            'Different independently compared commands')
    life = world_exit.imported_lifetimes(raw)
    require(rows[-2][0]['kind'] == 'ended' and rows[-2][0]['commands'] == life['command_end'],
            'Capture END differs from actual command END')
    return dict(nonce=life['nonce'], imports=len(life['imports']), command_end=life['command_end'],
                deleted_import_incarnations=len(life['imports']), captured_commands_sha256=sha(raw))


def source_header_boundary(raw, count, nonce):
    """The Results collector has slots but no optional close-caller diagnostic."""
    require(len(raw) == 35869312 and raw[:8] == b'MNMCON06', 'Wrong terminal source channel')
    h = struct.unpack_from('<32I', raw)
    require(h[2:6] == (6, 128, len(raw), 3) and h[6] + (h[7] << 32) == nonce
            and h[8:14] == (1, 1, 1, 0, 0, 0) and not any(h[14:]), 'Wrong source roles/nonce/cancellation')
    step = (len(raw) - 128) // 2
    slots = []
    for index in range(2):
        state, reserved, lo, hi = struct.unpack_from('<4I', raw, 128 + index * step)
        require(reserved == 0, 'Source slot reserved ownership changed')
        slots.append(dict(index=index, state=state, generation=lo + (hi << 32)))
    require(sorted((v['state'], v['generation']) for v in slots) == [(0, count), (1, count + 1)],
            'Native-owned or wrong-generation source slot survives')
    return dict(verified=True, nonce=nonce, cancelled=True, finished=False,
                next_unsubmitted_generation=count + 1, slots=slots, native_owned_slots=0,
                header_only=True, close_caller_verified=False,
                scope='Raw cancelled source slots and returned-generation boundary; this collector did not retain the optional close caller/thread diagnostic. No caller attribution is inferred.')


def retire_case(reader, item):
    a = checked(reader, item['reference'])
    names = ('native-only-session.json', 'native-output-admission.json', 'original-process-result.json',
             'render-input.bin', 'render-commands.bin', 'render-commands.bin.control',
             'native-cursor-lifetime.bin', 'native-draw-retirement.bin')
    raw = {name: artifact(reader, a, name) for name in names}
    values = [json.loads(raw[name]) for name in names[:3]]
    events = menu_events.decode(artifact(reader, a, 'events.bin'))
    callbacks = [[v['menu_id'], v['argument']] for v in events if v['event'] == 3]
    proof = terminal_state(a, *values, *(raw[name] for name in names[3:]), callbacks)
    count = proof['source_imports']
    if count:
        channel = artifact(reader, a, 'world-continuous.bin')
        # The source closure checks the actual aggregate returned-generation
        # boundary. It does not invent per-import surface observations when
        # this collector did not capture the full command stream.
        nonce = integer(a['resident']['nonce'])
        terminals = [v for v in a['artifacts'] if Path(v['path']).name.startswith('world-terminal-')]
        if terminals:
            require([Path(v['path']).name for v in terminals] == ['world-terminal-0001.bin'],
                    'Unexpected source diagnostics in a normal execution')
            proof['source_terminal'] = world_exit.terminal_source(channel,
                artifact(reader, a, 'world-terminal-0001.bin'),
                dict(nonce=nonce, imports=range(count)), proof['original_thread'])
        else:
            require(a['kind'] == 'public_results_return_acquisition', 'Missing required source diagnostic')
            proof['source_terminal'] = source_header_boundary(channel, count, nonce)
        if item['comparisons']:
            require(len(item['comparisons']) == 1, 'Mixed comparison phases')
            require(checked(reader, item['comparisons'][0])['acquisition'] == item['reference'],
                    'Comparison belongs to another primary execution')
            proof['command_lifetimes'] = full_command_history(reader, a, item['comparisons'][0])
            if proof['command_lifetimes']:
                require(proof['command_lifetimes']['nonce'] == nonce
                        and proof['command_lifetimes']['imports'] == count, 'Source/import nonce/count differs')
    else:
        require(a['kind'] in ('native_public_main_setup_menu_acquisition', 'native_public_movie_skip_acquisition'),
                'World execution silently became a zero-World case')
    proof['raw_terminal_sha256'] = {name: sha(body) for name, body in raw.items()}
    result = dict(item, kind=a['kind'], identity=a.get('session_identity', a['session']),
                  native_source_phase='original recorded sources, binaries and execution; no current native claim', proof=proof)
    check_retirement(result)
    return result


def check_retirement(case):
    p = case['proof']
    require(p['verified'] and p['normal_quit'] and p['original_exit'] == 0 and p['private_wine_stopped']
            and p['command_ended'] and p['producer_stopped'] and p['renderer_retired']
            and p['held_keys'] == p['input_active'] == p['graphics_owners'] == p['pending_jobs'] == 0
            and p['published'] == p['acknowledged'] > 0 and p['stop_requested'] == p['stop_acknowledged'] > 0,
            'Incomplete normal retirement')
    c = p['cursor']
    require(c['closed'] and c['active_clients'] == c['pending_jobs'] == 0
            and c['queue_accepted'] == c['queue_completed'] + c['queue_cancelled'], 'Cursor ownership survives')
    require(p['original_callbacks'][-1] == [3, 4] and p['original_callbacks'].count([3, 4]) == 1,
            'Normal Main Quit is missing/repeated')
    if p['source_imports']:
        s = p['source_terminal']
        require(s['verified'] and s['cancelled'] and not s['finished'] and s['native_owned_slots'] == 0
                and s['next_unsubmitted_generation'] == p['source_imports'] + 1, 'Source work survives normal shutdown')
        if s.get('header_only'):
            require(case['kind'] == 'public_results_return_acquisition' and not s['close_caller_verified'],
                    'Header-only boundary is limited to the known Results collector')
        else:
            require(s['original_thread'] == p['original_thread'], 'Wrong diagnostic source thread')
        life = p.get('command_lifetimes')
        require(case['kind'] not in ('public_native_normal_acquisition', 'public_native_interaction_acquisition')
                or life is not None, 'Recorded command lifetimes were omitted')
        if life:
            require(life['imports'] == life['deleted_import_incarnations'] == p['source_imports']
                    and life['nonce'] == s['nonce'], 'Imported incarnation survives')


def software_frames(reader):
    q = reader.json(ANIMATION)
    require(q['success'] and q['analysis_sources_stable'] and q['passed_requirements'] == 67, 'Unaccepted AA07 input')
    selected = [c for c in q['executions'] if c['domain'][0] == 'player-software']
    require(len(selected) == 2 and {tuple(c['domain'][1]) for c in selected} == {(640, 480), (800, 600)},
            'Missing software creature domain')
    result = []
    for case in selected:
        aa.check_output(case['witnesses'])
        a, c = [checked(reader, case['pair'][key]) for key in ('acquisition', 'comparison')]
        with configured(ROOT) as pair:
            for asset in case['assets']:
                require(a['installation_inputs'][asset['path']]['sha256'] == c['asset_inputs'][asset['path']] == asset['sha256']
                        and sha(pair.read(asset['sha256'], asset['bytes'])) == asset['sha256'], 'Changed archived creature input')
        observed = acquisition([artifact(reader, a, Path(v['path']).name) for v in a['artifacts']
                                if '/capture/' in v['path'] and v['path'].endswith('.bin')], a['session_identity'])
        require(sha(observed['commands']) == c['captured_command_stream_sha256'], 'Different creature command history')
        originals = aa.blend.OriginalWorlds(reader, c)
        history = History(originals)
        rows = [v for v in case['witnesses'] if v['kind'].startswith('animation:')]
        paints = {v['presentation']['paint']: v for v in rows}
        at_sequence = {}
        for paint, row in paints.items():
            p, body = observed['paints'][paint]
            require(paint in observed['swaps'], 'Creature frame was not actually swapped')
            at_sequence.setdefault(p['commands'], []).append((p, body))
        found = set()
        for op, sequence, payload in commands(observed['commands']):
            history.apply(op, sequence, payload)
            for p, body in at_sequence.get(sequence, []):
                for row in rows:
                    shown = row['presentation']
                    if shown['paint'] != int(p['paint']):
                        continue
                    generation = row['source']['generation']
                    require(int(p['world']) == generation and int(p['lease']) == shown['lease'] > 0,
                            'Creature generation/lease differs')
                    sealed = history.sealed
                    require(sealed and sealed['tags'] is not None and p['scaling'] == 2, 'Missing creature coordinate lineage')
                    if p['mode'] == 0:
                        require(history.display[0] == sealed['surface'] == p['surface'] and sealed['generation'] == generation,
                                'Wrong direct creature source')
                    else:
                        require(sealed['generation'] == int(p['sealed_world']), 'Wrong sealed creature source')
                    tags = sealed['tags']
                    yy, xx = np.nonzero(np.isin(tags, shown['mapped_source_words']))
                    indices = tags[yy, xx].astype('int64')
                    require(set(map(int, np.unique(indices))) == set(shown['mapped_source_words']), 'Missing actual creature pixels')
                    scale = p['image_width'] // p['frame_width']
                    require(scale > 0 and p['image_width'] == scale * p['frame_width']
                            and p['image_height'] == scale * p['frame_height'], 'Unsupported creature scaling')
                    raw = body() if callable(body) else body
                    require(sha(raw) == shown['complete_viewport_sha256'], 'Changed complete creature viewport')
                    actual = np.frombuffer(raw, dtype='u1').reshape(p['height'], p['width'], 4)
                    expected = np.array([aa.blend.rgba(int(originals[generation].flat[at])) for at in indices], dtype='uint8')
                    parts = []
                    for dy in range(scale):
                        for dx in range(scale):
                            value = actual[p['top'] + yy * scale + dy, p['left'] + xx * scale + dx]
                            require(np.array_equal(value, expected), 'Actual software creature WORDs differ from original')
                            parts.append(value.tobytes())
                    require(sha(b''.join(parts)) == shown['actual_sha256'] == shown['expected_sha256'],
                            'Frozen software creature region differs')
                    found.add((row['kind'], generation))
            if len(found) == 6:
                break
        require(len(found) == 6, 'Incomplete software creature resource/frame coverage')
        result.append(dict(domain=case['domain'], identity=case['identity'], pair=case['pair'], assets=case['assets'],
                           witnesses=case['witnesses'], actual_pixels_verified=True))
    return result


def close_rows(plan, parent, cohort, retired, creatures):
    validate(plan)
    require(parent['success'] and parent['dossier_identity'] == fingerprint(requirements(plan))
            and len(parent['requirements']) == 156, 'Changed acceptance inventory')
    for profile in ('player-default', 'player-software'):
        require(all(next(v for v in parent['requirements'] if v['configuration'] == profile and v['id'] == leaf)['state'] == 'passed'
                    for leaf in ('journey.shutdown.world-exit', 'journey.shutdown.menu-exit', 'journey.shutdown.cleanup')),
                'All shutdown prerequisites must already be accepted')
    require(len(cohort) == len(retired) and {v['reference']['record'] for v in cohort} ==
            {v['reference']['record'] for v in retired} and len({v['identity'] for v in retired}) == len(retired),
            'Accepted primary cohort omitted, duplicated or replaced')
    for item in retired:
        original = next(v for v in cohort if v['reference']['record'] == item['reference']['record'])
        require(all(item[key] == original[key] for key in ('reference', 'profile', 'rows', 'comparisons')),
                'Primary-cohort identity/profile/attachments changed')
        check_retirement(item)
    require({v['domain'][0] for v in creatures} == {'player-software'}
            and {tuple(v['domain'][1]) for v in creatures} == {(640, 480), (800, 600)}
            and len(creatures) == 2 and all(v['actual_pixels_verified'] for v in creatures), 'Incomplete software creature output')
    result, changed = [], set()
    for old in parent['requirements']:
        row = dict(old)
        key = row['configuration'], row['id']
        if key in ROWS:
            require(row['state'] == 'pending', 'Selected bounded row already accepted')
            if row['id'] == 'gate.normal-lifetime':
                proof = [dict(execution=v['reference'], reason='Original normal Quit and all raw terminal input/cursor/graphics/process/channel/source owners are checked for every admitted primary case in this exact cohort. Future cohort additions require the same admission; faults and unknown later journeys remain separate.') for v in retired if v['profile'] == row['configuration']]
            else:
                proof = [dict(execution=v['pair'], actual_world_extents=[v['domain'][1]],
                              reason='AA07 software creature frames and archived named resources are rechecked against actual native viewport bytes and independently expected original WORDs. Original simulation remains the frame supplier; native controller/timing/lifecycle recovery and current engine execution are separate.') for v in creatures]
            require(proof, 'Empty row evidence')
            row.update(state='passed', admitted_execution_evidence=proof)
            changed.add(key)
        result.append(row)
    require(changed == ROWS and sum(v['state'] == 'passed' for v in result) == parent['passed_requirements'] + 3,
            'Closeout exceeds the three selected rows')
    return result


def sources(reader):
    reg = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    paths = {n for b in reg['behaviors'] if b['id'] in BEHAVIORS
             for role in ('implementation', 'tests', 'validation_dependencies') for n in b[role]}
    return {n: reader.digest(n) for n in sorted(paths)}


def analyze(reader, parent_ref):
    parent = checked(reader, parent_ref)
    cohort = primary_cohort(reader, parent)
    retired = []
    for item in cohort:
        retired.append(retire_case(reader, item))
        print('Verified normal retirement', item['profile'], item['reference']['record'], flush=True)
    creatures = software_frames(reader)
    rows = close_rows(reader.json('docs/native-rendering-closure.json'), parent, cohort, retired, creatures)
    current = sources(reader)
    combined = dict(parent['sources'])
    combined.update({n: h for n, h in current.items() if n not in combined})
    passed = parent['passed_requirements'] + 3
    return dict(schema=1,kind='retained_creature_and_normal_lifetime_closeout',success=True,
                sources=combined,analysis_sources=current,parent=parent_ref,
                animation_input=dict(record=ANIMATION,sha256=reader.digest(ANIMATION)),
                cohort=cohort,retirements=retired,creatures=creatures,requirements=rows,
                dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {leaf for profile,leaf in ROWS}),
                passed_requirements=passed,pending_requirements=156-passed,newly_passed_requirements=3,
                actual_primary_executions=len(retired),source_imports=sum(v['proof']['source_imports'] for v in retired),
                actual_software_creature_contexts=2,software_frame_states=12,
                scope='Close the software creature-animation row and the two normal-lifetime gate rows by fresh retained-data analysis. Every primary execution referenced by this frozen accepted-row inventory must satisfy raw normal retirement; new cohort additions require their own admission under this guard. Check actual software creature viewport bytes against independent original WORDs. Preserve every inherited row and both decisions. Native execution remains at recorded sources; original ANI/timing, faults/restart, unexecuted future journeys, arbitrary providers and final decisions remain separate.',
                original_game_runs=0,new_native_code=False,new_fixture=False,
                current_native_engine_execution=False,historical_failures_retired=False)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--parent', required=True)
    p.add_argument('--report', required=True)
    args = p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts
            and not (ROOT/args.report).exists(), 'Fresh owned result required')
    result = dict(schema=1,success=False)
    try:
        with Reader(ROOT) as reader:
            before = sources(reader)
            result = analyze(reader, dict(record=args.parent,sha256=reader.digest(args.parent)))
            require(before == result['analysis_sources'], 'Analyzer/helper source changed during execution')
            result['analysis_sources_stable'] = True
    except Exception as error:
        result.update(success=False,error=str(error))
    digest = write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=digest)),flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
