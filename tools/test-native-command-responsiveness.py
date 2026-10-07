#!/usr/bin/env python3
"""Check bounded command repaint coalescing, input delivery and actual shell drain.

Run under an isolated Xvfb. Uses synthetic producers only; no original game,
Wine hooks or user-hardware latency/equivalence claim.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('launch_fixture', ROOT/'tools/test-native-command-launch.py')
launch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(launch)
SOURCES = sorted(set(launch.SOURCES + [
    'tools/test-native-command-responsiveness.py', 'tests/live-render-channel-test.cpp',
    'tests/qt-input-test.cpp', 'tools/test-native-command-fallback.py',
    'renderer/surface_backend.cpp', 'renderer/surface_backend.hpp',
    'renderer/surface_copy.cpp', 'renderer/surface_copy.hpp',
    'renderer/dib.cpp', 'renderer/dib.hpp',
]))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    args = parser.parse_args()
    build = args.build.resolve()
    parent = ROOT/'working/tests/native-command-responsiveness'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    hashes = {p: sha(ROOT/p) for p in SOURCES}
    binaries = ['mnm-qt-shell', 'live-render-channel-test', 'qt-input-test']
    paths = {p: build/p if (build/p).is_file() else build/'renderer'/p for p in binaries}
    artifacts = {p: sha(paths[p]) for p in binaries}
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', QT_SCALE_FACTOR='1')
    report = dict(schema=1, success=False,
                  scope='Synthetic Xvfb/Mesa: bounded FIFO command execution with queued presentation and no synchronous reader repaints, event delivery between polls, latest retained full pixels and actual Qt shell final-frame drain beyond 4096 commands. Targeted input and recovery regressions. No original execution, user hardware latency, producer contention repair or live replacement claim.',
                  source_sha256=hashes, artifacts_sha256=artifacts, runs=[])
    try:
        for name in binaries[1:]:
            result = subprocess.run([str(paths[name])], env=env, capture_output=True, text=True, timeout=60)
            (run/(name+'.stdout')).write_text(result.stdout)
            (run/(name+'.stderr')).write_text(result.stderr)
            assert result.returncode==0, (name, result.returncode, result.stderr)
            if name=='live-render-channel-test':
                channel = json.loads(result.stdout)
                assert channel['burst_presents']==70 and channel['burst_polls']==3
                assert channel['burst_paints']==0
                assert channel['input_between_polls'] and channel['ordinary_readbacks']==channel['viewport_uploads']==0
                report['channel']=channel
            report['runs'].append(dict(name=name, returncode=result.returncode,
                                       stdout_sha256=sha(run/(name+'.stdout')), stderr_sha256=sha(run/(name+'.stderr'))))
        result=subprocess.run(['python3',str(ROOT/'tools/test-native-command-fallback.py'),str(build)],
                              env=env, capture_output=True, text=True, timeout=60)
        (run/'fallback.stdout').write_text(result.stdout);(run/'fallback.stderr').write_text(result.stderr)
        assert result.returncode==0, ('fallback',result.returncode,result.stderr,result.stdout)
        fallback=json.loads(Path(json.loads(result.stdout)['report']).read_text())
        assert fallback['success']
        report['fallback']=fallback
        # Every earlier synthetic frame is black. Only the last of2100 updates
        # is colored, so a visible sample proves completion of the whole burst,
        # not merely the first healthy frame or an early owned-copy ACK.
        old='bytes([127,67,31])*12'
        assert launch.PRODUCER.count(old)==1
        launch.PRODUCER=launch.PRODUCER.replace(old, '(bytes([127,67,31])*12 if i==2099 else bytes(36))')
        for name, scale, fullscreen in [('normal','1',False), ('fullscreen-hidpi','1.5',True)]:
            start=time.monotonic()
            result=launch.check_case(build/'mnm-qt-shell', run/name, None, scale, fullscreen)
            result['elapsed_including_three_second_retention_check']=time.monotonic()-start
            report['runs'].append(dict(name=name, **result))
        assert hashes=={p:sha(ROOT/p) for p in SOURCES}, 'Sources changed during validation'
        assert artifacts=={p:sha(paths[p]) for p in binaries}, 'Binaries changed during validation'
        report.update(success=True, native_integration=dict(all_match=True, burst_presents=70,
                      burst_synchronous_paint_events=0, burst_polls=3, input_between_polls=True,
                      latest_frame_pixels=True, bounded_fifo_and_terminal_cleanup=True,
                      actual_shell_final_frame_beyond_4096=True,
                      windowed_fullscreen_hidpi=True, targeted_input=True, recovery=True,
                      ordinary_readbacks=0, viewport_uploads=0))
    except Exception as error:
        report['error']=str(error)
    path=run/'report.json';path.write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(dict(success=report['success'], report=str(path))))
    if not report['success']:
        raise SystemExit(1)


if __name__=='__main__':
    main()
