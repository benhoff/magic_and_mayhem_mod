#!/usr/bin/env python3
"""Bounded public native-only clipping journey on a private 16-bit desktop.

Use the existing public Quick journey. Low requests one ordinary Win32 window
move at first Main and keeps it through normal Quit; High uses natural windowed
clipping. Bounded diagnostic selection changes no draw or original pixel input.
"""
import argparse
import ctypes as C
import errno
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sqlite3
import shutil
import sys
import time
from native_copy_window_exposure import move as win32_move

from coverage_evidence import Reader
from private_x11_geometry import Attributes, inspect, move_game_left
from rendering_interaction_budget import admit
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-clipped-operations-results-20261107/'
PLAN=PREFIX+'input-plan-12.json'
OWN={'tools/run-rendering-clipping.py','tools/prepare-rendering-clipping.py',
     'tools/private_x11_geometry.py','tools/private_x11_windows.py',
     'tools/native_windowed_copy.py','tools/native_copy_window_exposure.py','tests/native-copy-window-exposure.c'}


def accounted_game_count(reader,plan,name,data):
    """Apply an explicit count-only review without changing historical payloads."""
    matches=[]
    for ref in plan.get('game_count_reviews',[]):
        require(reader.digest(ref['record'])==ref['sha256'],'Changed game-count review')
        review=reader.json(ref['record'])
        if review['acquisition']==dict(record=name,sha256=reader.digest(name)):
            matches.append((ref,review))
    if not matches and type(data['original_game_runs']) is int:return data
    require(len(matches)==1,'Unknown original game count; exact review required')
    ref,review=matches[0]
    require(review['kind']=='clipping_acquisition_game_count_review' and review['success']
            and review['recorded_original_game_runs']==data['original_game_runs']
            and review['reviewed_original_game_runs']==1 and not data['success'],
            'Game-count review cannot confer acceptance')
    return dict(data,original_game_runs=1,game_count_review=ref)


def borrow_with_deadline(borrow,template,target,environment,timeout=120):
    deadline=time.monotonic()+timeout
    while True:
        try:return borrow(template,target,environment)
        except (ValueError,BlockingIOError) as error:
            busy=(isinstance(error,ValueError) and str(error)=='Both environment slots are busy or held; no additional clone was created')
            locked=(isinstance(error,BlockingIOError) and error.errno in (errno.EAGAIN,errno.EWOULDBLOCK)
                    and not target.exists() and not target.is_symlink())
            if not (busy or locked) or time.monotonic()>=deadline:raise
            time.sleep(min(2,max(0,deadline-time.monotonic())))


def prepare_capture_paths(out,ram):
    require(not (out/'worlds.bin').exists() and not (out/'worlds.bin').is_symlink()
            and not (out/'capture').exists() and not (out/'capture').is_symlink(),
            'Capture destinations must be fresh; QFile NewOnly cannot follow a precreated file alias')
    ram.mkdir(parents=True,exist_ok=False);(ram/'.retain').touch();(ram/'capture').mkdir()
    (out/'capture').symlink_to(ram/'capture',target_is_directory=True)


def restore_window(display, movement):
    """Restore only the same still-mapped ordinary X window to its old origin."""
    import ctypes.util
    x=C.CDLL(ctypes.util.find_library('X11'))
    x.XOpenDisplay.argtypes=[C.c_char_p];x.XOpenDisplay.restype=C.c_void_p
    x.XCloseDisplay.argtypes=[C.c_void_p]
    x.XGetWindowAttributes.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(Attributes)]
    x.XMoveWindow.argtypes=[C.c_void_p,C.c_ulong,C.c_int,C.c_int]
    x.XSync.argtypes=[C.c_void_p,C.c_int]
    d=x.XOpenDisplay(display.encode());require(d,'Private display unavailable on restore')
    try:
        a=Attributes();window=movement['window']
        require(x.XGetWindowAttributes(d,window,C.byref(a)) and a.map_state==2
                and [a.x,a.y,a.width,a.height]==movement['after'],
                'Moved original window changed before restoration')
        x.XMoveWindow(d,window,*movement['before'][:2]);x.XSync(d,0)
        require(x.XGetWindowAttributes(d,window,C.byref(a))
                and [a.x,a.y,a.width,a.height]==movement['before'],
                'Original window restoration did not apply')
        return [a.x,a.y,a.width,a.height]
    finally:x.XCloseDisplay(d)


