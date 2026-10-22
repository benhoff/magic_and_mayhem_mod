"""Bind full physical witnesses to actual API journal rows; preserve partial scope."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import json
import struct
from pathlib import Path
from drawing_api_session import PRIMARY_METHODS, inspect, sha


def analyze(directory, report_path):
    directory = Path(directory)
    journal = evidence_bytes(directory / 'session.bin')
    observed = inspect(directory)
    peer = json.loads(evidence_text(Path(report_path)))
    if (not observed['physical_fence_enabled'] or peer.get('success') is not True or
            peer.get('original_draw_thread_fenced') is not True or
            peer.get('native_rendering') is not False or
            peer.get('all_global_presentation_routes') is not False or
            peer.get('source_inputs_bound') is not False or
            peer.get('boundary') != 'observed_primary_api_physical'):
        raise ValueError('Invalid API physical peer scope')
    frames = observed['frames']
    if not frames or len(peer['frames']) != len(frames):
        raise ValueError('Incomplete physical API witnesses')
    for pattern in ('api-physical-request-*.bin', 'api-physical-ack-*.bin', 'api-physical-oracle-*.rgba'):
        if len(list(directory.glob(pattern))) != len(frames):
            raise ValueError('Missing/extra physical API artifacts')
    transients = peer.get('transient_samples', [])
    if peer.get('all_physical_changes_captured', False) is not False:
        raise ValueError('Sampled settling cannot assert all physical changes')
    if not isinstance(transients, list) or len(transients) > 32768:
        raise ValueError('Invalid physical settling samples')
    transient_budget=peer.get('transient_byte_budget',256*1024*1024)
    if type(transient_budget) not in (int,float) or transient_budget!=int(transient_budget) or not 1024**2<=transient_budget<=2048*1024**2 or transient_budget%1024**2:raise ValueError('Invalid transient budget')
    transient_bytes = 0
    seen = set()
    for sample in transients:
        ordinal, index = sample['ordinal'], sample['sample']
        if type(ordinal) is not int or type(index) is not int or not 1 <= ordinal <= len(frames) or not 1 <= index <= 32:
            raise ValueError('Invalid transient identity')
        name = f'api-physical-transient-{ordinal:06d}-{index:02d}.rgba'
        frame = frames[ordinal-1]
        if name in seen or sample['path'] != name or sample['width'] != frame['width'] or sample['height'] != frame['height']:
            raise ValueError('Transient identity or extent changed')
        plane = evidence_bytes(directory / name)
        if len(plane) != frame['width']*frame['height']*4 or sha(plane) != sample['sha256']:
            raise ValueError('Incomplete or changed transient plane')
        transient_bytes += len(plane)
        seen.add(name)
    if (len(list(directory.glob('api-physical-transient-*.rgba'))) != len(seen) or
            transient_bytes > transient_budget or peer.get('transient_bytes', 0) != transient_bytes):
        raise ValueError('Transient artifact/byte accounting changed')
    result = []
    for frame, metadata in zip(frames, peer['frames']):
        ordinal = frame['ordinal']
        request = evidence_bytes(directory / f'api-physical-request-{ordinal:06d}.bin')
        start = 64 + (frame['record'] - 1) * 128
        if len(request) != 128 or request != journal[start:start+128]:
            raise ValueError('Physical API request differs from original journal')
        row = struct.unpack('<32I', request)
        if row[2] not in PRIMARY_METHODS or row[12] != ordinal:
            raise ValueError('Physical API route/ordinal changed')
        ack = evidence_bytes(directory / f'api-physical-ack-{ordinal:06d}.bin')
        if ack != b'MNMAPI01' + struct.pack('<II', ordinal, row[0]):
            raise ValueError('Physical API acknowledgement changed')
        name = f'api-physical-oracle-{ordinal:06d}.rgba'
        physical = evidence_bytes(directory / name)
        identity = dict(ordinal=ordinal, record=row[0], entry=row[29], method=row[2],
                        object=row[3], caller=row[5], thread=row[6], width=row[14], height=row[15])
        if (any(type(metadata.get(k)) is not int or metadata[k] != v for k, v in identity.items()) or
                metadata.get('path') != name or metadata.get('stable_double_capture') is not True or
                metadata.get('request_sha256') != sha(request) or metadata.get('sha256') != sha(physical)):
            raise ValueError('Physical API identity/hash/stability changed')
        samples = [s for s in transients if s['ordinal'] == ordinal]
        if [s['sample'] for s in samples] != list(range(1, len(samples)+1)) or metadata.get('settling_samples', []) != samples:
            raise ValueError('Transient sample order/frame binding changed')
        result.append(dict(**identity, path=name, sha256=sha(physical), settling_samples=samples))
    return dict(success=True, boundary='observed_primary_api_physical', frames=result,
                pixels=sum(f['width'] * f['height'] for f in result),
                transient_samples=transients, transient_bytes=transient_bytes, transient_byte_budget=int(transient_budget), all_physical_changes_captured=False,
                driver_readbacks_enabled=False, all_global_presentation_routes_observed=False,
                source_inputs_bound=False, complete_presented_journey=False,
                semantic_input_outcomes_complete=False, original_pixels_used_as_native_inputs=False)
