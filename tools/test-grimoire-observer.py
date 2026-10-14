#!/usr/bin/env python3
"""Synthetic guard/forwarding/LastError/dedup/bounded-log validation for book probe."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    result = importlib.util.module_from_spec(spec); spec.loader.exec_module(result); return result


def main():
    parent = ROOT / 'working/tests/grimoire-observer'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
    try:
        exporter = module('book_probe_image', 'tools/export-menu-support.py')
        exe = ROOT / 'original/Arcane_Nocd/Chaos.exe'; data = exporter.pinned_image(exe)
        at = exporter.image_offset(data, 0x4c7890, 8); prefix = data[at:at+8]
        (out / 'grimoire_observer_bytes.h').write_text('static const unsigned char book_tick_prefix[]={' + ','.join(hex(b) for b in prefix) + '};\n')
        builder = module('book_probe_build', 'tools/build-menu-observer.py')
        builder.build(out, selftest=True, fixture=False)
        imports = dict(builder.IMPORTS, GetStdHandle=4)
        definition = out / 'fixture-kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
        subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',definition,'-l',out/'fixture-kernel32.lib','--kill-at'],check=True)
        subprocess.run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-I'+str(out), '-c', ROOT / 'tests/grimoire-observer-reference.c', '-o', out / 'reference.obj'], check=True)
        subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/base:0x400000', '/dynamicbase:no', '/nodefaultlib', '/timestamp:0', '/out:'+str(out / 'reference.exe'), out / 'reference.obj', out / 'fixture-kernel32.lib', out / 'user32.lib'], check=True)
        env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(WINEPREFIX=str(ROOT / 'working/tests/save-load-contract-wine'), WINEDEBUG='-all', MNM_MENU_GRIMOIRE_OBSERVE='Z:'+str(out / 'grimoire.bin').replace('/', '\\'))
        result = subprocess.run(['wine', out / 'reference.exe'], env=env, capture_output=True, text=True, timeout=90)
        (out / 'wine.log').write_text(result.stdout+result.stderr); result.check_returncode()
        report = json.loads(result.stdout)
        rows = module('book_probe_decode', 'tools/test-live-grimoire-observation.py').decode((out / 'grimoire.bin').read_bytes())
        if not report.get('success') or report['bounded_records'] != 256 or len(rows) != 256:
            raise ValueError('Incomplete probe fixture')
        if rows[0]['owner'] != 0x6e0088 or rows[0]['parent'] != 0x6578c0 or rows[3]['owner'] != 0x6578c0 or rows[3]['depth'] != 4:
            raise ValueError('Incorrect serialized synthetic owner/return state')
        if rows[0]['callback_method'] != 0x4c77c0 or rows[0]['callback_receiver'] != 0x6e0088:
            raise ValueError('Incorrect serialized callback identity')
        if hashlib.sha256(exe.read_bytes()).hexdigest() != exporter.HASH: raise ValueError('Original input changed')
        paths = ['tools/test-grimoire-observer.py', 'tools/test-live-grimoire-observation.py', 'tests/grimoire-observer-reference.c', 'tools/build-menu-observer.py', 'tools/export-menu-support.py', 'runtime/shadow/win32_min.h']
        paths += [p.relative_to(ROOT).as_posix() for p in (ROOT / 'runtime/menu').iterdir() if p.is_file()]
        paths += [p.relative_to(ROOT).as_posix() for p in (ROOT / 'protocols/include/mnm').glob('menu_v*.h')]
        report.update(source_sha256=exporter.HASH, input_unchanged=True, real_game_launched=False,
                      scope='Synthetic original tick target and packed state: installer prefix/slot guards, result/LastError, unchanged-state dedup despite changing LastError, receiver rejection, owner/depth return serialization and256-record bound. Actual original tick/render/entry and game behavior are not executed.',
                      sources={p:hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sorted(set(paths))})
        (out / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print('Grimoire probe synthetic forwarding, guards and bounded records passed')
    finally:
        subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)


if __name__ == '__main__':
    main()
