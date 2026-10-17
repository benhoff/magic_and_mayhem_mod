"""Exact accepted callbacks and closed producer prefixes, including queue-free updates."""
import importlib.util
import struct
from pathlib import Path
from drawing_api_session import sha
from engine_cadence import decode as cadence
from drawing_producer_updates import analyze as updates


def decode(raw, cadence_rows, producer_records, queue_count):
    if len(raw)<256 or (len(raw)-64)%64 or len(raw)>64+512*64:raise ValueError('Source window envelope')
    h=struct.unpack_from('<16I',raw)
    if raw[:8]!=b'MNMDWIN1' or h[2:6]!=(1,64,0x40209ca7,512) or any(h[6:]):raise ValueError('Source window header')
    rows=list(struct.iter_unpack('<16I',raw[64:]));by_record={r[0]:r for r in cadence_rows};points=[];previous=None;cursor=0;last_queue=0
    for ordinal,r in enumerate(rows[:-1],1):
        c=by_record.get(r[2])
        if r[0]!=ordinal or r[1]!=1 or c is None or c[1]!=2 or (r[3],r[4],r[5],r[6])!=(c[6],c[5],c[10],c[2]) or r[9] or r[10]>128 or any(r[11:]):raise ValueError('Source window cadence identity/failure')
        if r[7]>len(producer_records) or r[8]>queue_count or previous and (r[2]<=previous[2] or (r[3]-previous[3])&0xffffffff!=1 or r[7]<previous[7] or r[8]<previous[8]):raise ValueError('Source window prefix/sequence order')
        while cursor<r[7]:
            record=producer_records[cursor][0];cursor+=1
            if record[2]==27:raise ValueError('Source boundary includes later closure record')
            if record[2]==11:last_queue=record[14]
        if last_queue!=r[8]:raise ValueError('Source window queue prefix identity')
        points.append(dict(cadence_record=r[2],sequence=r[3],counter_before=r[4],mode=r[5],thread=r[6],producer_prefix=r[7],queue_prefix=r[8],pending_frames=r[10]));previous=r
    t=rows[-1]
    if t[0]!=len(rows) or t[1]!=4 or any(t[2:6]) or t[6]!=points[-1]['thread'] or t[7]!=len(producer_records) or t[8]!=queue_count or any(t[9:]) or points[-1]['queue_prefix']!=queue_count:raise ValueError('Source window terminal')
    return dict(schema=1,success=True,journal_sha256=sha(raw),points=points,producer_records=len(producer_records),queue_count=queue_count,
                callbacks_with_pending_frames=sum(bool(p['pending_frames']) for p in points),
                original_source_atomicity_proved=False,complete_presented_journey=False)


def analyze(directory):
    _,rows=cadence(directory/'engine-cadence.bin');bound=updates(directory)
    spec=importlib.util.spec_from_file_location('source_windows_corpus',Path(__file__).with_name('inspect-canvas-producers.py'))
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    _,records=module.decode((directory/'canvas-producers.bin').read_bytes())
    result=decode((directory/'source-update-windows-0000.bin').read_bytes(),rows,records,bound['queue_count'])
    if result['points'][-1]['cadence_record']!=bound['whole_update_source_boundary']['cadence_record']:raise ValueError('Source window closure/whole boundary differ')
    return result
