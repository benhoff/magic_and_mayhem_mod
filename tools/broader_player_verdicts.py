"""Issue a finite current-player verdict while preserving original recovery exactly."""
import collections,copy
from native_render_driver import require_nvidia
from rendering_pilot import require
PROFILES=('player-default','player-software')
ROLES=dict(world=56,menu=12,component=8,fault=8,strict=4,boundary=2)
def issue(current,parent,prior):
 require(current['kind']=='broader_current_public_player_acceptance'and current['success']is True and current['sources_stable']is True and current['current_whole_player_proved']is True and current['supported_player_accepted']is True and current['original_recovery_complete']is False,'A complete current execution conjunction is required')
 require(current['current_native_sources_verified_without_waiver']and current['current_cases']==90 and current['current_source_original_game_starts']==98 and current['journey_cells']==94 and current['operation_cells']==42 and current['ignored_pixels']==0 and current['all_required_output_pixels_compared']and not current['original_padding_equivalence']and not current['arbitrary_maps_saves_durations_supported'],'Incomplete or widened current acceptance')
 require(current['historical_failures_retired']is False and current['physical_driver_reset_proved']is False and current['natural_player_overlap_domains']==0,'Historical or physical scope promotion is forbidden')
 cases=current['actual_cases'];require(len(cases)==len({v['id']for v in cases})==90 and collections.Counter(v['role']for v in cases)==ROLES and all(v['exact_current_native_source_candidate']for v in cases),'Missing, duplicated or stale current execution')
 require(sum(2 if v['role']=='fault'else 1 for v in cases)==98,'Changed original-start denominator')
 require(parent['success']is True and parent['passed_requirements']==156 and parent['pending_requirements']==0 and len(parent['requirements'])==156 and all(v['state']=='passed'for v in parent['requirements']),'Required finite boundaries remain pending')
 require(current['scope_amendments']==parent['scope_amendments']==prior['approved_scope_amendments'],'Approved support amendment changed')
 require(prior['success']is True and prior['original_recovery_complete']is False,'Unreviewed original recovery basis')
 original=[v for v in prior['verdicts']if v['id']=='original-recovery'];require(len(original)==1,'Missing original-recovery verdict');original=copy.deepcopy(original[0])
 require(original['issued']is True and original['state']=='pending'and original['accepted']is False and len(original['contract_groups'])==15 and len(original['historical_scope_review'])==8 and {v['configuration']for v in original['profiles']}==set(PROFILES)and all(v['state']=='pending'and v['accepted']is False for v in original['profiles']),'Original recovery must retain every independent pending scope')
 profiles=[]
 for profile in PROFILES:
  subset=[v for v in cases if v['profile']==profile];require(len(subset)==45,'Missing current profile cases')
  drivers=[d for v in subset for d in v['actual_drivers']]
  require(all(v['actual_drivers']for v in subset),'Missing observed provider')
  if profile=='player-default':require_nvidia(drivers)
  else:require(all(d.lower().startswith(('llvmpipe','softpipe'))for d in drivers),'Missing actual software provider')
  profiles.append(dict(configuration=profile,state='accepted',accepted=True,current_cases=len(subset),actual_drivers=sorted(set(drivers)),journey_cells=47,operation_cells=21,original_starts=sum(2 if v['role']=='fault'else 1 for v in subset)))
 player=dict(id='supported-player',issued=True,state='accepted',accepted=True,profiles=profiles,supported_input_domain=current['supported_input_domain'],required_boundary_impact=current['required_boundary_impact'],historical_failure_relevance=current['historical_failure_relevance'],historical_failures_retired=False,original_padding_equivalence=False,physical_driver_reset_proved=False,natural_alias_reachability_proved=False,reason='The exact current finite public-player conjunction supplies every required journey/operation attachment, independent output/state, response/progress/resource checks, declared failed/fresh pair, strict readiness and actual software/NVIDIA provider boundary. The declared native finite-edge policy and all original-supplier/input/map/save/duration limits remain explicit. Historical failures remain immutable and their current finite relevance is separately reviewed.',scope=current['scope'])
 return dict(decisions={'supported-player':'accepted','original-recovery':'pending'},verdicts=[player,original],original_recovery_verdict_preserved_exactly=original==next(v for v in prior['verdicts']if v['id']=='original-recovery'),supported_player_accepted=True,original_recovery_complete=False,current_whole_player_proved=True)
