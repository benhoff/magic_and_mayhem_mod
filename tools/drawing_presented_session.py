"""Owned native history backend for the independent DRAW-02 session ABI.

Only the external comparator holds original pixels and expected caller results.
The physical presenter receives frames derived by the native composition child.
"""
import json
import os
from pathlib import Path
import re
import subprocess
from drawing_session import same_json
from drawing_corpus import sha


class Unsupported(ValueError):
    pass


def fields(value, names):
    if not isinstance(value, dict) or set(value) != set(names):
        raise ValueError('Missing or extra presented-session field')


def number(value, lower, upper):
    if type(value) is not int or not lower <= value <= upper:
        raise ValueError('Invalid bounded presented-session number')
    return value


def token(value):
    if not isinstance(value, str) or not re.fullmatch(r'[A-Za-z0-9_-]{1,128}', value):
        raise ValueError('Invalid presented-session token')
    return value


def local(root, name):
    if not isinstance(name, str) or '\\' in name:
        raise ValueError('Invalid owned path')
    path = Path(name)
    if not path.parts or path.is_absolute() or '..' in path.parts:
        raise ValueError('Owned path escapes input directory')
    result = root/path
    if not result.resolve().is_relative_to(root.resolve()) or any(
            (root/Path(*path.parts[:i])).is_symlink() for i in range(1, len(path.parts)+1)):
        raise ValueError('Owned path traverses a symlink')
    return result


def read(root, name, limit):
    path = local(root, name)
    if not path.is_file() or path.stat().st_size > limit:
        raise ValueError('Owned input is missing or oversized')
    return path.read_bytes()


def asset_path(root, name):
    if not isinstance(name, str) or ':' in name:
        raise ValueError('Invalid native asset name')
    # Recovered asset requests use Windows separators. Keep their exact logical
    # names in the native binding map, but stage a contained POSIX file path.
    return local(root, name.replace('\\', '/').casefold())


