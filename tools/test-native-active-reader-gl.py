#!/usr/bin/env python3
"""Replay the actual PE32 active-reader FIFO through the native OpenGL consumer."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--pe-report',type=Path,required=True)
    args=parser.parse_args();declaration=json.loads(args.claims.read_text());pe=json.loads(args.pe_report.read_text())
    if not pe['success'] or not pe['sources_stable']:raise ValueError('Passing source-stable PE32 input required')
    if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise ValueError('Prospective GL source changed')
    if any(sha(ROOT/p)!=h for p,h in pe['sources'].items()):raise ValueError('PE32 source input is stale')
    archive=args.pe_report.resolve().parent/'ordered-active-readers/capture/session-00000001.bin'
    parent=ROOT/'working/tests/native-active-reader-gl';parent.mkdir(exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(success=False,sources=declaration['sources'],claims=declaration['claims'],recorded_inputs={str(args.pe_report):sha(args.pe_report),str(archive):sha(archive)},scope=__doc__,live_game_executed=False,complete_replacement=False)
    build=ROOT/'working/build/native-active-reader-gl';server=None
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',QT_QPA_PLATFORM='xcb')
    def run(label,command):
        with (out/(label+'.log')).open('x') as log:subprocess.run([str(x) for x in command],env=env,cwd=ROOT,stdout=log,stderr=log,timeout=300,check=True)
    try:
        run('configure',['cmake','-S',ROOT/'tests/native-active-reader-gl','-B',build,'-DCMAKE_BUILD_TYPE=Debug','-DBUILD_TESTING=OFF','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'])
        run('build',['cmake','--build',build,'--target','native-active-reader-gl','-j4'])
        closure={'tests/native-active-reader-gl/CMakeLists.txt','renderer/CMakeLists.txt'}
        for dep in build.rglob('*.o.d'):
            for token in dep.read_text().replace('\\\n',' ').split():
                p=Path(token).resolve()
                if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):closure.add(str(p.relative_to(ROOT)))
        if closure-set(declaration['sources']):raise ValueError('Undeclared native GL compiler input: '+repr(sorted(closure-set(declaration['sources']))))
        report['compiled_sources']={p:sha(ROOT/p) for p in sorted(closure)}
        with (out/'display').open('w+') as display,(out/'xvfb.log').open('x') as log:
            server=subprocess.Popen(['Xvfb','-displayfd',str(display.fileno()),'-screen','0','1280x1024x24','-nolisten','tcp'],pass_fds=(display.fileno(),),stdout=log,stderr=log,start_new_session=True)
            deadline=time.monotonic()+10
            while time.monotonic()<deadline:
                display.seek(0);number=display.read().strip()
                if number:env['DISPLAY']=':'+number;break
                if server.poll() is not None:raise RuntimeError('Private display exited')
                time.sleep(.1)
            else:raise TimeoutError('Private display startup')
        binary=build/'native-active-reader-gl';run('native-gl',[binary,archive]);report['native_gl']=json.loads((out/'native-gl.log').read_text().splitlines()[-1]);report['binary_sha256']=sha(binary)
        report['success']=report['native_gl']['success']
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if server and server.poll() is None:os.killpg(server.pid,signal.SIGTERM);server.wait(timeout=10)
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in declaration['sources'].items());report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out/'report.json',flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
