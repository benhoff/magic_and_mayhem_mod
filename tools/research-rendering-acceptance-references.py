#!/usr/bin/env python3
"""Re-admit existing reference artifacts, without running a rendering backend.

Reference admission preserves each historical collector/presentation policy.
No original installation, original code, native backend or player is executed.
"""
import argparse
import importlib
import json
from pathlib import Path
import time

from coverage_evidence import Reader, write_report
from drawing_journeys import admit_suite

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-acceptance-research-results-20261031/'
ADAPTERS={
    'components':('drawing_corpus','operation/world_canvas'),
    'scene':('drawing_scene_corpus','composed_canvas'),
    'producer_sequence':('drawing_sequence','composed_canvas'),
    'publication':('drawing_publication_corpus','observed_primary_blt'),
    'recorded_session':('drawing_recorded_session','observed_egl_session'),
    'recorded_action_session':('drawing_recorded_session','observed_egl_action_session'),
}


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    args=parser.parse_args()
    if not args.report.startswith(PREFIX) or (ROOT/args.report).exists():
        parser.error('Require a new owned immutable report')
    with Reader(ROOT) as reader:
        names=['tools/research-rendering-acceptance-references.py','tools/drawing_journeys.py',
            'tools/check-drawing-journeys.py','tools/drawing_corpus.py','tools/drawing_scene_corpus.py',
            'tools/drawing_sequence.py','tools/drawing_publication_corpus.py',
            'tools/drawing_recorded_session.py','tools/drawing_action_session.py',
            'tools/drawing_session.py','tools/drawing_egl_gpu_reference.py',
            'tools/drawing_egl_destination.py','tools/rendering_session_contracts.py']
        sources={n:reader.digest(n) for n in names}
        catalog_path='tests/fixtures/drawing-journeys/suite.json'
        suite=admit_suite(ROOT,reader.json(catalog_path))
        sources[catalog_path]=reader.digest(catalog_path)
        register=reader.json('research/runtime/coverage/register.json')
        results=[]
        for case in suite['cases']:
            started=time.monotonic();name=case['catalog']['path'];child=reader.json(name)
            row=dict(id=case['id'],adapter=case['adapter'],catalog=dict(record=name,sha256=reader.digest(name)),
                reference_admitted=False,native_backend_executed=False,player_accepted=False,
                runner_journey_leaves=[],runner_operation_claims=[],
                reference_profile=child.get('reference_profile',{'name':'not_declared_for_this_adapter'}),
                scope=case['scope'])
            try:
                if case['adapter'] not in ADAPTERS:
                    raise ValueError('Unexpected new generic adapter; review admission separately')
                module_name,boundary=ADAPTERS[case['adapter']]
                module=importlib.import_module(module_name)
                admitted=module.admit(ROOT,child,register)
                row.update(reference_admitted=True,boundary=child.get('boundary',boundary),
                    admitted_outer_items=len(admitted))
                if case['adapter'] in ('recorded_session','recorded_action_session'):
                    row.update(observed_frames=len(admitted[2]),
                        immutable_original_evidence=child['evidence'],
                        acquisition_policy='Corrected stale-upload crop with independent destination/XCB witnesses' if child.get('schema')==3 else 'Unmodified-upload observed interval; exact policy retained')
            except Exception as error:
                row['error']=str(error)
            row['elapsed_seconds']=time.monotonic()-started;results.append(row)
            print(json.dumps({k:row.get(k) for k in ['id','reference_admitted','boundary','error']}),flush=True)
        report=dict(schema=1,kind='rendering_acceptance_reference_readmission',
            success=len(results)==len(suite['cases']),all_references_admitted=all(r['reference_admitted'] for r in results),
            sources=sources,cases=results,original_media_consumed=False,original_code_executed=False,
            native_backend_executed=False,player_acceptance_assessed=False,status_promotions=[],
            generic_presented_session_cases=sum(c['adapter']=='session' for c in suite['cases']),
            scope='Read-only actual admission of all shipped historical reference artifacts with existing validators. Admission failures are retained research findings, not new engine regressions. Source freshness, current replay equality, configuration adequacy and complete player acceptance remain separate. No guarded original directory can be read by these corpus artifact loaders.')
        report['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        digest=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],all_references_admitted=report['all_references_admitted'],sha256=digest)))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
