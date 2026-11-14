"""B06 matrix with source-complete unstarted auxiliary inputs; retain accepted cases."""
from broader_player_copy_matrix import P,BINARY,current,requests,replacements,replacement
from broader_player_acceptance import checked
from rendering_pilot import require

def specs(reader):
 name=P+'COPY-source-catalog-03.json';q=checked(reader,dict(record=name,sha256=reader.digest(name)));require(q['kind']=='broader_COPY_complete_auxiliary_input_catalog'and q['success']and q['total_positive_cases']==90 and q['total_positive_original_starts']==98 and q['source_inputs_completed_without_new_game']and q['Native_player_and_sources_unchanged']and q['old_source_current_credit']is False,'Changed complete B06 source-input domain');result=[]
 for binding in q['case_bindings']:
  phase=checked(reader,binding['plan']);case=next(v for v in phase['broader_cases']if v['id']==binding['case_id']);result.append(dict(plan=binding['plan']['record'],case=case,role=binding['role'],family=binding['family']))
 require(len(result)==90 and len({v['case']['id']for v in result})==90 and sum(v['case']['reserved_original_games']for v in result)==98,'B06 positive denominator changed');return result
