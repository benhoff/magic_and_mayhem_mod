"""Portable complete native canvas replay; original outputs stay in the comparator."""
from collections import Counter
import json
import lzma
import os
from pathlib import Path
import subprocess
import tempfile

from drawing_corpus import ROOT, BUILD, artifact, compare, local, load_tool, sha

CATALOG = 'tests/fixtures/drawing-scene/scene.json'
RETENTION = 'NR.drawing-scene.retention-20261010'


def asset_name(name):
    parts = name.replace('\\', '/').split('/')
    if not parts or any(p in ('', '.', '..') or ':' in p for p in parts):
        raise ValueError('Invalid closed scene asset name')
    return '/'.join(parts)


def input_entries(catalog):
    entry = catalog['input']
    return entry.get('chunks', [entry])


def input_bytes(root, catalog):
    entries = input_entries(catalog)
    if not 1 <= len(entries) <= 8:
        raise ValueError('Invalid bounded scene input chunks')
    def chunk(entry):
        if entry['encoding'] == 'gzip':
            return artifact(root, entry)
        if entry['encoding'] != 'xz':
            raise ValueError('Unsupported scene chunk encoding')
        path = local(root, entry['path'])
        if path.stat().st_size > 32 * 1024 * 1024:
            raise ValueError('Scene input chunk storage exceeds bound')
        packed = path.read_bytes()
        if sha(packed) != entry['sha256']:
            raise ValueError('Scene input chunk hash changed')
        try:
            decoder = lzma.LZMADecompressor(format=lzma.FORMAT_XZ, memlimit=128*1024*1024)
            raw = decoder.decompress(packed, max_length=32*1024*1024+1)
            if not decoder.eof or decoder.unused_data or len(raw) > 32*1024*1024 or sha(raw) != entry['raw_sha256']:
                raise ValueError('Scene input chunk extent/hash/trailing data changed')
            return raw
        except lzma.LZMAError as error:
            raise ValueError('Invalid bounded scene input chunk') from error
    raw = b''.join(chunk(entry) for entry in entries)
    if len(raw) > 128 * 1024 * 1024 or sha(raw) != catalog['input']['raw_sha256']:
        raise ValueError('Scene input concatenation identity changed')
    return raw


