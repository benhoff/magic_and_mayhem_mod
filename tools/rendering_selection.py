"""Source-derived selection corners and conservative delivery to real viewports.

Unknown World colours never supply expected pixels. The command model carries
coordinate provenance; a colour-key decision on an unknown input discards it.
Expected WORDs come only from actual immutable original producer inputs.
"""
from collections import Counter, defaultdict
from bisect import bisect_left
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import struct
from types import SimpleNamespace

import numpy as np

from coverage_claims import fingerprint
from rendering_interaction import decode
from rendering_interaction_live import sprite_target
from rendering_menu_acceptance import artifact
from rendering_pilot import History, acquisition, commands, require, sha
from rendering_acceptance import requirements, validate
from resident_capture import records

ROOT = Path(__file__).resolve().parents[1]
BID = 'NP.selection-highlight-closeout'
OUT = 'research/runtime/rendering-selection-results-20261108/'
MATRIX = 'research/runtime/rendering-interaction-results-20261107/matrix-01.json'
PARENT = 'research/runtime/rendering-defeat-results-20261108/acceptance-01.json'
LEAF = 'journey.cursor-interaction.selection'
MARKERS = 'Sprites/markers.spr'
CALLER = 0x500845
MIN_CONTRAST = 4


class BytesSource:
    def __init__(self, body): self.body = body
    def stat(self): return SimpleNamespace(st_size=len(self.body))
    def open(self, mode):
        require(mode == 'rb', 'Read-only retained source required')
        return io.BytesIO(self.body)


