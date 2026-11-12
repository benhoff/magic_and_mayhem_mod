#!/usr/bin/env python3
"""Replay retained source/command inputs with an explicit offline observer.

Run under the repository storage guard with TMPDIR on checked bounded staging.
The binary accepts source history, commands and assets only. Output observations
are read here afterwards; they cannot influence native execution.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess

from coverage_evidence import Reader, write_report
from copy_tail_replay import compare_records, compare_cpu, offline_observer
from rendering_pilot import capture, require


def sha(data):
    return hashlib.sha256(data).hexdigest()


def closure(root, build):
    paths = set()
    for dep in build.rglob('*.o.d'):
        obj = dep.with_suffix('')
        require(obj.is_file(), 'Compiler object missing')
        for name in shlex.split(dep.read_text().replace('\\\n', ' ').split(':', 1)[1]):
            p = Path(name).resolve()
            if p.is_relative_to(root):
                require(p.stat().st_mtime_ns <= obj.stat().st_mtime_ns,
                        'Compiler dependency changed after compilation: ' + str(p))
                paths.add(p.relative_to(root).as_posix())
    require('renderer/command_consumer.cpp' in paths and 'tests/copy-tail-replay/main.cpp' in paths,
            'Incomplete compiler closure')
    for directory in ('tools', 'tests/copy-tail-replay'):
        for p in (root/directory).glob('*.py'):
            if directory != 'tools' or p.name in ('copy_tail_replay.py', 'run-copy-tail-replay.py',
                    'trace-copy-tail.py', 'native_copy_routes.py', 'native_gpu_copy_diagnostics.py',
                    'rendering_pilot.py', 'rendering_capture.py', 'coverage_evidence.py'):
                paths.add(p.relative_to(root).as_posix())
    paths.add('tests/copy-tail-replay/CMakeLists.txt')
    paths.add('renderer/command_copy_diagnostics.cpp')
    paths.add('tests/test-copy-tail-replay.py')
    return {p: sha((root/p).read_bytes()) for p in sorted(paths)}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    p.add_argument('--build', type=Path, required=True)
    p.add_argument('--stage', type=Path, required=True)
    p.add_argument('--assets', type=Path, required=True)
    p.add_argument('--acquisition', required=True)
    p.add_argument('--output', required=True)
    args = p.parse_args()
    root, build, stage = args.root.resolve(), args.build.resolve(), args.stage.resolve()
    require(not stage.exists(), 'New bounded replay stage required')
    stage.mkdir(parents=True)
    output = stage/'output'
    result = dict(schema=1, kind='offline-source-only-copy-replay', success=False,
                  original_pixels_used_as_native_inputs=False, historical_capture_complete=False,
                  boundary='Native replay self-consistency anchored to actual captured outputs. This cannot recover unwitnessed historical execution, assert original COPY equivalence or promote whole-operation/player acceptance.')
    manifest_checked = False
    sources = {}
    try:
        subprocess.run([root/'tools/original-manifest.sh', 'verify'], check=True)
        manifest_checked = True
        sources = closure(root, build)
        binary = build/'copy-tail-replay'
        binary_hash = sha(binary.read_bytes())
        variant = (build/'offline-observer.cpp').read_text()
        require(variant == offline_observer((root/'renderer/command_copy_diagnostics.cpp').read_text()),
                'Offline observer preimage differs')
        link = (build/'replay.map').read_text()
        require('command_copy_diagnostics.cpp.o' not in link and 'offline-observer.cpp.o' in link,
                'Production diagnostic observer unexpectedly linked')
        inputs = {}
        with Reader(root) as reader:
            acquired = reader.json(args.acquisition)
            result['acquisition'] = dict(record=args.acquisition, sha256=reader.digest(args.acquisition))
            def read(suffix):
                selected = [a for a in acquired['artifacts'] if a['path'].endswith(suffix)]
                require(len(selected) == 1, 'Unique retained input required')
                a = selected[0]
                data = reader.read_bytes(a['record'])
                require(len(data) == a['bytes'] and sha(data) == a['sha256'], 'Retained input changed')
                inputs[suffix] = dict(record=a['record'], sha256=a['sha256'], bytes=a['bytes'])
                return data
            rows = capture(read('/commands-2.bin'))
            command_file = stage/'commands.bin'
            command_file.write_bytes(b''.join(b for r, b in rows if r['kind'] == 'fragment'))
            profile = stage/'worlds.bin'
            profile.write_bytes(read('/worlds.bin'))
            captured = [json.loads(line) for line in read('/copy-diagnostics/copies.jsonl').splitlines()]
        result['inputs'] = inputs
        # Preserve exact fixture byte identities. No executable is launched and
        # no original destination/reference plane is accepted by the driver.
        assets = {p.relative_to(args.assets).as_posix(): sha(p.read_bytes())
                  for p in args.assets.rglob('*') if p.is_file()}
        require(bool(assets), 'Missing asset fixture')
        result.update(sources=sources, asset_fixture=assets, binary_sha256=binary_hash,
                      offline_observer_sha256=sha(variant.encode()))
        env = dict(os.environ, LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='4', QT_QPA_PLATFORM='xcb')
        command = ['xvfb-run', '-a', '-s', '-screen 0 1024x768x24', str(binary),
                   str(profile), str(command_file), str(args.assets.resolve()), str(output)]
        result['command'] = command
        with (stage/'execution.log').open('xb') as log:
            done = subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=900)
        result['exit_code'] = done.returncode
        require(done.returncode == 0, 'Native replay failed; inspect retained execution log')
        diagnostic = json.loads((output/'copies/report.json').read_bytes())
        native = json.loads((output/'native.json').read_bytes())
        require(native['success'] and diagnostic['success'] and not diagnostic['truncated']
                and diagnostic['ended'] and diagnostic['copies_seen'] == diagnostic['copies_checked'],
                'Incomplete offline diagnostic')
        replayed = [json.loads(line) for line in (output/'copies/copies.jsonl').read_bytes().splitlines()]
        result['comparison'] = compare_records(captured, replayed, diagnostic['copies_seen'])
        result['cpu_reply_anchors'] = compare_cpu(rows, [json.loads(line) for line in (output/'cpu.jsonl').read_bytes().splitlines()])
        result.update(success=True, native=native, diagnostics=diagnostic)
        require(sources == closure(root, build) and sha(binary.read_bytes()) == binary_hash,
                'Replay source/binary changed during execution')
        require(all(sha((args.assets/n).read_bytes()) == h for n, h in assets.items()), 'Asset fixture changed')
        result['sources_stable'] = True
    except Exception as e:
        result['error'] = str(e)
    finally:
        if manifest_checked:
            result['original_manifest_verified_before_after'] = subprocess.run([root/'tools/original-manifest.sh', 'verify']).returncode == 0
            result['success'] &= result['original_manifest_verified_before_after']
        prefix = args.output.removesuffix('.json')
        artifacts = []
        selected = [p for p in output.rglob('*') if p.is_file()] if output.exists() else []
        selected += [stage/'execution.log', build/'offline-observer.cpp', build/'replay.map', build/'copy-tail-replay', build/'compile_commands.json']
        require(sum(f.stat().st_size for f in selected if f.is_file()) < 256*1024**2, 'Bounded replay evidence exceeded')
        for index, path in enumerate(selected):
            if not path.is_file():
                continue
            data = path.read_bytes()
            record = prefix+'-raw-'+str(index)+'.bin'
            write_report(root, record, data)
            artifacts.append(dict(name=path.relative_to(output).as_posix() if path.is_relative_to(output) else path.name,
                                  record=record, sha256=sha(data), bytes=len(data)))
        result['artifacts'] = artifacts
        write_report(root, args.output, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps({k: result[k] for k in ('success', 'comparison', 'cpu_reply_anchors', 'error') if k in result}))
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
