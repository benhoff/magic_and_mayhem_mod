#!/usr/bin/env python3
"""Bounded natural GPU-copy caller controls, retained analysis and private acquisition."""
import argparse
import ctypes
import hashlib
import io
import json
import os
from pathlib import Path
import shlex
import signal
import subprocess
import sys
import tarfile
import time
from coverage_evidence import read_bytes,read_json,write_report,evidence_digest
from natural_gpu_copy_branches import analyze
from private_x11_geometry import inspect,Attributes,move_game_left
from native_prefix_template import clone
from native_copy_window_exposure import build as exposure_build,move as exposure_move
from native_draw_retirement import require_retired
ROOT=Path(__file__).resolve().parents[1]

def sha(p):
    with Path(p).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()

def probe_binary(out,env):
    binary=out/'input-target-probe'
    flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core','Qt6Gui','xcb'],text=True))
    args=['c++','-std=c++17','-O1','-Wall','-Wextra','-Werror','-fPIC','-I'+str(ROOT/'apps/qt-shell'),str(ROOT/'tests/native-input-target-probe.cpp'),str(ROOT/'apps/qt-shell/window_host.cpp'),*flags,'-o',str(binary)]
    t=subprocess.run(args,env=env,capture_output=True,text=True,timeout=120)
    if t.returncode:raise ValueError('Input observation probe build: '+t.stderr[-3000:])
    p=subprocess.run([str(binary),'800','600'],env=env|dict(QT_QPA_PLATFORM='offscreen'),capture_output=True,text=True,timeout=10,check=True)
    abi=json.loads(p.stdout)
    if (abi['attributes_bytes'],abi['map_state_offset'],abi['all_event_masks_offset'])!=(ctypes.sizeof(Attributes),Attributes.map_state.offset,Attributes.all_event_masks.offset):
        raise ValueError('X11 attributes observation ABI differs from installed SDK')
    return binary,dict(command=args,sha256=sha(binary),abi=abi)

def extract_diagnostics(raw,out):
    total=0;count=0
    with tarfile.open(fileobj=io.BytesIO(raw),mode='r:gz') as archive:
        for member in archive:
            name=Path(member.name)
            if name.is_absolute() or '..' in name.parts:raise ValueError('Unsafe retained archive path')
            selected=member.name.endswith(('/lock-capture/copy-routes-v2.bin','/lock-capture/gpu-copy-census-v1.bin')) or '/copy-diagnostics/' in member.name
            if not selected or not member.isfile():continue
            total+=member.size;count+=1
            if total>512*1024**2 or count>256:raise ValueError('Retained diagnostics extraction cap')
            target=out/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(archive.extractfile(member).read())
    censuses=list(out.rglob('copy-routes-v2.bin'));reports=[p for p in out.rglob('report.json') if p.parent.name=='copy-diagnostics']
    if len(censuses)!=1 or len(reports)!=1:raise ValueError('Require one unmixed caller/output acquisition')
    return censuses[0].parent.parent,reports[0].parent

