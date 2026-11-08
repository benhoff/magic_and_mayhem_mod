"""Finite natural-defeat admission from immutable original/native observations.

This is retained-data analysis. It neither executes the game nor renews the
historical compiler, native runtime or original-history oracle source phases.
"""
import hashlib
import importlib.util
import json
import struct
import subprocess
from collections import Counter
from pathlib import Path

from campaign_gameplay import read_rows, creatures
from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, require, sha
from rendering_progression import validate_callbacks, validate_flow

BID = 'NP.natural-defeat-closeout'
LEAF = 'journey.battle-results.defeat'
OUT = 'research/runtime/rendering-defeat-results-20261108/'
INPUT = 'research/runtime/rendering-progression-results-20261101/'
PARENT = 'research/runtime/rendering-clipped-fill-results-20261108/acceptance-01.json'
HELPER_REVISION = '8badfb18f1c87482b8f60dec58adb508656ada0c'
HELPER_SHA = 'f9e14a4ff25f70e383941e8a3048fa7f00b4ba915599d669c4b7743a5eafefee'
SCOPE = ('Natural Campaign defeat in four retained default/software actual640/800 domains: '
         'original observed player death, original result OK/World pop/fresh Main return, '
         'every captured native swap independently compared and normal input/source/graphics retirement. '
         'Compiled public-button menu driver and physical World input; no simulation writes. '
         'Historical player/compiler/runtime/oracle phases remain pinned, with no new game or pixel comparison. '
         'No current whole-player, widget glyph/font/DC, complete external-object acquire/release, '
         'victory/later-map, physical driver/scanout or final-decision claim.')


class BytesInput:
    """Read-only adapter for an existing strict path-based binary decoder."""
    def __init__(self, body):
        self.body = body

    def read_bytes(self):
        return self.body


