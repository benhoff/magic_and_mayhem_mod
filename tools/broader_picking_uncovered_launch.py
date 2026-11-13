"""Bind source-only unoccluded body targets within the existing quiet-source loop."""
from pathlib import Path
from broader_picking_rebuild_launch import binding_class as rebuild_binding
from broader_picking_pose_target import bound_targets as original_targets
from broader_picking_uncovered_target import bound_targets as uncovered_targets,POLICY
from rendering_interaction_live import SpriteNotReady
from picking_closeout import quiet_sources as original_quiet
from rendering_pilot import require

def quiet_sources(samples):
    return all(all(v>=0 for v in s['point'])for s in samples)and original_quiet(samples)

def binding_class(base):
    class UncoveredBinding(rebuild_binding(base)):
        def __init__(self,*args,**kwargs):
            super().__init__(*args,**kwargs);self.uncovered_controllers=set();self.uncovered_pending_samples=0
        def patch(self,module):
            name=Path(getattr(module,'__file__','/')).name
            if name=='run-rendering-picking-closeout.py'and id(module)in self.uncovered_controllers:
                require(module.targets is self.uncovered_callable and module.quiet_sources is quiet_sources
                    and module.ready is self.uncovered_ready,'Uncovered controller binding changed')
                return module
            module=super().patch(module)
            if name=='run-rendering-picking-closeout.py':
                require(module.targets is original_targets and module.quiet_sources is original_quiet,
                        'Unexpected original source target or quiet-source predicate')
                owner=self
                def pending_targets(records,history,position):
                    try:return uncovered_targets(records,history,position)
                    except SpriteNotReady as error:
                        require(str(error)=='No source-only player body pixel outside other creature masks',
                                'Unexpected source-target readiness refusal')
                        require(owner.uncovered_pending_samples<16384,'Uncovered source readiness budget exhausted')
                        owner.uncovered_pending_samples+=1;body,cell=original_targets(records,history,position)
                        return dict(body,point=[-1,-1],source_only_target_available=False),cell
                module.targets=pending_targets;module.quiet_sources=quiet_sources
                self.uncovered_callable=pending_targets;self.uncovered_ready=module.ready
                self.uncovered_controllers.add(id(module))
            return module
        def receipt(self):
            q=super().receipt();require(len(self.uncovered_controllers)==1,'Missing unique uncovered body binding')
            q['picking_uncovered_body_target']=dict(policy=POLICY,source_actor_slot=0,
                only_captured_creature_opacity_masks=True,all_other_body_masks_excluded_without_depth_assumptions=True,
                original_hit_feedback_used=False,Native_or_Original_destination_pixels_used=False,
                cell_ANI_policy_unchanged=True,physical_route_unchanged=True,original_quiet_deadline_seconds=8,
                original_witness_deadline_seconds=8,unavailable_source_samples=self.uncovered_pending_samples,
                unavailable_sample_limit=16384,source_sprite_cache_limit=64)
            return q
    return UncoveredBinding
