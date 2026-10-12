#!/usr/bin/env python3
"""Bounded Linux/i386 execution of unchanged pinned PE creature scheduling and transition bodies."""
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
RANGES = [(0x52a780,0x52a9ee),(0x50a930,0x50a9e2),(0x50aa30,0x50ab05),(0x50ab30,0x50ac91),(0x524460,0x5244ea),(0x50fae0,0x50fd4c),(0x50da50,0x50daf8),(0x519ae0,0x51a5b6)]
ENDPOINTS = [0x5128a0,0x51a600,0x50ed80,0x50bd60,0x4f5230,0x4f4900,0x4f5a40,0x510e80,0x519ae0,0x522ad0,0x518420,0x5244f0,0x523470,0x53d7d0,0x441560,0x50acb0,0x510010,0x50afa0,0x50ccd0,0x50d580,0x50d840,0x50d860,0x50d850,0x50d870,0x50d8f0,0x51c510,0x51c230,0x521750,0x521770,0x521780,0x50db60,0x54dce0,0x50eb60,0x4943f0,0x516fd0,0x5084f0,0x5120c0]



def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--claims', type=Path)
    p.add_argument('--suite', choices=['control', 'actions', 'orders', 'strategy', 'behaviors', 'lifecycle', 'callbacks'], default='control')
    args = p.parse_args()
    args.output = args.output.resolve()
    if args.output.exists():
        p.error('Refusing existing evidence output')
    if args.suite == 'callbacks':
        ranges = [(0x50f090,0x50f329),(0x50f360,0x50f3ae),(0x50e7a0,0x50eb24),(0x5170e0,0x517427),(0x50eba0,0x50ec71),(0x50f590,0x50f7bb)]
        endpoints = [0x464ec0,0x516220,0x5164f0,0x50aa30,0x464d70,0x50c0d0,0x50ec80,0x49d310,0x571310,0x5705e0,0x570a90,0x570480,0x5708e0,0x570bd0,0x514860,0x50eb60,0x54e200,0x485b10,0x50d060,0x520c70,0x54dce0,0x54dcb0,0x4943f0,0x5120c0,0x50ab30,0x50db60]
    elif args.suite == 'lifecycle':
        ranges = [(0x509e50,0x50a8d0),(0x50a8d0,0x50a92c),(0x510010,0x510480),(0x50d580,0x50d817)]
        endpoints = [0x464b30,0x464c10,0x464d70,0x54e200,0x485b10,0x54dba0,0x54dad0,0x54dce0,0x54dd60,0x508040,0x507cd0,0x571460,0x571310,0x464ec0,0x514c60,0x49d070,0x51f5e0,0x506d30,0x5076f0,0x508b00,0x514040,0x520e70,0x5142b0,0x520c70,0x51ef60,0x49d400,0x518140,0x50fae0,0x51f270,0x510e80,0x50ab30,0x50ed80,0x464c80,0x4de090,0x513c20,0x514860,0x4f5230,0x5093e0,0x5070e0,0x509230,0x51a8f0,0x50f850,0x51aa30,0x520100,0x5141e0,0x416d80,0x52ccc0,0x52af60,0x50db60]
    elif args.suite == 'behaviors':
        ranges = [(0x50acb0,0x50ad03),(0x50afa0,0x50b0b1),(0x50b0c0,0x50b1e3),(0x50ccd0,0x50cdb4),(0x50d870,0x50d8f0),(0x50d8f0,0x50d9e3),(0x51c230,0x51c398),(0x50d840,0x50d84e),(0x50d850,0x50d85e),(0x50d860,0x50d86e),(0x521750,0x521766),(0x521770,0x521778),(0x521780,0x521788)]
        endpoints = [0x516220,0x516800,0x513850,0x524130,0x517750,0x5164f0,0x5142b0,0x50aa30,0x50ab30,0x510e80,0x50b2c0,0x50b0c0,0x508810,0x5084f0,0x508250,0x515350,0x50bd60,0x50db60]
    elif args.suite == 'strategy':
        ranges = [(0x523470,0x523858),(0x523860,0x523bc0)]
        endpoints = [0x54e200,0x524910,0x50ad10,0x51c3a0,0x4e1380,0x4f4900,0x414890,0x4f3840,0x56cfe0,0x53e5e0,0x50fae0,0x441560,0x50db60,0x50eb60]
    elif args.suite == 'orders':
        ranges = [(0x50ad10,0x50af02),(0x50ca30,0x50cc0e),(0x513850,0x51397f),(0x508b00,0x508c95),(0x523be0,0x5240ae),(0x524130,0x524382),(0x5243b0,0x524460),(0x520ad0,0x520b24)]
        endpoints = [0x597850,0x597870,0x512220,0x514630,0x517e80,0x520ad0,0x4f5230,0x50fae0,0x50ad10,0x50ca30,0x50db60,0x5084f0,0x516fd0,0x50eb60,0x4943f0]
    elif args.suite == 'actions':
        ranges = [(0x50db60,0x50e36b),(0x5244f0,0x5248e2),(0x522ad0,0x522dfe),(0x505cf0,0x505d18),(0x50b2c0,0x50b643),(0x50e670,0x50e6ea),(0x50e6f0,0x50e791),(0x50f7c0,0x50f84f)]
        endpoints = [0x54e200,0x464cb0,0x464ca0,0x50e400,0x510480,0x513f50,0x5142b0,0x464ec0,0x57b710,0x534490,0x517750,0x515620,0x50efc0,0x50b650,0x50b0c0,0x50fae0,0x50ab30,0x5084f0,0x50db60]
    else:
        ranges, endpoints = RANGES, ENDPOINTS
    claims = json.loads(args.claims.read_text()) if args.claims else {'sources': {}, 'claims': []}
    if any(sha(ROOT / path) != digest for path, digest in claims['sources'].items()):
        raise ValueError('Declared source changed before execution')
    args.output.mkdir(parents=True)
    report = dict(success=False, claims=claims['claims'], source_sha256=claims['sources'],
                  source_executable_sha256=HASH, original_target_text_unchanged=False,
                  scope=f'{args.suite} suite: unchanged pinned original bodies listed in original_target_sha256, with declared helper services listed in endpoint_addresses. Full actor bytes and ordered consequences compared within the fixture domain; complete lower world services, native application integration and live replacement remain separate pending boundaries.',
                  suite=args.suite,
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
        suite_sources = {
            'control': [], 'actions': ['action_execution', 'periodic', 'ranged_execution'],
            'orders': ['orders'], 'strategy': ['strategy'], 'behaviors': ['behavior_execution'],
            'lifecycle': ['maintenance', 'movement_behavior', 'death_behavior'],
            'callbacks': ['action_callbacks'],
        }
        sources = [f'tests/creature-execution{ "-"+args.suite if args.suite != "control" else ""}-test.cpp',
                   'reconstruction/creature-ai/execution.cpp', 'reconstruction/creature-ai/route_execution.cpp',
                   'reconstruction/combat/control.cpp']
        sources += [f'reconstruction/creature-ai/{name}.cpp' for name in suite_sources[args.suite]]
        # Development runs also need a stable transitive source closure. A
        # declaration remains mandatory for scope-bound evidence, but omitted
        # claims must never permit a mixed-version build to look successful.
        dependency_text = run('pre-build-dependencies', flags + ['-m32', '-DMNM_EXECUTION_REFERENCE', '-MM'] + sources)
        initial_dependencies = {'tools/test-creature-execution.py', *sources}
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
        dependencies = {'tools/test-creature-execution.py'}
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
