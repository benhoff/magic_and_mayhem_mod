"""Interpret the current empty preimage sequence for the legacy mapping reader."""
from rendering_pilot import require

def caller_preimages(plan):
    value=plan.get('source_preimages',{})
    require(isinstance(value,dict)or value==[],'Nonempty or unknown caller source-preimage schema')
    if isinstance(value,dict):return plan,False
    result=dict(plan);result['source_preimages']={}
    return result,True
