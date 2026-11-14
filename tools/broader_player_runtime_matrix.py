"""Fresh current runtime cohort, with every historical Native phase preserved."""
import sqlite3
from pathlib import Path
from broader_player_copy_matrix import P,BINARY,current,replacements,replacement
from broader_runtime_continuation_budget import bound,predecessors
from broader_player_acceptance import checked
from rendering_pilot import require

def specs(reader):
 n=P+'COPY-source-catalog-05.json';q=checked(reader,dict(record=n,sha256=reader.digest(n)));require(q['kind']=='reviewed_fading_runtime_positive_catalog'and q['total_positive_cases']==90 and q['total_positive_original_starts']==98 and q['retained_current_cases']==0 and q['old_source_current_credit']is False,'Changed fresh runtime90/98 domain')
 out=[]
 for b in q['case_bindings']:
  p=checked(reader,b['plan']);c=next(v for v in p['broader_cases']if v['id']==b['case_id']);out.append(dict(plan=b['plan']['record'],case=c,role=b['role'],family=b['family']))
 require(len(out)==90 and len({v['case']['id']for v in out})==90 and sum(v['case']['reserved_original_games']for v in out)==98,'Changed runtime90/98 scope');return out

def requests(root,reader):
 with sqlite3.connect('file:'+str(Path(root)/'working/broader-player/requests.sqlite3')+'?mode=ro',uri=True)as d:
  d.row_factory=sqlite3.Row;rows=[dict(v)for v in d.execute('select *from requests order by started_ns')]
 plan=reader.json(P+'plan-29.json');limit=bound(reader,plan);charge=predecessors(reader,rows,plan)
 return {v['case_id']:v for v in rows},dict(reserved_additional_original_starts=charge,maximum_additional_original_starts=limit,maximum_combined_original_starts=239,prior_WP10_original_starts=8,all_reservations_retained=True,rows=rows)
