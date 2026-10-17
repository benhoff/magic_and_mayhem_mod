#!/usr/bin/env python3
"""Revalidate retained raw observations and account for the first120updates."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_api_session import BUILD,inspect,sha
from drawing_api_physical import analyze as api_physical
from drawing_engine_windows import analyze as engine_windows
from drawing_update_routes import analyze as routes
from drawing_update_timing import decode as timing
from drawing_source_callback_windows import analyze as source_windows
from drawing_producer_updates import analyze as producer_updates
from drawing_producer_points import inspect_points
from drawing_update_accounting import build

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('original','egl','claims','report'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    declaration=json.loads(a.claims.read_bytes());sources=declaration['sources']
    def stable():return all(sha((ROOT/name).read_bytes())==digest for name,digest in sources.items())
    if not stable():raise ValueError('Prospective accounting sources changed')
    report=dict(schema=1,success=False,analysis_completed=False,sources=sources,claims=declaration['claims'],original_game_executed=False,
                scope='Static raw accepted-update/QPC/source-prefix/API/source-worker-EGL accounting. First120advancing windows retained without excluding queue-free updates or straddlers. Complete observed accounting and complete presented/owned acquisition are independent gates; no new engine execution, original allocation ownership, scanout, semantic outcome or native parity.')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,check=True)
    try:
        original_bytes=a.original.read_bytes();original=json.loads(original_bytes);egl_bytes=a.egl.read_bytes();egl=json.loads(egl_bytes)
        if original.get('source_sha256')!=BUILD or not all(original.get(k) is True for k in ('success','sources_stable','normal_original_quit_verified','original_manifest_verified_before_after','original_sections_unchanged','original_drawing_active')):raise ValueError('Completed original acquisition required')
        if egl.get('success') is not True or egl.get('sources_stable') is not True or egl.get('original_report_sha256')!=sha(original_bytes):raise ValueError('Pixel analysis original-report binding differs')
        if any(sha((ROOT/name).read_bytes())!=digest for name,digest in egl['sources'].items()):raise ValueError('Pixel analysis sources changed')
        compiled=['runtime/engine-cadence/observer.c','runtime/engine-cadence/entry.S','runtime/drawing/update_timing.h','runtime/drawing/session.c','runtime/scene/whole_update_capture.h','runtime/scene/canvas_producers.c']
        for name in compiled:
            if original['sources'].get(name)!=sha((ROOT/name).read_bytes()):raise ValueError('Acquisition observer changed: '+name)
        root=Path(original['experiment']);capture=root/'capture';peer=root/'physical-report.json'
        if sha(peer.read_bytes())!=original['physical_peer_report_sha256']:raise ValueError('API physical peer changed')
        observed={'observation':inspect(capture),'physical_api_observation':api_physical(capture,peer),
                  'engine_update_observation':engine_windows(capture/'engine-cadence.bin'),'update_route_observation':routes(capture/'engine-cadence.bin'),
                  'update_call_timing_observation':timing((capture/'update-call-timing.bin').read_bytes(),(capture/'session.bin').read_bytes()),
                  'source_update_window_observation':source_windows(capture),'producer_update_observation':producer_updates(capture),'producer_observation':inspect_points(capture)}
        for name,value in observed.items():
            if value!=original[name]:raise ValueError('Raw acquisition observation changed: '+name)
        spec=importlib.util.spec_from_file_location('accounting_producers',ROOT/'tools/inspect-canvas-producers.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        _,records=module.decode((capture/'canvas-producers.bin').read_bytes())
        result=build(observed['update_route_observation'],observed['update_call_timing_observation'],observed['source_update_window_observation'],
                     observed['producer_update_observation'],records,observed['physical_api_observation']['frames'],egl,observed['producer_observation']['frames'])
        report.update(result,analysis_completed=True,original_report=str(a.original),original_report_sha256=sha(original_bytes),egl_report=str(a.egl),egl_report_sha256=sha(egl_bytes),
                      acquisition_observers_match_captured_sources=True,raw_observations_match_original_report=True)
    except (OSError,ValueError,KeyError) as error:report.update(success=False,error=str(error))
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],cwd=ROOT,check=True)
        report['sources_stable']=stable();report['success']=bool(report['success'] and report['sources_stable'])
        with a.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
    print(json.dumps({k:report[k] for k in ('success','gates','source_queues','input_calls','physical_primary_witnesses','egl_witnesses') if k in report}))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
