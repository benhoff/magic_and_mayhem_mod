"""Keep the failed caller coordinator immutable while selecting its reviewed successor."""
import ast
import inspect
from broader_player_runtime_inputs_matrix import P,BINARY,specs,requests,replacements,replacement
from broader_player_copy_matrix import current as original_current
from broader_player_acceptance import checked
from rendering_pilot import require
FIRST='bp29-world-caller-default640-01'
REVIEW=dict(record=P+'runtime29-caller-root-analysis-plan-05.json',sha256='2e884bedfe342d8dac9bf286d9d59f0a3a15b6abe716c364e695998d2f8a3e3f')


def coordinator_record(reader,spec):
    cid=spec['case']['id']
    if cid!=FIRST:return P+cid+'-coordinator-01.json'
    require(spec['family']=='caller'and spec['role']=='world','Caller coordinator override outside its exact context')
    review=checked(reader,REVIEW)
    require(review['kind']=='reviewed_runtime29_caller_root_retained_analysis_plan'
            and review['case']==cid and review['original_game_runs']==0
            and review['tests']==11 and review['resolved_source_count']==1376
            and review['evidence_payload_controls']==9 and review['total_controls']==20
            and len(review['resolved_evidence_payloads'])==7
            and review['Native_source_waivers']is False and review['source_lookup_waivers']is False
            and all(reader.digest(n)==h for n,h in review['sources'].items()),
            'Caller source-root review changed or weakened')
    phase=dict(record=spec['plan'],sha256=reader.digest(spec['plan']))
    require(review['phase']==phase,'Caller review belongs to another prospective phase')
    old=review['failed_coordinator'];require(old['record']==P+cid+'-coordinator-01.json'
            and reader.digest(old['record'])==old['sha256'],'Caller refusal identity changed')
    refusal=reader.json(old['record'])
    require(refusal['success']is False and refusal['case']==cid and refusal['plan']==phase,
            'Caller setup refusal was replaced or promoted')
    name=P+cid+'-coordinator-02.json';record=checked(reader,dict(record=name,sha256=reader.digest(name)))
    require(record['kind']=='broader_domain_case_coordinator'and record['case']==cid
            and record['plan']==phase and record['acquisition']==review['acquisition']
            and record['previous_failed_coordinator']==old and record['retained_analysis_plan']==REVIEW
            and record['original_game_runs_added']==0 and record['new_acquisition_performed']is False,
            'Caller successor does not preserve the exact acquired context and refusal')
    execution=checked(reader,record['retained_analysis_execution'])
    require(execution['plan']==REVIEW and execution['original_game_runs']==0
            and execution['new_public_player_executions']==0
            and execution['new_request_reservations']==0 and execution['request_rows_unchanged']is True,
            'Caller successor is not a reviewed retained-data-only analysis')
    return name


def patched_source(source):
    old="co_ref=ref(P+case['id']+'-coordinator-01.json')"
    new='co_ref=ref(_caller_coordinator_record(r,spec))'
    require(source.count(old)==1,'Expected one exact coordinator reference expression')
    result=source.replace(old,new)
    require(result.replace(new,old)==source,'Non-coordinator current-case predicate changed')
    original=ast.parse(source);adapted=ast.parse(result)
    class Restore(ast.NodeTransformer):
        def visit_Call(self,node):
            if isinstance(node.func,ast.Name)and node.func.id=='_caller_coordinator_record':
                require(ast.dump(node,include_attributes=False)==ast.dump(ast.parse('_caller_coordinator_record(r,spec)',mode='eval').body,include_attributes=False),'Unknown coordinator replacement')
                return ast.parse("P+case['id']+'-coordinator-01.json'",mode='eval').body
            return self.generic_visit(node)
    require(ast.dump(Restore().visit(adapted),include_attributes=False)==ast.dump(original,include_attributes=False),
            'Current Native/provider/start/lifetime/source/retention predicate changed')
    return result

_namespace=dict(original_current.__globals__)
_namespace['_caller_coordinator_record']=coordinator_record
exec(compile(patched_source(inspect.getsource(original_current)),__file__,'exec'),_namespace)
_reviewed_current=_namespace['current']


def current(root,reader,spec,row):
    if spec['case']['id']!=FIRST:return original_current(root,reader,spec,row)
    return _reviewed_current(root,reader,spec,row)
