#!/usr/bin/env python3
"""Run an owned PE32 copy journal fixture with real thread return reversal."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_copy_order import decode

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    claims = json.loads(a.claims.read_text())
    sources = dict(claims['sources'])
    sources.update({n: sha((ROOT/n).read_bytes()) for n in ['tools/test-drawing-copy-order.py', 'tests/drawing-copy-order-test.c', 'tools/drawing_copy_order.py']})
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT / 'working/tests/drawing-copy-order'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    imports = dict(ExitProcess=4, GetEnvironmentVariableA=12, GetLastError=0, SetLastError=4,
                   GetCurrentThreadId=0, GetTickCount=0, CreateFileA=28, WriteFile=20,
                   CloseHandle=4, CreateThread=24, WaitForSingleObject=8,
                   InitializeCriticalSection=4, EnterCriticalSection=4, LeaveCriticalSection=4)
    definition = run / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{n}@{size}\n' for n, size in imports.items()))
    subprocess.run(['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', str(definition), '-l', str(run/'kernel32.lib')], check=True)
    flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
             '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror']
    subprocess.run([*flags, '-c', str(ROOT/'tests/drawing-copy-order-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run / 'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'), str(run/'kernel32.lib')], check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(a.prefix_template.resolve()), str(run/'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'), WINEDEBUG='-all', MNM_DRAW02_PRODUCER_BIND='1')
    cases = []
    try:
        with (run/'wineboot.log').open('x') as log:
            subprocess.run(['wineboot', '-u'], env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90, check=True)
        for name, mode, expected in [('reverse', '1', 0), ('disabled', None, 0), ('invalid', 'x', 0), ('partial', '1', 0), ('budget', '1', 99)]:
            directory = run / name
            directory.mkdir()
            extra = dict(MNM_FIXTURE_DIRECTORY='Z:'+str(directory).replace('/', '\\'), MNM_COPY_ORDER_CASE=name)
            if mode is not None:
                extra['MNM_DRAW_COPY_ORDER'] = mode
            with (directory/'fixture.log').open('x') as log:
                process = subprocess.run(['wine', str(binary)], env=dict(env, **extra), cwd=directory,
                                         stdout=log, stderr=subprocess.STDOUT, timeout=30)
            if process.returncode != expected:
                raise ValueError(f'{name} expected {expected}, got {process.returncode}')
            file = directory/'copy-order-0000.bin'
            if name == 'reverse':
                observation = decode(file.read_bytes())
                if [r['request'] for r in observation['returns']] != [2, 1] or len({r['owner'] for r in observation['entries']}) != 2:
                    raise ValueError('Actual two-thread reversal missing')
            elif name in ('partial', 'budget'):
                try:
                    decode(file.read_bytes())
                except ValueError:
                    observation = dict(incomplete_refused=True, sha256=sha(file.read_bytes()))
                else:
                    raise ValueError('Incomplete journal admitted')
            else:
                if file.exists():
                    raise ValueError('Disabled/invalid mode wrote a journal')
                observation = dict(no_journal=True)
            cases.append(dict(case=name, exit=process.returncode, observation=observation))
        if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
            raise ValueError('Fixture sources changed')
        report = dict(schema=1, success=True, sources=sources, sources_stable=True,
                      claims=claims['claims'], artifacts=str(run.relative_to(ROOT)), binary_sha256=sha(binary.read_bytes()),
                      cases=cases, original_game_executed=False, last_error_preserved=True,
                      request_descriptor_unchanged=True, driver_order_or_api_binding_proved=False,
                      scope='Actual owned PE32 two-thread copy request entry/return journal, reversed returns, disabled/invalid configuration, incomplete terminal and row-budget refusal. No original game, driver/API association or native replay correction.')
        with a.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, cases=len(cases), artifacts=str(run))))
    finally:
        subprocess.run(['wineserver', '-k'], env=env, timeout=10, check=False)


if __name__ == '__main__':
    main()
