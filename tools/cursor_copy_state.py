"""Strict companion-state join for two selected original cursor COPY callers."""
from pathlib import Path
import struct
from native_copy_routes import decode as decode_routes
from rendering_operations_history import sha, signed, inside

CAPACITY = 8192
WORDS = 80
SELECTED = {0x18c488, 0x18cbb2}


def decode(data, census_header, census_rows):
    """Require a strictly decoded closed v2 census; state is observation only."""
    if len(data) != 128+CAPACITY*WORDS*4:
        raise ValueError('Cursor companion extent')
    h = struct.unpack_from('<32I', data)
    if (data[:8] != b'MNMCCS01' or h[2:6] != (1,128,320,CAPACITY)
            or h[6] != h[8] or h[6] > CAPACITY or h[7] or h[9] != 1
            or h[10] != 0x40209ca7 or any(h[16:])):
        raise ValueError('Cursor companion incomplete, exhausted or unsupported')
    if (census_header[2] != 2 or h[11:13] != census_header[15:17]
            or h[13:15] != (census_header[6],census_header[8]) or h[15]):
        raise ValueError('Cursor companion/census identity or completeness differs')
    expected = [r for r in census_rows if r[2] in SELECTED]
    if len(expected) != h[6] or any(data[128+h[6]*320:]):
        raise ValueError('Cursor companion selected coverage or unused tail differs')
    observations = []
    for index, original in enumerate(expected):
        r = struct.unpack_from('<80I', data,128+index*320)
        if r[:44] != original or r[46] not in (0,1,2) or r[77] > 1 or r[78] > 1 or r[79]:
            raise ValueError('Cursor companion row binding or reserved state differs')
        stable = bool(r[46] and r[47] == r[53] and not r[47]&1)
        if bool(r[77]) != stable or (not r[46] and any(r[47:54])):
            raise ValueError('Cursor companion input snapshot identity differs')
        sr, dr = list(map(signed,r[24:28])),list(map(signed,r[28:32]))
        saved = list(map(signed,r[55:59]))
        observations.append(dict(call=r[0],caller=hex(r[1]),caller_rva=hex(r[2]),thread=r[3],
                                 tick=r[44],result=hex(r[13]),failed=bool(r[13]&0x80000000),
                                 source_generation=r[9],target_generation=r[8],
                                 source_extent=list(r[16:19]),target_extent=list(r[19:22]),
                                 source_rectangle=sr,target_rectangle=dr,
                                 source_inside=inside(sr,r[16],r[17]),target_inside=inside(dr,r[19],r[20]),
                                 window=hex(r[45]),coordinate_mode=r[46],wire_sequence_before=r[47],
                                 wire_sequence_after=r[53],wire_snapshot_stable=stable,wire_active=bool(r[48]),
                                 wire_point=list(map(signed,r[49:51])),wire_extent=list(r[51:53]),
                                 saved_state_readable=bool(r[78]),saved_valid=r[54],saved_rectangle=saved,
                                 saved_offset=list(map(signed,r[59:61])),cursor_class=r[61],
                                 cursor_enabled=r[62],deferred=r[63],
                                 window_rectangle_valid=bool(r[64]),window_rectangle=list(map(signed,r[65:69])),
                                 client_rectangle_valid=bool(r[69]),client_rectangle=list(map(signed,r[70:74])),
                                 native_desktop_cursor_valid=bool(r[74]),native_desktop_cursor=list(map(signed,r[75:77])),
                                 request_source_equals_saved=sr==saved,request_target_equals_saved=dr==saved))
    return dict(verified=True,sha256=sha(data),observations=observations,
                selected_calls=len(observations),failed_calls=[r['call'] for r in observations if r['failed']],
                unstable_wire_snapshots=sum(not r['wire_snapshot_stable'] for r in observations),
                original_driver_equivalence=False,actual_getcursorpos_return_joined=False,
                scope='Exact selected COPY metadata joined to saved original cursor globals, coherent-or-explicitly-unstable published Qt input and queried window/client/native-desktop state. Published state is not proof of the actual preceding GetCursorPos return or coordinate writer.')


def validate(experiment):
    directory = Path(experiment)/'lock-capture'
    header,rows = decode_routes(directory/'copy-routes-v2.bin')
    return decode((directory/'cursor-copy-state-v1.bin').read_bytes(),header,rows)