def retain_live(out,experiment,prefix):
    raw=io.BytesIO();references=[];files=0
    with tarfile.open(fileobj=raw,mode='w:gz',compresslevel=1) as archive:
        roots=[out]+([experiment] if experiment else [])
        for base in roots:
            for p in sorted(base.rglob('*')):
                if not p.is_file() or p.is_symlink():continue
                rel=p.relative_to(ROOT).as_posix()
                if '/wineprefix/' in rel and p.name not in ['system.reg','user.reg','userdef.reg','.mnm-directplay-dx6.json']:continue
                if experiment and base==experiment:
                    old=prefix/p.relative_to(experiment)
                    if old.is_file() and old.stat().st_size==p.stat().st_size and sha(old)==sha(p):
                        references.append(dict(path=rel,retained_path=str(old),sha256=sha(old)));continue
                archive.add(p,arcname=rel,recursive=False);files+=1
                if raw.tell()>160*1024**2:raise ValueError('Unique live archive cap')
    data=raw.getvalue()
    return data,dict(files=files,bytes=len(data),sha256=hashlib.sha256(data).hexdigest(),retained_elsewhere=references,scope='Unique run diagnostics and changed staged inputs retained. Unchanged input files remain exact-hash pinned in the prior owned stage;prefix binaries remain in the admitted source template. No cleanup or engine validation.')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--mode',choices=['controls','retained','published','quick'],required=True)
    for name in ['claims','output','report']:p.add_argument('--'+name,type=Path,required=True)
    for name in ['archive','shell','shell-build-report','prefix-template','source-game','retained-stage']:p.add_argument('--'+name,type=Path)
    p.add_argument('--fullscreen',action='store_true')
    p.add_argument('--win32-exposure',action='store_true',help='Use an ordinary Win32 SetWindowPos desktop exposure stimulus')
    p.add_argument('--clip-window',action='store_true',help='Declare one ordinary half-off-private-desktop movement after World entry')
    a=p.parse_args();out=a.output.resolve()
    if not out.is_relative_to(ROOT/'working') or out.exists() or a.report.exists():p.error('Fresh owned output/report required')
    if a.mode in ('retained','published') and not a.archive:p.error('Retained archive required')
    if a.mode=='quick' and not all([a.shell,a.shell_build_report,a.prefix_template,a.source_game,a.retained_stage]):p.error('Quick mode requires exact shell build,input stage and prefix ownership')
    declaration=json.loads(a.claims.read_text());sources=declaration['sources'];stable=lambda:all(evidence_digest(ROOT/name,root=ROOT)==h for name,h in sources.items())
    if not stable():p.error('Prospective sources changed')
    out.mkdir(parents=True);(out/'.retain').write_text('Natural GPU-copy caller acquisition and negative evidence;preserve required inputs.\n')
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],mode=a.mode,scope=__doc__,original_game_executed=False,complete_replacement=False)
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))};env.update(PYTHONDONTWRITEBYTECODE='1',TMPDIR=str(out))
    child=server=None;experiment=None;original_before=False
    def original(label):
        with (out/(label+'.log')).open('xb') as log:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],stdout=log,stderr=subprocess.STDOUT,check=True)
    try:
        if a.mode=='controls':
            c=[sys.executable,'-B',str(ROOT/'tests/test-natural-gpu-copy-branches.py'),'-v'];t=subprocess.run(c,cwd=ROOT,env=env,capture_output=True,text=True,timeout=60)
            report['controls']=dict(command=c,exit_code=t.returncode,stdout=t.stdout,stderr=t.stderr)
            if t.returncode:raise ValueError('Geometry/caller controls failed')
            c2=[sys.executable,'-B',str(ROOT/'tests/test-published-copy-routes.py'),'-v'];t2=subprocess.run(c2,cwd=ROOT,env=env,capture_output=True,text=True,timeout=60)
            report['published_controls']=dict(command=c2,exit_code=t2.returncode,stdout=t2.stdout,stderr=t2.stderr)
            if t2.returncode:raise ValueError('Published positive-only controls failed')
            _,report['input_probe_build']=probe_binary(out,env)
            _,report['exposure_helper_build']=exposure_build(out/'exposure-helper',env)
            report['success']=True
        elif a.mode in ('retained','published'):
            data=read_bytes(a.archive);report['input_archive']=dict(record=str(a.archive.resolve()),sha256=hashlib.sha256(data).hexdigest())
            e,d=extract_diagnostics(data,out/'decoded');report['analysis']=analyze(e,d,published_only=a.mode=='published');report['success']=True
            report['historical_acquisition']=True
        else:
            build=read_json(a.shell_build_report);report['shell_build_input']=dict(record=str(a.shell_build_report.resolve()),sha256=evidence_digest(a.shell_build_report))
            if not build['success'] or sha(a.shell)!=build['binary_sha256'] or any(sources.get(n)!=h for n,h in build['sources'].items()):raise ValueError('Shell differs from exact source-bound merged build')
            report['shell_sha256']=sha(a.shell)
            probe,report['input_probe_build']=probe_binary(out,env)
            if a.win32_exposure:exposure,report['exposure_helper_build']=exposure_build(out/'exposure-helper',env)
            original('original-before');original_before=True
            env.update(XDG_CONFIG_HOME=str(out/'config'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4',WINEDEBUG='-all',MNM_LIVE_MENU_TEST_QT_JOURNEY='1',MNM_ORIGINAL_WINDOWED_DIAGNOSTIC='0' if a.fullscreen else '1',MNM_MENU_GAME_SOURCE=str(a.source_game.resolve()),MNM_OTHER_PIXEL_PRODUCERS='1',MNM_NATIVE_PIXEL_NO_FORWARD='1',MNM_GLYPH_BACKEND='takeover',MNM_NATIVE_CPU_GUARD='3',MNM_NATIVE_COPY_DIAGNOSTICS=str(out/'copy-diagnostics'),MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all',MNM_VERIFY_ASSETS='1',MNM_VERIFY_ORIGINALS='1')
            screen='1280x1024x24' if a.fullscreen else '1280x1024x16';report['private_desktop_screen']=screen
            with (out/'display').open('w+') as display,(out/'xvfb.log').open('xb') as log:
                server=subprocess.Popen(['Xvfb','-displayfd',str(display.fileno()),'-screen','0',screen,'-nolisten','tcp'],pass_fds=(display.fileno(),),stdout=log,stderr=subprocess.STDOUT)
                end=time.monotonic()+10
                while time.monotonic()<end:
                    display.seek(0);number=display.read().strip()
                    if number:env['DISPLAY']=':'+number;break
                    if server.poll() is not None:raise ValueError('Private display exited')
                    time.sleep(.1)
                else:raise ValueError('Private display deadline')
            report['prefix_template']=clone(a.prefix_template,out/'wineprefix',env)
            command=[str(a.shell.resolve()),'--repo',str(ROOT),'--live-menus','--native-commands','--resident-world','--legacy-audio','--skip-movies','--no-native-music','--live-menu-test',str(out/'flow.json')]
            report['command']=command;report['private_window_observations']=[]
            start=time.monotonic()
            with (out/'shell.log').open('xb') as log:
                child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
                while child.poll() is None:
                    if time.monotonic()-start>260:raise ValueError('Bounded Quick Battle acquisition deadline')
                    lines=(out/'shell.log').read_text().splitlines();paths=[Path(x[20:]) for x in lines if x.startswith('Evidence directory: ')]
                    if len(paths)==1:experiment=paths[0]
                    observed=inspect(env['DISPLAY']);old=report['private_window_observations']
                    if (not old or old[-1]['windows']!=observed['windows']) and len(old)<64:
                        q=subprocess.run([str(probe),'800','600'],env=env,capture_output=True,text=True,timeout=10)
                        observed.update(seconds=time.monotonic()-start,selector_probe_exit=q.returncode,selector_probe=json.loads(q.stdout) if not q.returncode else dict(error=q.stderr[-1000:]));old.append(observed)
                    if (a.clip_window or a.win32_exposure) and 'desktop_exposure' not in report and experiment is not None and time.monotonic()-start>=100:
                        desktops=[x for x in observed['windows'] if x['title'].startswith('MagicMayhem - ') and x.get('attributes',{}).get('map_state')==2]
                        games=[x for x in observed['windows'] if x['title']=='Magic & Mayhem' and x.get('attributes',{}).get('map_state')==2]
                        if len(desktops)==len(games)==1:
                            report['desktop_exposure']=exposure_move(exposure,out/'wineprefix',env) if a.win32_exposure else move_game_left(env['DISPLAY'],desktops[0]['window'],games[0]['window']);report['desktop_exposure']['seconds']=time.monotonic()-start
                    time.sleep(1)
            report['shell_exit']=child.returncode
            if (out/'flow.json').exists():
                report['flow']=json.loads((out/'flow.json').read_text());report['original_game_executed']=bool(report['flow'].get('states'))
            report['lifecycle_passed']=child.returncode==0 and report.get('flow',{}).get('success') is True
    except Exception as error:report['error']=str(error)
    finally:
        if child is not None and child.poll() is None:
            os.killpg(child.pid,signal.SIGTERM)
            try:child.wait(timeout=10)
            except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
        if (out/'wineprefix').is_dir():
            subprocess.run(['wineserver','-k'],env=env|dict(WINEPREFIX=str(out/'wineprefix')),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10)
            subprocess.run(['wineserver','-w'],env=env|dict(WINEPREFIX=str(out/'wineprefix')),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True,timeout=10)
        if server is not None and server.poll() is None:server.terminate();server.wait(timeout=10)
        if a.mode=='quick':
            if experiment is None and (out/'shell.log').exists():
                paths=[Path(x[20:]) for x in (out/'shell.log').read_text().splitlines() if x.startswith('Evidence directory: ')]
                if len(paths)==1:experiment=paths[0]
            if experiment is not None:
                try:
                    report['analysis']=analyze(experiment,out/'copy-diagnostics');report['observation_verified']=True
                    report['all_requested_branches_observed']=all(report['analysis']['branches'][k]['proven'] for k in ['geometric_clipping','self_copy','keyed_gpu'])
                    report['success']=report['all_requested_branches_observed'] and report.get('lifecycle_passed',False) and report['analysis']['all_application_output_closed']
                except Exception as error:report['analysis_error']=str(error)
                resident=experiment/'resident-world-report.json'
                if resident.exists():report['resident']=json.loads(resident.read_text());report['original_game_executed'] |= int(report['resident'].get('completed',0))>0
                try:report['retirement']=require_retired(experiment/'native-draw-retirement.bin');report['retirement_passed']=True
                except Exception as error:report['retirement_passed']=False;report['retirement_error']=str(error)
                report['success'] &= report['retirement_passed']
            if original_before:
                try:original('original-after');report['original_verified_after']=True
                except Exception as error:report.update(success=False,original_manifest_error=str(error))
            try:
                data,receipt=retain_live(out,experiment,a.retained_stage);record=a.report.with_name(a.report.stem+'-raw.tar.gz');write_report(ROOT,record,data);write_report(ROOT,a.report.with_name(a.report.stem+'-retention.json'),(json.dumps(receipt,indent=2)+'\n').encode());report['raw_record']=str(record)
            except Exception as error:report.update(success=False,retention_error=str(error))
        report['sources_stable']=stable();report['success'] &= report['sources_stable'];write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ['success','error','analysis_error','observation_verified','lifecycle_passed','all_requested_branches_observed','sources_stable']}),flush=True)
    if report.get('analysis'):print(json.dumps({k:{x:y for x,y in v.items() if x!='examples'} for k,v in report['analysis']['branches'].items()}),flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
