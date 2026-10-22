#!/usr/bin/env python3
"""Plan bounded offline restore/full/save transpositions from metadata, never pixels."""
import argparse
import importlib.util
import json
from pathlib import Path
from coverage_evidence import read_bytes, read_json, write_report, evidence_digest
import subprocess
from drawing_api_session import sha
from drawing_copy_phase import select

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('reference', 'bindings', 'claims', 'report'):
        p.add_argument('--' + name, type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    raw, binding_raw = read_bytes(a.reference, root=ROOT), read_bytes(a.bindings, root=ROOT)
    ref, bindings = json.loads(raw), json.loads(binding_raw)
    claims = json.loads(a.claims.read_bytes())
    if (not ref['success'] or not ref['normal_original_quit_verified'] or
            not bindings['success'] or bindings['reference']['sha256'] != sha(raw)):
        raise ValueError('Completed exact original/request provenance required')
    if any(evidence_digest(ROOT/n,root=ROOT) != h for n, h in claims['sources'].items()):
        raise ValueError('Prospective source changed')
    wire = (Path(ref['experiment']) / 'capture/canvas-producers.bin').read_bytes()
    if sha(wire) != ref['producer_observation']['producer_journal_sha256']:
        raise ValueError('Original producer journal changed')
    spec = importlib.util.spec_from_file_location('phase_producers', ROOT / 'tools/inspect-canvas-producers.py')
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    _, records = m.decode(wire, complete=True)
    choices = select(records, ref['copy_order_observation'], bindings)
    if any(evidence_digest(ROOT/n,root=ROOT) != h for n, h in claims['sources'].items()):
        raise ValueError('Source changed during plan')
    result = dict(schema=1, success=True, **choices, sources=claims['sources'], claims=claims['claims'],
                  reference=dict(path=str(a.reference), sha256=sha(raw)),
                  bindings=dict(path=str(a.bindings), sha256=sha(binding_raw)),
                  producer_sha256=sha(wire), original_pixels_used_as_native_inputs=False,
                  original_artifacts_consumed=False, native_replay_performed=False,
                  scope='Authored experimental offline policy, selected only from exact successful request/API/worker metadata. Outside-World adjacent full2-to1/save1-to5 return inversion requires immediately preceding in-bounds restore5-to1, complete restore/full/save worker ordering, supported caller/geometry/key and no other observed draw/lifetime in the full interval. Restore and save may differ in position because the intervening full worker copy overwrites the whole primary. Transpose full/save COPY bodies while preserving sequential wire IDs; no checkpoint crossed. Logical recorder stability is not all-writer/GPU/CPU ownership or general schedule proof. No pixel oracle is read by this planner; no live replacement.')
    write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(selected=len(choices['selected']), refused=len(choices['refused']))))


if __name__ == '__main__':
    main()
