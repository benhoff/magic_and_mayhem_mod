#!/usr/bin/env python3
"""Build the opt-in allocation repair against one immutable original image."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
from original_x86 import PE32

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
HOOKS=[(0x4e40d0,6),(0x58ad90,5),(0x58b3e0,5),(0x58acf0,5),(0x594508,6),(0x57dc20,10)]
IMPORTS={'GetModuleHandleA':4,'VirtualAlloc':16,'VirtualFree':12,'VirtualProtect':16,
 'FlushInstructionCache':12,'GetCurrentProcess':0,'GetCurrentThreadId':0,
 'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'GetLastError':0,'SetLastError':4,
 'GetEnvironmentVariableA':12,'ExitProcess':4,'SetFilePointer':16,'FlushFileBuffers':4}

def _build(output,selftest=False):
 output=Path(output).resolve();output.mkdir(parents=True,exist_ok=True)
 image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
 if image.sha256!=BUILD:raise ValueError('Unsupported original executable')
 witnesses=[dict(address=a,length=n,bytes=image.read(a,n).hex()) for a,n in HOOKS]
 (output/'alternate_owner_pinned.h').write_text('static const struct {unsigned address,length;unsigned char bytes[10];} owner_hooks[]={\n'+''.join('{'+hex(w['address'])+','+str(w['length'])+',{'+','.join(hex(b) for b in bytes.fromhex(w['bytes']))+'}},\n' for w in witnesses)+'};\n')
 definition=output/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(n+'@'+str(v)+'\n' for n,v in IMPORTS.items()))
 subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',output/'kernel32.lib'],check=True)
 flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I'+str(output)]
 if selftest:flags+=['-DMNM_ALT_OWNER_SELFTEST']
 compiled={}
 for name in ['owner.c','owner.S']:
  dep=output/(name+'.d');subprocess.run([*flags,'-MD','-MF',dep,'-c',ROOT/'runtime/alternate-canvas'/name,'-o',output/(name+'.obj')],check=True)
  for n in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
   p=Path(n).resolve()
   if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):compiled[p.relative_to(ROOT).as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
 dll=output/'MnmAlt.dll'
 subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/timestamp:0','/safeseh:no','/export:AlternateOwnerAnchor',*(['/export:AlternateOwnerInstallForTest=_AlternateOwnerInstallForTest@0','/export:AlternateOwnerSaturateLogForTest=_AlternateOwnerSaturateLogForTest@0'] if selftest else []),'/out:'+str(dll),output/'owner.c.obj',output/'owner.S.obj',output/'kernel32.lib'],check=True)
 (output/'manifest.json').write_text(json.dumps(dict(original_sha256=BUILD,dll_sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),selftest=selftest,hooks=witnesses,compiled_sources=compiled,policy='Authored guarded original alternate allocation repair; historical allocator not recovered.'),indent=2)+'\n')
 return dll

def build(output,selftest=False):
 subprocess.run([ROOT/'tools/original-manifest.sh','verify'],cwd=ROOT,check=True)
 try:return _build(output,selftest)
 finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],cwd=ROOT,check=True)

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',required=True,type=Path);p.add_argument('--selftest',action='store_true');a=p.parse_args();print(build(a.output,a.selftest))
