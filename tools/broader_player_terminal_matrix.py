"""Select the exact retained pilot successor and preserve every current guard."""
import inspect
from types import FunctionType,SimpleNamespace
import broader_player_derived_matrix as derived
import broader_player_secondary_matrix as secondary
from broader_player_copy_matrix import current as base
from broader_player_runtime_caller_matrix import patched_source
from broader_player_acceptance import checked
from broader_terminal_pause import verify_pause
from broader_retained_staged_manifest import verify_staged
from rendering_pilot import require
from broader_player_derived_matrix import specs,requests,P,BINARY,replacements,replacement
RECOVERED='bp48-world-interaction-default800-01'
def coordinator_record(reader,spec):
    require(spec['case']['id']==RECOVERED and spec['family']=='interaction'and spec['role']=='world','Terminal successor outside exact context')
    name=P+RECOVERED+'-coordinator-03.json';value=checked(reader,dict(record=name,sha256=reader.digest(name)))
    old=value['previous_failed_coordinator'];require(old['record']==P+RECOVERED+'-coordinator-01.json'and reader.digest(old['record'])==old['sha256'],'Terminal refusal identity changed')
    admission_failure=value['previous_failed_admission_coordinator'];require(admission_failure['record']==P+RECOVERED+'-coordinator-02.json'and reader.digest(admission_failure['record'])==admission_failure['sha256']and not reader.json(admission_failure['record'])['success'],'Prior admission refusal changed')
    phase=dict(record=spec['plan'],sha256=reader.digest(spec['plan']));failure=reader.json(old['record'])
    require(not failure['success']and failure['case']==RECOVERED and failure['plan']==phase,'Original coordinator failure changed')
    require(value['kind']=='broader_domain_case_coordinator'and value['case']==RECOVERED and value['plan']==phase
        and value['original_game_runs_added']==0 and value['new_acquisition_performed']is False,'Terminal recovery widened acquisition')
    execution=checked(reader,value['retained_analysis_execution'])
    require(execution['original_game_runs']==execution['new_public_player_executions']==execution['new_request_reservations']==0
        and execution['request_rows_unchanged']and execution['original_acquisition_preserved']and execution['only_admission_fields_changed']
        and execution['validation']==value['validation']and execution['acquisition']==value['acquisition'],'Not exact retained analysis')
    return name
ns=dict(base.__globals__,_caller_coordinator_record=coordinator_record)
exec(compile(patched_source(inspect.getsource(base)),__file__,'exec'),ns)
def prior(root,reader,spec,row):
    if spec['case']['id']==RECOVERED:return ns['current'](root,reader,spec,row)
    return secondary.prior_current(root,reader,spec,row)
def clone(f,**values):
    q=FunctionType(f.__code__,dict(f.__globals__,**values),f.__name__,f.__defaults__,f.__closure__);q.__kwdefaults__=f.__kwdefaults__;return q
secondary_current=clone(secondary.current,prior_current=prior,verify_staged=verify_staged)
current=clone(derived.current,previous=SimpleNamespace(current=secondary_current),verify_pause=verify_pause)
