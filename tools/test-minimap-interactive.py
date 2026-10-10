#!/usr/bin/env python3
"""Validate the public bounded V2 minimap launcher, paced motion and unpaced costs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    sources = declaration['sources']
    if not all(sha(ROOT / n) == h for n, h in sources.items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT / 'working/tests/minimap-interactive'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(success=False, sources=sources, claims=declaration['claims'],
                  scope='Public startup-history V2 CPU/GPU World and minimap composition, first16 map2/zero-items queues. Paced motion coverage and separate unpaced diagnostic costs; no gameplay frame-rate claim.',
                  original_pixels_used_as_native_inputs=False, original_work_bypassed=False,
                  live_replacement=False, runs={})
    try:
        for name, motion in [('motion', True), ('unpaced', False)]:
            command = ['xvfb-run', '-a', '-s', '-screen 0 1280x1024x24',
                       'python3', ROOT/'tools/run-native-world.py', '--startup-history',
                       '--minimap-owned', '--claims', args.claims.resolve()]
            if motion:
                command.append('--minimap-motion')
            with (out/(name+'.log')).open('x') as log:
                subprocess.run([str(x) for x in command], cwd=ROOT,
                               env={**os.environ, 'LIBGL_ALWAYS_SOFTWARE':'1'},
                               stdout=log, stderr=subprocess.STDOUT, timeout=900, check=True)
            roots = [Path(line.split(': ', 1)[1]) for line in (out/(name+'.log')).read_text().splitlines()
                     if line.startswith('Native World shadow session: ')]
            if len(roots) != 1:
                raise ValueError('Missing unique launcher session')
            root = roots[0]
            native = json.loads((root/'native-world.json').read_text())
            captured = json.loads((root/'report.json').read_text())
            if not native['success'] or not captured['success'] or not captured['sources_stable']:
                raise ValueError('Failed or stale launcher session')
            report['source_executable_sha256']=captured['source_executable_sha256']
            checks = native['checkpoints']
            viewer = native['native']
            if (len(viewer['producer_frames']) != 16 or viewer['wire_version'] != 2
                    or any(c['mismatches'] for c in checks) or not viewer['presentations']
                    or viewer['viewport_image_uploads'] or viewer['remaining_surfaces']):
                raise ValueError('Incomplete native frame delivery')
            report['runs'][name] = dict(root=str(root.relative_to(ROOT)),
                checkpoints=len(checks), equal=len(checks), mismatches=0,
                pixels_compared=sum(c['width']*c['height'] for c in checks),
                fixture_pacing=native['minimap_fixture_pacing'], observed=native['minimap_owned'],
                native=viewer, original_manifest_verified_before_after=captured['original_manifest_verified_before_after'],
                records={str(p.relative_to(ROOT)):sha(p) for p in [root/'native-world.json',root/'report.json',root/'capture/canvas-producers.bin',root/'capture/canvas-producers.done']})
        with (out/'tail-refusals.log').open('x') as log:
            subprocess.run(['python3',str(ROOT/'tests/canvas-producer-tail-test.py'),str(ROOT/'working/build/canvas-producers/mnm-canvas-producers-live'),str(out/'tail-refusals')],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=180,check=True)
        report['tail_refusals']=json.loads((out/'tail-refusals/report.json').read_text())
        compiled = set()
        for build in [ROOT/'working/build/canvas-producers', ROOT/'working/build/canvas-producers/minimap-input']:
            for dep in build.rglob('*.o.d'):
                for raw in dep.read_text().replace('\\\n', ' ').split():
                    path = Path(raw).resolve()
                    try:
                        name = path.relative_to(ROOT).as_posix()
                    except ValueError:
                        continue
                    if path.is_file() and not name.startswith('working/'):
                        if name not in sources or sha(path) != sources[name]:
                            raise ValueError('Undeclared compiled dependency: '+name)
                        compiled.add(name)
        report['compiled_dependencies'] = sorted(compiled)
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
    finally:
        report['sources_stable'] = all(sha(ROOT/n) == h for n,h in sources.items())
        report['success'] = report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    if not report['success']:
        raise SystemExit(1)

if __name__ == '__main__':
    main()
