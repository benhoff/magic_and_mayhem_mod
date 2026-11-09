"""Bounded retained proof of natural victory and later-map output.

The original execution/oracle phases remain immutable. QWidget presentation is
functional source-bound policy, not an original glyph/font/DC pixel comparison.
"""
import importlib.util
import json
from collections import Counter
from pathlib import Path

from campaign_transition_review import semantic_handoff
from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_defeat import cohort
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, require, sha
from rendering_progression import validate_callbacks

BID = 'NP.campaign-victory-next-map-closeout'
SID = 'native.campaign-victory-next-map-closeout'
OUT = 'research/runtime/campaign-victory-next-map-results-20261109/'
PARENT = 'research/runtime/campaign-progression-closeout-results-20261108/acceptance-01.json'
LEAVES = ('journey.battle-results.victory', 'journey.map-transitions.next-map')
INPUT = 'research/runtime/rendering-progression-results-20261101/'
CASES = tuple(INPUT + 'closeout-ready-' + name + '.json' for name in (
    'software640-full-43', 'software800-full-44', 'default640-full-45', 'default800-full-46'))
UI_SOURCES = ('apps/qt-shell/main.cpp', 'apps/qt-shell/campaign_progression_test.cpp',
              'apps/qt-shell/live_campaign_defeat_controller.cpp',
              'apps/qt-shell/live_menu_session.cpp', 'apps/qt-shell/menu_bridge.cpp',
              'apps/qt-shell/battle_result_widget.cpp', 'apps/qt-shell/battle_result_widget.hpp')
SCOPE = ('Only natural Celtic region1 victory/result acknowledgement and actual region2 first/resumed '
         'World output in four retained default/software640/800 domains. Complete captured native swaps '
         'are rechecked against the immutable independent comparison; native World resources retire '
         'and reacquire normally. The compiled native result control is visible/enabled, projects the '
         'original report and is withdrawn on fresh Realm return. QWidget glyph/font/DC pixels and '
         'physical scanout are separate. Initial original Load is input only. Original player/compiler/'
         'runtime/oracle source hashes remain historical; no new game, original oracle execution, '
         'current whole-player validation, assembled section/RNG recovery or final decision follows.')


