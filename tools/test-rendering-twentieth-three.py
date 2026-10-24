#!/usr/bin/env python3
"""Bounded map geometry, palette preference and mode-zero shading comparisons."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import subprocess

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
SELECTED = ['RE.terrain_map', 'RE.palette_preferences', 'RE.palette_shading']
POLICY = 'NP.palette-option-strict-parsing'
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--asset-root', type=Path, required=True)
    args = parser.parse_args()
    out, report_path = args.output.resolve(), args.report.absolute()
    if (out.exists() or not out.is_relative_to(ROOT/'working/tests') or report_path.exists()
            or not report_path.is_relative_to(ROOT/'research/runtime')):
        parser.error('Require new checkout-local execution and research report paths')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain bounded rendering controls, original comparisons and failures.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'), ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    controls = {
        'map': ['tests/terrain-map-test.cpp', 'reconstruction/rendering/terrain_map.cpp'],
        'preferences': ['tests/palette-lighting-test.cpp', 'assets/palette_lighting.cpp',
                        'assets/config_loader.cpp', 'reconstruction/rendering/palette_shading.cpp'],
        'shading': ['tests/palette-shading-test.cpp', 'reconstruction/rendering/palette_shading.cpp'],
    }
    references = {
        'map': ['tests/terrain-map-reference.cpp', 'reconstruction/rendering/terrain_map.cpp'],
        'preferences': ['tests/terrain-palette-preference-reference.cpp', 'reconstruction/rendering/palette_shading.cpp'],
        'global-options': ['tests/palette-lighting-reference.cpp', 'reconstruction/rendering/palette_shading.cpp'],
        'shading': ['tests/rendering-boundary-reference.cpp', 'reconstruction/rendering/palette_shading.cpp',
                    'reconstruction/rendering/terrain_camera.cpp'],
    }
    flags = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Iassets', '-Ireconstruction/rendering']
    # Dead-section removal confines the direct parser control to decode APIs;
    # real file-loading integration is deliberately outside this batch.
    parser_link = ['-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections']
    paths = set().union(*(required_sources(behaviors[b]) for b in [*SELECTED, POLICY]))
    paths.update(['tools/test-rendering-twentieth-three.py', 'tools/coverage_claims.py',
                  'tools/coverage_evidence.py', 'tools/decode-cfg.py', 'tools/test-terrain-map.py',
                  'tools/terrain_validation_inputs.py', 'tools/terrain_validation_pixels.py',
                  'tools/original-manifest.sh'])
    dep_sources = sorted({p for values in [*controls.values(), *references.values()] for p in values})
    deps = subprocess.check_output(flags+['-MM']+dep_sources, cwd=ROOT, env=env, text=True)
    (out/'dependencies.txt').write_text(deps)
    for token in shlex.split(deps.replace('\\\n', ' ')):
        path = ROOT/token
        if path.is_file():
            paths.add(path.resolve().relative_to(ROOT).as_posix())
    sources = {p: sha(ROOT/p) for p in sorted(paths)}
    claims = [dict(behavior=b, contract_sha256=behavior_contract(behaviors[b], builds)) for b in [*SELECTED, POLICY]]
    report = dict(schema=1, success=False, sources=sources, claims=claims, original_claims=[c for c in claims if c['behavior'] in SELECTED],
                  original_sha256=BUILD, inputs={}, commands=[], controls={}, comparisons={},
                  live_replacement=False,
                  scope='Owned ordinary map projection and surface admission, selected palette option '
                        'clamp/missing semantics and strict native parsing, and mode-zero palette construction '
                        'and checked shade selection. Malformed-string rejection, complete odd-count storage '
                        'and invalid-input refusals are native policies, not original failure behavior. '
                        'No whole-world production, file loader integration, nondefault floating-point '
                        'equivalence, special effects, driver/allocator lifetime, live replacement or FPS credit.')

    def run(label, argv, structured=False):
        command = [str(a) for a in argv]
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, timeout=300)
        log = out/(label+'.log')
        log.write_bytes(result.stdout+b'\n'+result.stderr)
        report['commands'].append(dict(label=label, argv=command, exit_code=result.returncode, log_sha256=sha(log)))
        result.check_returncode()
        return json.loads(result.stdout) if structured else None

    verified_before = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        verified_before = True
        exe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(exe) != BUILD:
            raise ValueError('Unsupported original executable')
        realm = args.asset_root.resolve()/'Realms/Celtic/Forest'
        maps = [realm/name for name in ['CFsec01.map', 'CFsec02.map', 'CFsec38.map']]
        inputs = [exe, realm/'Terrain.ttd', *maps]
        report['inputs'] = {str(p): sha(p) for p in inputs}
        codec = module('bounded_map_codec', ROOT/'tools/decode-cfg.py')
        geometry = module('bounded_map_geometry', ROOT/'tools/test-terrain-map.py').geometry
        map_args = []
        for i, path in enumerate(maps):
            _, raw = codec.decode(path.read_bytes())
            base, final = geometry(raw, (realm/'Terrain.ttd').read_bytes())
            fixtures = [out/f'map-{i}-{label}.bin' for label in ['raw', 'base', 'final']]
            for target, payload in zip(fixtures, [raw, base, final]):
                target.write_bytes(payload)
            report.setdefault('derived_inputs', {}).update({str(p.relative_to(out)): sha(p) for p in fixtures})
            map_args += [realm/'Terrain.ttd', *fixtures]
        for mode, options in [('native', ['-O2']), ('sanitized', ['-O1', '-g',
                            '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'])]:
            for label, src in controls.items():
                binary = out/(label+'-'+mode)
                run(label+'-'+mode+'-build', flags+['-Werror']+options+parser_link+src+['-o', binary])
                run(label+'-'+mode, [binary])
                report['controls'][label+'-'+mode] = dict(success=True, binary_sha256=sha(binary))
            binary = out/('map-installed-'+mode)
            run('map-installed-'+mode+'-build', flags+options+['-DMNM_NATIVE_ONLY']+references['map']+['-o', binary])
            report['comparisons']['map-installed-'+mode] = run('map-installed-'+mode, [binary, exe, *map_args], True)
        for label, src in references.items():
            binary = out/(label+'-original')
            run(label+'-original-build', flags+['-m32', '-O2', '-fno-pie', '-no-pie']+src+['-o', binary])
            value = run(label+'-original', [binary, exe]+(map_args if label=='map' else []), True)
            if not value.get('all_match', value.get('success', False)):
                raise ValueError('Original comparison refused: '+label)
            report['comparisons'][label] = value
        report['native_all_passed'] = len(report['controls'])==6 and all(v['success'] for v in report['controls'].values())
        report['sources_stable'] = all(sha(ROOT/p)==h for p,h in sources.items())
        report['inputs_stable'] = all(sha(Path(p))==h for p,h in report['inputs'].items())
        if not report['sources_stable'] or not report['inputs_stable']:
            raise ValueError('Execution inputs changed')
        report['success'] = True
    finally:
        try:
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = verified_before
        except BaseException:
            report['success'] = False
            raise
        finally:
            write_report(ROOT, report_path, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'], controls=len(report['controls']), comparisons=report['comparisons']), indent=2))


if __name__=='__main__':
    main()
