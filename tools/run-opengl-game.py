#!/usr/bin/env python3
"""Stage and launch the pinned game with the DirectDraw frame bridge for Qt."""
import argparse
import hashlib
import importlib.util
import json
import os
import re
from pathlib import Path
import shutil
import subprocess
import tempfile
REPO=Path(__file__).resolve().parent.parent

def load(name,file):
    spec=importlib.util.spec_from_file_location(name,REPO/file)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

def skip_movies(game):
    """Edit only staged preferences, including the higher-precedence container."""
    plain=game/'CFG/prefs.cfg'; packed=game/'CFG/Encrypted/prefs.cfg'
    encoder=load('movie_cfg_encoder','tools/encode-cfg.py')
    decoder=encoder.load_decoder(REPO)
    paths=[path for path in (plain,packed) if path.exists()]
    if not paths:raise ValueError('No staged movie preferences found')
    edits=[]
    for path in paths:
        before=path.read_bytes()
        text=decoder.decode(before)[1] if path==packed else before
        video=False;counts={'PlayFMV':0,'PlayFMVOut':0};lines=[]
        for line in text.splitlines(keepends=True):
            section=re.match(rb'\s*\[([^]]+)\]',line)
            if section:video=section[1].upper()==b'VIDEO'
            match=re.match(rb'(\s*(PlayFMVOut|PlayFMV)\s*=\s*)(TRUE|FALSE)([ \t]*(?:;[^\r\n]*)?)(\r?\n)?$',line,re.I) if video else None
            if match:
                key=match[2].decode('ascii').lower()
                name='PlayFMVOut' if key=='playfmvout' else 'PlayFMV'
                counts[name]+=1
                line=match[1]+b'FALSE'+match[4]+(match[5] or b'')
            lines.append(line)
        if any(count!=1 for count in counts.values()):raise ValueError(f'Ambiguous movie preferences: {path}')
        changed=b''.join(lines)
        after=encoder.encode(changed,decoder)[0] if path==packed else changed
        if path==packed and decoder.decode(after)[1]!=changed:raise ValueError('Movie CFG round trip failed')
        edits.append((path,before,after))
    # Validate every copy before writing any staged preference.
    result=[]
    for path,before,after in edits:
        path.write_bytes(after)
        result.append({'path':str(path.relative_to(game)),'before_sha256':hashlib.sha256(before).hexdigest(),
                       'after_sha256':hashlib.sha256(after).hexdigest()})
    return result

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--stream',type=Path,required=True);parser.add_argument('--stage-only',action='store_true')
    parser.add_argument('--capture-draws',action='store_true',help='Record bounded draw events and one small native-pixel blit')
    parser.add_argument('--capture-history',action='store_true',help='Opt in to bounded indexed/RGB surface history; implies --capture-draws')
    parser.add_argument('--skip-movies',action='store_true',help='Disable movie playback only in the disposable staged preferences')
    args=parser.parse_args();args.capture_draws |= args.capture_history
    stream=args.stream.resolve()
    if not stream.is_relative_to(REPO/'working'):raise ValueError('Frame stream must be under working/')
    with stream.open('rb') as file:
        if stream.stat().st_size!=64+2048*2048*4 or file.read(16)!=b'MNMGL001\x01\0\0\0\x40\0\0\0':raise ValueError('Invalid pre-created frame stream')
    stage=load('shadow_stage','tools/prepare-shadow-experiment.py');source=REPO/'working/game-nocd';data=(source/'Chaos.exe').read_bytes()
    if hashlib.sha256(data).hexdigest()!=stage.HASH:raise ValueError('Unsupported game hash')
    dll=load('render_build','tools/build-render-bridge.py').build()
    parent=REPO/'working/experiments/opengl-render';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));game=root/'game'
    subprocess.run(['cp','-a','--reflink=auto',str(source),str(game)],check=True)
    if hashlib.sha256((game/'Chaos.exe').read_bytes()).hexdigest()!=stage.HASH:raise ValueError('Copied game hash changed')
    (game/'Chaos.exe').write_bytes(stage.add_import(data,dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
    shutil.copy2(dll,game/dll.name);shutil.copy2(dll.parent/'manifest.json',root/'bridge-build.json')
    movie_edits=skip_movies(game) if args.skip_movies else []
    metadata={'failure_log':str(root/'surface-failures.log'),'skip_movies':args.skip_movies,'movie_preference_edits':movie_edits,'origin':'directdraw_opengl_presentation','source_sha256':stage.HASH,'stream':str(stream),
              'staged_sha256':hashlib.sha256((game/'Chaos.exe').read_bytes()).hexdigest(),'capture_history':args.capture_history,'graphics_environment':{key:os.environ.get(key,'') for key in ('LIBGL_ALWAYS_SOFTWARE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','WINE_D3D_CONFIG')},'dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest()}
    if args.capture_draws:
        capture=root/'draw-capture';capture.mkdir();metadata['draw_capture_directory']=str(capture)
        print(f'Draw capture: {capture}',flush=True)
    (root/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n');print(f'Render experiment: {root}',flush=True)
    if args.stage_only:return 0
    env=os.environ.copy();env['MNM_RENDER_STREAM']='Z:'+str(stream).replace('/','\\');env['MNM_RENDER_EXPERIMENT']=str(root);env.pop('MNM_RUNNER',None)
    env['MNM_RENDER_FAILURE_LOG']='Z:'+metadata['failure_log'].replace('/','\\')
    env.pop('MNM_RENDER_CAPTURE_DIR',None);env.pop('MNM_RENDER_HISTORY',None)
    if args.capture_history:env['MNM_RENDER_HISTORY']='1'
    if args.capture_draws:env['MNM_RENDER_CAPTURE_DIR']='Z:'+str(capture).replace('/','\\')
    result=subprocess.run([str(REPO/'tools/run-game.sh'),'launch','--no-gamescope','--prefix',str(REPO/'working/wineprefix-x86_64'),'--runner',str(REPO/'tools/render-game-runner.py')],env=env,check=False)
    for name,key in [('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256')]:
        if hashlib.sha256((game/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} changed')
    if hashlib.sha256((source/'Chaos.exe').read_bytes()).hexdigest()!=stage.HASH:raise ValueError('Source game changed')
    return result.returncode
if __name__=='__main__':raise SystemExit(main())
