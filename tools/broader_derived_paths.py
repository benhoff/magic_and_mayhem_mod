"""Move only prospective continuation47 outputs and counters to continuation48."""
import copy
from pathlib import PurePosixPath
from rendering_pilot import require


def rebind(phase):
    q=copy.deepcopy(phase);old=q['collector_prefix']
    require(old.startswith('research/runtime/')and old.endswith('/continuation-47/')
            and '..'not in PurePosixPath(old).parts,'Expected exact prior continuation47 namespace')
    new=old.removesuffix('continuation-47/')+'continuation-48/';outputs={};cases=q['broader_cases']
    require(cases and all(c['reserved_original_games']==1 for c in cases),'Only unrequested single-start contexts')
    for c in cases:
        for key in('report','comparison','prepare_progression'):
            value=c.get(key)
            if value is None:continue
            require(isinstance(value,str)and value.startswith(old)and PurePosixPath(value).parent==PurePosixPath(old),
                    'Prospective output outside exact collection')
            outputs[value]=new+PurePosixPath(value).name
    require(len({c['report']for c in cases})==len(cases),'Duplicate prospective acquisition')
    def adapt(value):
        if isinstance(value,dict):return{k:adapt(v)for k,v in value.items()}
        if isinstance(value,list):return[adapt(v)for v in value]
        return outputs.get(value,value)if isinstance(value,str)else value
    q=adapt(q);q['collector_prefix']=new;counters={}
    for origin,target in q.get('counter_bindings',{}).items():
        require(target.startswith('working/broader-player/bp47/')and '..'not in PurePosixPath(target).parts,
                'Unexpected prior counter namespace')
        counters[origin]=target.replace('working/broader-player/bp47/','working/broader-player/bp48/',1)
    require(counters and len(counters)==len(set(counters.values())),'Missing/aliased prospective counters')
    q['counter_bindings']=counters
    require(q.get('bounds')==phase.get('bounds')and all(q['broader_cases'][i].get('historical_primary')==c.get('historical_primary')for i,c in enumerate(cases)),
            'Historical input or inner limit changed')
    return q
