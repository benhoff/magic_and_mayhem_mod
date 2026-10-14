#!/usr/bin/env python3
"""Execute original multiplayer form dispatch/setters and guarded native transactions in private PE32."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
RANGES = {"lobby": (0x4b7bf0,0x4b7c1f)}


def main():
    parent = ROOT / 'working/tests/multiplayer-lobby-contract'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)

    def run(args, name, **kwargs):
        r = subprocess.run(list(map(str, args)), capture_output=True, text=True, timeout=90, **kwargs)
        (out / (name + '.log')).write_text(r.stdout + r.stderr); r.check_returncode(); return r

    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        spec_build=importlib.util.spec_from_file_location('mp_build',ROOT/'tools/build-menu-observer.py');module_builder=importlib.util.module_from_spec(spec_build);spec_build.loader.exec_module(module_builder)
        module_builder.build(out,selftest=True,fixture=False)
        spec = importlib.util.spec_from_file_location('mp_image', ROOT / 'tools/export-menu-support.py')
        exporter = importlib.util.module_from_spec(spec); spec.loader.exec_module(exporter)
        executable = ROOT / 'original/Arcane_Nocd/Chaos.exe'; data = exporter.pinned_image(executable)
        blocks = {}; anchors = {}
        for name,(start,end) in RANGES.items():
            at=exporter.image_offset(data,start,end-start);raw=data[at:at+end-start]
            if raw[-3:]!=bytes.fromhex('c20800' if name=='radio' else 'c20400'):raise ValueError('Incomplete original RET immediate: '+name)
            blocks[name]=raw;anchors[hex(start)]=raw[:16].hex()
        (out/'multiplayer_fixture_bytes.h').write_text(''.join('static const unsigned char '+name+'_bytes[]={'+','.join(hex(b) for b in raw)+'};\n' for name,raw in blocks.items()))
        imports = dict(module_builder.IMPORTS, GetStdHandle=4)
        definition = out / 'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{n}@{s}\n' for n,s in imports.items()))
        run(['llvm-dlltool', '-m', 'i386', '-D', 'KERNEL32.dll', '-d', definition, '-l', out / 'kernel32.lib', '--kill-at'], 'imports')
        run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-I'+str(out), '-c', ROOT / 'tests/multiplayer-lobby-contract.c', '-o', out / 'fixture.obj'], 'compile')
        run(['clang', '--target=i686-pc-windows-msvc', '-c', ROOT / 'runtime/menu/contract_selftest.S', '-o', out / 'abi.obj'], 'compile-abi')
        run(['lld-link', '/safeseh:no', '/machine:x86', '/entry:start', '/subsystem:console', '/base:0x400000', '/dynamicbase:no', '/nodefaultlib', '/timestamp:0', '/out:'+str(out / 'fixture.exe'), out / 'fixture.obj', out / 'abi.obj', out / 'kernel32.lib',out/'user32.lib'], 'link')
        env = dict(os.environ, WINEPREFIX=str(ROOT / 'working/tests/save-load-contract-wine'), WINEDEBUG='-all')
        result = json.loads(run(['wine', out / 'fixture.exe'], 'fixture', env=env).stdout)
        if not result.get('success') or result['cases'] != 7 or result['guard_rejections']!=35: raise ValueError('Incomplete multiplayer form fixture')
        unchanged = hashlib.sha256(executable.read_bytes()).hexdigest() == exporter.HASH
        if not unchanged: raise ValueError('Original input changed')
        sources = ['tools/test-multiplayer-lobby-contract.py', 'tests/multiplayer-lobby-contract.c', 'tools/export-menu-support.py', 'runtime/shadow/win32_min.h', 'runtime/menu/contract_selftest.S','tools/build-menu-observer.py'] + [p.relative_to(ROOT).as_posix() for p in (ROOT/'runtime/menu').iterdir() if p.is_file()] + [p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols/include/mnm').glob('menu_v*.h')]
        result.update(source_sha256=exporter.HASH, input_unchanged=True, ranges=RANGES, anchors=anchors,
                      scope='Original full-object lobby Cancel receiver to primary-menu fade, Quick destination and interface disconnect, unknown callback no-op and ABI; native exact host/parent/context/service/callback/one-player/transition guards. Fade/disconnect stubbed; live host networking and lobby work not run.',
                      sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sources})
        (out / 'report.json').write_text(json.dumps(result, indent=2) + '\n')
        print('7 original lobby callback cases and 35 adapter guard rejections passed')
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')


if __name__ == '__main__':
    main()
