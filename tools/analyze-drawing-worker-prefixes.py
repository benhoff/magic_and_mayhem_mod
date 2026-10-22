#!/usr/bin/env python3
"""Classify worker source-prefix bounds against exact retained copy identities."""
import argparse
from collections import Counter
import json
from pathlib import Path
from coverage_evidence import read_bytes, write_report
import struct
from drawing_api_session import sha
from drawing_worker_producer_points import decode
from drawing_copy_return_execution import associate_return
from drawing_observed_inputs import unique

ROOT = Path(__file__).resolve().parents[1]
MUTATIONS = {1, 2, 5, 6, 7, 8, 9, *range(14, 25)}


def classify(reference, joined, observed, wire):
    records, offset = {}, 64
    while offset < len(wire):
        if len(wire) - offset < 96:
            raise ValueError('Truncated source record')
        row = struct.unpack_from('<24I', wire, offset)
        if row[0] < 96 or offset + row[0] > len(wire) or row[1] != len(records) + 1:
            raise ValueError('Source record extent/sequence')
        offset += row[0]
        records[row[1]] = row
    workers = unique(observed['calls'], 'worker')
    requests = unique(reference['copy_order_observation']['entries'], 'request')
    returns = unique(reference['copy_order_observation']['returns'], 'producer_return_sequence')
    bindings = unique(joined['bindings'], 'request')
    result, counts = [], Counter()
    seen = set()
    for association in joined['bindings']:
        request = requests[association['request']]
        canvases = {request['source'], request['destination']}
        unique(association['commands'], 'command')
        for command in association['commands']:
            worker = workers[command['worker']]
            if command['worker'] in seen or worker['thread'] != command['worker_thread']:
                raise ValueError('Changed/shared command worker identity')
            seen.add(command['worker'])
            before, after = worker['before'], worker['after']
            if command['worker_entry_record']!=before['legacy_entry_record']:
                raise ValueError('Changed packet worker execution prefix')
            if not worker['both_points_available'] or before['point'][2] != 3 or after['point'][2] != 3 or before['point'][5] or after['point'][5]:
                classification, inside, touched = 'not-live-or-failed', [], []
            else:
                lower, upper = before['point'][3], after['point'][3]
                if not 0<=lower<=upper<=len(records):
                    raise ValueError('Worker prefix outside complete source journal')
                inside = [records[s] for s in range(lower + 1, upper + 1)]
                touched = [r for r in inside if r[2] in MUTATIONS and r[3] in canvases]
                classification = 'stable-prefix' if lower == upper else 'touches-copy-canvas' if touched else 'other-records-only'
            counts[classification] += 1
            copy_returns = []
            for row in touched:
                if row[2] != 6:
                    continue
                returned = returns.get(row[1])
                if returned is None or returned['request'] not in bindings:
                    raise ValueError('Touched COPY lacks exact request/API/worker binding')
                copy_returns.append(associate_return(row, returned, bindings[returned['request']], command))
            result.append(dict(request=association['request'], worker=command['worker'], command=command['command'],
                               source=request['source'], destination=request['destination'],
                               before_prefix=before['point'][3], after_prefix=after['point'][3],
                               classification=classification,
                               inside_kind_counts={str(k): v for k, v in Counter(r[2] for r in inside).items()},
                               observed_canvas_mutations=[dict(sequence=r[1], kind=r[2], destination=r[3]) for r in touched],
                               copy_return_execution_associations=copy_returns,
                               source_storage_owned=False, execution_order_repaired=False))
    outside = sorted(set(workers) - seen)
    return result, counts, outside


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--bindings', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    raw, joined_raw = read_bytes(args.reference, root=ROOT), read_bytes(args.bindings, root=ROOT)
    reference, joined = json.loads(raw), json.loads(joined_raw)
    if not reference['success'] or not reference['normal_original_quit_verified'] or not joined['success'] or joined['reference']['sha256'] != sha(raw):
        raise ValueError('Completed reference and exact request/API/worker bindings required')
    capture = Path(reference['experiment']) / 'capture'
    observed = decode((capture / 'wine-worker-producer-points.bin').read_bytes(), (capture / 'wine-blit-worker.bin').read_bytes())
    if observed != reference['wine_worker_producer_observation']:
        raise ValueError('Recorded worker source points changed')
    wire = (capture / 'canvas-producers.bin').read_bytes()
    if sha(wire) != reference['producer_observation']['producer_journal_sha256']:
        raise ValueError('Recorded producer input changed')
    result, counts, outside = classify(reference, joined, observed, wire)
    report = dict(schema=1, success=True, review_only=True,
                  sources={p: sha((ROOT / p).read_bytes()) for p in
                           ['tools/analyze-drawing-worker-prefixes.py', 'tools/drawing_copy_return_execution.py',
                            'tools/drawing_worker_producer_points.py', 'tools/drawing_api_session.py']},
                  reference=dict(path=str(args.reference), sha256=sha(raw)),
                  bindings=dict(path=str(args.bindings), sha256=sha(joined_raw)), producer_sha256=sha(wire),
                  counts=dict(counts), copy_commands=result, workers_outside_copy_scope=outside,
                  original_game_reexecuted=False, byte_or_backing_aliases_proved=False,
                  general_replay_order_proved=False,
                  scope='Static exact source-prefix intervals around request/API-bound worker bodies. Classify observed producer records on logical source/destination IDs and associate touched COPY returns with exact request/API/command worker intervals. A producer return within a prefix interval does not establish when its driver work executed. All other records and workers remain accounted. Stable prefix or unrelated logical canvas IDs do not prove immutable bytes/backing aliases. No opcode reordering, failed-context exclusion or original output input.')
    write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, counts=dict(counts), outside_copy_scope=len(outside))))


if __name__ == '__main__':
    main()
