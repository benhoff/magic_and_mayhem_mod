#!/usr/bin/env python3
"""Compare whole maintenance/resource/account owned call graph with an owned native model."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = [(0x509e50,0x50a8d0),(0x466870,0x4668b0),(0x507f00,0x507f9c),(0x416d80,0x416ea0),(0x505bf0,0x505c49),(0x505c50,0x505c95),(0x59bee0,0x59bf07)]
ENDPOINTS = [4461920, 4606768, 4606992, 4607104, 4607344, 4607680, 4741904, 4801520, 4837488, 4838400, 5061136, 5074736, 5103760, 5196032, 5198384, 5200448, 5270832, 5271776, 5273328, 5274832, 5275712, 5276912, 5278464, 5280304, 5280736, 5286704, 5287088, 5287840, 5291360, 5295312, 5297536, 5298240, 5298256, 5298272, 5298288, 5298416, 5299040, 5303136, 5303680, 5306448, 5307104, 5308432, 5312128, 5316800, 5318816, 5323808, 5324864, 5325280, 5325488, 5326944, 5327968, 5337040, 5341504, 5342240, 5348064, 5350912, 5351664, 5351984, 5358128, 5358864, 5369696, 5370480, 5371360, 5374208, 5377136, 5377648, 5379920, 5379952, 5379968, 5384912, 5387376, 5391600, 5418848, 5426368, 5494736, 5561040, 5561248, 5561568, 5561696, 5562880, 5706512, 5706848]



def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--claims', type=Path)
    args = p.parse_args()
    args.output = args.output.resolve()
    if args.output.exists():
        p.error('Refusing existing evidence output')
    ranges, endpoints = RANGES, ENDPOINTS
    claims = json.loads(args.claims.read_text()) if args.claims else {'sources': {}, 'claims': []}
    if any(sha(ROOT / path) != digest for path, digest in claims['sources'].items()):
        raise ValueError('Declared source changed before execution')
    args.output.mkdir(parents=True)
    report = dict(success=False, claims=claims['claims'], source_sha256=claims['sources'],
                  source_executable_sha256=HASH, original_target_text_unchanged=False,
                  scope=f'Whole owned contribution: unchanged pinned original bodies listed in original_target_sha256, with declared helper services listed in endpoint_addresses. Full actor bytes and ordered consequences compared within the fixture domain; complete lower world services, native application integration and live replacement remain separate pending boundaries.',
                  suite='contribution-owned',
                  endpoint_addresses=[hex(a) for a in endpoints], wine_required=False,
                  live_replacement=False, commands=[])

    def run(name, command, env=None):
        report['commands'].append([str(x) for x in command])
        try:
            result = subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                                    capture_output=True, text=True, timeout=120)
        except subprocess.TimeoutExpired as exc:
            for stream in ['stdout', 'stderr']:
                value = getattr(exc, stream) or b''
                (args.output / (name + '.' + stream)).write_text(value.decode(errors='replace') if isinstance(value, bytes) else value)
            raise
        (args.output / (name + '.stdout')).write_text(result.stdout)
        (args.output / (name + '.stderr')).write_text(result.stderr)
        if result.returncode or 'ERROR: AddressSanitizer' in result.stderr or 'runtime error:' in result.stderr:
            raise RuntimeError(f'{name} failed ({result.returncode}): {result.stdout[-3000:]}{result.stderr[-3000:]}')
        return result.stdout

    run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
    try:
        executable = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        raw = executable.read_bytes()
        if sha(executable) != HASH:
            raise ValueError('Unsupported immutable PE; no execution/redirects permitted')
        pe = struct.unpack_from('<I', raw, 60)[0]
        sections = pe + 24 + struct.unpack_from('<H', raw, pe + 20)[0]

        def image_bytes(va, size):
            for i in range(struct.unpack_from('<H', raw, pe + 6)[0]):
                rva, n, offset = struct.unpack_from('<3I', raw, sections + 40 * i + 12)
                if 0x400000 + rva <= va and va + size <= 0x400000 + rva + n:
                    return raw[offset + va - 0x400000 - rva:offset + va - 0x400000 - rva + size]
            raise ValueError('Unbacked original address')
        report['original_entry_anchors'] = {hex(a): image_bytes(a, 16).hex() for a, _ in ranges}
        report['original_target_sha256'] = {hex(a): hashlib.sha256(image_bytes(a, b-a)).hexdigest() for a, b in ranges}
        report['redirect_original_bytes'] = {hex(a): image_bytes(a, 5).hex() for a in endpoints}
        flags = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-Ireconstruction/combat', '-Iassets']
        sources = ['tests/creature-contribution-owned-test.cpp', *['reconstruction/creature-ai/'+n+'.cpp' for n in ['contribution_owned','resource','contribution_account','resource_contribution','execution','route_execution','maintenance','movement_behavior','death_behavior']], 'reconstruction/combat/control.cpp']
        # Development runs also need a stable transitive source closure. A
        # declaration remains mandatory for scope-bound evidence, but omitted
        # claims must never permit a mixed-version build to look successful.
        dependency_text = run('pre-build-dependencies', flags + ['-m32', '-DMNM_EXECUTION_REFERENCE', '-MM'] + sources)
        initial_dependencies = {'tools/test-creature-contribution-owned.py', *sources}
        for token in shlex.split(dependency_text.replace('\\\n', ' ')):
            path = Path(token)
            if not path.is_absolute() and (ROOT / path).is_file():
                initial_dependencies.add((ROOT / path).resolve().relative_to(ROOT).as_posix())
        report['build_inputs_sha256'] = {path: sha(ROOT / path) for path in sorted(initial_dependencies)}
        def build(name, options):
            objects=[]
            for i, source in enumerate(sources):
                obj=args.output/f'{name}-{i}.o'
                run(f'build-{name}-{i}',flags+options+['-MMD','-MF',args.output/f'{name}-{i}.d','-c',source,'-o',obj])
                objects.append(obj)
            run(f'link-{name}',flags+options+objects+['-o',args.output/name])
        build('native',['-O2'])
        build('reference',['-m32','-O1','-g','-no-pie','-DMNM_EXECUTION_REFERENCE'])
        build('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])
        dependencies = {'tools/test-creature-contribution-owned.py'}
        for dependency in args.output.glob('*.d'):
            for token in shlex.split(dependency.read_text().replace('\\\n', ' ')):
                file = Path(token)
                if file.is_absolute():
                    if file.is_relative_to(ROOT):
                        file = file.relative_to(ROOT)
                    else:
                        continue
                if (ROOT / file).is_file() and not str(file).startswith('working/'):
                    dependencies.add((ROOT / file).resolve().relative_to(ROOT).as_posix())
        report['compiler_dependencies'] = sorted(dependencies)
        if dependencies - report['build_inputs_sha256'].keys():
            raise ValueError('Source closure changed during compilation: ' + str(sorted(dependencies - report['build_inputs_sha256'].keys())))
        if any(sha(ROOT / path) != digest for path, digest in report['build_inputs_sha256'].items()):
            raise ValueError('Source edited during compilation; refusing mixed-version execution')
        if args.claims and dependencies - claims['sources'].keys():
            raise ValueError('Unbound compiler inputs: ' + str(sorted(dependencies - claims['sources'].keys())))
        native = json.loads(run('native', [args.output / 'native']))
        original = json.loads(run('original', [args.output / 'reference', executable]))
        sanitized = json.loads(run('sanitized', [args.output / 'sanitized'], env={**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1', 'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1'}))
        assert native == original == sanitized
        assert executable.read_bytes() == raw
        report.update(**original,
                      original_target_text_unchanged=True, object_write_guards=True,
                      native_passed=True, sanitizer_passed=True, input_unchanged=True,
                      binaries={name: sha(args.output / name) for name in ['native', 'reference', 'sanitized']},
                      compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0])
    finally:
        run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['sources_unchanged'] = all(sha(ROOT / path) == digest for path, digest in claims['sources'].items())
        report['build_inputs_unchanged'] = all(sha(ROOT / path) == digest for path, digest in report.get('build_inputs_sha256', {}).items())
        (args.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    if not report['sources_unchanged'] or not report['build_inputs_unchanged']:
        raise ValueError('Source changed during comparison')
    print(json.dumps({k: report[k] for k in ['success', 'cases', 'functions', 'sanitizer_passed']}))


if __name__ == '__main__':
    main()
