#!/usr/bin/env python3
"""Multiple native actor pixels, identity and fresh-process presentation continuation."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT = Path(__file__).resolve().parents[1]

def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec);spec.loader.exec_module(result)
    return result

def main():
    if '--inner' not in sys.argv:
        subprocess.run(['xvfb-run', '-a', sys.executable, __file__, *sys.argv[1:], '--inner'], check=True, timeout=300)
        return
    args = [a for a in sys.argv[1:] if a != '--inner']
    if len(args) != 3:
        raise ValueError('Supply scene executable, world-sandbox executable and new output directory')
    scene, sandbox = map(lambda p: str(Path(p).resolve()), args[:2])
    out = Path(args[2]).resolve();out.mkdir(parents=True, exist_ok=False);print(out, flush=True)
    os.environ.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
    def run(command):
        p = subprocess.run(list(map(str, command)), capture_output=True, text=True, timeout=120)
        if p.returncode or 'runtime error:' in p.stderr or 'AddressSanitizer' in p.stderr:
            raise RuntimeError((command, p.stdout, p.stderr))
        return p.stdout
    def verify(phase):
        (out/f'manifest-{phase}.log').write_text(run([ROOT/'tools/original-manifest.sh', 'verify']))
    verify('before')
    try:
        fixture = module('movement_fixture', ROOT/'tools/create-movement-fixture.py')
        ani = module('scene_ani', ROOT/'tests/test-native-ani-motion.py')
        pixels = module('scene_pixels', ROOT/'tools/test-terrain-preview.py')
        assets = ROOT/'working/game-clean'
        paths = [assets/'Realms/Celtic/Forest/Terrain.spr', assets/'Realms/Celtic/Forest/Terrain.ttd', assets/'Creatures/RedCap.spr']
        terrain = {'sprite': paths[0].read_bytes(), 'frames': {}}
        creature = {'sprite': paths[2].read_bytes(), 'frames': {}}
        animation = out/'movement.ani';animation.write_bytes(ani.ani())
        comparisons = 0;continuations = 0;rows = [];two_visible = 0;independent_frames = 0
        for profile, start, goal, other_start, other_goal, region in [
            ('terrace', (1,1,1), (5,1,1), (5,3,1), (1,3,1), '0,0,1,8,4'),
            ('slope', (1,1,1), (5,1,2), (1,3,1), (5,3,2), '0,0,1,8,4'),
            ('vertical', (1,1,2), (1,1,4), (3,3,2), (3,3,4), '0,0,2,4,4')]:
            frozen = out/f'{profile}.bin';frozen.write_bytes(fixture.terrain_fixture(profile))
            initial = out/f'{profile}.mnms'
            run([sandbox, 'move-pair-terrain-ani', frozen, animation, 8, initial, *start, *goal, *other_start, *other_goal, 1])
            # Owned fixture-only mutation: verify the envelope and original generation,
            # then give slot zero a value beyond signed 32-bit and recompute its digest.
            raw = bytearray(initial.read_bytes())
            assert raw[:8] == b'MNMNWLD\0' and struct.unpack_from('<I', raw, 8)[0] == 6
            assert struct.unpack_from('<I', raw, 12)[0] == len(raw)-24
            def digest(data):
                value = 14695981039346656037
                for byte in data:value = ((value ^ byte)*1099511628211) & ((1 << 64)-1)
                return value
            assert struct.unpack_from('<Q', raw, 16)[0] == digest(raw[24:])
            at = 24+20
            for _ in range(3):at += 4+struct.unpack_from('<I', raw, at)[0]
            assert raw[at] == 1;at += 21
            assert raw[at] == 1;at += 1
            at += 4+struct.unpack_from('<I', raw, at)[0]+4
            assert struct.unpack_from('<I', raw, at)[0] == 8;at += 4
            assert struct.unpack_from('<I', raw, at)[0] == 1 and raw[at+4] == 1
            struct.pack_into('<I', raw, at, 0xfedcba98)
            struct.pack_into('<Q', raw, 16, digest(raw[24:]))
            initial.write_bytes(raw)
            trace = [json.loads(line) for line in run([sandbox, 'trace', initial, 11]).splitlines()]
            for view in range(4):
                prefix = out/f'{profile}-v{view}'
                options = ['--root', assets, '--region', region, '--view', view]
                run([scene, '--checkpoint', initial, *options, '--output', prefix, '--frames', 12])
                frames = json.loads(Path(str(prefix)+'.json').read_text())['frames']
                for i, frame in enumerate(frames):
                    assert frame['tick'] == trace[i]['tick']
                    actors = {a['slot']: a for a in frame['actors']}
                    assert set(actors) == {0, 1}
                    for entity in trace[i]['entities']:
                        assert actors[entity['slot']]['fine'] == entity['fine']
                        assert actors[entity['slot']]['generation'] == entity['generation']
                    keys = [d['key'] for d in frame['queue']];assert keys == sorted(keys)
                    body = [d for d in frame['queue'] if d['creature']]
                    assert len({d['slot'] for d in body}) == len(body) <= 2
                    two_visible += len(body) == 2
                    independent_frames += len(body) == 2 and body[0]['frame'] != body[1]['frame']
                    for draw in body:
                        entity = next(e for e in trace[i]['entities'] if e['slot'] == draw['slot'])
                        if 'animation' in entity:
                            sequence, _, displayed, *_ = entity['animation']
                            assert draw['frame'] == 99+sequence+displayed
                        actor = actors[draw['slot']]
                        assert draw['generation'] == actor['generation']
                        fx, fy, fz = actor['fine']
                        rx, ry, _, width, height = map(int, region.split(','))
                        dx, dy = fx+16-(rx*32+width*16), fy+16-(ry*32+height*16)
                        px, py = [(dx,dy),(dy,-dx),(-dx,-dy),(-dy,dx)][view]
                        assert (draw['x'], draw['y']) == (256+px-py, 160+int((px+py)/2)-fz)
                        depth = [fx+fy+32, fy-fx, -fx-fy-32, fx-fy][view]
                        assert draw['key'] == depth+fz*82//100+6
                    expected = [0x2124]*(512*256)
                    for draw in frame['queue']:
                        asset = creature if draw['creature'] else terrain
                        ox, oy, opaque = pixels.frame(asset, draw['frame'])
                        for x, y, value in opaque:
                            x += draw['x']-ox;y += draw['y']-oy
                            if 0 <= x < 512 and 0 <= y < 256:
                                expected[y*512+x] = value
                    image = Path(f'{prefix}-{i:03}.565')
                    assert image.read_bytes() == struct.pack('<'+'H'*len(expected), *expected)
                    comparisons += 1
                split = out/f'{profile}-v{view}-split'
                run([scene, '--checkpoint', initial, *options, '--output', split, '--frames', 6])
                resumed = out/f'{profile}-v{view}-resumed'
                run([scene, '--checkpoint', Path(str(split)+'.mnms'), *options, '--ticks', 1, '--output', resumed, '--frames', 6])
                resumed_frames = json.loads(Path(str(resumed)+'.json').read_text())['frames']
                assert resumed_frames == frames[6:]
                for i in range(6):
                    assert Path(f'{resumed}-{i:03}.565').read_bytes() == Path(f'{prefix}-{i+6:03}.565').read_bytes()
                    assert Path(f'{resumed}-{i:03}.png').read_bytes() == Path(f'{prefix}-{i+6:03}.png').read_bytes()
                assert Path(str(resumed)+'.mnms').read_bytes() == Path(str(prefix)+'.mnms').read_bytes()
                continuations += 1
                rows.append({'profile': profile, 'view': view, 'frames': str(prefix)+'.json'})
        assert two_visible and independent_frames, (two_visible, independent_frames)
        sources = sorted(set([*ROOT.glob('apps/world-scene/*.cpp'), *ROOT.glob('apps/world-scene/*.hpp'), *ROOT.glob('apps/world-sandbox/*.cpp'), *ROOT.glob('apps/world-sandbox/*.hpp'), *ROOT.glob('game/**/*.cpp'), *ROOT.glob('game/**/*.hpp'), ROOT/'tools/test-multi-world-scene.py', ROOT/'tests/world-scene-test.cpp', ROOT/'tools/create-movement-fixture.py', ROOT/'tests/test-native-ani-motion.py', ROOT/'apps/world-scene/CMakeLists.txt']))
        report = {'all_match': True, 'live_validated': False, 'pixel_comparisons': comparisons,
                  'fresh_process_continuations': continuations, 'runs': rows, 'two_visible_frames': two_visible, 'distinct_actor_display_frames': independent_frames, 'unsigned_generation_export': True,
                  'source_sha256': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},
                  'scope': 'Two same-profile native terrain-motion actors; four diagnostic views, synthetic owned ANI and terrain fixtures with installed SPR pixels; actor identity/independent display and exact split/whole presentation/checkpoints. No captured original multi-actor scene, installed multi-map world or live equivalence.',
                  'sha256': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(scene), Path(sandbox), *paths, animation]}}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(json.dumps({'pixel_comparisons': comparisons, 'continuations': continuations, 'all_match': True}))
    finally:
        verify('after')

if __name__ == '__main__':
    main()
