#!/usr/bin/env python3
"""Exercise actual PE32 queue forwarding, refused/empty queues and raw kind retention."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    spec = importlib.util.spec_from_file_location('queues', ROOT/'tools/inspect-startup-queues.py')
    inspector = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(inspector)
    paths = [*sorted((ROOT/'runtime/scene').glob('*.[chS]')), ROOT/'runtime/shadow/win32_min.h',
             ROOT/'tools/build-scene-observer.py', ROOT/'tools/test-startup-queues.py',
             ROOT/'tools/inspect-startup-queues.py', ROOT/'tests/startup-queue-test.c',
             ROOT/'protocols/include/mnm/scene_snapshot_v1.h', ROOT/'protocols/include/mnm/world_frame_v1.h',
             ROOT/'protocols/include/mnm/world_channel_v1.h']
    claims = json.loads(args.claims.read_text()) if args.claims else None
    if claims:
        for source, digest in claims['sources'].items():
            path = ROOT/source
            if not path.resolve().is_relative_to(ROOT) or sha(path) != digest:
                raise RuntimeError('Coverage declaration source changed: '+source)
            paths.append(path)
    paths = sorted(set(paths))
    fingerprints = {str(p.relative_to(ROOT)): sha(p) for p in paths}
    parent = ROOT/'working/tests/startup-queues'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    capture = out/'capture'
    capture.mkdir()
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(ROOT/'working/tests/scene-selftest-wine'), WINEDEBUG='-all',
               MNM_SCENE_DIR='Z:'+str(capture).replace('/', '\\'), MNM_SCENE_SAMPLES='2', MNM_STARTUP_QUEUES='8')
    def run(name, command):
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command], cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=180, check=True)
    run('build', ['python3', ROOT/'tools/build-scene-observer.py', '--selftest'])
    build = ROOT/'working/build/scene-observer-selftest'
    run('compile', ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
                    '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
                    '-c', ROOT/'tests/startup-queue-test.c', '-o', out/'fixture.obj'])
    run('link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib', '/safeseh:no',
                 '/timestamp:0', '/out:'+str(out/'fixture.exe'), out/'fixture.obj', build/'scene.lib', build/'kernel32.lib'])
    (out/'MnmScene.dll').write_bytes((build/'MnmScene.dll').read_bytes())
    run('forwarding', ['wine', out/'fixture.exe'])
    result = inspector.collect(capture, 8)
    assert [q['status'] for q in result['queues']] == [3, 0, 0, 2, 1, 0, 0, 0]
    assert [q['sample'] for q in result['queues']] == [0, 1, 0, 0, 0, 2, 0, 0]
    assert result['queues'][2]['rows_captured'] == 0 and result['queues'][-1]['view'] == 19
    raw = (capture/'startup-queue-0002.bin').read_bytes()
    decoded = inspector.decode(raw)
    assert len(decoded['raw_rows']) == 45
    for i, row in enumerate(decoded['raw_rows']):
        assert all(row[j] == (0 if j == 1 else i*100+j) for j in range(9) if j != 6)
    observed = {item['kind'] for item in decoded['unsupported_draws']}
    expected = (set(range(41)) | {-2, -1, 0x7fffffff, -0x80000000}) - inspector.ADMITTED
    assert observed == expected
    replay = inspector.decode(raw, startup_replay=True)
    assert {d['kind'] for d in replay['unsupported_draws']} == expected - {16, 17, 20}
    assert {d['kind'] for d in replay['default_policy_unsupported_draws']} == expected
    assert inspector.collect(capture, 8, startup_replay=True)['default_policy_unsupported_kind_counts'] == result['unsupported_kind_counts']
    kinds = {inspector.signed(int(v, 0)) for v in re.findall(r'kind==(0x[0-9a-f]+|[0-9]+)', (ROOT/'runtime/scene/world_kinds.h').read_text())}
    assert kinds == inspector.ADMITTED
    scenes = sorted(capture.glob('scene-*.bin'))
    assert len(scenes) == 2
    refusals = 0
    damaged = [raw[:63], raw[:-1], b'BADMAGIC'+raw[8:]]
    for offset, value in [(8, 2), (12, 32), (16, len(raw)+1), (20, 0), (24, 17), (28, 4), (52, 32), (56, 44), (60, 1)]:
        bad = bytearray(raw)
        struct.pack_into('<I', bad, offset, value)
        damaged.append(bytes(bad))
    for bad in damaged:
        try:
            inspector.decode(bad)
        except ValueError:
            refusals += 1
        else:
            raise AssertionError('Malformed queue admitted')
    try:
        inspector.collect(capture, 9)
    except ValueError:
        refusals += 1
    else:
        raise AssertionError('Missing startup queue admitted')
    assert fingerprints == {str(p.relative_to(ROOT)): sha(p) for p in paths}
    report = {'success': True, 'sources': fingerprints, 'sources_stable': True,
              'scope': 'Synthetic PE32 original-compatible queue entry, every raw row/kind, empty/refused queues, entries beyond sample budget, exact prefix cap and forwarding EAX/LastError; no original raster equivalence.',
              'forwarding_passed': True, 'queues': result, 'malformed_refusals': refusals,
              'binaries': {p.name: sha(p) for p in [out/'fixture.exe', out/'MnmScene.dll']}}
    if claims:
        report['claims'] = claims['claims']
    with (out/'report.json').open('x') as output:
        json.dump(report, output, indent=2)
        output.write('\n')
    print(out/'report.json', flush=True)


if __name__ == '__main__':
    main()
