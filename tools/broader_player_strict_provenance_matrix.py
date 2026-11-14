"""Keep strict acquisition bytes immutable while selecting exact runtime joins."""
import inspect
from types import FunctionType
from broader_player_strict_binding_matrix import P,BINARY,specs,requests,replacements,replacement
from broader_player_strict_binding_matrix import current as prior_current
from broader_player_copy_matrix import current as original_current
from broader_player_runtime_caller_matrix import patched_source
from broader_strict_provenance import native_phase
from broader_player_acceptance import checked
from rendering_pilot import require
PILOT='bp42-strict-default10-01'
def coordinator_record(reader,spec):
 cid=spec['case']['id'];require(cid==PILOT and spec['role']==spec['family']=='strict','Strict successor outside its exact context')
 name=P+cid+'-coordinator-02.json';q=checked(reader,dict(record=name,sha256=reader.digest(name)));old=q['previous_failed_coordinator'];require(old['record']==P+cid+'-coordinator-01.json'and reader.digest(old['record'])==old['sha256']and reader.json(old['record'])['success']is False,'Strict failed coordinator rewritten')
 require(q['new_acquisition_performed']is False and q['original_game_runs_added']==q['new_request_reservations']==0 and q['plan']==dict(record=spec['plan'],sha256=reader.digest(spec['plan'])),'Strict successor widened execution')
 execution=checked(reader,q['retained_analysis_execution']);require(execution['original_game_runs']==execution['new_native_executions']==execution['new_request_reservations']==0 and execution['original_acquisition_preserved']and execution['only_admission_fields_changed'],'Strict successor changed captured inputs or added runs')
 return name
_globals=dict(original_current.__globals__,native_phase=native_phase)
_complete=FunctionType(original_current.__code__,_globals,original_current.__name__,original_current.__defaults__,original_current.__closure__)
_namespace=dict(_globals,_caller_coordinator_record=coordinator_record)
exec(compile(patched_source(inspect.getsource(original_current)),__file__,'exec'),_namespace)
def current(root,reader,spec,row):
 if spec['role']!='strict':return prior_current(root,reader,spec,row)
 if spec['case']['id']==PILOT:return _namespace['current'](root,reader,spec,row)
 return _complete(root,reader,spec,row)
