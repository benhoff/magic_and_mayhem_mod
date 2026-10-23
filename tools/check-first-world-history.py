#!/usr/bin/env python3
"""Check a complete current native replay against retained first-World history."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
from coverage_evidence import read_json, evidence_digest, write_report
from canvas_initial_history import analyze
from drawing_storage_transitions import decode

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('reference', 'replay-report', 'claims', 'report'):
        p.add_argument('--'+name, type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    declaration = read_json(a.claims)
    report = dict(schema=1, success=False, sources=declaration['sources'], claims=declaration['claims'])
    inputs = {}
    def pin(path):
        inputs[str(path)] = evidence_digest(path)
    try:
        reference, replay = read_json(a.reference), read_json(a.replay_report)
        pin(a.reference); pin(a.replay_report)
        if not reference['success'] or replay['reference']['sha256'] != evidence_digest(a.reference):
            raise ValueError('Reference/replay provenance differs')
        capture = Path(reference['experiment'])/'capture'
        stream = capture/'canvas-producers.bin'; pin(stream)
        if evidence_digest(stream) != replay['producer_sha256']:
            raise ValueError('Producer bytes changed')
        spec = importlib.util.spec_from_file_location('initial_producers', ROOT/'tools/inspect-canvas-producers.py')
        inspector = importlib.util.module_from_spec(spec); spec.loader.exec_module(inspector)
        _, records = inspector.decode(stream.read_bytes(), complete=True)
        first = next(r for r, _ in records if r[2] == 11)
        before = next(r for r, _ in reversed(records[:first[1]-1]) if r[2] == 10 and r[3] == first[3])
        original = capture/f'producer-oracle-{before[14]:04d}.565'
        native = ROOT/replay['artifacts']/f'native/checkpoint-{before[14]:04d}.565'
        journals = list(capture.glob('canvas-storage-transitions-*.bin'))
        if len(journals) != 1:
            raise ValueError('Exactly one same-run storage journal required')
        for path in (original, native, journals[0]): pin(path)
        report.update(analyze(records, decode(journals[0].read_bytes()), replay, original.read_bytes(), native.read_bytes()))
    except Exception as error:
        report.update(success=False, error=str(error))
    report.update(inputs=inputs, inputs_stable=all(evidence_digest(Path(n)) == h for n, h in inputs.items()),
                  sources_stable=all(evidence_digest(ROOT/n, root=ROOT) == h for n, h in report['sources'].items()))
    report['success'] &= report['inputs_stable'] and report['sources_stable']
    write_report(ROOT, a.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps({k: report.get(k) for k in ('success', 'error', 'initial_pixels_compared', 'all_replay_checkpoints')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
