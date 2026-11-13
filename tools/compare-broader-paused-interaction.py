#!/usr/bin/env python3
"""Full unchanged pixel comparison with an explicit paused collection schedule."""
import argparse,hashlib,importlib.util,inspect,json,sys
from pathlib import Path
from types import FunctionType
import coverage_evidence
from coverage_evidence import Reader,write_report
import rendering_interaction_assessment as semantics
from broader_interaction_source_chunks import RetainedCameraHistory
from broader_interaction_pause import POLICY,selected_interaction_proof_source,verify_pause
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
SUPPORT=['tools/compare-broader-paused-interaction.py','tools/broader_interaction_pause.py','tools/broader_interaction_source_chunks.py']

def bind_assessor(original,proof,history):
    namespace=dict(original.__globals__,interaction_proof=proof,source_contexts=history)
    bound=FunctionType(original.__code__,namespace,original.__name__,original.__defaults__,original.__closure__)
    bound.__kwdefaults__=original.__kwdefaults__;require(bound.__code__ is original.__code__,'Outer full interaction assessor changed')
    return bound

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in('acquisition','analysis-plan','report'):p.add_argument('--'+key,required=True)
    a=p.parse_args();require(not(ROOT/a.report).exists(),'Fresh paused comparison required')
    with Reader(ROOT,memoize=True)as reader:
        ref=lambda n:dict(record=n,sha256=reader.digest(n));acquired=reader.json(a.acquisition);phase=reader.json(acquired['broader_phase']['record']);config=phase['interaction_pause_policy']
        require(config['policy']==POLICY and acquired['success']and acquired['sources_stable'],'Completed declared public pause acquisition required')
        controls=reader.json(config['controls']['record']);require(reader.digest(config['controls']['record'])==config['controls']['sha256']
            and controls['success']and controls['tests']>=20 and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current public pause controls required')
        chunks=reader.json(config['source_chunk_controls']['record']);require(chunks['success']and chunks['tests']==16
            and reader.digest(config['source_chunk_controls']['record'])==config['source_chunk_controls']['sha256']
            and all(reader.digest(n)==h for n,h in chunks['sources'].items()),'Unchanged source-chunk controls required')
        selected=selected_interaction_proof_source(inspect.getsource(semantics.interaction_proof));name=a.report.removesuffix('.json')+'-selected-paused-proof.bin';selected_ref=dict(record=name,sha256=write_report(ROOT,name,selected.encode()))
        support={n:reader.digest(n)for n in SUPPORT};controls_ref=config['controls'];chunk_controls=config['source_chunk_controls']
    ns=dict(semantics.interaction_proof.__globals__);exec(compile(selected,__file__,'exec'),ns);selected_proof=ns['interaction_proof'];history=RetainedCameraHistory(ROOT)
    def proof(reader,acquired,*args):
        value=selected_proof(reader,acquired,*args);value['public_pause_collection']=verify_pause(reader,acquired);return value
    original_assess=semantics.assess;semantics.assess=bind_assessor(original_assess,proof,history)
    original_write=coverage_evidence.write_report;argv=sys.argv
    def writer(root,target,raw):
        if str(target)==a.report:
            q=json.loads(raw);q['sources']=q.get('sources',{})|support
            if 'analysis_input_binding'in q:q['analysis_input_binding']['semantic_guards_changed']=True
            q['paused_interaction_analysis']=dict(policy=POLICY,controls=controls_ref,source_chunk_controls=chunk_controls,selected_interaction_proof=selected_ref,
                unchanged_outer_assessor_code=True,exact_other_proof_AST_preserved=True,explicit_changes=['optional public interaction-pause before target actions','interaction-resume after all target actions','existing complete source-chunk camera-history reader','additional actual pause-byte/counter/living-wizard proof'],
                original_game_runs=0,Native_runtime_compiler_files_changed=False,source_chunks_credited_as_Worlds=0,source_chunks_credited_as_action_witnesses=0,source_chunk_diagnostics=history.diagnostics)
            if q['success']:require(history.calls==1 and history.diagnostics is not None and q['proof']['public_pause_collection']['verified'],'Missing complete paused/source-chunk proof')
            with Reader(ROOT,memoize=True)as reader:stable=all(reader.digest(n)==h for n,h in q['sources'].items())
            q['sources_stable']=q.get('sources_stable',False)and stable;q['success'] &= q['sources_stable'];raw=(json.dumps(q,indent=2)+'\n').encode()
        return original_write(root,target,raw)
    coverage_evidence.write_report=writer
    try:
        path=ROOT/'tools/compare-broader-current-interaction.py';sp=importlib.util.spec_from_file_location('paused_current_input_comparison',path);core=importlib.util.module_from_spec(sp);sp.loader.exec_module(core)
        sys.argv=[str(path),'--acquisition',a.acquisition,'--analysis-plan',a.analysis_plan,'--report',a.report];return core.main()
    finally:semantics.assess=original_assess;coverage_evidence.write_report=original_write;sys.argv=argv
if __name__=='__main__':raise SystemExit(main())
