#!/usr/bin/env python3
"""Current production splice/ABI comparisons with bounded durable SQLite output."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', '--git-common-dir'], text=True).strip()).resolve().parent
ORIGINAL = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PREFIX = 'research/runtime/caller-state-results-20261108/'
RECIPES = {
    'panel': ('MNM_PANEL_WRITER_SELFTEST', [
        'reconstruction/rendering/panel_kernel.c', 'runtime/canvas-writers/panel_route.c',
        'runtime/canvas-writers/panel_entry.S', 'runtime/canvas-writers/panel_bridge.c',
        'tests/panel-splice-state-probe.S']),
    'pixel': ('MNM_PIXEL_WRITER_SELFTEST', [
        'reconstruction/rendering/pixel_kernel.c', 'runtime/canvas-writers/pixel_route.c',
        'runtime/canvas-writers/pixel_entry.S', 'runtime/canvas-writers/pixel_bridge.c',
        'runtime/canvas-writers/fade_route.c', 'runtime/canvas-writers/fade_entry.S',
        'reconstruction/rendering/fade_kernel.c', 'tests/pixel-splice-state-probe.S']),
    'minimap': ('MNM_MINIMAP_WRITER_SELFTEST', [
        'reconstruction/rendering/minimap_pixels.c', 'reconstruction/rendering/pixel_kernel.c',
        'runtime/canvas-writers/minimap_route.c', 'runtime/canvas-writers/minimap_entry.S',
        'runtime/canvas-writers/minimap_bridge.c', 'tests/pixel-splice-state-probe.S']),
    'colour-rect': ('MNM_COLOUR_RECT_SELFTEST', [
        'reconstruction/rendering/colour_rect.c', 'reconstruction/rendering/pixel_kernel.c',
        'runtime/canvas-writers/colour_rect_entry.S', 'runtime/canvas-writers/colour_rect_bridge.c',
        'tests/pixel-splice-state-probe.S']),
    'raster': ('MNM_RASTER_SELFTEST', [
        'reconstruction/rendering/raster_pixels.c', 'reconstruction/rendering/pixel_kernel.c',
        'runtime/canvas-writers/raster_entry.S', 'runtime/canvas-writers/raster_bridge.c',
        'tests/raster-splice-state-probe.S']),
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--name', required=True)
    parser.add_argument('--family', choices=RECIPES, action='append', required=True)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    if not re.fullmatch('[a-z0-9-]+', args.name) or len(set(args.family)) != len(args.family):
        parser.error('Require a unique lowercase name and distinct families')
    destination = ROOT / (PREFIX + args.name + '.json')
    if destination.exists():
        raise ValueError('Preserve previous immutable results')
    claims = json.loads(args.claims.read_text())
    prospective = {}
    for folder in ('runtime', 'reconstruction', 'renderer', 'protocols', 'tests'):
        for path in (ROOT / folder).rglob('*'):
            if path.is_file() and path.suffix in ('.c', '.h', '.hpp', '.cpp', '.S'):
                prospective[path.relative_to(ROOT).as_posix()] = sha(path.read_bytes())
    for path in ('tools/test-rendering-kernel-caller-state.py', 'tools/original-manifest.sh',
                 *claims['sources']):
        prospective[path] = sha((ROOT / path).read_bytes())
    if any(prospective[p] != h for p, h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    report = dict(schema=1, kind='rendering_kernel_caller_state_matrix', success=False,
        claims=claims['claims'], families=args.family, results={}, commands=[],
        original_sha256=ORIGINAL, original_game_runs=0, newly_passed_requirements=0,
        scope='Actual current installer/resolver/production splice assembly and pixel kernel against unchanged original PE32 code mapped into a private Linux i386 process, with explicitly supplied provider adapters. Complete guarded destination and reference-defined machine-state comparisons. Raster fast-body volatile ECX/EDX/upper EAX and scratch arguments retain the existing exclusions. No World whole-entry bypass, real surface driver, installed game/profile or whole acceptance claim.',
        temporary_classification='Objects/depfiles/build trees are reproducible temporary output; tested reference executables, dependency manifests, logs, result bytes and changed source preimages are durable SQLite evidence.')
    sources = set(claims['sources']) | {'tools/test-rendering-kernel-caller-state.py', 'tools/original-manifest.sh'}
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', TMPDIR='/tmp')
    with tempfile.TemporaryDirectory(prefix='mnm-kernel-caller-state-', dir='/tmp') as temporary:
        out = Path(temporary)
        def retain(label, raw):
            path = PREFIX + args.name + '-' + label
            write_report(ROOT, path, raw)
            return dict(path=path, sha256=sha(raw), bytes=len(raw))
        def run(label, argv, timeout=300):
            result = subprocess.run(list(map(str, argv)), cwd=ROOT, env=env, capture_output=True, timeout=timeout)
            log = retain(label + '.log.bin', result.stdout + result.stderr)
            report['commands'].append(dict(label=label, argv=list(map(str, argv)), returncode=result.returncode, log=log))
            if result.returncode:
                raise RuntimeError(label + ': exit ' + str(result.returncode))
            return result.stdout
        try:
            run('manifest-before', [MAIN / 'tools/original-manifest.sh', 'verify'])
            original = MAIN / 'original/Arcane_Nocd/Chaos.exe'
            if sha(original.read_bytes()) != ORIGINAL:
                raise ValueError('Unsupported original image')
            run('clang-version', ['clang', '--version'])
            run('clang-cxx-version', ['clang++', '--version'])
            for family in args.family:
                directory = out / family
                directory.mkdir()
                define, names = RECIPES[family]
                objects = []
                for index, name in enumerate(names):
                    obj = directory / (str(index) + '.o')
                    run(family + '-compile-' + str(index), ['clang', '-m32', '-O2', '-fno-pie',
                        '-fdeclspec', '-Wno-ignored-attributes', '-D' + define, '-mno-sse', '-mno-mmx',
                        '-Wall', '-Wextra', '-Werror', '-MMD', '-MF', obj.with_suffix('.d'),
                        '-c', ROOT / name, '-o', obj])
                    objects.append(obj)
                reference = directory / 'reference'
                source = 'tests/' + family + '-splice-reference.cpp'
                run(family + '-link', ['clang++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie',
                    '-fdeclspec', '-Wno-ignored-attributes', '-Wall', '-Wextra', '-Werror',
                    '-MMD', '-MF', directory / 'reference.d', ROOT / source, *objects, '-o', reference])
                compiled = set(names) | {source}
                for depfile in directory.glob('*.d'):
                    for token in shlex.split(depfile.read_text().replace('\\\n', ' ').split(':', 1)[1]):
                        path = Path(token).resolve()
                        if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT / 'working'):
                            compiled.add(path.relative_to(ROOT).as_posix())
                if compiled - prospective.keys():
                    raise ValueError('Compiler dependencies not frozen before execution: ' + repr(sorted(compiled - prospective.keys())))
                if any(sha((ROOT / p).read_bytes()) != prospective[p] for p in compiled):
                    raise ValueError('Compiler sources changed before execution')
                sources.update(compiled)
                binary = retain(family + '-reference.bin', reference.read_bytes())
                dependencies = retain(family + '-dependencies.bin',
                    b'\n'.join(p.name.encode() + b'\n' + p.read_bytes() for p in sorted(directory.glob('*.d'))))
                # Persist the tested executable and exact inputs before running it.
                result_path = directory / 'comparison.json'
                run(family + '-original-native', [reference, original, result_path])
                raw = result_path.read_bytes()
                result = json.loads(raw)
                report['results'][family] = dict(comparison=result,
                    result=retain(family + '-comparison.bin', raw), tested_executable=binary,
                    dependencies=dependencies, compiled_sources=sorted(compiled))
                if result.get('success') is not True or result.get('formats', 2) != 2:
                    raise ValueError('Incomplete two-format comparison: ' + family)
            report['success'] = set(report['results']) == set(args.family)
        except (ValueError, OSError, RuntimeError, subprocess.SubprocessError) as error:
            report['error'] = str(error)
        finally:
            try:
                run('manifest-after', [MAIN / 'tools/original-manifest.sh', 'verify'])
            except (RuntimeError, OSError, subprocess.SubprocessError) as error:
                report.update(success=False, manifest_error=str(error))
            report['sources'] = {p: prospective[p] for p in sorted(sources)}
            report['changed_sources'] = [p for p in sorted(sources) if sha((ROOT / p).read_bytes()) != prospective[p]]
            report['source_preimages'] = {}
            # Owned source edits must stay reproducible relative to HEAD.
            for p in sorted(sources):
                raw = (ROOT / p).read_bytes()
                previous = subprocess.run(['git', 'show', 'HEAD:' + p], cwd=ROOT, capture_output=True)
                if previous.returncode or previous.stdout != raw:
                    report['source_preimages'][p] = retain('source-' + sha(p.encode())[:16] + '.bin', raw)
            report['sources_stable'] = not report['changed_sources']
            report['inputs_stable'] = sha((MAIN / 'original/Arcane_Nocd/Chaos.exe').read_bytes()) == ORIGINAL
            report['success'] = report['success'] and report['sources_stable'] and report['inputs_stable']
            write_report(ROOT, destination, (json.dumps(report, indent=2) + '\n').encode())
    print(json.dumps(dict(success=report['success'], results=report['results'], error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
