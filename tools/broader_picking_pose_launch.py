"""Enable the existing passive sidecar and bind its exact source generation."""
from pathlib import Path
import subprocess
from broader_picking_pose_source import LiveActorFrames
from broader_picking_pose_target import PosedRows,bound_targets,POLICY
from rendering_picking_resize import targets as original_targets
from rendering_pilot import require


def binding_class(base):
    class PoseBinding(base):
        def __init__(self,*args,**kwargs):
            super().__init__(*args,**kwargs)
            require(self.plan['family']=='picking','Pose target binding requires picking family')
            self.pose_journeys=set();self.pose_controllers=set();self.pose_launches=[]
            self.pose_original_popen=self.popen

        def patch(self,module):
            module=super().patch(module);name=Path(getattr(module,'__file__','/')).name
            if name=='run-rendering-interaction.py'and id(module)not in self.pose_journeys:
                original=module.InteractionJourney
                class ObservedJourney(original):
                    def __init__(self,*args,**kwargs):
                        super().__init__(*args,**kwargs);self.picking_pose_reader=None
                    def ready_target(self):
                        state,body=super().ready_target();generation=state['frame']['generation']
                        if self.picking_pose_reader is None:
                            paths=list(self.session.rglob('scene-actors-0001.bin'))
                            require(len(paths)==1,'Missing unique passive picking actor sidecar')
                            self.picking_pose_reader=LiveActorFrames(paths[0])
                        self.picking_pose_reader.poll(generation)
                        frame=self.picking_pose_reader.frames.get(generation)
                        require(frame is not None and str(frame['nonce'])==state['frame']['nonce']
                                and frame['thread']==state['thread']and frame['view']==state['camera'][0]
                                and frame['extent']==list(self.size),'Picking actor/source generation identity differs')
                        self.source_frames.frames[generation]=PosedRows(self.source_frames.frames[generation],frame)
                        return state,body
                module.InteractionJourney=ObservedJourney;self.pose_journeys.add(id(module))
            if name=='run-rendering-picking-closeout.py':
                if id(module)not in self.pose_controllers:
                    require(module.targets is original_targets,'Unexpected picking target callable')
                    module.targets=bound_targets;self.pose_controllers.add(id(module))
                else:require(module.targets is bound_targets,'Picking pose target binding changed')
            return module

        def install(self):
            parent=self.pose_original_popen;binary=(self.root/self.plan['player_binary']['path']).resolve()
            def popen(command,*args,**kwargs):
                direct=command and Path(str(command[0])).is_absolute()and Path(str(command[0])).resolve()==binary
                if direct:
                    env=dict(kwargs['env']);require(env.get('MNM_SCENE_ACTORS')in(None,'1'),'Conflicting picking actor observer flag')
                    env['MNM_SCENE_ACTORS']='1';kwargs['env']=env
                    self.pose_launches.append(dict(flag='MNM_SCENE_ACTORS',value='1',existing_compiled_observer=True))
                return parent(command,*args,**kwargs)
            self.popen=popen;super().install()

        def restore(self):
            try:super().restore()
            finally:subprocess.Popen=self.pose_original_popen;self.popen=self.pose_original_popen

        def receipt(self):
            q=super().receipt()
            require(len(self.pose_journeys)==len(self.pose_controllers)==len(self.pose_launches)==1,
                    'Missing or duplicate picking source observer binding')
            q['game_actions_changed']=True
            q['picking_pose_target']=dict(policy=POLICY,launches=self.pose_launches,actor_wire_version=2,
                same_generation_nonce_thread_required=True,exact_record_and_body_CRC_required=True,
                Native_runtime_sources_changed=False,original_memory_writes=False,hit_feedback_search=False,
                quiet_source_samples=3,exact_displayed_witnesses=3,readiness_seconds=8,
                all48_windows_and_two_rebuilds_required=True,normal_retirement_required=True)
            return q
    return PoseBinding
