#!/usr/bin/env python3
"""Freeze eight current operation-component domains inside the unchanged cap."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys

from broader_player_acceptance import native_phase
from broader_player_sources import python_sources
from coverage_evidence import Reader,write_report
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
PH='research/runtime/pixels-history-gate-results-20261109/admission-01-proof.json'
CP='research/runtime/rendering-clipped-operations-results-20261107/'
ST='research/runtime/rendering-copy-natural-results-20261108/'


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    parser.add_argument('--basis',required=True)
    parser.add_argument('--global-plan',required=True)
    parser.add_argument('--phase-key',required=True)
    parser.add_argument('--capture-ram',action='store_true')
    args=parser.parse_args()
    require(args.phase_key.isalnum()and args.phase_key.startswith('bp'),'Safe current component phase required')
    require(args.report.startswith(P)and not(ROOT/args.report).exists(),'Fresh component preflight required')
    test_files=['tests/test-rendering-clipping.py','tests/test-rendering-clipping-bindings.py',
        'tests/test-rendering-clipped-fill.py','tests/test-native-copy-clip-regions.py','tests/test-natural-stretch-proof.py','tests/test-natural-stretch-admission.py']
    results=[]
    for test in test_files:
        command=[sys.executable,'tools/run-broader-component-controls.py']if test=='tests/test-rendering-clipped-fill.py'else[sys.executable,test]
        result=subprocess.run(command,cwd=ROOT,capture_output=True)
        if result.returncode:
            diagnostic=dict(schema=1,kind='current_component_control_refusal',success=False,original_game_runs=0,test=test,exit_code=result.returncode,log=(result.stdout+result.stderr).decode(errors='replace'),test_sha256=hashlib.sha256((ROOT/test).read_bytes()).hexdigest())
            write_report(ROOT,P+'component-control-refusal-'+args.phase_key+'-'+Path(test).stem+'.json',(json.dumps(diagnostic,indent=2)+'\n').encode())
        require(result.returncode==0,'Current component reader control failed: '+test)
        results.append((test,result))
    with Reader(ROOT)as r:
        built_name='research/runtime/whole-player-validation-results-20261110/build-02.json';built=r.json(built_name)
        basis=r.json(args.basis)
        require(all(r.digest(n)==h for n,h in(basis['compiler_sources']|basis['runtime_sources']).items()),'Current native source changed')
        global_name=args.global_plan;global_plan=r.json(global_name)
        global_ref=dict(record=global_name,sha256=r.digest(global_name))
        inventory_ref=global_plan['inventory'];inventory=r.json(inventory_ref['record'])
        require(r.digest(inventory_ref['record'])==inventory_ref['sha256']and len(inventory['cases'])==94,'Full94-primary inventory required')
        components=r.json(PH)['component_cases'];require(len(components)==8,'Missing selected-plane component denominator')
        control_name=P+'component-reader-controls-'+args.phase_key+'.json';logs=[]
        for i,(test,result)in enumerate(results):
            raw=result.stdout+result.stderr;name=control_name.removesuffix('.json')+'-log-'+str(i)+'.bin';logs.append(dict(test=test,exit_code=result.returncode,record=name,sha256=write_report(ROOT,name,raw)))
        controls=dict(schema=1,kind='current_broader_component_reader_controls',success=True,sources_stable=True,original_game_runs=0,
            sources={n:r.digest(n)for n in test_files+['tools/run-broader-component-controls.py']},groups=len(test_files),logs=logs)
        controls_ref=dict(record=control_name,sha256=write_report(ROOT,control_name,(json.dumps(controls,indent=2)+'\n').encode()))
        clip_template=r.json(CP+'input-plan-12.json');helper_ref=clip_template['exposure_helper_build'];helper=r.json(helper_ref['record'])
        require(r.digest(helper_ref['record'])==helper_ref['sha256']and helper['success']and all(r.digest(n)==h for n,h in helper['sources'].items()),'Existing unchanged Win32 helper controls impacted')
        artifact=next(v for v in helper['artifacts']if v.get('path','').endswith('/exposure.exe'))
        raw=r.read_bytes(artifact['record']);require(hashlib.sha256(raw).hexdigest()==artifact['sha256']==clip_template['exposure_helper']['sha256'],'Exact exposure helper changed')
        helper_path=ROOT/('working/broader-player/'+args.phase_key+'/component-exposure.exe');require(not helper_path.exists(),'Preserve existing helper materialization');helper_path.parent.mkdir(parents=True,exist_ok=True);helper_path.write_bytes(raw);helper_path.chmod(0o755)
        fill_binary=Path('/dev/shm/mnm-broader-player-20261110/native-controls/renderer/mnm-render-commands')
        require(fill_binary.is_file(),'Current separately built renderer command replay missing')
        fill_path=ROOT/('working/broader-player/'+args.phase_key+'/component-render-commands');require(not fill_path.exists()and not fill_path.is_symlink(),'Preserve existing replay materialization');fill_path.symlink_to(fill_binary)
        binary_name=P+'component-render-commands-'+args.phase_key+'.bin';fill_ref=dict(path=str(fill_path.relative_to(ROOT)),record=binary_name,sha256=write_report(ROOT,binary_name,fill_binary.read_bytes()))
        source_names=python_sources(ROOT,['tools/run-broader-component.py'])|{'tools/broader_player_sources.py'}|set(basis['compiler_sources'])|set(basis['runtime_sources'])
        sources={n:r.digest(n)for n in sorted(source_names)}
        plans=[]
        for family,prefix,controller in [('clipping',CP,'run-rendering-clipping.py'),('stretch',ST,'run-rendering-copy-branches.py')]:
            namespace=prefix+args.phase_key+'/';name=namespace+'input-plan-01.json';require(not(ROOT/name).exists(),'Preserve prior component phase')
            chosen=[v for v in components if v['pair']['acquisition']['record'].startswith(prefix)];require(len(chosen)==4,'Missing component domain')
            execution_names=source_names|python_sources(ROOT,['tools/'+controller])
            sources={n:r.digest(n)for n in sorted(execution_names)}
            q=dict(schema=1,kind='current_broader_component_prospective_plan',success=True,original_game_runs=0,family=family,
                collector_prefix=namespace,capture_storage=dict(enabled=args.capture_ram,policy='owned-tmpfs-diagnostic-capture-exact-copy-v1'),broader_plan=global_ref,current_player_build=dict(record=built_name,sha256=r.digest(built_name)),
                sources=sources,compiler_sources=basis['compiler_sources'],runtime_sources=basis['runtime_sources'],player_binary=basis['player_binary'],oracle=basis['oracle'],
                reader_controls=controls_ref,controller_modules=[controller,'run-rendering-interaction.py'],
                bounds=dict(maximum_requests=6,maximum_original_games=5,maximum_charged_original_games=5,
                    primary_original_games=4,stop_on_first_failure=True,stop_on_game_failure=True,case_deadline_seconds=600,retained_artifact_bytes=2*1024**3),
                configurations=[dict(software=s,size=z)for s in(False,True)for z in([640,480],[800,600])],
                reviewed_failures=[],repair_of=dict(record=None),
                scope='Fresh current player, actual NVIDIA/default or Mesa/software provider and full native/runtime source binding for the exact finite selected-plane clipping/fill/stretch domains. Preserve partial global capture and no whole-viewport credit. No natural alias, physical reset, required-boundary, original whole-operation or global acceptance promotion.')
            database='interaction-attempts.sqlite3'if family=='clipping'else'copy-natural-attempts.sqlite3'
            q['counter_bindings']={str(ROOT/'working'/database):'working/broader-player/'+args.phase_key+'/component-'+family+'/attempts.sqlite3'}
            if family=='clipping':
                q.update(map=clip_template['map'],actor=clip_template['actor'],input_policy=dict(clip_template['input_policy'],resources='Four new task-local domains in the unchanged broader cap; preserve every historical clipping charge and refusal. Stop after an unreviewed result.'),
                    clip_oracle_sources={n:r.digest(n)for n in clip_template['clip_oracle_sources']},
                    exposure_helper=dict(path=str(helper_path.relative_to(ROOT)),sha256=artifact['sha256']),exposure_helper_build=helper_ref,
                    native_fill_replayer=fill_ref)
            cases=[]
            for item in sorted(chosen,key=lambda v:(v['configuration'],v['pair']['acquisition']['record'])):
                a=r.json(item['pair']['acquisition']['record']);software=item['configuration']=='player-software';size=a['configuration']['actual_size'];low=size[0]==640
                cid=args.phase_key+'-component-'+family+'-'+('software'if software else'default')+str(size[0])+'-01';report=namespace+cid+'-acquisition.json'
                arguments=['--report',report]if family=='clipping'else['--plan',name,'--name',cid]
                if low:arguments.append('--low')
                if software:arguments.append('--software')
                comparison=namespace+cid+('-comparison-01.json'if family=='clipping'else'-branch-review.json')
                compare=['python3','tools/check-rendering-clipping.py','--require-regions']if family=='clipping'else['python3','tools/analyze-rendering-natural-stretch.py']
                compare+=['--acquisition',report,'--report',comparison]
                cases.append(dict(id=cid,report=report,command=['python3','tools/run-broader-component.py','--plan',name,'--report',report],
                    environment={'ATSPI_DBUS_IMPLEMENTATION':'dbus-daemon'},provider='mesa'if software else'nvidia',profile=item['configuration'],size=size,reserved_original_games=1,
                    controller_file=controller,controller_arguments=arguments,historical_primary=item['pair'],comparison=comparison,comparison_command=compare))
            q['broader_cases']=cases
            preimages={}
            for source in sorted(sources):
                git=subprocess.run(['git','show','HEAD:'+source],cwd=ROOT,capture_output=True)
                if git.returncode==0 and hashlib.sha256(git.stdout).hexdigest()==sources[source]:continue
                raw=(ROOT/source).read_bytes();record=namespace+'source-'+sources[source]+'.bin';preimages[source]=dict(record=record,sha256=write_report(ROOT,record,raw))
            q['source_preimages']=preimages
            plans.append(dict(record=name,sha256=write_report(ROOT,name,(json.dumps(q,indent=2)+'\n').encode()),family=family,primary_cases=4))
        q=dict(schema=1,kind='current_broader_component_preflight',success=True,original_game_runs=0,plans=plans,reader_controls=controls_ref,
            inventory=inventory_ref,broader_plan=global_ref,primary_component_original_games=8,total_additional_primary_original_games=global_plan['prospective_total_additional_primary_starts'],
            maximum_additional_original_games=global_plan['maximum_additional_original_games'],maximum_combined_original_games=global_plan['maximum_combined_original_games'],old_execution_payloads_refreshed=False)
        h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=h,component_original_games=8,total_primary_original_games=global_plan['prospective_total_additional_primary_starts'])),flush=True)


if __name__=='__main__':main()
