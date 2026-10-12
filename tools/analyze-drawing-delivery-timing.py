#!/usr/bin/env python3
"""Read hash-bound physical/EGL metadata on a shared Linux monotonic clock."""
import argparse
import json
from pathlib import Path
from drawing_api_session import BUILD,sha
from drawing_api_physical import analyze
from drawing_x11_delivery import decode_delivery
from drawing_delivery_timing import analyze_timing
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 reference=json.loads(a.reference.read_text());claims=json.loads(a.claims.read_text());sources=claims['sources']
 if reference.get('success') is not True or reference.get('source_sha256')!=BUILD or reference.get('original_drawing_active') is not True:raise ValueError('Original-active reference required')
 if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
 run=Path(reference['experiment']);capture=run/'capture';peer_bytes=(run/'physical-report.json').read_bytes()
 if sha(peer_bytes)!=reference.get('physical_peer_report_sha256'):raise ValueError('Original physical timing report changed/unbound')
 physical=analyze(capture,run/'physical-report.json')
 if physical!=reference['physical_api_observation']:raise ValueError('Original physical admission changed')
 raw=(capture/'x11-delivery.bin').read_bytes()
 if sha(raw)!=reference['x11_delivery_observation']['sha256']:raise ValueError('Original delivery journal changed')
 result=analyze_timing(json.loads(peer_bytes),decode_delivery(raw,False));stable=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items())
 report=dict(schema=1,success=stable,sources=sources,sources_stable=stable,claims=claims['claims'],reference=dict(path=str(a.reference),sha256=sha(a.reference.read_bytes())),physical_peer_sha256=sha(peer_bytes),delivery_sha256=sha(raw),analysis=result,original_game_executed=False,original_pixels_used_as_native_inputs=False,source_submission_identity_bound=False,coherent_presented_frames=False,complete_journey_coverage=False,scope='Offline hash-bound ordering/overlap of exact root-read spans and reached EGL calls in the same Linux CLOCK_MONOTONIC. No Win tick calibration,submitted-source ownership,scanout,all-route or full-journey proof.')
 a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:result[k] for k in ('physical_frames','egl_swaps','physical_pairs_before_first_egl','pairs_overlapping_egl')}))
if __name__=='__main__':main()
