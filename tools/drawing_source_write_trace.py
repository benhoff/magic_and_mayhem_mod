"""Strict protected-lease writer and canvas-cache metadata; no ownership claim."""
import struct
from pathlib import Path
from drawing_api_session import sha


def snapshot(data):
    if len(data) < 128 or (len(data)-64) % 64:
        raise ValueError('Write trace envelope')
    h = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMSWT01' or h[2:5] != (1,64,0x40209ca7) or not all(h[5:8]) or h[10] >= 4096 or h[11] != 960000 or h[12] != ((h[10]+960000+4095)//4096)*4096 or not 1 <= h[13] <= 128 or h[14:] != (128,0) or len(data) != 64+h[13]*64:
        raise ValueError('Write trace header')
    rows = []; ids = set()
    for ordinal, r in enumerate(struct.iter_unpack('<16I', data[64:]), 1):
        if r[0] != ordinal or r[1] != 1 or not r[2] or r[2] in ids or r[3] & ~255 or r[4] not in (0,1) or r[15]:
            raise ValueError('Write trace canvas identity/reserve')
        ids.add(r[2]); flags = r[3]
        start = r[5] if r[4] else -r[5]
        valid_shape = bool(flags & 128 and 1 <= r[6] <= 2048 and 1 <= r[7] <= 2048 and r[6] <= r[8] <= 4096)
        span = ((r[7]-1)*r[8]+r[6])*2 if valid_shape else 0
        # Absolute-address overflow may make an otherwise valid shape unknown.
        if bool(flags & 32) and (not valid_shape or span != r[9]):
            raise ValueError('Write trace canvas extent')
        if not flags & 32 and r[9] or not flags & 128 and (r[4] or r[5] or flags & 126):
            raise ValueError('Write trace unknown canvas')
        if bool(flags & 1) != (r[2] == h[6]) or bool(flags & 2) != bool(flags & 128 and start == h[10]) or bool(flags & 4) != bool(flags & 32 and start < h[10]+960000 and start+r[9] > h[10]) or bool(flags & 8) != bool(flags & 32 and start < h[12] and start+r[9] > 0):
            raise ValueError('Write trace overlap flags')
        if not flags & 64 and any(r[12:15]) or flags & 16 and (not flags & 64 or r[12] != 0x1000):
            raise ValueError('Write trace memory metadata')
        if r[2] == h[6] and (flags != 255 or r[6:10] != (800,600,800,960000) or r[12:14] != (0x1000,4)):
            raise ValueError('Write trace source canvas')
        rows.append(dict(canvas=r[2],source=bool(flags & 1),same_pixel_address=bool(flags & 2),pixel_span_overlap=bool(flags & 4),protected_page_overlap=bool(flags & 8),same_virtual_reservation=bool(flags & 16),layout_known=bool(flags & 32),memory_query_known=bool(flags & 64),has_pixels=bool(flags & 128),start_relative_to_page=start,width=r[6],height=r[7],stride=r[8],span_bytes=r[9],dirty=r[10],sampled=r[11],state=r[12],protection=r[13],memory_type=r[14]))
    if h[6] not in ids:
        raise ValueError('Missing source cache')
    return dict(schema=1,lease=h[5],canvas=h[6],owner=h[7],queue=h[8],sequence=h[9],pixel_offset=h[10],protected_bytes=h[12],canvases=rows,journal_sha256=sha(data),all_backing_aliases_accounted=False,original_source_atomicity_proved=False)


def marker(data, trace, phase):
    if phase not in (2,3):
        raise ValueError('Write trace marker phase')
    if len(data) != 32:
        raise ValueError('Write trace marker extent')
    r = struct.unpack('<8I', data)
    if data[:8] != (b'MNMSWA01' if phase == 2 else b'MNMSWD01') or r[2:] != (1,32,trace['lease'],trace['canvas'],trace['owner'],phase):
        raise ValueError('Write trace marker identity')
    return sha(data)


def fault(data, trace):
    if len(data) != 128:
        raise ValueError('Write fault extent')
    r = struct.unpack('<32I', data)
    if data[:8] != b'MNMSWF01' or r[2:6] != (1,128,trace['lease'],trace['canvas']) or not r[6] or r[7] != trace['owner'] or r[8] >= trace['protected_bytes'] or r[9] != int(trace['pixel_offset'] <= r[8] < trace['pixel_offset']+960000) or r[10] not in (1,2,3) or r[11:13] != (0xc0000005,1) or r[14:16] != (trace['protected_bytes'],trace['pixel_offset']) or r[16] & ~7 or r[28:30] != (trace['sequence'],trace['queue']) or any(r[30:]):
        raise ValueError('Write fault identity/target/reserve')
    if r[16] & 2:
        if not r[16] & 1 or r[18] != 0x14c or not r[20] or r[17] >= r[20] or r[22] != 0x1000000:
            raise ValueError('Write fault PE identity')
    elif any(r[17:22]):
        raise ValueError('Write fault unknown image metadata')
    if not r[16] & 1 and r[22] or r[23] != (16 if r[16] & 4 else 0) or not r[16] & 4 and any(r[24:28]):
        raise ValueError('Write fault instruction metadata')
    aliases = []
    for c in trace['canvases']:
        offset = r[8]-c['start_relative_to_page']
        if c['layout_known'] and 0 <= offset < c['span_bytes'] and offset % (2*c['stride']) < 2*c['width']:
            aliases.append(c['canvas'])
    return dict(schema=1,lease=r[4],canvas=r[5],writer_thread=r[6],owner=r[7],foreign_thread=r[6]!=r[7],target_page_offset=r[8],target_is_source_pixel=bool(r[9]),phase=r[10],target_protection=r[13],pe_identity_known=bool(r[16]&2),instruction_rva=r[17] if r[16]&2 else None,image_machine=r[18],image_timestamp=r[19],image_size=r[20],image_checksum=r[21],code_hex=data[96:112].hex() if r[23] else None,recorded_pixel_aliases=aliases,journal_sha256=sha(data),write_resumed=False,acquisition_cache_is_current_at_fault=False)


