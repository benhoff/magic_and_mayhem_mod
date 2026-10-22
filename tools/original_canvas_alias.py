"""Validate complete actual-driver alias/drain cohorts without physical claims."""
import struct


def analyze(raw):
    if len(raw)%24:raise ValueError('Truncated alias row')
    rows=[row for row in struct.iter_unpack('<6I',raw) if row[0]!=14]
    cursor=0;release_calls=pixels=extra=0
    def take(kind,phase,id,result=0,a=None,b=None):
        nonlocal cursor
        if cursor>=len(rows):raise ValueError('Incomplete alias cohort')
        row=rows[cursor];cursor+=1
        if any(v is not None and row[i]!=v for i,v in enumerate((kind,phase,id,result,a,b))):
            raise ValueError('Unexpected alias event: '+repr(row))
        return row
    phase=0
    for first in (0,1,3):
        for second in (0,1,3):
            for locked in (0,1):
                take(60,phase,0,a=first,b=second);take(61,phase,0,a=1,b=locked)
                take(62,phase,1,a=0,b=48);take(62,phase,2,a=0,b=48);pixels+=96
                if locked:take(63,phase,2,a=0,b=0)
                for id,held in ((1,first),(2,second)):
                    for count in range(held,-1,-1):take(64,phase,id,result=count,a=0,b=0);release_calls+=1
                take(65,phase,0,a=first+1,b=second+1);take(66,phase,0,a=0,b=0)
                extra+=first+second;phase+=1
    take(99,99,0,a=18,b=0)
    if cursor!=len(rows):raise ValueError('Trailing alias events')
    return {'cohorts':18,'canonical_identity_pairs':18,'defined_shared_pixels':pixels,
            'actual_release_calls':release_calls,'externally_retained_credits_drained':extra,
            'actual_page_unlocks':9,'original_registry_empty':True,
            'calls_after_final_release':0,'physical_allocation_retirement_proved':False,
            'rows':rows}
