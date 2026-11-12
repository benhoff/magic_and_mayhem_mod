#!/usr/bin/env python3
"""Revalidate the complete prospective current-player cohort, with scoped claims."""
import argparse
import ast
import hashlib
import importlib.util
import json
import sqlite3
from pathlib import Path
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/whole-player-validation-results-20261110/'
NORMAL='research/runtime/rendering-normal-results-20261031/'
BID='NP.current-whole-player-validation'
SID='native.current-whole-player-validation'
CASES=('default-quick-01','software-quick-01','default-storage-01','software-storage-01','default-low-01','software-low-01','default-low-02','software-low-02')


def module(name):
    spec=importlib.util.spec_from_file_location('whole_player_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value)
    return value


def python_sources():
    pending=[ROOT/'tools'/name for name in ('run-current-player-validation.py','run-current-player-low-validation.py','run-current-player-low-fixture.py','check-current-player-validation.py',
        'close-current-player-validation.py','run-rendering-acceptance-pilot.py','check-rendering-acceptance-pilot.py',
        'join-rendering-normal-acceptance.py','test-native-campaign.py','recover-current-player-acquisition.py')]
    seen=set()
    while pending:
        path=pending.pop().resolve();name=path.relative_to(ROOT).as_posix()
        if name in seen:continue
        seen.add(name)
        for node in ast.walk(ast.parse(path.read_bytes())):
            names=[v.name for v in node.names] if isinstance(node,ast.Import) else ([node.module] if isinstance(node,ast.ImportFrom) and node.module else [])
            for name in names:
                for base in (ROOT/'tools',ROOT/'protocols/python'):
                    target=base/Path(*name.split('.')).with_suffix('.py')
                    if target.is_file():pending.append(target)
    return seen


def cohort_domains(executions):
    expected={(p,s)for p in ('player-default','player-software')for s in ('normal-quick','normal-storage')}
    require(len(executions)==8 and {(e['profile'],e['scenario'])for e in executions}==expected
            and len({e['identity']for e in executions})==8 and all(e['verified']is True for e in executions),
            'Missing, duplicate or unverified current profile/scenario')
    for profile in ('player-default','player-software'):
        extents={tuple(v)for e in executions if e['profile']==profile for v in e['world_extents']}
        require(extents=={(640,480),(800,600)},'Both actual World extents are required per profile')
    return True


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report',required=True);p.add_argument('--controls',required=True)
    args=p.parse_args();require(args.report.startswith(PREFIX)and'..'not in Path(args.report).parts and not(ROOT/args.report).exists(),'Fresh immutable closeout required')
    result=dict(schema=1,kind='current_player_functional_cohort_closeout',success=False,
        current_finite_cohort_proved=False,current_whole_player_proved=False,
        supported_player_accepted=False,original_recovery_complete=False,historical_failures_retired=False)
    with Reader(ROOT)as reader:
        try:
            reg=reader.json('research/runtime/coverage/register.json')
            b=next(v for v in reg['behaviors']if v['id']==BID);s=next(v for v in reg['scenarios']if v['id']==SID)
            require(python_sources()<=required_sources(b),'Incomplete conservative Python source closure')
            sources={n:reader.digest(n)for n in sorted(required_sources(b))};result['sources']=sources
            controls=reader.json(args.controls)
            require(controls['success']is True and controls['sources_stable']is True and controls['tests']>=12
                    and controls['failures']==controls['errors']==controls['skipped']==0
                    and all(sources.get(n)==h for n,h in controls['sources'].items()),'Current refusal controls required')
            with sqlite3.connect(ROOT/'working/whole-player/requests.sqlite3')as db:
                requests=db.execute('SELECT report,scenario,software,state,result_sha256,comparison FROM requests ORDER BY started_ns').fetchall()
            require(len(requests)==8 and all(row[3]=='compared'for row in requests), 'Eight compared original starts and no failed predecessor required')
            require({row[0]for row in requests}=={NORMAL+'wp10-'+case+'-acquisition.json'for case in CASES},'Launch budget differs from prospective cohort')
            original_build=next(v for v in reg['builds'] if v['id']=='nocd-40209ca7')
            checker=module('check-current-player-validation');executions=[]
            built=reader.json(PREFIX+'build-02.json')
            require(built['success']is True and reader.digest(built['player']['record'])==built['player']['sha256'],
                    'Initial tested player binary was not retained')
            for key,path in (('player','working/build/qt-shell/mnm-qt-shell'),('world-history','working/build/qt-shell/mnm-acceptance-world-history'),('original-reference','working/original-reference')):
                require(hashlib.sha256((ROOT/path).read_bytes()).hexdigest()==built[key]['sha256'],
                        'Current tested player/oracle binary differs from retained build')
            for case in CASES:
                name=NORMAL+'wp10-'+case+'-acquisition.json'
                require(next(row[4]for row in requests if row[0]==name)==reader.digest(name),'Acquisition budget hash changed')
                acquired=reader.json(name)
                require(acquired['installation_inputs']['Chaos.exe']['sha256']==original_build['source_sha256'],
                        'Current cohort original executable differs from the registered build')
                require(acquired['player_build']['binary']['sha256']==built['player']['sha256'],
                        'Cohort rebuilt a different player binary')
                execution=checker.assess_pair(reader,name,NORMAL+'wp10-'+case+'-comparison.json')
                require(all(sources.get(n)==h for n,h in execution.pop('sources').items()),'Mixed current source phases')
                if acquired.get('metadata_recovered_after_retention_refusal') is True:
                    recovered=reader.json(acquired['metadata_recovery']['record'])
                    require(reader.digest(acquired['metadata_recovery']['record'])==acquired['metadata_recovery']['sha256']
                            and recovered['success']is True and recovered['new_original_game_runs']==0
                            and recovered['actual_run_completed']is True and recovered['retained_original_start']==4,
                            'Unbound or misclassified post-run metadata recovery')
                    execution['metadata_recovery']=acquired['metadata_recovery']
                if case in ('default-low-01','software-low-01','default-low-02','software-low-02'):
                    manifest_path=str(Path(acquired['session']).relative_to(ROOT)/'manifest.json')
                    matches=[v for v in acquired['artifacts']if v['path']==manifest_path]
                    require(len(matches)==1,'Ambiguous original low-fixture launch manifest')
                    manifest_raw=reader.read_bytes(matches[0]['record'])
                    require(hashlib.sha256(manifest_raw).hexdigest()==matches[0]['sha256'],'Low-fixture launch manifest changed')
                    launched=json.loads(manifest_raw)
                    if case.endswith('-01'):
                        require(execution['world_extents']==[[800,600]] and launched['preference_store']is None,
                                'Retained ignored Preferences fixture differs from its negative diagnosis')
                        execution['requested_low_fixture_not_consumed']=True
                        execution['counted_only_as_actual_800_world']=True
                    else:
                        fixture_ref=acquired['configuration']['source_game_fixture'];fixture=reader.json(fixture_ref['record'])
                        require(reader.digest(fixture_ref['record'])==fixture_ref['sha256'] and fixture['success']is True
                                and fixture['shared_game_unchanged']is True and fixture['engine_executable_unchanged']is True
                                and acquired['installation_inputs']==fixture['after_inputs']
                                and launched['source_game']==fixture['owned_game']
                                and execution['world_extents']==[[640,480]],'Actual owned low input or World differs')
                        path=str(Path(acquired['session']).relative_to(ROOT)/'game/CFG/prefs.cfg')
                        configs=[v for v in acquired['artifacts']if v['path']==path]
                        require(len(configs)==1,'Actual staged low configuration missing')
                        body=reader.read_bytes(configs[0]['record'])
                        import re
                        require(hashlib.sha256(body).hexdigest()==configs[0]['sha256']
                                and re.findall(rb'(?mi)^IsHighRes\s*=\s*(TRUE|FALSE)',body)==[b'FALSE'],
                                'Original staged low setting was not consumed')
                        execution['low_resolution_input']=dict(fixture=fixture_ref,staged_configuration=configs[0],verified=True,
                            preferences_import_required=False)
                executions.append(execution)
            cohort_domains(executions)
            impact=reader.json(PREFIX+'historical-impact-01.json')
            require(impact['success']is True and impact['historical_success']is False
                    and impact['historical_failures_retired']is False and impact['historical_root_cause_proved']is False,
                    'Historical negative evidence or root-cause boundary lost')
            require(all(reader.digest(n)==h for n,h in sources.items()),'Source drift during complete cohort analysis')
            ref=lambda n:dict(record=n,sha256=reader.digest(n))
            result.update(success=True,current_finite_cohort_proved=True,sources_stable=True,
                executable_sha256=original_build['source_sha256'],
                executions=executions,original_game_runs=8,new_native_executions=8,new_pixel_comparisons=8,
                total_source_generations=sum(e['source_generations']for e in executions),
                total_world_presentations=sum(e['world_presentations']for e in executions),
                total_pixels_compared=sum(e['pixels_compared']for e in executions),
                retention_metadata_recoveries=[e['metadata_recovery']for e in executions if 'metadata_recovery'in e],
                controls=ref(args.controls),historical_failure_impact=ref(PREFIX+'historical-impact-01.json'),
                claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],
                remaining_boundaries=['Full historical journey/fault/provider combinations retain their exact execution/source applicability; this cohort does not refresh every accepted matrix row.',
                    'Required unknown caller/provider/ownership boundaries and complete original recovery remain independent.',
                    'Both configured profiles exercised llvmpipe on this host; physical GPU/reset compatibility, long-duration bounds and universal historical defect repair are unproved.'])
        except (ValueError,KeyError,OSError)as error:result['error']=str(error)
    result['scope']='Eight actual current linked-player executions: enabled-intro800x600 Quick Battle with original-state gameplay witnesses, two ignored-Preferences800x600 cases retained as negative fixture evidence, and corrected640x480 Quick Battle from verified owned configuration and enabled-intro campaign Save/Mini/Main Load/changed-draft Cancel/display recreation/normal Quit, in default and software profiles at actual 640x480 and 800x600 World extents. Continuous independent original-history comparisons, exact source/compiler/binary identity, current response/watchdog policies, advancing source generations, bounded capture, logical resource conservation and terminal owner retirement. This finite current whole-application cohort does not grant global supported-player acceptance or original recovery.'
    sha=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=sha,
        worlds=result.get('total_source_generations'),presentations=result.get('total_world_presentations'))),flush=True)
    return 0 if result['success']else 1

if __name__=='__main__':raise SystemExit(main())
