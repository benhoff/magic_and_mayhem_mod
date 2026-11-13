#!/usr/bin/env python3
"""Freeze complete finite current World replacements without renewing old runs."""
import argparse
import collections
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys

from broader_world_cases import FAMILIES,RETAINED_SOFTWARE,family_of,initial_extent
from coverage_claims import behavior_contract,scenario_contract
from broader_player_sources import python_sources
from coverage_evidence import Reader,write_report
from native_music_launch import catalog_inputs
from check_inputs import paths
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
PH='research/runtime/pixels-history-gate-results-20261109/admission-01-proof.json'
WP='research/runtime/whole-player-validation-results-20261110/'


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    parser.add_argument('--global-plan',default=P+'plan-02.json')
    parser.add_argument('--basis',default='research/runtime/surface-loss-restore-results-20261109/bp10-input-plan-01.json')
    parser.add_argument('--phase-key',default='bp10')
    parser.add_argument('--capture-ram',action='store_true')
    parser.add_argument('--prerequisites')
    parser.add_argument('--all-worlds',action='store_true')
    args=parser.parse_args()
    require(args.phase_key.isalnum()and args.phase_key.startswith('bp'),'Safe current phase key required')
    require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh current World preflight required')
    test_commands=[[sys.executable,'tests/test-broader-worker-world-launch.py'],[sys.executable,'tests/test-broader-world-admission.py']]
    tests=[subprocess.run(command,cwd=ROOT,capture_output=True)for command in test_commands]
    require(all(value.returncode==0 for value in tests),'Current World launch/admission controls failed')
    with Reader(ROOT)as r:
        global_name=args.global_plan;global_plan=r.json(global_name)
        prerequisites=r.json(args.prerequisites)['controls']if args.prerequisites else{}
        def choose(key,default):
            if key not in prerequisites:return default
            value=prerequisites[key];require(r.digest(value['record'])==value['sha256'],'Changed prerequisite identity: '+key);return value['record']
        basis=r.json(args.basis)
        current_build=dict(record=P+'build-current-04.json',sha256=r.digest(P+'build-current-04.json'))
        built=r.json(current_build['record'])
        require(all(r.digest(n)==h for n,h in(basis['compiler_sources']|basis['runtime_sources']|basis['oracle']['native_sources']).items()),'Native player/runtime/oracle source changed')
        require(hashlib.sha256((ROOT/basis['player_binary']['path']).read_bytes()).hexdigest()==built['player']['sha256'],'Current player changed')
        for name in(choose('native',P+'native-controls-05.json'),choose('hardware',P+'hardware-host-controls-01.json'),choose('overlap',P+'overlap-controls-01.json')):
            q=r.json(name);require(q['success']and q['sources_stable']and all(r.digest(n)==h for n,h in q['sources'].items()),'Current prospective control changed: '+name)
        proof=r.json(PH);historical=proof['cases']+proof['additional_presented_cases']
        require(len(historical)==56,'Changed finite historical World inventory')
        groups=collections.defaultdict(list)
        for row in historical:
            pair=row['pair'];a=r.json(pair['acquisition']['record']);c=r.json(pair['comparison']['record'])
            require(a['success']and c['success']and c['acquisition']==pair['acquisition'],'Unaccepted historical policy template')
            family=family_of(pair['acquisition']['record'],a);size=initial_extent(c);profile='player-software'if a['configuration']['software_flag']else'player-default'
            groups[family].append(dict(historical=pair,acquired=a,comparison=c,size=size,profile=profile))
        require(set(groups)==set(FAMILIES),'Missing current World family')
        names=python_sources(ROOT,['tools/run-broader-worker-world.py'])|{'tools/check-broader-world.py','tests/test-broader-worker-world-launch.py','tests/test-broader-world-admission.py','tests/test-broader-player-launch.py','docs/native-rendering-closure.json','tools/broader_world_launch.py'}
        sources={n:r.digest(n)for n in sorted(names)}
        reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors']if v['id']=='NP.broader-player-acceptance');s=r.json(P+'prospective-scenario-01.json')['scenario']
        claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)})]
        controls_name=P+'world-launch-controls-'+args.phase_key+'.json';log_name=controls_name.removesuffix('.json')+'-log.bin'
        controls=dict(schema=1,kind='current_broader_world_launch_controls',success=True,sources_stable=True,original_game_runs=0,test_count=13,commands=test_commands,
            sources={n:sources[n]for n in('tools/broader_worker_world_launch.py','tools/broader_player_launch.py','tools/broader_native_host.py','tests/test-broader-worker-world-launch.py','tests/test-broader-world-admission.py','tools/check-broader-world.py')},
            log=dict(record=log_name,sha256=write_report(ROOT,log_name,b'\n'.join(value.stdout+value.stderr for value in tests))))
        controls_ref=dict(record=controls_name,sha256=write_report(ROOT,controls_name,(json.dumps(controls,indent=2)+'\n').encode()))
        wp=r.json(WP+'cohort-03.json');require(wp['success']and wp['current_finite_cohort_proved'],'Existing finite current software basis is incomplete')
        retained=[];plans=[];total=0
        for family in FAMILIES:
            definition=FAMILIES[family];prefix='research/runtime/'+definition['prefix']+'/'+args.phase_key+'/world-'+family+'/'
            plan_name=prefix+'input-plan-01.json';require(not(ROOT/plan_name).exists(),'Preserve existing World phase')
            template=groups[family][0]['acquired'];old_plan_ref=template.get('input_plan',template.get('prospective_plan'))
            if old_plan_ref:
                old_name=old_plan_ref['record']if isinstance(old_plan_ref,dict)else old_plan_ref
                old=r.json(old_name)
                # Only authored route/configuration fields are policy templates.
                # Old controls, executions, claims, source hashes and repairs are
                # never refreshed into a new execution or new accepted result.
                keep=('map','actor','input_policy','assets','required_nonzero_classes','minimum_displayed_generations_per_state',
                    'diagnose_rebuild','edge_schedule','camera_schedule','rebuild_route','excluded','preferences')
                q={k:copy.deepcopy(old[k])for k in keep if k in old}
                policy_template=dict(record=old_name,sha256=r.digest(old_name),role='Authored finite route configuration only; no historical execution or validation renewed')
            else:q={};policy_template=None
            execution_seeds=['tools/run-broader-worker-world.py','tools/'+definition['controller']]
            if 'low_controller'in definition:execution_seeds.append('tools/'+definition['low_controller'])
            if family in('victory','defeat'):execution_seeds.append('tools/prepare-rendering-progression.py')
            names=python_sources(ROOT,execution_seeds)|{'tools/broader_player_sources.py'}
            sources={n:r.digest(n)for n in sorted(names)}
            q.update(schema=1,kind='current_broader_world_prospective_plan',success=True,original_game_runs=0,family=family,
                collector_prefix=prefix,capture_storage=dict(enabled=args.capture_ram,policy='owned-tmpfs-diagnostic-capture-exact-copy-v1'),broader_plan=dict(record=global_name,sha256=r.digest(global_name)),request_driver='tools/run-broader-worker-source-case.py',
                current_player_build=current_build,current_player_build_binding=dict(record=P+'player-build-only-binding-04.json',sha256=r.digest(P+'player-build-only-binding-04.json')),
                policy_template=policy_template,claims=claims,launch_controls=controls_ref,
                sources=sources,compiler_sources=basis['compiler_sources'],runtime_sources=basis['runtime_sources'],
                player_binary=basis['player_binary'],oracle=basis['oracle'],oracle_sources=basis['oracle']['native_sources'],
                oracle_binaries=[basis['player_binary'],*basis['oracle']['binaries']],
                domains=[dict(software=z['profile']=='player-software',size=z['size'])for z in groups[family]],
                bounds=dict(primary_games=len(groups[family]),maximum_requests=6,maximum_original_games=5,
                    stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True,automatic_retries=False),
                maximum_actual_games=5,controller_modules=[definition['controller'],'run-rendering-interaction.py'],
                counter_bindings={},scope='Fresh actual current linked-player execution of the unchanged finite historical route/input domain, with specialized raw semantics, complete independent original output/history, observed NVIDIA/default or Mesa/software provider, advancing original source, bounded response/resource checks and raw terminal retirement. No unknown required caller, scope amendment, physical reset, whole-state or complete original-recovery promotion.')
            for name in('low_controller',):
                if name in definition:q['controller_modules'].append(definition[name])
            if family=='ui':q['controller_modules']+=['run-rendering-ui.py','run-rendering-ui-low.py']
            counters=['interaction-attempts.sqlite3']
            specific={'caller':'caller-state-requests.sqlite3','viewport':'viewport-cursor-requests.sqlite3','ui':'ui-requests.sqlite3','picking':'picking-closeout-requests.sqlite3','pause':'pause-attempts.sqlite3','font':'font-dc-attempts.sqlite3','palette':'palette-cycling/attempts.sqlite3','attachment':'scene-attachment/attempts.sqlite3','combat':'scene-effects/attempt-budget.sqlite3'}
            if family in specific:counters.append(specific[family])
            q['counter_bindings']={str(ROOT/'working'/name):'working/broader-player/'+args.phase_key+'/world-'+family+'/'+Path(name).name for name in counters}
            if family=='caller':q['bounds'].update(maximum_requests=9,maximum_original_games=7)
            if family=='hotspot':q['bounds'].update(maximum_requests=9,maximum_original_games=6)
            if family in('font','palette'):q.update(maximum_launch_attempts=9,maximum_actual_game_starts=7)
            if family=='font':
                amendment_name=prefix+'task-local-cap-01.json'
                amendment=dict(schema=1,kind='fresh_broader_font_task_local_cap',success=True,authorized=True,original_game_runs=0,
                    maximum_launch_attempts=9,maximum_actual_game_starts=7,primary_cases=4,
                    authorization_basis='The user requested do 2., the fresh broader player acceptance work. This is a new task-local compatibility ceiling for four required cases inside the exact bound cumulative broader-player plan; it does not assert a new numerical approval or reopen the previous font task. Any failure stops the cohort and is retained; no automatic repair starts are authorized.',historical_caps_and_charges_unchanged=True)
                q['budget_amendment']=dict(record=amendment_name,sha256=write_report(ROOT,amendment_name,(json.dumps(amendment,indent=2)+'\n').encode()))
                controls_map={'controls':choose('font_observer','research/runtime/font-dc-lifetime-results-20261108/bp10-observer-controls-01.json'),
                    'dc_controls':choose('font_dc','research/runtime/font-dc-lifetime-results-20261108/bp10-dc-controls-01.json'),
                    'hidden_controls':choose('hidden','research/runtime/viewport-cursor-results-20261108/bp10-hidden-copy-controls-01.json'),
                    'native_font_controls':choose('font_native','research/runtime/font-dc-lifetime-results-20261108/bp10-native-font-controls-02.json')}
                for key,name in controls_map.items():
                    control=r.json(name);require(control['success']and control['sources_stable']and all(r.digest(n)==h for n,h in control['sources'].items()),'Fresh font prerequisite changed: '+key)
                    q[key]=dict(record=name,sha256=r.digest(name))
                q['music_inputs'],q['music_toc']=catalog_inputs(paths(ROOT)['assets'])
            if family=='palette':
                for key,name in {'controls':choose('palette_observer','research/runtime/palette-cycling-results-20261108/bp10-observer-controls-01.json'),
                    'staging_preflight':choose('palette_staging','research/runtime/palette-cycling-results-20261108/bp10-staging-preflight-01.json'),
                    'probe_review':choose('palette_probe','research/runtime/palette-cycling-results-20261108/bp10-semantic-review-01.json')}.items():
                    control=r.json(name);require(control['success']and control.get('sources_stable',True)and all(r.digest(n)==h for n,h in control.get('sources',{}).items()),'Fresh palette prerequisite changed: '+key)
                    q[key]=dict(record=name,sha256=r.digest(name))
            cases=[]
            for prior in sorted(groups[family],key=lambda z:(z['profile'],z['size'][0])):
                software=prior['profile']=='player-software';size=prior['size'];low=size[0]==640
                reusable=RETAINED_SOFTWARE.get((family,size[0]))if software and not args.all_worlds else None
                if reusable:
                    name='research/runtime/rendering-normal-results-20261031/'+reusable
                    matches=[v for v in wp['executions']if v['pair']['acquisition']['record']==name]
                    require(len(matches)==1 and matches[0]['profile']==prior['profile']and size in matches[0]['world_extents'],'Missing exact reusable current software domain')
                    retained.append(dict(historical_primary=prior['historical'],current=matches[0]['pair'],accepted_current_receipt=dict(record=WP+'cohort-03.json',sha256=r.digest(WP+'cohort-03.json')),family=family,profile=prior['profile'],size=size))
                    continue
                cid=args.phase_key+'-world-'+family+'-'+('software'if software else'default')+str(size[0])+'-01';report=prefix+cid+'-acquisition.json'
                controller=definition.get('low_controller',definition['controller'])if low else definition['controller']
                arguments=['--report',report]
                prepare=None
                if family in('quick','storage','combat'):
                    arguments+=['--scenario','normal-'+family,'--size','800x600'if family=='quick'else str(size[0])+'x'+str(size[1])]
                    if family=='quick'and not low:arguments.append('--gameplay-witness')
                    if family=='combat':arguments.append('--scene-effects')
                elif family in('victory','defeat'):
                    prepare=prefix+cid+'-preparation-01.json'
                    arguments+=['--case',family,'--size',str(size[0])+'x'+str(size[1]),'--evidence-mode','full','--preparation',prepare]
                    if family=='victory':
                        entry=prior['acquired']['entry_checkpoint'];arguments+=['--entry-checkpoint',entry['name'],'--entry-checkpoint-reference',entry['reference']['record'],'--input-policy',prior['acquired']['configuration']['input_policy']]
                elif family in('font','palette','attachment'):
                    arguments+=['--plan',plan_name,'--size',str(size[0])+'x'+str(size[1])]
                else:
                    if family!='interaction':arguments+=['--plan',plan_name]
                    if low:arguments.append('--low')
                    if family=='viewport'and q.get('diagnose_rebuild'):arguments.append('--diagnose-rebuild')
                if software:arguments.append('--software')
                comparison=prefix+cid+'-comparison-01.json'
                checker=definition.get('low_comparison',definition['comparison'])if low else definition['comparison']
                compare=['python3','tools/'+checker,'--acquisition',report,'--report',comparison]
                if family=='font':compare+=['--native-font-controls',q['native_font_controls']['record']]
                if family=='caller':
                    declaration=ROOT/('working/broader-player/'+args.phase_key+'/world-caller/comparison-claims.json');declaration.parent.mkdir(parents=True,exist_ok=True)
                    raw=(json.dumps(dict(sources=sources|basis['compiler_sources']|basis['runtime_sources']|basis['oracle']['native_sources'],claims=claims),indent=2)+'\n').encode()
                    require(not declaration.exists()or declaration.read_bytes()==raw,'Preserve differing caller comparison declaration')
                    declaration.write_bytes(raw);q['comparison_declaration']=dict(path=str(declaration.relative_to(ROOT)),consumer='tools/check-rendering-caller-state.py --claims',record=prefix+'comparison-claims.bin',sha256=write_report(ROOT,prefix+'comparison-claims.bin',raw))
                    compare+=['--claims',str(declaration),'--requirements',choose('caller_requirements','research/runtime/caller-state-results-20261108/bp10-world-requirements-01.json'),'--model-build',choose('caller_model','research/runtime/caller-state-results-20261108/bp10-model-replay-build-01.json'),'--history-build',choose('caller_history','research/runtime/caller-state-results-20261108/bp10-history-service-build-02.json')]
                cases.append(dict(id=cid,report=report,command=['python3','tools/run-broader-worker-world.py','--plan',plan_name,'--report',report],
                    environment={'ATSPI_DBUS_IMPLEMENTATION':'dbus-daemon'},provider='mesa'if software else'nvidia',profile=prior['profile'],size=size,reserved_original_games=1,
                    controller_file=controller,controller_arguments=arguments,prepare_progression=prepare,
                    historical_primary=prior['historical'],comparison=comparison,comparison_command=compare))
            q['broader_cases']=cases;q['bounds']['primary_games']=len(cases)
            preimages={}
            for source in sorted(sources):
                old_git=subprocess.run(['git','show','HEAD:'+source],cwd=ROOT,capture_output=True)
                if old_git.returncode==0 and hashlib.sha256(old_git.stdout).hexdigest()==sources[source]:continue
                raw=(ROOT/source).read_bytes();record=prefix+'source-'+sources[source]+'.bin';preimages[source]=dict(record=record,sha256=write_report(ROOT,record,raw))
            q['source_preimages']=preimages
            plans.append(dict(record=plan_name,sha256=write_report(ROOT,plan_name,(json.dumps(q,indent=2)+'\n').encode()),family=family,primary_cases=len(cases)))
            total+=len(cases)
        expected=56 if args.all_worlds else 53
        require(total==expected and len(retained)==(0 if args.all_worlds else 3),'Changed current World replacement denominator')
        report=dict(schema=1,kind='complete_current_broader_world_preflight',success=True,original_game_runs=0,
            historical_policy_inventory=dict(record=PH,sha256=r.digest(PH)),historical_world_domains=56,
            plans=plans,primary_current_original_games=total,retained_current_software=retained,
            launch_controls=controls_ref,global_additional_primary_starts=30+(16 if args.all_worlds else 20)+12+total+8,
            maximum_additional_original_games=global_plan['maximum_additional_original_games'],
            all_previous_original_starts_and_failures_preserved=True,global_player_accepted=False)
        require(report['global_additional_primary_starts']<=global_plan['maximum_additional_original_games'],'Prospective global primary start bound exceeded')
        h=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,current_world_starts=total,retained_current_software=len(retained),additional_primary_games=report['global_additional_primary_starts'])),flush=True)


if __name__=='__main__':main()
