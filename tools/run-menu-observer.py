#!/usr/bin/env python3
"""Run a fresh menu observer experiment through the normal game launcher."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
REPO=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('experiment',type=Path)
    parser.add_argument('--menu-channel',type=Path,help='Opt in to the versioned menu action bridge')
    parser.add_argument('--native-render',action='store_true',help='Launch the staged native menu/rendering session')
    parser.add_argument('--seconds',type=int,help='Bounded smoke duration (1..120 seconds)')
    parser.add_argument('--prefix',type=Path,default=REPO/'working/tests/menu-live-wine')
    args=parser.parse_args()
    if args.seconds is not None and not 1<=args.seconds<=120:parser.error('--seconds must be 1..120')
    root=args.experiment.resolve();metadata=json.loads((root/'manifest.json').read_text())
    if metadata['origin'] not in ('menu_observation_only','menu_action_bridge') or metadata['events']!=str(root/'events.bin'):raise ValueError('Unsupported experiment')
    if (root/'events.bin').exists():raise ValueError('Stage a fresh experiment; observation evidence already exists')
    for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
        if hashlib.sha256((root/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} hash mismatch')
    if args.native_render and not metadata.get('native_render'):raise ValueError('Native rendering was not staged')
    if args.native_render:
        if hashlib.sha256((root/'game/MnmRender.dll').read_bytes()).hexdigest()!=metadata['render_dll_sha256']:raise ValueError('Render DLL hash mismatch')
    if args.menu_channel:
        if metadata['origin']!='menu_action_bridge':raise ValueError('Stage with --actions to enable commands')
        channel=args.menu_channel.resolve()
        if channel!=root/'channel.bin' or channel.stat().st_size not in (128,32768,65536,69632,73728,77824,81920,86016,90112,94208,102400,106496,131072,147456,163840,180224,196608,262144,327680,393216):raise ValueError('Invalid menu channel')
        if channel.stat().st_size==69632 and not metadata.get('experimental_mini',False):raise ValueError('V4 requires explicitly staged experimental Mini Menu hooks')
    env=dict(os.environ,MNM_MENU_EXPERIMENT=str(root),WINEDEBUG=os.environ.get('WINEDEBUG','fixme-all'))
    env.pop('MNM_MENU_CHANNEL',None)
    env.pop('MNM_AUDIO_CHANNEL',None)
    env.pop('MNM_MENU_GRIMOIRE_OBSERVE',None)
    if metadata.get('grimoire_observe'):env['MNM_MENU_GRIMOIRE_OBSERVE']='Z:'+str(root/'grimoire.bin').replace('/','\\')
    env['MNM_MENU_NATIVE_RENDER']='1' if args.native_render else '0'
    if type(metadata.get('original_gameplay',False)) is not bool:raise ValueError('Invalid original gameplay observation policy')
    if metadata.get('original_gameplay'):
        if args.native_render or args.menu_channel or metadata['origin']!='menu_observation_only' or not metadata.get('campaign_observe') or metadata.get('native_render') or metadata.get('native_draw') or metadata.get('native_voices'):
            raise ValueError('Original gameplay observation requires original display and controls')
        env['MNM_MENU_ORIGINAL_GAMEPLAY']='1'
        env['MNM_MENU_CAMPAIGN_OBSERVE']='Z:'+str(root/'campaign.bin').replace('/','\\')
    if args.menu_channel:env['MNM_MENU_CHANNEL']='Z:'+str(channel).replace('/','\\')
    if metadata.get('directplay_dx6') is not None:
        from directplay_dx6_launch import admit_prefix
        admit_prefix(root,metadata,args.prefix)
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        command=[str(REPO/'tools/run-game.sh'),'smoke' if args.seconds else 'launch','--no-gamescope',
                 '--prefix',str(args.prefix.resolve()),'--runner',str(REPO/'tools/menu-game-runner.py')]
        if metadata.get('source_game') and Path(metadata['source_game']).resolve()!=(REPO/'working/game-nocd').resolve():command+=['--source-game',metadata['source_game']]
        if args.seconds:command+=['--seconds',str(args.seconds)]
        result=subprocess.run(command,env=env)
        if metadata.get('directplay_dx6') is not None:
            from directplay_dx6_launch import check as check_directplay,check_prefix
            check_directplay(root,metadata['directplay_dx6'])
            installed=json.loads((root/'manifest.json').read_text()).get('directplay_prefix_result')
            if installed is not None:check_prefix(args.prefix.resolve())
            elif result.returncode==0:raise ValueError('Supplied DirectPlay was not installed')
        for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
            if hashlib.sha256((root/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise ValueError(f'{name} changed')
        if metadata.get('save_store'):
            save_result={'merged':False,'reason':'Unclean or bounded termination; session saves retained.'}
            if result.returncode==0 and args.seconds is None:
                from menu_save_store import commit as commit_saves
                try:
                    save_result=commit_saves(metadata['save_store']);save_result['merged']=True
                except (OSError,ValueError) as error:
                    save_result['error']=str(error)
                    (root/'save-store-result.json').write_text(json.dumps(save_result,indent=2)+'\n')
                    print('Save synchronization failed; session files retained: '+str(error),flush=True)
                    return 1
            (root/'save-store-result.json').write_text(json.dumps(save_result,indent=2)+'\n')
        return result.returncode
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':raise SystemExit(main())
