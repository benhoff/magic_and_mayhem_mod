#!/usr/bin/env python3
"""Compare selected original rectangle-wrapper forwarding in a private PE mapping.

COM endpoints capture successful-call arguments only. No driver pixel, clipping,
error, retry, Restore or gameplay equivalence is inferred from those endpoints.
"""
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BUILD_HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ENTRIES = (0x58c360, 0x58c4a0, 0x58c8a0, 0x58ca90, 0x58cbc0)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def wrap(value):
    return (value + 2**31) % 2**32 - 2**31


def cases():
    # No endpoint crops, repairs or rejects an input: this tests original forwarding.
    rectangles = [
        ('inside', (1, 1, 4, 3), (2, 2)),
        ('left', (0, 0, 4, 3), (-2, 1)),
        ('top', (0, 0, 4, 3), (1, -2)),
        ('right', (0, 0, 4, 3), (7, 1)),
        ('bottom', (0, 0, 4, 3), (1, 5)),
        ('corner', (0, 0, 4, 3), (-2, -2)),
        ('outside-left', (0, 0, 4, 3), (-20, 1)),
        ('outside-right', (0, 0, 4, 3), (20, 1)),
        ('outside-top', (0, 0, 4, 3), (1, -20)),
        ('outside-bottom', (0, 0, 4, 3), (1, 20)),
        ('negative-source', (-2, -3, 2, 1), (1, 1)),
        ('excess-source', (7, 5, 12, 9), (1, 1)),
        ('empty-width', (1, 1, 1, 3), (1, 1)),
        ('empty-height', (1, 1, 3, 1), (1, 1)),
        ('reversed', (4, 3, 1, 1), (1, 1)),
        ('overflow', (-2**31, -2**31, 2**31-1, 2**31-1), (2**31-1, -2**31)),
    ]
    result = []
    for entry in ENTRIES:
        for label, source, (x, y) in rectangles:
            for fullscreen in (0, 1):
                for no_wait in (0, 1):
                    for window in (0, 1, 2):
                        # Explicit Blt also exercises reversed/scaled destination dimensions.
                        dest = (x, y, wrap(x+5), wrap(y+2))
                        result.append(dict(id=len(result), entry=entry, label=label,
                            fullscreen=fullscreen, no_wait=no_wait, window=window, missing=0,
                            source=list(source), x=x, y=y, destination=list(dest)))
        if entry in (0x58c360, 0x58c4a0):
            for missing in (1, 2, 3):
                result.append(dict(id=len(result), entry=entry, label='null-interface',
                    fullscreen=0, no_wait=0, window=0, missing=missing,
                    source=[0, 0, 4, 3], x=1, y=2, destination=[1, 2, 5, 5]))
    return result


