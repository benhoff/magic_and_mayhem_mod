"""Strict reader for passive MNMBND01 guest metadata (addresses local to one run)."""
import struct
from rendering_pilot import require

PREFIX='research/runtime/required-boundaries-results-20261110/'
CONTROLS=('tools/required_boundary_observation.py','tools/run-required-boundary-session.py',
          'tools/check-required-boundary-session.py','tests/test-required-boundary-observation.py')
ENTRY_ONLY={18,19}
NAMES=('display-init','format-query','main-prepare','main-layouts','main-release','main-exit',
       'quick-enter','quick-release','quick-exit','mini-release','report-prepare','report-layouts',
       'report-release','report-exit','quick-report-release','quick-report-prepare','world-enter',
       'world-exit','realm-enter','realm-exit','region-prepare','region-release','region-exit')

def decode(raw):
    require(len(raw)>=64+512 and (len(raw)-64)%512==0,'Torn boundary metadata')
    h=struct.unpack_from('<16I',raw)
    require(raw[:8]==b'MNMBND01' and h[2:8]==(1,64,512,8192,0x40209ca7,23)
            and not any(h[8:]),'Boundary header/version/build mismatch')
    rows=list(struct.iter_unpack('<128I',raw[64:]))
    require(len(rows)<=8193 and rows[-1][1]==4,'Missing terminal boundary observation')
    active={};entries=[];worlds=[];pairs=[];thread=rows[-1][3]
    require(thread>0,'Missing observer thread')
    for ordinal,r in enumerate(rows,1):
        require(r[0]==ordinal and r[3]==thread,'Boundary sequence/thread mismatch')
        require(r[1] in (1,2,3,4),'Unknown boundary event')
        if r[1]==4:
            require(ordinal==len(rows) and r[2]==0 and r[5]==ordinal-1 and r[8]==0
                    and r[7]==len(worlds) and r[6]>0,'Loss, live frames, or wrong footer')
            require(not active,'Unreturned observed provider')
            require(worlds and 0<worlds[-1][11]<=r[6],'Missing World coverage')
        elif r[1]==1:
            require(r[2]<23 and r[5] and not r[11],'Unknown entry tag/continuation')
            entries.append(r)
            if r[2] not in ENTRY_ONLY:active[ordinal]=r
        elif r[1]==2:
            require(r[11] in active,'Orphan/duplicate boundary return')
            e=active.pop(r[11]);require(r[2]==e[2] and r[5:7]==e[5:7],'Changed return identity')
            pairs.append((e,r))
        else:
            require(r[6] and (r[7],r[8]) in ((800,600),(640,480)) and r[7]<=r[9]<=4096,'Invalid observed World storage')
            require(not worlds or r[11]>worlds[-1][11],'World summary order')
            worlds.append(r)
    return dict(rows=rows,entries=entries,pairs=pairs,worlds=worlds,
                worlds_observed=rows[-1][6],world_changes=len(worlds),
                counts={name:sum(e[2]==i for e in entries) for i,name in enumerate(NAMES)})

def storage_summary(d):
    # Selection is observed at every original World queue, not inferred from a
    # helper call count. These are observations, not global reachability claims.
    worlds=d['worlds']
    return dict(worlds_observed=d['worlds_observed'],changes=d['world_changes'],
        extents=sorted({(r[7],r[8],r[9]) for r in worlds}),
        alternate_states=sorted({tuple(r[24:30]) for r in worlds}),
        formats=sorted({(r[29],r[30]&65535,r[30]>>16,r[31]&65535) for r in worlds}),
        calls=d['counts'],normal_return_pairs=len(d['pairs']))
