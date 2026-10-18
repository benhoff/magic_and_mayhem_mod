#!/usr/bin/env python3
"""Build the freestanding PE32 native graphics compatibility DLL; no ddraw link."""
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
FLAGS = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding',
         '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx',
         '-Wall', '-Wextra', '-Werror']
IMPORTS = {
    'kernel32': {'GetModuleHandleA': 4, 'GetEnvironmentVariableA': 12, 'GetProcessHeap': 0, 'HeapAlloc': 12, 'HeapFree': 12,
                 'GetLastError': 0, 'SetLastError': 4, 'ExitProcess': 4,
                 'FlushInstructionCache': 12, 'VirtualAlloc': 16, 'VirtualFree': 12, 'VirtualProtect': 16, 'VirtualQuery': 12,
                 'AddVectoredExceptionHandler': 8, 'RemoveVectoredExceptionHandler': 4, 'GetCurrentThreadId': 0, 'CreateFileA': 28,
                 'WriteFile': 20, 'CloseHandle': 4, 'CreateFileMappingA': 24,
                 'MapViewOfFile': 20, 'FlushViewOfFile': 8, 'UnmapViewOfFile': 4},
    'user32': {'IsWindow': 4, 'GetClientRect': 8, 'MapWindowPoints': 16,
               'GetSystemMetrics': 4, 'GetDC': 4, 'ReleaseDC': 8},
    'gdi32': {'CreateCompatibleDC': 4, 'CreateDIBSection': 24, 'SelectObject': 8,
              'DeleteObject': 4, 'DeleteDC': 4, 'GdiFlush': 0, 'GetMapMode': 4, 'GetGraphicsMode': 4,
              'GetViewportOrgEx': 8, 'GetWindowOrgEx': 8, 'CreateRectRgn': 16, 'GetClipRgn': 8,
              'ExtCreateRegion': 12, 'GetRegionData': 12, 'CombineRgn': 16, 'GetRandomRgn': 12,
              'GetCurrentObject': 8, 'EqualRgn': 8},
}


def build(output, declared=None):
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    libraries = []
    for dll, imports in IMPORTS.items():
        definition = output/(dll+'.def')
        definition.write_text('LIBRARY '+dll+'.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n, v in imports.items()))
        library = output/(dll+'.lib')
        subprocess.run(['llvm-dlltool', '-m', 'i386', '--kill-at', '-d', definition, '-l', library], check=True)
        libraries.append(library)
    objects = []
    for name in ['runtime/native-draw/factory.c', 'reconstruction/rendering/pixel_kernel.c']:
        obj = output/(Path(name).stem+'.obj')
        dep = output/(Path(name).stem+'.d')
        subprocess.run([*FLAGS, '-MD', '-MF', dep, '-c', ROOT/name, '-o', obj], check=True)
        objects.append(obj)
    closure = set()
    for dep in [p.with_suffix('.d') for p in objects]:
        for token in dep.read_text().replace('\\\n', ' ').split():
            path = Path(token).resolve()
            if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                closure.add(path.relative_to(ROOT).as_posix())
    closure.add('tools/build-native-draw.py')
    if declared is not None and closure-set(declared):
        raise ValueError('Undeclared native DLL compiler/builder inputs: '+repr(sorted(closure-set(declared))))
    dll = output/'MnmDraw.dll'
    exports = {'NativeDrawCreate': ('NativeDrawCreate', 12),
               'DirectDrawCreate': ('NativeDrawCreate', 12),
               'NativeDrawEnumerateA': ('NativeDrawEnumerateA', 8),
               'DirectDrawEnumerateA': ('NativeDrawEnumerateA', 8),
               'NativeDrawObjects': ('NativeDrawObjects', 0),
               'NativeDrawClippers': ('NativeDrawClippers', 0),
               'NativeDrawPalettes': ('NativeDrawPalettes', 0),
               'NativeDrawDCs': ('NativeDrawDCs', 0),
               'NativeDrawDibWordsV1': ('NativeDrawDibWordsV1', 52),
               'NativeDrawBitBltV1': ('NativeDrawBitBltV1', 36),
               'NativeDrawSurfaces': ('NativeDrawSurfaces', 0),
               'NativeDrawPlaneV1': ('NativeDrawPlaneV1', 12),
               'NativeDrawCpuHistoryV1': ('NativeDrawCpuHistoryV1', 24),
               'NativeDrawGpuCopyV1': ('NativeDrawGpuCopyV1', 28),
               'NativeDrawGpuCopyPlanV1': ('NativeDrawGpuCopyPlanV1', 28),
               'NativeDrawCpuTransferV1': ('NativeDrawCpuTransferV1', 16),
               'NativeDrawCpuLeaseTransferV1': ('NativeDrawCpuLeaseTransferV1', 16),
               'NativeDrawCpuReaderV1': ('NativeDrawCpuReaderV1', 4),
               'NativeDrawCpuSnapshotV1': ('NativeDrawCpuSnapshotV1', 16),
               'NativeDrawCpuLeaseMaskV1': ('NativeDrawCpuLeaseMaskV1', 20),
               'NativeDrawCpuLeaseSnapshotV1': ('NativeDrawCpuLeaseSnapshotV1', 16),
               'NativeDrawCpuGuardWorldV1': ('NativeDrawCpuGuardWorldV1', 4),
               'NativeDrawCpuGuardWorldV2': ('NativeDrawCpuGuardWorldV2', 8),
               'NativeDrawProducerCountV1': ('NativeDrawProducerCountV1', 12),
               'NativeDrawAnchor': ('NativeDrawAnchor', 0),
               'NativeDrawStatusStage': ('NativeDrawStatusStage', 0),
               'NativeDrawStatusDeferV1': ('NativeDrawStatusDeferV1', 4)}
    subprocess.run(['lld-link', '/dll', '/machine:x86', '/entry:DllMain@12', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(dll),
                    *[f'/export:{alias}=_{symbol}@{size}' for alias, (symbol, size) in exports.items()],
                    *objects, *libraries], check=True)
    inspection = subprocess.check_output(['llvm-readobj', '--coff-imports', dll], text=True)
    if 'ddraw.dll' in inspection.lower():
        raise ValueError('Native DLL imports DirectDraw')
    (output/'native-imports.txt').write_text(inspection)
    (output/'manifest.json').write_text(json.dumps(dict(
        architecture='PE32 i386', sha256=hashlib.sha256(dll.read_bytes()).hexdigest(),
        directdraw_imports=False,
        sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in sorted(closure)}), indent=2)+'\n')
    return dll


if __name__ == '__main__':
    print(build(ROOT/'working/build/native-draw'))
