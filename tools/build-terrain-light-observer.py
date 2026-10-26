#!/usr/bin/env python3
"""Build the opt-in pointer-free PE32 terrain-light observer and guarded adapter."""
import argparse,hashlib,json,subprocess,importlib.util
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def build(out):
 out.mkdir(parents=True,exist_ok=False)
 spec=importlib.util.spec_from_file_location('shadow_build',ROOT/'tools/build-shadow-bridge.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
 imports={**mod.IMPORTS,'ReadFile':20,'Sleep':4,'GetTickCount':0}
 definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in imports.items()))
 subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',definition,'-l',out/'kernel32.lib','--kill-at'],check=True)
 subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',ROOT/'runtime/lighting/observer.c','-o',out/'observer.obj'],check=True)
 dll=out/'MnmLight.dll';subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0','/export:LightAnchor',f'/out:{dll}',out/'observer.obj',out/'kernel32.lib'],check=True)
 (out/'manifest.json').write_text(json.dumps(dict(sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),mode='Explicit original-active observation or bounded reply-validated replacement; original fallback on refusal/timeout.'),indent=2)+'\n');return dll
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);print(build(p.parse_args().output.resolve()))
