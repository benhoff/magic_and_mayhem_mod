"""Preserve all prior final predicates and add the exact missing-reference refusal."""
import inspect,sqlite3
from pathlib import Path
import broader_font_reference_final_gate as previous
import broader_prerequisite_final_gate as parent
from broader_prerequisite_final_gate import same_context,caller_binding
from broader_font_reference_budget import review as reference_review,FAILED as REFERENCE_FAILED,PILOT as REFERENCE_PILOT,P
from broader_palette_schema_budget import review as schema_review,FAILED as SCHEMA_FAILED,PILOT as SCHEMA_PILOT
from broader_player_acceptance import checked
from rendering_pilot import require
FAILED=parent.FAILED

def exact_palette_schema_ledger(rows,values,old,failed,zero,font,reference,schema):
 require(len(rows)==188 and sum(v['reserved_games']for v in rows.values())==236,'Complete188/236ledger required')
 require(schema['case_id']==SCHEMA_FAILED and schema['state']=='failed'and schema['reserved_games']==1 and schema['acquisition_sha256']and rows.get(SCHEMA_FAILED)==schema and all(v['spec']['case']['id']!=SCHEMA_FAILED for v in values),'Exact palette schema refusal changed or received positive credit')
 prior={k:v for k,v in rows.items()if k!=SCHEMA_FAILED};previous.exact_font_reference_ledger(prior,values,old,failed,zero,font,reference)

def _fading():
 original=inspect.getsource(parent.fading_supplement);old='total_rows=183,total_charges=231,combined_with_WP10=239';new='total_rows=188,total_charges=236,combined_with_WP10=244';require(original.count(old)==1,'Unexpected fading metadata');source=original.replace(old,new);require(source.replace(new,old)==original,'Unexpected fading predicate change');namespace=dict(parent.fading_supplement.__globals__)
 def exact_ledger(rows,values,q):exact_palette_schema_ledger(rows,values,q['old_requests'],rows[FAILED],rows['bp30-world-caller-default800-01'],rows['bp31-world-font-default800-01'],rows[REFERENCE_FAILED],rows[SCHEMA_FAILED])
 namespace['exact_ledger']=exact_ledger;exec(compile(source,__file__,'exec'),namespace);return namespace['supplement']
_FADING=_fading();_CALLER_GLOBALS=dict(parent.caller_supplement.__globals__);_CALLER_GLOBALS['original_supplement']=_FADING;exec(compile(inspect.getsource(parent.caller_supplement),__file__,'exec'),_CALLER_GLOBALS);_CALLER=_CALLER_GLOBALS['supplement']

