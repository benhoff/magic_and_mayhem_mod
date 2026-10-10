"""Application lifecycle for the bounded interactive guarded World batch experiment."""
import json
import os
from pathlib import Path
import signal
import subprocess
import struct
import sys


def producer_refusal(root):
    """Preserve original-side refusals even if the viewer was closed first."""
    if list((root/'capture').glob('world-batch-fault-*.bin')):
        return 'Original destination access guard refused; inspect '+str(root/'capture')
    path=root/'capture/canvas-producers.bin'
    if not path.exists():return None
    if path.stat().st_size>128*1024*1024:return 'Original producer stream exceeded its bound'
    raw=path.read_bytes();at=64
    while len(raw)-at>=96:
        size,sequence,kind=struct.unpack_from('<3I',raw,at)
        if size<96 or size>128*1024*1024:return 'Invalid original producer extent'
        if size>len(raw)-at:break # A user close may interrupt a source write.
        if kind==13:
            reason,detail=struct.unpack_from('<2I',raw,at+56)
            return f'Original producer refused at record {sequence}: reason {reason}, detail {detail:#x}'
        at+=size
    return None


def command(repository, build, prefix_template, manual, claims=None):
    args = [sys.executable, str(repository/'tools/capture-scene-game.py'),
            '--canvas-producers', '--world-producer-handoff', '--samples', '16',
            '--world-raster-prefix', '16', '--world-raster-batch',
            '--producer-oracle-mib', '3072', '--skip-window-screenshot',
            '--map', '2', '--magic-items', '0', '--producer-live', str(build/'mnm-canvas-producers-live'),
            '--prefix-template', str(prefix_template.resolve())]
    if manual: args.append('--manual-input')
    if claims: args += ['--claims', str(claims.resolve())]
    return args


def run_batch(repository, build, prefix_template, manual=True, claims=None):
    print('Guarded native World batching: bounded first 16 queues, 800x600 RGB565.', flush=True)
    if manual:
        print('Use the original window: Single Player > Quick Battle, map 2, zero magic items. '
              'Close either window to cancel; the session ends after queue 16.', flush=True)
    else:
        print('Automatically running the bounded Quick Battle map 2 startup route.', flush=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    root = None
    process = subprocess.Popen(command(repository, build, prefix_template, manual, claims),
                               env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               text=True, start_new_session=True)
    try:
        for line in process.stdout:
            line = line.rstrip('\n')
            candidate = Path(line)
            if line.startswith(str(repository/'working/experiments/scene-observer/run-')) and candidate.is_dir():
                root = candidate
                print('Native World batch session: '+str(root), flush=True)
            else: print(line, flush=True)
        process.wait()
        if process.returncode:
            detail = str(root/'native-producers/live-report.json') if root else 'launcher output'
            if root and (root/'native-producers/live-report.json').exists():
                detail = json.loads((root/'native-producers/live-report.json').read_text()).get('error') or detail
            raise RuntimeError('Native World batch refused: '+detail)
        if root is None: raise RuntimeError('Batch launcher ended before staging')
        result = json.loads((root/'report.json').read_text())
        (root/'native-world.json').write_text(json.dumps(result, indent=2)+'\n')
        if result.get('cancelled'):
            refusal=producer_refusal(root)
            if refusal:
                result.update(cancelled=False,error=refusal)
                (root/'native-world.json').write_text(json.dumps(result,indent=2)+'\n')
                raise RuntimeError(refusal)
            print('Native World batch cancelled before completion; no validation claimed.', flush=True)
            return
        if not result.get('success') or result.get('native_canvas_writebacks') != 16:
            raise RuntimeError('Incomplete native World batch; details: '+str(root/'report.json'))
        profile = result['native_producers']['profile']
        print('Completed 16 guarded World batches. Profiling: '+str(root/'native-producers/live-report.json'), flush=True)
        print('Native consumer totals (ms): CPU composition {:.1f}, checkpoint diagnostics {:.1f}, stream ingest {:.1f}.'.format(
            profile['cpu_composition_ms'], profile['checkpoint_diagnostics_ms'], profile['stream_ingest_ms']), flush=True)
    finally:
        if process.poll() is None:
            process.send_signal(signal.SIGINT)
            try: process.wait(timeout=15)
            except subprocess.TimeoutExpired: process.kill();process.wait(timeout=5)
        process.stdout.close()
