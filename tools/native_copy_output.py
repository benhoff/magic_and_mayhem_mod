"""Independent all-COPY output proof bound to actual closed application calls."""
import json
from pathlib import Path
from native_copy_routes import decode,validate as validate_routes
from native_gpu_copy_diagnostics import validate as validate_gpu


def signed(word):
    return word if word < 0x80000000 else word - 0x100000000


def validate(experiment,directory):
    experiment,directory=Path(experiment),Path(directory)
    routes=validate_routes(experiment)
    if routes['census_version']!=2:raise ValueError('All-copy proof requires version2 caller bindings')
    gpu=validate_gpu(experiment,directory)
    report=json.loads((directory/'report.json').read_text())
    if report.get('selection')!='all-copies' or report.get('copies_seen')!=report['copies_checked']:
        raise ValueError('All-copy selection must compare every native COPY')
    records=[json.loads(line) for line in (directory/'copies.jsonl').read_text().splitlines()]
    checked={r['sequence']:r for r in records}
    header,rows=decode(experiment/'lock-capture/copy-routes-v2.bin')
    if routes['failed_calls'] or routes['routes'].get('unprepared',0):raise ValueError('Active copy failed or lacked preparation')
    linked=set();cpu=[];keyed=[];gpu_rows=[];empty=[]
    for row in rows:
        route=row[15]
        if route==3:continue  # Fills emit UPDATE, not COPY; no copy-output credit.
        first,last,source,target=row[40:44]
        is_keyed=bool(row[12]&(1 if row[11] else 0x8000))
        if is_keyed and not row[10]&64:raise ValueError('Keyed caller lacks known source key')
        if route==2:
            if first:
                if any(seq not in checked or seq in linked for seq in range(first,last+1)):
                    raise ValueError('Missing or duplicate GPU command interval')
                # The legacy GPU validator proves resources and a frozen self
                # snapshot. Also bind every actual piece to the application's
                # signed source/destination rectangle and original key.
                sr=list(map(signed,row[24:28]));dr=list(map(signed,row[28:32]))
                if sr[2]-sr[0]!=dr[2]-dr[0] or sr[3]-sr[1]!=dr[3]-dr[1]:raise ValueError('GPU caller rectangle extents disagree')
                for seq in range(first+bool(row[10]&128),last+1):
                    record=checked[seq];w=record['words']
                    if (w[8:10]!=[int(is_keyed),row[22] if is_keyed else 0]
                        or [record[k] for k in ('source_width','source_height','target_width','target_height')]!=[row[16],row[17],row[19],row[20]]
                        or not(sr[0]<=w[2]<w[4]<=sr[2] and sr[1]<=w[3]<w[5]<=sr[3])
                        or w[6]-w[2]!=dr[0]-sr[0] or w[7]-w[3]!=dr[1]-sr[1]):
                        raise ValueError('GPU caller/geometry/key/extent output mismatch')
                linked.update(range(first,last+1));gpu_rows.append(row)
            else:empty.append(row[0])
            continue  # Existing GPU validator proves pieces/snapshot/resources.
        if route!=1 or not first or first!=last or first not in checked or first in linked:
            raise ValueError('CPU copy lacks unique native output command')
        record=checked[first];w=record['words']
        expected=[source,target,*row[24:28],row[28],row[29],int(is_keyed),row[22] if is_keyed else 0]
        if w[:10]!=expected or any(w[10:]) or [record[k] for k in ('source_width','source_height','target_width','target_height')]!=[row[16],row[17],row[19],row[20]]:
            raise ValueError('CPU caller/geometry/key/resource output mismatch')
        linked.add(first);cpu.append(row[0])
        if is_keyed:keyed.append((row[0],first,'files' in record,row[2]!=0xffffffff))
    if not cpu or not any(k[3] for k in keyed) or not any(r[2]!=0xffffffff for r in gpu_rows):raise ValueError('Both real CPU-keyed and GPU original-module callers are required')
    if not any(retained for _,_,retained,_ in keyed):raise ValueError('Missing retained independent CPU-keyed pixel witness')
    return dict(verified=True,application_calls=len(rows),cpu_copy_calls=len(cpu),cpu_keyed_calls=len(keyed),
        gpu_copy_calls=len(gpu_rows),gpu_empty_calls=len(empty),fill_calls=routes['routes'].get('fill_prepared',0),
        caller_bound_commands=len(linked),native_copies_compared=report['copies_checked'],
        cpu_keyed_retained_cases=sum(retained for _,_,retained,_ in keyed),post_retirement_calls_excluded=header[21],
        caller_census_sha256=routes['census_sha256'],records_sha256=gpu['diagnostic_records_sha256'],
        scope='Every emitted CPU/GPU application COPY binds actual caller metadata and complete independently checked native destination. Fills, no-command GPU plans and retired tails are explicit separate boundaries; no arbitrary raw-reader, original-driver, whole-session or complete replacement claim.')
