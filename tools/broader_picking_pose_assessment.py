"""Retain every stopped picking predicate; change only target-source replay."""
import inspect
import rendering_picking_resize_assessment as previous
from broader_picking_pose_target import targets
from rendering_pilot import require


def with_actor_frames(frames):
    original=inspect.getsource(previous.assess)
    old="body,foot=targets(source_rows,state['history'],state['position'])"
    new="body,foot=pose_targets(source_rows,state['history'],state['position'],actor_frames[generation])"
    require(original.count(old)==1,'Unexpected frozen picking target replay')
    source=original.replace(old,new)
    require(source.replace(new,old)==original,'Unexpected picking assessment predicate change')
    namespace=dict(previous.assess.__globals__);namespace.update(pose_targets=targets,actor_frames=frames)
    exec(compile(source,__file__,'exec'),namespace)
    return namespace['assess']
