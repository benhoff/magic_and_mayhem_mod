"""Bounded physical dismissal policy after original victory report/World pop.

This identifies a candidate input window, not a recovered tutorial-state API.
Original settled Realm guards remain authoritative and unchanged.
"""
import struct


def realm_return_state(data, events, report):
    if len(data)!=524288 or data[:16]!=b'MNMMCM22'+struct.pack('<II',22,524288):
        raise ValueError('Unexpected campaign return channel')
    if report.get('return_sequence') is None:return 'wait'
    engine=struct.unpack_from('<8I',data,128)
    sequence,generation,screen,ready,ack,status,thread,handoff=engine
    if not sequence or sequence&1:return 'wait'
    if thread!=report['thread'] or status!=1:
        raise ValueError('Campaign return owner/status changed')
    if ready not in (0,1):raise ValueError('Invalid campaign return readiness')
    request=struct.unpack_from('<I',data,28)[0]
    if ack!=request:return 'wait'
    popped=any(r[0]>report['return_sequence'] and r[1]==28 and r[2]==thread and r[3]==2 and r[11]==0x659408 and r[12]==3 for r in events)
    if not popped:return 'wait'
    if screen==4 and ready and handoff==0:return 'ready'
    if screen==0 and ready==0 and handoff==3:return 'dismiss'
    return 'wait'


def read_realm_return_state(path, events, report):
    data=path.read_bytes()
    with path.open('rb') as stream:
        stream.seek(128);sequence=stream.read(4)
    if len(data)<132 or data[128:132]!=sequence:return 'wait'
    return realm_return_state(data,events,report)
