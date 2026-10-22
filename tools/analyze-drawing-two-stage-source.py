#!/usr/bin/env python3
"""Hash-bound reanalysis of protected two-blit source chains and native parity."""
import argparse
import json
from pathlib import Path
from coverage_evidence import read_bytes, write_report
import subprocess
from drawing_api_session import BUILD,sha
from drawing_storage_transitions import decode as decode_storage
from drawing_two_stage_source import bind
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--prediction',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    raw=read_bytes(a.reference, root=ROOT);native_raw=read_bytes(a.prediction, root=ROOT);reference=json.loads(raw);prediction=json.loads(native_raw)
    if reference.get('source_sha256')!=BUILD or reference.get('original_drawing_active') is not True or reference.get('original_manifest_verified_before_after') is not True or prediction.get('original_pixels_used_as_native_inputs') is not False or prediction['reference']['sha256']!=sha(raw):
        raise ValueError('Independent original/native provenance differs')
    capture=(Path(reference['experiment'])/'capture').resolve()
    if not capture.is_relative_to(ROOT/'working'):raise ValueError('Capture outside worktree')
    journals={'wine-submission.bin':'wine_submission_observation','wine-blit-packet.bin':'wine_blit_packet_observation','wine-blit-worker.bin':'wine_blit_worker_observation','wine-retained-map-source.bin':'wine_retained_map_source_observation','api-source-worker-lease.bin':'api_source_worker_lease_observation','source-held-snapshot.bin':'source_held_snapshot_observation','canvas-storage-transitions-0000.bin':'canvas_storage_transition_observation'}
    hashes={}
    for name,key in journals.items():
        data=(capture/name).read_bytes();hashes[name]=sha(data)
        if hashes[name]!=reference[key]['journal_sha256']:raise ValueError('Retained journal changed:'+name)
        if key=='canvas_storage_transition_observation':
            current=decode_storage(data)
            if any(current[k]!=reference[key][k] for k in ('calls','returns','closed','queries','events')):raise ValueError('Storage decoder/reference mismatch')
    result=bind(reference,prediction)
    if not result['accepted_frames']:raise ValueError('No fully linked two-stage frames')
    controls=subprocess.run(['python3',str(ROOT/'tests/test-drawing-two-stage-source.py')],capture_output=True,text=True,check=True)
    names=['tools/analyze-drawing-two-stage-source.py','tools/drawing_two_stage_source.py','tests/test-drawing-two-stage-source.py','tools/drawing_storage_transitions.py','tools/drawing_storage_point.py']
    report=dict(schema=1,success=True,sources={n:sha((ROOT/n).read_bytes()) for n in names},reference=dict(path=str(a.reference),sha256=sha(raw)),prediction=dict(path=str(a.prediction),sha256=sha(native_raw)),journal_sha256=hashes,analysis=result,guards=controls.stderr,original_game_executed=False,original_pixels_used_as_native_inputs=False,scope='Current guarded offline linkage of retained exact original protected source epochs/bytes through initial/intermediate/final two-blit commands and ordered checked worker returns to independently matched EGL/native frames. No new original execution, all-route/unobserved upload/allocation alias or scanout ownership proof.')
    write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=True,accepted=result['accepted_frames'],excluded=result['excluded_frames'])))


if __name__=='__main__':main()
