"""Select the exact reviewed font successor without changing current-case predicates."""
import inspect
from broader_player_prerequisite_matrix import P,BINARY,specs,requests,replacements,replacement
from broader_player_prerequisite_matrix import current as prior_current
from broader_player_copy_matrix import current as original_current
from broader_player_runtime_caller_matrix import patched_source
from broader_player_acceptance import checked
from rendering_pilot import require
FONT='bp32-world-font-default800-01'
REVIEW=P+'prerequisite-font-retained-analysis-review-01.json'
def coordinator_record(reader,spec):
 require(spec['case']['id']==FONT and spec['family']=='font'and spec['role']=='world','Font successor outside its exact context')
 ref=lambda n:dict(record=n,sha256=reader.digest(n));review=checked(reader,ref(REVIEW));phase=ref(spec['plan'])
 require(review['kind']=='reviewed_font_retained_control_rebinding'and review['case']==FONT and review['phase']==phase and review['original_game_runs']==review['new_public_player_executions']==review['new_request_reservations']==0 and review['Native_source_waivers']is False and all(reader.digest(n)==h for n,h in review['sources'].items()),'Changed or widened font retained-data review')
 require(review['acquisition']==ref(spec['case']['report'])and review['failed_comparison']==ref(spec['case']['comparison'])and review['failed_coordinator']==ref(P+FONT+'-coordinator-01.json'),'Font original identities changed')
 failure=reader.json(review['failed_comparison']['record']);require(failure['success']is False and failure['error']=='Native font controls no longer applicable','Font failure overwritten')
 phase_value=reader.json(spec['plan']);require(review['current_control']==phase_value['native_font_controls'],'Font control not bound to acquisition phase')
 control=checked(reader,review['current_control']);require(control['sources_stable']and all(reader.digest(n)==h for n,h in control['sources'].items()),'Font control source drift')
 co=checked(reader,ref(P+FONT+'-coordinator-02.json'));require(co['plan']==phase and co['case']==FONT and co['acquisition']==review['acquisition']and co['previous_failed_coordinator']==review['failed_coordinator']and co['retained_analysis_review']==ref(REVIEW)and co['original_game_runs_added']==0 and co['new_acquisition_performed']is False,'Font successor does not preserve exact capture')
 execution=checked(reader,co['retained_analysis_execution']);require(execution['review']==ref(REVIEW)and execution['request_rows_unchanged']and execution['original_game_runs']==execution['new_public_player_executions']==execution['new_request_reservations']==0,'Font successor added execution or requests')
 comparison=checked(reader,execution['comparison']);require(comparison['acquisition']==review['acquisition']and comparison['native_font_controls']==review['current_control'],'Foreign font comparison')
 return P+FONT+'-coordinator-02.json'
_namespace=dict(original_current.__globals__);_namespace['_caller_coordinator_record']=coordinator_record
exec(compile(patched_source(inspect.getsource(original_current)),__file__,'exec'),_namespace)
def current(root,reader,spec,row):
 if spec['case']['id']!=FONT:return prior_current(root,reader,spec,row)
 return _namespace['current'](root,reader,spec,row)
