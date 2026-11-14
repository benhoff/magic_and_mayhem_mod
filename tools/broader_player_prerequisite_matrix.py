"""Retain ten current contexts and bind eighty finite capacity-continuation cases."""
import sqlite3
from pathlib import Path
from broader_player_runtime_caller_matrix import P,BINARY,current,replacements,replacement
from broader_player_acceptance import checked
from broader_prerequisite_budget import bound,predecessors
from rendering_pilot import require

def specs(reader):
    catalog=checked(reader,dict(record=P+'prerequisite-source-catalog-01.json',sha256=reader.digest(P+'prerequisite-source-catalog-01.json')))
    require(catalog['kind']=='broader_font_prerequisite_continuation_catalog'
            and catalog['total_positive_cases']==90 and catalog['total_positive_original_starts']==98
            and catalog['retained_current_cases']==13 and catalog['remaining_positive_cases']==77,
            'Changed90/98positive capacity-continuation domain')
    out=[]
    for b in catalog['case_bindings']:
        phase=checked(reader,b['plan']);case=next(c for c in phase['broader_cases']if c['id']==b['case_id'])
        if b['role']=='strict':
            analysis=checked(reader,dict(record=phase['actual_context_analysis_plan'],sha256=reader.digest(phase['actual_context_analysis_plan'])))
            require(analysis['strict_phase']==b['plan'],'Strict actual-context analysis belongs to another phase')
        out.append(dict(plan=b['plan']['record'],case=case,role=b['role'],family=b['family']))
    require(len(out)==90 and len({s['case']['id']for s in out})==90
            and sum(s['case']['reserved_original_games']for s in out)==98,'Changed positive context/start denominator')
    return out

def requests(root,reader):
    with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as db:
        db.row_factory=sqlite3.Row;rows=[dict(v)for v in db.execute('select *from requests order by started_ns')]
    plan=reader.json(P+'plan-32.json');limit=bound(reader,plan);charge=predecessors(reader,rows,plan)
    return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,
        maximum_additional_original_starts=limit,maximum_combined_original_starts=242,
        prior_WP10_original_starts=8,all_reservations_retained=True,rows=rows)
