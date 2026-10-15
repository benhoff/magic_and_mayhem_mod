#!/usr/bin/env python3
"""Compare native RGB565/RGB555 glyph RET16 entries with the pinned original."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', required=True, type=Path)
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p, h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/glyph-formats'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, sources=claims['sources'], claims=claims['claims'], original_sha256=HASH,
        scope='Synthetic bounded RGB565/RGB555 glyph RLE, tint, clipping and padding; actual native route/RET16 entry versus unchanged original, complete physical pixels/table/caller GPR/defined flags/DF/selected FX. Twelve precision/rounding controls, cold/warm tables, zero/one occupied finite stack value, original-once fallback for two occupied values and unsupported controls/table seeds. No installed live hook, process-memory/glyph lifetime, text layout or resident presentation claim.',
        experiment=str(out.relative_to(ROOT)), live_replacement=False, original_work_bypassed=False)
    commands = []

    def run(label, argv):
        r = subprocess.run([str(a) for a in argv], cwd=ROOT, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'),
                           capture_output=True, text=True, timeout=240)
        log = out/(label+'.log')
        log.write_text(r.stdout+r.stderr)
        commands.append(dict(label=label, returncode=r.returncode, log=str(log.relative_to(ROOT)), sha256=sha(log)))
        r.check_returncode()
        return r.stdout
    try:
        run('manifest-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        exe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(exe)!=HASH:
            raise ValueError('Unsupported original build')
        spec = importlib.util.spec_from_file_location('format_glyphs', ROOT/'tools/test-font-glyph-raster.py')
        glyphs = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(glyphs)
        records = []
        for pattern in range(4):
            frame = glyphs.synthetic(pattern)
            fw, fh, ox, oy = struct.unpack_from('<IIii', frame, 4)
            w, h = max(32, fw+8), max(24, fh+8)
            positions = [(4,4), (-1,4), (w-fw+1,4), (4,-3), (4,h-3), (-fw,4), (4,h), (0,0)]
            for x, y in positions:
                for tint in [(0,0,0), (255,255,255), (173,91,237), (7,3,7)]:
                    for cold in (0,1):
                        for variant in range(12):
                            for active in (0,1,2):
                                fields = [328+len(frame), len(frame), w, h, w+variant%3, x+ox, y+oy,
                                          0, 0, w, h, *tint, 113, cold, variant, active]
                                table = [i/64 for i in range(64)]
                                records.append(struct.pack('<18I', *(v&0xffffffff for v in fields))+struct.pack('<64f', *table)+frame)
        corpus = out/'cases.bin'
        corpus.write_bytes(b'MNMGLS01'+struct.pack('<I', len(records))+b''.join(records))
        report['corpus_sha256'] = sha(corpus)
        run('atomic-build', ['gcc', '-O1', '-g', '-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
            '-fsanitize=address,undefined', '-fno-omit-frame-pointer', ROOT/'reconstruction/rendering/glyph_backend.c',
            ROOT/'reconstruction/rendering/glyph_backend_state.c', ROOT/'tests/glyph-backend-test.c', '-o', out/'admission'])
        report['atomic_result'] = run('atomic-sanitized', [out/'admission']).strip()
        names = ['reconstruction/rendering/glyph_backend.c', 'reconstruction/rendering/glyph_backend_state.c',
                 'reconstruction/rendering/glyph_backend_x87.S', 'runtime/scene/glyph_route.c', 'runtime/scene/glyph_entry.S',
                 'tests/glyph-backend-state-probe.S', 'tests/glyph-backend-entry-probe.S']
        objects = []
        for name in names:
            obj = out/(Path(name).name+'.o')
            run(Path(name).name, ['gcc', '-m32', '-O2', '-fno-pie', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
                '-MMD', '-MF', obj.with_suffix('.d'), '-c', ROOT/name, '-o', obj])
            objects.append(obj)
        run('reference-build', ['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-Wall', '-Wextra', '-Werror',
            '-DMNM_GLYPH_FORMATS_TEST', '-MMD', '-MF', out/'reference.d', ROOT/'tests/glyph-backend-reference.cpp', *objects, '-o', out/'reference'])
        closure = {'tools/test-glyph-formats.py', 'tools/test-font-glyph-raster.py', 'tests/glyph-backend-test.c', 'tools/original-manifest.sh'}
        for dep in out.glob('*.d'):
            for token in shlex.split(dep.read_text().replace('\\\n', ' ').split(':', 1)[1]):
                p = Path(token).resolve()
                if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    closure.add(p.relative_to(ROOT).as_posix())
        if closure-claims['sources'].keys():
            raise ValueError('Undeclared compiler dependencies: '+repr(sorted(closure-claims['sources'].keys())))
        report['compiled_project_sources'] = sorted(closure)
        run('original-native-formats', [out/'reference', exe, corpus, out/'results.bin'])
        raw = (out/'results.bin').read_bytes()
        if raw[:8]!=b'MNMGLBK1' or (len(raw)-12)%64:
            raise ValueError('Incomplete format comparison records')
        failures, handled, forwarded, words = [], 0, 0, 0
        for i in range((len(raw)-12)//64):
            row = struct.unpack_from('<16I', raw, 12+i*64)
            words += row[6]
            handled += row[1]
            forwarded += row[3]
            if row[0]!=255 or row[3]>1 or (row[1] and row[3]):
                failures.append(dict(record=i, words=list(row)))
        report.update(cases=(len(raw)-12)//64, native_handled=handled, original_once_forwards=forwarded,
                      physical_words=words, failures=failures, results_sha256=sha(out/'results.bin'))
        report['success'] = not failures and handled>0 and forwarded>0
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('manifest-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT/p)==h for p, h in claims['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        report['commands'] = commands
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