def load(root, name):
    spec = importlib.util.spec_from_file_location(name.replace('-', '_'), root / 'tools' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def defeat_route(flow, helper, events, gameplay):
    semantic = validate_flow(flow, 'defeat')
    callbacks = validate_callbacks(events, flow)
    states = flow['states']
    require([s['event'] for s in states] == ['main', 'display-configuration', 'configured-main',
            'region-entry', 'natural-report', 'result-return'], 'Unexpected defeat route')
    report, returned = states[-2:]
    require(returned['screen'] == 3 and returned['generation'] > report['generation']
            and returned['sequence'] > report['sequence'] and len(flow['worlds']) == 1,
            'Defeat must return freshly to Main without another World')
    require('defeat' in report['title'].lower() and report['texts'][19] == 'You were killed.',
            'Natural death report text missing')
    require(helper['success'] is helper['completed'] is True and helper['case'] == 'defeat'
            and helper['resume_checkpoint'] is False and helper['simulation_writes'] is False
            and helper['semantic_outcome_accepted'] is False and helper['casts'] == []
            and helper['combat_policy'] == 'ordinary-defeat-v1'
            and helper['frame_size'] == [int(x) for x in flow['size'].split('x')],
            'Manufactured, resumed or incomplete defeat input')
    raw = read_rows(BytesInput(gameplay), complete=True)
    require(raw and all(r[2] == report['thread'] for r in raw), 'Original gameplay owner differs')
    terminal = helper['terminal_hero']
    require(terminal['type'] == terminal['owner'] == terminal['active'] == 0 and terminal['health'] <= 0,
            'No original dead player')
    actors = {r[0]: creatures([r])[r[4]] for r in raw if r[1] == 1}
    require(actors.get(terminal['sequence']) == terminal, 'Helper death does not match original actor bytes')
    health = helper['health_observations']
    require(health and health[0]['health'] > 0 and health[-1]['sequence'] < terminal['sequence']
            and all(a['sequence'] < b['sequence'] for a, b in zip(health, health[1:])),
            'No positive-to-dead original player observation chain')
    for observation in health:
        actor = actors.get(observation['sequence'])
        require(actor is not None and actor['slot'] == observation['slot'] == terminal['slot']
                and actor['health'] == observation['health'] and actor['type'] == actor['owner'] == 0,
                'Helper health does not match original actor bytes')
    require(helper['original_report_dispatch_observed'] == callbacks['original_report_dispatch'],
            'Physical helper did not observe this original report dispatch')
    rows = list(struct.iter_unpack('<16I', events[16:]))
    ok, quit_seq = callbacks['original_result_ok_sequence'], callbacks['original_main_quit_sequence']
    pops = [r for r in rows if r[1] == 28 and r[3] == 2]
    require(len(pops) == 1 and ok < pops[0][0] < quit_seq
            and pops[0][2] == report['thread'] and pops[0][11:13] == (0x659408, 3),
            'Original defeat World pop absent, duplicated or out of order')
    require(not any(r[1] in (11, 12) and r[0] > ok for r in rows), 'World ticks resume after defeat')
    require(len([r for r in rows if r[1] == 3 and r[3] == 6 and r[9] == 21]) == 1,
            'Multiple result returns')
    actions = helper['actions']
    require(actions and all(0 <= a['seconds'] <= b['seconds'] for a, b in zip(actions, actions[1:]))
            and actions[-1]['seconds'] <= helper['seconds'], 'Physical input ordering differs')
    return dict(verified=True, semantic=semantic, callbacks=callbacks, report=report,
                returned_main=returned, world_pop=list(pops[0]), terminal_hero=terminal,
                health_samples=len(health), original_actor_records=len(actors), physical_actions=len(actions),
                scope='Observed original player death and separate original defeat report/return/World pop. '
                      'Actor slot belongs to this launch; no stable cross-launch pointer or combat formula claim.')


def admit(a, c, pair, profile, size):
    require(a['kind'] == 'public_native_progression_acquisition'
            and c['kind'] == 'public_native_progression_comparison' and c['acquisition'] == pair['acquisition']
            and a['scenario'] == 'defeat', 'Mixed defeat input collectors')
    for obj in (a, c):
        require(obj['success'] is obj['sources_stable'] is obj['original_manifest_verified_before_after']
                is obj['artifact_retention_complete'] is True and obj['player_accepted'] is False
                and obj['original_pixels_used_as_native_inputs'] is False, 'Incomplete historical execution phase')
    cfg = a['configuration']
    require(cfg['id'] == profile and cfg['software_flag'] is (profile == 'player-software')
            and cfg['requested_size'] == 'x'.join(map(str, size)) == a['flow']['size']
            and cfg['skip_intros'] is False and cfg['terrain_lighting'] == 'native'
            and cfg['native_lighting_capture'] is False and cfg['viewport_scaling'] == 'integer-sharp'
            and cfg['source_chunks'] is True and cfg['world_capture_format'] == 4
            and cfg['diagnostic_only'] is False and 'injected_fault' not in cfg
            and cfg['evidence_mode'] == cfg['capture_policy']['mode'] == 'full'
            and cfg['capture_policy']['full_capture'] is cfg['capture_policy']['pixel_comparison_eligible'] is True
            and cfg['experimental_campaign_menus'] is True
            and all(cfg.get(k) is False for k in ('follow_reference_observation', 'input_admission_observation',
                     'order_admission_observation', 'startup_timing_observation')),
            'Defeat configuration differs from normal full target')
    require(a['exit_code'] == 0 and a['private_wine_stopped'] is a['compiler_closure_stable']
            is a['player_binary_stable'] is True and a['retirement']['state'] == 'completed'
            and a['retirement']['forwarded'] == 0 and c['comparison_processes_stopped'] is True,
            'Original or comparison process did not stop normally')
    resident = a['resident']; cap = resident['stage_profile']['capture']
    n = int(resident['completed'])
    require(n > 0 and n == int(resident['imports']) == int(resident['verified_returns'])
            == cap['accepted_worlds'] == cap['written_worlds'] == c['original_reference']['worlds']
            and int(resident['source_chunks']) == cap['accepted_source_chunks']
            == cap['written_source_chunks'] == c['original_reference']['source_chunks']
            and resident['pending'] is False and resident['normal_quit_retired'] is True and not resident['error']
            and int(resident['source_copy_rect_refusals']) == 0 and cap['format'] == 4
            and cap['complete'] is cap['finished'] is True and cap['truncated'] is False
            and cap['codec'] == 'zstd-prefix-sha256-v1' and not cap['error'] and cap['queue_bytes'] == 0,
            'Incomplete World source/return/capture history')
    if profile == 'player-software':
        require(resident['renderer'].lower().startswith(('llvmpipe', 'softpipe')), 'Software provider unproved')
    require(c['world_domain'] == dict(verified=True, format='RGB565', extents=[size],
            worlds=[dict(generation=i, width=size[0], height=size[1]) for i in range(1, n + 1)]),
            'Compared original World domain differs')
    ref = c['original_reference']
    require(ref['continuous_from_owned_initial_history'] is True and ref['native_player_pixels_read']
            is ref['original_destination_capture_read'] is False and c['lighting']['verified']
            is c['asset_inputs_stable'] is True, 'Dependent reference, assets or lighting')
    require(c['player_build_binding']['verified'] is True
            and c['player_build_binding']['build'] == a['player_build'], 'Historical player compiler binding differs')
    return n


def cohort(observed, comparison):
    """Join the entire native swap multiset, preserving repeated swap occurrences."""
    shown = [observed['paints'][p][0] for p in observed['swaps']
             if observed['paints'][p][0].get('reference_binding')]
    expected = Counter((int(r['paint']), int(r['lease']), int(r['reference_binding']),
                        r['immutable_lease'], int(r['world']), r['width'] * r['height']) for r in shown)
    rows = comparison['comparisons']
    actual = Counter((r['paint'], r['lease'], r['reference_binding'], r['immutable_lease'], r['world'], r['pixels'])
                     for r in rows)
    require(shown and expected == actual and comparison['success'] is True
            and comparison['remaining_surfaces'] == comparison['mismatching_pixels'] == 0
            and comparison['pixels_compared'] == sum(r['pixels'] for r in rows)
            and comparison['world_presentations'] == sum(r['world'] > 0 for r in rows)
            and all(r['mismatching_pixels'] == 0 and r['actual_sha256'] == r['expected_sha256'] for r in rows),
            'Independent comparison omits or changes an actual native swap')
    # The parser has already verified every compressed plane. Hash each selected
    # actual plane again against the independent comparison, one paint at a time.
    by_paint = {}
    for row in rows:
        require(row['paint'] not in by_paint or by_paint[row['paint']] == row['actual_sha256'],
                'Repeated paint has conflicting pixel hashes')
        by_paint[row['paint']] = row['actual_sha256']
    for p, digest in by_paint.items():
        body = observed['paints'][p][1]
        require(sha(body() if callable(body) else body) == digest, 'Compared native pixels differ from retained GPU plane')
    return dict(verified=True, native_swaps=len(rows), world_swaps=comparison['world_presentations'],
                pixels=comparison['pixels_compared'], mismatching_pixels=0,
                unleased_paints_excluded=True, complete_capture_validated=True)


def analyze_case(reader, profile, size):
    root = reader.root
    short = profile.removeprefix('player-')
    name = INPUT + 'matrix-defeat-' + short + '-' + 'x'.join(map(str, size)) + '-07'
    pair = {k: dict(record=name + suffix, sha256=reader.digest(name + suffix))
            for k, suffix in (('acquisition', '.json'), ('comparison', '-comparison.json'))}
    a, c = [reader.json(pair[k]['record']) for k in ('acquisition', 'comparison')]
    n = admit(a, c, pair, profile, size)
    route = defeat_route(a['flow'], json.loads(artifact(reader, a, 'progression-input-0.json')),
                         artifact(reader, a, 'events.bin'), artifact(reader, a, 'gameplay-events.bin'))
    require(route['callbacks'] == a['original_callbacks'] == c['original_menu_actions']
            and route['semantic'] == a['semantic_route'] == c['semantic_route'], 'Historical semantic analysis differs')
    worlds = artifact(reader, a, 'worlds.bin')
    cap = a['resident']['stage_profile']['capture']
    require(worlds[:8] == b'MNMPROF4' and len(worlds) == cap['file_bytes'], 'Retained source corpus differs')
    binary = a['player_build']['binary']
    require(reader.digest(binary['record']) == binary['sha256'], 'Tested historical player binary differs')
    observed = acquisition([artifact(reader, a, v['path']) for v in a['artifacts']
                            if '/capture/' in v['path'] and v['path'].endswith('.bin')], a['session_identity'])
    require(observed['observations'] == c['observations']
            and sha(observed['commands']) == c['captured_command_stream_sha256'], 'Compared capture source differs')
    pixels = cohort(observed, c['comparison'])
    normal = load(root, 'join-rendering-normal-acceptance')
    exit_module = load(root, 'close-rendering-world-exit')
    cleanup = normal.cleanup_proof(reader, a)
    resources = normal.resource_history(observed['commands'])
    imports = exit_module.imported_lifetimes(observed['commands'])
    require(n == len(imports['imports']) == len(resources['worlds']), 'World incarnations differ')
    terminal = exit_module.terminal_source(artifact(reader, a, 'world-continuous.bin'),
        artifact(reader, a, 'world-terminal-0001.bin'), imports, route['report']['thread'])
    require(cleanup['verified'] is True and cleanup['normal_quit'] is True
            and cleanup['pending_jobs'] == cleanup['graphics_owners'] == 0, 'Incomplete normal retirement')
    helper = subprocess.check_output(['git', 'show', HELPER_REVISION + ':tools/campaign-outcome-input.py'], cwd=root)
    require(sha(helper) == HELPER_SHA == a['sources']['tools/campaign-outcome-input.py'], 'Historical physical helper source differs')
    unchanged_driver = {p: reader.digest(p) for p in ('apps/qt-shell/campaign_progression_test.cpp',
                                                     'apps/qt-shell/battle_result_widget.cpp')}
    require(all(a['sources'][p] == h for p, h in unchanged_driver.items()), 'Reviewed menu driver/widget changed')
    return dict(verified=True, identity=a['session_identity'], domain=[profile, size], pair=pair,
                provider=a['resident']['renderer'], route=route, output=pixels, cleanup=cleanup,
                resources=resources, terminal_source=terminal, source_imports=n,
                first_import=imports['imports'][0], last_import=imports['imports'][-1],
                command_end=imports['command_end'], tested_binary=binary,
                historical_phases=dict(acquisition_sources=a['sources'], comparison_sources=c['sources'],
                    compiler_sources=a['player_build']['compiler_sources'], source_commit=a['source_commit'],
                    physical_helper=dict(revision=HELPER_REVISION, sha256=HELPER_SHA),
                    unchanged_driver_sources=unchanged_driver,
                    changed_native_runtime_sources=[p for p in a['native_runtime_sources']
                         if reader.digest(p) != a['sources'][p]],
                    current_native_execution=False, current_original_oracle_execution=False),
                populated_provider_lifetime_gap=c['populated_handoff_lifetime_closure'])


def close_rows(plan, parent, cases):
    validate(plan)
    template = requirements(plan)
    require(parent['success'] is True and parent['dossier_identity'] == fingerprint(template)
            and len(parent['requirements']) == 156
            and [(r['configuration'], r['id']) for r in parent['requirements']]
                == [(r['configuration'], r['id']) for r in template]
            and parent['passed_requirements'] == sum(r['state'] == 'passed' for r in parent['requirements']) == 100,
            'Changed100row accepted parent inventory')
    require(len(cases) == 4 and len({c['identity'] for c in cases}) == 4
            and {(c['domain'][0], tuple(c['domain'][1])) for c in cases}
                == {(p, s) for p in ('player-default', 'player-software') for s in ((640, 480), (800, 600))}
            and all(c['verified'] is True for c in cases), 'Missing or repeated defeat domain')
    rows = []
    for old in parent['requirements']:
        row = dict(old)
        if row['id'] == LEAF:
            require(row['state'] == 'pending', 'Defeat row already passed')
            row.update(state='passed', admitted_execution_evidence=[dict(reason=SCOPE, execution=c['pair'],
                actual_world_extents=[c['domain'][1]], provider=c['provider'])
                for c in cases if c['domain'][0] == row['configuration']])
        rows.append(row)
    require(sum(r['state'] == 'passed' for r in rows) == 102, 'Defeat promotion exceeds two rows')
    return rows
