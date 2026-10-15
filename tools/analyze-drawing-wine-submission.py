#!/usr/bin/env python3
"""Join admitted Wine submission calls to original API/source metadata only."""
import argparse
from collections import Counter
import json
from pathlib import Path
import struct
from drawing_api_session import sha
from drawing_wine_submission import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--reference',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    original=a.reference.read_bytes();reference=json.loads(original)
    if reference.get('success') is not True or reference.get('original_drawing_active') is not True:
        raise ValueError('Admitted original-active reference required')
    capture=Path(reference['experiment'])/'capture'
    calls={r[29]:r for r in struct.iter_unpack('<32I',(capture/'session.bin').read_bytes()[64:-128])}
    raw=(capture/'wine-submission.bin').read_bytes()
    observed=decode(raw,calls,(capture/'wine-lifetime.bin').read_bytes() if raw[:8]==b'MNMWIN02' else None)
    if {k:v for k,v in observed.items() if k!='wire_version'}!={k:v for k,v in reference['wine_submission_observation'].items() if k!='wire_version'}:raise ValueError('Submission journal/reference changed')
    points={f['entry']:f for f in reference['producer_observation']['frames']}
    associations=[]
    for submission in observed['calls']:
        api=calls.get(submission['api_entry']);point=points.get(submission['api_entry'])
        associations.append(dict(submission=submission['call'],api_entry=submission['api_entry'],
            method=submission['method'],thread=submission['thread'],
            api_method=api[2] if api else None,api_source_generation=api[4] if api else None,
            api_destination_generation=api[3] if api else None,
            objects=[submission['first_object'],submission['second_object']],
            flags=submission['flags'],hresult=submission['hresult'],
            api_physical_ordinal=api[12] if api else None,
            producer_live_at_api_points=point['producer_live_at_all_points'] if point else False,
            metadata_point_entry=point['entry'] if point else None,
            source_storage_owned=False,egl_temporal_assignment_performed=False))
    names=['tools/analyze-drawing-wine-submission.py','tools/drawing_wine_submission.py']
    report=dict(schema=1,success=True,sources={n:sha((ROOT/n).read_bytes()) for n in names},
        reference=dict(path=str(a.reference),sha256=sha(original)),associations=associations,
        submissions=len(associations),unbound_submissions=observed['unbound_calls'],
        method_counts=dict(Counter(str(s['method']) for s in associations)),
        api_method_counts=dict(Counter(str(s['api_method']) for s in associations)),
        submissions_with_api_physical_witness=sum(bool(s['api_physical_ordinal']) for s in associations),
        submissions_with_live_api_source_points=sum(s['producer_live_at_api_points'] for s in associations),
        texture_generation_reuse_validated=False,source_storage_owned=False,
        egl_temporal_assignment_performed=False,complete_presented_journey=False,
        scope='Offline exact enclosing API entry/thread and optional API source-point/witness association of admitted Wine submission rows. Opaque texture identities are not lifetime generations; no texture bytes, storage ownership or asynchronous EGL frame assignment.')
    a.report.parent.mkdir(parents=True,exist_ok=True)
    a.report.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:report[k] for k in ('submissions','unbound_submissions','api_method_counts','submissions_with_api_physical_witness','submissions_with_live_api_source_points')}))


if __name__=='__main__':main()
