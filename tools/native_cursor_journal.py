"""Strict reader for the bounded native cursor lifetime journal (MNMCROW1)."""
import hashlib
import struct

def decode(data):
    if len(data)<128 or len(data)%64:raise ValueError('Incomplete cursor journal')
    header=struct.unpack_from('<16I',data)
    if header[:7]!=(0x434d4e4d,0x31574f52,1,64,0x40209ca7,18,32768) or any(header[7:]):raise ValueError('Unknown cursor journal header')
    rows=[struct.unpack_from('<16I',data,n) for n in range(64,len(data),64)]
    if [r[0] for r in rows]!=list(range(1,len(rows)+1)) or rows[0][1]!=1 or rows[-1][1]!=10:raise ValueError('Missing install/terminal cursor records')
    if any(r[1]<1 or r[1]>11 for r in rows) or any(rows[-1][k] for k in (3,4)) or not rows[-1][5]:raise ValueError('Cursor did not quiesce at shutdown')
    if rows[-1][6]!=rows[-1][7]+rows[-1][8]:raise ValueError('Unbalanced deferred cursor jobs')
    for previous,current in zip(rows,rows[1:]):
        if any(current[k]<previous[k] for k in range(6,16)):raise ValueError('Cursor counters decreased')
    return dict(sha256=hashlib.sha256(data).hexdigest(),rows=len(rows),installed_hooks=18,
                completed_clients=sorted({r[3] for r in rows if r[1]==7}),
                client_calls=rows[-1][14],client_refusals=rows[-1][13],
                queue_accepted=rows[-1][6],queue_completed=rows[-1][7],queue_cancelled=rows[-1][8],
                queue_full=rows[-1][9],busy_retained=rows[-1][11],geometry_cancelled=rows[-1][12],
                outer_closes=rows[-1][15],active_clients=rows[-1][3],pending_jobs=rows[-1][4],closed=bool(rows[-1][5]))
