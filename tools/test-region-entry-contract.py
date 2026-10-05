#!/usr/bin/env python3
"""Execute selected original Region Entry callbacks in a private PE mapping."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
STUBS={'transition':0x557510,'admission':0x54eec0,'count':0x594230,'kind':0x4bfb10,'wizard':0x5908e0,'world':0x470950,'pop':0x557130}
def main():
    parent=ROOT/'working/tests/region-entry-contract';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(args,name,timeout=180):
        r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=timeout);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        s=importlib.util.spec_from_file_location('region_pe',ROOT/'tools/export-menu-support.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);exe=ROOT/'working/game-nocd/Chaos.exe';data=m.pinned_image(exe)
        def at(va,n):i=m.image_offset(data,va,n);return data[i:i+n]
        header='// Generated from complete pinned executable.\n'
        for name,va in STUBS.items():header+=f'static const unsigned {name}_address=0x{va:x};\nstatic const unsigned char {name}_bytes[]={{'+','.join(hex(b) for b in at(va,8))+'};\n'
        (out/'region_entry_fixture_bytes.h').write_text(header)
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(out),ROOT/'tests/region-entry-contract-reference.cpp','-o',out/'reference'],'compile')
        result=json.loads(run([out/'reference',exe],'reference',20).stdout)
        if not result.get('success') or (result['callback_cases'],result['deferred_cases'],result['return_cases'])!=(168,4,3):raise ValueError('Incomplete Region Entry matrix')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=m.HASH:raise ValueError('PE input changed')
        result.update(source_sha256=m.HASH,scope='Original action/caller/difficulty/Cancel/auxiliary mapping, deferred world request and pending Realm exit only; privately stubbed dependencies, no resource creation, campaign simulation or native/live equivalence',real_game_launched=False,anchors={hex(va):at(va,8).hex() for va in [0x4b3420,0x4b3880,0x552210]},sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__),ROOT/'tests/region-entry-contract-reference.cpp',ROOT/'tests/sprite-binary-reference.cpp']})
        (out/'report.json').write_text(json.dumps(result,indent=2)+'\n');print('Region Entry original contract passed',flush=True)
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')
if __name__=='__main__':main()
