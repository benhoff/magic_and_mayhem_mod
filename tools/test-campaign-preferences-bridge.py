#!/usr/bin/env python3
"""Check campaign Mini Preferences entry/preview/Cancel/OK in isolated PE32."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
REPO = Path(__file__).resolve().parents[1]
SOURCES = ['protocols/include/mnm/menu_v12.h', 'runtime/menu/preferences.h',
           'runtime/menu/channel.h', 'runtime/menu/mini.h', 'runtime/menu/observer.c',
           'tests/campaign-preferences-reference.c', 'tests/campaign-preferences-selftest.h',
           'tools/build-menu-observer.py', 'tools/test-campaign-preferences-bridge.py']

def module(name, path):
    spec = importlib.util.spec_from_file_location(name, REPO / path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result

def main():
    parent = REPO / 'working/tests/campaign-preferences-bridge'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(root, flush=True)
    subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        exporter = module('menu_export', 'tools/export-menu-support.py')
        executable = REPO / 'working/game-nocd/Chaos.exe'
        data = exporter.pinned_image(executable)
        header = '/* Hash-pinned original bytes; mapped only in private PE32. */\n'
        anchors = {}
        for name, start, end in [('mini_button', 0x4b23f0, 0x4b24f0),
                                  ('slider_set', 0x4cdf40, 0x4cdfe4),
                                  ('pref_button_bytes', 0x4a9840, 0x4a9b00),
                                  ('pref_slider_bytes', 0x4a9700, 0x4a97d0),
                                  ('pref_group_bytes', 0x4ce730, 0x4ce798)]:
            at = exporter.image_offset(data, start, end-start)
            header += 'static const unsigned char '+name+'[]={'+','.join(hex(v) for v in data[at:at+end-start])+'};\n'
            anchors[hex(start)] = data[at:at+8].hex()
        (root / 'menu_fixture_bytes.h').write_text(header)
        module('menu_build', 'tools/build-menu-observer.py').build(root, True, fixture=False)
        flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding',
                 '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx',
                 '-Wall', '-Wextra', '-Werror', '-I', str(root)]
        subprocess.run(flags+['-c', str(REPO / 'tests/campaign-preferences-reference.c'), '-o', str(root / 'reference.obj')], check=True)
        subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console',
                        '/base:0x400000', '/nodefaultlib', '/timestamp:0',
                        f'/out:{root/"selftest.exe"}', str(root / 'reference.obj'),
                        str(root / 'kernel32.lib'), str(root / 'user32.lib')], check=True)
        size = 106496
        channel = bytearray(size)
        channel[:16] = b'MNMMCM12'+struct.pack('<II', 12, size)
        struct.pack_into('<III', channel, 16, 2, 1, 1)
        (root / 'channel.bin').write_bytes(channel)
        env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(WINEPREFIX=str(root / 'wineprefix'), WINEDEBUG='-all',
                   WINEDLLOVERRIDES='winedbg.exe=d',
                   MNM_MENU_CHANNEL='Z:'+str(root / 'channel.bin').replace('/', '\\'))
        with (root / 'wine.log').open('w') as log:
            subprocess.run(['wine', str(root / 'selftest.exe')], env=env, cwd=root,
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=60)
        assert hashlib.sha256(executable.read_bytes()).hexdigest() == exporter.HASH
        report = dict(success=True, source_sha256=exporter.HASH, anchors=anchors,
                      input_unchanged=True, real_game_launched=False,
                      original_callbacks_executed=True,
                      scope='V12 original Mini local-2 entry and Preferences preview/Cancel/OK, exact parent/depth/stack/context guards, audio rollback and synthetic writer counts, Main caller and stale/invalid regression; original slider setter/group bytecode, audio/display/fade/writer dependencies synthetic; outer engine lifecycle and full installation not executed',
                      sources={p:hashlib.sha256((REPO/p).read_bytes()).hexdigest() for p in SOURCES})
        (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print('Campaign Preferences fixture passed', flush=True)
    finally:
        if (root / 'wineprefix').exists():
            subprocess.run(['wineserver', '-k'], env=dict(os.environ, WINEPREFIX=str(root / 'wineprefix')),
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=10, check=False)
        subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], check=True)

if __name__ == '__main__':
    main()
