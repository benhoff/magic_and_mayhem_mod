"""Exact current runtime cases with rebound strict extent-analysis identities."""
from broader_player_runtime_matrix import P,BINARY,current,requests,replacements,replacement
from broader_player_acceptance import checked
from rendering_pilot import require

def specs(reader):
 n=P+'COPY-source-catalog-06.json';q=checked(reader,dict(record=n,sha256=reader.digest(n)));require(q['kind']=='reviewed_fading_runtime_positive_catalog'and q['total_positive_cases']==90 and q['total_positive_original_starts']==98 and q['retained_current_cases']==0 and q['old_source_current_credit']is False,'Changed current runtime90/98 domain');out=[]
 for b in q['case_bindings']:
  p=checked(reader,b['plan']);c=next(v for v in p['broader_cases']if v['id']==b['case_id'])
  if b['role']=='strict':
   a=p['actual_context_analysis_plan'];analysis=checked(reader,dict(record=a,sha256=reader.digest(a)));require(analysis['strict_phase']==b['plan'],'Strict actual-context analysis belongs to another phase')
  out.append(dict(plan=b['plan']['record'],case=c,role=b['role'],family=b['family']))
 require(len(out)==90 and len({v['case']['id']for v in out})==90 and sum(v['case']['reserved_original_games']for v in out)==98,'Changed current runtime90/98 scope');return out
