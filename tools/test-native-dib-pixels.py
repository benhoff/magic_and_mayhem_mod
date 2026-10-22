#!/usr/bin/env python3
"""Check native owned HDC painting against GDI and exercise lease refusals."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--wine-prefix', type=Path, help='Reuse a stopped owned test prefix; no cold-start claim')
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p, h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/native-dib-pixels'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, sources=claims['sources'], claims=claims['claims'],
        scope='256 canonical1/4/8/24 BI_RGB source cases in RGB565/RGB555, signed clipped destinations and current native owned HDC leases: GDI painting versus native painting and ReleaseDC/Lock pixels, invalid handles/ROP/source offset and retired lease refusals, complete teardown. No original EXE wrapper execution, physical DirectDraw driver equivalence or live game replacement.',
        experiment=str(out.relative_to(ROOT)), live_replacement=False)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEDEBUG='-all', LIBGL_ALWAYS_SOFTWARE='1', WINEPREFIX=str(out/'wineprefix'), WINEDLLOVERRIDES='ddraw=')
    if args.wine_prefix:
        prefix = args.wine_prefix.resolve()
        if not prefix.is_relative_to(ROOT/'working/tests') or not prefix.is_dir() or prefix.is_symlink():
            raise ValueError('Reused prefix must be an owned test directory')
        env['WINEPREFIX'] = str(prefix)
        report['reused_wine_prefix'] = str(prefix.relative_to(ROOT))
    commands = []

    def run(label, argv, cwd=None):
        with (out/(label+'.log')).open('x') as log:
            r = subprocess.run([str(a) for a in argv], cwd=cwd or ROOT, env=env, stdout=log, stderr=log, timeout=180)
        commands.append(dict(label=label, returncode=r.returncode, log=str((out/(label+'.log')).relative_to(ROOT))))
        r.check_returncode()
    try:
        if not args.wine_prefix:
            run('prefix', ['cp', '-a', '--reflink=auto', (ROOT/'working/wineprefix-x86_64').resolve(), out/'wineprefix'])
        spec = importlib.util.spec_from_file_location('dib_build', ROOT/'tools/build-native-draw.py')
        builder = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(builder)
        dll = builder.build(out/'native-build', claims['sources'])
        shutil.copyfile(dll, out/'MnmDraw.dll')
        imports = {'kernel32': {'LoadLibraryA': 4, 'GetProcAddress': 8, 'CreateFileA': 28, 'WriteFile': 20, 'CloseHandle': 4, 'ExitProcess': 4},
                   'user32': {'GetDesktopWindow': 0}, 'gdi32': {'StretchDIBits': 52, 'CreateCompatibleBitmap': 12, 'SelectObject': 8, 'DeleteObject': 4}}
        libraries = []
        for name, symbols in imports.items():
            definition = out/(name+'.def')
            definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n, s in symbols.items()))
            run(name+'-imports', ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', out/(name+'.lib')])
            libraries.append(out/(name+'.lib'))
        run('probe-build', ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin', '-fno-stack-protector',
            '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-c', ROOT/'tests/native-dib-pixels-probe.c', '-o', out/'probe.obj'])
        run('probe-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib', '/safeseh:no',
            '/timestamp:0', '/out:'+str(out/'probe.exe'), out/'probe.obj', *libraries])
        run('gdi-native-lease', ['xvfb-run', '-a', 'wine', out/'probe.exe'], cwd=out)
        raw = (out/'dib-pixels-results.bin').read_bytes()
        if len(raw)!=256*12*4:
            raise ValueError('Incomplete owned lease cases')
        mismatch = []
        for i in range(256):
            row = struct.unpack_from('<12I', raw, i*48)
            if row[1]!=5 or row[2] or any(v!=0x80000000 for v in row[3:7]) or any(row[7:9]):
                mismatch.append(dict(case=i, words=list(row)))
        report.update(cases=256, physical_visible_words=256*117, lease_refusals=256*5, mismatches=mismatch,
                      results_sha256=sha(out/'dib-pixels-results.bin'), native_ddraw_disabled=True,
                      remaining_dcs=0 if not mismatch else None, remaining_surfaces=0 if not mismatch else None)
        report['success'] = not mismatch
    except Exception as error:
        report['error'] = repr(error)
    finally:
        if Path(env['WINEPREFIX']).exists():
            subprocess.run(['wineserver', '-k'], env=env, timeout=10, check=False)
            subprocess.run(['wineserver', '-w'], env=env, timeout=10, check=False)
        report['sources_stable'] = all(sha(ROOT/p)==h for p, h in claims['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        report['commands'] = commands
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