def expected(case):
    source = case['source']
    dest = case['destination'].copy()
    result = dict(id=case['id'], calls=0, operation=0, flags=0,
                  source=[0]*4, destination=[0]*4, source_after=source,
                  destination_after=dest, window_calls=0, source_identity=False,
                  destination_identity=False)
    if case['missing']:
        return result
    explicit = case['entry'] == 0x58cbc0
    fast = bool(case['fullscreen']) and not explicit
    translated = not case['fullscreen'] and bool(case['window'])
    dx, dy = (-19, 33) if translated and case['window'] == 1 else (0, 0)
    if explicit:
        dest = [wrap(v+(dx if i % 2 == 0 else dy)) for i, v in enumerate(dest)]
        result['destination_after'] = dest
    elif fast:
        dest = [case['x'], case['y'], 0, 0]
    else:
        dest = [wrap(case['x']+dx), wrap(case['y']+dy),
                wrap(case['x']+source[2]-source[0]+dx),
                wrap(case['y']+source[3]-source[1]+dy)]
    keyed = case['entry'] in (0x58c8a0, 0x58ca90, 0x58cbc0)
    flags = (1 if keyed else 0) if fast else (0x8000 if keyed else 0)
    if not case['no_wait'] or explicit:
        flags |= 0x10 if fast else 0x01000000
    return dict(result, calls=1, operation=2 if fast else 1, flags=flags,
                source=source, destination=dest, window_calls=int(translated),
                source_identity=True, destination_identity=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, default=ROOT/'working/game-nocd/Chaos.exe')
    parser.add_argument('--report', type=Path, required=True, help='New report path')
    parser.add_argument('--fixture-dir', type=Path, help='New portable trace corpus in this directory')
    args = parser.parse_args()
    if args.report.exists():
        parser.exit(2, 'Refusing to overwrite report\n')
    parent = ROOT/'working/tests/surface-rectangles'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))

    def verify(phase):
        run = subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'],
                             capture_output=True, text=True, timeout=120)
        (output/f'original-{phase}.log').write_text(run.stdout+run.stderr)
        run.check_returncode()

    verify('before')
    try:
        image = args.executable.read_bytes()
        if sha(image) != BUILD_HASH:
            raise ValueError('Unsupported executable; original wrappers are build-specific')
        binary = output/'reference'
        subprocess.run(['g++', '-m32', '-std=c++17', '-O2', '-fno-pie', '-no-pie',
            '-Wall', '-Wextra', '-Werror', str(ROOT/'tests/surface-rectangle-reference.cpp'),
            '-o', str(binary)], check=True)
        inputs = cases()
        path = output/'inputs.txt'
        path.write_text(''.join(' '.join(str(x) for x in [c['id'], c['entry'], c['fullscreen'],
            c['no_wait'], c['window'], c['missing'], *c['source'], c['x'], c['y'],
            *c['destination']])+'\n' for c in inputs))
        run = subprocess.run([str(binary), str(args.executable.resolve()), str(path)],
                             capture_output=True, text=True, timeout=30)
        (output/'original-traces.json').write_text(run.stdout)
        (output/'reference.stderr').write_text(run.stderr)
        run.check_returncode()
        actual = json.loads(run.stdout)
        if len(actual) != len(inputs):
            raise ValueError('Original fixture omitted cases')
        comparisons = [dict(input=c, original=a, matching=a == expected(c))
                       for c, a in zip(inputs, actual)]
        if sha(args.executable.read_bytes()) != BUILD_HASH:
            raise ValueError('Input executable changed during comparison')
        # Pin original entry bytes without altering or redistributing the executable.
        pe = struct.unpack_from('<I', image, 60)[0]
        count, opt_size = struct.unpack_from('<H', image, pe+6)[0], struct.unpack_from('<H', image, pe+20)[0]
        anchors = {}
        for entry in ENTRIES:
            for n in range(count):
                at = pe+24+opt_size+40*n
                rva, size, raw = struct.unpack_from('<3I', image, at+12)
                if rva <= entry-0x400000 < rva+size:
                    offset = raw+entry-0x400000-rva
                    anchors[f'0x{entry:08x}'] = image[offset:offset+16].hex()
                    break
        sources = ['tests/surface-rectangle-reference.cpp', 'tools/compare-surface-rectangles.py',
                   'tests/test-surface-rectangles.py']
        fixture = None
        if args.fixture_dir and all(c['matching'] for c in comparisons):
            args.fixture_dir.mkdir(parents=True, exist_ok=True)
            dataset = json.dumps(comparisons, separators=(',', ':')).encode()
            packed = gzip.compress(dataset, mtime=0)
            capture_path = args.fixture_dir/'rectangle-forwarding-original.json.gz'
            catalog_path = args.fixture_dir/'rectangle-forwarding-corpus.json'
            if capture_path.exists() or catalog_path.exists():
                raise ValueError('Refusing to overwrite portable fixture')
            capture_path.write_bytes(packed)
            fixture = dict(schema=1, original_sha256=BUILD_HASH, cases=len(comparisons),
                           path=capture_path.name, sha256=sha(packed), raw_sha256=sha(dataset),
                           scope='Original wrapper argument traces from fake successful COM endpoints; no driver output.')
            catalog_path.write_text(json.dumps(fixture, indent=2)+'\n')
            sources.extend(str(p.resolve().relative_to(ROOT)) for p in (capture_path, catalog_path))
        report = dict(schema=1, success=all(c['matching'] for c in comparisons),
            original_sha256=BUILD_HASH, original_entry_bytes=anchors,
            scope='Five unchanged original rectangle wrappers, successful fake COM calls and mocked ClientToScreen. Argument forwarding/translation only; driver pixels/clipping, error/retry/Restore and live execution excluded.',
            summary=dict(cases=len(inputs), matched=sum(c['matching'] for c in comparisons),
                         driver_executed=False, original_text_patched=False),
            artifacts=str(output.relative_to(ROOT)), cases=comparisons, fixture=fixture,
            sources={p: sha((ROOT/p).read_bytes()) for p in sources})
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(report['summary']))
        return 0 if report['success'] else 1
    finally:
        verify('after')


if __name__ == '__main__':
    raise SystemExit(main())
