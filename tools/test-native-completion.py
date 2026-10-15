#!/usr/bin/env python3
"""Bounded native CPU-authority, driver held-flip and resident restart controls."""
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
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--wine-prefix', type=Path, required=True)
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    if any(sha(ROOT/p) != h for p, h in declaration['sources'].items()):
        raise ValueError('Prospective sources changed before execution')
    prefix = args.wine_prefix.resolve()
    if not prefix.is_relative_to(ROOT/'working/tests') or not prefix.is_dir() or prefix.is_symlink():
        raise ValueError('Reuse only an owned stopped test prefix')
    parent = ROOT/'working/tests/native-completion'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(success=False, claims=declaration['claims'], sources=declaration['sources'],
                  scope=__doc__, experiment=str(out.relative_to(ROOT)), live_replacement=False)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINEPREFIX', 'WINEDLLOVERRIDES'))}
    env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all', WINEDLLOVERRIDES='ddraw=', LIBGL_ALWAYS_SOFTWARE='1',
               GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4', QT_QPA_PLATFORM='xcb')
    reservation = None
    verified=False

    def run(label, command, cwd=None, extra=None, expected=0):
        with (out/(label+'.log')).open('x') as log:
            completed = subprocess.run([str(x) for x in command], cwd=cwd or ROOT, env={**env, **(extra or {})},
                                       stdout=log, stderr=log, timeout=180)
        if completed.returncode != expected:
            raise ValueError(f'{label}: return {completed.returncode}, expected {expected}')

    try:
        reservation = module('completion_reservation', 'tools/capture-original-surface-keys.py').reserve_wine()
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        verified=True
        builder = module('completion_builder', 'tools/build-native-draw.py')
        dll = builder.build(out/'build', declaration['sources'])
        shutil.copyfile(dll, out/'MnmDraw.dll')
        imports = {
            'kernel32': {'LoadLibraryA': 4, 'GetProcAddress': 8, 'CreateFileA': 28, 'WriteFile': 20,
                         'CloseHandle': 4, 'ExitProcess': 4, 'GetEnvironmentVariableA': 12, 'Sleep': 4},
            'user32': {'GetDesktopWindow': 0},
            'gdi32': {'CreateDIBSection': 24, 'SelectObject': 8, 'DeleteObject': 4, 'GetDIBColorTable': 16},
        }
        libraries = []
        for name, symbols in imports.items():
            definition = out/(name+'.def')
            definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n, s in symbols.items()))
            library = out/(name+'.lib')
            run(name, ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', library])
            libraries.append(library)
        for name, source in [('history', 'tests/native-history-probe.c'), ('flip', 'tests/native-held-flip-probe.c'), ('gpu-copy', 'tests/native-gpu-copy-probe.c')]:
            obj = out/(name+'.obj')
            run(name+'-compile', [*builder.FLAGS, '-c', ROOT/source, '-o', obj])
            run(name+'-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console',
                              '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:'+str(out/(name+'.exe')), obj, *libraries])
        for name, extra, expected in [('current', {}, 0), ('read-refusal', {'MNM_HISTORY_REFUSE': '1'}, 124),
                                      ('dc-refusal', {'MNM_HISTORY_REFUSE': '2'}, 124),
                                      ('copy-refusal', {'MNM_HISTORY_REFUSE': '3'}, 124)]:
            dest = out/name
            dest.mkdir()
            shutil.copyfile(dll, dest/'MnmDraw.dll')
            run(name, ['xvfb-run', '-a', 'wine', out/'history.exe'], dest, extra, expected)
            if expected:
                q = struct.unpack('<11I', (dest/'native-cpu-history-refusal.bin').read_bytes())
                if q[0:2] != (0x484d4e4d, 2) or q[3:8] != (2, 0, 5, 6, 1):
                    raise ValueError('Wrong history authority refusal')
        run('gpu-copy', ['xvfb-run', '-a', 'wine', out/'gpu-copy.exe'], out)
        report['gpu_copy'] = dict(success=True, cpu_storage_untouched=True, authority_inherited=True, invalid_geometry_lease_key_self_refused=True, full_cpu_copy_restores=True)
        report['history'] = dict(success=True, refusal_cases=3, stale_identity=[2, 0, 5, 6],
                                 interior_pointer=True, partial_write_stays_stale=True,
                                 full_fill_establishes_history=True, retired_pointer_refused=True)
        for name, extra in [('driver', {'WINEDLLOVERRIDES': 'ddraw=b'}), ('native', {'MNM_FACTORY_NATIVE': '1'})]:
            dest = out/name
            dest.mkdir()
            shutil.copyfile(dll, dest/'MnmDraw.dll')
            run(name, ['xvfb-run', '-a', 'wine', out/'flip.exe'], dest, extra)
        original = (out/'driver/held-flips.bin').read_bytes()
        native = (out/'native/held-flips.bin').read_bytes()
        if len(original) != 4*12*4 or native != original:
            raise ValueError('Native held-flip schedules differ from the independent Wine process')
        report['held_flip'] = dict(success=True, cases=4, rows=[list(struct.unpack_from('<12I', native, i*48)) for i in range(4)],
                                   driver_sha256=sha(prefix/'drive_c/windows/syswow64/ddraw.dll'),
                                   wine_version=subprocess.check_output(['wine', '--version'], text=True).strip(),
                                   native_sha256=sha(out/'native/held-flips.bin'), original_sha256=sha(out/'driver/held-flips.bin'))
        report['compatibility']={}
        for label in ['render-resident-world-test','render-command-consumer-test','native-frame-recipe-test']:
            binary=ROOT/'working/build/qt-shell/renderer'/label
            run(label,['xvfb-run','-a',binary])
            report['compatibility'][label]=dict(success=True,sha256=sha(binary))
        run('menu-cpu-read-lease',['xvfb-run','-a',ROOT/'working/build/qt-shell/menu-resident-quit-test'])
        report['menu_cpu_read_lease']=dict(success=True,bounded=True,same_ticket_cannot_renew=True,request_and_state_unchanged=True)
        report['cpu_materialization_binary_sha256']=sha(ROOT/'working/build/qt-shell/native-cpu-materialization-test')
        run('native-cpu-materialization', ['xvfb-run', '-a', ROOT/'working/build/qt-shell/native-cpu-materialization-test'])
        report['cpu_materialization']=json.loads((out/'native-cpu-materialization.log').read_text().splitlines()[-1])
        run('resident-restart', ['xvfb-run', '-a', ROOT/'working/build/qt-shell/resident-terminal-restart-test'])
        report['resident_restart'] = json.loads((out/'resident-restart.log').read_text().splitlines()[-1])
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        report['original_verified_after']=True
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in declaration['sources'].items())
        report['success'] = report['sources_stable'] and report['resident_restart']['success']
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if reservation:
            try:
                subprocess.run(['wineserver', '-k'], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=15)
                subprocess.run(['wineserver', '-w'], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=15, check=True)
            finally:
                reservation.close()
        if verified and not report.get('original_verified_after'):
            run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
            report['original_verified_after']=True
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
