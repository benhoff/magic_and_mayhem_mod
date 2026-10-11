#!/usr/bin/env python3
"""Retain exact historical World inputs/oracles, preserving independent provenance."""
import argparse
import gzip
import json
from pathlib import Path

from drawing_corpus import BUILD, ROOT, SCENARIOS, load_tool, local, sha, world_inputs


def retain(destination):
    from rendering_coverage import evidence_record
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    ev = next(e for e in register['evidence'] if e['id'] == 'RS.world-frame.original-20261007')
    report = evidence_record(ROOT, ev, {b['id']: b for b in register['builds']})
    if report['original_pixels_used_as_native_inputs'] is not False:
        raise ValueError('Historical replay used original destination pixels')
    if destination.exists() or not destination.resolve().is_relative_to(ROOT / 'tests/fixtures'):
        raise ValueError('Choose a new directory beneath tests/fixtures')
    surface = load_tool('check-surface-fixtures')
    surface_path = ROOT / 'tests/fixtures/surfaces/corpus.json'
    catalog = {'schema': 1, 'build': ev['build'], 'source_sha256': BUILD,
               'scope': 'Portable five surface operations and four World consumer canvases; no whole-window/session closure.',
               'surface_corpus': {'path': str(surface_path.relative_to(ROOT)), 'sha256': sha(surface_path.read_bytes())},
               'scenarios': [{'id': s, 'remaining': 'Complete presented-scene/input journey comparison pending; bounded component evidence cannot close this scenario.'} for s in SCENARIOS],
               'cases': []}
    for c in json.loads(surface_path.read_text())['cases']:
        raw, decoded = surface.read_case(surface_path.parent, c)
        import struct
        h = struct.unpack_from('<32I', raw)
        at = 128 + h[26] + h[27]
        sanitized = raw[:at] + bytes(h[27]) + raw[at + h[27]:]
        catalog['cases'].append({'id': c['id'], 'kind': 'surface', 'surface_case': c['id'],
                                 'scenario': 'unattributed-surface', 'scope': c['scope'],
                                 'identity': {'capture_sha256': sha(raw), 'native_input_sha256': sha(sanitized),
                                              'width': h[20], 'height': h[21], 'caller_return_va': decoded['caller_return_va']}})
    prepared = []
    for f in report['frames']:
        name = f['name']
        paths = [p for p in report['inputs'] if Path(p).name == name + '.bin']
        if len(paths) != 1:
            raise ValueError('Ambiguous historical frame input')
        input_path = paths[0]
        oracle_path = str(Path(input_path).with_suffix('.after.565'))
        entries = {}
        for role, p in (('input', input_path), ('oracle', oracle_path)):
            raw = local(ROOT, p).read_bytes()
            if sha(raw) != report['inputs'][p]:
                raise ValueError('Historical input/output changed: ' + p)
            packed = gzip.compress(raw, mtime=0)
            relative = str(destination.relative_to(ROOT) / (name + '-' + role + '.gz'))
            entries[role] = {'path': relative, 'encoding': 'gzip', 'sha256': sha(packed),
                             'raw_sha256': sha(raw), 'historical_path': p}
            prepared.append((ROOT / relative, packed))
        raw = local(ROOT, input_path).read_bytes()
        identity, _ = world_inputs(raw)
        identity['native_input_sha256'] = sha(raw)
        catalog['cases'].append({'id': name, 'kind': 'world_canvas', 'scenario': 'quick-battle-entry',
                                 'scope': 'Complete selected 800x600 World consumer output from zero native base; outside-World/window drawing excluded.',
                                 'frame': name, 'evidence': ev['id'], 'identity': identity, **entries})
    destination.mkdir()
    for path, packed in prepared:
        with path.open('xb') as out:
            out.write(packed)
    with (destination / 'corpus.json').open('x') as out:
        json.dump(catalog, out, indent=2)
        out.write('\n')
    return catalog


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    try:
        result = retain(args.output.resolve())
        print(json.dumps({'cases': len(result['cases']), 'original_artifacts_consumed': False}))
    except (ValueError, OSError, KeyError, StopIteration) as error:
        p.exit(2, str(error) + '\n')


if __name__ == '__main__':
    main()
