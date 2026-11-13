"""Source-only cell targeting from captured sprite identity and ANI placement.

This is an authored test-target policy. It does not replace original ground
picking, infer a terrain hit from image pixels, or search observed hit results.
"""
import hashlib
import struct

from rendering_interaction import signed
from rendering_interaction_live import sprite_target
from rendering_pilot import require

POLICY = 'inverse-unique-ANI-body-placement-v1'


def frame_identity(frame):
    """Match the recorder's pointer-free SPR header (DWORD +28 is cleared)."""
    require(len(frame) >= 40 and struct.unpack_from('<I', frame)[0] == len(frame),
            'Incomplete indexed SPR frame')
    canonical = bytearray(frame)
    canonical[28:32] = bytes(4)
    return hashlib.sha256(canonical).hexdigest()


def asset_offsets(sprite, animation):
    """Decode only the selected v4 SPR/v5 ANI placement contract, fail closed."""
    require(len(sprite) >= 24 and len(animation) >= 44, 'Truncated placement assets')
    magic, size, version, count, palettes, _ = struct.unpack_from('<6I', sprite)
    require(magic == 0x525053 and size == len(sprite) and version == 4
            and 0 < count <= 4096 and 1 <= palettes <= 4, 'Unsupported indexed SPR asset')
    table = 24 + palettes * 768
    base = table + count * 4
    require(base <= len(sprite), 'Truncated SPR offset table')
    frames = []
    for i in range(count):
        at = base + struct.unpack_from('<I', sprite, table + i*4)[0]
        require(at + 40 <= len(sprite), 'SPR frame offset outside asset')
        length = struct.unpack_from('<I', sprite, at)[0]
        require(length >= 40 and at + length <= len(sprite), 'SPR frame extent outside asset')
        frames.append(frame_identity(sprite[at:at+length]))
    magic, size, count, version, _, offsets = struct.unpack_from('<6I', animation)
    require(magic == 0x494e41 and size == len(animation) and version == 5
            and 0 < count <= 65536 and 2 <= offsets <= 65536, 'Unsupported ANI asset')
    base = 44 + offsets*4
    require(base + count*44 == len(animation), 'ANI record extent differs')
    starts = struct.unpack_from('<' + 'I'*offsets, animation, 44)
    require(starts[0] == 0 and starts[-1] == count
            and all(a < b for a, b in zip(starts, starts[1:])), 'ANI sequence bounds differ')
    result = {}
    for i in range(count):
        opcode, frame, x, y = struct.unpack_from('<4i', animation, base + i*44)
        if opcode != 0:
            continue
        require(0 <= frame < len(frames), 'ANI frame outside selected SPR')
        result.setdefault(frames[frame], set()).add((x, y))
    return {digest: sorted(values) for digest, values in result.items()}


def projected_targets(records, history, position, index):
    body = sprite_target(records, history)
    require(len(position) == 3 and len(history) >= 2, 'Missing observed actor pose')
    offsets = index.get(body['frame_sha256'], [])
    require(len(offsets) == 1 and len(offsets[0]) == 2
            and all(type(v) is int and -(2**31) <= v < 2**31 for v in offsets[0]),
            'Missing or ambiguous source ANI body placement')
    # The finite actor has footprint 1. Size-2 orientation adjustments are not
    # admitted here. Original draw history = projected actor base + ANI offset.
    x, y = offsets[0]
    cell = {k: v for k, v in body.items() if k != 'opaque_pixel'}
    cell.update(point=[signed((history[0]-x) % 2**32), signed((history[1]-y) % 2**32)],
                expected_cell_xy=[signed(v) for v in position[:2]],
                animation_body_offset=[x, y], footprint=1, target_policy=POLICY)
    return body, cell


def load_index(reader, reference, installation_inputs):
    """Rebuild the index from retained exact assets, checked against this launch."""
    require(reader.digest(reference['record']) == reference['sha256'], 'Placement review hash differs')
    review = reader.json(reference['record'])
    require(review['kind'] == 'reviewed_finite_picking_animation_placement'
            and review['success'] and review['policy'] == POLICY
            and review['actor'] == dict(slot=0, type=0, owner=0, footprint=1),
            'Unsupported picking actor/placement review')
    assets = {}
    for name, source in review['assets'].items():
        require(installation_inputs[name]['sha256'] == source['sha256']
                and reader.digest(source['record']) == source['sha256'],
                'Current installation differs from reviewed placement asset')
        assets[name] = reader.read_bytes(source['record'])
    config = assets['CFG/creature.cfg'].decode('ascii')
    import re
    first = re.search(r'(?s)\[CREATURE_0\](.*?)(?:\[CREATURE_1\])', config)
    require(first is not None and re.findall(r'(?m)^TileSizeXY\s*=\s*(\d+)\s*$', first[1]) == ['1'],
            'Finite wizard footprint differs')
    expected = {'CFG/creature.cfg'}
    merged = {}
    for pair in review['asset_pairs']:
        spr, ani = pair['sprite'], pair['animation']
        expected.update((spr, ani))
        for digest, offsets in asset_offsets(assets[spr], assets[ani]).items():
            merged.setdefault(digest, set()).update(tuple(v) for v in offsets)
    require(set(assets) == expected, 'Unexpected placement inputs')
    index = {digest: [list(v) for v in sorted(offsets)] for digest, offsets in merged.items()}
    require(index == review['offset_index'], 'Placement index differs from retained asset reconstruction')
    return index
