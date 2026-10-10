#!/usr/bin/env python3
"""Public batch launcher execution and timing assertions; declare exact scope before launch."""
import argparse
import ctypes
import ctypes.util
import hashlib
import json
import os
from pathlib import Path
import signal
import statistics
import subprocess
import tempfile
import time
from coverage_claims import behavior_contract, scenario_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]


def close_native_window():
    """Exercise Qt's real window-close event without requiring a window manager."""
    x=ctypes.CDLL(ctypes.util.find_library('X11'))
    window=ctypes.c_ulong;pointer=ctypes.c_void_p
    x.XOpenDisplay.argtypes=[ctypes.c_char_p];x.XOpenDisplay.restype=pointer
    x.XDefaultRootWindow.argtypes=[pointer];x.XDefaultRootWindow.restype=window
    x.XQueryTree.argtypes=[pointer,window,ctypes.POINTER(window),ctypes.POINTER(window),ctypes.POINTER(ctypes.POINTER(window)),ctypes.POINTER(ctypes.c_uint)]
    x.XFetchName.argtypes=[pointer,window,ctypes.POINTER(pointer)]
    x.XFree.argtypes=[pointer]
    x.XInternAtom.argtypes=[pointer,ctypes.c_char_p,ctypes.c_int];x.XInternAtom.restype=window
    class Message(ctypes.Structure):
        _fields_=[('type',ctypes.c_int),('serial',window),('send_event',ctypes.c_int),('display',pointer),('window',window),('message_type',window),('format',ctypes.c_int),('data',ctypes.c_long*5)]
    class Event(ctypes.Union):
        _fields_=[('message',Message),('padding',ctypes.c_long*24)]
    x.XSendEvent.argtypes=[pointer,window,ctypes.c_int,ctypes.c_long,ctypes.POINTER(Event)]
    x.XSync.argtypes=[pointer,ctypes.c_int];x.XCloseDisplay.argtypes=[pointer]
    display=x.XOpenDisplay(None)
    if not display:raise RuntimeError('Cannot open test X11 display')
    try:
        pending=[x.XDefaultRootWindow(display)]
        while pending:
            w=pending.pop();name=pointer()
            if x.XFetchName(display,w,ctypes.byref(name)) and name.value:
                title=ctypes.string_at(name);x.XFree(name)
                if title==b'Magic & Mayhem native canvas sequence':
                    event=Event();m=event.message;m.type=33;m.display=display;m.window=w;m.format=32
                    m.message_type=x.XInternAtom(display,b'WM_PROTOCOLS',0);m.data[0]=x.XInternAtom(display,b'WM_DELETE_WINDOW',0)
                    if not x.XSendEvent(display,w,0,0,ctypes.byref(event)):raise RuntimeError('Window close request failed')
                    x.XSync(display,0);return True
            root=window();parent=window();children=ctypes.POINTER(window)();count=ctypes.c_uint()
            if x.XQueryTree(display,w,ctypes.byref(root),ctypes.byref(parent),ctypes.byref(children),ctypes.byref(count)):
                pending.extend(children[i] for i in range(count.value))
                if children:x.XFree(children)
        return False
    finally:x.XCloseDisplay(display)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--mode',choices=['complete','cancel'],default='complete')
    p.add_argument('--queues',type=int,choices=(16,32),default=16)
    a=p.parse_args()
    if a.queues!=16 and a.mode!='complete':p.error('Extended prefix is complete-mode only')
    parent=ROOT/'working/tests/native-world-batch';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    ids=['NR.world-interactive-batch-launcher','NR.world-batch-profiling']
    scenario_id='world-raster-batch-prefix32-20261010' if a.queues==32 else 'world-batch-public-'+a.mode+'-20261009'
    if a.queues==32:ids+=['NR.world-batch-extended-prefix','NR.world-cpu-composition-reuse','NR.world-batch-throughput']
    scenario=next(s for s in register['scenarios'] if s['id']==scenario_id)
    if a.mode=='complete':ids+=['RS.world-raster-batch-return','NR.world-raster-batch-transport','NR.world-resource-reuse']
    selected=[b for b in register['behaviors'] if b['id'] in ids]
    builds={b['id']:b for b in register['builds']}
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in sorted(set().union(*(required_sources(b) for b in selected)))}
    claims=[]
    for b in selected:
        scenarios={scenario_id:scenario_contract(scenario)} if b['id'] in scenario['behaviors'] else {}
        if b['id'] in ('RS.world-raster-batch-return','NR.world-raster-batch-transport'):
            s=next(s for s in register['scenarios'] if s['id']=='world-raster-batch-startup-20261008')
            scenarios[s['id']]=scenario_contract(s)
        if b['id']=='NR.world-resource-reuse':
            s=next(s for s in register['scenarios'] if s['id']=='world-resource-reuse-live-20261009')
            scenarios[s['id']]=scenario_contract(s)
        claims.append(dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios=scenarios))
    declaration=run/'claims.json';declaration.write_text(json.dumps(dict(sources=sources,claims=claims),indent=2)+'\n')
    command=[str(ROOT/'tools/run-native-world.py'),'--world-batch','--world-queues',str(a.queues),'--claims',str(declaration)]
    if a.mode=='complete':command.append('--startup-history')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    # Keep software-GPU scheduling reproducible on the measured host; preserve
    # an explicit caller setting so worker-count regressions can be exercised.
    env.setdefault('LP_NUM_THREADS','8')
    with (run/'launcher.log').open('x') as log:
        process=subprocess.Popen(command,env=env,stdout=log,stderr=subprocess.STDOUT)
        experiment=None;result=None;closed=False;deadline=time.monotonic()+600
        try:
            while process.poll() is None:
                rows=[l for l in (run/'launcher.log').read_text().splitlines() if l.startswith('Native World batch session: ')]
                if rows:experiment=Path(rows[-1].split(': ',1)[1])
                if a.mode=='cancel' and experiment and not closed and (experiment/'native-producers/ready').exists() and (experiment/'channel.bin').exists():
                    # Send the same WM_DELETE_WINDOW request used by a desktop close button.
                    closed=close_native_window()
                if time.monotonic()>deadline:raise RuntimeError('Public batch launcher deadline')
                time.sleep(.2)
            if process.returncode:raise RuntimeError('Public batch launcher failed; inspect '+str(run/'launcher.log'))
            result=json.loads((experiment/'native-world.json').read_text())
            if a.mode=='cancel':assert closed and result['cancelled'] and not result['success']
            else:
                assert result['success'] and result['canvas_guard_verified'] and result['native_canvas_writebacks']==a.queues
                n=result['native_producers'];assert n['world_readbacks']==a.queues
                if a.queues==32:
                    assert n['wire_version']==3 and 0<n['owned_payload_sources']<=2048
                    assert 0<n['owned_payload_bytes']<=16*1024*1024 and n['stream_bytes']<=128*1024*1024
                assert [f['queue'] for f in n['world_frames']]==list(range(1,a.queues+1))
                for f in n['world_frames']:
                    assert f['profile']['visible_draws']>0
                    assert all(v>=0 for v in f['profile'].values())
                assert sum(f['identities']['hits'] for f in n['world_frames'])>0
                assert all(f['identities']['frames']<=4096 and f['identities']['bytes']<=16*1024*1024 for f in n['world_frames'])
                assert sum(f['reuse']['visual_reuses'] for f in n['world_frames'])>0
                assert sum(f['reuse']['cache_hits'] for f in n['world_frames'])>0
                assert all(c['mismatches']==0 for c in result['producer_comparison'])
                if a.queues==32:
                    warm=[f['profile'] for f in n['world_frames'][1:]]
                    native=statistics.median(sum(f[k] for k in ('cpu_composition_ms','history_adopt_ms','resource_prepare_ms','gpu_submit_ms','gpu_readback_ms','gpu_compare_ms')) for f in warm)
                    diagnostic=statistics.median(f['consumer_entry_to_return_ms'] for f in warm)
                    result['performance_budget']=dict(native_work_warm_median_ms=native,native_work_limit_ms=50,diagnostic_queue_warm_median_ms=diagnostic,diagnostic_queue_limit_ms=100,passed=native<=50 and diagnostic<=100)
                    assert result['performance_budget']['passed'], 'Guarded native renderer exceeded its declared warm performance budget'
            assert all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
            report=dict(success=True,mode=a.mode,experiment=str(experiment),sources=sources,claims=claims,sources_stable=True,
                        software_mesa_threads=env['LP_NUM_THREADS'],
                        original_manifest_verified_before_after=result['original_manifest_verified_before_after'],
                        source_executable_sha256=json.loads((experiment/'manifest.json').read_text())['source_sha256'],completed=result)
            (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
        except Exception as error:
            report=dict(success=False,error=str(error),mode=a.mode,experiment=str(experiment),sources=sources,claims=claims,
                        software_mesa_threads=env['LP_NUM_THREADS'],
                        sources_stable=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items()),completed=result)
            (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
            raise
        finally:
            if process.poll() is None:
                process.send_signal(signal.SIGINT)
                try:process.wait(timeout=20)
                except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)


if __name__=='__main__':main()