def desktop_display(launcher, directory):
    """Use the existing private-server owner setup with the declared 16-bit mode.

    A local subprocess facade changes only that function's Xvfb screen argument;
    the source function and this adapter are both pinned in the input plan.
    """
    class Processes:
        DEVNULL=subprocess.DEVNULL
        def Popen(self, args, **kwargs):
            require(args[:2]==['Xvfb','-displayfd'] and args.count('1280x1024x24')==1,
                    'Private display launcher shape changed')
            return subprocess.Popen(['1280x1024x16' if v=='1280x1024x24' else v for v in args],**kwargs)
    original=launcher.subprocess
    try:
        launcher.subprocess=Processes()
        return launcher.start_display(directory)
    finally:launcher.subprocess=original


def main():
    parser=argparse.ArgumentParser(add_help=False)
    parser.add_argument('--report',required=True)
    parser.add_argument('--low',action='store_true')
    args,_=parser.parse_known_args()
    # Count preparation refusals as requests too. The reused base registers a
    # request only after borrowing a prefix, which omits these real failures.
    with Reader(ROOT) as reader, sqlite3.connect(ROOT/'working/interaction-attempts.sqlite3') as db:
        plan=reader.json(PLAN)
        previous=[(p.relative_to(ROOT).as_posix(),accounted_game_count(reader,plan,p.relative_to(ROOT).as_posix(),reader.json(p.relative_to(ROOT))))
                  for p in sorted((ROOT/PREFIX).glob('*-acquisition.json'))]
        require(all(type(a['original_game_runs']) is int and a['original_game_runs'] in (0,1)
                    for _,a in previous),'Unknown original game count; refuse another launch')
        require(len(previous)<plan['bounds']['maximum_requests'] and
                sum(a['original_game_runs'] for _,a in previous)<plan['bounds']['maximum_original_games'],
                'Complete clipping preparation/game budget exhausted')
        db.execute('CREATE TABLE IF NOT EXISTS attempts(report TEXT PRIMARY KEY,repair_of TEXT)')
        for name,a in previous:db.execute('INSERT OR IGNORE INTO attempts VALUES(?,?)',(name,a.get('repair_of')))
    if os.environ.get('MNM_INTERACTION_BUS_OWNED')!='1':
        runtime=ROOT/'working/interaction-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True,mode=0o700,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_INTERACTION_BUS_OWNED='1')
        env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    spec=importlib.util.spec_from_file_location('clipping_base',ROOT/'tools/run-rendering-interaction.py')
    base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    base.PREFIX=PREFIX;base.PLAN=PLAN
    borrow=base.borrow
    base.borrow=lambda template,target,environment:borrow_with_deadline(borrow,template,target,environment)
    original_sources=base.native_runtime_source_paths
    base.native_runtime_source_paths=lambda root:original_sources(root)|OWN
    configure=base.configure
    ram=Path('/tmp/mnm-rendering-clipping-20261107')/Path(args.report).stem
    out=ROOT/'working/interaction-acceptance'/Path(args.report).stem
    def setup(env,argv):
        require(subprocess.check_output(['findmnt','-T','/tmp','-n','-o','FSTYPE'],text=True).strip()=='tmpfs'
                and shutil.disk_usage('/tmp').free>=8*1024**3,'Insufficient RAM-backed capture capacity')
        prepare_capture_paths(out,ram)
        return dict(configure(env,argv),MNM_ORIGINAL_WINDOWED_DIAGNOSTIC='1',
            MNM_NATIVE_COPY_DIAGNOSTICS=str(out/'copy-diagnostics'),MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all',
            MNM_NATIVE_COPY_CLIP_REGIONS='1',MNM_NATIVE_COPY_DISTINCT_INPUTS='1')
    base.configure=setup
    module=base.menus.module
    def load(name):
        value=module(name)
        if name=='run-native-owned.py':
            start=value.start_display
            # A separate module retains the unmodified function/global namespace
            # used for the 16-bit owner; host presentation keeps its 24-bit mode.
            original=module(name)
            value.start_display=lambda directory,allow_glx=False: start(directory,allow_glx=True) if allow_glx else desktop_display(original,directory)
        return value
    base.menus.module=load

    class ClippingJourney(base.InteractionJourney):
        def wait_progress(self,after=0,minimum=5,timeout=30):
            # A windowed primary may retain800x600 while the Low World is640x480.
            # Offline admission checks every actual World CREATE/import/source.
            deadline=time.monotonic()+timeout
            while time.monotonic()<deadline:
                require(self.child.poll() is None,'Native player exited before clipping progress')
                row=self.viewport.poll()
                if row:
                    require([row['frame_width'],row['frame_height']] in ([800,600],[640,480]),
                            'Windowed primary presentation extent unsupported')
                    if int(row['world'])>=after+minimum:return row
                time.sleep(.05)
            raise ValueError('New native clipping source/presentation deadline')

        def mark(self,action,phase):
            point=super().mark(action,phase);row=self.viewport.poll()
            point.update(presentation_size=[row['frame_width'],row['frame_height']],
                         actual_size_role='original-world-source-not-primary-presentation')
            return point

        def read(self):
            value=super().read()
            # Owner journal polling is sufficient during staging. Walking the
            # accessibility tree before admission can time out while Qt starts
            # native assets. The existing finite await_main deadline remains.
            if args.low and value and value[0]['ready'] and value[0]['screen']==3 and not getattr(self,'win32_exposure',None):
                binary=ROOT/plan['exposure_helper']['path']
                require(hashlib.sha256(binary.read_bytes()).hexdigest()==plan['exposure_helper']['sha256'],
                        'Pinned ordinary Win32 exposure helper changed')
                environment=dict(self.env,DISPLAY=self.env['MNM_LEGACY_DISPLAY'],
                                 XAUTHORITY=str(self.out/'owner/Xauthority'))
                # The private /tmp hides RAM helper paths and Wine sockets.
                # Stage exact helper bytes in the admitted session and enter
                # only that runner's validated descendant namespace set.
                staged=self.session/'window-exposure-helper.exe'
                with staged.open('xb') as target:target.write(binary.read_bytes())
                admission=json.loads((self.session/'native-output-admission.json').read_bytes())
                require(admission['session']==str(self.session) and
                        Path(admission['prefix']).resolve()==(self.out/'wineprefix').resolve(),
                        'Exposure admission does not own this session/prefix')
                self.win32_exposure=win32_move(staged,self.out/'wineprefix',environment,admission['runner_identity'])
                self.actions.append(dict(action='ordinary-win32-clipping-move',host_ns=time.monotonic_ns(),
                    movement=self.win32_exposure,stage='first-ready-main-before-quick',
                    retirement='Keep ordinary window position through normal Quit; private window is then destroyed'))
            return value

        def interaction_schedule(self):
            require(not args.low or getattr(self,'win32_exposure',None), 'Low needs actual Win32 geometry admission')
            self.focus_viewport()
            self.held_key('center-first',ord('w'),.22,1.)
            self.held_key('center-second',ord('w'),.22,1.)
            point=self.mark('declared-windowed-clipping','begin')
            self.wait_progress(int(point['world']),minimum=8,timeout=15)
            time.sleep(1.)
            self.mark('declared-windowed-clipping','settled')

    base.InteractionJourney=ClippingJourney
    writer=base.write_report
    def write(root,path,raw):
        if path==args.report:
            data=json.loads(raw)
            data.update(kind='public_native_clipping_acquisition',prospective_plan=PLAN,scope=__doc__)
            data['configuration'].update(original_windowed=True,private_desktop='1280x1024x16',
                desktop_stimulus='ordinary-win32-low-window-move-at-main-to-normal-quit' if args.low else 'natural-high-windowed-client-clipping',viewport_scaling='sharp',
                temporary_capture_storage='C5-and-fresh-exclusive-C3-in-tmpfs',ram_capture_root=str(ram),
                clip_region_observation='native-provider-rgndata-v1',
                copy_output_scope='all-selected-distinct-input-planes; duplicate/class/byte exclusions receive no output credit')
            # The base's filesystem census deliberately skips aliases. Retain
            # each stopped RAM capture through the same immutable writer and
            # preserve the usual logical artifact path for existing readers.
            for file in sorted(ram.rglob('*')) if ram.exists() else []:
                if not file.is_file() or file.name=='.retain':continue
                relative=file.relative_to(ram)
                limit=2*1024**3 if relative.as_posix()=='worlds.bin' else 1024**3
                require(not file.is_symlink() and file.stat().st_size<=limit,'RAM capture exceeds existing retention bound')
                logical=(out/relative).relative_to(ROOT).as_posix()
                name=args.report.removesuffix('.json')+'-ram-'+hashlib.sha256(logical.encode()).hexdigest()[:20]+'.bin'
                body=file.read_bytes()
                data['artifacts'].append(dict(path=logical,record=name,sha256=writer(root,name,body),bytes=len(body)))
                if relative.as_posix()=='worlds.bin':
                    # Compatibility for stopped physical readers, only after
                    # the exclusive source producer has finished and retention.
                    require(not (out/'worlds.bin').exists() and not (out/'worlds.bin').is_symlink(),
                            'Stopped source compatibility path changed')
                    (out/'worlds.bin').symlink_to(file)
            raw=(json.dumps(data,indent=2)+'\n').encode()
        return writer(root,path,raw)
    base.write_report=write
    # The existing request gate still stops on an unreviewed pilot. A later
    # domain requires a genuine clipping comparison, not an interaction label.
    def budget(cases,repair_of,maximum_requests,maximum_games):
        with Reader(ROOT) as reader:
            for case in cases:
                case['result']=accounted_game_count(reader,plan,case['report'],case['result'])
                ref=dict(record=case['report'],sha256=reader.digest(case['report']))
                paths=(ROOT/PREFIX).glob(Path(case['report']).name.replace('-acquisition.json','-comparison')+'*.json')
                case['comparison_failed']=any((failed:=reader.json(p.relative_to(ROOT))).get('kind')=='public_native_clipping_comparison' and failed.get('acquisition')==ref and failed.get('success') is False for p in paths)
                basis_ref=plan.get('repair_basis')
                if basis_ref:
                    require(reader.digest(basis_ref['record'])==basis_ref['sha256'],'Changed clipping repair basis')
                    basis=reader.json(basis_ref['record'])
                    if basis['acquisition']==ref:
                        failure=basis['formal_refusal']
                        require(reader.digest(failure['record'])==failure['sha256'] and reader.json(failure['record'])['success'] is False,
                                'Clipping repair lacks the exact retained failure')
                        case['comparison_failed']=True
                paths=(ROOT/PREFIX).glob(Path(case['report']).name.replace('-acquisition.json','-comparison')+'*.json')
                case['proved']=any((v:=reader.json(p.relative_to(ROOT))).get('kind')=='public_native_clipping_comparison'
                    and v.get('acquisition')==ref and v.get('success') is True
                    and v.get('complete_output_verified') is True and v.get('positive_clipped_calls',0)>0
                    and v.get('native_clip_regions_verified') is True
                    for p in paths)
        return admit(cases,repair_of,maximum_requests,maximum_games)
    base.admit=budget
    return base.main(source_capture_path=ram/'worlds.bin')


if __name__=='__main__':raise SystemExit(main())
