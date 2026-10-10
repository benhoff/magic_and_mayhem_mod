#!/usr/bin/env python3
"""Bounded enabled native intro playback followed by original frame publication."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import signal
import struct
import subprocess
import sys
import tempfile
import time

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import frame_v1


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under isolated Xvfb')
    claims=json.loads(args.claims.read_text())
    for name,digest in claims['sources'].items():
        if sha(ROOT/name)!=digest:
            raise ValueError('Prospective source changed: '+name)
    parent=ROOT/'working/tests/native-enabled-movies';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report={'success':False,'sources':claims['sources'],'claims':claims['claims'],
            'scope':'Bounded enabled NoCD intro0/intro1 through native Qt media hook/broker and subsequent original frame publication. No original DirectShow equivalence, arbitrary controls/placement, audible device, active World ownership or full replacement.',
            'live_replacement':False,'original_pixels_used_as_native_movie_inputs':False}
    inputs={};reservation=None;wine=None;qt=None;env=None

    def run(label,command):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(v) for v in command],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=240,check=True)

    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        spec=importlib.util.spec_from_file_location('movie_wine_reservation',ROOT/'tools/capture-original-surface-keys.py')
        keys=importlib.util.module_from_spec(spec);spec.loader.exec_module(keys);reservation=keys.reserve_wine()
        source=ROOT/'working/game-nocd/Chaos.exe'
        expected='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
        if sha(source)!=expected:
            raise ValueError('Unsupported original build')
        report['source_executable_sha256']=expected;inputs[str(source.relative_to(ROOT))]=sha(source)
        for path in sorted((ROOT/'working/game-nocd/FMV').glob('Intro*.avi')):
            inputs[str(path.relative_to(ROOT))]=sha(path)
        shell=ROOT/'working/build/qt-shell/mnm-qt-shell'
        run('build',['cmake','--build',ROOT/'working/build/qt-shell','--target','mnm-qt-shell','-j4'])
        # Bind the actual shell's complete local compile closure, not just its
        # media translation unit. Execution claims must have declared it first.
        import shlex
        dependencies=set()
        for dependency in (ROOT/'working/build/qt-shell').rglob('*.o.d'):
            if any(part.startswith('mnm-') and part.endswith('.dir') for part in dependency.parts):
                for name in shlex.split(dependency.read_text().replace('\\\n',' ')):
                    p=Path(name).resolve() if Path(name).is_absolute() else Path(name)
                    if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                        dependencies.add(str(p.relative_to(ROOT)))
        if dependencies-set(report['sources']):
            raise ValueError('Undeclared shell compiler inputs: '+repr(sorted(dependencies-set(report['sources']))))
        frame=out/'frame.bin'
        with frame.open('xb') as f:
            f.write(frame_v1.initial_header());f.truncate(frame_v1.SIZE)
        channel=out/'media.bin';history=out/'media.json'
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.update(QT_QPA_PLATFORM='xcb',QT_MEDIA_BACKEND='ffmpeg',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all')
        with (out/'qt.log').open('x') as log:
            qt=subprocess.Popen([shell,'--media-server-test',channel,'--assets',ROOT/'working/game-nocd',
                '--media-movie-count','2','--media-server-timeout','240000','--media-report',history],env=env,stdout=log,stderr=subprocess.STDOUT)
        ready=time.monotonic()+10
        while not channel.exists() or channel.stat().st_size!=2048:
            if qt.poll() is not None or time.monotonic()>ready:
                raise ValueError('Native movie server did not initialize')
            time.sleep(.025)
        run('stage',['python3',ROOT/'tools/run-opengl-game.py','--stream',frame,'--media-channel',channel,'--stage-only'])
        paths=[Path(line.split('Render experiment: ',1)[1]) for line in (out/'stage.log').read_text().splitlines() if line.startswith('Render experiment: ')]
        if len(paths)!=1:
            raise ValueError('Expected one disposable render installation')
        stage=paths[0];metadata=json.loads((stage/'manifest.json').read_text());game=stage/'game'
        if metadata['skip_movies'] or metadata['movie_preference_edits']:
            raise ValueError('Movies were disabled during the enabled scenario')
        for name,key in [('Chaos.exe','staged_sha256'),('MnmRender.dll','dll_sha256')]:
            if sha(game/name)!=metadata[key]:
                raise ValueError('Staged game/bridge changed')
        prefix=out/'wineprefix'
        run('clone-prefix',['cp','-a','--reflink=auto',ROOT/'working/wineprefix-x86_64',prefix])
        env.update(WINEPREFIX=str(prefix),MNM_RENDER_MEDIA='Z:'+str(channel).replace('/','\\'),
                   MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'))
        with (out/'wine.log').open('x') as log:
            wine=subprocess.Popen(['wine','explorer','/desktop=NativeMovieTest,800x600',game/'Chaos.exe'],
                cwd=game,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
        started=time.monotonic()
        while qt.poll() is None:
            response=channel.read_bytes()
            if struct.unpack_from('<I',response,516)[0] in (5,6):
                raise ValueError('Native broker refused an enabled movie request')
            if time.monotonic()-started>250:
                raise ValueError('Enabled intro playback timed out')
            time.sleep(.05)
        if qt.returncode:
            raise ValueError('Native broker did not complete both intros')
        records=json.loads(history.read_text());movies=[r for r in records if r['operation']==1]
        if len(movies)!=2 or any(r['status']!=3 or r.get('result')!=3 or r.get('video_frames',0)<=0 for r in movies):
            raise ValueError('Native intro completion records incomplete')
        if {Path(r['path']).name.lower() for r in movies}!={'intro0.avi','intro1.avi'}:
            raise ValueError('Unexpected enabled movie paths')
        if any(r['status'] in (5,6) for r in records):
            raise ValueError('Media decoder/unsupported fallback encountered')
        with frame.open('rb') as f:
            f.seek(frame_v1.FRAME_COUNT_OFFSET);initial=struct.unpack('<I',f.read(4))[0]
        deadline=time.monotonic()+20
        while time.monotonic()<deadline:
            with frame.open('rb') as f:
                header=f.read(64)
            count=struct.unpack_from('<I',header,40)[0];status=struct.unpack_from('<I',header,36)[0]
            if count>=initial+3 and status==1:
                break
            time.sleep(.05)
        else:
            raise ValueError('Original frame publication did not resume after native movies')
        report.update(records=records,completed_native_intros=2,post_movie_frames=count-initial,
                      original_frame_publication_resumed=True,qt_binary_sha256=sha(shell),
                      stage=str(stage.relative_to(ROOT)),staged_binary_sha256=metadata['staged_sha256'],
                      bridge_binary_sha256=metadata['dll_sha256'],movie_preferences_enabled=True,
                      elapsed_seconds=time.monotonic()-started,success=True)
    except Exception as exc:
        report['error']=str(exc)
    finally:
        for process in (qt,wine):
            if process is not None and process.poll() is None:
                if process is wine:
                    os.killpg(process.pid,signal.SIGTERM)
                else:
                    process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill();process.wait(timeout=5)
        if env and 'WINEPREFIX' in env:
            subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
            subprocess.run(['wineserver','-w'],env=env,check=False,timeout=10)
        if reservation:
            reservation.close()
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['inputs']=inputs;report['inputs_stable']=all(sha(ROOT/n)==h for n,h in inputs.items())
        report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
