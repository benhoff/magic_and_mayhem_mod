"""Decode bounded recorder refusal inputs; never admit a failed source session."""
import struct

REASONS = {1: 'unbound_destination', 2: 'unreadable_shape', 3: 'pixel_format',
           4: 'trace_uniform_policy', 5: 'uniform_channel_range',
           6: 'primitive_extent_budget', 7: 'unreadable_plane', 8: 'plane_channel_range', 9: 'source_destination_alias'}


def decode(raw):
    if len(raw) != 192 or raw[:8] != b'MNMCRF01':
        raise ValueError('Colour refusal extent/magic')
    w = struct.unpack('<48I', raw)
    if (w[2:4] != (1, 192) or w[4] not in REASONS or not w[6] or
            w[18] > 1 or w[19] not in (0, 1, 3) or not w[36] or
            any(w[37:]) or raw[102:104] != b'\0\0' or
            not w[19] & 1 and any(raw[80:140]) or
            not w[19] & 2 and any(raw[104:140])):
        raise ValueError('Colour refusal identity/flags/reserved')
    signed = lambda v: v if v < 0x80000000 else v - 0x100000000
    shape = None
    if w[19] & 1:
        mode, width, height, red, green, blue, plane = struct.unpack_from('<IiiHHHI', raw, 80)
        shape = dict(mode=mode, width=width, height=height, channels=[red, green, blue],
                     plane=plane, plane_prefix_hex=raw[104:140].hex() if w[19] & 2 else None)
    return dict(reason=REASONS[w[4]], reason_code=w[4], producer_prefix=w[5],
                failure_ordinal=w[6], caller=w[7], shape_address=w[8],
                binding=w[9], pixel_format=w[10], surface=w[11],
                clip=list(map(signed, w[12:16])), x=signed(w[16]), y=signed(w[17]),
                trace_mode=bool(w[18]), shape=shape, queue=w[35], thread=w[36],
                source_admission=False)
