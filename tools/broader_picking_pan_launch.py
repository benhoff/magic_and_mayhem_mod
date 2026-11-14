"""Change only pan release observation; preserve source target and capture contracts."""
from pathlib import Path
from broader_picking_uncovered_launch import binding_class as uncovered_binding
from broader_picking_pan_ack import PAN_ACTIONS,POLICY,perform
from rendering_pilot import require

def binding_class(base):
    class PanBinding(uncovered_binding(base)):
        def __init__(self,*args,**kwargs):
            super().__init__(*args,**kwargs);self.pan_modules=set();self.pan_journeys=[]
        def patch(self,module):
            module=super().patch(module)
            if Path(getattr(module,'__file__','/')).name=='run-rendering-interaction.py'and id(module)not in self.pan_modules:
                original=module.InteractionJourney;owner=self
                class Journey(original):
                    def __init__(self,*args,**kwargs):
                        super().__init__(*args,**kwargs);self.pan_receipts=[];owner.pan_journeys.append(self)
                    def held_key(self,action,symbol,hold=.18,settle=2.5,advance=True):
                        if action in PAN_ACTIONS:return perform(self,action,symbol,hold,settle,advance)
                        return super().held_key(action,symbol,hold,settle,advance)
                module.InteractionJourney=Journey;self.pan_modules.add(id(module))
            return module
        def receipt(self):
            q=super().receipt();require(len(self.pan_modules)==1,'Missing pan observer binding')
            q['picking_source_pan_ack']=dict(policy=POLICY,physical_key_identities_and_order_unchanged=True,
                hold_release_policy_changed=True,minimum_hold_seconds=.22,maximum_hold_plus_settle_seconds=1.02,
                settlement_uses_remaining_existing_budget=True,source_generation_nonce_thread_required=True,
                changed_camera_same_view_required=True,original_five_World_thirty_second_progress_wait_unchanged=True,
                original_pan_analysis_predicate_unchanged=True,journey_instances=len(self.pan_journeys),
                attempts=[v for j in self.pan_journeys for v in j.pan_receipts])
            return q
    return PanBinding
