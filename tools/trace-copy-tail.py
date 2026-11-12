#!/usr/bin/env python3
"""Attribute missing COPY witnesses from immutable retained acquisition inputs."""
import argparse
import json
from pathlib import Path
import tempfile

from coverage_evidence import Reader, write_report
from copy_tail_replay import trace
from native_copy_routes import decode
from rendering_pilot import capture


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    p.add_argument('--acquisition', required=True)
    p.add_argument('--output', required=True)
    args = p.parse_args()
    with Reader(args.root) as reader:
        acquisition = reader.json(args.acquisition)
        inputs = {}
        def read(suffix):
            selected = [a for a in acquisition['artifacts'] if a['path'].endswith(suffix)]
            if len(selected) != 1:
                raise ValueError('Unique artifact required: ' + suffix)
            a = selected[0]
            data = reader.read_bytes(a['record'])
            import hashlib
            if len(data) != a['bytes'] or hashlib.sha256(data).hexdigest() != a['sha256']:
                raise ValueError('Acquisition artifact identity changed')
            inputs[suffix] = dict(record=a['record'], sha256=a['sha256'], bytes=a['bytes'])
            return data
        rows = capture(read('/commands-2.bin'))
        raw = b''.join(b for r, b in rows if r['kind'] == 'fragment')
        witnessed = [json.loads(line) for line in read('/copy-diagnostics/copies.jsonl').splitlines()]
        with tempfile.TemporaryDirectory(prefix='mnm-copy-routes-') as temporary:
            path = Path(temporary)/'routes.bin'
            path.write_bytes(read('/copy-routes-v2.bin'))
            _, routes = decode(path)
        checkpoints = json.loads(read('/flow.json'))['checkpoints']
        result = trace(raw, witnessed, routes, checkpoints)
        result.update(schema=1, kind='offline-copy-tail-attribution', success=True,
                      inputs=inputs, acquisition=dict(record=args.acquisition, sha256=reader.digest(args.acquisition)),
                      boundary='Exact command/call/World bindings. Checkpoint windows bound phases; they are not per-call menu observations. Missing historical pixel witnesses remain missing.')
    write_report(args.root, args.output, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps({k: v for k, v in result.items() if k not in ('missing_copies', 'inputs')}))


if __name__ == '__main__':
    main()
