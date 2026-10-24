#!/usr/bin/env python3
"""Renew the camera, ordinary section-copy and tile-submission native contracts."""
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
SELECTED = ['RE.terrain_camera', 'RE.terrain_sections', 'RE.terrain_submission']
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--asset-root', type=Path, required=True,
                        help='Existing read-only installed game; no prefix or installation is created')
    args = parser.parse_args()
    out = args.output.resolve()
    report_path = args.report.absolute()
    if (out.exists() or not out.is_relative_to(ROOT/'working/tests') or
            report_path.exists() or not report_path.is_relative_to(ROOT/'research/runtime')):
        parser.error('Require new execution and research report paths within this checkout')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain three terrain rendering controls, original comparisons and failures.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'), ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    controls = {
        'camera': ['tests/terrain-camera-test.cpp', 'reconstruction/rendering/terrain_camera.cpp'],
        'sections': ['tests/terrain-sections-test.cpp', 'reconstruction/rendering/terrain_sections.cpp'],
        'submission': ['tests/terrain-submission-test.cpp', 'reconstruction/rendering/terrain_submission.cpp',
                       'reconstruction/rendering/sprite_queue.cpp'],
    }
    references = {k: [v[0].replace('-test.cpp', '-reference.cpp'), *v[1:]]
                  for k, v in controls.items()}
    flags = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Ireconstruction/rendering']
    paths = set().union(*(required_sources(behaviors[b]) for b in SELECTED))
    paths.update(['tools/test-rendering-nineteenth-three.py', 'tools/coverage_claims.py',
                  'tools/coverage_evidence.py', 'tools/decode-cfg.py', 'tools/original-manifest.sh'])
    dep_sources = sorted({p for values in [*controls.values(), *references.values()] for p in values})
    deps = subprocess.check_output(flags+['-MM']+dep_sources, cwd=ROOT, env=env, text=True)
    (out/'dependencies.txt').write_text(deps)
    for token in shlex.split(deps.replace('\\\n', ' ')):
        path = ROOT/token
        if path.is_file():
            paths.add(path.resolve().relative_to(ROOT).as_posix())
    sources = {p: sha(ROOT/p) for p in sorted(paths)}
    claims = [dict(behavior=b, contract_sha256=behavior_contract(behaviors[b], builds)) for b in SELECTED]
    report = dict(schema=1, success=False, sources=sources, claims=claims,
                  original_claims=claims, original_sha256=BUILD, inputs={}, commands=[],
                  controls={}, comparisons={}, live_replacement=False,
                  scope='Selected owned camera setters/map-axis and screen scrolling, ordinary section '
                        'copy destination/source effects, and ordinary tile draw fields only. '
                        'Checked native arithmetic/refusals and immutable aliased section sources are '
                        'native policies. No object rotation, allocator/driver, original whole-world '
                        'production, full scene pixels, installed camera timing, live, FPS or replacement credit.')

    def run(label, argv, timeout=300, structured=False):
        command = [str(a) for a in argv]
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, timeout=timeout)
        log = out/(label+'.log')
        log.write_bytes(result.stdout+b'\n'+result.stderr)
        report['commands'].append(dict(label=label, argv=command, exit_code=result.returncode,
                                       log_sha256=sha(log)))
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
        inputs = [exe, realm/'Terrain.ttd', realm/'Terrain.spr']
        maps = [realm/name for name in ['CFsec01.map', 'CFsec02.map', 'CFsec38.map']]
        inputs += maps
        report['inputs'] = {str(p): sha(p) for p in inputs}
        spec = importlib.util.spec_from_file_location('terrain_three_codec', ROOT/'tools/decode-cfg.py')
        codec = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(codec)
        section_args = []
        for i, path in enumerate(maps):
            _, raw = codec.decode(path.read_bytes())
            decoded = out/f'section-{i}.bin'
            decoded.write_bytes(raw)
            section_args += [realm/'Terrain.ttd', decoded]
        for mode, options in [('native', ['-O2']), ('sanitized', ['-O1', '-g',
                            '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'])]:
            for label, src in controls.items():
                binary = out/(label+'-'+mode)
                run(label+'-'+mode+'-build', flags+['-Werror']+options+src+['-o', binary])
                run(label+'-'+mode, [binary])
                report['controls'][label+'-'+mode] = dict(success=True, binary_sha256=sha(binary))
        for label, src in references.items():
            binary = out/(label+'-original')
            run(label+'-original-build', flags+['-m32', '-O2', '-fno-pie', '-no-pie']+src+['-o', binary])
            argv = [binary, exe]
            if label == 'sections':
                argv += section_args
            if label == 'submission':
                argv += [realm/'Terrain.ttd', realm/'Terrain.spr']
            value = run(label+'-original', argv, structured=True)
            if not value.get('all_match', label == 'submission'):
                raise ValueError('Original comparison refused: '+label)
            # Preview journals are raw original output, not a new scene acceptance result.
            if label == 'submission':
                report['original_preview_journals'] = len(value.pop('previews'))
            report['comparisons'][label] = value
        report['native_all_passed'] = all(v['success'] for v in report['controls'].values())
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['inputs_stable'] = all(sha(Path(p)) == h for p, h in report['inputs'].items())
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
    print(json.dumps(dict(success=report['success'], controls=len(report['controls']),
                          comparisons=report['comparisons']), indent=2))


if __name__ == '__main__':
    main()
