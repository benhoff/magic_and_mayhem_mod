#!/usr/bin/env python3
"""Conjoin current World semantics, full independent output/history and retirement."""
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
from pixels_history_gate import Join
from rendering_menu_acceptance import artifact
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
FIELDS=dict(interaction='proof',viewport='proof',attachment='scene_attachment_route',combat='scene_combat_route',
    ui='ui_proof',hotspot='proof',picking='proof',pause='proof',victory='semantic_route',defeat='semantic_route',
    font='font_dc_lifetime',palette='palette_cycle_route',caller='caller_state')


def module(name):
    spec=importlib.util.spec_from_file_location('broader_world_check_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def specialized_semantics(family,comparison):
    require(family in FIELDS,'Unknown specialized World family')
    value=comparison.get(FIELDS[family])
    flag='semantic_route_verified'if family in('victory','defeat')else'verified'
    require(isinstance(value,dict)and value.get(flag)is True,'Missing specialized current World semantics')
    return value


def assess(reader, acquisition_name, comparison_name):
    ref=lambda n:dict(record=n,sha256=reader.digest(n))
    pair=dict(acquisition=ref(acquisition_name),comparison=ref(comparison_name))
    a,c=checked(reader,pair['acquisition']),checked(reader,pair['comparison'])
    require(c['acquisition']==pair['acquisition']and c['comparison_processes_stopped']and c['sources_stable'],
            'Foreign or incomplete independent comparison')
    phase=checked(reader,a['broader_phase'])
    cases=[v for v in phase['broader_cases']if v['report']==acquisition_name]
    require(len(cases)==1,'Unbound current World domain')
    case=cases[0];family=phase['family']
    require(a['actual_provider_verified']and a['configuration']['software_flag'] is(case['profile']=='player-software'),
            'Wrong current player provider/profile')
    built=checked(reader,phase['current_player_build'])
    provenance=native_phase(ROOT,reader,a,built['player']['sha256'])
    require(provenance['current_native_source_candidate'],'Historical or incomplete native execution source phase')
    require(all(reader.digest(n)==h for n,h in(a['sources']|c['sources']).items()),'Current acquisition/comparison sources changed')
    driver=a['resident']['renderer']
    if case['provider']=='nvidia':require_nvidia([driver])
    else:require(driver.lower().startswith(('llvmpipe','softpipe')),'Actual software renderer missing')
    global_plan=checked(reader,phase['broader_plan']);fixture=checked(reader,global_plan['owned_input'])
    initial,actual=fixture['input_fingerprints'],a['installation_inputs'];allowed=set(fixture['permitted_mutations'])
    require(actual.keys()==initial.keys()and all(actual[n]==v for n,v in initial.items()if n not in allowed),
            'Unreviewed current original input change')
    from broader_preferences_artifact import preferences_artifact
    cfg=preferences_artifact(reader,a,ROOT)
    wanted=b'FALSE'if case['size'][0]==640 else b'TRUE'
    require(re.findall(rb'(?mi)^IsHighRes\s*=\s*(TRUE|FALSE)',cfg)==[wanted],'Wrong staged original preference context')
    output=Join(reader).pair(pair,case['profile'])
    require(output['size']==case['size'],'Current independently observed initial extent differs')
    retired=lifetime(reader,pair['acquisition'],case['profile'],dict(case=output))
    if family in('quick','storage'):
        semantics=module('check-current-player-validation').assess_pair(reader,acquisition_name,comparison_name)
        response=semantics['response_progress']
    else:
        semantics=specialized_semantics(family,c)
        # Specialized comparers perform the exact state/physical-action/font/
        # palette/caller assertions before setting whole-report success.
        response=module('check-broader-player-case').epoch_progress(dict(resident=a['resident']),c)
    if family=='font':require(c['native_menu_fonts']['verified']and c['font_lifetimes_verified'],'Native font/DC lifetime incomplete')
    if family in('victory','defeat'):
        require(a['flow']['success']and a['flow']['normal_quit']and c['transition_worlds'],
                'Progression/results output or normal return missing')
    sources=a['sources']|c['sources']|{'tools/broader_preferences_artifact.py':reader.digest('tools/broader_preferences_artifact.py')}|{'tools/check-broader-relative-world.py':reader.digest('tools/check-broader-relative-world.py')}
    require(all(reader.digest(n)==h for n,h in sources.items()),'Current World analysis sources changed')
    return dict(schema=1,kind='current_broader_world_case_validation',success=True,sources_stable=True,
        original_game_runs=0,actual_acquisition_original_games=1,acquisition=pair['acquisition'],comparison=pair['comparison'],
        family=family,profile=case['profile'],provider=case['provider'],actual_driver=driver,
        actual_initial_size=case['size'],current_native_phase=provenance,input_binding=dict(verified=True,
            fixture=global_plan['owned_input'],staged_cfg_sha256=hashlib.sha256(cfg).hexdigest()),
        output_history=output,lifetime=retired,semantics=semantics,response_progress=response,
        historical_primary_replaced=case['historical_primary'],sources=sources,
        current_case_proved=True,global_player_accepted=False,required_boundaries_closed=False)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in('acquisition','comparison','report'):p.add_argument('--'+name,required=True)
    args=p.parse_args();require(args.report.startswith(PREFIX)and not(ROOT/args.report).exists(),'Fresh immutable current World validation required')
    with Reader(ROOT)as r:
        try:q=assess(r,args.acquisition,args.comparison)
        except(ValueError,KeyError,OSError,TypeError)as error:
            q=dict(schema=1,kind='current_broader_world_case_validation',success=False,error=str(error),original_game_runs=0,
                acquisition=dict(record=args.acquisition,sha256=r.digest(args.acquisition)),comparison=dict(record=args.comparison,sha256=r.digest(args.comparison)))
    h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=args.report,sha256=h)),flush=True)
    return int(not q['success'])


if __name__=='__main__':raise SystemExit(main())
