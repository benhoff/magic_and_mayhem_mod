"""Observe every actual swapped candidate while keeping all event/action deadlines."""
from pathlib import Path
from broader_picking_pose_launch import binding_class as pose_binding
from broader_picking_presented_viewport import PresentedViewport
from rendering_picking_resize import ready as original_ready
from rendering_pilot import require

def binding_class(base):
    class WitnessBinding(pose_binding(base)):
        def __init__(self,*args,**kwargs):
            super().__init__(*args,**kwargs);self.witness_viewports=[];self.witness_modules=set();self.witness_journeys=set();self.witness_controllers=set()
        def patch(self,module):
            module=super().patch(module);name=Path(getattr(module,'__file__','/')).name
            if name=='run-rendering-camera-acceptance.py'and id(module)not in self.witness_modules:
                owner=self
                class Viewport(PresentedViewport):
                    def __init__(self,*args,**kwargs):super().__init__(*args,**kwargs);owner.witness_viewports.append(self)
                module.LiveViewport=Viewport;self.witness_modules.add(id(module))
            if name=='run-rendering-interaction.py'and id(module)not in self.witness_journeys:
                original=module.InteractionJourney
                class Journey(original):
                    def mark(self,action,phase):
                        result=super().mark(action,phase)
                        if action.endswith(('-object','-cell')):
                            if phase=='released':self.viewport.arm(action,int(result['host_us']))
                            elif phase=='settled':self.viewport.finish(action)
                        return result
                module.InteractionJourney=Journey;self.witness_journeys.add(id(module))
            if name=='run-rendering-picking-closeout.py'and id(module)not in self.witness_controllers:
                require(module.ready is original_ready,'Unexpected exact picking witness predicate')
                owner=self
                def buffered_ready(rows,admissions,paint,target,baseline,kind):
                    require(len(owner.witness_viewports)==1,'Missing unique actual viewport history')
                    return owner.witness_viewports[0].match(rows,admissions,paint,target,baseline,kind)
                module.ready=buffered_ready;self.witness_controllers.add(id(module))
            return module
        def receipt(self):
            q=super().receipt();require(len(self.witness_modules)==len(self.witness_journeys)==len(self.witness_controllers)==1,'Incomplete picking witness observer binding')
            q['picking_presented_witness_collection']=dict(policy='all-presented-exact-joins-v1',
                original_ready_predicate_unchanged=True,physical_actions_unchanged=True,
                controller_eight_second_loop_unchanged=True,paint_event_deadline_us=8_000_000,
                only_actual_swapped_paints=True,history_row_limit=4096,diagnostic_row_limit=16384,
                viewport_instances=len(self.witness_viewports),windows=[w for v in self.witness_viewports for w in v.completed],
                observations=[row for v in self.witness_viewports for row in v.trace])
            return q
    return WitnessBinding
