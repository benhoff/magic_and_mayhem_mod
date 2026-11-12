#!/usr/bin/env python3
"""Independent complete-output proof for a normally retired natural stretch case."""
import argparse,hashlib,json,tempfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from natural_stretch_copy import analyze
from natural_copy_applicability import execution_review
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-copy-natural-results-20261108/'


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--acquisition',required=True);parser.add_argument('--report',required=True)
    args=parser.parse_args()
    require(args.acquisition.startswith(PREFIX) and args.report.startswith(PREFIX)
            and not (ROOT/args.report).exists(),'Fresh owned comparison required')
    names=['tools/analyze-rendering-natural-stretch.py','tools/natural_stretch_copy.py','tools/rendering_pilot.py',
           'tools/rendering_capture.py','tools/rendering_menu_acceptance.py','tools/rendering_copy_readiness.py',
           'tools/native_copy_routes.py','tools/native_gpu_copy_diagnostics.py','tools/coverage_evidence.py','tools/natural_copy_applicability.py']
    with Reader(ROOT) as reader:
        sources={n:reader.digest(n) for n in names}
        acquired=reader.json(args.acquisition)
        applicability=execution_review(reader,acquired)
        require(acquired['retirement']['state']=='completed' and acquired['original_pixels_used_as_native_inputs'] is False,
                'Ordinary native retirement required')
        require([(s['screen'],s['ack']) for s in acquired['states']]==
                [(3,0),(18,1),(27,2),(27,3),(27,4),(27,5),(27,6),(18,7),(3,8)],
                'Physical Grimoire transition sequence differs')
        require([v['physical']['symbol'] for v in acquired['actions'] if v['action'] in
                 ('book-page-key','book-close-escape','region-cancel-escape')]==[0xff56,0xff56,0xff55,0xff1b,0xff1b],
                'Public physical navigation keys differ')
        with tempfile.TemporaryDirectory(prefix='natural-stretch-') as scratch:
            result=analyze(reader,acquired,scratch)
        require(all(p['target_size']==acquired['configuration']['actual_size'] for p in result['proofs']),
                'Requested resolution lacks actual destination evidence')
        require(sources=={n:reader.digest(n) for n in names},'Analyzer changed during comparison')
        result.update(schema=1,kind='natural_stretch_complete_output_proof',
            acquisition=dict(record=args.acquisition,sha256=reader.digest(args.acquisition)),
            plan=acquired['plan'],configuration=acquired['configuration'],
            analysis_sources=sources,execution_sources=acquired['sources'],player_binary=acquired['player_binary'],
            interesting_branches=result['natural_stretch_calls'],original_game_runs=0,newly_passed_requirements=0,
            native_policy=True,original_driver_comparison=False,sources_stable=True)
        result['execution_source_review']=applicability
    digest=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,record=args.report,sha256=digest,
                         calls=result['natural_stretch_calls'],pixels=result['compared_pixels'])))


if __name__=='__main__':main()
