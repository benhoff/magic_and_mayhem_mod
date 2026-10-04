#!/usr/bin/env python3
"""Original geometry-pass byte comparisons and initialized native terrain scenes."""
import argparse
import configparser
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def geometry(raw, ttd):
    """Independent assembly transcription; projection/base setup is static evidence."""
    w, h, layers = struct.unpack_from('<3I', raw, 4)
    records = [ttd[i:i+356] for i in range(16, len(ttd), 356)]
    cells = [list(c) for c in struct.iter_unpack('<6H', raw[76:])]
    assert len(cells) == w*h*layers and all(c[0] < len(records) for c in cells)
    for c in cells:
        c[1:4] = [65535]*3
        c[5] &= 0xdff6
        c[5] |= int(c[0] != 0 and struct.unpack_from('<I', records[c[0]], 0x94)[0] == 16)
    encode = lambda: raw[:76] + b''.join(struct.pack('<6H', *c) for c in cells)
    base = encode()
    lookup = [records[c[0]][0xa8] for c in cells]
    plane = w*h
    for z in range(layers-1):
        for y in range(h):
            for x in range(w):
                i = z*plane+y*w+x
                north = lookup[z*plane+((y-1) % h)*w+x]
                east = lookup[z*plane+y*w+(x+1) % w]
                south = lookup[z*plane+((y+1) % h)*w+x]
                west = lookup[z*plane+y*w+(x-1) % w]
                c, upper = cells[i], cells[i+plane]
                flags = c[4] & 0xff87
                if lookup[i+plane] & 32:
                    flags |= 8 if east & 8 and south & 1 else 0
                    flags |= 16 if south & 1 and west & 2 else 0
                    flags |= 32 if north & 4 and west & 2 else 0
                    flags |= 64 if north & 4 and east & 8 else 0
                bit = 512 << (flags & 3)
                c[5] = c[5] | bit if (flags ^ upper[4]) & 3 else c[5] & (~bit & 0xbfff)
                c[4] = flags | 128 if c[0] == 0 and c[1:4] == [65535]*3 and not c[5] & 8192 else flags & 0xff7f
    for c in cells[:-plane]:
        if not c[5] & (512 << (c[4] & 3)) and c[4] & 120 == 120:
            c[0] = 0
            c[5] = c[5] & 0xfffc | 16384
    return base, encode()


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--source-root', type=Path, default=ROOT)
    p.add_argument('--preview', type=Path)
    p.add_argument('--sanitized', type=Path)
    args = p.parse_args()
    if bool(args.preview) != bool(args.sanitized):
        p.error('Supply both preview builds')
    source = args.source_root.resolve()
    parent = ROOT / 'working/tests/terrain-map'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    def run(command, name, **kwargs):
        try:
            r = subprocess.run(command, capture_output=True, text=True, timeout=240, **kwargs)
        except subprocess.TimeoutExpired:
            raise RuntimeError(name+' exceeded the 240-second validation limit') from None
        (out / (name+'.stdout')).write_text(r.stdout)
        (out / (name+'.stderr')).write_text(r.stderr)
        r.check_returncode()
        return r.stdout
    def verify(phase):
        run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], 'original-'+phase)
    verify('before')
    try:
        pe = ROOT / 'working/game-nocd/Chaos.exe'
        assert sha(pe) == HASH
        root = ROOT / 'working/game-clean'
        files = sorted(x for x in root.rglob('*') if x.suffix.lower() == '.map')
        assert len(files) == 683
        paths = [pe, Path(__file__)] + files
        paths += list((source / 'reconstruction/rendering').glob('*'))
        paths += [source / x for x in ['tests/terrain-map-reference.cpp', 'tests/terrain-map-test.cpp', 'tests/world-terrain-reference.cpp', 'apps/terrain-preview/main.cpp', 'tools/decode-cfg.py', 'tools/test-terrain-preview.py']]
        codec = module('geometry_codec', source / 'tools/decode-cfg.py')
        configs = [root / f'Realms/{realm}/{realm}.cfg' for realm in ['Celtic', 'Greek', 'Medieval']]
        aliases = {}
        for config in configs:
            data = config.read_bytes()
            if not data.startswith(b';'):
                _, data = codec.decode(data)
            parsed = configparser.ConfigParser(interpolation=None, strict=False)
            parsed.read_string(data.decode('latin1'))
            for section in parsed.sections():
                if parsed.has_option(section, 'path') and parsed.has_option(section, 'spritepath'):
                    directory = root / parsed.get(section, 'path').strip().replace('\\', '/')
                    spritepath = root / parsed.get(section, 'spritepath').strip().replace('\\', '/')
                    aliases[directory] = spritepath
        catalog_for = {f: (f.parent if (f.parent / 'Terrain.ttd').is_file() else aliases[f.parent]) / 'Terrain.ttd' for f in files}
        catalogs = set(catalog_for.values())
        paths += configs
        paths += list(catalogs)
        builds = [x.resolve() for x in [args.preview, args.sanitized] if x]
        paths += builds
        if builds:
            paths += [root / 'Realms/Celtic/Forest/Terrain.spr']
        inputs = {x: sha(x) for x in paths if x.is_file()}
        codec = module('geometry_codec', source / 'tools/decode-cfg.py')
        fixtures, selected, rows = [], {}, []
        for i, f in enumerate(files):
            _, raw = codec.decode(f.read_bytes())
            catalog = catalog_for[f]
            base, final = geometry(raw, catalog.read_bytes())
            rawpath, basepath, finalpath = [out / f'map-{i:03}-{kind}.bin' for kind in ['raw', 'base', 'final']]
            for path, data in [(rawpath, raw), (basepath, base), (finalpath, final)]:
                path.write_bytes(data)
            fixtures += [str(catalog), str(rawpath), str(basepath), str(finalpath)]
            row = dict(path=str(f.relative_to(root)), catalog=str(catalog.relative_to(root)), cells=(len(raw)-76)//12,
                       initialized_sha256=hashlib.sha256(final).hexdigest())
            rows.append(row)
            if f.parent.name == 'Forest' and f.name in ['CFsec01.map', 'CFsec02.map', 'CFsec38.map']:
                selected[f.name] = (raw, final, finalpath)
            if i % 100 == 0:
                print(f'Decoded and independently initialized {i+1}/{len(files)} MAPs', flush=True)
        include = '-I'+str(source / 'reconstruction/rendering')
        model = str(source / 'reconstruction/rendering/terrain_map.cpp')
        reference = out / 'reference'
        run(['g++', '-m32', '-std=c++17', '-O2', '-fno-pie', '-no-pie', include, str(source / 'tests/terrain-map-reference.cpp'), model, '-o', str(reference)], 'compile-reference')
        original = json.loads(run([str(reference), str(pe), *fixtures], 'reference'))
        native = out / 'native-sanitized'
        run(['g++', '-std=c++17', '-O1', '-DMNM_NATIVE_ONLY', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', include, str(source / 'tests/terrain-map-reference.cpp'), model, '-o', str(native)], 'compile-native')
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        sanitized = json.loads(run([str(native), str(pe), *fixtures], 'native-sanitized', env=env))
        assert original['installed_cells'] == sanitized['installed_cells'] == sum(row['cells'] for row in rows)
        asm = run(['objdump', '-d', '-Mintel', '--start-address=0x4ecf20', '--stop-address=0x4ed43c', str(pe)], 'surfaces')
        (out / 'surfaces.asm').write_text(asm)
        asm = run(['objdump', '-d', '-Mintel', '--start-address=0x4ee684', '--stop-address=0x4eea3d', str(pe)], 'initialization')
        (out / 'initialization.asm').write_text(asm)
        images = []
        if builds:
            helper = out / 'world-reference'
            run(['g++', '-m32', '-std=c++17', include, str(source / 'tests/world-terrain-reference.cpp'), '-o', str(helper)], 'compile-world')
            decoder = module('geometry_sprites', source / 'tools/test-terrain-preview.py')
            realm = root / 'Realms/Celtic/Forest'
            sprite = {'sprite': (realm / 'Terrain.spr').read_bytes(), 'frames': {}}
            for name, (raw, final, finalpath) in selected.items():
                w, h, layers = struct.unpack_from('<3I', raw, 4)
                for recovered in [False, True]:
                    span = min(w, h) if recovered else min(20, w, h)
                    camera = [w//2, h//2, span, layers]
                    pan = [0, 0] if recovered else [256, 64]
                    origins = [0, 0, 0, 0, 512, 256] if recovered else [2*(span//2), 0, 0, 0, 0, 0]
                    for view in range(4):
                        for visibility in [False, True]:
                            index = len(images)
                            oracle = json.loads(run([str(helper), str(pe), str(realm / 'Terrain.ttd'), str(realm / 'Terrain.spr'), str(finalpath), *map(str, camera), *map(str, pan), str(int(visibility)), str(view), *map(str, origins)], f'oracle-{index}'))
                            pixels = [0x2124]*(512*256)
                            for draw in oracle['queue']:
                                if draw['kind'] == -2:
                                    continue
                                ox, oy, values = decoder.frame(sprite, draw['frame'])
                                for dx, dy, value in values:
                                    x, y = draw['x']-ox+dx, draw['y']-oy+dy
                                    if 0 <= x < 512 and 0 <= y < 256:
                                        pixels[y*512+x] = value
                            words = struct.pack('<'+'H'*len(pixels), *pixels)
                            rgba = b''.join(bytes((((v>>11)&31)*255//31, ((v>>5)&63)*255//63, (v&31)*255//31, 255)) for v in pixels)
                            for build, binary in enumerate(builds):
                                prefix = out / f'frame-{index}-{build}'
                                command = [str(binary), '--root', str(root), '--map', 'Realms/Celtic/Forest/'+name, '--world', '--initialize-terrain', '--view', str(view), '--output', str(prefix)]
                                if recovered:
                                    command += ['--recovered-camera']
                                else:
                                    command += ['--camera', ','.join(map(str, camera)), '--pan', ','.join(map(str, pan))]
                                if visibility:
                                    command += ['--visibility']
                                run(command, f'native-{index}-{build}', env=env)
                                actual = json.loads(prefix.with_suffix('.json').read_text())
                                queue = []
                                for draw in actual['queue']:
                                    d = {k: v for k, v in draw.items() if k not in ['tile', 'role']}
                                    d['cell'] = actual['tiles'][draw['tile']]['cell']
                                    queue.append(d)
                                assert queue == oracle['queue'] and actual['remaining_surfaces'] == 0
                                assert prefix.with_suffix('.565').read_bytes() == words
                                assert actual['rgba_sha256'] == hashlib.sha256(rgba).hexdigest()
                                for tile, owner in zip(actual['tiles'], actual['owners']):
                                    c = struct.unpack_from('<6H', final, 76+12*tile['cell'])
                                    assert (tile['definition'], tile['flags8'], tile['flags10']) == (c[0], c[4], c[5])
                                    assert owner == oracle['owners'][tile['cell']]
                                images.append(dict(map=name, recovered_camera=recovered, view=view, visibility=visibility, build=build, draws=len(queue), hidden=sum(d['kind'] == -2 for d in queue), pixels_sha256=sha(prefix.with_suffix('.565')), geometry=actual['map']['terrain_geometry']))
        assert all(sha(path) == digest for path, digest in inputs.items()), 'Inputs changed'
        report = dict(all_match=True, live_validated=False, original=original, sanitized=sanitized, maps=rows, images=images,
                      source_and_input_sha256={str(p): h for p, h in inputs.items()},
                      scope='Unchanged geometry pass compares complete cells; base setup static evidence plus independent Python checks; explicit ordinary-terrain projection excludes entities/lights; scene queues/owners and independently composed SPR pixels checked')
        (out / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(dict(original=original, images=len(images)), flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
