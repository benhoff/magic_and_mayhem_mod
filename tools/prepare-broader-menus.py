#!/usr/bin/env python3
"""Freeze fresh current-player menu/movie cases inside the existing global cap."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

from broader_player_sources import python_sources
from coverage_evidence import Reader, write_report
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
FAMILIES={'menu':('rendering-main-setup-results-20261106','menu-attempts.sqlite3'),
          'intros':('rendering-enabled-intros-results-20261107','enabled-intros-attempts.sqlite3'),
          'skip':('rendering-movie-skip-results-20261106','movie-skip-attempts.sqlite3')}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',required=True)
    parser.add_argument('--global-plan',default=P+'plan-02.json')
    parser.add_argument('--basis',default='research/runtime/surface-loss-restore-results-20261109/bp10-input-plan-01.json')
    parser.add_argument('--phase-key',default='bp10')
    parser.add_argument('--capture-ram',action='store_true')
    parser.add_argument('--native-controls',default=P+'native-controls-01.json')
    parser.add_argument('--hardware-controls',default=P+'hardware-host-controls-01.json')
    args=parser.parse_args()
    require(args.phase_key.isalnum()and args.phase_key.startswith('bp'),'Safe phase key required')
    require(args.report.startswith(P) and not(ROOT/args.report).exists(),'Fresh current menu preflight required')
    tests=subprocess.run([sys.executable,'tests/test-broader-player-launch.py'],cwd=ROOT,capture_output=True)
    log=tests.stdout+tests.stderr
    require(tests.returncode==0,'Current launch binding controls failed')
    with Reader(ROOT)as r:
        global_name=args.global_plan;global_plan=r.json(global_name)
        native_name=args.native_controls;hardware_name=args.hardware_controls
        for name in (native_name,hardware_name):
            control=r.json(name)
            require(control['success']and control['sources_stable']and all(r.digest(n)==h for n,h in control['sources'].items()),'Current native/hardware controls changed')
        basis=r.json(args.basis)
        require(all(r.digest(n)==h for n,h in (basis['compiler_sources']|basis['runtime_sources']).items()),'Current linked player source changed')
        require(hashlib.sha256((ROOT/basis['player_binary']['path']).read_bytes()).hexdigest()==basis['player_binary']['sha256'],'Current player binary changed')
        names=python_sources(ROOT,['tools/run-broader-menu.py'])|{'tools/broader_player_sources.py','tests/test-broader-player-launch.py'}
        sources={n:r.digest(n)for n in sorted(names)}
        control_name=P+'menu-launch-controls-'+args.phase_key+'.json'
        control=dict(schema=1,kind='current_broader_menu_launch_controls',success=True,sources_stable=True,
            original_game_runs=0,test_count=7,sources={n:sources[n]for n in('tools/broader_player_launch.py','tools/broader_native_host.py','tests/test-broader-player-launch.py')})
        log_name=control_name.removesuffix('.json')+'-log.bin';control['log']=dict(record=log_name,sha256=write_report(ROOT,log_name,log))
        control_ref=dict(record=control_name,sha256=write_report(ROOT,control_name,(json.dumps(control,indent=2)+'\n').encode()))
        plans=[]
        for family,(directory,counter)in FAMILIES.items():
            prefix='research/runtime/'+directory+'/'+args.phase_key+'/'
            name=prefix+'input-plan-01.json'
            require(not(ROOT/name).exists(),'Preserve prior menu phase')
            cases=[]
            for software in(False,True):
                for low in(True,False):
                    size=[640,480]if low else[800,600]
                    cid=args.phase_key+'-'+family+'-'+('software'if software else'default')+str(size[0])+'-01'
                    report=prefix+cid+'-acquisition.json'
                    command=['python3','tools/run-broader-menu.py','--kind',family,'--plan',name,'--report',report]
                    if software:command.append('--software')
                    if low:command.append('--low')
                    if family=='skip':command+=['--skip-index',str(int(low))]
                    cases.append(dict(id=cid,report=report,command=command,
                        environment={'ATSPI_DBUS_IMPLEMENTATION':'dbus-daemon'},provider='mesa'if software else'nvidia',
                        profile='player-software'if software else'player-default',size=size,reserved_original_games=1))
            q=dict(schema=1,kind='current_broader_menu_prospective_plan',success=True,original_game_runs=0,
                family=family,collector_prefix=prefix,capture_storage=dict(enabled=args.capture_ram,policy='owned-tmpfs-diagnostic-capture-exact-copy-v1'),broader_plan=dict(record=global_name,sha256=r.digest(global_name)),
                current_controls=dict(record=native_name,sha256=r.digest(native_name)),
                hardware_controls=dict(record=hardware_name,sha256=r.digest(hardware_name)),launch_controls=control_ref,
                sources=sources,compiler_sources=basis['compiler_sources'],runtime_sources=basis['runtime_sources'],
                player_binary=basis['player_binary'],oracle=basis['oracle'],broader_cases=cases,
                bounds=dict(primary_cases=4,maximum_attempts=6,maximum_requests=6,maximum_original_games=5,
                    stop_on_first_failure=True,automatic_retries=False,pilot_comparison_before_expansion=True),
                counter_bindings={str(ROOT/'working'/counter):'working/broader-player/'+args.phase_key+'/'+family+'/attempts.sqlite3'},
                authorization='Fresh broader acceptance work requested by do 2.; each task-local case counts in its exact bound global cumulative plan. All historical caps, starts and failures remain retained.',
                scope='Actual current NVIDIA/default and Mesa/software menu/media contexts at both original resolution selectors; native Qt widget policy stays semantic. Full independent movie output is required only for the enabled-intros family. No World, physical scanout or global acceptance claim.')
            unique=[]
            for source in ('tools/broader_player_launch.py','tools/run-broader-menu.py','tools/check-broader-menu.py','tools/prepare-broader-menus.py','tests/test-broader-player-launch.py'):
                raw=(ROOT/source).read_bytes();record=prefix+'source-'+r.digest(source)+'.bin'
                unique.append(dict(path=source,record=record,sha256=write_report(ROOT,record,raw)))
            q['source_preimages']=unique
            if family=='menu':
                policy=dict(schema=1,kind='fresh_broader_menu_counter_namespace',success=True,original_game_runs=0,
                    preparation_refusals=[],historical_requests_excluded_or_deleted=False,
                    reason='Compatibility filename required by unchanged collector; this new owned namespace has no historical preparation exclusions. Every actual new attempt is charged globally and locally.')
                write_report(ROOT,prefix+'staging-repair-02.json',(json.dumps(policy,indent=2)+'\n').encode())
            plans.append(dict(record=name,sha256=write_report(ROOT,name,(json.dumps(q,indent=2)+'\n').encode()),family=family))
        q=dict(schema=1,kind='current_broader_menu_preflight',success=True,original_game_runs=0,
            plans=plans,launch_controls=control_ref,maximum_primary_original_games=12,
            original_game_launched_by_preflight=False,global_bounds_unchanged=True)
        h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode())
        print(json.dumps(dict(success=True,sha256=h,plans=plans)),flush=True)


if __name__=='__main__':main()
