"""Strict API/source-point cross-binding; metadata never proves owned pixels."""
import importlib.util
from pathlib import Path
import struct
from drawing_call_intervals import decode_intervals
from drawing_api_session import sha


def decode_points(raw, journal, intervals, producer_records):
    ordering=decode_intervals(intervals,journal)
    if len(raw)<256 or (len(raw)-64)%192 or len(raw)>64+786433*192:raise ValueError('Incomplete producer point envelope')
    h=struct.unpack_from('<16I',raw)
    if raw[:8]!=b'MNMDSRC1' or h[2:6]!=(1,192,0x40209ca7,786433) or any(h[6:]):raise ValueError('Unsupported producer point header')
    calls={r[29]:r for r in struct.iter_unpack('<32I',journal[64:-128]) if r[1]==1}
    points=list(struct.iter_unpack('<48I',raw[64:]))
    if points[-1][1]!=4 or any(p[1]==4 for p in points[:-1]):raise ValueError('Missing/early producer point terminal')
    grouped={};last_producer=len(producer_records)
    # Check each queried generation at exactly its recorded closed prefix.
    targets=[]
    for i,p in enumerate(points[:-1],1):
        if p[0]!=i or p[1] not in (1,2,3) or p[2] not in calls or any(p[11:16]):raise ValueError('Invalid producer point identity/reserved fields')
        c=calls[p[2]]
        if p[4:6]!=(c[2],c[3]) or p[7]!=c[6]:raise ValueError('Producer point differs from original API call')
        if c[2] in (5,7,11) and p[6]!=c[4]:raise ValueError('Producer point input generation changed')
        if p[1]==1 and any((p[3],p[9],p[10])):raise ValueError('Result/frame in producer entry point')
        if p[1]>=2 and p[9]!=c[8]:raise ValueError('Producer point original result changed')
        if p[1]==2 and (p[3] or p[10]):raise ValueError('Journal/frame in producer return point')
        if p[1]==3 and (p[3],p[10])!=(c[0],c[12]):raise ValueError('Producer completion journal/frame changed')
        dst,src=p[16:32],p[32:48]
        for value in (dst,src):
            if value[:2]!=(64,1) or not value[2]&1 or value[2]&~31 or value[2]&8 and value[2]&16 or value[15] or value[14]!=c[6] or value[3]>last_producer or value[12]>128 or value[13]>128:raise ValueError('Invalid scalar source point')
            if value[2]&16:
                if not value[6]:raise ValueError('Matched source point lacks generation')
                targets.append((value[3],value[6],value[7:9]))
            elif any(value[6:12]):raise ValueError('Unknown/ambiguous source point contains an owned extent')
        if dst[2]&7!=src[2]&7 or dst[3:6]!=src[3:6] or dst[12:15]!=src[12:15]:raise ValueError('Source pair recorder snapshots differ')
        grouped.setdefault(p[2],[]).append(p)
    for entry,c in calls.items():
        ps=grouped.get(entry,[])
        if [p[1] for p in ps]!=[1,2,3] or any(p[6]!=ps[0][6] for p in ps) or any(ps[i][19]>ps[i+1][19] for i in (0,1)):raise ValueError('Missing/reordered producer point phases or sequence')
    terminal=struct.unpack_from('<32I',journal,len(journal)-128);end=points[-1]
    if len(points)!=3*len(calls)+1 or end[0]!=len(points) or end[2]!=terminal[2] or end[3]!=terminal[0] or not end[7] or end[9:13]!=(0,terminal[22],0,0) or any(end[4:7]) or any(end[13:]):raise ValueError('Producer point terminal accounting failure')
    active={};cursor=0
    for prefix,identity,extent in sorted(targets):
        while cursor<prefix:
            record,_=producer_records[cursor];cursor+=1
            if record[2]==1:active[record[3]]=record[5:7]
            elif record[2]==2:active.pop(record[3],None)
        if active.get(identity)!=extent:raise ValueError('Source generation/extent absent at recorded producer prefix')
    frames=[]
    for f in ordering['frames']:
        ps=grouped[f['entry']]
        frames.append(dict(ordinal=f['ordinal'],entry=f['entry'],
                           points=[dict(phase=p[1],destination=list(p[16:32]),source=list(p[32:48])) for p in ps],
                           both_generations_known=all(p[18]&16 and p[34]&16 for p in ps),
                           producer_live_at_all_points=all(p[18]&2 and not p[18]&4 and not p[21] for p in ps),
                           competing_mutation_count=f['competing_mutation_count'],
                           coherent_presented_frame=False))
    return dict(schema=1,success=True,boundary='observed_api_producer_metadata',
                point_journal_sha256=sha(raw),producer_records=last_producer,api_surface_calls=len(calls),
                point_records=len(points),frames=frames,
                frames_with_known_generations=sum(f['both_generations_known'] for f in frames),
                frames_with_live_producer_points=sum(f['producer_live_at_all_points'] for f in frames),
                source_inputs_owned=False,original_pixels_used_as_native_inputs=False,
                coherent_presented_frames=False,complete_presented_journey=False)


def inspect_points(directory,session=False):
    root=Path(directory)
    path=Path(__file__).with_name('inspect-canvas-producers.py')
    spec=importlib.util.spec_from_file_location('producer_inspector',path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    producers=(root/'canvas-producers.bin').read_bytes();_,records=module.decode_session(producers,(root/'canvas-session-0000.done').read_bytes()) if session else module.decode(producers,complete=True)
    result=decode_points((root/'api-producer-points.bin').read_bytes(),(root/'session.bin').read_bytes(),(root/'api-call-intervals.bin').read_bytes(),records)
    result['producer_journal_sha256']=sha(producers)
    if session:result['session_producer_marker_sha256']=sha((root/'canvas-session-0000.done').read_bytes())
    return result
