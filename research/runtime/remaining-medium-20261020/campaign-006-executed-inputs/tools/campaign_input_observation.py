"""Decode a bounded sampled prefix; never infer delivered clicks or casts."""
import struct


def decode(data):
    if (len(data)<16 or data[:8] != b'MNMCAI01' or struct.unpack_from('<II',data,8) not in ((1,128),(2,128))
            or (len(data) - 16) % 128 or len(data) > 16 + 4096 * 128):
        raise ValueError('Invalid input admission prefix')
    version=struct.unpack_from('<I',data,8)[0]
    rows = []
    owner = None
    for index, row in enumerate(struct.iter_unpack('<32I', data[16:]), 1):
        if row[0] != index or row[4] & ~(31 if version==1 else 127) or version==1 and any(row[23:]):
            raise ValueError('Input admission loss/overflow/reserved fields')
        if not row[2] or owner is not None and row[2] != owner:
            raise ValueError('Input observation owner changed')
        if rows and (row[1] - rows[-1]['tick']) % 2**32 >= 2**31:
            raise ValueError('Input observation clock moved backwards')
        owner = row[2]
        if not row[4] & 1 and any(row[5:7]) or not row[4] & 2 and row[7]:
            raise ValueError('Invalid original spell-bar sample')
        if not row[4] & 4 and any(row[8:13]):
            raise ValueError('Invalid hit sample')
        if not row[4] & 8 and any(row[13:21]) or row[4] & 8 and row[13] & 1:
            raise ValueError('Invalid or torn Qt input snapshot')
        if row[4] & 2 and (not row[4] & 1 or not 1 <= row[7] <= 256):
            raise ValueError('Invalid validated original receiver slot')
        if version==2 and (row[3] not in (1,2,3) or not row[4]&32 and any(row[23:29]) or not row[4]&64 and any(row[29:])):
            raise ValueError('Invalid original input callback/ground sample')
        rows.append(dict(sequence=row[0],tick=row[1],thread=row[2],screen=row[3],flags=row[4],
                         spell_count=row[5],selected_slot=row[6],receiver_slot_plus_one=row[7],
                         mode=row[8],cursor_type=row[9],hit_class=row[10],hit_index=row[11],ring_index=row[12],
                         input_sequence=row[13],input_active=row[14],x=row[15],y=row[16],
                         left=row[17],right=row[18],width=row[19],height=row[20],
                         world_counter=row[21],gameplay_sequence=row[22]))
        if version==2:
            signed=lambda value: value if value<2**31 else value-2**32
            rows[-1].update(kind=row[3],ground=[signed(v) for v in row[23:26]],ground_bounds=list(row[26:29]),callback_mask=row[29],mouse=[row[30]&65535,row[30]>>16],mouse_flags=row[31])
    return dict(wire_version=version,rows=rows,owner_thread=owner,prefix_only=True,click_delivery_proved=False,
                cast_admission_proved=False,scope=__doc__)
