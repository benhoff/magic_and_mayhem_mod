#!/usr/bin/env python3
"""Execute PE32 observation preservation and strict engine trace refusal controls."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
from engine_cadence import decode

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise ValueError('Changed fixture source claims')
    parent=ROOT/'working/tests/engine-cadence-controls';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all')
    report=dict(success=False,claims=declaration['claims'],sources=declaration['sources'])
    try:
        spec=importlib.util.spec_from_file_location('cadence_build',ROOT/'tools/build-engine-cadence.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        with (out/'build.log').open('x') as log:
            dll=module.build(out/'build')
            for name in ('engine-cadence-reference.c','engine-cadence-reference.S'):
                subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin',
                    '-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests'/name),'-o',str(out/(name+'.obj'))],stdout=log,stderr=subprocess.STDOUT,check=True)
            executable=out/'fixture.exe'
            subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/fixed','/dynamicbase:no','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{executable}',
                str(out/'engine-cadence-reference.c.obj'),str(out/'engine-cadence-reference.S.obj'),str(out/'build/entry.S.obj'),str(out/'build/kernel32.lib')],stdout=log,stderr=subprocess.STDOUT,check=True)
        with (out/'wine.log').open('x') as log:
            result=subprocess.run(['xvfb-run','-a','wine',str(executable)],env=env,cwd=out,stdout=log,stderr=subprocess.STDOUT,timeout=120)
        report.update(exit_code=result.returncode,binary_sha256=sha(executable),dll_sha256=sha(dll))
        if result.returncode:raise ValueError('PE32 thunk preservation failed')
        header=[0]*16;header[:2]=struct.unpack('<2I',b'MNMCAD01');header[2:11]=[1,96,1000,0,1,0,0,7,262144]
        row=[0]*24;row[:5]=[1,1,7,10,0]
        good=struct.pack('<16I',*header)+struct.pack('<24I',*row)
        path=out/'valid.bin';path.write_bytes(good);decode(path)
        rejected=0
        for at in (0,8,12,24,28,32,36,40,44,64,68,72):
            bad=bytearray(good);bad[at]^=255;path=out/f'bad-{at}.bin';path.write_bytes(bad)
            try:decode(path)
            except ValueError:rejected+=1
            else:raise ValueError('Malformed engine trace admitted')
        for i,bad in enumerate((good[:-1],good+b'X')):
            path=out/f'bad-size-{i}.bin';path.write_bytes(bad)
            try:decode(path)
            except ValueError:rejected+=1
            else:raise ValueError('Malformed engine trace length admitted')
        report.update(decoder_refusals=rejected,sources_unchanged=all(sha(ROOT/p)==h for p,h in report['sources'].items()),
            checks='Seven PE32 paths: all returned registers except physical ESP, flags,512B x87/SSE and LastError; ten independent original forwards; marker stores; volatile-ECX ANI snapshot;76 mutated guard bytes; bounded recorder overflow; strict decoder controls.')
        if not report['sources_unchanged']:raise ValueError('Fixture sources changed')
        report['success']=True
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if (out/'wineprefix').exists():subprocess.run(['wineserver','-k'],env=env,timeout=10,check=True)
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
