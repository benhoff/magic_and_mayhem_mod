"""Enable the unchanged opt-in UI observer and public pause collection control."""
from pathlib import Path
import subprocess
from broader_interaction_pause import POLICY,interaction_schedule
from rendering_pilot import require

def pause_binding(base):
    class Binding(base):
        def __init__(self,*args,**kwargs):
            super().__init__(*args,**kwargs);self.pause_modules=set();self.pause_launches=[];self.pause_parent_popen=self.popen
            require(self.plan['family']=='interaction'and self.plan['interaction_pause_policy']['policy']==POLICY,'Missing explicit public pause collection policy')
        def patch(self,module):
            module=super().patch(module)
            if Path(getattr(module,'__file__','/')).name=='run-rendering-interaction.py'and id(module)not in self.pause_modules:
                original=module.InteractionJourney
                class Journey(original):
                    def __init__(self,*args,**kwargs):super().__init__(*args,**kwargs);self.pause_transitions=[]
                    interaction_schedule=interaction_schedule
                    def run(self):return dict(super().run(),public_pause_collection=self.pause_transitions)
                module.InteractionJourney=Journey;self.pause_modules.add(id(module))
            return module
        def install(self):
            binary=(self.root/self.plan['player_binary']['path']).resolve();parent=self.pause_parent_popen
            def popen(command,*args,**kwargs):
                if command and Path(str(command[0])).is_absolute()and Path(str(command[0])).resolve()==binary:
                    env=dict(kwargs['env']);require(env.get('MNM_ACCEPTANCE_UI')in(None,'1'),'Conflicting UI observer flag')
                    env['MNM_ACCEPTANCE_UI']='1';kwargs['env']=env;self.pause_launches.append(dict(flag='MNM_ACCEPTANCE_UI',value='1',existing_compiled_observer=True))
                return parent(command,*args,**kwargs)
            self.popen=popen;super().install()
        def restore(self):
            try:super().restore()
            finally:self.popen=self.pause_parent_popen;subprocess.Popen=self.pause_parent_popen
        def receipt(self):
            q=super().receipt();require(len(self.pause_modules)==len(self.pause_launches)==1,'Missing unique public pause binding/observer launch')
            q['game_actions_changed']=True;q['interaction_pause_policy']=dict(policy=POLICY,launches=self.pause_launches,actual_pause_offset='0xfd5a',
                input_mode_not_used_as_pause=True,public_Space_only=True,ordinary_target_and_selection_actions_unchanged=True,
                camera_collection_uses_paused_simulation=True,simulation_must_resume_with_eight_counters=True,Native_runtime_compiler_sources_changed=False)
            return q
    return Binding
