#!/usr/bin/env python3
"""Run selected pinned campaign bytecode with private dependencies, without a game."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
STUBS={'transition':0x557510,'config_reset':0x4ca460,'wizard_reset':0x593be0,
       'wizard_load':0x590c10,'controller_reset':0x5755d0,'realm_load':0x54e230,
       'script_reset':0x56cf00,'sample':0x56f000,'cue':0x4a39a0,'push':0x557040,'pop':0x557130}
def main():
    parent=ROOT/'working/tests/campaign-menu-contract';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,name,timeout=180):
        r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout)
        (out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        s=importlib.util.spec_from_file_location('campaign_export',ROOT/'tools/export-menu-support.py');export=importlib.util.module_from_spec(s);s.loader.exec_module(export)
        exe=ROOT/'working/game-nocd/Chaos.exe';data=export.pinned_image(exe)
        def at(va,n):offset=export.image_offset(data,va,n);return data[offset:offset+n]
        if struct.unpack('<6I',at(0x4a782c,24))!=(0x4a7611,0x4a75e8,0x4a76c4,0x4a76d5,0x4a76e6,0x4a7707):raise ValueError('Main dispatch mismatch')
        header='// Generated only from complete hash-pinned executable.\n'
        for name,va in STUBS.items():header+=f'static const unsigned {name}_address=0x{va:x};\nstatic const unsigned char {name}_bytes[]={{'+','.join(hex(b) for b in at(va,8))+'};\n'
        (out/'campaign_fixture_bytes.h').write_text(header)
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(out),ROOT/'tests/campaign-menu-contract-reference.cpp','-o',out/'reference'],'compile')
        report=json.loads(run([out/'reference',exe],'reference',20).stdout)
        if not report.get('success') or (report['main_cases'],report['occupancy_cases'],report['admission_cases'])!=(7,25,216) or (report['auxiliary_cases'],report['navigation_cases'])!=(7,14):raise ValueError('Incomplete campaign matrix')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=export.HASH:raise ValueError('Input changed')
        report.update(source_sha256=export.HASH,real_game_launched=False,live_validated=False,original_bytecode_unchanged=True,
            scope='Original Main New Game request/reset ordering, noncampaign controls, occupancy, selected region admission, auxiliary flags, Escape Mini ingress and pending return. Config/wizard/resource/script/audio dependencies privately stubbed; no full initialization, live return, Qt bridge or gameplay.',
            anchors={hex(va):at(va,8).hex() for va in [0x4a75c0,0x54e690,0x54eec0,0x54fd20,0x552310,0x5517a0,0x552210]},
            sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__),ROOT/'tests/campaign-menu-contract-reference.cpp',ROOT/'tests/sprite-binary-reference.cpp']},
            artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Campaign menu contract oracle passed',flush=True)
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')
if __name__=='__main__':main()
