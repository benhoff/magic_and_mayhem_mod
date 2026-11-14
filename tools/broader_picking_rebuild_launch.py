"""Observe source/paint rebuild joins without changing the frozen journey code."""
from pathlib import Path
import sys,time
from broader_picking_witness_launch import binding_class as witness_binding
from broader_picking_rebuild_join import rebuild_code,select,receipt,POLICY,TRACE_LIMIT,INPUT_LIMIT
from rendering_interaction import decode
from rendering_pilot import require

def binding_class(base):
    class RebuildBinding(witness_binding(base)):
        def __init__(self,*args,**kwargs):
            super().__init__(*args,**kwargs)
            self.rebuild_camera=set();self.rebuild_journeys=set();self.rebuild_controllers=set()
            self.rebuild_code=None;self.rebuild_trace=[];self.rebuild_joins=[];self.rebuild_pending={}
        def patch(self,module):
            module=super().patch(module);name=Path(getattr(module,'__file__','/')).name;owner=self
            if name=='run-rendering-picking-closeout.py'and id(module)not in self.rebuild_controllers:
                self.rebuild_code=rebuild_code(module.main);self.rebuild_controllers.add(id(module))
            if name=='run-rendering-camera-acceptance.py'and id(module)not in self.rebuild_camera:
                original=module.LiveViewport
                class Viewport(original):
                    def poll(self):
                        caller=sys._getframe(1);latest=super().poll()
                        if caller.f_code is not owner.rebuild_code:return latest
                        local=caller.f_locals
                        if not all(k in local for k in ('deadline','before','old_world','low','self')):return latest
                        journey=local['self'];deadline=local['deadline'];before=local['before'];now=time.monotonic()
                        if now>=deadline:return None
                        path=journey.session/'frame-input.bin'
                        require(path.stat().st_size<INPUT_LIMIT,'Rebuild input observation budget exhausted')
                        raw=path.read_bytes();require(len(raw)<INPUT_LIMIT,'Rebuild input observation budget exhausted')
                        raw=raw[:16+max(0,(len(raw)-16)//384)*384]
                        rows=decode(raw,allow_pending=True)['rows'];now=time.monotonic()
                        result=select(self.presented,rows,before,journey.size,local['old_world'],local['low'],now,deadline)
                        now=time.monotonic()
                        if now>=deadline:result=None
                        require(len(owner.rebuild_trace)<TRACE_LIMIT,'Rebuild join diagnostic budget exhausted')
                        owner.rebuild_trace.append(dict(now=now,deadline=deadline,rows=len(rows),paints=len(self.presented),
                            selected_generation=int(result[0]['world'])if result else None))
                        if result is None:return None
                        paint,row=result
                        require(id(journey)not in owner.rebuild_pending,'Unconsumed rebuild observation')
                        owner.rebuild_pending[id(journey)]=receipt(before,paint,row,local['old_world'],local['low'],deadline,now)
                        return paint
                module.LiveViewport=Viewport;self.rebuild_camera.add(id(module))
            if name=='run-rendering-interaction.py'and id(module)not in self.rebuild_journeys:
                original=module.InteractionJourney
                class Journey(original):
                    def latest(self):
                        caller=sys._getframe(1)
                        if caller.f_code is owner.rebuild_code and caller.f_locals.get('paint')is not None:
                            local=caller.f_locals;join=owner.rebuild_pending.pop(id(self),None)
                            require(join is not None and local['paint']==join['paint']
                                and local['before']['ordinal']==join['original_before_ordinal']
                                and local['deadline']==join['deadline'],'Rebuild source/paint consumption differs')
                            require(join['selected_monotonic']<join['deadline'],'Rebuild selection exceeded original deadline')
                            owner.rebuild_joins.append(join);return join['original_after']
                        return super().latest()
                module.InteractionJourney=Journey;self.rebuild_journeys.add(id(module))
            return module
        def receipt(self):
            q=super().receipt()
            require(len(self.rebuild_camera)==len(self.rebuild_journeys)==len(self.rebuild_controllers)==1,
                    'Incomplete source/paint rebuild observer binding')
            require(not self.rebuild_pending,'Unconsumed source/paint rebuild join')
            q['picking_rebuild_source_join']=dict(policy=POLICY,original_rebuild_bytecode_unchanged=True,
                original_deadline_seconds=30,physical_actions_unchanged=True,same_generation_nonce_thread_required=True,
                only_actual_swapped_paints=True,history_row_limit=4096,diagnostic_row_limit=TRACE_LIMIT,
                joins=self.rebuild_joins,observations=self.rebuild_trace)
            return q
    return RebuildBinding
