"""Bind current source targets without changing the interaction action schedule."""
from pathlib import Path
from broader_interaction_recent_sources import RecentSources,fresh_target,POLICY
from rendering_pilot import require

def recent_binding(base):
    class Binding(base):
        def __init__(self,*args,**kwargs):
            super().__init__(*args,**kwargs);self.recent_modules=set()
            require(self.plan['family']=='interaction'and self.plan['recent_interaction_sources']['policy']==POLICY,'Missing recent source-target declaration')
        def patch(self,module):
            module=super().patch(module)
            if Path(getattr(module,'__file__','/')).name=='run-rendering-interaction.py'and id(module)not in self.recent_modules:
                original=module.InteractionJourney
                class Journey(original):
                    def __init__(self,*args,**kwargs):
                        super().__init__(*args,**kwargs);self.source_frames.close();self.source_frames=RecentSources(self.out/'worlds.bin')
                    ready_target=fresh_target
                module.InteractionJourney=Journey;self.recent_modules.add(id(module))
            return module
        def receipt(self):
            result=super().receipt();require(len(self.recent_modules)==1,'Missing unique current source-target binding')
            result['recent_interaction_sources']=dict(policy=POLICY,maximum_age_frames=2,selected_opaque_pixel_formula_unchanged=True,
                prefix_dependencies_decoded_and_hashed=True,materialized_only_requested_World=True,action_schedule_changed=False,
                old_15second_readiness_deadline_preserved=True,hit_feedback_search=False,Native_runtime_sources_changed=False)
            return result
    return Binding
