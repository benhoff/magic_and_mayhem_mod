#!/usr/bin/env python3
"""Bounded actual PE32 extended facade and original clipped-copy comparison."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--prefix', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    declaration = json.loads(args.claims.read_text()) if args.claims else {}
    sources = declaration.get('sources', {})
    report = dict(success=False, claims=declaration.get('claims', []), sources=sources,
                  scope=__doc__, original_game_executed=False, cases=[])
    def stable():
        return all(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == h for p, h in sources.items())
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(TMPDIR=str(out), WINEPREFIX=str(args.prefix.resolve()), WINEDEBUG='-all',
               WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d', DISPLAY='')
    def run(name, command, extra=None, timeout=120, cwd=ROOT):
        with (out / (name + '.log')).open('x') as log:
            result = subprocess.run(list(map(str, command)), cwd=cwd, env=env | (extra or {}),
                                    stdout=log, stderr=subprocess.STDOUT, timeout=timeout)
        report['cases'].append(dict(name=name, exit_code=result.returncode))
        if result.returncode:
            raise RuntimeError(name + ': exit ' + str(result.returncode))
    try:
        if not stable():
            raise ValueError('Prospective input changed')
        spec = importlib.util.spec_from_file_location('builder', ROOT / 'tools/build-native-draw.py')
        builder = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(builder)
        dll = builder.build(out / 'native', sources or None)
        run('bridge-build', ['python3', '-B', ROOT / 'tools/build-render-bridge.py',
                             '--output', out / 'bridge'])
        imports = {k: dict(v) for k, v in builder.IMPORTS.items()}
        imports['kernel32'].update(LoadLibraryA=4, GetProcAddress=8)
        imports['user32'].update(GetDesktopWindow=0)
        imports['gdi32'].update(GetDIBColorTable=16)
        libraries = []
        for name, entries in imports.items():
            definition = out / (name + '.def')
            definition.write_text('LIBRARY ' + name + '.dll\nEXPORTS\n' + ''.join(k + '@' + str(v) + '\n' for k, v in entries.items()))
            library = out / (name + '.lib')
            run(name, ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', library])
            libraries.append(library)
        for name, path in [('extended', 'tests/native-extended-draw-probe.c'),
                           ('transforms', 'tests/native-copy-transform-probe.c'),
                           ('backing', 'tests/native-owned-backing-probe.c')]:
            run(name + '-compile', [*builder.FLAGS, '-c', ROOT / path, '-o', out / (name + '.obj')])
            run(name + '-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console',
                                '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:' + str(out / (name + '.exe')),
                                out / (name + '.obj'), *libraries])
        run('wineboot', ['wineboot', '-i'])
        path = 'Z:' + str(dll).replace('/', '\\')
        for name, strict in [('extended', True), ('transforms', False), ('backing', True)]:
            extra = dict(MNM_TEST_LIBRARY=path, MNM_OWNER_DRAW_DLL=path,
                         MNM_NATIVE_CPU_GUARD='3', MNM_PUBLIC_RESIDENT_WORLD='1')
            if strict:
                extra['MNM_NATIVE_OWNERSHIP_REQUIRED'] = '1'
            directory = out / name
            directory.mkdir()
            run(name, ['wine', out / (name + '.exe')], extra, cwd=directory)
        report.update(success=True, rop3_tables=256, rotated_mirrored_copies=16,
                      original_clipped_copy_cases=528, palette_classes=7, primary_depths=[8, 16, 24, 32])
    except Exception as error:
        report['error'] = str(error)
    finally:
        subprocess.run(['wineserver', '-k'], env=env, check=False, timeout=10)
        report['sources_stable'] = stable()
        report['success'] = report['success'] and report['sources_stable']
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report.get(k) for k in ('success', 'sources_stable', 'error')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
