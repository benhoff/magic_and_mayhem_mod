#!/usr/bin/env python3
"""Bounded Linux/i386 execution of unchanged pinned PE creature reach/stun/AI routines."""
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
RANGES = [(0x50bd60,0x50c0c3),(0x50c930,0x50ca27),(0x50d060,0x50d2dd),(0x50f590,0x50f7bb),(0x50f7c0,0x50f84f),(0x518420,0x5186b3),(0x521160,0x52141f),(0x521420,0x521573),(0x5189e0,0x518c49),(0x518c50,0x518f77),(0x518750,0x5189df),(0x518f80,0x5190fb),(0x519100,0x5192c9),(0x519330,0x51983e)]
ENDPOINTS = [0x4f4ba0,0x4f3260,0x50aa30,0x50ab30,0x50db60,0x5120c0,0x54dcb0,0x54dce0,0x5708e0,0x520c70,0x464ec0,0x464cb0,0x516220,0x51d3a0,0x517820,0x51d660,0x5186c0,0x51c3a0,0x5198b0,0x54e200,0x519330,0x518f80,0x519100,0x518750,0x518c50,0x5189e0,0x51d7b0,0x51db30,0x514360,0x512800,0x524910,0x5075f0,0x51e090]


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
    claims = json.loads(args.claims.read_text()) if args.claims else {'sources': {}, 'claims': []}
    if any(sha(ROOT / path) != digest for path, digest in claims['sources'].items()):
        raise ValueError('Declared source changed before execution')
    args.output.mkdir(parents=True)
    report = dict(success=False, claims=claims['claims'], source_sha256=claims['sources'],
                  source_executable_sha256=HASH, original_target_text_unchanged=False,
                  scope='Bounded footprint reach/direction, ordinary stun entry/action, knockback admission/response, ready callback timing, AI controller precedence, enemy/home/assist/corpse selection and ordinary retaliation branch. Explicit supplied animation, terrain, collision, behavior, target admission, assist and move endpoints. Empty effect ownership; hostile spell scan, all-species AI, special ownership, full fight equivalence and live replacement pending.',
                  endpoint_addresses=[hex(a) for a in ENDPOINTS], wine_required=False,
                  live_replacement=False, commands=[])

    def run(name, command, env=None):
        report['commands'].append([str(x) for x in command])
        result = subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                                capture_output=True, text=True, timeout=120)
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
        report['original_entry_anchors'] = {hex(a): image_bytes(a, 16).hex() for a, _ in RANGES}
        report['original_target_sha256'] = {hex(a): hashlib.sha256(image_bytes(a, b-a)).hexdigest() for a, b in RANGES}
        report['redirect_original_bytes'] = {hex(a): image_bytes(a, 5).hex() for a in ENDPOINTS}
        flags = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-Ireconstruction/combat', '-Iassets']
        sources = ['tests/creature-control-test.cpp', 'reconstruction/combat/control.cpp', 'reconstruction/combat/ai.cpp']
        def build(name, options):
            objects=[]
            for i, source in enumerate(sources):
                obj=args.output/f'{name}-{i}.o'
                run(f'build-{name}-{i}',flags+options+['-MMD','-MF',args.output/f'{name}-{i}.d','-c',source,'-o',obj])
                objects.append(obj)
            run(f'link-{name}',flags+options+objects+['-o',args.output/name])
        build('native',['-O2'])
        build('reference',['-m32','-O1','-g','-no-pie','-DMNM_CONTROL_REFERENCE'])
        build('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])
        dependencies = {'tools/test-creature-control.py'}
        for dependency in args.output.glob('*.d'):
            for token in shlex.split(dependency.read_text().replace('\\\n', ' ')):
                file = Path(token)
                if file.is_absolute():
                    if file.is_relative_to(ROOT):
                        file = file.relative_to(ROOT)
                    else:
                        continue
                if (ROOT / file).is_file() and not str(file).startswith('working/'):
                    dependencies.add(str(file))
        report['compiler_dependencies'] = sorted(dependencies)
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
        (args.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    if not report['sources_unchanged']:
        raise ValueError('Source changed during comparison')
    print(json.dumps({k: report[k] for k in ['success', 'reach_cases', 'stun_cases', 'sanitizer_passed']}))


if __name__ == '__main__':
    main()
