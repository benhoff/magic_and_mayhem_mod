#!/usr/bin/env python3
"""Execute pinned windowed copy wrappers after refusing the obsolete DC hook."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SITES=(0x58bfe2,0x58c1d6,0x58c3f5,0x58c534,0x58c6f8,0x58c922,0x58cb17,0x58cc16,0x58cdd1,0x58cfa1)


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--wine-prefix',type=Path,required=True)
    p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    a=p.parse_args();declaration=json.loads(a.claims.read_text());prefix=a.wine_prefix.resolve()
    if not prefix.is_relative_to(ROOT/'working/tests') or not prefix.is_dir() or prefix.is_symlink():p.error('Use an owned stopped prefix')
    if any(sha(ROOT/path)!=h for path,h in declaration['sources'].items()):p.error('Prospective sources changed')
    parent=ROOT/'working/tests/dc-copy-retirement';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(success=False,sources=declaration['sources'],claims=declaration['claims'],live_replacement=False,
                scope='Thirty actual pinned windowed copy-wrapper calls with supplied COM/Win32 endpoints; ten original ClientToScreen CALL instructions preserved, default/off DLL load succeeds, obsolete takeover load refuses, translated rectangles and stack cleanup checked. No driver/gameplay pixels or universal import census claim.')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINEPREFIX','WINEDLLOVERRIDES'))}
    env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=',LIBGL_ALWAYS_SOFTWARE='1')
    verified=False
    def run(label,command,cwd=None):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(v) for v in command],cwd=cwd or ROOT,env=env,stdout=log,stderr=log,check=True,timeout=180)
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
        run('admission-controls',['python3','-B',ROOT/'tests/test-pixel-route-requirements.py'])
        data=a.executable.read_bytes()
        if hashlib.sha256(data).hexdigest()!=BUILD:raise ValueError('Unsupported executable')
        inventory=json.loads((ROOT/'research/runtime/coverage/binary-nocd.json').read_text())
        imported=[i for i in inventory['imports'] if i['iat_va']==0x5c51fc]
        if len(imported)!=1 or imported[0]['name']!='ClientToScreen' or imported[0]['dll'].lower()!='user32.dll':raise ValueError('Wrong target API')
        if any(i.get('name')=='BitBlt' for i in inventory['imports']):raise ValueError('Re-review BitBlt imports')
        pe=struct.unpack_from('<I',data,60)[0];opt=pe+24
        base,size=struct.unpack_from('<I',data,opt+28)[0],struct.unpack_from('<I',data,opt+56)[0]
        if base!=0x400000 or not 0x300000<=size<=0x2000000:raise ValueError('Unsafe private PE extent')
        mapped=bytearray(size);count=struct.unpack_from('<H',data,pe+6)[0];headers=struct.unpack_from('<H',data,pe+20)[0]
        for i in range(count):
            at=opt+headers+i*40;rva,n,raw=struct.unpack_from('<III',data,at+12)
            if raw+n>len(data) or rva+n>size:raise ValueError('Unsafe private PE section')
            mapped[rva:rva+n]=data[raw:raw+n]
        for site in SITES:
            if mapped[site-base:site-base+6]!=bytes.fromhex('ff15fc515c00'):raise ValueError('Original CALL differs')
        (out/'image.bin').write_bytes(mapped);(out/'image-size.bin').write_bytes(struct.pack('<I',size))
        spec=importlib.util.spec_from_file_location('retired_dc_build',ROOT/'tools/build-dc-copy.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build();shutil.copyfile(dll,out/'MnmDcCopy.dll')
        imports_text=subprocess.check_output(['objdump','-p',str(dll)],text=True)
        if any(name in imports_text for name in ('VirtualProtect','FlushInstructionCache','VirtualAlloc','GetProcAddress')):
            raise ValueError('Retired production DLL still imports patch machinery')
        imports={'LoadLibraryA':4,'FreeLibrary':4,'SetEnvironmentVariableA':8,'VirtualProtect':16,'SetFilePointer':16,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        (out/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',out/'kernel32.def','-l',out/'kernel32.lib'])
        run('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',ROOT/'tests/dc-copy-retirement-probe.c','-o',out/'probe.obj'])
        run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',out/'kernel32.lib'])
        run('wrappers',['xvfb-run','-a','wine',out/'probe.exe'],out)
        raw=(out/'retirement.bin').read_bytes()
        if len(raw)!=30*64:raise ValueError('Incomplete wrapper report')
        rows=[struct.unpack_from('<16I',raw,i*64) for i in range(30)]
        for i,row in enumerate(rows):
            right,bottom=(9,41) if i%10<2 else (6,37) if i%10==7 else (4,37)
            # 58c6a0 translates/clamps the caller's RECT, then returns early
            # for this empty intersection. Other selected wrappers forward it.
            cropped=i%10==4
            expected_rect=(0,0,0,0) if cropped else (0xffffffee,35,(right-19)&0xffffffff,bottom)
            source_after=(0,34,0xfffffff1,6) if cropped else (1,1,4,3)
            if row[0]!=i//10 or row[2:7]!=(1,int(not cropped),0,0,0) or row[7:11]!=expected_rect or row[11]!=1 or row[12:16]!=source_after:
                raise ValueError('Original window translation/stack differs: '+str(row))
        report.update(success=True,cases=30,client_to_screen_calls=30,stack_mismatches=0,sites=[hex(s) for s in SITES],
                      target_import=imported[0],bitblt_imports=0,original_executable_sha256=BUILD,
                      results_sha256=sha(out/'retirement.bin'),dll_sha256=sha(out/'MnmDcCopy.dll'),
                      binary_inventory_sha256=sha(ROOT/'research/runtime/coverage/binary-nocd.json'))
    except (OSError,ValueError,subprocess.SubprocessError) as error:report.update(success=False,error=str(error))
    finally:
        subprocess.run(['wineserver','-k'],env=env,timeout=15,check=False)
        subprocess.run(['wineserver','-w'],env=env,timeout=15,check=False)
        if verified:run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['sources_stable']=all(sha(ROOT/path)==h for path,h in declaration['sources'].items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
