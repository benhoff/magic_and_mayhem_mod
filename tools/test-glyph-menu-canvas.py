#!/usr/bin/env python3
"""Exercise real PE32 glyph admission with menu clips and guarded short bitmaps."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working') or (ROOT/args.report).exists():
        parser.error('Require fresh task-owned working output and report paths')
    out.mkdir(parents=True)
    report = dict(schema=1, success=False, commands=[], original_game_executed=False,
                  live_replacement=False, scope=__doc__)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(WINEPREFIX=str(out/'prefix'), WINEDEBUG='-all',
               WINEDLLOVERRIDES='winemenubuilder.exe=d;mscoree,mshtml=')
    sources = {str(p.relative_to(ROOT)): sha(p) for p in
               [ROOT/'tools/test-glyph-menu-canvas.py', ROOT/'tests/glyph-production-probe.c',
                ROOT/'tests/glyph-live-admission.c']}

    def run(label, command, extra=None):
        with (out/(label+'.log')).open('xb') as log:
            result = subprocess.run(list(map(str, command)), cwd=out,
                                    env={**env, **(extra or {})}, stdout=log,
                                    stderr=subprocess.STDOUT, timeout=180)
        report['commands'].append(dict(label=label, command=list(map(str, command)),
                                       returncode=result.returncode))
        result.check_returncode()

    try:
        run('compiler-version', ['clang', '--version'])
        run('wine-version', ['wine', '--version'])
        run('scene-build', ['python3', ROOT/'tools/build-scene-observer.py', '--selftest'])
        build = ROOT/'working/build/scene-observer-selftest'
        manifest = json.loads((build/'manifest.json').read_bytes())
        sources.update(manifest['sources'])
        if sha(build/'MnmScene.dll') != manifest['sha256']:
            raise ValueError('Compiled scene identity mismatch')
        shutil.copyfile(build/'MnmScene.dll', out/'MnmScene.dll')
        shutil.copyfile(build/'manifest.json', out/'scene-build.json')
        flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding',
                 '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx',
                 '-Wall', '-Wextra', '-Werror']
        imports = {'LoadLibraryA': 4, 'GetProcAddress': 8, 'GetEnvironmentVariableA': 12,
                   'SetEnvironmentVariableA': 8, 'ExitProcess': 4}
        (out/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+
                                      ''.join(f'{n}@{s}\n' for n, s in imports.items()))
        run('kernel-imports', ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d',
                               out/'kernel32.def', '-l', out/'kernel32.lib'])
        (out/'scene.def').write_text('LIBRARY MnmScene.dll\nEXPORTS\nGlyphAdmissionForTest@0\n')
        run('scene-imports', ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d',
                              out/'scene.def', '-l', out/'scene.lib'])
        for label, source, libraries in [
                ('admission', 'tests/glyph-live-admission.c', [out/'scene.lib', build/'kernel32.lib']),
                ('production', 'tests/glyph-production-probe.c', [out/'kernel32.lib'])]:
            run(label+'-build', [*flags, '-c', ROOT/source, '-o', out/(label+'.obj')])
            run(label+'-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console',
                               '/nodefaultlib', '/safeseh:no', '/timestamp:0',
                               '/out:'+str(out/(label+'.exe')), out/(label+'.obj'), *libraries])
        run('admission', ['xvfb-run', '-a', 'wine', out/'admission.exe'])
        (out/'glyph-backend').mkdir()
        run('production', ['xvfb-run', '-a', 'wine', out/'production.exe'],
            dict(MNM_TEST_GLYPH_DIR='Z:'+str(out/'glyph-backend').replace('/', '\\'),
                 MNM_TEST_GLYPH_STRICT='1'))
        from glyph_live_capture import inspect
        stats = inspect(out/'glyph-backend', 'takeover')['stats']
        if stats['handled'] != 65545 or stats['forwarded'] or stats['errors']:
            raise ValueError('Production completion counters failed: '+repr(stats))
        report.update(success=True, production_stats=stats,
                      old_full_prefix_refusal_reproduced=True,
                      geometry=dict(physical_strides=[8, 520], physical_height=8,
                                    clip_bounds=[[8, 8], [800, 600]], final_rows=[2, 7]),
                      controls=['last physical row admitted before no-access page',
                                'first row beyond allocation refused', 'nonzero Y origin',
                                'multirow overflow refused', 'read-only output refused',
                                'full-prefix capture still refused',
                                'existing read-only/guard/no-access/wrap/alias controls'],
                      formats=['RGB565', 'RGB555'])
    except Exception as error:
        report['error'] = repr(error)
    finally:
        for label, command in [('stop', ['wineserver', '-k']), ('wait', ['wineserver', '-w'])]:
            try:
                run(label, command)
            except Exception as error:
                report.update(success=False, cleanup_error=repr(error))
        report['sources'] = sources
        report['sources_stable'] = all(sha(ROOT/p) == value for p, value in sources.items())
        report['success'] &= report['sources_stable']
        report['artifacts'] = {}
        for path in [*sorted(out.glob('*.log')), out/'MnmScene.dll', out/'scene-build.json',
                     out/'admission.exe', out/'production.exe', out/'glyph-backend/glyph-stats.bin']:
            if path.is_file():
                name = str(args.report.parent/(args.report.stem+'-'+path.name+'.bin'))
                report['artifacts'][name] = write_report(ROOT, name, path.read_bytes())
        report['source_base'] = subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip()
        for source in ['runtime/scene/glyph_live.c', 'tests/glyph-production-probe.c',
                       'tools/test-glyph-menu-canvas.py']:
            name = str(args.report.parent/(args.report.stem+'-source-'+Path(source).name+'.bin'))
            report['artifacts'][name] = write_report(ROOT, name, (ROOT/source).read_bytes())
        write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps({k: report.get(k) for k in ('success', 'sources_stable', 'error')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
