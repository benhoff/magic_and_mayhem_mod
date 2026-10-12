"""Shared portable storage-point envelope; versions retain distinct identity rules."""


def point_header(m,thread,client=False):
    if len(m)!=16:raise ValueError('Storage point metadata length')
    flags=m[2]
    if m[0]!=64 or m[1] not in (1,2) or flags & ~(255 if client else 191) or not flags & 1 or m[14]!=thread:
        raise ValueError('Storage point metadata header/thread')
    if m[1]==1 and m[15] or m[1]==2 and (m[15]>131071 or (flags&2 and not flags&4 and not m[15])):
        raise ValueError('Storage point version/query identity')
    if sum(bool(flags&f) for f in (8,16,32,128))>1:
        raise ValueError('Conflicting storage point association')
