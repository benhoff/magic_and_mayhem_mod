"""Exact observed lifetime lookup with linear transition storage."""
from bisect import bisect_right
import struct
from drawing_wine_lifetime import decode


class LifetimeIndex:
    def __init__(self, data):
        decode(data)
        self.last_prefix=(len(data)-64)//96
        self.transitions={}
        for row in struct.iter_unpack('<24I',data[64:]):
            if row[1]!=2:continue
            kind=1 if row[3]<=2 else 3
            if row[3] in (1,3) and not row[8]&0x80000000 and not row[12]:
                self.transitions.setdefault((kind,row[10]),[]).append((row[0],row[11]))
            elif row[3] in (2,4) and row[12]==2:
                self.transitions.setdefault((kind,row[10]),[]).append((row[0],0))

    def valid_prefix(self, prefix):
        return 0<=prefix<=self.last_prefix

    def generation(self, prefix, kind, object_id):
        if not self.valid_prefix(prefix):raise ValueError('Lifetime prefix outside retained journal')
        events=self.transitions.get((kind,object_id),[])
        index=bisect_right(events,(prefix,0xffffffff))
        return events[index-1][1] if index else 0
