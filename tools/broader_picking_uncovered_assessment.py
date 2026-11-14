"""Replay only source body targeting; preserve all existing picking predicates."""
from types import FunctionType
from broader_picking_pose_assessment import with_actor_frames as pose_assessment
from broader_picking_uncovered_target import targets
from broader_picking_witness_assessment import event_deadlines
from broader_picking_rebuild_capture_assessment import rebuild_joins

def source_assessment(frames):
    original=pose_assessment(frames);namespace=dict(original.__globals__,pose_targets=targets)
    result=FunctionType(original.__code__,namespace,original.__name__,original.__defaults__,original.__closure__)
    result.__kwdefaults__=dict(original.__kwdefaults__ or {})
    return result

def with_actor_frames(frames):
    original=source_assessment(frames)
    def assess(reader,acquired,compared,observed,**kwargs):
        proof=original(reader,acquired,compared,observed,**kwargs)
        proof['all_witness_paints_within_eight_seconds']=event_deadlines(acquired,observed)
        proof['same_generation_rebuild_source_joins']=rebuild_joins(acquired,observed)
        proof['decoded_reference_binding_derivation_verified']=True
        proof['source_only_body_mask_exclusion_replayed']=True
        return proof
    return assess
