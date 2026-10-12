#!/usr/bin/env python3
"""Report exact retained native-content correspondence without assigning frames."""
import argparse
import json
from pathlib import Path
from drawing_api_session import sha
from drawing_egl_physical import analyze_egl_physical
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--prediction',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 r=json.loads(a.reference.read_text());prediction=json.loads(a.prediction.read_text());claims=json.loads(a.claims.read_text());sources=claims['sources']
 if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective source changed')
 if r.get('success') is not True or prediction.get('native_exit')!=0 or prediction['reference']['sha256']!=sha(a.reference.read_bytes()):raise ValueError('Retained original/native reference mismatch')
 run=Path(r['experiment']);peer=run/'egl-physical-report.json'
 if sha(peer.read_bytes())!=r['egl_physical_peer_report_sha256']:raise ValueError('EGL peer evidence changed')
 observed=analyze_egl_physical(run/'capture',peer)
 if observed!=r['egl_physical_observation']:raise ValueError('Retained EGL admission changed')
 lookup={}
 for f in observed['frames']:lookup.setdefault(f['sha256'],[]).append(f['ordinal'])
 findings=[dict(api_ordinal=c['ordinal'],source_sequence=c['boundary']['sequence'],native_sha256=c['native_sha256'],egl_ordinals=lookup.get(c['native_sha256'],[]),source_submission_identity_bound=False) for c in prediction['comparisons']]
 stable=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items())
 report=dict(schema=1,success=stable,sources=sources,sources_stable=stable,claims=claims['claims'],reference=dict(path=str(a.reference),sha256=sha(a.reference.read_bytes())),prediction=dict(path=str(a.prediction),sha256=sha(a.prediction.read_bytes())),findings=findings,native_images=len(findings),native_images_present_among_egl_witnesses=sum(bool(f['egl_ordinals']) for f in findings),egl_witnesses=len(observed['frames']),original_game_executed=False,native_reexecuted=False,original_pixels_used_as_native_inputs=False,temporal_frame_assignment_performed=False,source_submission_identity_bound=False,complete_native_parity=False,complete_journey_coverage=False,scope='Offline exact full-plane hash correspondence between retained independent source-only native predictions and admitted post-EGL root witnesses. Retain every duplicate/missing correspondence; no image is selected or relabelled as its temporal frame,submitted source,complete parity or scanout. Original output remains comparison-only.')
 a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ('success','native_images','native_images_present_among_egl_witnesses','egl_witnesses')}))
if __name__=='__main__':main()
