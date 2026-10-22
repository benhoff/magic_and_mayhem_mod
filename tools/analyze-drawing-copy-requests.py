#!/usr/bin/env python3
"""Cross-check a completed copy journal against every original producer COPY."""
import argparse
import json
from pathlib import Path
import struct
from drawing_api_session import sha
from drawing_copy_order import decode
from drawing_copy_records import producer_records
from coverage_evidence import read_bytes, write_report

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    raw = read_bytes(args.reference, root=ROOT)
    reference = json.loads(raw)
    if not reference['success'] or not reference['normal_original_quit_verified']:
        raise ValueError('Completed original capture required')
    capture = Path(reference['experiment']) / 'capture'
    journal = (capture / 'copy-order-0000.bin').read_bytes()
    observation = decode(journal)
    if observation != reference['copy_order_observation']:
        raise ValueError('Journal differs from recorded observation')
    wire = (capture / 'canvas-producers.bin').read_bytes()
    copies = {record[1]: record for record, _ in producer_records(wire) if record[2]==6}
    linked = set()
    for returned in observation['returns']:
        sequence = returned['producer_return_sequence']
        record = copies.get(sequence)
        descriptor = [returned['destination'], returned['source'],
                      *returned['destination_extent_stride'],
                      *returned['destination_position'],
                      *returned['source_rectangle'], returned['raw_key']]
        if sequence in linked or record is None or list(record[3:15]) != descriptor:
            raise ValueError('Nonunique or changed producer COPY binding')
        linked.add(sequence)
    if linked != set(copies):
        raise ValueError('Unaccounted producer COPY')
    positions = {r['request']: i for i, r in enumerate(observation['returns'])}
    reversals = [[i, i + 1] for i in range(1, observation['requests'])
                 if positions[i] > positions[i + 1]]
    result = dict(schema=1, success=True, review_only=True,
                  sources={p: sha((ROOT / p).read_bytes()) for p in
                           ['tools/analyze-drawing-copy-requests.py',
                            'tools/drawing_copy_order.py', 'tools/drawing_copy_records.py',
                            'tools/inspect-canvas-producers.py', 'tools/drawing_api_session.py']},
                  reference=dict(path=str(args.reference), sha256=sha(raw)),
                  journal_sha256=sha(journal), producer_sha256=sha(wire),
                  requests=observation['requests'], producer_copies=len(copies),
                  unmatched_copies=0, adjacent_entry_reversals=reversals,
                  original_game_reexecuted=False, driver_order_proved=False,
                  api_request_bindings_proved=False,
                  scope='Static exact journal/producer COPY identity check and entry/return reversals. No driver ordering, source ownership, replay repair or new original execution.')
    write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, requests=result['requests'],
                          adjacent_entry_reversals=len(reversals))))


if __name__ == '__main__':
    main()
