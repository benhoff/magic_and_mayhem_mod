#!/usr/bin/env python3
"""Compare unchanged pinned scheduler bodies with native host reconstruction."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = [(0x571ab0, 0x571bd1), (0x571be0, 0x571c2e),
          (0x571c30, 0x571d16), (0x571d20, 0x571d7d),
          (0x571d80, 0x571eb1), (0x571ec0, 0x571f7a)]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--executable', type=Path, default=ROOT / 'original/Arcane_Nocd/Chaos.exe')
    args = parser.parse_args()
    out = args.output.resolve()
    declaration = json.loads(args.claims.read_text())
    if out.exists():
        parser.error('Refusing existing evidence output')
    assert all(sha(ROOT / p) == h for p, h in declaration['sources'].items())
    out.mkdir(parents=True)
    report = dict(schema=1, success=False, sources=declaration['sources'],
                  claims=declaration['claims'], original_sha256=PIN,
                  scope='Six unchanged original scheduler bodies on authored valid 2..5-record rings; compare selected identity, all record/link/output/object guard bytes and scripted clock/COM order. Driver/tick implementations, invalid pointers/rings/missing lookup faults and live manager/cadence/cache/event replacement excluded.',
                  wine_required=False, live_replacement=False)

    def run(label, command, env=None):
        result = subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                                capture_output=True, text=True, timeout=180)
        (out / (label + '.stdout')).write_text(result.stdout)
        (out / (label + '.stderr')).write_text(result.stderr)
        if result.returncode:
            raise RuntimeError(f'{label} failed ({result.returncode}): {result.stderr[-2500:]}')
        return result.stdout

    verified = False
    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        verified = True
        data = args.executable.read_bytes()
        assert hashlib.sha256(data).hexdigest() == PIN, 'Unsupported original executable'
        pe = struct.unpack_from('<I', data, 60)[0]
        sections = pe + 24 + struct.unpack_from('<H', data, pe + 20)[0]

        def original_bytes(address, count):
            for i in range(struct.unpack_from('<H', data, pe + 6)[0]):
                rva, size, offset = struct.unpack_from('<III', data, sections + 40*i + 12)
                if 0x400000+rva <= address and address+count <= 0x400000+rva+size:
                    start = offset + address-0x400000-rva
                    return data[start:start+count]
            raise ValueError('Original range has no complete raw backing')

        report['original_bodies'] = {hex(a): dict(end=hex(b), bytes=b-a,
            sha256=hashlib.sha256(original_bytes(a, b-a)).hexdigest(),
            entry=original_bytes(a, 16).hex()) for a, b in RANGES}
        sources = ['tests/audio-scheduler-reference.cpp',
                   'reconstruction/audio/voice_scheduler.cpp',
                   'reconstruction/audio/voice_lifetime.cpp',
                   'reconstruction/audio/voice_contract.cpp']
        common = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror',
                  '-Ireconstruction/audio', '-Iaudio']
        builds = dict(native=['-O2'], original=['-O2', '-m32', '-fno-pie', '-no-pie',
                      '-mno-sse', '-mno-mmx', '-DMNM_SCHEDULER_REFERENCE'],
                      sanitized=['-O1', '-g', '-fsanitize=address,undefined',
                                 '-fno-omit-frame-pointer', '-fno-pie', '-no-pie'])
        dependencies = {'tools/test-audio-scheduler-original.py', 'tools/original-manifest.sh'}
        for name, flags in builds.items():
            text = run(name+'-dependencies', common + flags + ['-MM', *sources])
            for token in shlex.split(text.replace('\\\n', ' ')):
                p = Path(token)
                if p.is_absolute():
                    if not p.is_relative_to(ROOT):
                        continue
                    p = p.relative_to(ROOT)
                if (ROOT / p).is_file():
                    dependencies.add(str(p))
        report['compiler_dependencies'] = sorted(dependencies)
        assert dependencies <= declaration['sources'].keys(), 'Undeclared compiler inputs: ' + repr(sorted(dependencies-declaration['sources'].keys()))
        for name, flags in builds.items():
            run(name+'-build', common + flags + sources + ['-o', out/name])
            env = dict(os.environ)
            if name == 'sanitized':
                env.update(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
                           UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
            report[name] = json.loads(run(name, [out/name] + ([args.executable] if name=='original' else []), env))
        for field in ['success', 'operations', 'cases', 'events']:
            assert report['native'][field] == report['original'][field] == report['sanitized'][field], field
        assert report['native']['cases'] > 200000
        assert all(count > 0 for count in report['native']['operations'])
        assert report['original']['guard_bytes'] == report['original']['cases'] * 17 * 72
        assert report['native']['guard_bytes'] == report['sanitized']['guard_bytes'] == 0
        report.update(success=True, original_instructions_unchanged=True,
                      sanitizer_passed=True, input_unchanged=args.executable.read_bytes()==data,
                      binaries={name: sha(out/name) for name in builds},
                      compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0])
    except Exception as error:
        report['error'] = repr(error)
        raise
    finally:
        if verified:
            run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        report['sources_stable'] = all(sha(ROOT/p)==h for p,h in declaration['sources'].items())
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    assert report['sources_stable'] and report['input_unchanged']
    print(json.dumps({name: report['original'][name] for name in ['cases', 'events', 'operations', 'guard_bytes']}))


if __name__ == '__main__':
    main()
