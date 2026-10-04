#!/usr/bin/env python3
"""Hash-pinned isolated camera setter comparisons and optional Qt scene smoke tests."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source-root', type=Path, default=ROOT)
    parser.add_argument('--preview', type=Path)
    parser.add_argument('--sanitized', type=Path)
    args = parser.parse_args()
    if bool(args.preview) != bool(args.sanitized):
        parser.error('Supply both preview builds')
    (ROOT / 'working/tests/terrain-camera').mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=ROOT / 'working/tests/terrain-camera'))
    print(out, flush=True)
    sources = ['tests/terrain-camera-reference.cpp', 'tests/terrain-camera-test.cpp',
               'reconstruction/rendering/terrain_camera.cpp', 'reconstruction/rendering/terrain_camera.hpp',
               'reconstruction/rendering/terrain_traversal.hpp', 'apps/terrain-preview/main.cpp']
    sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    hashes = {p: sha(args.source_root / p) for p in sources}
    installed = {p: sha(ROOT / 'working/game-clean/Realms/Celtic/Forest' / p) for p in ['CFsec01.map', 'Terrain.ttd', 'Terrain.spr']} if args.preview else {}
    builds = {str(p): sha(p) for p in [args.preview, args.sanitized] if p}
    def verify(phase):
        result = subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], capture_output=True, text=True, timeout=120)
        (out / f'original-{phase}.log').write_text(result.stdout + result.stderr)
        result.check_returncode()
    verify('before')
    try:
        pe = ROOT / 'working/game-nocd/Chaos.exe'
        if sha(pe) != HASH:
            raise ValueError('Unsupported executable')
        include = args.source_root / 'reconstruction/rendering'
        subprocess.run(['g++', '-m32', '-std=c++17', '-O2', '-fno-pie', '-no-pie', '-I' + str(include),
                        str(args.source_root / sources[0]), str(include / 'terrain_camera.cpp'), '-o', str(out / 'reference')], check=True)
        reference = subprocess.check_output([str(out / 'reference'), str(pe)], text=True, timeout=120)
        (out / 'reference.json').write_text(reference)
        subprocess.run(['g++', '-std=c++17', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-I' + str(include),
                        str(args.source_root / sources[1]), str(include / 'terrain_camera.cpp'), '-o', str(out / 'sanitized-test')], check=True)
        subprocess.run([str(out / 'sanitized-test')], check=True, timeout=120)
        asm = subprocess.check_output(['objdump', '-d', '-Mintel', '--start-address=0x4f7860', '--stop-address=0x4f7d00', str(pe)], text=True)
        (out / 'camera.asm').write_text(asm)
        scenes = []
        if args.preview:
            for view in range(4):
                for trace in [[], ['-32,0'], ['19,-7', '-91,41'], ['2048,-2048']]:
                    images = []
                    for build, name in [(args.preview, 'normal'), (args.sanitized, 'sanitized')]:
                        prefix = out / f'scene-{view}-{len(scenes)}-{name}'
                        command = [str(build.resolve()), '--root', str(ROOT / 'working/game-clean'), '--map',
                                   'Realms/Celtic/Forest/CFsec01.map', '--world', '--recovered-camera', '--view', str(view),
                                   '--visibility', '--output', str(prefix)]
                        for step in trace:
                            command += ['--scroll', step]
                        subprocess.run(command, check=True, timeout=120)
                        report = json.loads(prefix.with_suffix('.json').read_text())
                        if report['remaining_surfaces'] != 0 or not report['map']['recovered_camera']:
                            raise ValueError('Camera scene lifecycle mismatch')
                        images.append((sha(prefix.with_suffix('.565')), report))
                    if images[0] != images[1]:
                        raise ValueError('Normal/sanitized scene mismatch')
                    scenes.append(dict(view=view, scroll=trace, pixels_sha256=images[0][0], queue=len(images[0][1]['queue'])))
        if sha(pe) != HASH or hashes != {p: sha(args.source_root / p) for p in sources}:
            raise ValueError('Inputs changed during validation')
        if installed != {p: sha(ROOT / 'working/game-clean/Realms/Celtic/Forest' / p) for p in installed} or builds != {p: sha(Path(p)) for p in builds}:
            raise ValueError('Installed inputs or builds changed')
        report = dict(installed_sha256=installed, build_sha256=builds, executable_sha256=HASH, source_sha256=hashes, reference=json.loads(reference), scenes=scenes,
                      scope='Original selected camera setters; native scene smoke checks compare normal/sanitized builds, not original scene pixels',
                      artifacts_sha256={p.name: sha(p) for p in out.iterdir() if p.is_file()})
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report['reference']), flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
