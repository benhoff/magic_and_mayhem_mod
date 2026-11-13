"""Preserve complete historical rows while attaching exact current replacements."""
from coverage_claims import fingerprint
from rendering_pilot import require
PROFILES=('player-default','player-software')
def expected_cells(closure):
 journeys={(p,'journey.'+j['id']+'.'+leaf)for p in PROFILES for j in closure['journeys']for leaf in j['requirements']}
 operations={(p,'operation.'+o['id'])for p in PROFILES for o in closure['operations']}
 require(len(closure['journeys'])==14 and len(journeys)==94 and len(closure['operations'])==21 and len(operations)==42,'Authored acceptance denominator changed')
 return journeys,operations

def exact_rows(rows,expected):
 keys=[(v['configuration'],v['requirement'])for v in rows]
 require(len(keys)==len(set(keys))and set(keys)==expected,'Missing, duplicate or foreign requirement row')

def primary_ref(execution,journey):
 value=execution['primary']if journey else execution['execution']
 return value if'record'in value else value['acquisition']

def join_rows(rows,expected,replacements,parent,*,journey,controlled_overlap=None):
 exact_rows(rows,expected);result=[]
 for index,row in enumerate(rows):
  profile=row['configuration'];requirement=row['requirement'];attachments=[]
  if requirement=='operation.copy-alias-overlap':
   require(not journey and 'executions'not in row and controlled_overlap is not None,'Controlled overlap cannot replace a natural execution')
   require(row['natural_player_reachability']=='pending'and row['controlled_mechanism']['natural_player_call_proved']is False and row['controlled_mechanism']['actual_player_domains']==0,'Unexpected natural-overlap promotion')
   require(controlled_overlap['verified']is True and controlled_overlap['natural_player_call_proved']is False and controlled_overlap['approved_amendment']=='ALIAS09.controlled-opaque-overlap-v1','Controlled overlap scope changed')
   attachments=[dict(role='unchanged-owner-approved-controlled-mechanism',current_control=controlled_overlap,natural_player_call_proved=False)]
  else:
   require(row['executions'],'Empty primary execution list')
   for ordinal,execution in enumerate(row['executions']):
    old=primary_ref(execution,journey);key=(old['record'],old['sha256']);require(key in replacements,'Unreplaced historical primary: '+old['record']);new=replacements[key]
    require(new['historical_primary']==old and new['profile']==profile and new['success']is True and new['sources_stable']is True and new['current_native_phase']['current_native_source_candidate']is True,'Mixed, stale or failed current primary')
    require(new['accepted']is True and new['role']in('world','menu','component','fault'),'Unadmitted or unknown current primary')
    if journey:require(new['role']in('world','menu'),'Selected planes or failed epochs cannot establish a normal journey')
    if new['role']=='component':require(new['whole_viewport_credit']is False,'Selected planes acquired false viewport credit')
    attachments.append(dict(historical_primary=old,historical_execution_index=ordinal,historical_execution_sha256=fingerprint(execution),current_validation=new['validation'],current_acquisition=new['acquisition'],role=new['role'],semantic_binding=new['semantic_binding']))
  result.append(dict(configuration=profile,requirement=requirement,parent_row=dict(parent=parent,index=index,sha256=fingerprint(row)),attachments=attachments,parent_row_preserved_exactly=True))
 return result
