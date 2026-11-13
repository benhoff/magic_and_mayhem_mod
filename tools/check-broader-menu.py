#!/usr/bin/env python3
"""Recheck current menu/movie semantics, independent movie output and retirement."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re

from broader_player_acceptance import PREFIX, checked, native_phase
from coverage_evidence import Reader, write_report
from journeys_gate import lifetime
from native_render_driver import require_nvidia
from rendering_menu_acceptance import artifact, validate_artifacts
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def module(name):
    spec = importlib.util.spec_from_file_location('broader_menu_'+name.replace('-', '_'), ROOT/'tools'/(name+'.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def assess(reader, name):
    ref = dict(record=name, sha256=reader.digest(name))
    a = checked(reader, ref)
    phase = checked(reader, a['input_plan'])
    require(phase['family'] in ('menu', 'intros', 'skip'), 'Unknown current menu family')
    built = reader.json('research/runtime/whole-player-validation-results-20261110/build-02.json')
    provenance = native_phase(ROOT, reader, a, built['player']['sha256'])
    require(provenance['current_native_source_candidate'], 'Incomplete current native source phase')
    require(a['sources_stable'] and all(reader.digest(n) == h for n, h in a['sources'].items()),
            'Menu acquisition sources changed')
    provider = a['provider_adaptation']['provider']
    require((provider == 'mesa') is a['configuration']['software_flag'], 'Mixed player provider/profile')
    driver = a['resident']['renderer']
    if provider == 'nvidia':
        require_nvidia([driver])
        require(len(a['provider_adaptation']['host_receipts']) == 1, 'Missing owned hardware host')
    else:
        require(driver.lower().startswith(('llvmpipe', 'softpipe')), 'Unobserved software provider')
    plan = checked(reader, phase['broader_plan'])
    fixture = checked(reader, plan['owned_input'])
    initial, actual = fixture['input_fingerprints'], a['installation_inputs']
    allowed = set(fixture['permitted_mutations'])
    require(actual.keys() == initial.keys() and all(actual[n] == v for n, v in initial.items()
            if n not in allowed), 'Unreviewed game input change')
    cfg = artifact(reader, a, str(Path(a['session'])/'game/CFG/prefs.cfg'))
    wanted = b'TRUE' if a['configuration']['high_resolution'] else b'FALSE'
    require(re.findall(rb'(?mi)^IsHighRes\s*=\s*(TRUE|FALSE)', cfg) == [wanted],
            'Wrong staged original resolution context')
    callbacks = [[v['menu_id'], v['argument']] for v in module('test-menu-observer').decode(artifact(reader, a, 'events.bin')) if v['event'] == 3]
    if phase['family'] == 'menu':
        proof = validate_artifacts(reader, a, callbacks)
        output = dict(role='semantic-native-widget-policy', original_widget_pixel_equality=False,
                      visible_main=proof['visible_main_presentations'])
    elif phase['family'] == 'skip':
        expected, anchor = module('close-rendering-movie-return').references(reader)
        proof = module('close-rendering-movie-skip').validate_artifacts(reader, a, {n:v['expected'] if 'expected' in v else v for n,v in expected.items()})
        output = dict(role='actual-visible-native-media-cancellation-and-menu-semantics',
                      whole_movie_pixel_credit=False, reference=anchor)
    else:
        full = module('close-rendering-enabled-intros').case(reader, ref)
        proof = dict(verified=full['success'], main=full['main'], media=full['media'])
        output = dict(role='independent-complete-enabled-movie-viewport', proof=full['output'])
    profile = 'player-software' if a['configuration']['software_flag'] else 'player-default'
    retirement = lifetime(reader, ref, profile, dict(case={}))
    sources = dict(a['sources'])
    sources['tools/check-broader-menu.py'] = reader.digest('tools/check-broader-menu.py')
    require(all(reader.digest(n) == h for n,h in sources.items()), 'Current menu analysis changed')
    return dict(schema=1, kind='current_broader_menu_case_validation', success=True, sources_stable=True,
        acquisition=ref, phase=a['input_plan'], family=phase['family'], profile=profile,
        original_preference_context=[800,600] if a['configuration']['high_resolution'] else [640,480],
        original_game_runs=0, actual_acquisition_original_games=1, current_native_phase=provenance,
        actual_driver=driver, input_binding=dict(verified=True, fixture=plan['owned_input'],
            staged_cfg_sha256=hashlib.sha256(cfg).hexdigest()), semantic_proof=proof,
        output=output, lifetime=retirement, sources=sources, global_player_accepted=False)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--acquisition', required=True)
    p.add_argument('--report', required=True)
    args = p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(), 'Fresh immutable current validation required')
    with Reader(ROOT) as reader:
        try:
            q = assess(reader, args.acquisition)
        except (ValueError, KeyError, OSError, TypeError) as error:
            q = dict(schema=1, kind='current_broader_menu_case_validation', success=False,
                error=str(error), original_game_runs=0,
                acquisition=dict(record=args.acquisition,sha256=reader.digest(args.acquisition)))
    digest = write_report(ROOT, args.report, (json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(success=q['success'],error=q.get('error'),record=args.report,sha256=digest)),flush=True)
    return int(not q['success'])


if __name__ == '__main__':
    raise SystemExit(main())
