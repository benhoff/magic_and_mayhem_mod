"""Strict observed-write versions; never infer unobserved source atomicity."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<192 or (len(data)-64)%128 or len(data)>64+131072*128:
        raise ValueError('Source version envelope')
    h=struct.unpack_from('<16I',data)
    version=2 if data[:8]==b'MNMSVG02' else 1
    if data[:8] not in (b'MNMSVG01',b'MNMSVG02') or h[2:8]!=(version,128,0x40209ca7,131072,2048,4) or any(h[8:]):
        raise ValueError('Source version header')
    rows=list(struct.iter_unpack('<32I',data[64:]))
    if rows[-1][1]!=7 or any(r[1] in (7,8) for r in rows[:-1]):
        raise ValueError('Source version incomplete or refused')
    states={};pending={};seen=set();commands=set();workers=set();links=[];mutations=[];open_writes={};write_kinds={}
    def state(resource):
        if not 1<=resource<=2048:raise ValueError('Source version resource identity')
        return states.setdefault(resource,[1,0])
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (*range(1,7),*((9,) if version==2 else ())) or not r[2] or any(r[24:]):
            raise ValueError('Source version row identity/reserved')
        epoch,writers=state(r[8])
        if r[1]==9:
            if r[8] in open_writes or not writers or r[11:13]!=(epoch,writers) or any(r[4:8]) or any(r[9:11]) or any(r[13:24]):raise ValueError('Source version open-write boundary')
            open_writes[r[8]]=dict(resource=r[8],epoch=epoch,writers=writers);continue
        if open_writes:raise ValueError('Source version events after boundary')
        if r[1] in (1,2):
            if any(r[4:8]) or r[10] or any(r[13:18]) or r[18] not in (1,2,3,4) or any(r[19:24]):
                raise ValueError('Source version mutation fields')
            count=writers+(1 if r[1]==1 else -1)
            if count<0 or r[11:13]!=(epoch+1,count):raise ValueError('Source version mutation epoch/lease')
            key=(r[8],r[9],r[18]);kind_count=write_kinds.get(key,0)+(1 if r[1]==1 else -1)
            if kind_count<0:raise ValueError('Source version mutation kind pairing')
            write_kinds[key]=kind_count
            states[r[8]]=[r[11],count]
            mutations.append(dict(record=r[0],phase=r[1],thread=r[2],resource=r[8],subresource=r[9],epoch=r[11],writers=count,kind=r[18]))
            continue
        if not r[4] or not r[6] or not r[7] or not r[13] or r[16] or r[18] or r[23]>1 or r[11:13]!=(epoch,writers):
            raise ValueError('Source version snapshot state')
        identity=(r[5],r[6],r[7],r[8],r[9],r[10],r[13])
        if r[1]==3:
            if r[4] in seen or r[10]!=epoch or writers or any(r[14:16]) or any(r[19:22]) or r[22]!=r[7] or r[23]:
                raise ValueError('Source version snapshot admission')
            seen.add(r[4]);pending[r[4]]=dict(identity=identity,submission=r[4],api_entry=r[5],source_texture=r[6],source_generation=r[7],resource=r[8],subresource=r[9],epoch=r[10],payload_ordinal=r[13],snapshot_record=r[0])
            continue
        s=pending.get(r[4])
        if s is None or s['identity']!=identity:raise ValueError('Source version missing/changed snapshot')
        if r[1]==4:
            if not r[14] or r[14] in commands or 'command' in s or r[15] or any(r[19:22]) or r[23]:
                raise ValueError('Source version packet binding')
            commands.add(r[14]);s.update(command=r[14],packet_record=r[0]);continue
        if s.get('command')!=r[14] or not r[15] or r[22]!=r[7] or not r[19] or r[23]!=int(r[19]==r[8]):
            raise ValueError('Source version worker identity/generation')
        if r[20:22]!=tuple(state(r[19])):raise ValueError('Source version destination state')
        if r[1]==5:
            if 'worker' in s or r[15] in workers or epoch!=s['epoch'] or writers:
                raise ValueError('Source version worker entry changed')
            workers.add(r[15]);s.update(worker=r[15],worker_thread=r[2],worker_entry_record=r[0],destination_resource=r[19],aliased_destination=bool(r[23]))
        else:
            if (s.get('worker'),s.get('worker_thread'),s.get('destination_resource'))!=(r[15],r[2],r[19]) or epoch!=s['epoch']+2*r[23] or writers:
                raise ValueError('Source version worker return changed')
            s.update(worker_return_record=r[0],return_epoch=epoch);s.pop('identity');links.append(s);pending.pop(r[4])
    t=rows[-1]
    expected_open={k for k,v in states.items() if v[1]}
    if t[0]!=len(rows) or t[4] not in (1,2) or not t[2] or t[13]!=len(seen) or any(t[5:13]) or any(t[16:] if version==2 else t[14:]) or pending or set(open_writes)!=expected_open or (version==2 and (t[14],t[15])!=(sum(v['writers'] for v in open_writes.values()),len(open_writes))):
        raise ValueError('Source version terminal/pending write or snapshot')
    return dict(schema=1,wire_version=version,success=True,journal_sha256=sha(data),bindings=links,mutations=mutations,snapshots=len(seen),checked_workers=len(links),open_write_leases=list(open_writes.values()),all_global_write_leases_closed=not open_writes,all_observed_snapshot_workers_checked=bool(links) and len(links)==len(seen),
                unobserved_direct_writes_accounted=False,separate_resource_backing_aliases_accounted=False,original_source_atomicity_proved=False,gpu_completion_proved=False,complete_presented_journey=False)


def analyze(directory, original):
    from drawing_wine_source_map import analyze as maps
    from drawing_wine_blit_packet import decode as packets
    from drawing_wine_blit_worker import decode as workers
    from drawing_wine_submission import decode as submissions
    from drawing_api_session import inspect
    api=inspect(directory)
    if api!=original['observation']:raise ValueError('Raw API reference changed')
    rows=struct.iter_unpack('<32I',(directory/'session.bin').read_bytes()[64:-128])
    by_api={r[29]:r for r in rows};life=(directory/'wine-lifetime.bin').read_bytes()
    sub=submissions((directory/'wine-submission.bin').read_bytes(),by_api,life)
    work=workers((directory/'wine-blit-worker.bin').read_bytes(),life)
    packet=packets((directory/'wine-blit-packet.bin').read_bytes(),{c['call']:c for c in sub['calls']},{c['call']:c for c in work['calls']})
    source=maps(directory)
    states={0:{}};active={}
    for r in struct.iter_unpack('<24I',life[64:]):
        if r[1]==2:
            kind=1 if r[3]<=2 else 3
            if r[3] in (1,3) and not r[8]&0x80000000 and not r[12]:active[kind,r[10]]=r[11]
            elif r[3] in (2,4) and r[12]==2:active.pop((kind,r[10]),None)
        states[r[0]]=dict(active)
    version_raw=(directory/'wine-source-version.bin').read_bytes()
    for r in struct.iter_unpack('<32I',version_raw[64:]):
        if r[1] in (3,4,5,6) and (r[17] not in states or states[r[17]].get((1,r[6]),0)!=r[22]):raise ValueError('Version exact original lifetime prefix')
    for name,value in [('wine_submission_observation',sub),('wine_blit_worker_observation',work),('wine_blit_packet_observation',packet),('wine_source_map_observation',source)]:
        if original[name]!=value:raise ValueError('Raw version dependency changed: '+name)
    result=decode(version_raw)
    by_sample={s['submission']:s for s in source['samples']};by_packet={s['command']:s for s in packet['bindings']}
    if set(by_sample)!={s['submission'] for s in result['bindings']}:
        raise ValueError('Missing/extra source version snapshots')
    for link in result['bindings']:
        s=by_sample[link['submission']];p=by_packet.get(link['command'])
        if s['status'] or (link['api_entry'],link['source_texture'],link['source_generation'],link['resource'],link['subresource'],link['payload_ordinal'])!=(s['api_entry'],s['source_texture'],s['source_generation'],s['resource'],s['subresource'],s['payload_ordinal']):
            raise ValueError('Version/source payload identity')
        if p is None or (p['submission'],p['worker'],p['worker_thread'])!=(link['submission'],link['worker'],link['worker_thread']):
            raise ValueError('Version/exact packet worker identity')
    result.update(source_journal_sha256=source['journal_sha256'],packet_journal_sha256=packet['journal_sha256'],worker_journal_sha256=work['journal_sha256'])
    return result
