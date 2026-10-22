#!/usr/bin/env python3
"""Classify immutable lease refusals by geometry and bounded original API caller."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
from collections import Counter
import importlib.util
import json
from pathlib import Path
import re
import struct
import subprocess

from drawing_api_session import BUILD, inspect, sha
from drawing_source_lease import decode

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--executable', type=Path, required=True)
    p.add_argument('--reference', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    names = ['tools/inspect-drawing-source-lease-clients.py', 'tools/inspect-drawing-canvas-storage.py',
             'tools/drawing_source_lease.py', 'tools/drawing_api_session.py',
             'research/runtime/coverage/binary-nocd.json', 'runtime/scene/canvas_producers.c',
             'runtime/scene/drawing_source_lease.h']
    sources = {n: sha(evidence_bytes(ROOT / n)) for n in names}
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        binary, reference_raw = evidence_bytes(a.executable), evidence_bytes(a.reference)
        r = json.loads(reference_raw)
        if sha(binary) != BUILD or r['source_sha256'] != BUILD or not r['success'] or not r['normal_original_quit_verified']:
            raise ValueError('Pinned executable and successful immutable original reference required')
        capture = Path(r['experiment']) / 'capture'
        lease_raw, api_raw = evidence_bytes(capture / 'api-source-lease.bin'), evidence_bytes(capture / 'session.bin')
        leases, api = decode(lease_raw), inspect(capture)
        if leases != r['api_source_lease_observation'] or api['calls'] != r['observation']['calls']:
            raise ValueError('Retained lease/API journal differs')
        calls = {c['entry']: c for c in api['calls']}
        rows = {v[29]: v for v in struct.iter_unpack('<32I', api_raw[64:]) if v[1] == 1}
        selected = []
        classified = Counter()
        for lease in leases['calls']:
            if lease['lease']:
                continue
            m = lease['before']
            if lease['begin_result'] == 3 and m[6] and m[7:10] == [800, 600, 800]:
                label = 'live-full-frame-pending' if m[2] == 19 and m[12] else 'stopped-full-frame' if m[2] & 4 else 'other-full-frame-refusal'
            else:
                label = 'other-geometry-or-getter-refusal'
            classified[label] += 1
            if label == 'live-full-frame-pending':
                c = calls.get(lease['api_entry'])
                if not c or c['thread'] != lease['thread'] or c['method'] != 7 or c['hresult'] < 0 or m[5] or m[13]:
                    raise ValueError('Selected live refused call owner/method/result differs')
                selected.append(dict(api_entry=lease['api_entry'], caller=c['caller'],
                                     thread=c['thread'], source_canvas=m[6], producer_sequence=m[3],
                                     pending_frames=m[12], raw_destination=list(rows[c['entry']][22:24]),
                                     raw_source_rectangle=list(rows[c['entry']][24:28]),
                                     source_rectangle_extent=[rows[c['entry']][26] - rows[c['entry']][24],
                                                              rows[c['entry']][27] - rows[c['entry']][25]],
                                     pending_frame_identity_proved=False))
        if not selected:
            raise ValueError('No live full-frame pending refusals')
        inventory_path = ROOT / 'research/runtime/coverage/binary-nocd.json'
        inventory = json.loads(evidence_text(inventory_path))
        spec = importlib.util.spec_from_file_location('canvas', ROOT / 'tools/inspect-drawing-canvas-storage.py')
        pe = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(pe)
        functions = []
        for caller in sorted({c['caller'] for c in selected}):
            owners = [f for f in inventory['functions'] if any(lo <= caller < hi for lo, hi in f['ranges'])]
            if len(owners) != 1 or len(owners[0]['ranges']) != 1:
                raise ValueError('Missing or ambiguous original caller function boundary')
            f = owners[0]
            start, end = f['ranges'][0]
            if start != f['entry'] or end - start > 4096:
                raise ValueError('Caller function bounds outside review policy')
            output = subprocess.run(['objdump', '-d', '-Mintel', '--wide',
                                     f'--start-address={start}', f'--stop-address={end}',
                                     str(a.executable)], capture_output=True, text=True, check=True).stdout
            instructions = [s for s in output.splitlines() if re.match(r'^\s*[0-9a-f]+:', s)]
            if not instructions or int(instructions[0].split(':')[0], 16) != start or not any(int(s.split(':')[0], 16) == caller for s in instructions):
                raise ValueError('Original caller or entry instruction alignment differs')
            at = pe.offset(binary, start)
            functions.append(dict(caller=caller, entry=start, end=end,
                                  whole_inventory_function=True,
                                  bytes_sha256=sha(binary[at:at + end - start]), instructions=instructions))
        if sha(evidence_bytes(a.executable)) != BUILD or evidence_bytes(a.reference) != reference_raw or any(sha(evidence_bytes(ROOT / n)) != h for n, h in sources.items()):
            raise ValueError('Original executable/reference/inspection sources changed')
        report = dict(schema=1, success=True, review_only=True,
                      sources=sources, sources_stable=True,
                      reference=dict(path=str(a.reference), sha256=sha(reference_raw)),
                      journal_sha256=dict(lease=sha(lease_raw), api=sha(api_raw)),
                      source_executable_sha256=BUILD, refusal_classes=dict(classified),
                      selected=selected, caller_functions=functions,
                      raw_rectangle_extents=[dict(extent=list(e), count=n) for e, n in Counter(tuple(s['source_rectangle_extent']) for s in selected).items()],
                      pending_frame_identity_proved=False, source_alias_or_safe_admission_proved=False,
                      source_admission_changed=False, original_game_reexecuted=False,
                      scope='Static immutable lease/API journal and exact inventory-bounded original caller inspection. Callers identify synchronous original clients, not the exact outstanding recorder frame or byte alias. Admission guards unchanged; no original reexecution or full frame/native closure.')
        with a.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, refusal_classes=dict(classified),
                              callers=[hex(f['caller']) for f in functions])))
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
