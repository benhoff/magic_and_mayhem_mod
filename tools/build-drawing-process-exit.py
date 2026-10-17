#!/usr/bin/env python3
"""Build an independent process-handle observer without game artifacts."""
import hashlib,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCES=('runtime/drawing/process_exit_observer.c','runtime/shadow/win32_min.h','tools/build-drawing-process-exit.py')
IMPORTS=dict(GetEnvironmentVariableA=12,GetLastError=0,SetLastError=4,OpenThread=12,GetProcessIdOfThread=4,OpenProcess=12,GetExitCodeProcess=8,WaitForSingleObject=8,GetTickCount=0,CreateFileA=28,WriteFile=20,FlushFileBuffers=4,CloseHandle=4,ExitProcess=4)
def build(output=None):
 output=output or ROOT/'working/build/drawing-process-exit';output.mkdir(parents=True,exist_ok=True)
 definition=output/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in IMPORTS.items()))
 subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(output/'kernel32.lib')],check=True)
 subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/SOURCES[0]),'-o',str(output/'observer.obj')],check=True)
 binary=output/'process-exit.exe'
 subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(output/'observer.obj'),str(output/'kernel32.lib')],check=True)
 (output/'manifest.json').write_text(json.dumps({'schema':1,'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'sources':{n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in SOURCES}},indent=2)+'\n')
 return binary
if __name__=='__main__':print(build())
