#!/usr/bin/env python3
"""Compare original empty RGB-add caller state and inaccessible pixel backing."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists(): parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_text())
    if any(sha(ROOT / p) != h for p, h in declaration['sources'].items()): parser.error('Prospective sources changed')
    parent = ROOT / 'working/tests/rgb-add-empty'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    report = dict(schema=1, success=False, sources=declaration['sources'], claims=declaration['claims'],
                  original_sha256=BUILD, commands=[], original_game_executed=False,
                  scope='Original RGB-add wrapper versus actual production splices. Positive-height zero/negative horizontal clipping, RGB565/555, DF0/1, inaccessible pixels and eight-byte guarded source descriptors. Complete physical/caller/FX state and no native history request; no live or whole-renderer equivalence.')

    def run(name, command):
        command = [str(x) for x in command]
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=600)
        log = out / (name + '.log'); log.write_text(result.stdout + result.stderr)
        report['commands'].append(dict(command=command, exit_code=result.returncode, log=str(log.relative_to(ROOT)), sha256=sha(log)))
        result.check_returncode(); return result.stdout

    verified = False
    try:
        run('manifest-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        verified = True
        image = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(image) != BUILD: raise ValueError('Unsupported original')
        child = Path(run('pixel-splices', ['python3', '-B', ROOT / 'tools/test-pixel-splice.py', '--claims', args.claims.resolve()]).strip().splitlines()[-1])
        if not child.resolve().is_relative_to(ROOT / 'working/tests/pixel-splice'): raise ValueError('Unexpected pixel result')
        pixels = json.loads(child.read_text())
        if not (pixels['success'] and pixels['sources_stable'] and pixels['inputs_stable']): raise ValueError('Positive pixel comparison failed')
        report['pixel_comparison'] = dict(path=str(child.relative_to(ROOT)), sha256=sha(child), comparison=pixels['comparison'])
        objects = [child.parent / (name + '.o') for name in ('pixel_kernel.c', 'pixel_route.c', 'pixel_entry.S', 'pixel_bridge.c', 'fade_route.c', 'fade_entry.S', 'fade_kernel.c', 'pixel-splice-state-probe.S')]
        run('empty-build', ['clang++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-fdeclspec',
                           '-Wno-ignored-attributes', '-Wall', '-Wextra', '-Werror', '-MMD', '-MF', out / 'reference.d',
                           ROOT / 'tests/rgb-add-empty-reference.cpp', *objects, '-o', out / 'reference'])
        closure = set()
        for name in shlex.split((out / 'reference.d').read_text().replace('\\\n', ' ').split(':', 1)[1]):
            p = Path(name).resolve()
            if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT / 'working'): closure.add(str(p.relative_to(ROOT)))
        if closure - declaration['sources'].keys(): raise ValueError('Undeclared empty fixture sources: ' + repr(sorted(closure - declaration['sources'].keys())))
        report['compiled_sources'] = sorted(closure)
        run('original-empty', [out / 'reference', image, out / 'comparison.json'])
        report['comparison'] = json.loads((out / 'comparison.json').read_text())
        report['success'] = (report['comparison']['success'] and report['comparison']['empty_native_cases'] == 512 and
                             report['comparison']['default_native_cases'] == 256 and
                             report['comparison']['explicit_native_cases'] == 256 and report['comparison']['disabled_cases'] == 8)
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('manifest-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_verified_before_after'] = verified
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT / p) == h for p, h in declaration['sources'].items())
        report['success'] &= report['sources_stable']
        (out / '.retain').write_text('Empty RGB-add original comparison: ' + str(args.report) + '\n')
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open('x') as stream: json.dump(report, stream, indent=2); stream.write('\n')
        print(args.report, flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
