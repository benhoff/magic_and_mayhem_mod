#!/usr/bin/env python3
"""New bounded source-epoch, palette lifetime and opaque ownership fixtures."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--case',choices=['source','palette','borrow'],required=True)
    p.add_argument('--output',type=Path,required=True);p.add_argument('--wine-prefix',type=Path)
    a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    report=dict(schema=1,success=False,case=a.case,results=[],scope=__doc__,original_game_executed=False)
    def run(label,cmd,env=None,cwd=out):
        result=subprocess.run([str(v) for v in cmd],cwd=cwd,env=env,capture_output=True,text=True,timeout=180)
        (out/(label+'.log')).write_text(result.stdout+result.stderr)
        if result.returncode:raise RuntimeError(label+': exit '+str(result.returncode))
        return result.stdout
    try:
        if a.case in ('source','borrow'):
            path=ROOT/('tests/world-source-regions-test.cpp' if a.case=='source' else 'tests/render-opaque-borrow-test.c')
            for label,flags in [('native',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
                binary=out/label
                run(label+'-build',['g++' if a.case=='source' else 'cc','-Wall','-Wextra','-Werror',*flags,path,'-o',binary])
                result=run(label,[binary],dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1'))
                report['results'].append(dict(mode=label,cases=[json.loads(v) for v in result.splitlines()]))
            if a.case=='source':run('producer-build',['python3',ROOT/'tools/build-scene-observer.py'])
        else:
            if not a.wine_prefix or not a.wine_prefix.resolve().is_relative_to(ROOT/'working/tests'):raise ValueError('Owned test prefix required')
            spec=importlib.util.spec_from_file_location('palette_builder',ROOT/'tools/build-render-bridge.py');b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
            dll=b.build(True)
            run('probe-build',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror','-c',ROOT/'tests/render-palette-lifetime-probe.c','-o',out/'probe.obj'])
            run('probe-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',dll.with_suffix('.lib'),dll.parent/'kernel32.lib'])
            env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
            env.update(WINEPREFIX=str(a.wine_prefix.resolve()),WINEDEBUG='-all',MNM_RENDER_ORDERED_COPIES='1',MNM_RENDER_PALETTE_RESOURCES='1')
            (out/'MnmRender.dll').write_bytes(dll.read_bytes())
            for name,collision in [('ordered-new-generation',False),('release-recreate-during-bind',True)]:
                dest=out/name;dest.mkdir();capture=dest/'capture';capture.mkdir()
                sys.path.insert(0,str(ROOT/'protocols/python'))
                from mnm_protocols import frame_v1,render_commands_v2 as ring
                frame=dest/'frame.bin'
                with frame.open('xb') as f:f.write(frame_v1.initial_header());f.truncate(frame_v1.SIZE)
                channel=dest/'commands.bin';header=bytearray(ring.initial_header());struct.pack_into('<I',header,16,123)
                with channel.open('xb') as f:f.write(header);f.truncate(ring.SIZE)
                child=dict(env,MNM_PALETTE_RACE=str(int(collision)),MNM_RENDER_CONTINUOUS='1',MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_RENDER_AUTO_SHUTDOWN='0',MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
                run(name,['wine',out/'probe.exe'],child,dest)
                result=struct.unpack('<7I',(dest/'palette-race.bin').read_bytes())
                assert result[0]==int(collision) and result[5:]==(1,2)
                assert not result[3] if collision else result[2]>result[1] and result[3:5]==(1,1)
                report['results'].append(dict(case=name,state=list(result),exact_com_results=True,last_error_preserved=True))
            subprocess.run(['wineserver','-k'],env=env,check=False,timeout=15)
        report['success']=True
    except Exception as error:report['error']=str(error)
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
