#!/usr/bin/env python3
"""Automated offline Preferences oracle; no game launch, device or settings writes."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]
STUBS = {'music_set':0x482190, 'sound_set':0x56fd30,
         'music_get':0x4818b0, 'sound_get':0x56fd10,
         'start_music':0x4819b0, 'sample':0x56f000,
         'frame_set':0x4f7970, 'close_screen':0x557510,
         'formatting':0x59c1fd, 'decimal':0x59de2f}

def main():
    parent=ROOT/'working/tests/preferences-contract';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,name,timeout=120):
        result=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout)
        (out/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode();return result
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        spec=importlib.util.spec_from_file_location('export',ROOT/'tools/export-menu-support.py');export=importlib.util.module_from_spec(spec);spec.loader.exec_module(export)
        exe=ROOT/'working/game-nocd/Chaos.exe';data=export.pinned_image(exe)
        def at(va,n):return data[export.image_offset(data,va,n):export.image_offset(data,va,n)+n]
        table=list(struct.unpack('<12I',at(0x5c648c,48)))
        expected=[0x4a8af0,0x4a8b60,0x4a8be0,0x4a8bd0,0x5595d0,0x559390,0x4a9b00,0x4a8bf0,0x4a8d00,0x4a8e40,0x4a9670,0x475c40]
        if table!=expected:raise ValueError('Preferences vtable mismatch')
        if list(struct.unpack('<12I',at(0x5c63a4,48)))!=[0,0,1,1,2,2,2,3,3,3,4,4]:raise ValueError('Radio groups mismatch')
        if at(0x4a85b1,13)!=bytes.fromhex('c7068c645c00c746040a000000'):raise ValueError('Preferences constructor identity mismatch')
        header='// Generated from complete hash-pinned input, checked before private stubs.\n'
        for name,address in STUBS.items():
            header+=f'static const unsigned {name}_address=0x{address:x};\nstatic const unsigned char {name}_bytes[]={{'+','.join(hex(b) for b in at(address,8))+'};\n'
        (out/'preferences_fixture_bytes.h').write_text(header)
        helper=out/'reference'
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-I'+str(out),ROOT/'tests/preferences-contract-reference.cpp','-o',helper],'compile')
        report=json.loads(run([helper,exe],'reference',20).stdout)
        if not report.get('success') or report['ok_cases']!=216:raise ValueError('Incomplete callback matrix')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=export.HASH:raise ValueError('Input changed')
        report.update(source_sha256=export.HASH,real_game_launched=False,live_validated=False,
                      callback_bytecode_unchanged=True,persistence_api_stubbed=True,devices_stubbed=True,
                      scope='Original Preferences enter, slider, OK/Cancel and writer with private dependency stubs; no real display/audio/filesystem or caller round trip')
        report['artifacts']={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()}
        report['source_files']={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__),ROOT/'tests/preferences-contract-reference.cpp',ROOT/'tests/sprite-binary-reference.cpp']}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Preferences contract oracle passed',flush=True)
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')
if __name__=='__main__':main()
