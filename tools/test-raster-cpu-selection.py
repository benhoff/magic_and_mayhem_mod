#!/usr/bin/env python3
"""Execute original CPU-family raster selection without patching its code."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists(): p.error('Refusing evidence overwrite')
    declaration = json.loads(a.claims.read_text())
    if any(sha(ROOT / n) != h for n, h in declaration['sources'].items()):
        p.error('Prospective sources changed')
    parent = ROOT / 'working/tests/raster-cpu-selection'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    report = dict(schema=1, success=False, sources=declaration['sources'], claims=declaration['claims'],
                  original_sha256=BUILD, original_game_executed=False, commands=[],
                  scope='Original initializer484470 and helper4845f0 under Linux per-thread CPUID trapping at exactly two verified original instructions. All16 base families, MMX0/1 and override0/1/2/UINT_MAX. Original computes four dispatch pointers; unchanged original code and object guards checked. Authored platform contexts, no live game or native pixel comparison.')

    def execute(name, command):
        r = subprocess.run([str(x) for x in command], cwd=ROOT, capture_output=True, text=True, timeout=120)
        log = out / (name + '.log'); log.write_text(r.stdout + r.stderr)
        report['commands'].append(dict(command=[str(x) for x in command], exit_code=r.returncode,
                                       log=str(log.relative_to(ROOT)), sha256=sha(log)))
        r.check_returncode()

    verified = False
    try:
        execute('original-before', [ROOT / 'tools/original-manifest.sh', 'verify']); verified = True
        exe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(exe) != BUILD: raise ValueError('Unsupported executable')
        execute('compile', ['clang++', '-m32', '-std=c++17', '-O2', '-fno-pie', '-no-pie',
                            '-Wall', '-Wextra', '-Werror', ROOT / 'tests/raster-cpu-selection-reference.cpp', '-o', out / 'reference'])
        execute('original-selection', [out / 'reference', exe, out / 'selection.json'])
        report['selection'] = json.loads((out / 'selection.json').read_text())
        report['success'] = report['selection']['success'] and report['selection']['case_count'] == 128
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            execute('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['original_verified_before_after'] = verified and not report.get('manifest_error')
        report['sources_stable'] = all(sha(ROOT / n) == h for n, h in declaration['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        (out / '.retain').write_text('Original CPU-family selector acquisition; ' + str(a.report) + '\n')
        a.report.parent.mkdir(parents=True, exist_ok=True)
        with a.report.open('x') as stream:
            json.dump(report, stream, indent=2); stream.write('\n')
        print(a.report)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
