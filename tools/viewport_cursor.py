"""Finite viewport-edge and cursor-continuity proof; no picking or balance policy."""
import ast
from pathlib import Path
import struct
from rendering_pilot import require
from rendering_interaction import signed

PREFIX = 'research/runtime/viewport-cursor-results-20261108/'
LEAVES = {'journey.camera-motion.viewport-boundaries',
          'journey.cursor-interaction.cursor-motion'}
SIZES = [[800, 600], [640, 480], [800, 600]]
CONTROLS = ('tools/viewport_cursor.py', 'tools/run-rendering-viewport-cursor.py',
            'tools/prepare-rendering-viewport-cursor.py',
            'tools/check-rendering-viewport-cursor.py', 'tools/close-rendering-viewport-cursor.py',
            'tests/test-viewport-cursor.py', 'tests/test-viewport-cursor-admission.py')


def analysis_paths(root):
    """Pin local imports, including the three explicitly loaded analyzers."""
    pending = list(CONTROLS) + ['tools/join-rendering-normal-acceptance.py',
        'tools/inspect-canvas-producers.py', 'tools/rendering_normal_acceptance.py']
    seen = set()
    while pending:
        name = pending.pop()
        if name in seen:
            continue
        seen.add(name)
        for node in ast.walk(ast.parse((root/name).read_bytes())):
            names = [v.name for v in node.names] if isinstance(node, ast.Import) else (
                [node.module] if isinstance(node, ast.ImportFrom) and node.module else [])
            for module in names:
                target = 'tools/'+module.split('.')[0]+'.py'
                if (root/target).is_file() and target not in seen:
                    pending.append(target)
    return seen


def hidden_copy_witness(raw):
    require(len(raw) == 128, 'Incomplete hidden redraw diagnostic')
    r = struct.unpack('<32I', raw)
    require(r[:6] == (0x484d4e4d, 0x31305043, 1, 128, 0x40209ca7, 0x5770fa)
            and r[6:10] == (640,480,640,480) and r[16:20] == (15,1,16,16)
            and all(v == 0 for v in r[22:]) and min(r[20:22]) > 0,
            'Wrong hidden redraw build/caller/format/ownership')
    l,t,right,bottom,x,y = r[10:16]
    require(all(v < 2**31 for v in r[10:16]) and right-l == bottom-t == 40
            and [x,y] == [l,t] and (l >= 640 or t >= 480),
            'Hidden redraw is visible or outside the guarded policy')
    return dict(verified=True, caller='0x5770fa', rectangle=list(r[10:14]),
                source_lifetime=r[20], destination_lifetime=r[21])


def edge_points(size):
    require(size in SIZES, 'Unsupported viewport domain')
    w, h = size
    return [('center', [(w-200)//2, (h-50)//2]),
            ('top', [w//2, 0]), ('top-right', [w-1, 0]),
            ('right', [w-1, h//2]), ('bottom-right', [w-1, h-1]),
            ('bottom', [w//2, h-1]), ('bottom-left', [0, h-1]),
            ('left', [0, h//2]), ('top-left', [0, 0])]


def cursor_ready(rows, paint, point, baseline):
    """Require a fresh actual paint and completed original cursor poll."""
    if not paint or not all(paint.get(k) is True for k in
                           ('native_gpu_frame', 'gpu_read_complete', 'pixels_captured')):
        return None
    generation = int(paint['world'])
    if generation <= baseline['frame']['generation'] or paint.get('presentation_owner') != 'gui':
        return None
    row = next((r for r in reversed(rows) if r['kind'] == 1 and r['frame']
                and r['frame']['generation'] == generation), None)
    if not row or row['frame']['nonce'] != baseline['frame']['nonce']:
        return None
    cursor, old = row['cursor'], baseline['cursor']
    if not cursor or not old or cursor['point'] != point or cursor['poll'] <= old['poll']:
        return None
    if (cursor['tick']-baseline['tick']) % 2**32 >= 2**31:
        return None
    if (row['tick']-cursor['tick']) % 2**32 > 500:
        return None
    return dict(generation=generation, ordinal=row['ordinal'], poll=cursor['poll'],
                cursor_class=row['cursor_type'], saved=row['saved_cursor'],
                camera=list(map(signed, row['camera'])), paint=int(paint['paint']))


def physical_point(pointer, paint, size):
    """Independent display-pixel-centre mapping using the actual paint box."""
    require(pointer['viewport'][2:] == [paint['width'], paint['height']]
            and [paint['frame_width'], paint['frame_height']] == size,
            'Pointer/paint extent differs')
    physical = [pointer['screen'][i]-pointer['viewport'][i] for i in (0, 1)]
    box = [paint['left'], paint['top'], paint['image_width'], paint['image_height']]
    require(all(box[i] <= physical[i] < box[i]+box[i+2] for i in (0, 1)),
            'Pointer is outside the actual displayed image')
    result = [int((physical[i]+.5-box[i])*size[i]//box[i+2]) for i in (0, 1)]
    require(result == pointer['logical'] and all(0 <= result[i] < size[i] for i in (0, 1)),
            'Physical point maps to another logical pixel')
    return result


def complete_domains(cases):
    require(len(cases) == 2 and {v['software'] for v in cases} == {False, True},
            'Both actual rendering profiles are required')
    for case in cases:
        require(case['verified'] and case['normal_quit'] and case['mismatching_pixels'] == 0,
                'Incomplete output or normal retirement')
        require([p['size'] for p in case['phases']] == SIZES,
                'Missing actual 800/640/800 continuity')
        require(len(case['rebuilds']) == 2 and all(r['verified'] for r in case['rebuilds']),
                'Missing real rebuild ownership proof')
        for phase in case['phases']:
            require({(w['scaling'], w['label']) for w in phase['windows']} ==
                    {(m, label) for m in ('sharp', 'integer') for label, _ in edge_points(phase['size'])},
                    'Missing edge/corner/scaling domain')
            for window in phase['windows']:
                require(len(window['generations']) >= 3 and window['verified'],
                        'Three independently checked displayed generations required')
    return True
