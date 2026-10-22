#!/usr/bin/env python3
"""Compare six known original kind8 constructors and nondeleting lifetimes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    p.add_argument('--prefix-template',type=Path,required=True)
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        p.error('Require a new retained output under working/tests')
    out.mkdir(parents=True)
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bs={b['id']:b for b in r['behaviors']};builds={b['id']:b for b in r['builds']}
    ids=['RS.kind8-construction-lifetime']
    paths=set().union(*(required_sources(bs[i]) for i in ids))
    paths.update(['tools/coverage_claims.py','runtime/shadow/win32_min.h'])
    sources={n:sha(ROOT/n) for n in sorted(paths)}
    claims=[{'behavior':i,'contract_sha256':behavior_contract(bs[i],builds)} for i in ids]
    report={'schema':1,'success':False,'original_sha256':BUILD,'sources':sources,
            'claims':claims,'scope':__doc__,'original_instruction_bytes_patched':False,
            'game_allocator_or_deleting_destructor_executed':False,
            'live_replacement':False,'terrain_lookup_or_original_rng_seeding_recovered':False}
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(WINEDEBUG='-all',WINEPREFIX=str(out/'wineprefix'),
               MNM_ORIGINAL_IMAGE='Z:'+str(a.executable.resolve()).replace('/','\\'))

    def run(name,command,timeout=120):
        print(name,flush=True)
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=out,env=env,stdout=log,
                           stderr=subprocess.STDOUT,check=True,timeout=timeout)
    before=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        if sha(a.executable)!=BUILD:raise ValueError('Unsupported original input')
        raw=a.executable.read_bytes();pe=struct.unpack_from('<I',raw,60)[0]
        sections=pe+24+struct.unpack_from('<H',raw,pe+20)[0]
        anchors={}
        for va in (0x545dc0,0x546590,0x546230,0x548560,0x548650,0x548ac0):
            for i in range(struct.unpack_from('<H',raw,pe+6)[0]):
                _,rva,size,offset=struct.unpack_from('<4I',raw,sections+40*i+8)
                delta=va-0x400000-rva
                if 0<=delta and delta+16<=size:
                    anchors[hex(va)]=raw[offset+delta:offset+delta+16].hex();break
        if len(anchors)!=6:raise ValueError('Missing constructor entry bytes')
        report['original_entry_anchors']=anchors
        run('prefix',['cp','-a','--reflink=auto',a.prefix_template.resolve(),out/'wineprefix'])
        imports={'GetEnvironmentVariableA':12,'CreateFileA':28,'GetFileSize':8,
                 'ReadFile':20,'CloseHandle':4,'GetProcessHeap':0,'HeapAlloc':12,
                 'HeapFree':12,'VirtualAlloc':16,'WriteFile':20,'ExitProcess':4}
        imports.update(GetModuleHandleA=4,VirtualProtect=16)
        definition=out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        run('original-build',['clang','--target=i686-pc-windows-msvc','-O2',
            '-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx',
            '-Wall','-Wextra','-Werror','-MD','-MF',out/'probe.d','-c',
            ROOT/'tests/kind8-construction-reference.c','-o',out/'probe.obj'])
        closure=[]
        for name in (out/'probe.d').read_text().replace('\\\n',' ').split()[1:]:
            path=Path(name)
            if path.is_file() and path.resolve().is_relative_to(ROOT):
                name=path.resolve().relative_to(ROOT).as_posix()
                if name not in sources:raise ValueError('Undeclared compiler input: '+name)
                closure.append(name)
        report['compiled_original_sources']=closure
        pad=out/'private-image-pad.S'
        pad.write_text('.text\n.globl _original_fixture_pad\n_original_fixture_pad:\n.space 0x400000\n')
        run('original-pad',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj'])
        run('original-link',['lld-link','/machine:x86','/entry:start','/subsystem:console',
            '/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0',
            '/include:_original_fixture_pad','/out:'+str(out/'probe.exe'),out/'pad.obj',
            out/'probe.obj',out/'kernel32.lib'])
        run('original-execute',['wine',out/'probe.exe'])
        run('native-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',
                           ROOT/'tests/kind8-construction-test.cpp','-o',out/'native'])
        run('native-execute',[out/'native',out/'native.bin'])
        original=(out/'kind8-construction.bin').read_bytes();native=(out/'native.bin').read_bytes()
        if len(original)!=2592*304 or len(native)!=len(original):
            raise ValueError('Incomplete known-constructor matrix')
        if original!=native:
            first=next(i for i,(o,n) in enumerate(zip(original,native)) if o!=n)
            row=(first//304)*304
            (out/'mismatch.json').write_text(json.dumps({'case':first//304,
                'byte_offset_in_record':first%304,
                'original_words':list(struct.unpack('<76I',original[row:row+304])),
                'native_words':list(struct.unpack('<76I',native[row:row+304]))},indent=2)+'\n')
            raise ValueError('Original construction/destruction differs from native model')
        if sha(a.executable)!=BUILD:raise ValueError('Original input changed')
        report.update(success=True,cases=2592,known_classes=6,
                      object_bytes_compared=2592*128*2,atomic_native_refusals=5,
                      whole_objects_match=True,nondeleting_virtual_destruction_match=True,
                      binary_sha256={'original':sha(out/'probe.exe'),'native':sha(out/'native')},
                      original_result_sha256=sha(out/'kind8-construction.bin'),
                      native_result_sha256=sha(out/'native.bin'))
    except (OSError,ValueError,subprocess.SubprocessError) as e:report['error']=str(e)
    finally:
        if (out/'wineprefix').exists():
            subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
        if before:
            try:
                run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
                report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as e:report.update(success=False,preservation_error=str(e))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
