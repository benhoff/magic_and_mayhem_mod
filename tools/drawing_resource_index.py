"""Exact map state lookup with linear event storage, no per-prefix snapshots.

Inputs are the retained validated public resource journal. States are metadata
associations only; a closed map never establishes retained allocation ownership.
"""
from bisect import bisect_right
import struct


class ResourceIndex:
    def __init__(self, raw):
        self.last_prefix=(len(raw)-64)//128
        self.events={};self.keys_by_texture={}
        for r in struct.iter_unpack('<32I',raw[64:]):
            if r[1]!=2 or r[8]>=0x80000000:continue
            key=(r[10],r[12])
            if r[3]==2:
                state=dict(resource=r[10],sub=r[12],texture=r[28],generation=r[29],call=r[2],interval=r[27],data=r[26],row=r[24],slice=r[25],open=1)
                self.events.setdefault(key,[]).append((r[0],state))
                self.keys_by_texture.setdefault((r[28],r[29],r[12]),set()).add(key)
            elif r[3]==3 and key in self.events:
                previous=self.events[key][-1][1]
                if previous['interval']==r[27]:self.events[key].append((r[0],dict(previous,open=0)))

    def valid_prefix(self, prefix):
        return 0<=prefix<=self.last_prefix

    def candidates(self, prefix, texture, generation, subresource):
        if not self.valid_prefix(prefix):raise ValueError('Resource prefix outside retained journal')
        if not generation:return []
        result=[]
        for key in sorted(self.keys_by_texture.get((texture,generation,subresource),())):
            events=self.events[key];at=bisect_right(events,prefix,key=lambda item:item[0])
            if at:
                state=events[at-1][1]
                if (state['texture'],state['generation'],state['sub'])==(texture,generation,subresource):result.append(state)
        return result
