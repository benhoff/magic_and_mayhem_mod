#!/usr/bin/env python3
"""Current selected-plane operation output and normal-owner admission."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re

from broader_player_acceptance import PREFIX,checked,native_phase
from coverage_evidence import Reader,write_report
from journeys_gate import lifetime
from rendering_menu_acceptance import artifact
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]


def module(name):
    spec=importlib.util.spec_from_file_location('broader_component_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def assess(reader,acquisition_name,comparison_name,report_name):
    ref=lambda n:dict(record=n,sha256=reader.digest(n))
    a,c=checked(reader,ref(acquisition_name)),checked(reader,ref(comparison_name))
    require(c['acquisition']==ref(acquisition_name)and a['actual_provider_verified'],'Mixed/incomplete current component')
    phase=checked(reader,a['broader_phase']);case=next(v for v in phase['broader_cases']if v['report']==acquisition_name)
    built=checked(reader,phase['current_player_build']);provenance=native_phase(ROOT,reader,a,built['player']['sha256'])
    require(provenance['current_native_source_candidate'],'Historical/incomplete component native source phase')
    require(a['sources_stable']and all(reader.digest(n)==h for n,h in a['sources'].items()),'Current component source changed')
    global_plan=checked(reader,phase['broader_plan']);fixture=checked(reader,global_plan['owned_input'])
    allowed=set(fixture['permitted_mutations']);initial,actual=fixture['input_fingerprints'],a['installation_inputs']
    require(actual.keys()==initial.keys()and all(actual[n]==v for n,v in initial.items()if n not in allowed),'Unreviewed component input change')
    cfg=artifact(reader,a,str(Path(a['session'])/'game/CFG/prefs.cfg'))
    require(re.findall(rb'(?mi)^IsHighRes\s*=\s*(TRUE|FALSE)',cfg)==[b'FALSE'if case['size'][0]==640 else b'TRUE'],'Wrong actual component preference context')
    extra={}
    if phase['family']=='clipping':
        require(c['analysis_completed']and c['complete_output_verified']and c['native_clip_regions_verified']and c['analysis_sources_stable'],
                'Incomplete current selected-plane clipping proof')
        proof=c['case'];require(proof['actual_size']==case['size']and proof['source_domain']['verified']and proof['complete_reduced_intervals']>0,'Missing actual clipping domain')
        for key in('primary_identity_verified','normal_lifetime_verified','positive_complete_pixel_witnesses','actual_clip_region_verified','caller_clip_plan_verified'):
            require(proof['proof'][key]is True,'Incomplete component binding: '+key)
        fill=module('check-rendering-clipped-fill');fill.P=phase['collector_prefix'];fill.RUN=Path(report_name).stem;fill.COMPARISONS={case['id']:'01'}
        binary=ROOT/phase['native_fill_replayer']['path'];require(hashlib.sha256(binary.read_bytes()).hexdigest()==phase['native_fill_replayer']['sha256'],'Current clipped-fill replay binary differs')
        compiler=fill.compiler_sources(reader,binary.resolve())
        fill_case=fill.inspect(reader,case['id'],binary,0);fill.check_case(fill_case)
        extra=dict(clipped_fill=fill_case,clipped_fill_compiler=compiler)
        retirement=proof['terminal'];semantics=proof['proof']
        sources=a['sources']|c['analysis_sources']|compiler['sources']
    else:
        require(c['sources_stable']and c['closed_call_census']and c['natural_stretch_calls']>0 and c['mismatching_pixels']==0,
                'Incomplete current natural stretch planes/census')
        require(all(p['target_size']==case['size']for p in c['proofs']),'Actual stretch destination domain differs')
        retirement=lifetime(reader,ref(acquisition_name),case['profile'],dict(case={}))
        semantics=dict(verified=True,natural_stretch_calls=c['natural_stretch_calls'],proofs=c['proofs'])
        sources=a['sources']|c['analysis_sources']
    sources['tools/check-broader-component.py']=reader.digest('tools/check-broader-component.py')
    require(all(reader.digest(n)==h for n,h in sources.items()),'Current component analysis source changed')
    return dict(schema=1,kind='current_broader_component_case_validation',success=True,sources_stable=True,
        original_game_runs=0,actual_acquisition_original_games=1,acquisition=ref(acquisition_name),comparison=ref(comparison_name),
        family=phase['family'],profile=case['profile'],provider=case['provider'],actual_driver=a['resident']['renderer'],size=case['size'],
        current_native_phase=provenance,input_binding=dict(verified=True,fixture=global_plan['owned_input']),
        semantics=semantics,lifetime=retirement,additional_output=extra,historical_primary_replaced=case['historical_primary'],
        sources=sources,current_case_proved=True,whole_viewport_credit=False,
        scope='Actual current operation-specific selected whole planes and unchanged exterior with exact caller/clip/state and normal retirement. Global capture truncation earns no complete-World history or whole-viewport credit.',global_player_accepted=False)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in('acquisition','comparison','report'):p.add_argument('--'+name,required=True)
    args=p.parse_args();require(args.report.startswith(PREFIX)and not(ROOT/args.report).exists(),'Fresh immutable component validation required')
    with Reader(ROOT)as r:
        try:q=assess(r,args.acquisition,args.comparison,args.report)
        except(ValueError,KeyError,OSError,TypeError)as error:q=dict(schema=1,kind='current_broader_component_case_validation',success=False,error=str(error),original_game_runs=0,acquisition=dict(record=args.acquisition,sha256=r.digest(args.acquisition)),comparison=dict(record=args.comparison,sha256=r.digest(args.comparison)))
    h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=args.report,sha256=h)),flush=True)
    return int(not q['success'])


if __name__=='__main__':raise SystemExit(main())