def pe_info(data):
    if len(data)<64 or data[:2] != b'MZ':
        raise ValueError('PE DOS header')
    at = struct.unpack_from('<I', data, 60)[0]
    if at+96 > len(data) or data[at:at+4] != b'PE\0\0' or struct.unpack_from('<H',data,at+24)[0] != 0x10b:
        raise ValueError('PE image header')
    return at, dict(machine=struct.unpack_from('<H',data,at+4)[0],timestamp=struct.unpack_from('<I',data,at+8)[0],size=struct.unpack_from('<I',data,at+80)[0],checksum=struct.unpack_from('<I',data,at+88)[0])


def resolve_writer(observation, images):
    matches = []
    if observation['pe_identity_known'] and observation['code_hex']:
        expected = {k:observation['image_'+k] for k in ('machine','timestamp','size','checksum')}
        rva = observation['instruction_rva']; code = bytes.fromhex(observation['code_hex'])
        for path in images:
            data = Path(path).read_bytes(); at, info = pe_info(data)
            if info != expected:
                continue
            count = struct.unpack_from('<H',data,at+6)[0]; table = at+24+struct.unpack_from('<H',data,at+20)[0]
            for i in range(count):
                entry = table+40*i
                if entry+40 > len(data):
                    raise ValueError('PE section table')
                address, size, offset = struct.unpack_from('<3I',data,entry+12)
                if address <= rva and rva+16 <= address+size:
                    begin = offset+rva-address
                    if data[begin:begin+16] == code:
                        matches.append(dict(path=str(path),sha256=sha(data),rva=rva,code_hex=code.hex()))
    return dict(resolved=len(matches)==1,matches=matches,reason='Exact image fields and 16 instruction bytes' if len(matches)==1 else 'Missing, changed or ambiguous image/instruction; no attribution asserted')


def analyze(directory):
    from drawing_source_lease import decode as leases
    from drawing_source_worker_lease import decode as workers
    from drawing_source_held_snapshot import analyze as held
    directory = Path(directory)
    lease_calls = {c['lease']:c for c in leases((directory/'api-source-lease.bin').read_bytes())['calls'] if c['lease']}
    worker_calls = {c['lease']:c for c in workers((directory/'api-source-worker-lease.bin').read_bytes())['calls']}
    held_result = held(directory)
    held_calls = {f['lease']:f for f in held_result['frames'] if f['phase']==1}
    paths = sorted(directory.glob('source-write-trace-*.bin')); results = []; seen = set()
    for path in paths:
        t = snapshot(path.read_bytes()); lease = t['lease']; suffix = f'{lease:04d}.bin'
        a,w,f = lease_calls.get(lease),worker_calls.get(lease),held_calls.get(lease)
        if path.name != 'source-write-trace-'+suffix or lease in seen or not a or not w or not f or (t['canvas'],t['owner'],t['sequence'],t['queue']) != (a['before'][6],a['thread'],a['before'][3],a['before'][4]) or w['owner'] != t['owner'] or w['api_entry'] != a['api_entry'] or f['api_entry'] != a['api_entry'] or f['before'][6] != t['canvas']:
            raise ValueError('Write trace lease/API/worker/held binding')
        seen.add(lease)
        t['armed_sha256'] = marker((directory/('source-write-armed-'+suffix)).read_bytes(),t,2)
        t['done_sha256'] = marker((directory/('source-write-done-'+suffix)).read_bytes(),t,3)
        t['api_entry'] = a['api_entry']; t['completed_commands'] = len(w['commands']); results.append(t)
    if not seen or seen != set(lease_calls) or seen != set(worker_calls) or seen != set(held_calls) or len(list(directory.glob('source-write-armed-*.bin'))) != len(seen) or len(list(directory.glob('source-write-done-*.bin'))) != len(seen) or list(directory.glob('source-write-fault-*.bin')) or list(directory.glob('source-lease-fault-*.bin')):
        raise ValueError('Write trace incomplete, extra or faulted lease cohort')
    return dict(schema=1,success=True,traced_leases=len(results),completed_commands=sum(t['completed_commands'] for t in results),stable_snapshot_pairs=held_result['stable_snapshot_pairs'],other_pixel_span_overlaps=sum(c['pixel_span_overlap'] and not c['source'] for t in results for c in t['canvases']),other_page_overlaps=sum(c['protected_page_overlap'] and not c['source'] for t in results for c in t['canvases']),unknown_layouts=sum(not c['layout_known'] for t in results for c in t['canvases']),leases=results,same_virtual_page_cpu_writes_fail_stop=True,all_backing_aliases_accounted=False,gpu_completion_proved=False,original_source_atomicity_proved=False)
