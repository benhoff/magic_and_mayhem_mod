#!/usr/bin/env python3
"""Launch a verified staged render game under PE32 WineDbg (no live attachment)."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil

REPO=Path(__file__).resolve().parent.parent

def prepare(root):
    root=root.resolve()
    if not root.is_relative_to(REPO/'working/experiments/opengl-render'):
        raise ValueError('Expected an existing render experiment under working/')
    manifest=json.loads((root/'manifest.json').read_text());game=root/'game'
    for name,key in [('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256')]:
        if hashlib.sha256((game/name).read_bytes()).hexdigest()!=manifest[key]:
            raise ValueError(f'{name} differs from experiment manifest')
    stream=Path(manifest['stream']).resolve()
    if not stream.is_relative_to(REPO/'working'):raise ValueError('Unexpected stream location')
    with stream.open('rb') as file:
        if file.read(16)!=b'MNMGL001\x01\0\0\0\x40\0\0\0' or stream.stat().st_size!=64+2048*2048*4:
            raise ValueError('Invalid existing frame stream')
    source=Path('/usr/lib/wine/i386-windows/winedbg.exe')
    data=source.read_bytes()
    import struct
    pe=struct.unpack_from('<I',data,0x3c)[0]
    if data[pe:pe+4]!=b'PE\0\0' or struct.unpack_from('<H',data,pe+4)[0]!=0x14c:
        raise ValueError('Startup debugger must be PE32 i386')
    evidence=root/'startup-debug';evidence.mkdir(exist_ok=True)
    debugger=evidence/'RenderStartupDbg.exe';shutil.copy2(source,debugger)
    env=os.environ.copy();env['WINEPREFIX']=str(REPO/'working/wineprefix-x86_64')
    env['WINEDEBUG']='-all'
    env['WINEDLLOVERRIDES']=(env.get('WINEDLLOVERRIDES','')+';RenderStartupDbg=n').lstrip(';')
    env['MNM_RENDER_STREAM']='Z:'+str(stream).replace('/','\\')
    env.pop('MNM_RENDER_CAPTURE_DIR',None);env.pop('MNM_RENDER_HISTORY',None)
    for key,value in manifest['graphics_environment'].items():
        if value:env[key]=value
        else:env.pop(key,None)
    command=['wine',str(debugger),str(game/'Chaos.exe')]
    record={'command':command,'cwd':str(game),'debugger_sha256':hashlib.sha256(debugger.read_bytes()).hexdigest(),
            'architecture':'PE32 i386','game_loop_launched_by_prepare':False,
            'draw_capture':False,'note':'Start under debugger; do not attach to a running game. No explorer desktop wrapper.'}
    (evidence/'manifest.json').write_text(json.dumps(record,indent=2)+'\n')
    return command,game,env,evidence

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('experiment',type=Path);parser.add_argument('--prepare-only',action='store_true')
    args=parser.parse_args();command,game,env,evidence=prepare(args.experiment)
    print(f'Startup debugger evidence: {evidence}',flush=True)
    print('Close other game/debugger windows first. At Wine-dbg>, enter:',flush=True)
    print('  break *0x004e8d80\n  break *0x004e986a\n  cont',flush=True)
    print('At each breakpoint: bt, info reg, then cont. Avoid Ctrl+C: this WoW64 runtime has faulted in its debugger interrupt thread.',flush=True)
    print('If already stopped at 0xffbb10ec: info thread, then thread 0xID for an original Chaos.exe thread, then bt, info reg, x /32x $esp.',flush=True)
    print('To leave the game running: detach, then quit. Save terminal output with script if needed.',flush=True)
    if args.prepare_only:return 0
    os.chdir(game);os.execvpe(command[0],command,env)

if __name__=='__main__':raise SystemExit(main())
