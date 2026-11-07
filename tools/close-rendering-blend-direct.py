#!/usr/bin/env python3
"""Join retained blend/direct source pixels to original results and native output."""
import argparse
from collections import Counter
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import numpy as np

from coverage_evidence import Reader, write_report
from rendering_pilot import require, acquisition, History, commands, inflate
from rendering_menu_acceptance import artifact
from resident_capture import records
from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-blend-direct-results-20261106/'
BID = 'NP.blend-direct-closeout'
LEAVES = {'operation.sprite-direct', 'journey.creatures-effects.effect-blending'}
KINDS = {'direct', 'blend-1', 'blend-2', 'blend-4'}
PARENT = 'research/runtime/rendering-destination-results-20261106/'
SCOPE = ('Finite natural Campaign/storage direct RGB565 sprite and half/quarter blend output at both explicit profiles and actual 640/800 sizes. '
         'Exact queued source/request bytes, last-writer coverage, independently reconstructed original result words and actual native viewport source-coordinate lineage. '
         'Complete prior composition comparisons and all 42 accepted rows are preserved. No whole original ABI, attachment/combat, upstream palette builder, raw CPU caller, new maps/faults/providers or final decision claim.')


def module(name):
    spec = importlib.util.spec_from_file_location('blend_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    value = importlib.util.module_from_spec(spec); spec.loader.exec_module(value); return value


base = module('close-rendering-destination-history')
sha, signed = base.sha, base.signed


def spans(fields, body):
    """Write footprints select witnesses; retained original planes are the oracle."""
    width, height = struct.unpack_from('<2I', body, 4)
    if width == height == 0: return []  # Captured empty source is an explicit no-op.
    width, height, ox, oy, runs = base.frame_runs(body[:fields[19]])
    left, top = signed(fields[8]) - ox, signed(fields[9]) - oy
    l, t, r, b = map(signed, fields[10:14])
    mode, backend = fields[14:16]
    if backend in (0, 9) and (left < l or left + width >= r): return []
    if mode == 5:
        shadow_top = top + height // 2
        if left < l or left + width + height // 2 > r or shadow_top >= b or shadow_top + height <= t: return []
        first, last = max(0, t - shadow_top), min(height, b - shadow_top)
        result = [(shadow_top + first + (y-first)//2,
                   left + a + last//2 - (y-first)//2, left + z + last//2 - (y-first)//2)
                  for y, a, z in runs if first <= y < first + 2*((last-first+1)//2) and (y-first)%2 == 0]
    else:
        result = []
        for y, a, z in runs:
            if not t <= top+y < b: continue
            if mode == 3 and backend == 7 and (left < l or left+width > r) and (left+a < l or left+z > r): continue
            x0, x1 = max(left+a, l, 0), min(left+z, r, fields[5])
            if x0 < x1: result.append((top+y, x0, x1))
    require(all(0 <= y < fields[6] and 0 <= a < z <= fields[5] for y, a, z in result), 'Covered raster footprint leaves owned canvas')
    return result


def last_writers(rows, rasters, size):
    entry = next(f for f, body in rows if f[2] == 11)
    require(not any(f[1] > entry[1] and f[2] in (14, 26) for f, body in rows), 'Additional queued writer needs its own footprint')
    result = np.zeros((size[1], size[0]), dtype='uint32')
    for fields, body, descriptor in rasters:
        for y, first, last in spans(fields, body): result[y, first:last] = fields[1]
    return result


def source_word(fields, body, x, y):
    require(fields[14] in (0, 1, 2, 4), 'Not a direct/colour-blend sample')
    width, height, ox, oy, runs = base.frame_runs(body[:fields[19]])
    row, column = y-signed(fields[9])+oy, x-signed(fields[8])+ox
    require(0 <= row < height and 0 <= column < width, 'Sample is outside source frame')
    cursor = struct.unpack_from('<I', body, 44+row*8)[0]; step = 1 if fields[17] else 2
    for ry, a, z in runs:
        if ry != row: continue
        require(cursor+(z-a)*step <= fields[19], 'Source pixel row is truncated')
        if a <= column < z:
            at = cursor+(column-a)*step
            if fields[17]:
                require(fields[20] == 512 and len(body) == fields[19]+512, 'Missing effective blend colour table')
                index = body[at]; word = struct.unpack_from('<H', body, fields[19]+index*2)[0]
            else:
                require(fields[20] == 0 and len(body) == fields[19], 'Unexpected direct-word palette')
                index = None; word = struct.unpack_from('<H', body, at)[0]
            return dict(word=word, palette_index=index, source=[column,row])
        cursor += (z-a)*step
    raise ValueError('Sample is transparent')


def candidates(rows, rasters, size, original):
    writers = last_writers(rows, rasters, size); result = {}
    for f, body, descriptor in rasters:
        kind = 'direct' if f[14] == 0 and f[17] == 0 else 'blend-'+str(f[14]) if f[14] in (1,2,4) else None
        if kind is None: continue
        yy, xx = np.nonzero(writers == f[1])
        values = []
        for y, x in zip(yy.tolist(), xx.tolist()):
            source = source_word(f, body, x, y); after = int(original[y,x])
            if not after or kind != 'direct' and after == source['word']: continue
            if kind == 'direct': require(after == source['word'], 'Direct final original pixel differs from its last source writer')
            values.append(dict(world_pixel=y*size[0]+x, destination=[x,y], source=source,
                               original_word=after, sequence=f[1], descriptor=list(descriptor),
                               fields=list(f), payload_sha256=sha(body)))
            if len(values) == 64: break
        if values: result.setdefault(kind, []).extend(values)
    return result


def rgba(word):
    r, g, b = (word>>11)&31, (word>>5)&63, word&31
    return [r<<3|r>>2, g<<2|g>>4, b<<3|b>>2, 255]




class OriginalWorlds(dict):
    """Load retained original planes only when the command lineage needs them."""
    def __init__(self, reader, compared):
        super().__init__(); self.reader = reader
        self.artifacts = {v['path']: v for v in compared['artifacts']}
        self.index = reader.json(self.artifacts['reference/worlds.json']['record'])
        self.rows = {int(v['generation']): v for v in self.index['worlds']}

    def __contains__(self, generation): return generation in self.rows

    def __getitem__(self, generation):
        if not dict.__contains__(self, generation):
            row = self.rows[generation]; item = self.artifacts['reference/'+row['path']]
            packed = self.reader.read_bytes(item['record'])
            require(sha(packed) == item['sha256'], 'Retained original plane identity differs')
            data = inflate(packed)
            require(len(data) == row['width']*row['height']*2, 'Original full plane truncated')
            self[generation] = np.frombuffer(data, dtype='<u2').astype('uint32').reshape(row['height'],row['width'])
        return dict.__getitem__(self, generation)

def viewport_samples(case, observed, worlds, wanted):
    """Select by independently expected lineage before reading native pixels."""
    history = History(worlds)
    parent_paints = {v['paint']: v for v in case['proof']['presented_worlds']}
    observations = {}
    for ordinal in observed['swaps']:
        paint, body = observed['paints'][ordinal]; generation = int(paint['world'])
        if generation in wanted and parent_paints[ordinal]['source_import_live']:
            observations.setdefault(paint['commands'], []).append((paint, body))
    require(observations, 'No retained current-import observations')
    samples = []
    last = max(observations)
    for op, sequence, payload in commands(observed['commands']):
        history.apply(op, sequence, payload)
        for paint, body in observations.get(sequence, []):
            generation = int(paint['world']); sealed = history.sealed
            if sealed is None or sealed['tags'] is None: continue
            if paint['mode'] == 0:
                if history.display[0] != paint['surface'] or sealed['surface'] != paint['surface'] or sealed['generation'] != generation: continue
            elif sealed['generation'] != int(paint['sealed_world']): continue
            tags = sealed['tags']
            if tags.shape != (paint['frame_height'], paint['frame_width']): continue
            require(paint['scaling'] == 2 and paint['image_width'] % paint['frame_width'] == 0
                    and paint['image_height'] % paint['frame_height'] == 0, 'Sample needs exact Integer viewport coordinates')
            scale = paint['image_width'] // paint['frame_width']
            require(scale > 0 and scale == paint['image_height'] // paint['frame_height'], 'Viewport sample scale differs')
            raw = None
            for kind, choices in wanted[generation].items():
                if any(v['generation'] == generation and v['kind'] == kind for v in samples): continue
                by_pixel = {v['world_pixel']: v for v in choices}
                visible = np.flatnonzero(np.isin(tags, list(by_pixel)))
                if not len(visible): continue
                at = int(visible[0]); y, x = divmod(at, paint['frame_width']); source = by_pixel[int(tags.flat[at])]
                vx, vy = paint['left']+x*scale+scale//2, paint['top']+y*scale+scale//2
                if raw is None:
                    raw = body() if callable(body) else body
                    require(sha(raw) == parent_paints[int(paint['paint'])]['sha256'], 'Native body differs from accepted comparison')
                offset = 4*(vy*paint['width']+vx); actual = list(raw[offset:offset+4]); expected = rgba(source['original_word'])
                require(actual == expected, 'Actual native indexed/direct/blend sample mismatch')
                samples.append(dict(kind=kind, generation=generation, observation='viewport', paint=int(paint['paint']),
                                    command_sequence=sequence, lease=int(paint['lease']), target_pixel=[x,y], viewport_pixel=[vx,vy],
                                    original_rgba=expected, actual_rgba=actual, source=source, lineage_world_pixel=int(tags.flat[at])))
        if sequence == last: break
    return samples


def check_samples(samples):
    require({v['kind'] for v in samples} == KINDS, 'Missing natural blend/direct mode')
    for kind in KINDS:
        rows = [v for v in samples if v['kind'] == kind]
        require(len(rows) == 3 and {v['generation'] for v in rows} == {2,3,4}, 'Missing/duplicate source-generation context')
        for v in rows:
            source = v['source']
            require(v['observation'] == 'viewport' and v['lease'] > 0 and v['command_sequence'] > 0
                    and source['world_pixel'] == v['lineage_world_pixel'] and source['original_word'] > 0
                    and v['actual_rgba'] == v['original_rgba'] == rgba(source['original_word']), 'Missing or different native source-coordinate result')
            require(source['descriptor'][0] == source['sequence'] == source['fields'][1], 'Different queued source descriptor')
            if kind == 'direct':
                require(source['fields'][14:18] == [0,8,0,0] and source['source']['word'] == source['original_word'], 'Not direct RGB565 source output')
            else:
                require(source['fields'][14] == int(kind[-1]) and source['source']['word'] != source['original_word'], 'Blend lacks history-dependent output')


def close_rows(plan, parent, runs):
    validate(plan); template = requirements(plan)
    require(parent['dossier_identity'] == fingerprint(template) and parent['passed_requirements'] == 42
            and [(r['configuration'],r['id']) for r in parent['requirements']] == [(r['configuration'],r['id']) for r in template], 'Accepted42-row parent differs')
    expected = {(p,tuple(z)) for p in ('player-default','player-software') for z in ([640,480],[800,600])}
    require(len(runs) == 4 and {(v['domain'][0],tuple(v['domain'][1])) for v in runs} == expected
            and len({v['identity'] for v in runs}) == 4 and all(v['scenario'] == 'normal-storage' for v in runs), 'Missing/duplicate actual caller/profile/size domain')
    for v in runs: check_samples(v['samples'])
    result = []
    for old in parent['requirements']:
        row = dict(old)
        if row['id'] in LEAVES:
            require(row['state'] == 'pending', 'Selected row already passed')
            row.update(state='passed', admitted_execution_evidence=[dict(execution=r['pair'], actual_world_extents=[r['domain'][1]],
                       provider=r['provider'], reason=SCOPE) for r in runs if r['domain'][0] == row['configuration']])
        result.append(row)
    require(sum(v['state'] == 'passed' for v in result) == 46, 'Closeout scope expanded')
    return result


def analyze(reader):
    binding, impact = module('close-rendering-world-exit').parent_plan(ROOT,reader,'DH06.indexed-history-closed01','NP.destination-history-closeout')
    ref = binding['inputs']['closeout-analysis-05.json']; parent = reader.json(ref['record'])
    require(reader.digest(ref['record']) == ref['sha256'] and parent['success'] and parent['passed_requirements'] == 42, 'Accepted parent identity differs')
    require(all(reader.digest(n) == h for n,h in binding['sources'].items()), 'Changed parent source closure')
    runs = []
    for case in parent['executions']:
        if case['scenario'] != 'normal-storage': continue
        pair = case['pair']; acquired, compared = [reader.json(pair[k]['record']) for k in ('acquisition','comparison')]
        require(all(reader.digest(pair[k]['record']) == pair[k]['sha256'] for k in pair)
                and acquired['success'] is compared['success'] is True and compared['acquisition'] == pair['acquisition']
                and acquired['session_identity'] == case['identity'], 'Different accepted caller execution')
        reference = compared['original_reference']
        require(reference['continuous_from_owned_initial_history'] is True and reference['native_player_pixels_read'] is reference['original_destination_capture_read'] is False
                and compared['comparison']['success'] is True and compared['comparison']['mismatching_pixels'] == 0, 'Dependent/failed original history')
        worlds = OriginalWorlds(reader,compared); wanted = {}; packets = []
        with tempfile.TemporaryDirectory(prefix='mnm-blend-direct-',dir='/tmp') as directory:
            path = Path(directory)/'sources.bin'; path.write_bytes(artifact(reader,acquired,'worlds.bin'))
            for g,chunk,raw,request in records(path):
                if chunk or g not in (2,3,4): continue
                old = case['proof']['source_worlds'][g-1]
                require(old['generation'] == g and sha(raw) == old['source_sha256'] and sha(request) == old['request_sha256'], 'Different accepted source packet/request')
                rows = base.module('inspect-canvas-producers').decode(raw,complete=False)[1]
                entry, rasters = base.checked_request(rows,request)
                wanted[g] = candidates(rows,rasters,case['domain'][1],worlds[g])
                packets.append(dict(generation=g,source_sha256=sha(raw),request_sha256=sha(request),
                                    candidates={k:len(v) for k,v in wanted[g].items()}))
                if g == 4: break
        streams = [artifact(reader,acquired,Path(v['path']).name) for v in acquired['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
        observed = acquisition(streams,acquired['session_identity'])
        require(sha(observed['commands']) == compared['captured_command_stream_sha256'], 'Different compared native command history')
        samples = viewport_samples(case,observed,worlds,wanted); check_samples(samples)
        runs.append(dict(domain=case['domain'],scenario=case['scenario'],identity=case['identity'],provider=case['provider'],pair=pair,packets=packets,samples=samples,
                         retained_complete_compositions=case['proof']['world_presentations'],retained_complete_pixels=case['proof']['retained_pixels']))
        print('Verified actual source-to-viewport samples',case['domain'],len(samples),flush=True)
    rows = close_rows(reader.json('docs/native-rendering-closure.json'),parent,runs)
    reg = json.loads((ROOT/'research/runtime/coverage/register.json').read_text()); b = next(v for v in reg['behaviors'] if v['id'] == BID)
    sources = dict(binding['sources']); sources.update({n:reader.digest(n) for n in b['implementation']+b['tests']+b['validation_dependencies']})
    require(all(reader.digest(n) == h for n,h in sources.items()), 'Changed current analysis/parent sources')
    return dict(schema=1,kind='retained_natural_blend_direct_closeout',success=True,scope=SCOPE,sources=sources,
                accepted_parent=dict(binding=impact['retained_input'],applicability=impact),input_parent=ref,executions=runs,requirements=rows,
                dossier_identity=parent['dossier_identity'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|LEAVES),
                passed_requirements=46,pending_requirements=110,inherited_current_rows=42,newly_passed_requirements=4,
                actual_source_packets=12,new_retained_pixel_samples=sum(len(v['samples']) for v in runs),
                retained_complete_compositions=sum(v['retained_complete_compositions'] for v in runs),retained_complete_pixels=sum(v['retained_complete_pixels'] for v in runs),
                decisions=parent['decisions'],original_game_runs=0,new_original_reference_runs=0,new_native_code=False,new_game_fixture=False,historical_failures_retired=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--report',required=True); args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(), 'Fresh owned immutable report required')
    result = dict(schema=1,kind='retained_natural_blend_direct_closeout',success=False)
    try:
        with Reader(ROOT) as reader: result = analyze(reader)
    except Exception as error: result['error'] = str(error)
    digest = write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=digest)),flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__': raise SystemExit(main())
