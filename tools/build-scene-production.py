#!/usr/bin/env python3
"""Build a separate opt-in PE32 scene-production experiment DLL, without game input."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]

def build(output,selftest=False):
    output.mkdir(parents=True,exist_ok=True)
    spec=importlib.util.spec_from_file_location('scene_imports',ROOT/'tools/build-menu-observer.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    imports=dict(module.IMPORTS,QueryPerformanceCounter=4,QueryPerformanceFrequency=4)
    definition=output/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(output/'kernel32.lib'),'--kill-at'],check=True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
           '-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    if selftest:flags.append('-DMNM_SCENE_PRODUCTION_SELFTEST')
    sources=[ROOT/'runtime/scene-production'/n for n in ('observer.c','entry.S')]
    for p in sources:subprocess.run([*flags,'-c',str(p),'-o',str(output/(p.name+'.obj'))],check=True)
    dll=output/'MnmSceneProduction.dll';exports=['/export:SceneProductionAnchor']
    if selftest:exports.extend(['/export:SceneProductionInstallForTest=_SceneProductionInstallForTest@4','/export:SceneProductionStatusForTest=_SceneProductionStatusForTest@4','/export:SceneProductionBudgetArmForTest=_SceneProductionBudgetArmForTest@4','/export:SceneProductionBudgetMetricsForTest=_SceneProductionBudgetMetricsForTest@0','/export:SceneProductionRefusedTimingForTest=_SceneProductionRefusedTimingForTest@0','/export:SceneProductionNextForTest=_SceneProductionNextForTest@0'])
    subprocess.run(['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0',
                    f'/out:{dll}',*exports,*[str(output/(p.name+'.obj')) for p in sources],str(output/'kernel32.lib')],check=True)
    sources += [ROOT/'runtime/scene-production/state_audit.h',ROOT/'runtime/scene-production/draw_budget.h',ROOT/'runtime/shadow/win32_min.h',ROOT/'tools/build-menu-observer.py',Path(__file__).resolve()]
    (output/'manifest.json').write_text(json.dumps(dict(sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),selftest=selftest,
        sources={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},
        scope='Bounded original scene-production repetition/state audit; no pacing or simulation implementation change.'),indent=2)+'\n')
    return dll

if __name__=='__main__':print(build(ROOT/'working/build/scene-production'))
