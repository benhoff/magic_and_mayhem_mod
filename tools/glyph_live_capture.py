"""Strict bounded live glyph wire decoder; counters are not whole-game proof."""
import hashlib
from pathlib import Path
import struct
STATS=('mode','installed','seen','admitted','handled','shadow_compared','forwarded','errors','samples','budget','busy_or_other_thread','memory_or_format_refusals','alias_refusals','backend_refusals','recheck_refusals','allocation_refusals','cap_refusals','reserved21','reserved22','reserved23')
def inspect(directory,mode):
    data=(directory/'glyph-stats.bin').read_bytes()
    if len(data)!=144 or data[:8] not in (b'MNMGLST1',b'MNMGLST2') or struct.unpack_from('<II',data,8)!=((1 if data[:8]==b'MNMGLST1' else 2),144):raise ValueError('Glyph stats header')
    production=data[:8]==b'MNMGLST2'
    words=struct.unpack('<36I',data);stats=dict(zip(STATS,words[4:24]));stats['fp_controls']=list(words[24:])
    if stats['mode']!=({'shadow':1,'takeover':2}[mode]) or stats['installed']!=1 or stats['errors'] or not stats['admitted']:raise ValueError('Glyph mode failed: '+repr(stats))
    if stats['admitted']!=stats['handled']+stats['shadow_compared'] or (mode=='shadow' and stats['handled']) or (mode=='takeover' and stats['shadow_compared']):raise ValueError('Glyph completion accounting')
    samples=sorted(directory.glob('glyph-?.bin'))
    if production:
        if stats['mode']!=2 or stats['budget'] or stats['samples'] or samples or stats['cap_refusals']:raise ValueError('Invalid uncaptured production glyph accounting')
    elif len(samples)!=stats['samples'] or not samples:raise ValueError('Missing retained glyph cohort')
    return dict(stats=stats,artifacts={str(p.name):dict(sha256=hashlib.sha256(p.read_bytes()).hexdigest(),bytes=p.stat().st_size) for p in [directory/'glyph-stats.bin',*samples]},scope='Fixed-build opt-in glyph hook only. Counters observe completed live calls; retained full canvases/coverage/FX/caller snapshots require independent original replay. Layout and all other rendering stay original.')
def decode(path):
    data=Path(path).read_bytes()
    if len(data)<256 or data[:8] not in (b'MNMGLIV1',b'MNMGLIV2'):raise ValueError('Glyph sample magic')
    h=struct.unpack_from('<64I',data)
    if h[2:4]!=((1 if data[:8]==b'MNMGLIV1' else 2),256) or h[19]!=(0 if data[:8]==b'MNMGLIV1' else 1) or h[4] not in (1,2) or not 40<=h[5]<=1048576 or not 0<h[6]<=h[8]<=2048 or not 0<h[7]<=2048 or h[9]!=h[8]*h[7]*2 or len(data)!=256+h[5]+2*h[9]+1536:raise ValueError('Glyph sample extent')
    return h,data
