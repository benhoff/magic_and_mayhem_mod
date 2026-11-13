"""Retain21 qualified contexts and bind the finite69-context ANI continuation."""
import sqlite3
from pathlib import Path
from broader_player_palette_schema_matrix import BINARY,current,replacements,replacement
from broader_player_acceptance import checked
from broader_picking_pose_budget import P,bound,predecessors
from rendering_pilot import require

def specs(reader):
    catalog=checked(reader,dict(record=P+'picking-pose-source-catalog-01.json',sha256=reader.digest(P+'picking-pose-source-catalog-01.json')))
    require(catalog['kind']=='broader_picking_exact_ANI_observation_continuation_catalog'
            and catalog['total_positive_cases']==90 and catalog['total_positive_original_starts']==98
            and catalog['retained_current_cases']==21 and catalog['remaining_positive_cases']==69,
            'Changed90/98 positive picking continuation domain')
    out=[]
    for binding in catalog['case_bindings']:
        phase=checked(reader,binding['plan'])
        cases=[c for c in phase['broader_cases']if c['id']==binding['case_id']]
        require(len(cases)==1,'Missing unique bound continuation case')
        if binding['role']=='strict':
            n=phase['actual_context_analysis_plan'];analysis=checked(reader,dict(record=n,sha256=reader.digest(n)))
            require(analysis['strict_phase']==binding['plan'],'Strict analysis belongs to another phase')
        out.append(dict(plan=binding['plan']['record'],case=cases[0],role=binding['role'],family=binding['family']))
    require(len(out)==90 and len({s['case']['id']for s in out})==90
            and sum(s['case']['reserved_original_games']for s in out)==98,'Changed positive denominator')
    return out

def requests(root,reader):
    with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
        db.row_factory=sqlite3.Row
        rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
    plan=reader.json(P+'plan-36.json');limit=bound(reader,plan);charge=predecessors(reader,rows,plan)
    return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,
        maximum_additional_original_starts=limit,maximum_combined_original_starts=246,
        prior_WP10_original_starts=8,all_reservations_retained=True,rows=rows)
