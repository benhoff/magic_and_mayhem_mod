"""Decoder and acceptance checks for the authored original allocation repair."""
import struct
FIELDS=['magic','version','header_bytes','reserved','allocations','publications','retirements','copies','world_bindings','error','active_base','active_source','width','height','padding_x','padding_y','ordinary_pitch','backing_width','backing_height','guard_checks','pixel_checks','pixel_words','rows','thread','enabled','initializations','shutdowns','creation_repairs','entry_checks','padding_resets','red_mask','green_mask','blue_mask','nonzero_words']
def decode(path):
 raw=path.read_bytes()
 if len(raw)<256 or (len(raw)-256)%80:raise ValueError('Owner record extent')
 h=struct.unpack('<64I',raw[:256]);rows=list(struct.iter_unpack('<20I',raw[256:]))
 if h[:3]!=(0x414e4d4d,1,256) or len(rows)!=h[22] or any(row[0]!=i+1 for i,row in enumerate(rows)):raise ValueError('Owner record identity/sequence')
 return dict(zip(FIELDS,h),events=[list(x) for x in rows])
def accept(v,live=False):
 if v['error'] or v['active_base'] or v['active_source']:raise ValueError('Owner failed or retained publication')
 if not v['allocations'] or not v['allocations']==v['publications']==v['retirements']==v['creation_repairs']:raise ValueError('Unbalanced generations')
 if not v['shutdowns'] or not v['copies'] or v['guard_checks']<v['copies']*2 or not v['pixel_checks']:raise ValueError('Missing copy/guard/shutdown checks')
 if live and (not v['world_bindings']>=v['copies'] or v['nonzero_words']<100):raise ValueError('Missing original alternate World bindings')
 return v