def _supplement():
 original=inspect.getsource(parent.supplement);source=original
 changes={"prerequisite-final-reader-controls-02.json":"palette-schema-final-reader-controls-01.json","controls['tests']==95":"controls['tests']==114",'total_rows=186,total_charges=234,combined_with_WP10=242':'total_rows=188,total_charges=236,combined_with_WP10=244',"exact_prerequisite_ledger(rows,values,old['old_requests'],negative['request_row_preserved_exactly'],zero['request_row_preserved_exactly'],font_negative['request_row_preserved_exactly'])":"exact_palette_schema_ledger(rows,values,old['old_requests'],negative['request_row_preserved_exactly'],zero['request_row_preserved_exactly'],font_negative['request_row_preserved_exactly'],reference_negative['request_row_preserved_exactly'],schema_negative['request_row_preserved_exactly'])"}
 for old,new in changes.items():require(old in source,'Missing exact parent final expression');source=source.replace(old,new)
 anchor='    prerequisites=prerequisite_review(reader,plan);font_negative=checked(reader,prerequisites[\'failed_font_custody\'])'
 insert="    schema=schema_review(reader,plan);schema_negative=checked(reader,schema['failed_schema_custody'])\n    reference=reference_review(reader,plan);reference_negative=checked(reader,reference['failed_reference_custody'])\n"+anchor
 require(source.count(anchor)==1,'Missing prerequisite review anchor');source=source.replace(anchor,insert)
 tail='    return result'
 add="""    replacement=checked(reader,dict(record=P+'font-reference-single-pilot-01.json',sha256=reader.digest(P+'font-reference-single-pilot-01.json')))
    value=next(v for v in values if v['spec']['case']['id']==REFERENCE_PILOT)
    require(replacement['qualified_current_contexts']==15 and replacement['conservative_charges_after']==152 and replacement['acquisition']==value['acquisition']and replacement['validation']==value['validation'],'Missing qualified font-reference replacement')
    result['font_reference_continuation']=dict(verified=True,review=plan['font_reference_review'],failed_zero_game_case=reference['failed_reference_custody'],replacement_pilot=dict(record=P+'font-reference-single-pilot-01.json',sha256=reader.digest(P+'font-reference-single-pilot-01.json')),retained_current_contexts=14,remaining_positive_contexts=76,remaining_positive_starts=84,total_rows=188,total_charges=236,combined_with_WP10=244,reservations_refunded=False,historical_failure_retired=False)
    palette_pilot_name=P+'palette-schema-single-pilot-01.json';palette_pilot=checked(reader,dict(record=palette_pilot_name,sha256=reader.digest(palette_pilot_name)));palette_value=next(v for v in values if v['spec']['case']['id']==SCHEMA_PILOT)
    require(palette_pilot['qualified_current_contexts']==18 and palette_pilot['conservative_charges_after']==156 and palette_pilot['acquisition']==palette_value['acquisition']and palette_pilot['validation']==palette_value['validation'],'Missing qualified prepared palette pilot')
    palette_phase=reader.json(palette_value['spec']['plan']);prepared=checked(reader,palette_phase['counter_schema_preparation']);require(prepared['preparation']['empty']and prepared['preparation']['synthetic_control_rolled_back']and prepared['preparation']['actual_game_attempts_inserted']==0 and prepared['typed_checkpoint']['logical_restore_verified']and prepared['typed_checkpoint']['replicas_verified']==2 and prepared['historical_databases_changed']is False,'Palette counter initialization changed or unretained')
    namespace=palette_phase['counter_bindings'][str(Path(root)/'working/palette-cycling/attempts.sqlite3')];require(namespace==prepared['namespace']and prepared['prospective_phase']==palette_value['spec']['plan'],'Palette counter bound to another namespace')
    with sqlite3.connect('file:'+str(Path(root)/namespace)+'?mode=ro',uri=True)as db:counter_rows=db.execute('select report,plan_sha256 from attempts order by id').fetchall()
    palette_reports={v['acquisition']['record']for v in values if v['spec']['family']=='palette'};require(len(counter_rows)==4 and {v[0]for v in counter_rows}==palette_reports and all(v[1]==reader.digest(palette_value['spec']['plan'])for v in counter_rows),'Palette counter lost,duplicated or borrowed actual attempts')
    result['palette_schema_continuation']=dict(verified=True,review=plan['palette_schema_review'],failed_setup=schema['failed_schema_custody'],prepared_namespace=palette_phase['counter_schema_preparation'],actual_counter_rows=counter_rows,replacement_pilot=dict(record=palette_pilot_name,sha256=reader.digest(palette_pilot_name)),retained_contexts=17,remaining_contexts=73,remaining_positive_starts=81,total_rows=188,total_charges=236,combined_with_WP10=244,historical_failure_retired=False,reservations_refunded=False)
"""+tail
 require(source.count(tail)==1,'Unexpected parent return');source=source.replace(tail,add)
 namespace=dict(parent.supplement.__globals__);namespace.update(_CALLER=_CALLER,reference_review=reference_review,REFERENCE_PILOT=REFERENCE_PILOT,schema_review=schema_review,SCHEMA_PILOT=SCHEMA_PILOT,sqlite3=sqlite3,Path=Path,exact_palette_schema_ledger=exact_palette_schema_ledger);exec(compile(source,__file__,'exec'),namespace);return namespace['supplement']
supplement=_supplement()
_source=inspect.getsource(parent.prior_context).replace("catalog=reader.json(P+'prerequisite-source-catalog-01.json')","catalog=reader.json(P+'palette-schema-source-catalog-01.json')").replace("['loader-source-catalog-01.json','capacity-source-catalog-01.json']","['font-reference-source-catalog-01.json','prerequisite-source-catalog-01.json','loader-source-catalog-01.json','capacity-source-catalog-01.json']")
_namespace=dict(parent.prior_context.__globals__);exec(compile(_source,__file__,'exec'),_namespace);prior_context=_namespace['prior_context']
