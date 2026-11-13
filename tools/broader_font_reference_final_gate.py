"""Preserve all prior final predicates and add the exact missing-reference refusal."""
import inspect
import broader_prerequisite_final_gate as parent
from broader_prerequisite_final_gate import same_context,caller_binding
from broader_font_reference_budget import review as reference_review,FAILED as REFERENCE_FAILED,PILOT as REFERENCE_PILOT,P
from broader_player_acceptance import checked
from rendering_pilot import require
FAILED=parent.FAILED

def exact_font_reference_ledger(rows,values,old,failed,zero,font,reference):
 require(len(rows)==187 and sum(v['reserved_games']for v in rows.values())==235,'Complete187/235ledger required')
 require(reference['case_id']==REFERENCE_FAILED and reference['state']=='failed'and reference['reserved_games']==1 and reference['acquisition_sha256']and rows.get(REFERENCE_FAILED)==reference and all(v['spec']['case']['id']!=REFERENCE_FAILED for v in values),'Exact missing-reference refusal changed or received positive credit')
 prior={k:v for k,v in rows.items()if k!=REFERENCE_FAILED}
 parent.exact_prerequisite_ledger(prior,values,old,failed,zero,font)

def _fading():
 original=inspect.getsource(parent.fading_supplement);old='total_rows=183,total_charges=231,combined_with_WP10=239';new='total_rows=187,total_charges=235,combined_with_WP10=243';require(original.count(old)==1,'Unexpected fading metadata');source=original.replace(old,new);require(source.replace(new,old)==original,'Unexpected fading predicate change');namespace=dict(parent.fading_supplement.__globals__)
 def exact_ledger(rows,values,q):exact_font_reference_ledger(rows,values,q['old_requests'],rows[FAILED],rows['bp30-world-caller-default800-01'],rows['bp31-world-font-default800-01'],rows[REFERENCE_FAILED])
 namespace['exact_ledger']=exact_ledger;exec(compile(source,__file__,'exec'),namespace);return namespace['supplement']
_FADING=_fading();_CALLER_GLOBALS=dict(parent.caller_supplement.__globals__);_CALLER_GLOBALS['original_supplement']=_FADING;exec(compile(inspect.getsource(parent.caller_supplement),__file__,'exec'),_CALLER_GLOBALS);_CALLER=_CALLER_GLOBALS['supplement']

def _supplement():
 original=inspect.getsource(parent.supplement);source=original
 changes={"prerequisite-final-reader-controls-02.json":"font-reference-final-reader-controls-01.json","controls['tests']==95":"controls['tests']==100",'total_rows=186,total_charges=234,combined_with_WP10=242':'total_rows=187,total_charges=235,combined_with_WP10=243',"exact_prerequisite_ledger(rows,values,old['old_requests'],negative['request_row_preserved_exactly'],zero['request_row_preserved_exactly'],font_negative['request_row_preserved_exactly'])":"exact_font_reference_ledger(rows,values,old['old_requests'],negative['request_row_preserved_exactly'],zero['request_row_preserved_exactly'],font_negative['request_row_preserved_exactly'],reference_negative['request_row_preserved_exactly'])"}
 for old,new in changes.items():require(old in source,'Missing exact parent final expression');source=source.replace(old,new)
 anchor='    prerequisites=prerequisite_review(reader,plan);font_negative=checked(reader,prerequisites[\'failed_font_custody\'])'
 insert="    reference=reference_review(reader,plan);reference_negative=checked(reader,reference['failed_reference_custody'])\n"+anchor
 require(source.count(anchor)==1,'Missing prerequisite review anchor');source=source.replace(anchor,insert)
 tail='    return result'
 add="""    replacement=checked(reader,dict(record=P+'font-reference-single-pilot-01.json',sha256=reader.digest(P+'font-reference-single-pilot-01.json')))
    value=next(v for v in values if v['spec']['case']['id']==REFERENCE_PILOT)
    require(replacement['qualified_current_contexts']==15 and replacement['conservative_charges_after']==152 and replacement['acquisition']==value['acquisition']and replacement['validation']==value['validation'],'Missing qualified font-reference replacement')
    result['font_reference_continuation']=dict(verified=True,review=plan['font_reference_review'],failed_zero_game_case=reference['failed_reference_custody'],replacement_pilot=dict(record=P+'font-reference-single-pilot-01.json',sha256=reader.digest(P+'font-reference-single-pilot-01.json')),retained_current_contexts=14,remaining_positive_contexts=76,remaining_positive_starts=84,total_rows=187,total_charges=235,combined_with_WP10=243,reservations_refunded=False,historical_failure_retired=False)
"""+tail
 require(source.count(tail)==1,'Unexpected parent return');source=source.replace(tail,add)
 namespace=dict(parent.supplement.__globals__);namespace.update(_CALLER=_CALLER,reference_review=reference_review,REFERENCE_PILOT=REFERENCE_PILOT,exact_font_reference_ledger=exact_font_reference_ledger);exec(compile(source,__file__,'exec'),namespace);return namespace['supplement']
supplement=_supplement()
_source=inspect.getsource(parent.prior_context).replace("catalog=reader.json(P+'prerequisite-source-catalog-01.json')","catalog=reader.json(P+'font-reference-source-catalog-01.json')").replace("['loader-source-catalog-01.json','capacity-source-catalog-01.json']","['prerequisite-source-catalog-01.json','loader-source-catalog-01.json','capacity-source-catalog-01.json']")
_namespace=dict(parent.prior_context.__globals__);exec(compile(_source,__file__,'exec'),_namespace);prior_context=_namespace['prior_context']
