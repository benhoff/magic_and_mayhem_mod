"""Verify the decoder's derived binding before comparing raw live paint metadata."""
from broader_picking_witness_assessment import with_actor_frames as witness_assessment
from broader_picking_rebuild_assessment import rebuild_joins as raw_rebuild_joins
from rendering_pilot import require

def rebuild_joins(acquired,observed):
    paints=dict(observed['paints'])
    for rebuild in acquired['proof']['display_rebuilds']:
        raw=rebuild['confirmed_paint'];key=int(raw['paint'])
        require('reference_binding'not in raw,'Live paint unexpectedly carries decoder bookkeeping')
        paint,body=observed['paints'][key]
        require(paint['presentation_owner']=='gui'and int(paint['lease'])>0
            and type(paint.get('reference_binding'))is int and paint['reference_binding']==key,
            'Decoded GUI paint reference binding differs')
        normalized=dict(paint);del normalized['reference_binding'];paints[key]=(normalized,body)
    return raw_rebuild_joins(acquired,dict(observed,paints=paints))

def with_actor_frames(frames):
    original=witness_assessment(frames)
    def assess(reader,acquired,compared,observed,**kwargs):
        proof=original(reader,acquired,compared,observed,**kwargs)
        proof['same_generation_rebuild_source_joins']=rebuild_joins(acquired,observed)
        proof['decoded_reference_binding_derivation_verified']=True
        return proof
    return assess
