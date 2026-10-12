#!/usr/bin/env python3
"""Execute bounded original Save/Load controls in a private, isolated PE32 fixture."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FUNCTIONS = {'load_button': (0x4aa800, 0x4aa84d), 'load_tick': (0x4aa5d0, 0x4aa6b9),
             'save_button': (0x4ab750, 0x4ab865), 'save_tick': (0x4abd00, 0x4abf16),
             'edit_set': (0x4cd0e0, 0x4cd10f), 'list_select': (0x4d1060, 0x4d113d),
             'list_get': (0x4d1df0, 0x4d1e4d)}

def main():
    parent = ROOT/'working/tests/save-load-menu-contract'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    def run(args, name, **kwargs):
        r = subprocess.run(list(map(str,args)), capture_output=True, text=True, timeout=60, **kwargs)
        (out/(name+'.log')).write_text(r.stdout+r.stderr); r.check_returncode(); return r
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        spec = importlib.util.spec_from_file_location('menu_export',ROOT/'tools/export-menu-support.py')
        exporter = importlib.util.module_from_spec(spec); spec.loader.exec_module(exporter)
        executable = ROOT/'original/Arcane_Nocd/Chaos.exe'; data = exporter.pinned_image(executable)
        header = '/* Generated from whole-file hash-pinned original; private mapping only. */\n'
        for name,(start,end) in FUNCTIONS.items():
            at=exporter.image_offset(data,start,end-start)
            if name in ('load_button','save_button','edit_set','list_select') and data[at+end-start-3:at+end-start]!=bytes.fromhex('c20400'):raise ValueError('Incomplete action return instruction: '+name)
            header+=f'static const unsigned {name}_address=0x{start:x};\nstatic const unsigned char {name}_bytes[]={{'+','.join(hex(v) for v in data[at:at+end-start])+'};\n'
        (out/'save_load_fixture_bytes.h').write_text(header)
        imports={'VirtualProtect':16,'WriteFile':20,'GetStdHandle':4,'ExitProcess':4}
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
        run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',definition,'-l',out/'kernel32.lib','--kill-at'],'imports')
        run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I'+str(out),'-c',ROOT/'tests/save-load-menu-contract.c','-o',out/'fixture.obj'],'compile')
        run(['clang','--target=i686-pc-windows-msvc','-c',ROOT/'runtime/menu/contract_selftest.S','-o',out/'abi.obj'],'compile-abi')
        run(['lld-link','/safeseh:no','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/dynamicbase:no','/nodefaultlib','/timestamp:0','/out:'+str(out/'fixture.exe'),out/'fixture.obj',out/'abi.obj',out/'kernel32.lib'],'link')
        env=dict(os.environ,WINEPREFIX=str(ROOT/'working/tests/save-load-contract-wine'),WINEDEBUG='-all')
        result=json.loads(run(['wine',out/'fixture.exe'],'fixture',env=env).stdout)
        if not result.get('success') or result['cases']!=13: raise ValueError('Incomplete callback fixture')
        if hashlib.sha256(executable.read_bytes()).hexdigest()!=exporter.HASH:raise ValueError('Input changed')
        result.update(source_sha256=exporter.HASH,input_unchanged=True,
            scope='Original Load pending/Cancel/caller flags, Save Cancel/success/overwrite admission/overwrite modal, edit setter, list toggle/reset/get. Serializers, fade and modal resource helpers privately stubbed; no actual save I/O, live menu ownership, failure rollback or replacement.',
            sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in ['tools/test-save-load-menu-contract.py','tests/save-load-menu-contract.c','runtime/shadow/win32_min.h','runtime/menu/contract_selftest.S']},
            anchors={hex(a):data[exporter.image_offset(data,a,8):exporter.image_offset(data,a,8)+8].hex() for a,b in FUNCTIONS.values()})
        (out/'report.json').write_text(json.dumps(result,indent=2)+'\n');print('Save/Load original callback fixture passed')
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')

if __name__=='__main__':main()
