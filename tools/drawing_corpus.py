"""Independent DRAW-02 replay. Original outputs belong only to the comparator."""
import gzip
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CATALOG = 'tests/fixtures/drawing/corpus.json'
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
MAX_BYTES = 32 * 1024 * 1024
SCENARIOS = ('enabled-movies', 'main-setup-menus', 'campaign-entry',
             'quick-battle-entry', 'active-world', 'camera-motion',
             'creatures-effects', 'hud-minimap-tooltips', 'cursor-interaction',
             'pause-preferences', 'save-load', 'map-transitions',
             'battle-results', 'shutdown')


def sha(data):
    return hashlib.sha256(data).hexdigest()


def load_tool(name):
    spec = importlib.util.spec_from_file_location(name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def local(root, name):
    p = Path(name)
    if p.is_absolute() or not p.parts or '..' in p.parts or p.parts[0] in {'original', '.git'}:
        raise ValueError('Corpus path must be relative and contained')
    target = (root / p).resolve()
    if not target.is_relative_to(root.resolve()):
        raise ValueError('Corpus path escapes its root')
    if any(target.is_relative_to((root / d).resolve()) for d in ('original', '.git')):
        raise ValueError('Corpus cannot read original artifacts or Git internals')
    return target


def artifact(root, entry):
    path = local(root, entry['path'])
    if path.stat().st_size > MAX_BYTES:
        raise ValueError('Oversized corpus artifact')
    packed = path.read_bytes()
    if sha(packed) != entry['sha256']:
        raise ValueError('Corpus artifact hash mismatch: ' + entry['path'])
    if entry['encoding'] == 'gzip':
        with gzip.GzipFile(fileobj=io.BytesIO(packed)) as stream:
            raw = stream.read(MAX_BYTES + 1)
    elif entry['encoding'] == 'raw':
        raw = packed
    else:
        raise ValueError('Unknown artifact encoding')
    if len(raw) > MAX_BYTES or sha(raw) != entry['raw_sha256']:
        raise ValueError('Corpus raw extent/hash mismatch')
    return raw


def world_inputs(raw):
    """Extract owned RLE frames; never reconstruct them from destination pixels."""
    if len(raw) < 80 or len(raw) > MAX_BYTES or raw[:8] != b'MNMWRLD1':
        raise ValueError('Invalid World envelope')
    h = struct.unpack_from('<18I', raw, 8)
    version, header, extent, sequence, width, height, stride, count, failure, build = h[:10]
    if (version != 1 or header != 80 or extent != len(raw) or not sequence or failure or
            build != 0x40209ca7 or h[16] or not 0 < width <= 2048 or
            not 0 < height <= 2048 or not width <= stride <= 4096 or not 0 < count <= 32768):
        raise ValueError('Unsupported World envelope')
    frames = {0: {}, 1: {}}
    at = 80
    for _ in range(count):
        if at + 64 > len(raw):
            raise ValueError('Truncated World record')
        r = struct.unpack_from('<16I', raw, at)
        size, op, encoded, indexed, payload = r[0], r[1], r[8], r[9], r[10]
        # This corpus admits only the six historically compared sprite modes.
        if (op > 5 or indexed > 1 or encoded < 40 or size != 64 + encoded + payload or
                at + size > len(raw) or payload != (64 if op == 3 else 512 if indexed else 0)):
            raise ValueError('Unsupported/truncated World sprite record')
        frame = raw[at + 64:at + 64 + encoded]
        if struct.unpack_from('<I', frame)[0] != encoded or any(frame[28:32]):
            raise ValueError('World frame extent or retained pointer')
        frames[indexed][sha(frame)] = frame
        at += size
    if at != len(raw):
        raise ValueError('Trailing World records')
    return {'sequence': sequence, 'width': width, 'height': height,
            'stride': stride, 'draws': count}, frames


def sprite_files(frames):
    """Wrap exact captured source frames in SPR v4; owned tables supply colours."""
    result = {}
    for indexed, group in sorted(frames.items()):
        if not group:
            continue
        data, offsets = bytearray(), []
        for _, frame in sorted(group.items()):
            offsets.append(len(data))
            data.extend(frame)
        palette = bytes(768) if indexed else b''
        length = 24 + len(palette) + 4 * len(offsets) + len(data)
        result[f'captured-{indexed}.spr'] = (struct.pack('<6I', 0x00525053, length, 4, len(offsets), indexed, 0) +
                                            palette + struct.pack('<' + 'I' * len(offsets), *offsets) + data)
    return result


def compare(actual, expected, width, height):
    if len(actual) != width * height * 2 or len(expected) != len(actual):
        raise ValueError('Comparison must cover the entire RGB565 canvas')
    bad = [i for i, (a, b) in enumerate(zip(struct.iter_unpack('<H', actual),
                                           struct.iter_unpack('<H', expected))) if a != b]
    return {'matching': not bad, 'pixels': width * height, 'mismatching_pixels': len(bad),
            'bounds': [min(i % width for i in bad), min(i // width for i in bad),
                       max(i % width for i in bad), max(i // width for i in bad)] if bad else None,
            'examples': [{'x': i % width, 'y': i // width,
                          'native': struct.unpack_from('<H', actual, i * 2)[0],
                          'original': struct.unpack_from('<H', expected, i * 2)[0]} for i in bad[:8]],
            'native_sha256': sha(actual), 'original_sha256': sha(expected)}


def admit(root, catalog, register):
    from rendering_coverage import evidence_record
    if catalog['schema'] != 1 or catalog['source_sha256'] != BUILD:
        raise ValueError('Corpus schema/build mismatch')
    scenarios = catalog['scenarios']
    if len(scenarios) != len(SCENARIOS) or {s['id'] for s in scenarios} != set(SCENARIOS):
        raise ValueError('Required scenario denominator changed')
    if any(not s['remaining'].strip() for s in scenarios):
        raise ValueError('Missing scenario limitations')
    evidence = {e['id']: e for e in register['evidence']}
    builds = {b['id']: b for b in register['builds']}
    records, cases, seen = {}, [], set()
    surface = load_tool('check-surface-fixtures')
    surface_path = local(root, catalog['surface_corpus']['path'])
    if sha(surface_path.read_bytes()) != catalog['surface_corpus']['sha256']:
        raise ValueError('Surface corpus identity changed')
    surfaces = {c['id']: c for c in json.loads(surface_path.read_text())['cases']}
    for c in catalog['cases']:
        if (not re.fullmatch(r'[A-Za-z0-9_-]+', c['id']) or c['id'] in seen or
                c['scenario'] not in (*SCENARIOS, 'unattributed-surface') or not c['scope'].strip()):
            raise ValueError('Duplicate case or missing scenario/scope')
        seen.add(c['id'])
        if c['kind'] == 'surface':
            case = surfaces[c['surface_case']]
            raw, decoded = surface.read_case(surface_path.parent, case)
            if decoded['bits'] != 16 or decoded['masks'] != (0xf800, 0x07e0, 0x001f):
                raise ValueError('Only RGB565 surfaces admitted')
            h = struct.unpack_from('<32I', raw)
            after_at = 128 + h[26] + h[27]
            # The original after-plane is removed before the native process runs.
            sanitized = raw[:after_at] + bytes(h[27]) + raw[after_at + h[27]:]
            identity = {'capture_sha256': sha(raw), 'native_input_sha256': sha(sanitized),
                        'width': h[20], 'height': h[21], 'caller_return_va': decoded['caller_return_va']}
            cases.append((c, identity, sanitized, decoded['after']))
        elif c['kind'] == 'world_canvas':
            ev = evidence[c['evidence']]
            if ev['kind'] not in {'isolated_original', 'live_equivalence', 'live_replacement'} or ev['build'] != catalog['build']:
                raise ValueError('World canvas requires independent original evidence')
            if ev['id'] not in records:
                records[ev['id']] = evidence_record(root, ev, builds)
            report = records[ev['id']]
            if report['original_pixels_used_as_native_inputs'] is not False:
                raise ValueError('Oracle-dependent historical result')
            raw = artifact(root, c['input'])
            identity, _ = world_inputs(raw)
            identity['native_input_sha256'] = sha(raw)
            if c['input']['historical_path'] not in report['inputs'] or report['inputs'][c['input']['historical_path']] != sha(raw):
                raise ValueError('World input not bound to original comparison')
            expected = artifact(root, c['oracle'])
            frame = next(f for f in report['frames'] if f['name'] == c['frame'])
            if (frame['width'] != identity['width'] or frame['height'] != identity['height'] or
                    frame['draws'] != identity['draws'] or not frame['live_original_pixels_match'] or
                    not frame['independent_original_pixels_match'] or frame['original_sha256'] != sha(expected)):
                raise ValueError('World oracle identity differs from independent execution')
            if report['inputs'].get(c['oracle']['historical_path']) != sha(expected):
                raise ValueError('Original live output provenance mismatch')
            cases.append((c, identity, raw, expected))
        else:
            raise ValueError('Unsupported corpus case kind')
        if identity != c['identity']:
            raise ValueError('Frame/input identity mismatch')
    if not cases:
        raise ValueError('Empty drawing corpus')
    return cases


def run_world(raw, output, executable, headless):
    identity, frames = world_inputs(raw)
    inputs = output / 'inputs'
    inputs.mkdir()
    assets = inputs / 'assets'
    assets.mkdir()
    bindings = []
    for name, data in sprite_files(frames).items():
        (assets / name).write_bytes(data)
        bindings.append({'id': Path(name).stem, 'sprite': name, 'sha256': sha(data)})
    (inputs / 'bindings.json').write_text(json.dumps(bindings))
    (inputs / 'world.bin').write_bytes(raw)
    command = [str(executable.resolve()), '--root', str(assets), '--snapshot', str(inputs / 'world.bin'),
               '--bindings', str(inputs / 'bindings.json'), '--output', str(output / 'native'), '--background', '0']
    env = dict(os.environ)
    if headless:
        command = ['xvfb-run', '-a', *command]
        env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
    result = subprocess.run(command, capture_output=True, timeout=180, env=env)
    (output / 'native.log').write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise ValueError('World native replay failed: ' + str(output / 'native.log'))
    metadata = json.loads((output / 'native.json').read_text())
    actual = (output / 'native.565').read_bytes()
    if (metadata['success'] is not True or metadata['sequence'] != identity['sequence'] or
            metadata['width'] != identity['width'] or metadata['height'] != identity['height'] or
            metadata['records'] != identity['draws'] or metadata['remaining_surfaces'] or
            metadata['pixel_sha256'] != sha(actual)):
        raise ValueError('Native World completion identity/ownership mismatch')
    return actual, metadata


def check(root=ROOT, catalog_path=CATALOG, surface_executable=None, world_executable=None, headless=False):
    root = Path(root).resolve()
    catalog = json.loads(local(root, catalog_path).read_text())
    register = json.loads(local(root, 'research/runtime/coverage/register.json').read_text())
    cases = admit(root, catalog, register)
    if not surface_executable or not world_executable:
        raise ValueError('Specify both built native replay executables; no backend substitution')
    binaries = {str(p.resolve()): sha(p.read_bytes()) for p in (surface_executable, world_executable)}
    parent = root / 'working/tests/drawing-corpus'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    surface = load_tool('check-surface-fixtures')
    rows = []
    # Admission loads all expected planes, but native processes receive only
    # owned drawing inputs. Oracles are never written to their input directory.
    for c, identity, raw, expected in cases:
        directory = output / c['id']
        directory.mkdir()
        if c['kind'] == 'surface':
            path = directory / 'input.bin'
            path.write_bytes(raw)
            actual, metadata = surface.replay.render_gl(path, directory, surface_executable, headless)
        else:
            actual, metadata = run_world(raw, directory, world_executable, headless)
        rows.append({'id': c['id'], 'scenario': c['scenario'], 'kind': c['kind'], 'scope': c['scope'],
                     'identity': identity, 'comparison': compare(actual, expected, identity['width'], identity['height']),
                     'native': metadata})
    # Recheck inputs/provenance after execution, retaining all original hashes.
    if admit(root, json.loads(local(root, catalog_path).read_text()), register) != cases:
        raise ValueError('Corpus changed during comparison')
    if any(sha(Path(p).read_bytes()) != h for p, h in binaries.items()):
        raise ValueError('Native executable changed during comparison')
    return {'schema': 1, 'success': all(c['comparison']['matching'] for c in rows),
            'source_executable_sha256': BUILD, 'cases': rows,
            'pixels_compared': sum(c['comparison']['pixels'] for c in rows),
            'mismatches': sum(c['comparison']['mismatching_pixels'] for c in rows),
            'original_pixels_used_as_native_inputs': False, 'original_work_bypassed': False,
            'draw02_complete': False, 'scoped_corpus_complete': all(c['comparison']['matching'] for c in rows),
            'remaining_scenarios': catalog['scenarios'],
            'artifacts': str(output.relative_to(root)), 'binaries': binaries,
            'scope': 'Five original driver surfaces and four complete World consumer canvases. Whole presented scenes, camera actions and active battles remain pending.'}
