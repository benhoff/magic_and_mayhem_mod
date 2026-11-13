"""Derive the frozen recovery report map from the actual bounded declarations."""
from copy import deepcopy
from pathlib import PurePosixPath
from rendering_pilot import require

def rebind(phase):
    result=deepcopy(phase)
    if 'execution_paths' not in result:return result
    family=result['family'];require(family in ('loss','transition'),'Unknown recovery collection')
    cases=result['broader_cases'];require(len(cases)==4,'Four recovery contexts required')
    reports={}
    for case in cases:
        command=case['command'];report=case['report'];software=case['profile']=='player-software'
        require(case['size'] in ([640,480],[800,600]) and case['reserved_original_games']==2,'Wrong recovery extent/charge')
        require(command.count('--report')==1 and command[command.index('--report')+1]==report,'Unbound report argument')
        require(('--software'in command)==software and ('--low'in command)==(case['size']==[640,480]),'Recovery flags differ from context')
        require(report not in reports and not PurePosixPath(report).is_absolute()and '..'not in PurePosixPath(report).parts,'Unsafe/duplicate report')
        reports[report]=dict(software=software,size=case['size'])
    require({(v['software'],tuple(v['size']))for v in reports.values()}=={(s,z)for s in (False,True)for z in [(640,480),(800,600)]},'Incomplete recovery domain')
    base='working/broader-player/bp41/'+family
    result['execution_paths']=dict(schema=1,output_root=base+'/acceptance',budget=base+'/attempts.sqlite3',reports=reports)
    result['pilot']=cases[0]['report']
    result['counter_bindings']=dict(result.get('counter_bindings',{}),recovery_collection_budget=base+'/attempts.sqlite3')
    return result