def admit(root, catalog, register, retention=RETENTION):
    from rendering_coverage import evidence_record
    evidence = {e['id']: e for e in register['evidence']}
    builds = {b['id']: b for b in register['builds']}
    # The immutable retention record pins the exact prefix slice made while
    # checking the full historical stream hash. Historical working files are
    # not required during portable replay.
    retained = evidence_record(root, evidence[retention], builds)
    if catalog != retained or catalog['schema'] not in (1, 2) or catalog['source_sha256'] != BUILD:
        raise ValueError('Scene retention identity changed')
    ev = evidence[catalog['evidence']]
    if (ev['build'] != catalog['build'] or ev['sha256'] != catalog['historical_report_sha256'] or
            catalog['original_inputs_verified'] is not True or
            ev['kind'] not in ('isolated_original', 'live_equivalence', 'live_replacement')):
        raise ValueError('Scene requires independent original evidence')
    historical = evidence_record(root, ev, builds)
    if historical['original_pixels_used_as_native_inputs'] or historical['original_oracles_read_by_native']:
        raise ValueError('Oracle-dependent original comparison')
    stream = catalog['historical_stream']
    if historical['inputs'].get(stream['path']) != stream['sha256']:
        raise ValueError('Historical stream provenance mismatch')
    raw = input_bytes(root, catalog)
    if len(raw) != stream['prefix_bytes']:
        raise ValueError('Retained prefix extent mismatch')
    _, records = load_tool('inspect-canvas-producers').decode(raw, complete=False)
    boundary = catalog['boundary']
    if (len(records) != boundary['sequence'] or records[-1][0][2] != 10 or
            records[-1][0][3] != boundary['canvas'] or records[-1][0][5:7] != (boundary['width'], boundary['height']) or
            not 0 < boundary['width'] <= 2048 or not 0 < boundary['height'] <= 2048):
        raise ValueError('Scene boundary is not a complete canvas checkpoint')
    if (sum(r[2] == 11 for r, _ in records) != boundary['queues'] or
            sum(r[2] == 12 for r, _ in records) != boundary['queues'] or
            dict(sorted(Counter(str(r[2]) for r, _ in records).items())) != catalog['producer_counts'] or
            not catalog['remaining'] or any(not s.strip() for s in catalog['remaining'])):
        raise ValueError('Scene queue/count/limitation identity mismatch')
    assets, names = [], set()
    for entry in catalog['assets']:
        name = asset_name(entry['name'])
        if name.casefold() in names:
            raise ValueError('Duplicate scene asset binding')
        names.add(name.casefold())
        data = artifact(root, entry)
        if historical['inputs'].get(entry['historical_path']) != sha(data):
            raise ValueError('Scene source provenance mismatch')
        assets.append((entry['name'], name, data))
    originals = {c['oracle']: c for c in historical['native']['checkpoints']}
    compared = {c['oracle']: c for c in historical['checkpoints']}
    record_checkpoints = {r[14]: r for r, _ in records if r[2] == 10}
    oracles, ordinals = [], set()
    for c in catalog['checkpoints']:
        ordinal = c['ordinal']
        if ordinal in ordinals:
            raise ValueError('Duplicate scene checkpoint')
        ordinals.add(ordinal)
        r = record_checkpoints[ordinal]
        n = originals[ordinal]
        if ((c['sequence'], c['canvas'], c['width'], c['height']) != (r[1], r[3], r[5], r[6]) or
                (n['sequence'], n['canvas']) != (r[1], r[3]) or not compared[ordinal]['equal']):
            raise ValueError('Scene oracle/frame identity mismatch')
        expected = artifact(root, c['oracle'])
        if historical['inputs'].get(c['oracle']['historical_path']) != sha(expected) or n['sha256'] != sha(expected):
            raise ValueError('Independent scene output provenance mismatch')
        if len(expected) != c['width'] * c['height'] * 2:
            raise ValueError('Scene oracle is not a full canvas')
        oracles.append((c, expected))
    if records[-1][0][14] not in ordinals:
        raise ValueError('Final composed scene lacks an independent oracle')
    if catalog['schema'] == 2:
        frames = catalog['frames']
        if not 1 <= len(frames) <= 4096 or frames[-1] != boundary:
            raise ValueError('Invalid bounded journey frame declaration')
        prior = 0
        completed = 0
        available = {}
        for r, _ in records:
            if r[2] == 12:
                completed += 1
            available[r[1]] = completed
        for frame in frames:
            if frame['sequence'] <= prior:
                raise ValueError('Journey frame ordering changed')
            matching = [c for c, _ in oracles if all(c[k] == frame[k] for k in ('sequence', 'canvas', 'width', 'height'))]
            if len(matching) != 1 or frame['queues'] != available.get(frame['sequence']):
                raise ValueError('Journey frame is not an independently compared closed completion')
            prior = frame['sequence']
    return raw, assets, oracles


