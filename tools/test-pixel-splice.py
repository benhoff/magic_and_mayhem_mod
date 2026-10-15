#!/usr/bin/env python3
"""Bounded production image/effect splice comparison with pinned original loops."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    sources = declaration['sources']
    if any(sha(ROOT/p) != h for p, h in sources.items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/pixel-splice'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    exe = ROOT/'original/Arcane_Nocd/Chaos.exe'
    report = dict(schema=1, success=False, sources=sources, claims=declaration['claims'], original_sha256=HASH,
        scope='Pinned original PCX surface/index/raw WORD, RGB-add, brighten, row-copy and fade wrappers versus production installer/resolver/entry/kernel, with supplied Lock/Unlock and Wine-independent Win32 memory/API adapters. Physical pixels, caller state and thread refusal. DIB kernel synthetic controls only; owned DC integration and native JPEG/minimap/glyph/raster closure remain separate.',
        live_replacement=False, original_work_bypassed=False, experiment=str(out.relative_to(ROOT)))
    commands = []

    def run(label, argv):
        r = subprocess.run([str(a) for a in argv], cwd=ROOT, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'),
                           capture_output=True, text=True, timeout=300)
        log = out/(label+'.log')
        log.write_text(r.stdout+r.stderr)
        commands.append(dict(label=label, argv=[str(a) for a in argv], returncode=r.returncode,
                             log=str(log.relative_to(ROOT)), sha256=sha(log)))
        r.check_returncode()
        return r.stdout
    try:
        run('manifest-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        if sha(exe) != HASH:
            raise ValueError('Unsupported original executable')
        from pixel_backend_launch import load
        original = exe.read_bytes()
        exporter = load('pixel_range_offsets', 'tools/export-menu-support.py')
        report['original_entry_anchors'] = {}
        report['recovered_bodies'] = {}
        for start, end in ((0x54b5f0,0x54b7fb),(0x58d770,0x58d8b6),(0x58d3b0,0x58d4a7)):
            at = exporter.image_offset(original,start,end-start)
            report['original_entry_anchors'][hex(start)] = original[at:at+16].hex()
            report['recovered_bodies'][hex(start)] = dict(end=end,bytes=end-start,
                sha256=hashlib.sha256(original[at:at+end-start]).hexdigest())
        run('kernel-build', ['gcc', '-O1', '-g', '-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
            '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-MMD', '-MF', out/'unit.d',
            ROOT/'tests/pixel-kernel-test.c', ROOT/'reconstruction/rendering/pixel_kernel.c', '-o', out/'unit'])
        report['kernel'] = json.loads(run('kernel-sanitized', [out/'unit']))
        names = ['reconstruction/rendering/pixel_kernel.c', 'runtime/canvas-writers/pixel_route.c',
            'runtime/canvas-writers/pixel_entry.S', 'runtime/canvas-writers/pixel_bridge.c',
            'runtime/canvas-writers/fade_route.c', 'runtime/canvas-writers/fade_entry.S',
            'reconstruction/rendering/fade_kernel.c', 'tests/pixel-splice-state-probe.S']
        objects = []
        for name in names:
            obj = out/(Path(name).name+'.o')
            run(Path(name).name, ['clang', '-m32', '-O2', '-fno-pie', '-fdeclspec', '-Wno-ignored-attributes',
                '-DMNM_PIXEL_WRITER_SELFTEST', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
                '-MMD', '-MF', obj.with_suffix('.d'), '-c', ROOT/name, '-o', obj])
            objects.append(obj)
        run('reference-build', ['clang++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-fdeclspec',
            '-Wno-ignored-attributes', '-Wall', '-Wextra', '-Werror', '-MMD', '-MF', out/'reference.d',
            ROOT/'tests/pixel-splice-reference.cpp', *objects, '-o', out/'reference'])
        closure = {'tools/test-pixel-splice.py', 'tools/original-manifest.sh'}
        for path in out.glob('*.d'):
            for name in shlex.split(path.read_text().replace('\\\n', ' ').split(':', 1)[1]):
                p = Path(name).resolve()
                if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    closure.add(str(p.relative_to(ROOT)))
        if closure-sources.keys():
            raise ValueError('Undeclared compiler dependencies: '+repr(sorted(closure-sources.keys())))
        report['compiled_project_sources'] = sorted(closure)
        run('production-pe32-build', ['python3', ROOT/'tools/build-pixel-writers.py'])
        run('native-draw-build', ['python3', ROOT/'tools/build-native-draw.py'])
        run('original-native-splices', [out/'reference', exe, out/'comparison.json'])
        report['comparison'] = json.loads((out/'comparison.json').read_text())
        report['success'] = report['comparison']['success'] and report['kernel']['success']
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('manifest-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['inputs_stable'] = sha(exe) == HASH
        report['success'] = report['success'] and report['sources_stable'] and report['inputs_stable']
        report['commands'] = commands
        report['artifacts'] = {str(p.relative_to(ROOT)): dict(sha256=sha(p), bytes=p.stat().st_size)
                               for p in sorted(out.iterdir()) if p.is_file()}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
