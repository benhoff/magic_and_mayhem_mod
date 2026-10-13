#!/usr/bin/env python3
"""Run bounded PE32 auxiliary probe guards with synthetic callbacks, not the game."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_aux_save_readback import analyze

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    claims = json.loads(a.claims.read_bytes())
    sources = claims['sources']
    sha = lambda data: hashlib.sha256(data).hexdigest()
    if any(sha((ROOT / name).read_bytes()) != h for name, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT / 'working/tests/drawing-aux-save-readback'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    spec = importlib.util.spec_from_file_location('shadow_aux', ROOT / 'tools/build-shadow-bridge.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    imports = dict(module.IMPORTS, GetProcAddress=8, GetCurrentThreadId=0, GetTickCount=0,
                   InitializeCriticalSection=4, EnterCriticalSection=4, LeaveCriticalSection=4,
                   VirtualFree=12, CreateThread=24, WaitForSingleObject=8)
    definition = run / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{k}@{v}\n' for k, v in imports.items()))
    subprocess.run(['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', str(definition), '-l', str(run / 'kernel32.lib')], check=True)
    binary = run / 'fixture.exe'
    subprocess.run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
                    '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
                    '-c', str(ROOT / 'tests/drawing-aux-save-readback-test.c'), '-o', str(run / 'fixture.obj')], check=True)
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:' + str(binary), str(run / 'fixture.obj'),
                    str(run / 'kernel32.lib')], check=True)
    prefix = run / 'wineprefix'
    subprocess.run(['cp', '-a', '--reflink=auto', str(a.prefix_template), str(prefix)], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all', MNM_DRAW_AUX_SAVE_READBACK='1')
    with (run / 'wineboot.log').open('x') as log:
        subprocess.run(['wineboot', '-u'], env=env, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
    statuses = {}
    for name in ['complete', 'forced']:
        capture = run / name
        capture.mkdir()
        current = dict(env, MNM_AUX_TEST_DIR='Z:' + str(capture).replace('/', '\\'))
        if name == 'forced':
            current['MNM_AUX_TEST_FORCED'] = '1'
        result = subprocess.run(['wine', str(binary)], env=current, capture_output=True, timeout=90)
        (run / (name + '.log')).write_bytes(result.stdout + result.stderr)
        statuses[name] = result.returncode
        result.check_returncode()
    observed = analyze(run / 'complete')
    if (len(observed['samples']) != 256 or len(observed['images']) != 247 or observed['refused'] != 9
            or observed['budget_skips'] != 1 or observed['original_driver_state_restored']):
        raise ValueError('Synthetic probe fault/budget accounting changed')
    expected = b''.join(((17 + i * 37) & 65535).to_bytes(2, 'little') for i in range(1600))
    if any(i['sha256'] != sha(expected) for i in observed['images']):
        raise ValueError('Mapped-view row/padding copy changed')
    try:
        analyze(run / 'forced')
    except ValueError as error:
        forced_reason = str(error)
    else:
        raise ValueError('Mid-call close incorrectly admitted')
    if any(sha((ROOT / name).read_bytes()) != h for name, h in sources.items()):
        raise ValueError('Fixture source changed during execution')
    report = dict(schema=1, success=True, sources=sources, claims=claims['claims'],
                  artifacts=str(run.relative_to(ROOT)), binary_sha256=sha(binary.read_bytes()),
                  exits=statuses, observation=observed, forced_close_refusal=forced_reason,
                  default_off_no_getter_or_driver_calls=True, error_x87_preserved=True,
                  reentrant_driver_without_journal_mutex=True, original_game_executed=False,
                  original_pixels_used_as_native_inputs=False,
                  scope='Synthetic PE32 selected-source callback/descriptor/Lock/Unlock failures, mapped RGB565 row padding, supported legacy/extended descriptor ABI, default-off, outside-queue/failed/keyed/partial/unknown-interface refusal, error/x87 preservation, no driver call under journal mutex, exact256attempt budget and mid-call-close rejection. No actual game/driver/backing equivalence.')
    with a.report.open('x') as output:
        json.dump(report, output, indent=2)
        output.write('\n')
    print(json.dumps(dict(success=True, samples=256, images=247, refused=9, forced_close_refused=True)))


if __name__ == '__main__':
    main()
