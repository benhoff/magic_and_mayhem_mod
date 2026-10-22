#!/usr/bin/env python3
"""Retain contiguous startup history with fifteen complete post-World canvases."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import json
from pathlib import Path
from drawing_corpus import ROOT, load_tool, sha

RETENTION = 'NR.drawing-journey.retention-20261010'
CATALOG = 'tests/fixtures/drawing-journey/scene.json'
ORACLES = (2, 907, 920, 930, 940, 950, 960, 970, 980, 990, 1000, 1011, 1021, 1031, 1041, 1051, 1061)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--historical-root', type=Path, default=ROOT)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    retained = load_tool('retain-drawing-scene')
    report = json.loads(evidence_text(ROOT / 'research/runtime/native-world-producer-handoff-gpu-current-comparison-20261008.json'))
    stream = next(p for p in report['inputs'] if p.endswith('/capture/canvas-producers.bin'))
    from drawing_corpus import local
    data = evidence_bytes(local(args.historical_root, stream))
    if sha(data) != report['inputs'][stream]:
        raise ValueError('Original producer history changed')
    _, records = load_tool('inspect-canvas-producers').decode(data)
    completed = 0
    frames = []
    for r, _ in records:
        if r[2] == 12:
            completed += 1
        if r[2] == 10 and r[14] in ORACLES:
            frames.append(dict(sequence=r[1], queues=completed, canvas=r[3], width=r[5], height=r[6]))
    if len(frames) != len(ORACLES) or frames[-1]['queues'] != 15:
        raise ValueError('Pinned composed-frame identities changed')
    destination = args.output.resolve()
    catalog = retained.retain(destination, sequence=frames[-1]['sequence'], oracles=ORACLES,
        scope='Contiguous original-active Quick Battle map2 startup: two complete queue-zero checkpoints and fifteen complete post-World HUD/minimap/marker/cursor canvases. Native multi-frame publication and history; not a full gameplay or physical-display journey.',
        frames=frames, historical_root=args.historical_root)
    catalog['retention_sources']['tools/retain-drawing-journey.py'] = sha(evidence_bytes(Path(__file__)))
    (destination / 'scene.json').write_text(json.dumps(catalog, indent=2) + '\n')
    print(json.dumps({'frames': len(frames), 'assets': len(catalog['assets']), 'boundary': catalog['boundary']}))


if __name__ == '__main__':
    main()
