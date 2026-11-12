"""Conservative two-generation request admission; all failed ancestors remain visible."""
from rendering_pilot import require

def admit(cases,repair_of,maximum_requests,maximum_games):
 require(len(cases)<maximum_requests,'Prospective recovery request budget exhausted')
 by_name={v['report']:v for v in cases};require(len(by_name)==len(cases),'Duplicate recovery request')
 games=[v['result']['original_game_runs'] for v in cases]
 require(all(type(v) is int and v in (0,1,2) for v in games) and sum(games)+2<=maximum_games,'Prospective original-start budget exhausted or unknown')
 def ancestors(name):
  seen=set()
  while name:
   require(name in by_name and name not in seen,'Missing/cyclic recovery repair ancestry');seen.add(name);name=by_name[name]['repair_of']
  return seen
 resolved=set()
 for v in cases:
  if v['proved'] and v['repair_of']:resolved.update(ancestors(v['repair_of']))
 repairing=ancestors(repair_of) if repair_of else set()
 for v in cases:
  if v['report'] in resolved:continue
  if v['report'] in repairing:require(not v['result']['success'] or v['comparison_failed'],'Repair has no retained failure')
  else:require(v['proved'],'Stop after unreviewed recovery acquisition/comparison')
 return dict(requests_before=len(cases),original_starts_before=sum(games),reserved_original_starts=2,repair_ancestors=sorted(repairing),resolved_ancestors=sorted(resolved))
