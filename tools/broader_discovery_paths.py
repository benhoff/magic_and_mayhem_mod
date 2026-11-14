"""Keep current finite collectors separate from unrelated historical discovery."""
import copy
from pathlib import PurePosixPath
from rendering_pilot import require

def rebind(phase):
    q=copy.deepcopy(phase);old=q['collector_prefix'];require(old.startswith('research/runtime/')and old.endswith('/')and '..'not in PurePosixPath(old).parts,'Unsafe collector prefix')
    new=old+'continuation-43/';outputs={};cases=q['broader_cases'];require(cases and all(c['reserved_original_games']==1 for c in cases),'Only unrequested single-start contexts')
    for c in cases:
        for k in ['report','comparison','prepare_progression']:
            n=c.get(k)
            if n is None:continue
            require(isinstance(n,str)and n.startswith(old)and PurePosixPath(n).parent==PurePosixPath(old),'Output is not directly in its original collection')
            outputs[n]=new+PurePosixPath(n).name
    require(len({c['report']for c in cases})==len(cases),'Duplicate prospective acquisition')
    def adapt(v):
        if isinstance(v,dict):return{k:adapt(x)for k,x in v.items()}
        if isinstance(v,list):return[adapt(x)for x in v]
        return outputs.get(v,v)if isinstance(v,str)else v
    q=adapt(q);q['collector_prefix']=new
    require(all(q['broader_cases'][i].get('historical_primary')==c.get('historical_primary')for i,c in enumerate(cases)),'Historical primary rewritten')
    counters={}
    for origin,target in q.get('counter_bindings',{}).items():
        require(target.startswith('working/broader-player/bp40/')and '..'not in PurePosixPath(target).parts,'Unreviewed prospective counter path')
        counters[origin]=target.replace('working/broader-player/bp40/','working/broader-player/bp43/',1)
    require(counters and len(set(counters.values()))==len(counters),'Missing/aliased current counters');q['counter_bindings']=counters
    require(q.get('bounds')==phase.get('bounds'),'Inner technical budgets changed');return q
