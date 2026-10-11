#!/usr/bin/env python3
"""Build/run the real census callbacks in a synthetic PE32 ABI fixture."""
import argparse
import hashlib
import importlib.util
import json
import os
import re
from pathlib import Path
import subprocess
import tempfile
from drawing_census import ROOT


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prefix-template', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    parent = ROOT / 'working/tests/drawing-census-abi'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    paths = ['tools/test-drawing-census-abi.py', 'tests/drawing-census-abi.c', 'tests/canvas-startup-call.S',
             'runtime/drawing-census/entry.S', 'runtime/drawing-census/observer.c',
             'runtime/shadow/win32_min.h', 'protocols/include/mnm/drawing_census_v1.h',
             'tools/build-menu-observer.py']
    sources = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}
    claims = json.loads(args.claims.read_text()) if args.claims else None
    if claims:
        for p, digest in claims['sources'].items():
            if hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != digest: raise ValueError('Changed declaration source: ' + p)
            sources[p] = digest
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(out / 'wineprefix'), WINEDEBUG='-all', WINEDLLOVERRIDES='mscoree,mshtml=d')
    spec = importlib.util.spec_from_file_location('abi_imports', ROOT / 'tools/build-menu-observer.py')
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    (out / 'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(
        '%s@%d\n' % item for item in module.IMPORTS.items()))
    def run(name, command, seconds=90):
        with (out / (name + '.log')).open('x') as log:
            subprocess.run([str(v) for v in command], env=env, stdout=log, stderr=subprocess.STDOUT,
                           timeout=seconds, check=True)
    run('imports', ['llvm-dlltool', '-m', 'i386', '-D', 'KERNEL32.dll', '-d', out / 'kernel32.def',
                    '-l', out / 'kernel32.lib', '--kill-at'])
    declarations = ['#define CENSUS_HOOKS 9',
        'struct Hook {u32 address,length,pop,site,relocs;u8 bytes[16];u32 reloc[3];void(*entry)(void);};',
        'static const u8 original_sha256[32]={0};']
    declarations += ['extern void census_enter_%d(void);' % tag for tag in range(9)]
    declarations += ['static const struct Hook hooks[9]={'] + [
        '{%d,5,%d,%d,0,{0},{0},census_enter_%d},' % (0x500100 + tag * 0x100, pop, tag == 6, tag)
        for tag, pop in enumerate([0, 4, 8, 12, 20, 24, 8, 8, 16])] + ['};']
    (out / 'hooks.inc').write_text('\n'.join(declarations) + '\n')
    (out / 'entry.S').write_text((ROOT / 'runtime/drawing-census/entry.S').read_text() + '\n' +
                                '\n'.join('census_entry %d' % tag for tag in range(9)) + '\n')
    objects = []
    for i, source in enumerate([ROOT / 'tests/drawing-census-abi.c', ROOT / 'tests/canvas-startup-call.S', out / 'entry.S']):
        obj = out / ('part%d.obj' % i); objects.append(obj)
        run('compile%d' % i, ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
            '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-I', out, '-c', source, '-o', obj])
    run('link', ['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
        '/base:0x400000', '/dynamicbase:no', '/safeseh:no', '/timestamp:0', '/export:census_leave', '/out:' + str(out / 'fixture.exe'),
        *objects, out / 'kernel32.lib'])
    exports = subprocess.check_output(['objdump', '-p', str(out / 'fixture.exe')], text=True)
    (out / 'export-table.txt').write_text(exports)
    if not re.search(r'^\s*\[\s*\d+\].*\bcensus_leave\s*$', exports, re.M):
        raise ValueError('Fixture named census leave export missing')
    run('prefix-copy', ['cp', '-a', '--reflink=auto', args.prefix_template.resolve(), out / 'wineprefix'])
    run('forwarding', ['xvfb-run', '-a', 'wine', out / 'fixture.exe'])
    run('stop-prefix', ['wineserver', '-k'], 10)
    if sources != {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sources}:
        raise ValueError('ABI sources changed during execution')
    report = {'schema': 1, 'success': True, 'sources': sources, 'sources_stable': True,
              'callee_pop_bytes': [0, 4, 8, 12, 16, 20, 24], 'register_flags_x87_sse_last_error_preserved': True,
              'same_stack_tail_unwind': True, 'interior_no_return_slot_change': True,
              'invalid_host_phase_refused': True,
              'named_leave_export_checked': True,
              'frame_exhaustion_forwards_and_records_loss': True, 'original_renderer_executed': False}
    if claims: report['claims'] = claims['claims']
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report), flush=True)


if __name__ == '__main__': main()