def load(root, name):
    spec = importlib.util.spec_from_file_location(name.replace('-', '_'), root / 'tools' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def victory_route(flow, events):
    handoff = semantic_handoff(flow, events)
    callbacks = validate_callbacks(events, flow)
    report, returned = flow['states'][4:6]
    require(report['outcome'] == 1 and report['title'].startswith('Victory!')
            and len(report['texts']) == 21 and all(isinstance(v, str) for v in report['texts'])
            and not any('quit the battle' in v.lower() or 'you were killed' in v.lower()
                        for v in report['texts']), 'Wrong, abandoned or synthetic primary result')
    require(returned['screen'] == 4 and returned['ack'] == report['ack'] + 1
            and returned['generation'] > report['generation']
            and returned['sequence'] > report['sequence'], 'Unacknowledged/stale result return')
    dispatch = callbacks['original_report_dispatch']
    require(dispatch['thread'] == report['thread'] and dispatch['return_sequence'] == handoff['report_ok']
            and dispatch['begin_sequence'] < dispatch['return_sequence'] < handoff['original_world_pop'][0],
            'Result dispatch/return and actual World retirement are not ordered')
    return dict(verified=True, report=report, returned_realm=returned,
                original_dispatch=dispatch, world_pop=handoff['original_world_pop'],
                later_entry=handoff['later_entry'],
                native_result_policy=dict(control='battleResultContinue', visible_enabled_control_consumed=True,
                    original_text_projection=True, withdrawn_on_fresh_realm=True,
                    persistent_widget_destroyed=False, widget_pixels_compared=False),
                scope='Original report bytes/ACK/callbacks plus exact compiled public-control projection. '
                      'The persistent QWidget is hidden/disabled on Realm return; no destruction or glyph-pixel equivalence is inferred.')


def map_identity(acquired, preparation, route):
    require(acquired['map_inputs_stable'] is True and preparation['success'] is True,
            'Unstable or incomplete campaign input preparation')
    maps = preparation['maps']
    require(len(maps) == 2 and [(m['realm'], m['region']) for m in maps] == [('Celtic', 1), ('Celtic', 2)],
            'Different campaign map selectors')
    cfg = acquired['installation_inputs'].get('Realms/Celtic/Celtic.cfg')
    require(cfg and cfg['bytes'] > 0 and len(cfg['sha256']) == 64, 'Missing exact campaign configuration')
    for m in maps:
        require(m['name'] and m['inputs'] and m['specific'] and m['random'], 'Empty campaign recipe domain')
        for item in m['inputs']:
            require(acquired['installation_inputs'].get(item['path']) ==
                    dict(sha256=item['sha256'], bytes=item['bytes']), 'Map input identity differs: ' + item['path'])
    entry = route['later_entry']
    require(entry['realm'] == maps[1]['realm'] and entry['region'] == maps[1]['region'] == 2
            and entry['enter_handoff'] == 3 and entry['different'] == 0,
            'Actual later-map entry does not bind the supplied map domain')
    require(maps[0]['name'].rstrip('.') in route['report']['title'], 'Victory title belongs to another initial map')
    return dict(verified=True, realm='Celtic', initial_region=1, region=2,
                name=maps[1]['name'], campaign_configuration=cfg,
                candidate_inputs=maps[1]['inputs'], recipe_sha256=fingerprint(maps[1]),
                actual_original_region_entry=entry,
                assembled_section_choices_recovered=False,
                scope='Actual original region2 admission after settled campaign travel, bound to exact configuration/candidate inputs. '
                      'The captured rendered World below binds actual output; candidate recipes do not identify section choices, layout or RNG state.')


def later_output(flow, observed, comparison):
    first, later = flow['worlds']
    baseline, witness = later['after_world'], later['world']
    require(later['entry_mode'] == 'campaign-travel' and later['realm'] == 'Celtic'
            and later['region'] == 2 and first['world'] < baseline < witness,
            'Reload or stale map cannot supply next-map output')
    shown = [observed['paints'][paint][0] for paint in observed['swaps']
             if observed['paints'][paint][0].get('reference_binding')
             and int(observed['paints'][paint][0]['world']) > baseline]
    rows = [r for r in comparison['comparisons'] if int(r['world']) > baseline]
    key = lambda r: (int(r['paint']), int(r['lease']), int(r['reference_binding']),
                     r['immutable_lease'], int(r['world']))
    require(shown and Counter(map(key, shown)) == Counter(map(key, rows)),
            'Later-map comparison omitted or mixed a native swap')
    require(all(r['pixels'] > 0 and r['mismatching_pixels'] == 0
                and r['actual_sha256'] == r['expected_sha256'] for r in rows),
            'Incomplete or mismatching later-map planes')
    require(any(int(r['world']) == witness and int(r['paint']) == later['paint'] for r in rows),
            'Fresh identity-bound later World was not compared')
    resumed = [r for r in rows if int(r['world']) > witness]
    require(resumed, 'No later-map resumed World output')
    first_paint = rows[0]
    require(int(first_paint['world']) <= witness and len({int(r['world']) for r in rows}) >= 2,
            'No complete first and resumed later-map output')
    return dict(verified=True, after_world=baseline, observed_identity_world=witness,
                first_displayed=first_paint, resumed=resumed[0],
                compared_swaps=len(rows), pixels=sum(r['pixels'] for r in rows),
                captured_world_generations=sorted({int(r['world']) for r in rows}),
                scope='Every captured leased swap after the fresh entry baseline, including first displayed and later distinct World generations. '
                      'The full cohort is independently checked; the100ms probe may skip intermediate generations.')


def analyze_case(reader, path):
    root = reader.root
    maps = load(root, 'close-campaign-map-transitions')
    prior = maps.analyze(reader, path)
    a = reader.json(path); name = path.removesuffix('.json') + '-comparison.json'; c = reader.json(name)
    route = victory_route(a['flow'], artifact(reader, a, 'events.bin'))
    source_pins = {p: reader.digest(p) for p in UI_SOURCES}
    require(all(a['sources'].get(p) == h and c['sources'].get(p) == h for p, h in source_pins.items()),
            'Native result projection/driver source phase changed')
    preparation_ref = a['preparation']
    require(reader.digest(preparation_ref['record']) == preparation_ref['sha256'], 'Preparation reference changed')
    identity = map_identity(a, reader.json(preparation_ref['record']), route)
    capture_refs = [v for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
    require(1 < len(capture_refs) <= 8 and sum(v['bytes'] for v in capture_refs) <= 2*1024**3,
            'Capture count/RAM budget exceeded')
    observed = acquisition([artifact(reader, a, v['path']) for v in capture_refs], a['session_identity'])
    require(observed['observations'] == c['observations']
            and sha(observed['commands']) == c['captured_command_stream_sha256'], 'Wrong native capture cohort')
    all_output = cohort(observed, c['comparison'])
    later = later_output(a['flow'], observed, c['comparison'])
    require(prior['cleanup']['normal_quit'] and prior['terminal_source']['native_owned_slots'] == 0,
            'Normal final native retirement missing')
    return dict(verified=True, identity=a['session_identity'], configuration=prior['configuration'],
                extent=prior['extent'], acquisition=prior['acquisition'], comparison=prior['comparison'],
                provider=prior['provider'], victory=route, map_identity=identity,
                later_output=later, complete_output=all_output, resources=prior['resources'],
                native_handoff=prior['native'], cleanup=prior['cleanup'], terminal_source=prior['terminal_source'],
                native_result_source_binding=source_pins, preparation=preparation_ref,
                historical_sources=dict(acquisition=a['sources'], comparison=c['sources']),
                current_native_execution=False, current_original_oracle_execution=False)


def close_rows(plan, parent, cases):
    validate(plan)
    template = requirements(plan)
    require(parent['success'] is True and parent['dossier_identity'] == fingerprint(template)
            and len(parent['requirements']) == 156
            and [(r['configuration'], r['id']) for r in parent['requirements']] ==
                [(r['configuration'], r['id']) for r in template]
            and parent['passed_requirements'] == sum(r['state'] == 'passed' for r in parent['requirements']) == 120,
            'Changed120row campaign parent')
    require(len(cases) == 4 and len({c['identity'] for c in cases}) == 4
            and {(c['configuration'], tuple(c['extent'])) for c in cases} ==
                {(p, s) for p in ('player-default', 'player-software') for s in ((640, 480), (800, 600))}
            and all(c['verified'] is True for c in cases), 'Four distinct complete campaign domains required')
    rows = []
    for old in parent['requirements']:
        row = dict(old)
        if row['id'] in LEAVES:
            require(row['state'] == 'pending', 'Campaign result row already accepted')
            row.update(state='passed', admitted_execution_evidence=[dict(acquisition=c['acquisition'],
                comparison=c['comparison'], actual_world_extents=[c['extent']], provider=c['provider'], reason=SCOPE)
                for c in cases if c['configuration'] == row['configuration']])
        rows.append(row)
    require(sum(r['state'] == 'passed' for r in rows) == 124
            and all(n == o for n, o in zip(rows, parent['requirements']) if o['id'] not in LEAVES),
            'Closure exceeds the requested four campaign profile cells')
    return rows
