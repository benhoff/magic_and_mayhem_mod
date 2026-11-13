"""Select the reviewed explicit picking-repair ancestry, preserving all bounds."""
import copy
from broader_player_picking_target_matrix import P,BINARY,requests,current,replacements,replacement
from broader_player_picking_target_matrix import specs as previous_specs
from broader_player_acceptance import checked
from rendering_pilot import require
PILOT='bp35-world-picking-default800-01'
REVIEW=P+'picking-target-repair-ancestry-review-01.json'


def verify_case(previous,actual,old_phase,new_phase,failed,reason):
    expected=copy.deepcopy(previous)
    def substitute(value):
        if isinstance(value,dict):return {k:substitute(v)for k,v in value.items()}
        if isinstance(value,list):return [substitute(v)for v in value]
        return new_phase if value==old_phase else value
    expected=substitute(expected)
    if previous['id']==PILOT:
        require('--repair-of'not in expected['controller_arguments']and '--repair-reason'not in expected['controller_arguments'],
                'Picking repair already bound')
        expected['controller_arguments']+=['--repair-of',failed,'--repair-reason',reason]
    require(actual==expected,'Picking repair changed more than explicit ancestry/phase identity')
    return True


def specs(reader):
    old=previous_specs(reader)
    review=checked(reader,dict(record=REVIEW,sha256=reader.digest(REVIEW)))
    require(review['kind']=='reviewed_picking_explicit_repair_ancestry'and review['original_game_runs']==0
            and review['request_rows']==120 and review['conservative_charges']==160
            and review['budget_changed']is False and review['case_ids_changed']is False,
            'Changed picking repair amendment')
    require(all(reader.digest(n)==h for n,h in review['sources'].items()),'Picking repair reader changed')
    phase=checked(reader,review['replacement_phase']);replacement={v['id']:v for v in phase['broader_cases']}
    out=[];count=0
    for spec in old:
        if spec['plan']==review['previous_phase']['record']:
            require(reader.digest(spec['plan'])==review['previous_phase']['sha256'],'Original prospective phase changed')
            case=replacement[spec['case']['id']]
            verify_case(spec['case'],case,spec['plan'],review['replacement_phase']['record'],review['failed_acquisition']['record'],review['repair_reason'])
            spec=dict(spec,plan=review['replacement_phase']['record'],case=case);count+=1
        out.append(spec)
    require(count==len(replacement)==2 and len(out)==90,'Changed picking repair context domain')
    return out
