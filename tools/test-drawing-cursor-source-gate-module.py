#!/usr/bin/env python3
"""Run the isolated PE32 diagnostic mutex gate forwarding fixture."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_cursor_gate_journal import decode, account
from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    prefix = parser.add_mutually_exclusive_group(required=True)
    prefix.add_argument('--prefix-template', type=Path)
    prefix.add_argument('--existing-fixture-prefix', type=Path)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_text())
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == 'NR.drawing-cursor-source-gate-module')
    expected = dict(behavior=behavior['id'], contract_sha256=behavior_contract(behavior, {b['id']: b for b in register['builds']}), scenarios={})
    if declaration['claims'] != [expected] or set(declaration['sources']) != set(required_sources(behavior)):
        raise ValueError('Prospective contract/source set differs')
    sources = declaration['sources']
    if any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT / 'working/tests/drawing-cursor-source-gate-module'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    imports = dict(CreateEventA=16,SetEvent=4,CreateThread=24,GetTickCount=0, CreateFileA=28, WriteFile=20, GetProcAddress=8, GetModuleHandleA=4, GetEnvironmentVariableA=12, SetEnvironmentVariableA=8, GetLastError=0, SetLastError=4, VirtualQuery=12, VirtualAlloc=16, VirtualProtect=16, CreateMutexA=12, DuplicateHandle=28, GetCurrentProcess=0, GetCurrentThreadId=0, WaitForSingleObject=8, ReleaseMutex=4, CloseHandle=4, ExitProcess=4, InitializeCriticalSection=4, EnterCriticalSection=4, LeaveCriticalSection=4)
    definition = run / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{name}@{size}\n' for name, size in imports.items()))
    with (run / 'build.log').open('x') as log:
        subprocess.run(['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', str(definition), '-l', str(run / 'kernel32.lib')], stdout=log, stderr=subprocess.STDOUT, check=True)
        subprocess.run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-c', str(ROOT / 'tests/drawing-cursor-source-gate-module-test.c'), '-o', str(run / 'fixture.obj')], stdout=log, stderr=subprocess.STDOUT, check=True)
        binary = run / 'fixture.exe'
        subprocess.run(['lld-link', '/machine:x86', '/base:0x18000000', '/entry:start', '/subsystem:console', '/nodefaultlib', '/safeseh:no', '/timestamp:0', '/out:' + str(binary), str(run / 'fixture.obj'), str(run / 'kernel32.lib')], stdout=log, stderr=subprocess.STDOUT, check=True)
    if args.existing_fixture_prefix:
        fixture_prefix = args.existing_fixture_prefix.resolve()
        fixture_root = (ROOT / 'working/tests/drawing-cursor-source-gate').resolve()
        if not fixture_prefix.is_relative_to(fixture_root) or fixture_prefix.name != 'wineprefix' or not (fixture_prefix.parent / 'execution-status.json').is_file():
            raise ValueError('Existing prefix must belong to a completed isolated gate fixture')
    else:
        fixture_prefix = run / 'wineprefix'
        subprocess.run(['cp', '-a', '--reflink=auto', str(args.prefix_template.resolve()), str(fixture_prefix)], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    capture = run / 'cursor-gate.bin'
    env.update(WINEPREFIX=str(fixture_prefix), WINEDEBUG='-all', MNM_DRAW_CURSOR_SOURCE_GATE_FILE='Z:' + str(capture).replace('/', '\\'))
    try:
        with (run / 'wineboot.log').open('x') as log:
            subprocess.run(['wineboot', '-u'], env=env, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run / 'execution.log').open('x') as log:
            result = subprocess.run(['wine', str(binary)], env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90)
        (run / 'execution-status.json').write_text(json.dumps(dict(return_code=result.returncode)) + '\n')
        if result.returncode:
            failure = dict(schema=1, success=False, sources=sources, claims=declaration['claims'], artifacts=str(run.relative_to(ROOT)), return_code=result.returncode, original_game_executed=False, live_capture_integrated=False, scope='Owned PE32 forwarding fixture failed; retain raw exit and logs, no ownership/lifetime validation claim.')
            with args.report.open('x') as out:
                json.dump(failure, out, indent=2)
                out.write('\n')
            raise ValueError('Cursor gate forwarding PE32 fixture exit: ' + str(result.returncode))
        if any(sha((ROOT / name).read_bytes()) != digest for name, digest in sources.items()):
            raise ValueError('Sources changed during execution')
        current = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
        current_behavior = next(b for b in current['behaviors'] if b['id'] == behavior['id'])
        if behavior_contract(current_behavior, {b['id']: b for b in current['builds']}) != expected['contract_sha256']:
            raise ValueError('Behavior contract changed during execution')
        observation=decode(capture.read_bytes())
        if observation['complete'] or observation['terminal_reason']!=3 or observation['active_leases'] or observation['fault'] or observation['orphans']:raise ValueError('Snapshot boundary differs')
        accounting=account(observation)
        if accounting['gaps'] or accounting['remaining_live_identities']:raise ValueError('Lifecycle pairing differs')
        if sum(row['phase']==11 for row in observation['rows'])!=2:raise ValueError('Live snapshot retirement events missing')
        report = dict(schema=1, success=True, sources=sources, claims=declaration['claims'], sources_stable=True, artifacts=str(run.relative_to(ROOT)), binary_sha256=sha(binary.read_bytes()), original_game_executed=False, live_capture_integrated=False, journal=observation, lifecycle_accounting=accounting, journal_sha256=sha(capture.read_bytes()), complete_journal_admitted=False, actual_mutex_handles=True, actual_contended_outer_refusal=True, refused_attempt_token_retained=True, refusal_leases_drained=True, full_fx_state_preserved=True, last_error_preserved=True, relocated_owned_installer=True, changed_byte_anchor_refusals=4, original_import_mismatch_refusal=True, exact_create_close_forward_cells=4, original_forwards_per_cell=1, scope='Owned PE32 relocated installer byte refusal, real kernel Create/duplicate/Close and event-synchronized competing outer owner. Failed begin returns0 with nonzero attempted token/raw timeout, no held levels/leases; exact512FX/error retained. Successful token/retirement/drain and prior forwarding guards retained. Production pins unchanged; no original game/frontend/source/worker or complete reference/parity.')
        report.update(live_snapshot_retirement_events=2,post_snapshot_begin_point_valid=True,post_snapshot_begin_reason=17,post_snapshot_begin_admitted=False)
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, actual_mutex_handles=True)))
    finally:
        subprocess.run(['wineserver', '-k'], env=env, check=False, timeout=10)


if __name__ == '__main__':
    main()
