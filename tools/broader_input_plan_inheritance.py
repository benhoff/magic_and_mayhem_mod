"""Resolve the one retained incomplete plan through its exact pinned parent."""
import hashlib,re
from pathlib import Path
from broader_player_acceptance import checked
from fault_lifetime_gate import Join
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
INCOMPLETE=dict(record=P+'plan-14.json',sha256='ccbdbe77326fed6086bfff95f63e062357275ade96009a95e0b1431968ba44d5')
PARENT=dict(record=P+'plan-13.json',sha256='7dbe8ce389c0652805394c08ace62b4aa2873b9cf1a27306037d767976fb0e79')

def resolve(reader,reference):
 plan=checked(reader,reference)
 if 'owned_input'in plan:return plan['owned_input'],dict(inherited=False,global_plan=reference)
 require(reference==INCOMPLETE and plan['previous_plan']==PARENT and plan['kind']=='broader_current_player_drain_continuation_plan','Unreviewed missing input reference')
 parent=checked(reader,PARENT);fixture=parent['owned_input'];checked(reader,fixture)
 return fixture,dict(inherited=True,incomplete_plan=reference,pinned_parent=PARENT,field='owned_input',immutable_reports_changed=False,input_domain_widened=False)

def input_binding(reader,acquired):
 phase=checked(reader,acquired['prospective_plan']);fixture_ref,inheritance=resolve(reader,phase['broader_plan']);fixture=checked(reader,fixture_ref);initial=fixture['input_fingerprints'];actual=acquired['installation_inputs'];allowed=set(fixture['permitted_mutations'])
 require(actual.keys()==initial.keys()and all(actual[n]==v for n,v in initial.items()if n not in allowed),'Undeclared game input change')
 wanted=b'TRUE'if acquired['configuration']['actual_size']==[800,600]else b'FALSE';join=Join(reader);configs=[]
 for epoch in acquired['proof']['epochs']:
  path=join.at(acquired,epoch['root'],'game/CFG/prefs.cfg');raw=join.raw(acquired,path);require(re.findall(rb'(?mi)^IsHighRes\s*=\s*(TRUE|FALSE)',raw)==[wanted],'Staged original preference differs from the actual World domain');configs.append(dict(path=path,sha256=hashlib.sha256(raw).hexdigest()))
 return dict(verified=True,fixture=fixture_ref,exact_input_count=len(actual),permitted_source_mutations=sorted(allowed),actual_staged_preferences=configs,plan_input_resolution=inheritance)
