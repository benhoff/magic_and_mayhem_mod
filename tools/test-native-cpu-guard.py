#!/usr/bin/env python3
"""Actual PE32 idle and active writable Lock census and private snapshot/World handoff controls."""
import argparse
import hashlib
import fcntl
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
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def reserve_prefix(prefix):
    # These tests use neither original-driver windows nor another installation.
    # Serialize this exact private prefix and refuse any live process using it.
    handle = (prefix.parent / (prefix.name + '.cpu-guard.lock')).open('a')
    try:
        fcntl.flock(handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
        for process in Path('/proc').iterdir():
            if not process.name.isdecimal() or int(process.name) == os.getpid():
                continue
            try:
                for item in (process / 'environ').read_bytes().split(b'\0'):
                    if item.startswith(b'WINEPREFIX=') and Path(os.fsdecode(item[11:])).resolve() == prefix:
                        raise RuntimeError('Private prefix is already active')
            except (FileNotFoundError, PermissionError, ProcessLookupError):
                continue
    except BaseException:
        handle.close()
        raise
    return handle


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--wine-prefix', type=Path, required=True)
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    if any(sha(ROOT / p) != h for p, h in declaration['sources'].items()):
        raise ValueError('Prospective source changed before execution')
    prefix = args.wine_prefix.resolve()
    if not prefix.is_dir() or not prefix.is_relative_to(ROOT / 'working/tests'):
        raise ValueError('Use only an isolated owned test prefix')
    parent = ROOT / 'working/tests/native-cpu-guard'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(success=False, claims=declaration['claims'], sources=declaration['sources'],
                  scope=__doc__, cases=[], complete_replacement=False, live_game_executed=False,
                  automatic_cpu_materialization=False, active_cpu_lease_reads_covered=False,
                  active_writable_lock_loads_covered=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINEPREFIX', 'WINEDLLOVERRIDES'))}
    env.pop('DISPLAY', None)
    env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all', WINEDLLOVERRIDES='ddraw=',
               LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='4')
    reservation = None

    def run(label, command, cwd=None, extra=None, expected=0):
        with (out / (label + '.log')).open('x') as log:
            result = subprocess.run([str(p) for p in command], cwd=cwd or out,
                                    env={**env, **(extra or {})}, stdout=log, stderr=log, timeout=120)
        if result.returncode != expected:
            raise ValueError(f'{label}: return {result.returncode}, expected {expected}')

    try:
        reservation = reserve_prefix(prefix)
        report['isolation'] = dict(private_prefix=str(prefix), prefix_serialized=True, display_access=False, native_cpu_only=True, unrelated_prefixes_untouched=True)
        builder = module('guard_native_builder', 'tools/build-native-draw.py')
        dll = builder.build(out / 'build', declaration['sources'])
        imports = {k: dict(v) for k, v in builder.IMPORTS.items()}
        imports['kernel32'].update(LoadLibraryA=4, FreeLibrary=4, GetProcAddress=8, VirtualQuery=12)
        imports['user32'].update(GetDesktopWindow=0)
        imports['gdi32'].update(GetDIBColorTable=16)
        libraries = []
        for name, symbols in imports.items():
            definition = out / (name + '.def')
            definition.write_text('LIBRARY ' + name + '.dll\nEXPORTS\n' + ''.join(f'{n}@{s}\n' for n, s in symbols.items()))
            library = out / (name + '.lib')
            run(name, ['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', library])
            libraries.append(library)
        obj, dep = out / 'probe.obj', out / 'probe.d'
        run('compile', [*builder.FLAGS, '-MD', '-MF', dep, '-c', ROOT / 'tests/native-cpu-guard-probe.c', '-o', obj])
        closure = set()
        for token in dep.read_text().replace('\\\n', ' ').split():
            path = Path(token).resolve()
            if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT / 'working'):
                closure.add(path.relative_to(ROOT).as_posix())
        if closure - set(declaration['sources']):
            raise ValueError('Undeclared probe compiler inputs: ' + repr(sorted(closure - set(declaration['sources']))))
        run('link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                     '/safeseh:no', '/timestamp:0', '/out:' + str(out / 'probe.exe'), obj, *libraries])
        report['native_build'] = json.loads((out / 'build/manifest.json').read_text())
        report['probe_sha256'] = sha(out / 'probe.exe')
        cases = [('current', '0', 0), ('disabled-control', 'd', 0), ('raw-read', 'r', 124),
                 ('raw-write', 'w', 124), ('interior-read', 'i', 124), ('writable-lease-rearm', 'l', 124),
                 ('active-lease-boundary', 'a', 0), ('matching-transfer', 't', 0), ('wrong-transfer', 'n', 124),
                 ('full-fill', 'f', 0), ('partial-fill', 'p', 124), ('full-cpu-copy', 'c', 0),
                 ('gpu-copy-baseline', 'b', 124), ('gpu-self-plan', 's', 124), ('read-only-lock', 'o', 124),
                 ('physical-held-flip', 'v', 124), ('foreign-fault-chain', 'q', 123), ('dll-unload-handler', 'u', 0),
                 ('dc-lease-rearm', 'h', 124), ('permission-owner-refusal', 'e', 124),
                 ('active-load-after-stores', 'A', 124), ('active-stores-idle-rearm', 'S', 124),
                 ('active-rmw-refusal', 'R', 124), ('active-rep-refusal', 'P', 124),
                 ('active-new-generation', 'B', 124), ('private-lease-snapshot', 'K', 0),
                 ('world-guard-handoff', 'W', 124), ('unfinished-world-loan', 'U', 124)]
        for label, mode, expected in cases:
            dest = out / label
            dest.mkdir()
            shutil.copyfile(dll, dest / 'MnmDraw.dll')
            extra = dict(MNM_CPU_GUARD_CASE=mode)
            if mode != 'd':
                extra['MNM_NATIVE_CPU_GUARD'] = '2' if mode.isupper() else '1'
            run(label, ['wine', out / 'probe.exe'], dest, extra, expected)
            fault = dest / 'native-cpu-guard-fault.bin'
            row = dict(id=label, mode=mode, exit_code=expected, success=True)
            if expected == 124 and mode not in ('o', 'e', 'U'):
                raw = fault.read_bytes()
                if len(raw) != 96:
                    raise ValueError('Wrong guard record size')
                q = struct.unpack('<24I', raw)
                witness = struct.unpack('<5I', (dest / 'cpu-guard-input.bin').read_bytes())
                offset = witness[1] * 9 - 2 if mode == 'i' else 0
                if (raw[:8] != b'MNMCPUF1' or q[2:4] != (1, 96) or not q[4] or
                        q[5] != witness[0] + offset or q[6] != (1 if mode in ('w', 'R', 'P') else 0) or not q[7] or
                        q[8] != witness[0] or q[9] != witness[1] * 9 or q[10] != 4096 or
                        q[11:14] != (13, 9, witness[1]) or q[14:18] != (3 if mode in ('B', 'W') else 2, 0, 5, 6) or
                        not q[18] or not q[19] or q[20:] != (1 if mode in ('A', 'R', 'P', 'B', 'W') else 0, 0, 1, 0)):
                    raise ValueError('Wrong fault identity/access/physical generation: ' + repr(q))
                row.update(fault_sha256=sha(fault), record=list(q), input_sha256=sha(dest / 'cpu-guard-input.bin'))
            elif fault.exists():
                raise ValueError('Unexpected owned stale fault: ' + label)
            elif mode == 'e':
                q = struct.unpack('<6I', (dest / 'native-cpu-guard-refusal.bin').read_bytes())
                if q[:2] != (0x474d4e4d, 1) or not q[2] or q[3:] != (0, 2, 1):
                    raise ValueError('Permission mismatch did not refuse before mutation')
                row['permission_refusal'] = list(q)
            elif mode in ('o', 'U'):
                q = struct.unpack('<11I', (dest / 'native-cpu-history-refusal.bin').read_bytes())
                if q[:2] != (0x484d4e4d, 2) or q[3:8] != (2, 0, 5, 6, 1):
                    raise ValueError('Read-only Lock did not use selected history refusal')
            elif expected == 0 and mode != 'a':
                if struct.unpack('<I', (dest / 'cpu-guard-success.bin').read_bytes())[0] < 10:
                    raise ValueError('Incomplete success case')
            if mode != 'e' and (dest / 'native-cpu-guard-refusal.bin').exists():
                raise ValueError('Guard permission ownership failure')
            if mode.isupper():
                m=struct.unpack('<14I',(dest/'native-cpu-guard-modules.bin').read_bytes())
                if m[:4]!=(0x434d4e4d,0x32304d47,2,56) or not m[4] or not m[5] or any(m[6:]):
                    raise ValueError('Invalid initialization-time module identities')
                row['module_catalog']=list(m)
            report['cases'].append(row)
        report['default_regressions'] = {}
        for label, source in [('history', 'tests/native-history-probe.c'),
                              ('gpu-copy', 'tests/native-gpu-copy-probe.c'),
                              ('resources', 'tests/native-draw-resource-probe.c')]:
            obj = out / (label + '.obj')
            dep = out / (label + '.d')
            run(label + '-compile', [*builder.FLAGS, '-MD', '-MF', dep, '-c', ROOT / source, '-o', obj])
            for token in dep.read_text().replace('\\\n', ' ').split():
                path = Path(token).resolve()
                if (path.is_file() and path.is_relative_to(ROOT) and
                        not path.is_relative_to(ROOT / 'working') and
                        path.relative_to(ROOT).as_posix() not in declaration['sources']):
                    raise ValueError('Undeclared default regression input: ' + str(path))
            run(label + '-link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console',
                                 '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:' + str(out / (label + '.exe')),
                                 obj, *libraries])
            dest = out / ('default-' + label)
            dest.mkdir()
            shutil.copyfile(dll, dest / 'MnmDraw.dll')
            run('default-' + label, ['wine', out / (label + '.exe')], dest)
            if (dest / 'native-cpu-guard-fault.bin').exists():
                raise ValueError('Default behavior unexpectedly enabled the guard')
            row = dict(success=True, probe_sha256=sha(out / (label + '.exe')))
            if label == 'resources':
                q = struct.unpack('<10I', (dest / 'resource-results.bin').read_bytes())
                if q[0] != 1 or q[2] != 80 or q[7] != 0:
                    raise ValueError('Incomplete default resource control')
                row['results'] = list(q)
            report['default_regressions'][label] = row
        for mode in ('1', '2', '3'):
            label = 'default-history-refusal-' + mode
            dest = out / label
            dest.mkdir()
            shutil.copyfile(dll, dest / 'MnmDraw.dll')
            run(label, ['wine', out / 'history.exe'], dest, dict(MNM_HISTORY_REFUSE=mode), 124)
            q = struct.unpack('<11I', (dest / 'native-cpu-history-refusal.bin').read_bytes())
            if q[:2] != (0x484d4e4d, 2) or q[3:8] != (2, 0, 5, 6, 1):
                raise ValueError('Wrong default stale reader refusal')
            report['default_regressions'][label] = dict(success=True, exit_code=124)
        report['success'] = True
    except (OSError, ValueError, subprocess.SubprocessError, RuntimeError) as error:
        report['error'] = str(error)
    finally:
        if reservation:
            subprocess.run(['wineserver', '-k'], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=15)
            subprocess.run(['wineserver', '-w'], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=15)
            reservation.close()
        report['sources_stable'] = all(sha(ROOT / p) == h for p, h in declaration['sources'].items())
        report['success'] &= report['sources_stable']
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(out / 'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
