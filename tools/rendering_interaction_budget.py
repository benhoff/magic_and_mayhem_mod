"""Finite acquisition admission; failed ancestors stay explicit in repair chains."""
from rendering_pilot import require


def admit(cases,repair_of,maximum_requests,maximum_games):
    require(len(cases)<maximum_requests,'Prospective interaction request budget exhausted')
    by_name={c['report']:c for c in cases}
    require(len(by_name)==len(cases),'Duplicate interaction request')
    games=[c['result']['original_game_runs'] for c in cases]
    require(all(type(n) is int and n in (0,1) for n in games) and sum(games)<maximum_games,'Prospective original-game budget exhausted or unknown')
    def ancestors(name):
        seen=set()
        while name:
            require(name in by_name and name not in seen,'Missing/cyclic repair ancestry')
            seen.add(name);name=by_name[name]['repair_of']
        return seen
    resolved=set()
    for case in cases:
        if case['proved'] and case['repair_of']:resolved.update(ancestors(case['repair_of']))
        if case['preparation_fixed'] and case['repair_of']:resolved.add(case['repair_of'])
    repairing=ancestors(repair_of) if repair_of else set()
    for case in cases:
        if case['report'] in resolved:continue
        if case['report'] in repairing:
            require(not case['result']['success'] or case['comparison_failed'],'Repair has no retained acquisition/comparison failure')
        else:require(case['proved'],'Stop after acquisition/comparison failure or unreviewed pilot')
    return dict(requests_before=len(cases),original_games_before=sum(games),repair_ancestors=sorted(repairing),resolved_ancestors=sorted(resolved))