def check(*, root=ROOT, catalog_path=CATALOG, executable, headless=False, retention=RETENTION):
    path = local(root, catalog_path)
    catalog = json.loads(path.read_text())
    register = json.loads((root / 'research/runtime/coverage/register.json').read_text())
    raw, assets, oracles = admit(root, catalog, register, retention)
    executable = executable.resolve()
    binary_hash = sha(executable.read_bytes())
    inputs = {str(path.relative_to(root)): sha(path.read_bytes())}
    for entry in (*input_entries(catalog), *catalog['assets'], *(c['oracle'] for c, _ in oracles)):
        inputs[entry['path']] = entry['sha256']
    parent = root / 'working/tests/drawing-scene'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    native_input = output / 'inputs'
    asset_root = native_input / 'assets'
    asset_root.mkdir(parents=True)
    (native_input / 'producers.bin').write_bytes(raw)
    for _, name, data in assets:
        # Windows source spellings may mix Interface/interface. One canonical
        # native tree avoids ambiguous case aliases in the strict resolver.
        target = asset_root / name.casefold()
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    # Neither oracle paths nor oracle bytes are sent to the renderer.
    declaration = {'schema': catalog['schema'], 'boundary': catalog['boundary'],
                   'assets': {name: sha(data) for name, _, data in assets},
                   'retain_checkpoints': [c['ordinal'] for c, _ in oracles]}
    if catalog['schema'] == 2:
        declaration['frames'] = catalog['frames']
    (native_input / 'scene.json').write_text(json.dumps(declaration, indent=2) + '\n')
    command = [str(executable), str(native_input / 'producers.bin'), str(asset_root),
               str(native_input / 'scene.json'), str(output / 'native')]
    environment = dict(os.environ)
    if headless:
        command = ['xvfb-run', '-a', *command]
        environment.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
    execution = subprocess.run(command, capture_output=True, timeout=180, env=environment)
    (output / 'native.log').write_bytes(execution.stdout + execution.stderr)
    if execution.returncode:
        raise ValueError('Native scene replay refused: ' + str(output / 'native.log'))
    native = json.loads((output / 'native/scene-report.json').read_text())
    if (not native['success'] or native['boundary'] != catalog['boundary'] or native['remaining_surfaces'] or
            native['original_pixels_used_as_native_inputs'] or native['original_oracles_read'] or
            not native['native_gpu_presentation_ready']):
        raise ValueError('Native scene completion/admission mismatch')
    completed = {c['ordinal']: c for c in native['checkpoints']}
    comparisons = []
    for c, expected in oracles:
        actual = (output / f"native/checkpoint-{c['ordinal']:04}.565").read_bytes()
        n = completed[c['ordinal']]
        if any(n[k] != c[k] for k in ('sequence', 'canvas', 'width', 'height')) or n['sha256'] != sha(actual):
            raise ValueError('Native scene checkpoint identity mismatch')
        comparisons.append({'ordinal': c['ordinal'], 'sequence': c['sequence'], 'canvas': c['canvas'],
                            **compare(actual, expected, c['width'], c['height'])})
    final = next((c, expected) for c, expected in oracles if c['sequence'] == catalog['boundary']['sequence'])
    scene = (output / 'native/scene.565').read_bytes()
    if scene != (output / f"native/checkpoint-{final[0]['ordinal']:04}.565").read_bytes() or sha(scene) != native['scene_sha256']:
        raise ValueError('Published scene differs from its complete final checkpoint')
    frame_comparisons = []
    if catalog['schema'] == 2:
        if len(native['frames']) != len(catalog['frames']) or native['frames_published'] != len(catalog['frames']):
            raise ValueError('Journey omitted a complete declared frame')
        for i, frame in enumerate(catalog['frames'], 1):
            actual = (output / f"native/frame-{i:04}.565").read_bytes()
            n = native['frames'][i-1]
            if any(n[k] != frame[k] for k in frame) or n['ordinal'] != i or n['sha256'] != sha(actual):
                raise ValueError('Journey frame identity mismatch')
            _, expected = next((c, expected) for c, expected in oracles if c['sequence'] == frame['sequence'])
            frame_comparisons.append({'ordinal': i, **frame, **compare(actual, expected, frame['width'], frame['height'])})
    if (sha(executable.read_bytes()) != binary_hash or
            any(sha(local(root, name).read_bytes()) != h for name, h in inputs.items())):
        raise ValueError('Scene inputs/binary changed during comparison')
    return {'success': all(c['matching'] for c in comparisons + frame_comparisons), 'scope': catalog['scope'],
            'boundary': catalog['boundary'], 'comparisons': comparisons,
            'pixels_compared': sum(c['pixels'] for c in comparisons),
            'mismatches': sum(c['mismatching_pixels'] for c in comparisons),
            'frame_comparisons': frame_comparisons,
            'native': native, 'inputs': inputs, 'binary_sha256': binary_hash,
            'original_pixels_used_as_native_inputs': False, 'original_oracles_read_by_native': False,
            'declared_canvas_complete': all(c['matching'] for c in comparisons),
            'draw03_complete': False, 'live_replacement': False, 'remaining': catalog['remaining'],
            'artifacts': str(output.relative_to(root))}
