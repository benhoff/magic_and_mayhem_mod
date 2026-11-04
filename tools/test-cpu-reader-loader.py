#!/usr/bin/env python3
"""Observe a forced PE32 loader relocation with no original game execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
from coverage_evidence import write_report
from cpu_reader_journal import pe_identity,loader_pe_identity
from native_prefix_template import borrow

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build',type=Path,required=True)
    p.add_argument('--run',type=Path,required=True)
    p.add_argument('--report',required=True)
    a=p.parse_args();build=a.build.resolve();run=a.run.resolve()
    if not run.is_relative_to(ROOT/'working') or run.exists():p.error('Fresh owned working run required')
    if not a.report.startswith('research/runtime/') or (ROOT/a.report).exists():p.error('New evidence identity required')
    build.mkdir(parents=True,exist_ok=True);run.mkdir(parents=True)
    (run/'.retain').write_text('Retain synthetic loader evidence and prefix return until verified handoff.\n')
    names=['tools/test-cpu-reader-loader.py','tools/cpu_reader_journal.py','tests/cpu-reader-loader-fixture.c','tests/cpu-reader-loader-win32.c','runtime/shadow/win32_min.h','tools/native_prefix_template.py']
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names}
    q=dict(schema=1,kind='pe32_loader_header_control',success=False,original_game_executed=False,sources=sources,
           scope='Synthetic DLL relocated by the installed Wine loader. Compare all mapped header bytes with the pinned file, check a relocated data pointer and retire the private prefix. No original-game caller or historical mapped-code identity is inferred.')
    lease=None;env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(WINEDEBUG='-all',TMPDIR=str(build))
    try:
        imports={'VirtualAlloc':16,'LoadLibraryA':4,'GetProcAddress':8,'FreeLibrary':4,'VirtualFree':12,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=build/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
        commands=[['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',definition,'-l',build/'kernel32.lib','--kill-at']]
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
        commands += [flags+['-c',ROOT/'tests/cpu-reader-loader-fixture.c','-o',build/'fixture.obj'],
                     ['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/timestamp:0','/dynamicbase','/base:0x10000000','/export:HeaderProbe=_HeaderProbe@0',f'/out:{build/"HeaderFixture.dll"}',build/'fixture.obj'],
                     flags+['-c',ROOT/'tests/cpu-reader-loader-win32.c','-o',build/'driver.obj'],
                     ['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{build/"loader.exe"}',build/'driver.obj',build/'kernel32.lib']]
        q['commands']=[list(map(str,c)) for c in commands]
        with (run/'build.log').open('xb') as log:
            for command in commands:subprocess.run(list(map(str,command)),stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
        for name in ['HeaderFixture.dll','loader.exe']:shutil.copyfile(build/name,run/name)
        lease,q['prefix_pool']=borrow(ROOT/'working/fixtures/prefix-seed',run/'wineprefix',env)
        env['WINEPREFIX']=str(run/'wineprefix')
        q['wine_version']=subprocess.check_output(['wine','--version'],env=env,text=True).strip()
        with (run/'wine.log').open('xb') as log:
            result=subprocess.run(['wine',str(run/'loader.exe')],cwd=run,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=45)
        q['exit_code']=result.returncode
        if result.returncode:raise ValueError('Synthetic loader failed: '+str(result.returncode))
        mapped=(run/'mapped-header.bin').read_bytes();header=struct.unpack_from('<8I',mapped)
        assert mapped[:8]==b'MNMLDR01' and header[2]==1 and header[7]==0 and len(mapped)==32+header[5]
        raw=(run/'HeaderFixture.dll').read_bytes();pe=struct.unpack_from('<I',raw,60)[0];offset=pe+24+28
        assert header[4]==pe and header[3]!=header[6]==0x10000000 and pe_identity(raw)[3]==header[5]
        adjusted=bytearray(raw[:header[5]]);struct.pack_into('<I',adjusted,offset,header[3])
        assert mapped[32:]==adjusted,'Mapped header has changes beyond ImageBase'
        assert loader_pe_identity(raw,header[3])==pe_identity(mapped[32:])
        q.update(success=True,file_identity=pe_identity(raw),mapped_identity=pe_identity(mapped[32:]),allocation_base=hex(header[3]),file_image_base=hex(header[6]),image_base_offset=offset,
                 changed_header_offsets=[i for i,(x,y) in enumerate(zip(raw,mapped[32:])) if x!=y],only_image_base_changed=True,relocated_pointer_value_verified=True)
    except Exception as error:q['error']=str(error)
    finally:
        if lease is not None:
            subprocess.run(['wineserver','-k'],env=env,check=True,timeout=10)
            subprocess.run(['wineserver','-w'],env=env,check=True,timeout=20)
            lease.close();q['prefix_pool']['returned']=True
        q['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());q['success'] &= q['sources_stable']
        q['artifacts']=[]
        for path in sorted(run.iterdir()):
            if not path.is_file() or path.is_symlink():continue
            raw=path.read_bytes();name=str(Path(a.report).with_suffix(''))+'-'+path.name.replace('.','-')+'.bin'
            q['artifacts'].append(dict(path=path.name,record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw)))
        sha=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
        print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=sha,only_image_base_changed=q.get('only_image_base_changed'),wine_version=q.get('wine_version'))),flush=True)
    return 0 if q['success'] else 1


if __name__=='__main__':raise SystemExit(main())