def module(name):
    spec = importlib.util.spec_from_file_location('selection_' + name.replace('-', '_'), ROOT/'tools'/(name+'.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


blend = module('close-rendering-blend-direct')
animation = module('close-rendering-animation-auxiliary')
base = blend.base


def marker_catalog(reader, acquired, compared):
    info = acquired['installation_inputs'][MARKERS]
    require(compared['asset_inputs'][MARKERS] == info['sha256'], 'Different admitted marker asset')
    output = io.BytesIO()
    reader.resolver.copy(info['sha256'], info['bytes'], output)
    raw = output.getvalue()
    require(sha(raw) == info['sha256'] and len(raw) == info['bytes'], 'Marker asset bytes differ')
    magic, size, version, count, palettes, reserved = struct.unpack_from('<6I', raw)
    require((magic, size, version, count, palettes, reserved) == (0x525053, len(raw), 4, 38, 0, 0),
            'Unreviewed direct-WORD marker asset')
    first = 24 + count*4
    catalog = {}
    for index in range(count):
        offset = first + struct.unpack_from('<I', raw, 24+index*4)[0]
        length = struct.unpack_from('<I', raw, offset)[0]
        require(offset+length <= len(raw) and length >= 40, 'Marker frame extent')
        frame = bytearray(raw[offset:offset+length])
        require(struct.unpack_from('<I', frame, 28)[0] == 0xffffffff, 'Unexpected original marker palette token')
        frame[28:32] = bytes(4)  # Only the existing producer's relocated pointer word.
        require(index >= 4 or struct.unpack_from('<4I', frame, 4) == (9, 9, 8*(index % 2), 8*(index//2)),
                'Selection corner shape/origin differs')
        digest = sha(frame)
        catalog[digest] = dict(index=index, offset=offset, bytes=length, frame_sha256=digest)
    require(len(catalog) == count, 'Ambiguous marker resources')
    return catalog, dict(path=MARKERS, **info)


def original_pair(pair, proof, rows, nonce):
    """Actual original callback transitions, then three pointer-away Worlds each."""
    require(pair['mode'] in ('sharp', 'integer') and pair['case'] in ('view-0','view-1','view-2','view-3','panned'),
            'Unknown selection schedule')
    require(len(pair['toggles']) == 2, 'Selection pair needs two returned callbacks')
    bindings = {v['action']: v for v in proof['cursor_bindings']}
    states = []
    for i, transition in enumerate(pair['toggles'], 1):
        entry, returned = (rows[transition[k]-1] for k in ('entry', 'returned'))
        require(entry['kind'] == 2 and returned['kind'] == 3
                and entry['call'] == returned['call'] == transition['call']
                and entry['ordinal'] < returned['ordinal'] and entry['mask'] & 1
                and entry['mouse'] == returned['mouse'] and entry['mouse'][:2] == transition['mouse']
                and entry['thread'] == returned['thread'] == entry['frame']['owner']
                and entry['frame']['nonce'] == returned['frame']['nonce'] == nonce,
                'Selection callback/argument/owner identity differs')
        a, b = entry['hit'], returned['hit']
        require(a and b and a['pool'] == b['pool'] == transition['pool']
                and (a['slot'], a['type'], a['owner']) == (b['slot'], b['type'], b['owner']) == (0,0,0)
                and a['active'] and b['active'] and 0 < a['health'] < 2**31 and 0 < b['health'] < 2**31
                and a['flag68'] == transition['before'] in (0,1)
                and b['flag68'] == transition['after'] == 1-a['flag68'],
                'No original living wizard selection toggle')
        name = pair['mode']+'-'+pair['case']+'-away-'+str(i)
        binding = bindings[name]
        generations = binding['source_frames']
        require(len(generations) == len(set(generations)) == 3, 'Selection needs three displayed source generations')
        observed = []
        for generation in generations:
            candidates = [r for r in rows if r['kind'] == 1 and r['frame'] and r['frame']['generation'] == generation]
            require(len(candidates) == 1, 'Ambiguous original World-return observation')
            row = candidates[0]; actor = row['tracked']
            require(row['ordinal'] > returned['ordinal'] and row['frame']['nonce'] == nonce
                    and row['thread'] == entry['thread'] and actor
                    and (actor['slot'],actor['type'],actor['owner']) == (0,0,0)
                    and actor['pool'] == b['pool'] and actor['active'] and 0 < actor['health'] < 2**31
                    and actor['flag68'] == b['flag68'] and actor['flag70'] == 0
                    and row['cursor'] and row['cursor']['point'] == binding['point'],
                    'Stale, hovering or different selected actor/frame')
            observed.append(row)
        require(len({r['cursor']['poll'] for r in observed}) == 3, 'Reused cursor poll')
        states.append(dict(selected=b['flag68'], generations=generations, observations=observed,
                           callback=transition, action=name))
    require(states[0]['selected'] != states[1]['selected']
            and pair['toggles'][1]['before'] == pair['toggles'][0]['after']
            and pair['toggles'][1]['after'] == pair['toggles'][0]['before'], 'Toggle pair does not restore original selection')
    all_rows = [r for state in states for r in state['observations']]
    require(all(r['position'] is not None and r['position'] == all_rows[0]['position']
                and r['camera'] == all_rows[0]['camera'] for r in all_rows), 'Actor or camera moved within the selection pair')
    return dict(mode=pair['mode'], case=pair['case'], states=states)


def marker_request(rows, request, selected, catalog):
    """Check the entire descriptor envelope and the four actual marker entries."""
    require(len(request) >= 64 and request[:8] == b'MNMWBQ03', 'Wrong selection request envelope')
    header = struct.unpack_from('<16I', request)
    entries = [f for f,b in rows if f[2] == 11]
    require(len(entries) == 1 and not any(f[2] in (12,13) for f,b in rows), 'Unclosed selection source interval')
    entry = entries[0]
    rasters = [(f,b) for f,b in rows if f[2] == 9 and f[1] > entry[1]]
    require(header[2:7] == (5,64,1,1,len(rows)) and header[7:11] == (entry[3],entry[5],entry[6],entry[5])
            and header[13] == 1 and header[14] == header[15] == len(rasters)
            and header[11] == 16*len(rasters) and len(request) == 64+header[11]
            and base.fnv(request[64:]) == header[12], 'Selection request/source count/checksum differs')
    found = []; auxiliary = []
    for index, (f,body) in enumerate(rasters):
        digest = sha(body[:f[19]])
        if f[22] != CALLER and digest not in catalog:
            continue
        require(digest in catalog and f[22] == CALLER and f[14:18] == (0,8,0,0)
                and len(body) == f[19] and f[20] == f[21] == 0
                and f[3] == entry[3] and f[5:7] == entry[5:7], 'Unselected, unknown or non-direct selection corner')
        descriptor = struct.unpack_from('<4I', request, 64+index*16)
        require(descriptor[0] == f[1] and base.ENTRIES.get(descriptor[1]) == 8
                and descriptor[2] == f[21] and descriptor[3] == base.source_hash(f,body),
                'Marker original entry/AX/source checksum differs')
        item=dict(index=catalog[digest]['index'],fields=f,body=body,descriptor=descriptor,resource=catalog[digest])
        if item['index'] < 4:
            require(selected, 'Selection corner without original selection')
            found.append(item)
        else:
            auxiliary.append(dict(index=item['index'],fields=list(f),descriptor=list(descriptor),resource=item['resource'],
                classification='Exact other marker-bank frame sharing caller; not selected-wizard corner evidence.'))
    require(len(found) == (4 if selected else 0) and (not selected or {v['index'] for v in found} == set(range(4))),
            'Missing, repeated or unselected marker corner')
    return found, rasters, auxiliary


def source_state(rows, request, observation, catalog):
    actor = observation['tracked']
    corners, rasters, auxiliary = marker_request(rows, request, actor['flag68'], catalog)
    writers, rectangles = animation.writer_map(rows, [(f,b,None) for f,b in rasters], (rasters[0][0][5],rasters[0][0][6]))
    require(observation['history'] is not None, 'Missing tracked draw history')
    body = sprite_target(rows, observation['history'])
    terrain = [f for f,b in rows if f[2] == 22]
    overlays = [struct.unpack_from('<37I', b) for f,b in rows if f[2] == 24]
    require(len(terrain) == 1 and overlays and terrain[0][16] == observation['camera'][0]
            and all(m[9] == terrain[0][16] and m[3:5] == terrain[0][10:12] for m in overlays),
            'Source camera does not match the original selection World')
    geometry = None
    if corners:
        by_index = {v['index']:v for v in corners}
        positions = [[base.signed(x) for x in by_index[i]['fields'][8:10]] for i in range(4)]
        left, top = positions[0]; right, bottom = positions[3]
        require(left < right and top < bottom and positions == [[left,top],[right,top],[left,bottom],[right,bottom]]
                and left <= body['point'][0] <= right and top <= body['point'][1] <= bottom,
                'Corners do not enclose the unique tracked body')
        geometry = [left,top,right,bottom]
    return dict(corners=corners, rasters=rasters, auxiliary_markers=auxiliary, by_sequence={f[1]:(f,b) for f,b in rows},
                writers=writers, rectangles=rectangles, geometry=geometry, body=body,
                camera=dict(view=terrain[0][16], center=list(terrain[0][10:12])))


def blend_word(mode, source, destination):
    """Previously recovered RGB565 WORD rounding; unknown history stays unknown."""
    mask=0x7bef
    if mode == 0:return source
    if mode == 7:return 0
    if destination is None:return None
    if mode == 1:return ((source>>1)&mask)+((destination>>1)&mask)
    if mode == 2:
        half=(destination>>1)&mask
        return half+((half>>1)&mask)+((source>>2)&(mask>>1)&mask)
    if mode == 4:
        half=(source>>1)&mask
        return half+((half>>1)&mask)+((destination>>2)&(mask>>1)&mask)
    if mode == 5:return (destination>>1)&mask
    return None


def resolve_points(packet, points, size):
    """Resolve only named points, beginning unknown, through original source writes.

    Unsupported operations invalidate knowledge until a later opaque write.
    Neither native pixels nor an assumed zero initial World supply history.
    """
    by_row=defaultdict(list)
    for point in sorted(points):
        y,x=divmod(point,size[0]);by_row[y].append(x)
    values={p:None for p in points};traces={p:[] for p in points}
    writes=packet['rasters']+[packet['by_sequence'][r['sequence']] for r in packet['rectangles']]
    for fields,body in sorted(writes,key=lambda v:v[0][1]):
        if fields[2] == 9:
            spans=blend.spans(fields,body);mode=fields[14]
        else:
            area=next(r['area'] for r in packet['rectangles'] if r['sequence']==fields[1])
            spans=[(y,area[0],area[2]) for y in range(area[1],area[3])];mode=-1
        for y,left,right in spans:
            xs=by_row.get(y,[])
            for x in xs[bisect_left(xs,left):bisect_left(xs,right)]:
                point=y*size[0]+x;before=values[point]
                if mode not in (0,1,2,4,5,7):
                    values[point]=None;traces[point]=[];continue
                source=None if mode in (5,7) else blend.source_word(fields,body,x,y)['word']
                after=blend_word(mode,source,before)
                values[point]=after
                if mode in (0,7) or after is None:traces[point]=[]
                if after is not None:
                    traces[point].append(dict(sequence=fields[1],mode=mode,backend=fields[15],
                        source_word=source,before=before if mode not in (0,7) else None,after=after))
    return {p:dict(word=word,history=traces[p]) for p,word in values.items() if word is not None}


def contrast_pair(pair, packets, size):
    on = next(s for s in pair['states'] if s['selected'])
    off = next(s for s in pair['states'] if not s['selected'])
    require(len({fingerprint(packets[g]['camera']) for s in pair['states'] for g in s['generations']}) == 1,
            'Source camera changed within selection contrast')
    expected = defaultdict(dict); result = []
    points={y*size[0]+x for g in on['generations'] for corner in packets[g]['corners']
            for y,left,right in blend.spans(corner['fields'],corner['body']) for x in range(left,right)}
    resolved={g:resolve_points(packets[g],points,size) for s in pair['states'] for g in s['generations']}
    # Original sub-cell/animation anchors may move while observed grid XYZ is
    # unchanged. Each selected frame supplies its own four corner witnesses;
    # all three unselected source generations must contrast at those points.
    for generation in on['generations']:
      packet_on = packets[generation]
      for corner in sorted(packet_on['corners'], key=lambda v:v['index']):
        f, body = corner['fields'], corner['body']
        points = sorted({y*size[0]+x for y,l,r in blend.spans(f,body) for x in range(l,r)})
        selected = []
        for point in points:
            y,x = divmod(point,size[0]); values = {}; valid = True
            for state, generations in ((on,[generation]),(off,off['generations'])):
                for g in generations:
                    packet = packets[g]; sequence = int(packet['writers'].flat[point])
                    if point not in resolved[g] or sequence <= 0:
                        valid = False; break
                    winner, payload = packet['by_sequence'][sequence]
                    if winner[2] != 9:
                        valid = False; break
                    if state['selected'] and not (winner[14] == 0 and winner[22] == CALLER and sha(payload[:winner[19]]) == corner['resource']['frame_sha256']):
                        valid = False; break
                    values[g] = resolved[g][point]['word']
                if not valid: break
            if not valid: continue
            on_word = values[generation]
            # RLE spans are opaque; WORD0 is drawn black, not transparency.
            if any(values[g] == on_word for g in off['generations']):
                continue
            for g, word in values.items(): expected[g][point] = word
            selected.append(dict(point=point, values=[dict(generation=g,word=values[g]) for g in sorted(values)]))
        require(len(selected) >= MIN_CONTRAST, 'No meaningful later-writer-safe contrast for every corner')
        result.append(dict(index=corner['index'],generation=generation,rectangle=packet_on['geometry'],
                           resource=corner['resource'], candidate_opaque_points=len(points),
                           samples=selected, on_generations=[generation], off_generations=off['generations']))
    require(len(result)==12,'Every selected generation needs four independent corner contrasts')
    return expected, dict(mode=pair['mode'], case=pair['case'], corners=result,
                         source_histories=[dict(generation=g,points=[dict(point=p,**resolved[g][p]) for p in sorted(words)])
                                           for g,words in sorted(expected.items())],
                         callbacks=[s['callback'] for s in pair['states']], source_generations=sorted(expected))


class PartialWorlds:
    """Only named original input WORDs have colour authority; shape is separate."""
    def __init__(self, count, size, expected): self.count,self.size,self.expected = count,size,expected
    def __contains__(self, g): return 1 <= g <= self.count
    def __getitem__(self, g):
        require(g in self, 'Unknown World generation')
        result = np.zeros((self.size[1],self.size[0]), dtype='uint32')
        for point,word in self.expected.get(g,{}).items(): result.flat[point] = word
        return result


class Lineage(History):
    """History geometry with explicit unknown-colour invalidation at key tests."""
    def __init__(self, worlds):
        super().__init__(worlds); self.known = {}; self.presented_lineage = None

    def copy(self, w):
        sid,did = w[:2]; l,t,r,b,x,y = w[2:8]
        known = self.known[sid][t:b,l:r].copy()
        pixels = self.handles[sid]['pixels'][t:b,l:r].copy()
        super().copy(w)
        target = self.known[did][y:y+b-t,x:x+r-l]
        if not w[8]: target[:] = known
        else:
            copied = known & (pixels != w[9]); unknown = ~known
            target[copied] = True; target[unknown] = False
            self.tags(self.handles[did])[y:y+b-t,x:x+r-l][unknown] = -1

    def apply(self, op, sequence, payload):
        if op == 11: ids = self.words(payload,2)
        super().apply(op,sequence,payload)
        if op == 1:
            sid = self.words(payload,1)[0]
            self.known[sid] = np.full(self.handles[sid]['pixels'].shape, len(payload)>28, dtype=bool)
        elif op in (2,20):
            sid,x,y,w,h = self.words(payload,5)
            target = self.known[sid][y:y+h,x:x+w]
            if op == 2: target[:] = True
            else:
                n=w*h*(self.handles[sid]['bits']//8)
                target[np.frombuffer(payload[20+n:],dtype='u1').reshape(h,w)!=0] = True
        elif op == 7: self.known.pop(self.words(payload,1)[0])
        elif op == 11: self.known[ids[0]],self.known[ids[1]] = self.known[ids[1]],self.known[ids[0]]
        elif op == 19:
            sid,lo,hi,_,_ = self.words(payload,5); generation=lo+(hi<<32)
            self.known[sid][:] = False
            for point in self.worlds.expected.get(generation,{}): self.known[sid].flat[point] = True
        if op == 6:
            surface = self.display[1]
            self.presented_lineage = dict(surface=self.display[0], generation=self.presented_generation,
                tags=None if surface['tags'] is None else surface['tags'].copy())


def viewport_points(history, paint, expected, raw):
    generation = int(paint['world']); sealed = history.sealed
    if paint['mode'] == 0:
        snapshot = history.presented_lineage
        require(snapshot and snapshot['surface'] == paint['surface'] and snapshot['generation'] == generation,
                'Direct presentation has another retained snapshot identity')
        tags = snapshot['tags']
    else:
        require(sealed and sealed['generation'] == int(paint['sealed_world']) and history.generation == generation,
                'Sealed UI source/generation differs')
        tags = sealed['tags']
    require(tags is not None, 'No World coordinate provenance in the actual presentation')
    require(tags.shape == (paint['frame_height'],paint['frame_width']) and paint['scaling'] in (0,2),
            'Unsupported actual selection viewport')
    width,height = paint['image_width'],paint['image_height']
    xs=np.minimum(((np.arange(width)+.5)*paint['frame_width']/width).astype(int),paint['frame_width']-1)
    ys=np.minimum(((np.arange(height)+.5)*paint['frame_height']/height).astype(int),paint['frame_height']-1)
    mapped=tags[ys[:,None],xs[None,:]]
    mask=np.isin(mapped,list(expected)); yy,xx=np.nonzero(mask)
    require(set(map(int,mapped[mask])) == set(expected), 'A source witness is hidden or not displayed')
    pixels=np.frombuffer(raw,dtype='u1').reshape(paint['height'],paint['width'],4)
    actual=pixels[paint['top']+yy,paint['left']+xx]
    wanted=np.array([blend.rgba(expected[int(point)]) for point in mapped[mask]],dtype='uint8')
    require(np.array_equal(actual,wanted), 'Actual selection pixels differ from original source WORDs')
    return dict(generation=generation,paint=int(paint['paint']),lease=int(paint['lease']),
                source_points=sorted(expected),viewport_pixels=len(xx),actual_sha256=sha(actual.tobytes()),
                expected_sha256=sha(wanted.tobytes()),scaling=paint['scaling'])


def compared_swaps(observed, compared):
    shown = [(observed['paints'][p]) for p in observed['swaps'] if int(observed['paints'][p][0]['world']) > 0]
    matches = [v for v in compared['comparison']['comparisons'] if v['world'] > 0]
    require(len(shown) == len(matches) == compared['comparison']['world_presentations']
            and observed['observations'] == compared['observations'], 'Missing/repeated native World swap')
    result = []
    for (paint, body), match in zip(shown, matches):
        require(int(paint['paint']) == match['paint'] and int(paint['world']) == match['world']
                and int(paint['lease']) == match['lease'] > 0
                and paint['native_gpu_frame'] is paint['gpu_read_complete'] is paint['pixels_captured'] is True
                and paint['presentation_owner'] == 'gui' and match['pixels'] == paint['width']*paint['height']
                and match['mismatching_pixels'] == 0 and match['actual_sha256'] == match['expected_sha256'],
                'Changed native paint/lease/comparison identity')
        # acquisition() validated all campaign planes, including coalesced ones.
        # Its verified per-plane digest avoids decoding every delta twice.
        digest = paint.get('capture_body_sha256')
        if digest is None: digest = sha(body() if callable(body) else body)
        require(digest == match['actual_sha256'], 'Actual native plane differs from independent comparison')
        result.append((paint,body,match))
    return result


def deliver(observed, compared, expected, frame_count, size):
    pairs = compared_swaps(observed, compared)
    selected = defaultdict(list)
    for paint,body,match in pairs:
        if int(paint['world']) in expected:
            selected[paint['commands']].append((paint,body,match))
    history = Lineage(PartialWorlds(frame_count,size,expected))
    outputs = {}; unadmitted = []
    for op,sequence,payload in commands(observed['commands']):
        history.apply(op,sequence,payload)
        for paint,body,match in selected.get(sequence,[]):
            generation = int(paint['world'])
            if generation in outputs: continue
            snapshot = history.presented_lineage if paint['mode'] == 0 else history.sealed
            if snapshot is None or snapshot['tags'] is None:
                unadmitted.append(dict(paint=int(paint['paint']),generation=generation,commands=sequence,
                    reason='No World coordinate lineage; complete prior comparison retained, no selection-state credit.'))
                continue
            tags = snapshot['tags']
            if tags.shape != (paint['frame_height'],paint['frame_width']):
                unadmitted.append(dict(paint=int(paint['paint']),generation=generation,commands=sequence,
                    reason='Different retained presentation extent; no selection-state credit.'))
                continue
            xs=np.minimum(((np.arange(paint['image_width'])+.5)*paint['frame_width']/paint['image_width']).astype(int),paint['frame_width']-1)
            ys=np.minimum(((np.arange(paint['image_height'])+.5)*paint['frame_height']/paint['image_height']).astype(int),paint['frame_height']-1)
            mapped=tags[ys[:,None],xs[None,:]]
            missing=set(expected[generation])-set(map(int,np.unique(mapped)))
            if missing:
                unadmitted.append(dict(paint=int(paint['paint']),generation=generation,commands=sequence,
                    missing_points=len(missing),reason='Witness points lack visible source provenance; no selection-state credit.'))
                continue
            raw = body() if callable(body) else body
            require(sha(raw) == match['actual_sha256'], 'Selected actual GPU plane hash differs')
            output = viewport_points(history,paint,expected[generation],raw)
            output.update(command_sequence=sequence,complete_viewport_sha256=match['actual_sha256'])
            outputs[generation] = output
    require(set(outputs) == set(expected), 'Original selection source generations lack visible native output: '
            +str(sorted(set(expected)-set(outputs)))+'; unadmitted='+str(unadmitted[:3]))
    return [outputs[g] for g in sorted(outputs)], dict(world_swaps=len(pairs),
        pixels=sum(m['pixels'] for p,b,m in pairs),mismatching_pixels=0,
        all_actual_world_swaps_bound=True,all_captured_planes_verified=True,
        unadmitted_for_selection=unadmitted)


def cursor_accounting(raw):
    """Every refusal stays visible; only a closed-owner invalidation qualifies."""
    summary = module('native_cursor_journal').decode(raw)
    rows = list(struct.iter_unpack('<16I',raw[64:]))
    require(len(rows) < 32768 and not any(rows[0][6:]), 'Cursor journal may have suppressed diagnostic events')
    require(summary['geometry_cancelled'] == summary['queue_full'] == 0,
            'Unexplained cursor geometry cancellation or overflow')
    closed = False; refusals = []; previous = rows[0]
    for row in rows[1:]:
        if row[1] == 9:
            require(row[3:6] == (0,0,0) and row[15] > 0, 'Outer cursor close left native work')
            closed = True
        delta = row[13]-previous[13]
        require(delta == (1 if row[1] == 6 else 0), 'A cursor refusal is missing from the complete raw journal')
        if row[1] == 6:
            require(closed and row[3:6] == (3,0,0) and row[15] > 0,
                    'Drawing/active-owner cursor refusal cannot supply nominal selection proof')
            refusals.append(dict(sequence=row[0],slot=3,wait=0,outer_generation=0,outer_closes=row[15],
                classification='Post-close invalidation rejected; no drawing client or copy was executed.'))
        if closed:
            require(row[1] not in (5,7), 'Cursor client resumed after outer close')
        previous = row
    require(len(refusals) == summary['client_refusals'], 'Unaccounted cursor refusal total')
    return dict(summary,verified=True,refusals=refusals,all_refusals_accounted=True,
                scope='Complete bounded raw journal. A slot3 invalidate after acknowledged outer close is a terminal-owner refusal, not an invalid draw/copy success. Every refusal remains retained; foreground/timer/active-owner refusals and geometry/overflow still refuse acceptance.')


def analyze_case(reader, reference):
    require(reader.digest(reference['record']) == reference['sha256'], 'Changed retained comparison')
    c = reader.json(reference['record']); a = reader.json(c['acquisition']['record'])
    require(reader.digest(c['acquisition']['record']) == c['acquisition']['sha256'], 'Changed original acquisition')
    cfg = a['configuration']; size = cfg['actual_size']; proof = c['proof']
    require(a['success'] is c['success'] is c['sources_stable'] is a['sources_stable'] is True
            and a['player_binary_stable'] is True
            and cfg['compiled_test_driver_invoked'] is False and cfg['skip_intros'] is False
            and cfg['terrain_lighting'] == 'native' and cfg['viewport_scaling'] == 'sharp-and-integer'
            and type(cfg['software_flag']) is bool and size in ([640,480],[800,600])
            and size == ([800,600] if cfg['high_resolution'] else [640,480])
            and ('--software-rendering' in a['command']) is cfg['software_flag']
            and a['private_wine_stopped'] is a['original_manifest_verified_before_after'] is True
            and a['exit_code'] == 0 and a['prefix_pool']['returned'] is True
            and not a.get('forced_private_prefix_stop',False), 'Incomplete actual normal selection domain')
    require(c['comparison_processes_stopped'] is True and c['comparison']['success'] is True
            and c['comparison']['mismatching_pixels'] == c['comparison']['remaining_surfaces'] == 0
            and proof['verified'] is True and len(proof['selection_pairs']) == 10
            and c['original_reference']['continuous_from_owned_initial_history'] is True
            and c['original_reference']['native_player_pixels_read'] is c['original_reference']['original_destination_capture_read'] is False
            and a['original_pixels_used_as_native_inputs'] is c['original_pixels_used_as_native_inputs'] is False,
            'Dependent/incomplete selection reference or matrix')
    frames = decode(artifact(reader,a,'frame-input.bin'))
    require(reader.digest(a['player_binary']['record']) == a['player_binary']['sha256'],
            'Exact tested player bytes unavailable')
    require(frames['nonce'] == a['resident']['nonce'] == proof['source_nonce']
            and frames['owner'] == proof['original_owner'], 'Original selection owner/source nonce differs')
    pairs = [original_pair(p,proof,frames['rows'],frames['nonce']) for p in proof['selection_pairs']]
    require({(p['mode'],p['case']) for p in pairs} == {(s,v) for s in ('sharp','integer')
            for v in ('view-0','view-1','view-2','view-3','panned')}, 'Incomplete selection view/scaling matrix')
    wanted = {}
    for pair in pairs:
        for state in pair['states']:
            for g,row in zip(state['generations'],state['observations']):
                require(g not in wanted, 'Selection windows reuse a settled source generation')
                wanted[g] = row
    catalog,asset = marker_catalog(reader,a,c)
    decoder = module('inspect-canvas-producers'); packets = {}; sources = {}; worlds=chunks=0
    for g,chunk,stream,request in records(BytesSource(artifact(reader,a,'worlds.bin'))):
        if chunk: chunks+=1; continue
        worlds+=1
        if g not in wanted: continue
        _,rows = decoder.decode(stream,complete=False)
        packets[g] = source_state(rows,request,wanted[g],catalog)
        require(all(list(f[5:7]) == size for f,b in packets[g]['rasters']), 'Selection source extent differs')
        sources[g] = dict(generation=g,source_sha256=sha(stream),request_sha256=sha(request),
            observation_ordinal=wanted[g]['ordinal'],actor=wanted[g]['tracked'],
            original_position=wanted[g]['position'],camera=packets[g]['camera'],auxiliary_markers=packets[g]['auxiliary_markers'],
            corners=[dict(index=v['index'],fields=list(v['fields']),descriptor=list(v['descriptor']),
                resource=v['resource'],payload_sha256=sha(v['body'])) for v in packets[g]['corners']])
    require(set(packets) == set(wanted) and worlds == proof['source_generations'] == c['original_reference']['worlds']
            == int(a['resident']['completed']) == int(a['resident']['imports']) == int(a['resident']['verified_returns'])
            and chunks == proof['source_chunks'] == c['original_reference']['source_chunks'], 'Selection source history is incomplete')
    expected = {}; contrasts = []
    for pair in pairs:
        values,contrast = contrast_pair(pair,packets,size); contrasts.append(contrast)
        for g,words in values.items():
            require(g not in expected, 'Conflicting selection witness state')
            expected[g] = words
    del packets
    observed = acquisition([artifact(reader,a,v['path']) for v in a['artifacts']
                            if '/capture/' in v['path'] and v['path'].endswith('.bin')],a['session_identity'])
    require(sha(observed['commands']) == c['captured_command_stream_sha256'], 'Different captured command history')
    outputs,whole = deliver(observed,c,expected,worlds,size)
    normal = module('join-rendering-normal-acceptance')
    cleanup = normal.cleanup_proof(reader,a)
    cursor = cursor_accounting(artifact(reader,a,'native-cursor-lifetime.bin'))
    owner_path = 'runtime/native-draw/cursor_owner.h'
    require(a['sources'][owner_path] == reader.digest(owner_path), 'Cursor journal owner interpretation source changed')
    require(cleanup['verified'] and cleanup['normal_quit'] and cleanup['producer_stopped']
            and cleanup['pending_jobs'] == cleanup['graphics_owners'] == 0, 'Selection owners did not retire normally')
    lifetimes = module('close-rendering-world-exit').imported_lifetimes(observed['commands'])
    require(len(lifetimes['imports']) == worlds and str(lifetimes['nonce']) == frames['nonce'],
            'Selection command/source owner differs')
    return dict(verified=True,identity=a['session_identity'],
        domain=['player-software' if cfg['software_flag'] else 'player-default',size],
        pair=dict(acquisition=c['acquisition'],comparison=reference),provider=a['resident']['renderer'],
        original_owner=frames['owner'],nonce=frames['nonce'],asset=asset,selection_callbacks=20,
        settled_source_generations=len(expected),source_imports=worlds,source_chunks=chunks,
        contrasts=contrasts,source_frames=[sources[g] for g in sorted(sources)],presentations=outputs,
        matched_viewport_pixels=sum(v['viewport_pixels'] for v in outputs),whole_output=whole,
        cleanup=cleanup,cursor_journal=cursor,command_end=lifetimes['command_end'],
        historical_phases=dict(acquisition_sources=a['sources'],comparison_sources=c['sources'],
            compiler_sources=a['compiler_sources'],tested_binary=a['player_binary'],
            current_native_execution=False,current_original_oracle_execution=False))


def close_rows(plan, parent, cases):
    validate(plan); template=requirements(plan)
    require(parent['success'] is True and parent['dossier_identity'] == fingerprint(template)
            and len(parent['requirements']) == 156
            and [(r['configuration'],r['id']) for r in parent['requirements']]
                == [(r['configuration'],r['id']) for r in template]
            and parent['passed_requirements'] == sum(r['state']=='passed' for r in parent['requirements']) == 102,
            'Frozen102row parent inventory differs')
    require(len(cases) == 4 and len({c['identity'] for c in cases}) == 4
            and {(c['domain'][0],tuple(c['domain'][1])) for c in cases}
                == {(p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600))},
            'Missing/repeated selection domain')
    for case in cases:
        require(case['verified'] is True and case['selection_callbacks'] == 20
                and case['settled_source_generations'] == len(case['presentations']) == 60
                and len(case['contrasts']) == 10 and case['matched_viewport_pixels'] > 0
                and case['cursor_journal']['all_refusals_accounted'] is True
                and case['cleanup']['verified'] is case['cleanup']['normal_quit'] is True,
                'Incomplete actual selection proof')
    scope=('Original selection callbacks and flag68on/off with flag70clear, exact four-corner marker resources '
           'and source requests, contrasting later-writer-safe original WORDs, three displayed pointer-away '
           'source generations per state and source-coordinate provenance through actual native commands. '
           'All World-associated leased swaps retain complete independent comparison; raw cursor refusals '
           'and normal retirement remain accounted. Finite four-view/pan Sharp/Integer Quick domain; '
           'no allocation generations, whole cursor/picking/hotspot, storage selection continuity, '
           'physical provider, current full player or final-decision claim.')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id'] == LEAF:
            require(row['state']=='pending','Selection row already accepted')
            row.update(state='passed',admitted_execution_evidence=[dict(execution=c['pair'],
                actual_world_extents=[c['domain'][1]],provider=c['provider'],reason=scope)
                for c in cases if c['domain'][0] == row['configuration']])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==104,'Selection promotion exceeds two rows')
    return rows
