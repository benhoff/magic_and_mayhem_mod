#!/usr/bin/env python3
"""Run the isolated PE32 nonblocking mutex ownership fixture."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
from coverage_claims import behavior_contract, required_sources

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
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == 'NR.drawing-cursor-source-gate')
    expected = dict(behavior=behavior['id'], contract_sha256=behavior_contract(behavior, {b['id']: b for b in register['builds']}), scenarios={})
    if declaration['claims'] != [expected] or set(declaration['sources']) != set(required_sources(behavior)):
        raise ValueError('Prospective contract/source set differs')
    sources = declaration['sources']
    if any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT / 'working/tests/drawing-cursor-source-gate'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    imports = dict(CreateMutexA=12, ReleaseMutex=4, CreateEventA=16, SetEvent=4, GetExitCodeThread=8, CreateThread=24, WaitForSingleObject=8, CloseHandle=4, ExitProcess=4)
    definition = run / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{name}@{size}\n' for name, size in imports.items()))
    with (run / 'build.log').open('x') as log:
        subprocess.run(['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', str(definition), '-l', str(run / 'kernel32.lib')], stdout=log, stderr=subprocess.STDOUT, check=True)
        subprocess.run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-c', str(ROOT / 'tests/drawing-cursor-source-gate-test.c'), '-o', str(run / 'fixture.obj')], stdout=log, stderr=subprocess.STDOUT, check=True)
        binary = run / 'fixture.exe'
        subprocess.run(['lld-link', '/machine:x86', '/base:0x18000000', '/entry:start', '/subsystem:console', '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:' + str(binary), str(run / 'fixture.obj'), str(run / 'kernel32.lib')], stdout=log, stderr=subprocess.STDOUT, check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(args.prefix_template.resolve()), str(run / 'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run / 'wineprefix'), WINEDEBUG='-all')
    try:
        with (run / 'wineboot.log').open('x') as log:
            subprocess.run(['wineboot', '-u'], env=env, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run / 'execution.log').open('x') as log:
            result = subprocess.run(['wine', str(binary)], env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90)
        (run / 'execution-status.json').write_text(json.dumps(dict(return_code=result.returncode)) + '\n')
        if result.returncode:
            raise ValueError('Cursor source gate PE32 fixture exit: ' + str(result.returncode))
        if any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
            raise ValueError('Sources changed during execution')
        if json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())['behaviors'] != register['behaviors']:
            raise ValueError('Behavior contracts changed during execution')
        report = dict(schema=1, success=True, sources=sources, claims=declaration['claims'], sources_stable=True, artifacts=str(run.relative_to(ROOT)), binary_sha256=sha(binary.read_bytes()), scripted_failure_cells=64, real_mutex_scenarios=['uncontended pair', 'preserved recursive ownership', 'competing outer owner', 'competing inner owner and outer unwind', 'abandoned outer', 'abandoned inner', 'null and duplicate handles/callbacks and invalid handle'], original_game_executed=False, live_capture_integrated=False, scope='Actual PE32 real Wine kernel mutex fixture plus64scripted wait/release failure cells. Both waits use timeout0; release reverse order, retain failed-release ownership bits, reject abandoned acquisition after unwinding acquired recursion. Caller-pinned handles required; no original game, production lifetime, live capture integration or full source/native parity.')
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, scripted_cells=64, real_mutex_scenarios=7)))
    finally:
        subprocess.run(['wineserver', '-k'], env=env, check=False, timeout=10)


if __name__ == '__main__':
    main()
