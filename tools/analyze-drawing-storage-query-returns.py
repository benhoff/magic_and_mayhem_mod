#!/usr/bin/env python3
"""Retain exact map-query/Lock-return numerical associations and every missing link."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_api_session import BUILD, sha
from drawing_storage_transitions import decode, bind_producer, bind_queries
from drawing_storage_query_returns import bind

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        raw = args.reference.read_bytes()
        reference = json.loads(raw)
        if not reference['success'] or reference['source_sha256'] != BUILD or not reference['original_manifest_verified_before_after'] or not reference['normal_original_quit_verified']:
            raise ValueError('Completed original-active capture required')
        capture = Path(reference['experiment']) / 'capture'
        transitions = (capture / 'canvas-storage-transitions-0000.bin').read_bytes()
        producers = (capture / 'canvas-producers.bin').read_bytes()
        observed = decode(transitions)
        if observed != reference['canvas_storage_transition_observation'] or sha(producers) != reference['producer_observation']['producer_journal_sha256']:
            raise ValueError('Recorded transition/source observation changed')
        spec = importlib.util.spec_from_file_location('query_return_producers', ROOT / 'tools/inspect-canvas-producers.py')
        inspector = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(inspector)
        _, records = inspector.decode(producers, complete=True)
        producer_binding = bind_producer(observed, records)
        queries = bind_queries(observed, reference.get('wine_map_storage_observation', {}),
                               reference.get('wine_client_memory_observation', {}),
                               reference.get('wine_retained_map_source_observation', {}))
        result = bind(observed)
        result.update(review_only=True,
                      sources={name: sha((ROOT / name).read_bytes()) for name in
                               ['tools/analyze-drawing-storage-query-returns.py',
                                'tools/drawing_storage_query_returns.py',
                                'tools/drawing_storage_transitions.py', 'tools/drawing_storage_point.py',
                                'tools/drawing_api_session.py', 'tools/inspect-canvas-producers.py']},
                      reference=dict(path=str(args.reference), sha256=sha(raw)),
                      transition_sha256=sha(transitions), producer_sha256=sha(producers),
                      producer_binding=producer_binding, external_queries=queries,
                      scope='Static same-thread active Lock interval and numerical input/return/cache identity association; every legacy/null/unknown/ambiguous/changed/unbound query is retained. Producer generations and external query IDs cross-check independently. No fresh original execution, byte/allocation/view/upload ownership, general ordering repair or complete presentation claim.')
        with args.report.open('x') as output:
            json.dump(result, output, indent=2)
            output.write('\n')
        print(json.dumps(dict(success=True, bound=len(result['bindings']),
                              unbound=len(result['unbound']), queries_accounted=result['queries_accounted'])))
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