def admit(root):
    root = Path(root).resolve()
    declaration = read(root, 'session.json', 4*1024*1024)
    session = json.loads(declaration)
    fields(session, ('schema', 'session_identity', 'inputs', 'actions', 'frames', 'effects'))
    number(session['schema'], 1, 1); token(session['session_identity'])
    if not isinstance(session['inputs'], list) or not 1 <= len(session['inputs']) <= 4096:
        raise ValueError('Owned binding budget exceeded')
    bindings = {}; identities = {'session.json': sha(declaration)}; total = 0; paths = set()
    for binding in session['inputs']:
        fields(binding, ('id', 'path', 'role', 'sha256')); name = token(binding['id'])
        if name in bindings or binding['path'] in paths:
            raise ValueError('Duplicate owned binding or path')
        if binding['role'] not in ('owned-state', 'encoded-asset', 'operation-input'):
            raise ValueError('Unknown owned role')
        limit = {'owned-state': 4*1024*1024, 'encoded-asset': 32*1024*1024,
                 'operation-input': 512*1024*1024}[binding['role']]
        data = read(root, binding['path'], limit); total += len(data)
        if total > 1536*1024*1024 or sha(data) != binding['sha256']:
            raise ValueError('Changed or oversized owned input')
        bindings[name] = (binding, data); paths.add(binding['path'])
        identities[binding['path']] = sha(data)
    states = [(name, data) for name, (b, data) in bindings.items() if b['role'] == 'owned-state']
    if len(states) != 1:
        raise Unsupported('Requires one owned drawing-history state')
    state_id, raw = states[0]; plan = json.loads(raw)
    fields(plan, ('schema', 'kind', 'session', 'assets', 'segments'))
    number(plan['schema'], 1, 1)
    if plan['kind'] != 'mnm-drawing-history-v1':
        raise Unsupported('Unsupported owned drawing-state protocol')
    if not isinstance(plan['session'], str) or not re.fullmatch(r'[1-9][0-9]{0,19}', plan['session']) or int(plan['session']) >= 2**64:
        raise ValueError('Invalid exact native session identity')
    if not isinstance(plan['assets'], dict) or len(plan['assets']) > 512:
        raise ValueError('Invalid native asset map')
    used = {state_id}; asset_paths = set(); asset_ids = set()
    for path, name in plan['assets'].items():
        normalized = asset_path(root, path).relative_to(root).as_posix().casefold()
        if normalized in asset_paths or name not in bindings or bindings[name][0]['role'] != 'encoded-asset':
            raise ValueError('Invalid native asset binding')
        if normalized == 'drawing-session.json' or re.fullmatch(r'producer-\d{4}\.bin', normalized):
            raise ValueError('Native asset collides with transport file')
        asset_paths.add(normalized); asset_ids.add(name); used.add(name)
    actions = session['actions']; frames = session['frames']
    if not isinstance(actions, list) or not 1 <= len(actions) <= 65536 or not isinstance(frames, list) or not 1 <= len(frames) <= 8192:
        raise ValueError('Action/frame budgets exceeded')
    start = None; prior = None
    for index, frame in enumerate(frames):
        fields(frame, ('identity', 'input_ids')); identity = frame['identity']
        fields(identity, ('session', 'presentation_index', 'width', 'height', 'pixel_format', 'actions_completed'))
        number(identity['presentation_index'], 0, 2**32-1)
        if start is None: start = identity['presentation_index']
        if identity['session'] != session['session_identity'] or identity['presentation_index'] != start+index:
            raise ValueError('Reordered native presented-frame identity')
        number(identity['width'], 1, 2048); number(identity['height'], 1, 2048)
        number(identity['actions_completed'], 0, len(actions))
        if identity['pixel_format'] not in ('rgb565', 'rgba8'):
            raise Unsupported('Unsupported physical output format')
        if not isinstance(frame['input_ids'], list) or len(set(frame['input_ids'])) != len(frame['input_ids']):
            raise ValueError('Invalid native frame input identities')
    for index, action in enumerate(actions):
        fields(action, ('index', 'action', 'before_presentation'))
        if number(action['index'], 0, 65535) != index or not isinstance(action['action'], str) or not 1 <= len(action['action']) <= 128:
            raise ValueError('Invalid original action identity')
        at = number(action['before_presentation'], start, start+len(frames)-1)
        if prior is not None and at < prior: raise ValueError('Reordered action boundary')
        prior = at
    for frame in frames:
        if frame['identity']['actions_completed'] != sum(a['before_presentation'] <= frame['identity']['presentation_index'] for a in actions):
            raise ValueError('Action/frame binding mismatch')
    if not isinstance(plan['segments'], list) or not 1 <= len(plan['segments']) <= 64:
        raise ValueError('Invalid native segment budget')
    mapped = []; segments = []
    for index, segment in enumerate(plan['segments'], 1):
        fields(segment, ('stage', 'producer', 'frames'))
        if segment['stage'] not in ('scene', 'menu', 'loading', 'world', 'results'):
            raise Unsupported('Unsupported native drawing stage')
        name = segment['producer']
        if name not in bindings or bindings[name][0]['role'] != 'operation-input':
            raise ValueError('Missing owned producer input')
        used.add(name)
        if not isinstance(segment['frames'], list): raise ValueError('Invalid native segment frame list')
        native_frames = []
        for frame in segment['frames']:
            fields(frame, ('frame_index', 'boundary'))
            ordinal = number(frame['frame_index'], 0, len(frames)-1)
            if ordinal != len(mapped): raise ValueError('Missing or reordered source/frame binding')
            b = frame['boundary']; fields(b, ('sequence', 'queues', 'canvas', 'width', 'height'))
            number(b['sequence'], 1, 1048576); number(b['queues'], 0, 16); number(b['canvas'], 1, 262144)
            identity = frames[ordinal]['identity']
            if any(type(b[k]) is not int or b[k] != identity[k] for k in ('width', 'height')):
                raise ValueError('Native frame boundary extent mismatch')
            if set(frames[ordinal]['input_ids']) != {state_id, name, *asset_ids}:
                raise ValueError('Incomplete frame dependency binding')
            mapped.append((index, b)); native_frames.append(b)
        segments.append(dict(segment=index, stage=segment['stage'], producer=dict(
            path=f'producer-{index:04}.bin', sha256=sha(bindings[name][1])), frames=native_frames))
    if len(mapped) != len(frames) or used != set(bindings):
        raise ValueError('Unused input or incomplete presented history')
    if not isinstance(session['effects'], list): raise ValueError('Invalid caller-effect declaration')
    if session['effects']:
        raise Unsupported('Native caller-effect execution requires a supported contract; results cannot be copied from references')
    return session, plan, bindings, identities, segments, mapped


def rgba565(raw):
    out = bytearray()
    for i in range(0, len(raw), 2):
        value = raw[i] | raw[i+1] << 8
        r, g, b = value >> 11, value >> 5 & 63, value & 31
        out.extend((r << 3 | r >> 2, g << 2 | g >> 4, b << 3 | b >> 2, 255))
    return bytes(out)


