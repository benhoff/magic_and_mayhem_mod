"""Admit original post-copy physical witnesses without widening route coverage."""
import json
from pathlib import Path
import struct
from drawing_presented_session import sha,rgba565

def admit_witness(request,ack,physical,oracle,metadata):
    if len(request)!=96:raise ValueError('Incomplete physical fence request')
    r=struct.unpack('<24I',request)
    if not r[0] or r[1] not in (1,2) or r[2]&0x80000000 or r[8]!=1 or not r[19] or not r[22] or r[14:18]!=(16,0xf800,0x7e0,31):
        raise ValueError('Physical fence is not a successful guarded primary copy')
    if ack!=b'MNMPACK1'+struct.pack('<II',r[19],r[0]):raise ValueError('Physical acknowledgement identity changed')
    if not 1<=r[12]<=2048 or not 1<=r[13]<=2048 or len(oracle)!=r[12]*r[13]*2 or len(physical)!=r[12]*r[13]*4:
        raise ValueError('Physical reference is not the entire plane')
    expected=dict(ordinal=r[19],attempt=r[0],width=r[12],height=r[13],source_sequence=r[4],queue=r[5],canvas=r[6],thread=r[22],action_record=r[23])
    if any(type(metadata.get(k)) is not int or metadata[k]!=v for k,v in expected.items()) or metadata.get('stable_double_capture') is not True:
        raise ValueError('Physical fence source/thread/action identity changed')
    if metadata.get('request_sha256')!=sha(request) or metadata.get('sha256')!=sha(physical):raise ValueError('Physical reference hash changed')
    if physical!=rgba565(oracle,'normalized-nearest'):raise ValueError('Original physical pixels differ from the fenced primary readback')
    return dict(**expected,pixels=r[12]*r[13],sha256=sha(physical),stable_double_capture=True)

def analyze(directory,peer_report,publication):
    directory=Path(directory);report=json.loads(Path(peer_report).read_text());frames=publication['frames']
    if report.get('success') is not True or report.get('original_draw_thread_fenced') is not True or report.get('native_rendering') is not False or report.get('all_global_presentation_routes') is not False:
        raise ValueError('Original physical peer did not finish its declared scope')
    if not frames or len(report['frames'])!=len(frames):raise ValueError('Incomplete physical witness sequence')
    trace=(directory/'drawing-publication-0000.bin').read_bytes();rows={r[19]:trace[32+i*96:32+(i+1)*96] for i,r in enumerate(struct.iter_unpack('<24I',trace[32:])) if r[19]}
    if len(list(directory.glob('physical-request-*.bin')))!=len(frames) or len(list(directory.glob('physical-ack-*.bin')))!=len(frames) or len(list(directory.glob('physical-oracle-*.rgba')))!=len(frames):
        raise ValueError('Missing or extra physical witness artifacts')
    results=[]
    for index,(frame,metadata) in enumerate(zip(frames,report['frames']),1):
        request=(directory/f'physical-request-{index:04}.bin').read_bytes()
        if request!=rows.get(index) or frame['ordinal']!=index or metadata['path']!=f'physical-oracle-{index:04}.rgba':
            raise ValueError('Physical reference does not match the exact original call record')
        results.append(admit_witness(request,(directory/f'physical-ack-{index:04}.bin').read_bytes(),
            (directory/metadata['path']).read_bytes(),(directory/frame['path']).read_bytes(),metadata))
    return dict(success=True,boundary='fenced_opaque_primary_point',frames=results,pixels=sum(f['pixels'] for f in results),
        original_draw_thread_fenced=True,all_global_presentation_routes_observed=False,entire_original_process_frozen=False,
        complete_presented_journey=False,original_pixels_used_as_native_inputs=False,peer_report_sha256=sha(Path(peer_report).read_bytes()))
