"""Bind explicit collector --plan arguments to their declared current phase."""
from pathlib import Path
from rendering_pilot import require
LEGACY=dict(caller='research/runtime/caller-state-installed-results-20261108/bp16/world-caller/input-plan-01.json',font='research/runtime/font-dc-lifetime-results-20261108/bp16/world-font/input-plan-01.json',palette='research/runtime/palette-cycling-results-20261108/bp16/world-palette/input-plan-01.json')
CONTROLLER_RAM={'caller':'mnm-caller-capture-','picking':'mnm-picking-capture-'}
def bind_arguments(arguments,old_phase,new_phase,family):
 require(isinstance(arguments,list)and all(isinstance(v,str)for v in arguments),'Invalid collector arguments')
 for name in(old_phase,new_phase):require(name.startswith('research/runtime/')and not Path(name).is_absolute()and '..'not in Path(name).parts,'Unbound phase path')
 require(arguments.count('--plan')<=1 and '--input-plan'not in arguments,'Ambiguous delegated plan selector')
 result=list(arguments);proof=None
 if '--plan'in result:
  i=result.index('--plan');require(i+1<len(result),'Missing delegated plan value');previous=result[i+1];require(previous in {old_phase,new_phase,LEGACY.get(family)},'Unknown delegated plan identity');result[i+1]=new_phase;proof=dict(previous=previous,current=new_phase,only_plan_identity_changed=True)
 return result,proof

def capture_backing(family,previous):
 result=dict(previous)
 if family in CONTROLLER_RAM:result=dict(enabled=False,policy='unchanged-controller-owned-tmpfs-v1',controller_RAM_prefix=CONTROLLER_RAM[family],raw_SQLite_import_required=True,paired_real_restore_required=True,physical_source_held_after_validation=True)
 return result