def replay(root, output, composition, presenter):
    root = Path(root).resolve(); output = Path(output).resolve()
    session, plan, bindings, identities, segments, mapped = admit(root)
    if output.exists() or output.is_relative_to(root): raise ValueError('Native output must be new and outside owned inputs')
    output.mkdir(parents=True); owned = output/'composition-input'; owned.mkdir()
    for path, name in plan['assets'].items():
        target = asset_path(owned, path); target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(bindings[name][1])
    for native, source in zip(segments, plan['segments']):
        local(owned, native['producer']['path']).write_bytes(bindings[source['producer']][1])
    native_assets={path:sha(bindings[name][1]) for path,name in plan['assets'].items()}
    # The producer callback names recovered requests exactly; WorldResources
    # independently indexes canonical filesystem SPR names. Bind both spellings
    # to the same declared bytes, without duplicating filesystem directories.
    for path,name in plan['assets'].items():
        canonical=asset_path(owned,path).relative_to(owned).as_posix()
        if canonical.endswith('.spr'): native_assets[canonical]=sha(bindings[name][1])
    if len(native_assets)>512: raise ValueError('Native asset alias budget exceeded')
    (owned/'drawing-session.json').write_text(json.dumps(dict(schema=1, session=plan['session'],
        assets=native_assets, segments=segments)))
    env = dict(os.environ)
    def execute(name, command):
        result = subprocess.run(command, env=env, capture_output=True, timeout=600)
        (output/(name+'.log')).write_bytes(result.stdout+result.stderr)
        if result.returncode: raise ValueError('Native '+name+' execution failed')
    execute('composition', [str(Path(composition).resolve()), str(owned), str(output/'composed')])
    native = json.loads((output/'composed/session-report.json').read_text())
    if native['success'] is not True or native['remaining_surfaces'] != 0 or native['original_oracles_read'] is not False or native['original_pixels_used_as_native_inputs'] is not False or native['session'] != plan['session'] or len(native['frames']) != len(mapped):
        raise ValueError('Incomplete or contaminated native composition')
    physical_input = output/'presentation-input'; physical_input.mkdir(); declarations = []; planes = []
    for index, (n, (segment, boundary)) in enumerate(zip(native['frames'], mapped)):
        if n['segment'] != segment or n['presentation'] != index+1 or not same_json({k:n[k] for k in boundary}, boundary):
            raise ValueError('Native composed frame identity changed')
        raw = read(output/'composed', n['path'], boundary['width']*boundary['height']*2)
        if len(raw) != boundary['width']*boundary['height']*2 or sha(raw) != n['sha256']:
            raise ValueError('Native derived frame hash/extent mismatch')
        name = f'frame-{index:06}.565'; (physical_input/name).write_bytes(raw); planes.append(raw)
        declarations.append(dict(path=name, width=boundary['width'], height=boundary['height'], sha256=sha(raw)))
    (physical_input/'presentation.json').write_text(json.dumps(dict(schema=1, frames=declarations)))
    execute('presentation', [str(Path(presenter).resolve()), str(physical_input), str(output/'physical')])
    physical = json.loads((output/'physical/presentation-report.json').read_text())
    if physical['success'] is not True or physical['remaining_surfaces'] != 0 or physical['qimage_uploads'] != 0 or physical['physical_presentation'] is not True or len(physical['frames']) != len(planes):
        raise ValueError('Native physical presentation did not finish')
    frames = []
    for index, (p, raw, frame) in enumerate(zip(physical['frames'], planes, session['frames'])):
        rgba = read(output/'physical', p['path'], len(raw)*2)
        if sha(rgba) != p['sha256'] or rgba != rgba565(raw) or p['native_source_sha256'] != sha(raw):
            raise ValueError('Native physical pixels differ from the derived frame')
        # RGB565 output is derived from the observed physical plane, never the
        # composition file. Exact RGBA equality above guards quantization loss.
        actual = rgba
        if frame['identity']['pixel_format'] == 'rgb565':
            actual = bytearray()
            for i in range(0,len(rgba),4):
                v = (rgba[i]>>3)<<11 | (rgba[i+1]>>2)<<5 | rgba[i+2]>>3
                actual.extend((v & 255, v >> 8))
            actual = bytes(actual)
        (output/f'frame-{index:06}.bin').write_bytes(actual)
        frames.append(dict(identity=frame['identity'], sha256=sha(actual)))
    for name, digest in identities.items():
        if sha(read(root,name,512*1024*1024)) != digest: raise ValueError('Owned input changed during native execution')
    report = dict(schema=1, success=True, session_identity=session['session_identity'], frames=frames,
                  effects=[], remaining_surfaces=0, original_oracles_read=False,
                  original_pixels_used_as_native_inputs=False, physical_presentation=True,
                  qimage_uploads=0, physical_frames=len(frames), semantic_input_equivalence=False,
                  full_draw02_completion=False)
    (output/'session-report.json').write_text(json.dumps(report,indent=2)+'\n')
    return report
