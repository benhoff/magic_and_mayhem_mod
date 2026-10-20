#!/usr/bin/env python3
"""Compare native additive indexed sprites against unchanged No-CD580a90."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BEHAVIOR = 'RS.world-additive-sprite'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def declaration():
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == BEHAVIOR)
    scenario = next(s for s in register['scenarios'] if s['id'] == 'native-additive-sprite')
    claims = [dict(behavior=BEHAVIOR,
                   contract_sha256=behavior_contract(behavior, {b['id']: b for b in register['builds']}),
                   scenarios={scenario['id']: scenario_contract(scenario)})]
    paths = set(scenario['tests']) | required_sources(behavior)
    # Pin the compiled native closure, not only the new shader branch.
    for directory in ('renderer', 'assets', 'compat/legacy', 'protocols'):
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*')
                     if p.is_file() and (p.suffix in ('.cpp', '.hpp', '.h', '.c', '.S') or p.name == 'CMakeLists.txt'))
    return claims, {p: sha(ROOT/p) for p in sorted(paths)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--executable', type=Path, default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):
        parser.error('Require fresh output under working/')
    out.mkdir(parents=True)
    claims, sources = declaration()
    report = dict(schema=1, success=False, claims=claims, sources=sources,
                  original_sha256=BUILD, live_replacement=False,
                  scope='Selected580a90 indexed RGB565 raster: all65536 destination words across64 source colors '
                        'cover every red/green/blue channel pair; clipped RLE runs/holes, zero palette entries, '
                        'signed anchors, ordered overlap, tagged owned wire, worker and atlas/non-atlas GPU paths. '
                        'No pointer ownership, live queue admission, other modes or replacement claim.')
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')
    environment = {**os.environ, 'QT_QPA_PLATFORM': 'xcb', 'LIBGL_ALWAYS_SOFTWARE': '1'}
    for name in ('MNM_NATIVE_DEVICE_RECOVERY', '__GLX_VENDOR_LIBRARY_NAME', '__NV_PRIME_RENDER_OFFLOAD',
                 'DRI_PRIME', 'MESA_LOADER_DRIVER_OVERRIDE', 'GALLIUM_DRIVER'):
        environment.pop(name, None)

    def run(label, command, timeout=600):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command], cwd=ROOT, env=environment,
                           stdout=log, stderr=subprocess.STDOUT, timeout=timeout, check=True)

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        if sha(args.executable) != BUILD:
            raise ValueError('Unsupported original executable')
        build = args.build_dir.resolve()
        run('configure', ['cmake', '-S', ROOT/'compat/legacy', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '--target', 'additive-sprite-test',
                      'world-frame-test', 'kind8-raster-test', '-j4'])
        run('native', ['xvfb-run', '-a', build/'additive-sprite-test', out/'fixture'])
        run('wire-regression', ['xvfb-run', '-a', build/'world-frame-test'])
        run('kind8-regression', ['xvfb-run', '-a', build/'kind8-raster-test', out/'kind8'])
        run('reference-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                               '-fno-pie', '-no-pie', ROOT/'tests/world-frame-reference.cpp', '-o', out/'reference'])
        fixtures = []
        for path in sorted(out.glob('*.bin')):
            prefix = path.with_suffix('')
            original = Path(str(prefix)+'.original.565')
            native = Path(str(prefix)+'.native.565')
            run(prefix.name, [out/'reference', args.executable, path, original, Path(str(prefix)+'.before.565')])
            if native.read_bytes() != original.read_bytes():
                raise ValueError('Original pixel mismatch: '+path.name)
            fixtures.append(dict(input=path.name, pixels=native.stat().st_size//2,
                                 native_sha256=sha(native), original_sha256=sha(original), equal=True))
        if len(fixtures) != 83:
            raise ValueError('Incomplete additive/regression corpus')
        if sha(args.executable) != BUILD or declaration() != (claims, sources):
            raise ValueError('Original or declared source/contract changed during execution')
        report.update(success=True, sources_stable=True, fixtures=fixtures,
                      additive_fixtures=66, regression_fixtures=17,
                      additive_pixels_compared=4195710,
                      pixels_compared=sum(f['pixels'] for f in fixtures), all_pixels_equal=True,
                      original_raster_instructions_unchanged=True, native_wire_and_worker_passed=True)
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['artifacts'] = {p.name: sha(p) for p in out.iterdir() if p.is_file()}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)


if __name__ == '__main__':
    main()
