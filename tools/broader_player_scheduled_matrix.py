"""Current finite matrix after one retained hotspot COPY-cap negative."""
import sqlite3
from pathlib import Path
from broader_player_claims_matrix import current,coordinator_sources
from broader_player_viewport_matrix import P,BINARY,replacements
from broader_scheduled_source_budget import bound,predecessors
from broader_player_acceptance import checked
from rendering_pilot import require

def specs(reader):
    name=P+'scheduled-source-catalog-01.json'
    catalog=checked(reader,dict(record=name,sha256=reader.digest(name)))
    require(catalog['kind']=='broader_copy_scheduling_source_catalog'
            and catalog['retained_current_cases']==35
            and catalog['remaining_positive_cases']==55
            and catalog['total_positive_cases']==90
            and catalog['total_positive_original_starts']==98
            and catalog['native_source_waivers']==[], 'Changed finite hotspot continuation domain')
    result=[]
    for binding in catalog['case_bindings']:
        phase=checked(reader,binding['plan'])
        case=next(c for c in phase['broader_cases']if c['id']==binding['case_id'])
        result.append(dict(plan=binding['plan']['record'],case=case,
                           role=binding['role'],family=binding['family']))
    require(len(result)==90 and len({s['case']['id']for s in result})==90
            and sum(s['case']['reserved_original_games']for s in result)==98,
            'Current positive context/start denominator changed')
    return result

def requests(root,reader):
    with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
        db.row_factory=sqlite3.Row
        rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
    plan=reader.json(P+'plan-25.json');maximum=bound(reader,plan)
    charge=predecessors(reader,rows,plan)
    require(charge<=maximum,'Cumulative start ceiling exceeded')
    return {v['case_id']:v for v in rows},dict(
        reserved_additional_original_starts=charge,maximum_additional_original_starts=maximum,
        prior_WP10_original_starts=8,maximum_combined_original_starts=164,
        all_reservations_retained=True,rows=rows)
