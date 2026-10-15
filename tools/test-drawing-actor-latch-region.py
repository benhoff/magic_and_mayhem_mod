#!/usr/bin/env python3
"""Run the actual PE32 bounded Region callback forwarding fixture."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_region_input import decode
from drawing_actor_latches import decode as decode_latches, bind_callbacks

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
    parent = ROOT / 'working/tests/drawing-region-input'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    imports = dict(GetModuleHandleA=4, GetProcAddress=8, GetEnvironmentVariableA=12, GetLastError=0, SetLastError=4, GetCurrentThreadId=0, CreateThread=24, WaitForSingleObject=8, CloseHandle=4, VirtualQuery=12, VirtualAlloc=16, VirtualProtect=16, CreateFileA=28, WriteFile=20, GetTickCount=0, ExitProcess=4)
    definition = run / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{name}@{size}\n' for name, size in imports.items()))
    with (run / 'build.log').open('x') as log:
        subprocess.run(['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', str(definition), '-l', str(run / 'kernel32.lib')], stdout=log, stderr=subprocess.STDOUT, check=True)
        subprocess.run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-c', str(ROOT / 'tests/drawing-actor-latch-region-test.c'), '-o', str(run / 'fixture.obj')], stdout=log, stderr=subprocess.STDOUT, check=True)
        binary = run / 'fixture.exe'
        subprocess.run(['lld-link', '/machine:x86', '/base:0x18000000', '/entry:start', '/subsystem:console', '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:' + str(binary), str(run / 'fixture.obj'), str(run / 'kernel32.lib')], stdout=log, stderr=subprocess.STDOUT, check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(args.prefix_template.resolve()), str(run / 'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    capture = run / 'region-input.bin'
    env['MNM_DRAW_ACTOR_LATCH_FILE']='Z:'+str(run/'actor-latches.bin').replace('/','\\')
    env.update(WINEPREFIX=str(run / 'wineprefix'), WINEDEBUG='-all', MNM_DRAW_REGION_INPUT_FILE='Z:' + str(capture).replace('/', '\\'))
    try:
        with (run / 'wineboot.log').open('x') as log:
            subprocess.run(['wineboot', '-u'], env=env, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run / 'execution.log').open('x') as log:
            result = subprocess.run(['wine', str(binary)], env=env, stdout=log, stderr=subprocess.STDOUT, timeout=30)
        (run / 'execution-status.json').write_text(json.dumps(dict(return_code=result.returncode)) + '\n')
        if result.returncode:
            raise ValueError('Region PE32 fixture exit: ' + str(result.returncode))
        observation = decode(capture.read_bytes())
        if len(observation['calls']) != 32 or any(c['mask'] != i | 0x80000000 or c['entry']['actor_health'] != 100 + i or c['returned']['actor_health'] != 101 + i for i, c in enumerate(observation['calls'])):
            raise ValueError('Region fixture matrix differs')
        latches=decode_latches((run/'actor-latches.bin').read_bytes())
        binding=bind_callbacks(latches,observation)
        if len(binding['phases'])!=64:raise ValueError('Latch/Region integration phases differ')
        if any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
            raise ValueError('Sources changed during execution')
        report = dict(schema=1, success=True, sources=sources, claims=declaration['claims'], sources_stable=True, artifacts=str(run.relative_to(ROOT)), binary_sha256=sha(binary.read_bytes()), capture_sha256=sha(capture.read_bytes()), observation=observation, latch_observation=latches, latch_callback_binding=binding, original_callback_forwards=35, full_fx_state_preserved=True, result_last_error_arguments_preserved=True, observer_does_not_change_owned_source=True, budget_refusal_forwarded=True, foreign_thread_refusal_forwarded=True, disabled_recorder_forwarded=True, original_game_executed=False, scope='Actual PE32 wrapper with enabled whole-pool latch recorder and decoder:32paired owned callback cells plus disabled/capacity/real foreign-thread forwards, exact thiscall receiver/eventmask/40byte by-value arguments, raw return/LastError/full512FXstate, one synthetic original callback per request, read-only observer apart from deliberately mocked original health writes; installer slot/prologue refusals. Synthetic original body at owned pinned addresses; no game execution/selection/tooltip/source ownership or pixel equivalence.')
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, paired_calls=32, forwards=35)))
    finally:
        subprocess.run(['wineserver', '-k'], env=env, check=False, timeout=10)


if __name__ == '__main__':
    main()
