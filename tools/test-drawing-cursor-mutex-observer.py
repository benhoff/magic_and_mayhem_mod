#!/usr/bin/env python3
"""Run actual PE32 original mutex import forwarding over owned callbacks."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_cursor_mutex import decode

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prefix-template', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_text())
    sources = declaration['sources']
    if any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT / 'working/tests/drawing-cursor-mutex'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    imports = dict(GetProcAddress=8, GetModuleHandleA=4, GetEnvironmentVariableA=12, GetLastError=0, SetLastError=4, GetCurrentThreadId=0, GetTickCount=0, InitializeCriticalSection=4, EnterCriticalSection=4, LeaveCriticalSection=4, CloseHandle=4, VirtualQuery=12, VirtualProtect=16, CreateFileA=28, WriteFile=20, ExitProcess=4)
    definition = run / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{name}@{size}\n' for name, size in imports.items()))
    with (run / 'build.log').open('x') as log:
        subprocess.run(['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', str(definition), '-l', str(run / 'kernel32.lib')], stdout=log, stderr=subprocess.STDOUT, check=True)
        subprocess.run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function', '-c', str(ROOT / 'tests/drawing-cursor-mutex-test.c'), '-o', str(run / 'fixture.obj')], stdout=log, stderr=subprocess.STDOUT, check=True)
        binary = run / 'fixture.exe'
        subprocess.run(['lld-link', '/machine:x86', '/base:0x18000000', '/entry:start', '/subsystem:console', '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:' + str(binary), str(run / 'fixture.obj'), str(run / 'kernel32.lib')], stdout=log, stderr=subprocess.STDOUT, check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(args.prefix_template.resolve()), str(run / 'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    capture = run / 'cursor-mutex.bin'
    env.update(WINEPREFIX=str(run / 'wineprefix'), WINEDEBUG='-all', MNM_DRAW_CURSOR_MUTEX_FILE='Z:' + str(capture).replace('/', '\\'))
    try:
        with (run / 'wineboot.log').open('x') as log:
            subprocess.run(['wineboot', '-u'], env=env, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run / 'execution.log').open('x') as log:
            result = subprocess.run(['wine', str(binary)], env=env, stdout=log, stderr=subprocess.STDOUT, timeout=30)
        (run / 'execution-status.json').write_text(json.dumps(dict(return_code=result.returncode)) + '\n')
        if result.returncode:
            raise ValueError('Mutex PE32 fixture exit: ' + str(result.returncode))
        observation = decode(capture.read_bytes())
        if len(observation['calls']) != 20 or observation['created_identities'] != 2 or observation['remaining_live_ids']:
            raise ValueError('Mutex fixture matrix differs')
        if any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
            raise ValueError('Sources changed during execution')
        report = dict(schema=1, success=True, sources=sources, claims=declaration['claims'], sources_stable=True, artifacts=str(run.relative_to(ROOT)), binary_sha256=sha(binary.read_bytes()), capture_sha256=sha(capture.read_bytes()), observation=observation, original_callback_forwards=20, full_fx_state_preserved=True, result_last_error_arguments_preserved=True, handle_reuse_has_distinct_ids=True, point_query_does_not_poll_input=True, original_game_executed=False, scope='Actual PE32 wrapper/decoder with20single synthetic original Create/Wait/Release/Close forwards. Three timeouts and four raw Wait responses, release0/1, failed/successful close, reused handle withnewID and unknownhandle visible. Exact stdcall args/results/error/512FX despite deliberately clobbering point query. No original game, production IAT installation, real mutex ownership/aliases or source protection/native parity proof.')
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, forwards=20, created_ids=2)))
    finally:
        subprocess.run(['wineserver', '-k'], env=env, check=False, timeout=10)


if __name__ == '__main__':
    main()
