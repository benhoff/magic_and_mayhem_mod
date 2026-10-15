#!/usr/bin/env python3
"""Execute native facade clipping and resource lifetimes in an isolated prefix."""
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


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--wine-prefix', type=Path)
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    if any(sha(ROOT/p) != h for p, h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/native-resource-state'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, sources=claims['sources'], claims=claims['claims'],
                  scope='Native PE32 CPU facade versus retained independent 408-case Wine Surface2 clipping corpus; supplied five-format/alias/palette/DC/healthy Restore policies. No game, live GPU history, stretched drawing, physical loss or original Windows driver equivalence.',
                  experiment=str(out.relative_to(ROOT)), live_replacement=False)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINEPREFIX', 'WINEDLLOVERRIDES'))}
    prefix = out/'wineprefix'
    if args.wine_prefix:
        prefix = args.wine_prefix.resolve()
        if not prefix.is_relative_to(ROOT/'working/tests') or not prefix.is_dir() or prefix.is_symlink():
            raise ValueError('Reuse only an owned stopped test prefix')
    env.update(WINEDEBUG='-all', WINEPREFIX=str(prefix), WINEDLLOVERRIDES='ddraw=', LIBGL_ALWAYS_SOFTWARE='1')
    reservation = None
    commands = []

    def run(label, argv, cwd=None, extra=None):
        with (out/(label+'.log')).open('x') as log:
            proc = subprocess.run([str(a) for a in argv], cwd=cwd or ROOT, env={**env, **(extra or {})},
                                  stdout=log, stderr=log, timeout=180)
        commands.append(dict(label=label, returncode=proc.returncode))
        proc.check_returncode()

    try:
        reservation = module('resource_reservation', 'tools/capture-original-surface-keys.py').reserve_wine()
        if not args.wine_prefix:
            run('prefix', ['cp', '-a', '--reflink=auto', (ROOT/'working/wineprefix-x86_64').resolve(), prefix])
        builder = module('resource_builder', 'tools/build-native-draw.py')
        dll = builder.build(out/'build', claims['sources'])
        shutil.copyfile(dll, out/'MnmDraw.dll')
        report['native_build'] = json.loads((out/'build/manifest.json').read_text())
        imports = {
            'kernel32': {'LoadLibraryA': 4, 'GetProcAddress': 8, 'CreateFileA': 28, 'ReadFile': 20,
                         'WriteFile': 20, 'CloseHandle': 4, 'ExitProcess': 4, 'GetEnvironmentVariableA': 12, 'Sleep': 4},
            'user32': {'GetDesktopWindow': 0, 'CreateWindowExA': 48, 'SetWindowPos': 28, 'ShowWindow': 8, 'DestroyWindow': 4, 'PeekMessageA': 20, 'TranslateMessage': 4, 'DispatchMessageA': 4},
            'gdi32': {'CreateDIBSection': 24, 'SelectObject': 8, 'DeleteObject': 4, 'GetDIBColorTable': 16},
        }
        libraries = []
        for name, symbols in imports.items():
            definition = out/(name+'.def')
            definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n, s in symbols.items()))
            library = out/(name+'.lib')
            run(name, ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', library])
            libraries.append(library)
        for label, source in [('clip', 'tests/native-draw-clip-reference.c'), ('resource', 'tests/native-draw-resource-probe.c'), ('window', 'tests/native-draw-window-clip-probe.c')]:
            obj = out/(label+'.obj')
            run(label+'-build', [*builder.FLAGS, '-c', ROOT/source, '-o', obj])
            run(label+'-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console',
                               '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:'+str(out/(label+'.exe')),
                               obj, *libraries])
        records = module('clip_reference', 'tests/test-surface-driver.py').load()
        (out/'inputs.bin').write_bytes(b''.join(struct.pack('<13I', *[
            v & 0xffffffff for v in [*(r['input'][k] for k in ['id', 'fast', 'clip', 'flags', 'held']),
                                    *r['input']['source'], *r['input']['destination']]]) for r in records))
        run('clip-execute', ['xvfb-run', '-a', 'wine', out/'clip.exe'], out)
        raw = (out/'outputs.bin').read_bytes()
        if len(raw) != len(records)*780:
            raise ValueError('Incomplete clipping execution')
        mismatches = []
        partial_failures = 0
        for i, expected in enumerate(records):
            row = raw[i*780:(i+1)*780]
            actual = dict(hresult=struct.unpack_from('<I', row, 104)[0],
                          source_before=list(struct.unpack_from('<48H', row, 396)),
                          destination_before=list(struct.unpack_from('<48H', row, 492)),
                          source_after=list(struct.unpack_from('<48H', row, 588)),
                          destination_after=list(struct.unpack_from('<48H', row, 684)),
                          input_after=list(struct.unpack_from('<5I8i', row, 52)))
            for key, value in actual.items():
                if value != expected[key]:
                    mismatches.append(dict(case=i, field=key, actual=value, expected=expected[key]))
            if expected['input']['clip']:
                for key, value in [('clip_size', struct.unpack_from('<I', row, 324)[0]),
                                   ('clip_result', struct.unpack_from('<I', row, 328)[0]),
                                   ('clip_data', list(struct.unpack_from('<16I', row, 332)))]:
                    if value != expected[key]:
                        mismatches.append(dict(case=i, field=key, actual=value, expected=expected[key]))
            partial_failures += bool(expected['hresult'] and expected['destination_after'] != expected['destination_before'])
        report.update(clipping_cases=len(records), clipping_word_checks=len(records)*48*4,
                      partial_failure_cases=partial_failures, clipping_mismatches=mismatches,
                      clipping_output_sha256=sha(out/'outputs.bin'))
        if mismatches:
            raise ValueError('Clipping mismatch: '+repr(mismatches[:3]))
        run('resource-execute', ['xvfb-run', '-a', 'wine', out/'resource.exe'], out)
        result = struct.unpack('<10I', (out/'resource-results.bin').read_bytes())
        if result[0] != 1 or result[2] != 80 or result[6:] != (32, 0, 32, 256):
            raise ValueError('Incomplete native resource policies')
        report['resources'] = dict(checks=result[1], format_cases=result[2], palette_word_checks=result[3],
                                   visible_byte_checks=result[4], padding_byte_checks=result[5],
                                   clip_lifetimes=result[6], final_owners=result[7], flip_lifetimes=result[8], palette_capacity=result[9],
                                   output_sha256=sha(out/'resource-results.bin'))
        # Identical private display geometry for both endpoints; only the driver differs.
        for name, extra in [('window-original', {'WINEDLLOVERRIDES': 'ddraw=b'}),
                            ('window-native', {'MNM_FACTORY_NATIVE': '1'})]:
            dest = out/name
            dest.mkdir()
            shutil.copyfile(out/'MnmDraw.dll', dest/'MnmDraw.dll')
            run(name, ['xvfb-run', '-a', 'wine', out/'window.exe'], dest, extra)
        report['window_driver'] = dict(wine_version=subprocess.check_output(['wine', '--version'], text=True).strip(),
                                      pe32_driver_sha256=sha(prefix/'drive_c/windows/syswow64/ddraw.dll'),
                                      override='ddraw=b', native_override='ddraw=')
        original = (out/'window-original/window-clips.bin').read_bytes()
        native = (out/'window-native/window-clips.bin').read_bytes()
        if len(original) != 6*1036*4 or original != native:
            a = struct.unpack('<'+str(len(original)//4)+'I', original)
            b = struct.unpack('<'+str(len(native)//4)+'I', native)
            report['window_mismatches'] = [dict(word=i, original=x, native=y) for i, (x, y) in enumerate(zip(a, b)) if x != y]
            raise ValueError('Dynamic HWND clip output mismatch: '+repr(report['window_mismatches'][:8]))
        report['window_clips'] = dict(cases=6, words=6*1036, original_sha256=sha(out/'window-original/window-clips.bin'),
                                     native_sha256=sha(out/'window-native/window-clips.bin'))
        report['success'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        if reservation:
            if prefix.exists():
                subprocess.run(['wineserver', '-k'], env=env, timeout=10, check=False)
                subprocess.run(['wineserver', '-w'], env=env, timeout=10, check=False)
            reservation.close()
        report['commands'] = commands
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in claims['sources'].items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
