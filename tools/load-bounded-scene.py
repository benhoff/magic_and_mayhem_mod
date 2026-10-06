#!/usr/bin/env python3
"""Load one hash-pinned Forest/Redcap scene through the existing native adapters."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
REALM = 'Realms/Celtic/Forest'
REQUESTS = (f'{REALM}/CFsec01.map', f'{REALM}/Terrain.ttd',
            f'{REALM}/Terrain.spr', 'CFG/Encrypted/creature.cfg',
            'Creatures/redcap.ani', 'Creatures/redcap.spr')
MAX_INPUT = 24 * 1024 * 1024


def sha(data):
    return hashlib.sha256(data).hexdigest()


def exact(value, keys, label):
    if not isinstance(value, dict) or set(value) != set(keys):
        raise ValueError(f'{label}: requires exactly {", ".join(keys)}; unsupported fields refuse')


def integer(value, low, high, label):
    if type(value) is not int or not low <= value <= high:
        raise ValueError(f'{label}: requires integer {low}..{high}')


def validate(spec):
    exact(spec, ('schema', 'policy', 'resources', 'crop', 'creature', 'presentation'), 'Scene')
    if type(spec['schema']) is not int or spec['schema'] != 1:
        raise ValueError('Unsupported scene schema')
    if spec['policy'] != 'ordinary-terrain-only':
        raise ValueError('Only ordinary-terrain-only projection is supported; runtime world content refuses')
    exact(spec['resources'], REQUESTS, 'Resources')
    for request, digest in spec['resources'].items():
        if not isinstance(digest, str) or not re.fullmatch('[0-9a-f]{64}', digest):
            raise ValueError(f'{request}: requires lowercase SHA-256')
    crop = spec['crop']
    if not isinstance(crop, list) or len(crop) != 4:
        raise ValueError('Crop requires [x,y,width,height]')
    for i, value in enumerate(crop):
        integer(value, 0 if i < 2 else 4, 127 if i < 2 else 16, 'Crop')
    creature = spec['creature']
    exact(creature, ('type', 'start', 'goal'), 'Creature')
    if type(creature['type']) is not int or creature['type'] != 10:
        raise ValueError('Only configured ordinary ground Redcap type 10 is supported')
    for label in ('start', 'goal'):
        point = creature[label]
        if not isinstance(point, list) or len(point) != 3:
            raise ValueError(f'{label}: requires crop-local XYZ')
        for i, value in enumerate(point):
            integer(value, 1, crop[2+i]-2 if i < 2 else 31, label)
    exact(spec['presentation'], ('view', 'ticks', 'frames'), 'Presentation')
    for key, low, high in (('view', 0, 3), ('ticks', 0, 4096), ('frames', 1, 64)):
        integer(spec['presentation'][key], low, high, key)
    return spec


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f'Duplicate specification key: {key}')
        result[key] = value
    return result


def read_spec(path):
    with path.open('rb') as source:
        data = source.read(65537)
    if len(data) > 65536:
        raise ValueError('Scene specification exceeds 64 KiB')
    return data, validate(json.loads(data, object_pairs_hook=unique_object))


def resolve(root, request):
    """Match the installed store's case-insensitive requests, refusing ambiguity."""
    current = root.resolve(strict=True)
    for part in request.split('/'):
        matches = [p for p in current.iterdir() if p.name.casefold() == part.casefold()]
        if len(matches) != 1 or matches[0].is_symlink():
            raise ValueError(f'Missing, ambiguous or symlink resource: {request}')
        current = matches[0]
    if not current.is_file():
        raise ValueError(f'Resource is not a file: {request}')
    return current


def admit(root, spec):
    resources = {}
    for request, expected in spec['resources'].items():
        with resolve(root, request).open('rb') as source:
            data = source.read(MAX_INPUT + 1)
        if len(data) > MAX_INPUT or sha(data) != expected:
            raise ValueError(f'Resource size/hash mismatch: {request}')
        resources[request] = data
    return resources


def write_new(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('xb') as destination:
        destination.write(data)


def run(command):
    result = subprocess.run(list(map(str, command)), capture_output=True, timeout=180)
    if result.returncode:
        detail = result.stderr.decode(errors='replace').strip()
        raise RuntimeError(f'Command failed ({result.returncode}): {command[0]}: {detail}')
    return result.stdout


def load(spec_path, root, build, output):
    data, spec = read_spec(spec_path)
    if output.resolve().is_relative_to((ROOT/'original').resolve()):
        raise ValueError('Original artifacts are read-only; choose an output under working/')
    build = build.resolve(strict=True)
    binaries = [build/'mnm-map-navigation-export', build/'mnm-world-scene-preview']
    binary_hashes = {p.name: sha(p.read_bytes()) for p in binaries}
    if output.exists():
        raise ValueError('Output directory must be new')
    before = run([ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        resources = admit(root, spec)
        output.mkdir(parents=True, exist_ok=False)
        output = output.resolve()
        write_new(output/'manifest-before.log', before)
        write_new(output/'scene.json', data)
        staged = output/'resources'
        for request, payload in resources.items():
            write_new(staged/request, payload)
        prefix = output/'world'
        creature = spec['creature']
        commands = [[binaries[0], staged, REQUESTS[0], REALM, *spec['crop'], prefix,
                     10, *creature['start'], *creature['goal'], 0]]
        report = json.loads(run(commands[0]))
        # Both endpoints must satisfy the admitted configured profile. The list
        # establishes support/validity, not route reachability.
        if any(creature[key] not in report['standing'] for key in ('start', 'goal')):
            raise ValueError('Scene start/goal is not an admitted configured standing cell')
        for request, expected in spec['resources'].items():
            if sha((staged/request).read_bytes()) != expected:
                raise ValueError(f'Staged resource changed: {request}')
        view = spec['presentation']
        commands.append([binaries[1], '--root', staged, '--checkpoint', str(prefix)+'.mnms',
                         '--realm', REALM, '--sprite', REQUESTS[5], '--terrain-map', str(prefix)+'.geometry',
                         '--view', view['view'], '--ticks', view['ticks'], '--frames', view['frames'],
                         '--output', output/'frame'])
        run(commands[1])
        for request, expected in spec['resources'].items():
            if sha((staged/request).read_bytes()) != expected:
                raise ValueError(f'Staged resource changed: {request}')
        for binary in binaries:
            if sha(binary.read_bytes()) != binary_hashes[binary.name]:
                raise ValueError(f'Executable changed during scene load: {binary.name}')
        result = {'schema': 1, 'success': True, 'live_validated': False,
                  'scope': 'Native ordinary Forest crop and one configured Redcap; no original scene equivalence.',
                  'spec_sha256': sha(data), 'resources': spec['resources'], 'binaries': binary_hashes,
                  'commands': [[str(v) for v in command] for command in commands],
                  'outputs': {p.name: sha(p.read_bytes()) for p in sorted(output.iterdir())
                              if p.is_file() and p.suffix in ('.geometry', '.frozen', '.mnms', '.565', '.png', '.json')}}
    finally:
        after = run([ROOT/'tools/original-manifest.sh', 'verify'])
        if output.is_dir():
            write_new(output/'manifest-after.log', after)
    # The success record is published only after both native consumers and the
    # immutable-input verification succeed. Partial bundles remain diagnostic.
    write_new(output/'result.json', (json.dumps(result, indent=2)+'\n').encode())
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('spec', type=Path)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    load(args.spec, args.root, args.build, args.output)
    print(args.output/'result.json')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, RuntimeError, subprocess.TimeoutExpired) as error:
        print(f'Scene refused: {error}', file=sys.stderr)
        sys.exit(1)
