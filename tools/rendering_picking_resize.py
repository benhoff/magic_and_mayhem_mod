"""Source-derived picking and exact original World-return joins after rebuilds."""
import hashlib
import struct
from rendering_interaction import signed
from rendering_interaction_live import sprite_target, body_record
from rendering_pilot import require


def targets(records, history, position):
    body = sprite_target(records, history)
    frames = {b[:f[19]] for f, b in records if body_record(f)
              and [signed(f[8]), signed(f[9])] == history[:2]
              and hashlib.sha256(b[:f[19]]).hexdigest() == body['frame_sha256']}
    require(len(frames) == 1, 'Ambiguous source-derived foot resource')
    frame = frames.pop()
    width, height, ox, oy = struct.unpack_from('<4I', frame, 4)
    end_table = struct.unpack_from('<I', frame, 44)[0]
    opaque = []
    for y in range(height):
        begin = struct.unpack_from('<I', frame, 40+y*8)[0]
        end = struct.unpack_from('<I', frame, 48+y*8)[0] if y+1 < height else end_table
        x, visible = 0, False
        for run in frame[begin:end]:
            if visible:
                opaque.extend((y, px) for px in range(x, x+run))
            x += run
            visible = not visible
        require(x == width, 'Incomplete source foot row')
    require(opaque and len(position) == 3, 'Missing observed foot/cell source')
    y = max(p[0] for p in opaque)
    x = min((p[1] for p in opaque if p[0] == y), key=lambda px: (abs(2*px-width+1), px))
    foot = dict(body, point=[history[0]-signed(ox)+x, history[1]-signed(oy)+y],
                opaque_pixel=[x, y], expected_cell_xy=[signed(v) for v in position[:2]],
                target_policy='last opaque body row nearest horizontal centre; no hit-feedback search')
    return body, foot


def ready(rows, admissions, paint, target, baseline, kind):
    """A periodic scalar alone cannot name a source frame or prove delivery."""
    if kind not in ('object', 'cell') or not paint:
        return None
    if not all(paint.get(k) is True for k in ('native_gpu_frame', 'gpu_read_complete', 'pixels_captured')):
        return None
    if paint.get('presentation_owner') != 'gui':
        return None
    generation = int(paint['world'])
    if generation <= baseline['frame']['generation']:
        return None
    row = next((r for r in reversed(rows) if r['kind'] == 1 and r['frame']
                and r['frame']['generation'] == generation), None)
    if not row or row['frame']['nonce'] != baseline['frame']['nonce']:
        return None
    cursor = row['cursor']
    if not cursor or cursor['point'] != target['point'] or cursor['poll'] <= baseline['cursor']['poll']:
        return None
    if (cursor['tick']-baseline['tick']) % 2**32 >= 2**31:
        return None
    actor = row['tracked']
    if not actor or (actor['slot'], actor['type'], actor['owner']) != (0, 0, 0) \
            or not actor['active'] or not 0 < actor['health'] < 2**31:
        return None
    admitted = [a for a in admissions if a['kind'] == 1
                and (a['thread'], a['world_return_ordinal']) == (row['thread'], row['world_return'])]
    if len(admitted) != 1:
        return None
    a = admitted[0]
    if a['flags'] & 168 != 168 or a['input_active'] != 1 or [a['x'], a['y']] != target['point']:
        return None
    if any(not 0 <= v < n for v, n in zip(a['ground'], a['ground_bounds'])):
        return None
    if kind == 'object':
        hit = row['hit']
        if not hit or (hit['slot'], hit['type'], hit['owner']) != (0, 0, 0) \
                or hit['pool'] != actor['pool'] or not hit['active'] or not 0 < hit['health'] < 2**31:
            return None
    elif a['ground'][:2] != target['expected_cell_xy']:
        return None
    callbacks = [r for r in rows[baseline['ordinal']:] if r['kind'] == 3
                 and r['ordinal'] < row['ordinal'] and r['mouse'][:2] == target['point']
                 and r['mask'] & 112 and not any(r['mouse'][3:7])]
    if not callbacks:
        return None
    return dict(generation=generation, frame_row=row['ordinal'], admission_row=a['sequence'],
                world_return=row['world_return'], point=target['point'], ground=a['ground'],
                bounds=a['ground_bounds'], pool=actor['pool'], callback=callbacks[-1]['call'],
                orientation=row['camera'][0], paint=int(paint['paint']))


def physical_point(pointer, paint, point):
    """Independent pixel-centre check against the actual captured image box."""
    require(pointer['viewport'][2:] == [paint['width'], paint['height']], 'Pointer/paint viewport differs')
    physical = [pointer['screen'][i]-pointer['viewport'][i] for i in (0, 1)]
    source = [paint['frame_width'], paint['frame_height']]
    box = [paint['left'], paint['top'], paint['image_width'], paint['image_height']]
    actual = [int((physical[i]+.5-box[i])*source[i]//box[i+2]) for i in (0, 1)]
    require(actual == point and all(0 <= point[i] < source[i] for i in (0, 1)),
            'Physical display pixel maps to another original source pixel')
    return actual


