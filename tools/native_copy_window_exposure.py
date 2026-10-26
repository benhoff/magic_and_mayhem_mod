"""Build a bounded ordinary Win32 desktop-exposure stimulus; no injected hooks."""
from pathlib import Path
import hashlib,struct,subprocess
ROOT=Path(__file__).resolve().parents[1]
IMPORTS={'user32':{'OpenDesktopA':16,'SetThreadDesktop':4,'FindWindowA':8,'IsWindowVisible':4,'GetWindowRect':8,'GetClientRect':8,'ClientToScreen':8,'GetWindowThreadProcessId':8,'SetWindowPos':28},'kernel32':{'GetLastError':0,'GetStdHandle':4,'WriteFile':20,'ExitProcess':4}}

def build(directory,environment):
    directory=Path(directory);directory.mkdir();libraries=[];commands=[]
    for dll,names in IMPORTS.items():
        definition=directory/(dll+'.def');definition.write_text('LIBRARY '+dll+'.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in names.items()))
        library=directory/(dll+'.lib');libraries.append(library);commands.append(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(library)])
    obj=directory/'exposure.obj';binary=directory/'exposure.exe'
    commands.append(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers','-c',str(ROOT/'tests/native-copy-window-exposure.c'),'-o',str(obj)])
    commands.append(['lld-link','/machine:x86','/subsystem:console','/entry:mainCRTStartup','/nodefaultlib','/out:'+str(binary),str(obj),*map(str,libraries)])
    for command in commands:subprocess.run(command,env=environment,capture_output=True,check=True,timeout=60)
    return binary,dict(commands=commands,sha256=hashlib.sha256(binary.read_bytes()).hexdigest())

def move(binary,prefix,environment):
    p=subprocess.run(['wine',str(Path(binary).resolve())],env=environment|dict(WINEPREFIX=str(Path(prefix).resolve())),capture_output=True,timeout=15)
    if p.returncode or len(p.stdout)!=72 or p.stdout[:8]!=b'MNMEXP01':raise ValueError('Private Win32 exposure refused: '+repr((p.returncode,p.stdout,p.stderr[-500:])))
    words=struct.unpack('<4I12i',p.stdout[8:])
    if words[0] or not words[2] or not words[3] or words[8]>=0 or words[14]>=0:raise ValueError('Private Win32 geometry proof malformed')
    return dict(result=words[0],last_error=words[1],window=words[2],pid=words[3],before=words[4:8],after=words[8:12],origin_before=words[12:14],origin_after=words[14:16],scope='One ordinary SetWindowPos half-off-private-desktop move. Unchanged original dispatch,clip-list API,copy commands and source pixels; declared desktop stress,not untouched journey.')
