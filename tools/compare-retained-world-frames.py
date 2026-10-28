#!/usr/bin/env python3
"""Compare immutable, relocated World samples without requiring colour draws.

Native replay starts from owned zero. Captured before pixels are supplied only
to a separate original diagnostic process; they never enter native rendering.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import read_bytes, write_report

ROOT = Path(__file__).resolve().parents[1]
SCENARIO = 'world-retained-busy-comparison-20261026'


def sha(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def declaration(world, scenario_id=SCENARIO):
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == 'RS.world-frame')
    scenario = next(s for s in register['scenarios'] if s['id'] == scenario_id)
    if behavior['id'] not in scenario['behaviors'] or scenario['level'] != 'headless':
        raise ValueError('Selected scenario does not declare the World headless comparison')
    builds = {b['id']: b for b in register['builds']}
    paths = (set(world.sources()) | required_sources(behavior) | set(scenario['tests'])
             | {'tools/coverage_evidence.py', 'tools/coverage_claims.py', 'tools/evidence_local.py'})
    claims = [{'behavior': behavior['id'], 'contract_sha256': behavior_contract(behavior, builds),
               'scenarios': {scenario_id: scenario_contract(scenario)}}]
    return claims, {p: sha(ROOT/p) for p in sorted(paths)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('capture-report', 'capture-directory', 'build-dir', 'output-dir', 'report'):
        parser.add_argument('--'+option, type=Path, required=True)
    parser.add_argument('--scenario',default=SCENARIO,help='Exact registered headless World comparison scope')
    parser.add_argument('--current-capture',action='store_true',help='Require every acquisition source fingerprint to match current code')
    args = parser.parse_args()
    out = args.output_dir.resolve()
    out.mkdir(parents=True, exist_ok=False)
    if args.report.exists():
        raise ValueError('Use a new immutable report path')
    spec = importlib.util.spec_from_file_location('world_frames', ROOT/'tools/test-world-frames.py')
    world = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(world)
    claims, sources = declaration(world,args.scenario)
    result = dict(schema=1, success=False, claims=claims, sources=sources, frames=[],
                  source_executable_sha256=world.BUILD_HASH,
                  current_source_capture_required=args.current_capture,
                  scope='Fresh finite native/isolated-original replay of immutable captured World inputs. '
                        'Zero native initialization; separate original-before diagnostic. No normal game journey, '
                        'normal Quit, continuous presentation, hardware or replacement acceptance.')
    (out/'prospective.json').write_text(json.dumps(result, indent=2)+'\n')
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')

    def run(name, command):
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT, timeout=600, check=True)

    inputs = {}

    def pin(path):
        path = Path(path).resolve()
        inputs[str(path.relative_to(ROOT))] = sha(path)

    def mismatches(a, b):
        if len(a) != len(b) or len(a) % 2:
            raise ValueError('RGB565 output extent differs')
        return sum(x != y for x, y in zip(struct.iter_unpack('<H', a), struct.iter_unpack('<H', b)))

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        capture_bytes = read_bytes(args.capture_report, root=ROOT)
        capture = json.loads(capture_bytes)
        directory = args.capture_directory.resolve()
        if not (directory/'game').is_dir():
            raise ValueError('Capture game assets are retired or absent; restore its environment-game.sqlite3 before comparison')
        if (directory/'report.json').read_bytes() != capture_bytes:
            raise ValueError('Relocated report differs from retained capture')
        if not (capture['success'] and capture['sources_stable']
                and capture['source_executable_sha256'] == world.BUILD_HASH):
            raise ValueError('Requires an accepted pinned original capture')
        result['capture_report_sha256'] = hashlib.sha256(capture_bytes).hexdigest()
        result['historical_capture_source_drift'] = [p for p, h in capture['sources'].items()
                                                   if not (ROOT/p).is_file() or sha(ROOT/p) != h]
        if args.current_capture and result['historical_capture_source_drift']:
            raise ValueError('Current acquisition sources changed after capture')
        pin(directory/'report.json')
        paths = sorted(directory/p for p in capture['world_frames'] if p.endswith('.bin'))
        if not 1 <= len(paths) <= 16 or len(paths) != capture['samples']:
            raise ValueError('Requires every declared sample, bounded to sixteen')
        if set(directory.joinpath('capture').glob('world-*.bin')) != set(paths):
            raise ValueError('Undeclared or missing World sample')
        for name, digest in capture['world_frames'].items():
            path = directory/name
            if sha(path) != digest:
                raise ValueError('Retained input changed: '+name)
            pin(path)
        needed = {world.identity(raw, h[9]) for p in paths for h, raw in world.records(p.read_bytes())
                  if h[1] not in (6, 7)}
        bindings = out/'bindings.json'
        data = world.pinned_bindings(directory/'game', needed)
        bindings.write_text(json.dumps(data, indent=2)+'\n')
        pin(bindings)
        for item in data:
            pin(directory/'game'/item['sprite'])
        pe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(pe) != world.BUILD_HASH:
            raise ValueError('Original executable hash differs')
        result['entry_anchors'] = world.reference_anchor(pe)
        build = args.build_dir.resolve()
        run('configure', ['cmake', '-S', ROOT/'compat/legacy', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '--target', 'mnm-world-frame-preview', '-j2'])
        reference = out/'reference'
        run('reference-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                                '-fno-pie', '-no-pie', ROOT/'tests/world-frame-reference.cpp', '-o', reference])
        preview = build/'mnm-world-frame-preview'
        result['binaries'] = {str(p.relative_to(ROOT)): sha(p) for p in (preview, reference)}
        for path in paths:
            prefix = out/path.stem
            run(path.stem+'-native', ['xvfb-run', '-a', preview, '--root', directory/'game',
                                     '--snapshot', path, '--bindings', bindings, '--output', prefix])
            zero = out/(path.stem+'-original-zero.565')
            prior = out/(path.stem+'-original-before.565')
            run(path.stem+'-original-zero', [reference, pe, path, zero])
            run(path.stem+'-original-before', [reference, pe, path, prior, path.with_suffix('.before.565')])
            metadata = json.loads(prefix.with_suffix('.json').read_bytes())
            if not metadata['success'] or metadata['background'] != 0 or metadata['remaining_surfaces']:
                raise ValueError('Native replay incomplete or surface ownership leaked')
            wire = path.read_bytes()
            width, height, stride = struct.unpack_from('<3I', wire, 24)
            live = path.with_suffix('.after.565').read_bytes()
            if len(live) != stride*height*2:
                raise ValueError('Captured live extent differs')
            cropped = b''.join(live[y*stride*2:(y*stride+width)*2] for y in range(height))
            native = prefix.with_suffix('.565').read_bytes()
            row = dict(sequence=path.stem, width=width, height=height, draws=metadata['records'],
                       operations=metadata['operations'], pixels=width*height, driver=metadata['driver'],
                       native_zero_original_mismatches=mismatches(native, zero.read_bytes()),
                       diagnostic_original_live_mismatches=mismatches(prior.read_bytes(), cropped),
                       native_zero_live_mismatches=mismatches(native, cropped),
                       native_sha256=sha(prefix.with_suffix('.565')), original_zero_sha256=sha(zero),
                       diagnostic_original_sha256=sha(prior), live_cropped_sha256=hashlib.sha256(cropped).hexdigest())
            result['frames'].append(row)
            if row['native_zero_original_mismatches'] or row['diagnostic_original_live_mismatches']:
                raise ValueError('Native arithmetic or original observation closure differs: '+path.stem)
        if declaration(world,args.scenario) != (claims, sources) or any(sha(ROOT/p) != h for p, h in inputs.items()):
            raise ValueError('Execution source, scope or retained input changed')
        result.update(success=True, sources_stable=True, inputs_stable=True, inputs=inputs,
                      samples_compared=len(paths), draws_compared=sum(f['draws'] for f in result['frames']),
                      pixels_compared=sum(f['pixels'] for f in result['frames']),
                      independent_original_pixels_match=True, diagnostic_original_live_match=True,
                      live_original_pixels_match=all(f['native_zero_live_mismatches'] == 0 for f in result['frames']),
                      original_pixels_used_as_native_inputs=False, fresh_live_execution=False,
                      original_work_bypassed=False, captured_frame_identities=len(needed), native_asset_files=len(data))
    except Exception as error:
        result['error'] = str(error)
        raise
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            result['original_manifest_verified_before_after'] = True
        finally:
            write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
            print('success:', result['success'], 'samples:', len(result['frames']), flush=True)


if __name__ == '__main__':
    main()
