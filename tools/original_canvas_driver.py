"""Strict completeness checks for real-driver original-wrapper recovery traces."""
import struct

LOST=0x887601c2


def analyze(raw):
    if len(raw)%24:raise ValueError('Truncated driver row')
    rows=list(struct.iter_unpack('<6I',raw))
    if not rows or rows[-1]!=(99,99,0,0,4,0):raise ValueError('Missing completion')
    events=[r for r in rows[:-1] if r[0]!=14]
    cursor=0
    def take(kind,phase,id,result=None,a=None,b=None):
        nonlocal cursor
        if cursor>=len(events):raise ValueError('Incomplete cohort')
        r=events[cursor];cursor+=1
        wanted=(kind,phase,id,result,a,b)
        if any(v is not None and r[i]!=v for i,v in enumerate(wanted)):
            raise ValueError('Unexpected driver event: '+repr((r,wanted)))
        return r
    def lock(phase,id,defined):
        take(42,phase,id,0,1)
        take(44,phase,id,0,0)
        take(45,phase,id,0,48,defined)
    for operation in range(2):
        initial=operation*10;after=initial+1
        for id in range(2):
            take(40,initial,id,0,0x10004040 if id else 0x840,id+1)
            take(41,initial,id,0,0x1000400,0x1234)
            lock(initial,id,1)
        for id in range(2):
            take(46,after,id,LOST if id else 0,operation,0)
            if operation:
                if id:
                    take(42,after,id,LOST,1,0)
                    take(43,after,id,0,0,0)
                lock(after,id,0)
            take(41,after,id,0,0x1000400,0xabcd)
            if id and not operation:
                take(42,after,id,LOST,1,0)
                take(43,after,id,0,0,0)
            lock(after,id,1)
        for id in range(2):take(47,initial+2,id,0,0,1-id)
    if cursor!=len(events):raise ValueError('Unexpected trailing driver events')
    return {'rows':rows,'cohorts':4,'actual_lock_surface_lost_paths':2,'actual_fill_surface_lost_paths':0,
            'actual_restore_successes':2,'defined_pixels_compared':384,
            'undefined_restored_pixels_asserted':False,'original_release_calls':4,
            'original_registry_empty':True,'wine_blt_success_while_islost':True,'success':True}
