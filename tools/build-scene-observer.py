#!/usr/bin/env python3
"""Build the bounded PE32 display queue observer without a CRT."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def build(selftest=False):
    spec = importlib.util.spec_from_file_location('shadow_build', ROOT/'tools/build-shadow-bridge.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    output = ROOT/('working/build/scene-observer-selftest' if selftest else 'working/build/scene-observer')
    output.mkdir(parents=True, exist_ok=True)
    definition = output/'kernel32.def'
    imports={**module.IMPORTS,'CreateFileMappingA':24,'MapViewOfFile':20,'GetFileSize':8,'UnmapViewOfFile':4,'ReadFile':20,'Sleep':4,'GetTickCount':0,'AddVectoredExceptionHandler':8,'HeapCreate':12,'GetProcAddress':8,'GetCurrentThreadId':0,'InitializeCriticalSection':4,'EnterCriticalSection':4,'LeaveCriticalSection':4}
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(output/'kernel32.lib'),'--kill-at'],check=True)
    flags = ['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin',
             '-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    if selftest:
        flags += ['-DMNM_SCENE_SELFTEST']
    names=('observer.c','entry.S','world_trace.c','world_entry.S','world_stream.c','world_lifetime.c','canvas_startup.c','canvas_startup.S','startup_queue.c','kind8_observer.c','canvas_producers.c','canvas_producers.S','drawing_publication.S','glyph_live.c','glyph_leave.S','glyph_route.c')
    for name in names:
        subprocess.run([*flags,'-c',str(ROOT/'runtime/scene'/name),'-o',str(output/(name+'.obj'))],check=True)
    backend_names=('glyph_backend.c','glyph_backend_state.c','glyph_backend_x87.S')
    for name in backend_names:
        subprocess.run([*flags,'-c',str(ROOT/'reconstruction/rendering'/name),'-o',str(output/(name+'.obj'))],check=True)
    dll = output/'MnmScene.dll'
    exports = ['/export:SceneDrawingSourceBytesV1=_SceneDrawingSourceBytesV1@20','/export:SceneDrawingActionV1=_SceneDrawingActionV1@28','/export:ScenePacerObservationV1=_ScenePacerObservationV1@8','/export:SceneDrawingSourcePointV1=_SceneDrawingSourcePointV1@12','/export:SceneDrawingSourcePairV1=_SceneDrawingSourcePairV1@16']
    if selftest:
        exports += ['/export:SceneInstallForTest=_SceneInstallForTest@4','/export:GlyphAdmissionForTest=_GlyphAdmissionForTest@0']
    subprocess.run(['lld-link',*exports,'/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no',
                    '/timestamp:0',f'/out:{dll}',*[str(output/(name+'.obj')) for name in (*names,*backend_names)],str(output/'kernel32.lib')],check=True)
    paths = [*sorted((ROOT/'runtime/scene').glob('*.[chS]')),ROOT/'runtime/shadow/win32_min.h',ROOT/'protocols/include/mnm/drawing_source_point_v1.h',ROOT/'reconstruction/rendering/vertical_bar.h',
             ROOT/'protocols/include/mnm/scene_snapshot_v1.h',ROOT/'protocols/include/mnm/world_frame_v1.h',ROOT/'protocols/include/mnm/world_channel_v1.h',ROOT/'protocols/include/mnm/world_channel_v2.h',ROOT/'protocols/include/mnm/canvas_producers_v1.h',ROOT/'protocols/include/mnm/canvas_producers_v2.h',ROOT/'protocols/include/mnm/canvas_producers_v3.h',ROOT/'protocols/include/mnm/world_continuous_v3.h',ROOT/'protocols/include/mnm/world_continuous_v4.h',ROOT/'protocols/include/mnm/world_continuous_checksum.h',ROOT/'protocols/include/mnm/world_raster_batch_v3.h',ROOT/'runtime/render/pacer_yield.h',ROOT/'runtime/render/precise_clock.h',Path(__file__).resolve(),ROOT/'tools/build-shadow-bridge.py',*[ROOT/'reconstruction/rendering'/n for n in (*backend_names,'glyph_backend.h','glyph_backend_state.h')]]
    (output/'manifest.json').write_text(json.dumps({'architecture':'PE32 i386','sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
        'sources':{str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}},indent=2)+'\n')
    if selftest:
        definition = output/'scene.def'
        definition.write_text('LIBRARY MnmScene.dll\nEXPORTS\nSceneInstallForTest@4\n')
        subprocess.run(['llvm-dlltool','-m','i386','-D','MnmScene.dll','-d',str(definition),'-l',str(output/'scene.lib'),'--kill-at'],check=True)
        subprocess.run([*flags,'-c',str(ROOT/'runtime/scene/selftest.c'),'-o',str(output/'selftest.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no',
                        '/timestamp:0',f'/out:{output / "selftest.exe"}',str(output/'selftest.obj'),str(output/'scene.lib'),str(output/'kernel32.lib')],check=True)
        subprocess.run([*flags,'-c',str(ROOT/'runtime/scene/world_stream_selftest.c'),'-o',str(output/'world_stream_selftest.obj')],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no',
                        '/timestamp:0',f'/out:{output / "world-stream-selftest.exe"}',str(output/'world_stream_selftest.obj'),
                        str(output/'world_stream.c.obj'),str(output/'kernel32.lib')],check=True)
    return dll


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--selftest',action='store_true')
    print(build(parser.parse_args().selftest))
