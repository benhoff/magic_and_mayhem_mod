#!/usr/bin/env python3
"""Retain one exact native/source mismatch and observed copy-order reversal."""
import argparse
import json
from pathlib import Path
import struct
from drawing_api_session import BUILD, sha

ROOT = Path(__file__).resolve().parents[1]


def one(values, reason):
    if len(values) != 1:
        raise ValueError(reason)
    return values[0]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--reference', type=Path, required=True)
    p.add_argument('--native', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    raw, native_raw = a.reference.read_bytes(), a.native.read_bytes()
    reference, native = json.loads(raw), json.loads(native_raw)
    if not reference['success'] or reference['source_sha256'] != BUILD or native['success'] or native['native_exit'] or native['reference']['sha256'] != sha(raw):
        raise ValueError('Successful original and failed completed native comparison required')
    failed = one([(i, c) for i, c in enumerate(native['comparisons'], 1)
                  if c['source_different_pixels'] or c['different_pixels']], 'One bounded mismatch required')
    ordinal, mismatch = failed
    capture = Path(reference['experiment']) / 'capture'
    actual = (ROOT / native['artifacts'] / f'native/frame-{ordinal:04d}.565').read_bytes()
    expected = (capture / mismatch['source_path']).read_bytes()
    if sha(expected) != mismatch['source_sha256'] or len(actual) != 960000 or len(expected) != len(actual):
        raise ValueError('Native/source mismatch plane changed')
    bad = [i for i, (x, y) in enumerate(zip(struct.unpack('<480000H', actual), struct.unpack('<480000H', expected))) if x != y]
    box = [min(i % 800 for i in bad), min(i // 800 for i in bad), max(i % 800 for i in bad) + 1, max(i // 800 for i in bad) + 1]
    if len(bad) != 1600 or box[2] - box[0] != 40 or box[3] - box[1] != 40:
        raise ValueError('Mismatch is outside the diagnosed 40x40 domain')
    checkpoints = [c for c in native['native']['checkpoints'] if c['canvas'] == mismatch['boundary']['canvas'] and c['sequence'] <= mismatch['boundary']['sequence']]
    before = one([c for c in checkpoints if c['sha256'] == sha(expected)], 'One matching prior checkpoint required')
    after = one([c for c in checkpoints if c['ordinal'] == before['ordinal'] + 1], 'Immediate same-canvas checkpoint required')
    prior_plane = (capture / f"producer-oracle-{before['ordinal']:04d}.565").read_bytes()
    after_plane = (capture / f"producer-oracle-{after['ordinal']:04d}.565").read_bytes()
    if sha(prior_plane) != sha(expected) or sha(after_plane) != sha(expected) or after['sha256'] == sha(expected):
        raise ValueError('Original unchanged / native changed checkpoint boundary differs')
    wire = (capture / 'canvas-producers.bin').read_bytes()
    records, at = [], 64
    while at < len(wire):
        r = struct.unpack_from('<24I', wire, at)
        if r[0] < 96 or at + r[0] > len(wire):
            raise ValueError('Producer wire extent changed')
        at += r[0]
        if before['sequence'] < r[1] <= after['sequence'] and r[2] == 6:
            records.append(r)
    full = one([r for r in records if r[4] == mismatch['boundary']['canvas'] and list(r[10:14]) == [0, 0, 800, 600] and r[8:10] == (0, 0)], 'One full-source copy in checkpoint interval required')
    save = one([r for r in records if r[4] == full[3] and list(r[10:14]) == box and r[8:10] == (0, 0)], 'One same-primary background save required')
    if save[1] >= full[1]:
        raise ValueError('Return stream does not contain the diagnosed reversal')
    points = {}
    for row in struct.iter_unpack('<48I', (capture / 'api-producer-points.bin').read_bytes()[64:]):
        if row[1] == 3 and before['sequence'] <= row[35] < after['sequence']:
            points[row[2]] = row
    calls = {r[29]: r for r in struct.iter_unpack('<32I', (capture / 'session.bin').read_bytes()[64:]) if r[1] == 1}

    def mapped(record):
        matches = []
        for api_entry, point in points.items():
            call = calls.get(api_entry)
            if call and call[2] == 7 and call[8] == 0 and call[21] == 16 and point[22] == record[3] and point[38] == record[4] and tuple(call[22:24]) == tuple(record[8:10]) and tuple(call[24:28]) == tuple(record[10:14]) and point[35] < record[1]:
                matches.append(api_entry)
        entry = one(matches, 'Copy/API matching is ambiguous within the bounded interval')
        submissions = [s['call'] for s in reference['wine_submission_observation']['calls'] if s['api_entry'] == entry]
        commands = [b for b in reference['wine_blit_packet_observation']['bindings'] if b['submission'] in submissions]
        if not commands:
            raise ValueError('Matched API lacks worker commands')
        return dict(producer_return_sequence=record[1], source_canvas=record[4], destination_canvas=record[3], api_entry=entry, api_thread=calls[entry][6], commands=commands)

    full_event, save_event = mapped(full), mapped(save)
    if max(c['worker_return_record'] for c in full_event['commands']) >= min(c['worker_entry_record'] for c in save_event['commands']):
        raise ValueError('Full driver copy did not finish before background save')
    result = dict(schema=1, success=True, review_only=True,
                  sources={n: sha((ROOT / n).read_bytes()) for n in ['tools/analyze-drawing-copy-order.py', 'tools/drawing_api_session.py']},
                  reference=dict(path=str(a.reference), sha256=sha(raw)), native=dict(path=str(a.native), sha256=sha(native_raw)),
                  mismatch=mismatch, native_frame_ordinal=ordinal, mismatch_box=box,
                  before_checkpoint=before, after_checkpoint=after,
                  original_checkpoint_sha256=sha(expected), original_cpu_checkpoint_unchanged=True,
                  copies=dict(full=full_event, background_save=save_event),
                  observed_driver_order=['full', 'background_save'], producer_return_order=['background_save', 'full'],
                  ordering_reversal_confirmed=True, mismatch_cause_fully_proved=False,
                  original_game_reexecuted=False, source_or_physical_ownership_proved=False,
                  scope='Static exact local mismatch and uniquely matched original API/driver command order reversal. Original CPU checkpoints remain equal while return-order native replay changes the40x40region. Ordering is confirmed; general cause, view/alias/lifetime and safe replay repair remain hypotheses. No original pixels enter native inputs and no code/admission/status is changed.')
    with a.report.open('x') as out:
        json.dump(result, out, indent=2)
        out.write('\n')
    print(json.dumps(dict(success=True, mismatch_box=box, return_sequences=[save[1], full[1]],
                          api_entries=[full_event['api_entry'], save_event['api_entry']])))


if __name__ == '__main__':
    main()
