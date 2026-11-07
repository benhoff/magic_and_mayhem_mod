"""Read-only readiness of an already prescribed public pointer action."""


def ready_generation(rows, paint, point, hover, after):
    if not paint or not all(paint.get(k) is True for k in
                            ('native_gpu_frame', 'gpu_read_complete', 'pixels_captured')):
        return None
    generation = int(paint['world'])
    if generation <= after or paint.get('presentation_owner') != 'gui':
        return None
    row = next((r for r in reversed(rows) if r['kind'] == 1 and r['frame']
                and r['frame']['generation'] == generation), None)
    if not row or not row['cursor'] or row['cursor']['point'] != point:
        return None
    if (row['tick'] - row['cursor']['tick']) % 2**32 > 500:
        return None
    if hover:
        actor = row['hit']
        if not actor or (actor['slot'], actor['type'], actor['owner']) != (0, 0, 0) \
                or not actor['active'] or not 0 < actor['health'] < 2**31:
            return None
    return generation
