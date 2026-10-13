#!/usr/bin/env python3
"""Compare whole original51ec80 and eleven actual ownership/reset/transition/idle helpers with native composition."""
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
RANGES = [(0x51ec80,0x51eded),(0x51a880,0x51a8ec),(0x512340,0x512383),(0x411e50,0x411e6e),(0x407b90,0x407bb4),(0x522340,0x52243e),(0x466870,0x4668b0),(0x51edf0,0x51ef5a),(0x50ab30,0x50ac91),(0x50aa30,0x50ab05),(0x50a930,0x50a9e2),(0x50acb0,0x50ad03)]
ENDPOINTS = [0x4dea00,0x4d7a00,0x4d78b0,0x516220,0x516800,0x513850,0x524130,0x50db60,0x54dce0,0x4943f0,0x4d3ea0,0x54dad0,0x5178c0,0x50eb60,0x416590]



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
                  scope=f'Whole ownership transfer with owned recount and associated-owner lookup, visibility, wizard classification, marker clearing root activity, owner-target repair reset/behavior transition and idle behavior: unchanged pinned original bodies listed in original_target_sha256, with declared helper services listed in endpoint_addresses. Full actor bytes and ordered consequences compared within the fixture domain; complete lower world services, native application integration and live replacement remain separate pending boundaries.',
                  suite='transfer-idle-owned',
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
        report['original_dispatch_sha256'] = {'0x50a9e4': hashlib.sha256(image_bytes(0x50a9e4, 64)).hexdigest()}
        report['original_entry_anchors'] = {hex(a): image_bytes(a, 16).hex() for a, _ in ranges}
        report['original_target_sha256'] = {hex(a): hashlib.sha256(image_bytes(a, b-a)).hexdigest() for a, b in ranges}
        report['redirect_original_bytes'] = {hex(a): image_bytes(a, 5).hex() for a in endpoints}
        flags = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-Ireconstruction/combat', '-Iassets']
        sources = ['tests/creature-transfer-idle-owned-test.cpp', 'reconstruction/creature-ai/owner_transfer.cpp', 'reconstruction/creature-ai/transfer_owned.cpp', 'reconstruction/creature-ai/ownership_helpers.cpp', 'reconstruction/creature-ai/owner_counts.cpp', 'reconstruction/creature-ai/resource.cpp', 'reconstruction/creature-ai/transfer_targets_owned.cpp', 'reconstruction/creature-ai/owner_target_repair.cpp', 'reconstruction/creature-ai/transfer_reset_owned.cpp', 'reconstruction/creature-ai/execution.cpp', 'reconstruction/creature-ai/transfer_idle_owned.cpp', 'reconstruction/creature-ai/behavior_execution.cpp']
        # Development runs also need a stable transitive source closure. A
        # declaration remains mandatory for scope-bound evidence, but omitted
        # claims must never permit a mixed-version build to look successful.
        dependency_text = run('pre-build-dependencies', flags + ['-m32', '-DMNM_TRANSFER_IDLE_REFERENCE', '-MM'] + sources)
        initial_dependencies = {'tools/test-creature-transfer-idle-owned.py', *sources}
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
        build('reference',['-m32','-O1','-g','-no-pie','-DMNM_TRANSFER_IDLE_REFERENCE'])
        build('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])
        dependencies = {'tools/test-creature-transfer-idle-owned.py'}
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
