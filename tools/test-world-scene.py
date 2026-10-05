#!/usr/bin/env python3
"""Bounded offline mixed-scene pixels and fresh-process presentation continuation."""
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
    if len(args) != 2:
        raise ValueError('Supply scene executable and world-sandbox executable')
    scene, sandbox = map(lambda p: str(Path(p).resolve()), args)
    parent = ROOT/'working/tests/world-scene';parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent));print(out, flush=True)
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
        comparisons = 0;continuations = 0;rows = []
        for profile, start, goal, region in [
            ('terrace', (1,1,1), (5,1,1), '0,0,1,8,4'),
            ('slope', (1,1,1), (5,1,2), '0,0,1,8,4'),
            ('vertical', (1,1,2), (1,1,4), '0,0,2,4,4')]:
            frozen = out/f'{profile}.bin';frozen.write_bytes(fixture.terrain_fixture(profile))
            initial = out/f'{profile}.mnms'
            run([sandbox, 'move-terrain-ani', frozen, animation, 8, initial, *start, *goal, 0])
            trace = [json.loads(line) for line in run([sandbox, 'trace', initial, 11]).splitlines()]
            for view in range(4):
                prefix = out/f'{profile}-v{view}'
                options = ['--root', assets, '--region', region, '--view', view]
                run([scene, '--checkpoint', initial, *options, '--output', prefix, '--frames', 12])
                frames = json.loads(Path(str(prefix)+'.json').read_text())['frames']
                for i, frame in enumerate(frames):
                    assert frame['tick'] == trace[i]['tick']
                    assert frame['actors'][0]['fine'] == trace[i]['entities'][0]['fine']
                    keys = [d['key'] for d in frame['queue']];assert keys == sorted(keys)
                    body = [d for d in frame['queue'] if d['creature']]
                    assert len(body) <= 1
                    if body:
                        fx, fy, fz = frame['actors'][0]['fine']
                        rx, ry, _, width, height = map(int, region.split(','))
                        dx, dy = fx+16-(rx*32+width*16), fy+16-(ry*32+height*16)
                        px, py = [(dx,dy),(dy,-dx),(-dx,-dy),(-dy,dx)][view]
                        assert (body[0]['x'], body[0]['y']) == (256+px-py, 160+int((px+py)/2)-fz)
                        depth = [fx+fy+32, fy-fx, -fx-fy-32, fx-fy][view]
                        assert body[0]['key'] == depth+fz*82//100+6
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
        report = {'all_match': True, 'live_validated': False, 'pixel_comparisons': comparisons,
                  'fresh_process_continuations': continuations, 'runs': rows,
                  'scope': 'Explicit diagnostic projection/TTD body fixture and synthetic owned ANI; installed SPR pixels; no installed MAP navigation or original mixed-scene comparison',
                  'sha256': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(scene), Path(sandbox), *paths, animation]}}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(json.dumps({'pixel_comparisons': comparisons, 'continuations': continuations, 'all_match': True}))
    finally:
        verify('after')

if __name__ == '__main__':
    main()
