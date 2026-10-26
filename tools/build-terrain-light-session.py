#!/usr/bin/env python3
"""Build source-pinned PE32 lighting adapter and persistent native64 worker."""
import argparse,hashlib,importlib.util,json,shlex,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MODELS=['terrain_lighting.cpp','terrain_creature_lighting.cpp','terrain_static_lighting.cpp','terrain_lighting_cycle.cpp']
def build_session_bridge(out):
 out.mkdir(parents=True,exist_ok=False)
 imported={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in ['tools/build-shadow-bridge.py','tools/build-terrain-light-session.py']}
 spec=importlib.util.spec_from_file_location('session_kernel_build',ROOT/'tools/build-shadow-bridge.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
 imports={**mod.IMPORTS,'ReadFile':20,'Sleep':4,'GetTickCount':0,'GetFileSize':8,'CreateFileMappingA':24,'MapViewOfFile':20,'UnmapViewOfFile':4}
 definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in imports.items()))
 subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',definition,'-l',out/'kernel32.lib','--kill-at'],check=True)
 flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
 listing=subprocess.check_output([*flags,'-MM',ROOT/'runtime/lighting/session_observer.c'],text=True).replace('\\\n',' ')
 sources=dict(imported)
 for n in shlex.split(listing.split(':',1)[1]):
  p=Path(n).resolve()
  if not p.is_relative_to(ROOT):raise ValueError('Unexpected lighting compiler input')
  sources[p.relative_to(ROOT).as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
 subprocess.run([*flags,'-MMD','-MF',out/'observer.d','-c',ROOT/'runtime/lighting/session_observer.c','-o',out/'observer.obj'],check=True)
 compiled=set()
 for n in shlex.split((out/'observer.d').read_text().replace('\\\n',' ').split(':',1)[1]):
  p=Path(n).resolve()
  if not p.is_relative_to(ROOT):raise ValueError('Unexpected lighting compiler input')
  compiled.add(p.relative_to(ROOT).as_posix())
 if compiled!=set(sources)-set(imported) or any(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()!=h for n,h in sources.items()):raise ValueError('Lighting DLL sources changed during compilation')
 dll=out/'MnmLight.dll';subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0','/export:LightAnchor',f'/out:{dll}',out/'observer.obj',out/'kernel32.lib'],check=True)
 (out/'manifest.json').write_text(json.dumps(dict(sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),sources=sources),indent=2)+'\n');return dll
def build(out):
 out.mkdir(parents=True,exist_ok=False)
 dll=build_session_bridge(out/'bridge')
 worker=out/'mnm-terrain-light-session';units=[ROOT/'tools/terrain-light-session-worker.cpp',*[ROOT/'reconstruction/rendering'/n for n in MODELS]];flags=['g++','-std=c++17','-O2','-DMNM_LIGHT_SESSION','-pthread']
 sources=set(json.loads((dll.parent/'manifest.json').read_text())['sources'])
 for unit in units:
  text=subprocess.check_output([*flags,'-MM',unit],text=True).replace('\\\n',' ')
  for name in shlex.split(text.split(':',1)[1]):
   p=Path(name).resolve()
   if p.is_relative_to(ROOT):sources.add(p.relative_to(ROOT).as_posix())
 sources.update(['tools/build-terrain-light-session.py','tools/terrain_light_session.py'])
 before={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(sources)}
 subprocess.run([*flags,*units,'-o',worker],check=True)
 if any(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=h for p,h in before.items()):raise ValueError('Lighting build sources changed')
 manifest=dict(schema=1,sources=before,dll_sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),worker_sha256=hashlib.sha256(worker.read_bytes()).hexdigest(),command=[str(x) for x in [*flags,*units,'-o',worker]])
 (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');return dll,worker,manifest
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args();print(build(a.output.resolve())[2])
