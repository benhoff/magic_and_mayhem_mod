"""Exact diagnostic pending-copy metadata; page protection supplies the guard.

Canvas identity is not proof of disjoint backing or complete byte ownership.
"""
import struct
from drawing_api_session import sha
from drawing_source_pending_refusal import decode as refusal


def decode(data):
    if len(data)<64 or data[:8]!=b'MNMSPA01':raise ValueError('Pending admission envelope')
    h=struct.unpack_from('<16I',data)
    if h[2] not in (1,2) or h[14:16]!=(3,1) or h[6]<2:raise ValueError('Pending admission policy')
    adapted=bytearray(data);adapted[:8]=b'MNMSPF01';struct.pack_into('<I',adapted,8,1);struct.pack_into('<II',adapted,56,1,0)
    r=refusal(adapted)
    selected=0
    for f in r['frames']:
        if f['reserved_frame'] or f['primitive']:raise ValueError('Pending admission reserved/primitive')
        if f['tag']==7 and f['owner']==r['thread'] and f['object_is_source']:
            selected+=1;continue
        if (f['tag'] not in ({32} if h[2]==1 else {32,39}) or not f['owner'] or f['owner']==r['thread'] or f['object_is_source']
                or not f['record_available'] or f['record_kind']!=6 or not f['record_canvas_word3']
                or not f['record_word4'] or f['record_word4']!=f['object_canvas']
                or f['record_word3_equals_source'] or f['record_word4_equals_source']):
            raise ValueError('Pending admission unknown/source copy')
    if selected!=1:raise ValueError('Pending admission selected multiplicity')
    r.update(journal_sha256=sha(data),reason=3,source_copy_admitted=True,
             wire_version=h[2],policy='diagnostic-readonly-pages-exact-foreign-copy',disjoint_backing_proved=False,
             page_protection_requires_independent_lease_evidence=True)
    return r
