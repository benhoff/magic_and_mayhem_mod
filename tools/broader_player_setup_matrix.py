"""Current B06 matrix after one explicitly charged zero-start setup replacement."""
import sqlite3
from pathlib import Path
from broader_player_hotspot_analysis_matrix import P,BINARY,current,replacements,replacement
from broader_setup_continuation_budget import bound,predecessors
from broader_player_acceptance import checked
from rendering_pilot import require

def specs(reader):
    n=P+'COPY-source-catalog-04.json';q=checked(reader,dict(record=n,sha256=reader.digest(n)))
    require(q['kind']=='B06_reviewed_attachment_setup_catalog'and q['total_positive_cases']==90
            and q['total_positive_original_starts']==98 and q['preserved_current_cases']==18
            and q['Native_player_and_sources_unchanged']and q['old_source_current_credit']is False,
            'Changed reviewed attachment setup matrix')
    result=[]
    for b in q['case_bindings']:
        p=checked(reader,b['plan']);c=next(v for v in p['broader_cases']if v['id']==b['case_id']);result.append(dict(plan=b['plan']['record'],case=c,role=b['role'],family=b['family']))
    require(len(result)==90 and len({v['case']['id']for v in result})==90
            and sum(v['case']['reserved_original_games']for v in result)==98,'Positive90/98 denominator changed')
    return result

def requests(root,reader):
    with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as d:
        d.row_factory=sqlite3.Row;rows=[dict(v)for v in d.execute('select *from requests order by started_ns')]
    plan=reader.json(P+'plan-28.json');maximum=bound(reader,plan);charge=predecessors(reader,rows,plan)
    return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,maximum_additional_original_starts=maximum,prior_WP10_original_starts=8,maximum_combined_original_starts=210,old_rows=71,old_charges=103,closed_zero_start_setup_rows=1,closed_setup_charges=1,all_reservations_retained=True,rows=rows)
