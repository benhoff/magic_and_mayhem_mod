#!/usr/bin/env python3
"""Check retained animated creature frames and auxiliary sprites in native output."""
import argparse
from collections import defaultdict
import importlib.util
import json
from pathlib import Path
import struct
import tempfile

import numpy as np

from coverage_claims import fingerprint
from coverage_evidence import Reader, write_report
from evidence_objects import configured
from rendering_acceptance import requirements, validate
from rendering_menu_acceptance import artifact
from rendering_pilot import History, acquisition, commands, require
from resident_capture import records

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-five-results-20261107/'
PARENT = 'research/runtime/rendering-next-two-results-20261107/closeout-analysis-04.json'
BEHAVIORS = {'NP.animation-output-closeout', 'NP.auxiliary-output-closeout'}
ASSETS = ('Creatures/RedCap.spr', 'Creatures/Wizard1.spr')
ROWS = {('player-default', 'journey.creatures-effects.creature-animation')}
ROWS |= {(p, leaf) for p in ('player-default', 'player-software')
         for leaf in ('journey.active-world.animation', 'operation.sprite-auxiliary')}


def module(name):
    spec = importlib.util.spec_from_file_location('animated_' + name.replace('-', '_'),
                                                ROOT / 'tools' / (name + '.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


blend = module('close-rendering-blend-direct')
base = blend.base
sha = base.sha


def decoded_frame(raw, indexed):
    """Decode the captured RLE pixels; names and palette-pointer words grant no animation."""
    width, height, ox, oy, runs = base.frame_runs(raw)
    pixels = np.zeros((height, width), dtype='uint32')
    mask = np.zeros((height, width), dtype='uint8')
    step = 1 if indexed else 2
    for y in range(height):
        cursor = struct.unpack_from('<I', raw, 44 + y * 8)[0]
        for row, first, last in runs:
            if row != y:
                continue
            size = (last - first) * step
            require(cursor + size <= len(raw), 'Truncated decoded animation row')
            pixels[y, first:last] = np.frombuffer(raw[cursor:cursor + size],
                                                dtype='u1' if indexed else '<u2')
            mask[y, first:last] = 1
            cursor += size
    signature = fingerprint(dict(shape=[height, width], origin=[ox, oy],
                                 pixels=sha(pixels.astype('<u2').tobytes()), mask=sha(mask.tobytes())))
    return dict(shape=[height, width], origin=[ox, oy], bitmap_sha256=signature,
                opaque_pixels=int(mask.sum()))


def asset_catalog(pair, acquired, compared):
    catalog = defaultdict(list)
    assets = []
    for name in ASSETS:
        info = acquired['installation_inputs'][name]
        require(compared['asset_inputs'][name] == info['sha256'], 'Different admitted animation asset')
        raw = pair.read(info['sha256'], info['bytes'])
        require(sha(raw) == info['sha256'] and len(raw) == info['bytes'], 'Archived creature asset differs')
        magic, size, version, count, palettes = struct.unpack_from('<5I', raw)
        require(magic == 0x525053 and size == len(raw) and version == 4 and
                0 < count <= 4096 and 1 <= palettes <= 4, 'Unsupported captured creature SPR envelope')
        table = 24 + palettes * 768
        first = table + count * 4
        require(first <= size, 'Truncated SPR frame table')
        source = dict(path=name, sha256=info['sha256'], bytes=size, frames=count)
        assets.append(source)
        for index in range(count):
            at = first + struct.unpack_from('<I', raw, table + index * 4)[0]
            require(at + 40 <= size, 'SPR frame header outside immutable asset')
            length, width, height = struct.unpack_from('<3I', raw, at)
            require(length >= 40 and at + length <= size, 'SPR frame extent outside immutable asset')
            if width == height == 0:
                continue
            frame = bytearray(raw[at:at + length])
            require(struct.unpack_from('<I', frame, 28)[0] < palettes, 'Unbound SPR palette member')
            # The original producer closes this one relocated palette member.
            # Every other encoded byte must match the actual immutable frame.
            frame[28:32] = bytes(4)
            frame = bytes(frame)
            decoded = decoded_frame(frame, True)
            catalog[sha(frame)].append(dict(asset=source, frame=index, offset=at,
                                           encoded_bytes=length, decoded=decoded))
    return catalog, assets


def camera_context(rows):
    terrain = [f for f, body in rows if f[2] == 22]
    overlays = [struct.unpack_from('<37I', body) for f, body in rows if f[2] == 24]
    require(len(terrain) == 1 and overlays, 'Missing exact source camera/map state')
    t = terrain[0]
    value = dict(view=t[16], center=list(t[10:12]), grid=list(t[14:16]))
    require(value['view'] == 0 and all(m[9] == value['view'] and
            list(m[3:5]) == value['center'] and list(m[1:3]) == value['grid'] for m in overlays),
            'Mixed/uncovered camera context')
    return value


def visible_points(fields, writers, original):
    yy, xx = np.nonzero(writers == fields[1])
    # The accepted native UI covers the right/bottom World. Pick the supported
    # scene interior before reading native output, keeping every selected WORD.
    keep = (xx < original.shape[1] - 200) & (yy < original.shape[0] - 50)
    yy, xx = yy[keep], xx[keep]
    return sorted(map(int, yy * original.shape[1] + xx))


def writer_map(rows, rasters, size):
    """Rectangle writers occlude earlier rasters; their output gets no new credit."""
    width, height = size
    entry = next(f for f, body in rows if f[2] == 11)
    raster_by_sequence = {f[1]: (f, body) for f, body, descriptor in rasters}
    result = np.zeros((height, width), dtype='int64')
    occlusions = []
    for fields, body in rows:
        if fields[1] <= entry[1]:
            continue
        require(fields[2] in (3, 4, 9, 10, 14, 25, 26), 'Unclassified queued writer')
        if fields[2] == 9:
            require(fields[1] in raster_by_sequence, 'Unbound raster footprint')
            for y, first, last in blend.spans(fields, body):
                result[y, first:last] = fields[1]
        elif fields[2] in (14, 26) and fields[3] == entry[3]:
            if fields[2] == 14:
                require(len(body) == 20, 'Truncated RGB-add occlusion input')
                w, h = struct.unpack_from('<2I', body)
                require(w <= 2048 and h <= 2048, 'Unbounded RGB-add occlusion')
            else:
                w, h = fields[19:21]
                require(0 < w <= 64 and 0 < h <= 64 and len(body) == w * h * 2,
                        'Unbounded colour-rectangle occlusion')
            x, y = map(base.signed, fields[8:10])
            left, top, right, bottom = map(base.signed, fields[10:14])
            area = [max(0, left, x), max(0, top, y), min(width, right, x + w), min(height, bottom, y + h)]
            if area[0] < area[2] and area[1] < area[3]:
                result[area[1]:area[3], area[0]:area[2]] = -fields[1]
            occlusions.append(dict(sequence=fields[1], fields=list(fields), area=area,
                                   payload_sha256=sha(body), policy='exclude every possible rectangle write'))
    return result, occlusions


def source_groups(reader, case, acquired, compared, catalog):
    decode = module('inspect-canvas-producers').decode
    originals = blend.OriginalWorlds(reader, compared)
    groups = {}
    animations = {name: [] for name in ASSETS}
    auxiliary = {'shadow': [], 'wave': []}
    with tempfile.TemporaryDirectory(dir='/tmp', prefix='animation-auxiliary-') as directory:
        path = Path(directory) / 'worlds.bin'
        path.write_bytes(artifact(reader, acquired, 'worlds.bin'))
        for generation, chunk, raw, request in records(path):
            if chunk or generation < 2:
                continue
            if generation > 64:
                break
            _, rows = decode(raw, complete=False)
            entry, rasters = base.checked_request(rows, request)
            accepted = case['proof']['source_worlds'][generation - 1]
            require(accepted['generation'] == generation and accepted['source_sha256'] == sha(raw)
                    and accepted['request_sha256'] == sha(request), 'Different accepted source generation')
            original = originals[generation]
            require(original.shape == (entry[6], entry[5]), 'Source/original extent differs')
            camera = camera_context(rows)
            writers, occlusions = writer_map(rows, rasters, (entry[5], entry[6]))
            by_asset = defaultdict(list)
            by_auxiliary = defaultdict(list)
            for fields, body, descriptor in rasters:
                refs = catalog.get(sha(body[:fields[19]]), [])
                if not refs:
                    continue
                require(len(refs) == 1, 'Ambiguous canonical creature frame')
                points = visible_points(fields, writers, original)
                if len(points) < 16 or len(np.unique(original.ravel()[points])) < 2:
                    continue
                ref = refs[0]
                proof = dict(sequence=fields[1], fields=list(fields), descriptor=list(descriptor),
                             payload_sha256=sha(body), frame_sha256=sha(body[:fields[19]]),
                             resource=ref, points=points)
                if fields[14] in (0, 1, 2, 3, 4):
                    by_asset[ref['asset']['path']].append(proof)
                if (fields[14], fields[15]) == (5, 10):
                    proof = dict(proof, route='shadow')
                    by_auxiliary['shadow'].append(proof)
                elif (fields[14], fields[15]) == (3, 7):
                    offsets = list(struct.unpack('<16I', body[fields[19]:]))
                    require(1 <= fields[16] <= 16 and max(offsets) <= 16,
                            'Unowned wave phase/offset extent')
                    require(all(0 <= at + max(offsets) < original.size for at in points),
                            'Selected wave reads outside the owned original plane')
                    proof = dict(proof, route='wave', offsets=offsets,
                                 table_sha256=sha(body[fields[19]:]), owned_reads=True)
                    by_auxiliary['wave'].append(proof)
            common = dict(generation=generation, source_sha256=sha(raw), request_sha256=sha(request),
                          source_incarnation=accepted['source_incarnation'], camera=camera,
                          source_shape=list(original.shape), primitive_occlusions=occlusions)
            for name in ASSETS:
                prior = animations[name]
                if len(prior) == 3:
                    continue
                for candidate in sorted(by_asset[name], key=lambda v: -len(v['points'])):
                    signature = candidate['resource']['decoded']['bitmap_sha256']
                    if any(v['source']['resource']['decoded']['bitmap_sha256'] == signature for v in prior):
                        continue
                    changed = []
                    if prior:
                        previous = prior[-1]
                        shared = sorted(set(candidate['points']) & set(previous['points']))
                        changed = [at for at in shared if int(original.flat[at]) != previous['values'][at]]
                        if not changed:
                            continue
                    group = dict(source=dict(common, **{k: v for k, v in candidate.items() if k != 'points'}),
                                 points=candidate['points'], values={at: int(original.flat[at])
                                 for at in candidate['points']}, changed=changed)
                    prior.append(group)
                    groups[(generation, 'animation:' + name)] = group
                    break
            for route in auxiliary:
                if len(auxiliary[route]) == 3 or not by_auxiliary[route]:
                    continue
                candidate = max(by_auxiliary[route], key=lambda v: len(v['points']))
                group = dict(source=dict(common, **{k: v for k, v in candidate.items() if k != 'points'}),
                             points=candidate['points'], values={at: int(original.flat[at])
                             for at in candidate['points']}, changed=[])
                auxiliary[route].append(group)
                groups[(generation, 'auxiliary:' + route)] = group
            if all(len(v) == 3 for v in [*animations.values(), *auxiliary.values()]):
                break
    require(all(len(v) == 3 for v in [*animations.values(), *auxiliary.values()]),
            'Missing three distinct visible creature frames or positive auxiliary routes')
    return groups, originals


def presented_groups(case, compared, observed, groups, originals):
    history = History(originals)
    prior = {v['paint']: v for v in case['proof']['presented_worlds']}
    matches = {v['paint']: v for v in compared['comparison']['comparisons']}
    by_generation = defaultdict(list)
    for (generation, kind), group in groups.items():
        by_generation[generation].append((kind, group))
    paints = defaultdict(list)
    for ordinal in observed['swaps']:
        paint, body = observed['paints'][ordinal]
        if int(paint['world']) in by_generation and prior[ordinal]['source_import_live']:
            paints[paint['commands']].append((paint, body))
    found = {}
    for op, sequence, payload in commands(observed['commands']):
        history.apply(op, sequence, payload)
        for paint, body in paints.get(sequence, []):
            generation = int(paint['world'])
            sealed = history.sealed
            if sealed is None or sealed['tags'] is None:
                continue
            if paint['mode'] == 0:
                if history.display[0] != paint['surface'] or sealed['surface'] != paint['surface'] or sealed['generation'] != generation:
                    continue
            elif sealed['generation'] != int(paint['sealed_world']):
                continue
            tags = sealed['tags']
            require(tags.shape == (paint['frame_height'], paint['frame_width']) and paint['scaling'] == 2
                    and paint['image_width'] % paint['frame_width'] == 0
                    and paint['image_height'] % paint['frame_height'] == 0, 'Uncovered animation viewport geometry')
            scale = paint['image_width'] // paint['frame_width']
            require(scale > 0 and scale == paint['image_height'] // paint['frame_height'], 'Unequal viewport scale')
            raw = None
            for kind, group in by_generation[generation]:
                key = (generation, kind)
                if key in found:
                    continue
                yy, xx = np.nonzero(np.isin(tags, group['points']))
                indices = tags[yy, xx].astype('int64')
                if set(map(int, np.unique(indices))) != set(group['points']):
                    continue
                match = matches[int(paint['paint'])]
                if raw is None:
                    raw = body() if callable(body) else body
                require(sha(raw) == match['actual_sha256'] == match['expected_sha256'] == prior[int(paint['paint'])]['sha256']
                        and match['world'] == generation and match['lease'] == int(paint['lease']) > 0
                        and match['mismatching_pixels'] == 0, 'Changed complete compared viewport/lease')
                pixels = np.frombuffer(raw, dtype='u1').reshape(paint['height'], paint['width'], 4)
                expected = np.array([blend.rgba(group['values'][int(at)]) for at in indices], dtype='uint8')
                actual = []
                for dy in range(scale):
                    for dx in range(scale):
                        part = pixels[paint['top'] + yy * scale + dy, paint['left'] + xx * scale + dx]
                        require(np.array_equal(part, expected), 'Actual animated/auxiliary pixels differ from original')
                        actual.append(part.tobytes())
                proof = dict(generation=generation, paint=int(paint['paint']), lease=int(paint['lease']),
                             command_sequence=sequence, mapped_source_words=list(map(int, np.unique(indices))),
                             expected_source_words=len(group['points']), mapped_words=len(set(map(int, indices))),
                             viewport_pixels=len(indices) * scale * scale,
                             actual_sha256=sha(b''.join(actual)), expected_sha256=sha(expected.tobytes() * scale**2),
                             complete_viewport_sha256=match['actual_sha256'], independently_matched=True,
                             changed_visible_words=len(group['changed']))
                found[key] = dict(kind=kind, source=group['source'], presentation=proof)
        if len(found) == len(groups):
            break
    require(set(found) == set(groups), 'A selected animated/auxiliary output is not fully visible')
    result = [found[key] for key in sorted(found)]
    check_output(result)
    return result


def check_output(rows):
    require(len(rows) == 12, 'Missing source/output witness groups')
    for kind in [*('animation:' + name for name in ASSETS), 'auxiliary:shadow', 'auxiliary:wave']:
        values = [v for v in rows if v['kind'] == kind]
        require(len(values) == 3 and [v['source']['generation'] for v in values] ==
                sorted({v['source']['generation'] for v in values}), 'Repeated/missing source generations')
        for value in values:
            source, shown = value['source'], value['presentation']
            fields, descriptor = source['fields'], source['descriptor']
            resource = source['resource']
            require(shown['generation'] == source['generation'] and shown['lease'] > 0
                    and shown['independently_matched'] and shown['actual_sha256'] == shown['expected_sha256']
                    and shown['mapped_words'] == shown['expected_source_words'] >= 16
                    and len(set(shown['mapped_source_words'])) == shown['mapped_words']
                    and shown['viewport_pixels'] >= shown['mapped_words'], 'Incomplete actual source-bound visible output')
            require(fields[1] == source['sequence'] == descriptor[0] and fields[21] == descriptor[2]
                    and base.ENTRIES[descriptor[1]] == fields[15] and source['source_incarnation']['canvas'] == fields[3]
                    and source['camera']['view'] == 0 and resource['asset']['path'] in ASSETS
                    and resource['decoded']['opaque_pixels'] > 0, 'Wrong original entry/AX/source/resource context')
            width = source['source_shape'][1]
            for occlusion in source['primitive_occlusions']:
                if occlusion['sequence'] > source['sequence']:
                    left, top, right, bottom = occlusion['area']
                    require(not any(left <= at % width < right and top <= at // width < bottom
                                    for at in shown['mapped_source_words']), 'Later rectangle cannot receive sprite credit')
            if kind.startswith('auxiliary:'):
                route = kind.split(':')[1]
                require(source['route'] == route and fields[14:16] == ([5, 10] if route == 'shadow' else [3, 7]),
                        'Wrong actual auxiliary producer')
                if route == 'wave':
                    require(source['owned_reads'] and len(source['offsets']) == 16 and max(source['offsets']) <= 16,
                            'Wave outside the proved owned-read policy')
        if kind.startswith('animation:'):
            require(len({v['source']['resource']['decoded']['bitmap_sha256'] for v in values}) == 3
                    and len({v['source']['frame_sha256'] for v in values}) == 3
                    and len({v['source']['resource']['frame'] for v in values}) == 3
                    and len({fingerprint(v['source']['resource']['asset']) for v in values}) == 1
                    and len({fingerprint(v['source']['source_incarnation']) for v in values}) == 1
                    and len({fingerprint(v['source']['camera']) for v in values}) == 1
                    and all(v['presentation']['changed_visible_words'] > 0 for v in values[1:]),
                    'Metadata/redraw/resource/camera changes cannot stand in for animation')


def close_rows(plan, parent, runs):
    validate(plan)
    require(parent['passed_requirements'] == 62 and parent['dossier_identity'] == fingerprint(requirements(plan)),
            'Different accepted 62-row inventory')
    require(len(runs) == 4 and {(v['domain'][0], tuple(v['domain'][1])) for v in runs} ==
            {(p, size) for p in ('player-default', 'player-software') for size in ((640, 480), (800, 600))}
            and len({v['identity'] for v in runs}) == 4, 'Missing/duplicate actual profile/size domain')
    for run in runs:
        check_output(run['witnesses'])
    result = []
    changed = set()
    for old in parent['requirements']:
        row = dict(old)
        key = row['configuration'], row['id']
        if key in ROWS:
            require(row['state'] == 'pending', 'Selected acceptance row already closed')
            row.update(state='passed', admitted_execution_evidence=[dict(execution=v['pair'],
                       actual_world_extents=[v['domain'][1]], provider=v['provider'],
                       reason='Actual named archived creature frames, changing source generations/decoded bitmaps and visible original/native output; actual projected-shadow/owned-wave routes with exact entry/AX/order/resource/import lineage. Finite recorded source phases; original ANI control/timing, full actor identity/attachment/combat/visibility and current native execution remain separate.')
                       for v in runs if v['domain'][0] == row['configuration']])
            changed.add(key)
        result.append(row)
    require(changed == ROWS and sum(v['state'] == 'passed' for v in result) == 67, 'Closeout exceeds five rows')
    return result


def analysis_sources(reader):
    reg = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    paths = {n for b in reg['behaviors'] if b['id'] in BEHAVIORS
             for role in ('implementation', 'tests', 'validation_dependencies') for n in b[role]}
    return {n: reader.digest(n) for n in sorted(paths)}


def analyze(reader):
    parent = reader.json(PARENT)
    cp = reader.json('research/runtime/rendering-copy-results-20261106/closeout-analysis-01.json')
    bd = reader.json(cp['input_parent']['record'])
    dh = reader.json(bd['input_parent']['record'])
    require(reader.digest(cp['input_parent']['record']) == cp['input_parent']['sha256'] and
            reader.digest(bd['input_parent']['record']) == bd['input_parent']['sha256'], 'Changed accepted composition chain')
    runs = []
    with configured(ROOT) as pair:
        for case in dh['executions']:
            if case['scenario'] != 'normal-storage':
                continue
            a, c = [reader.json(case['pair'][k]['record']) for k in ('acquisition', 'comparison')]
            require(a['success'] and c['success'] and c['acquisition'] == case['pair']['acquisition']
                    and a['session_identity'] == case['identity'] and c['comparison']['mismatching_pixels'] == 0
                    and c['original_reference']['continuous_from_owned_initial_history']
                    and not c['original_reference']['native_player_pixels_read']
                    and not c['original_reference']['original_destination_capture_read']
                    and all(reader.digest(case['pair'][k]['record']) == case['pair'][k]['sha256'] for k in case['pair']),
                    'Changed or dependent retained execution/comparison')
            catalog, assets = asset_catalog(pair, a, c)
            observed = acquisition([artifact(reader, a, Path(v['path']).name) for v in a['artifacts']
                                    if '/capture/' in v['path'] and v['path'].endswith('.bin')], a['session_identity'])
            require(sha(observed['commands']) == c['captured_command_stream_sha256'], 'Different compared commands')
            groups, originals = source_groups(reader, case, a, c, catalog)
            witnesses = presented_groups(case, c, observed, groups, originals)
            runs.append(dict(domain=case['domain'], scenario=case['scenario'], identity=case['identity'],
                             pair=case['pair'], provider=case['provider'], assets=assets, witnesses=witnesses))
            print('Verified animation/auxiliary output', case['domain'],
                  [(v['kind'], v['source']['generation']) for v in witnesses], flush=True)
    rows = close_rows(reader.json('docs/native-rendering-closure.json'), parent, runs)
    sources = analysis_sources(reader)
    combined = dict(parent['sources'])
    combined.update({n: h for n, h in sources.items() if n not in combined})
    return dict(schema=1, kind='retained_animation_auxiliary_five_row_closeout', success=True,
                sources=combined, analysis_sources=sources, parent=dict(record=PARENT, sha256=reader.digest(PARENT)),
                executions=runs, requirements=rows, dossier_identity=parent['dossier_identity'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {leaf for profile, leaf in ROWS}),
                decisions=parent['decisions'], passed_requirements=67, pending_requirements=89,
                newly_passed_requirements=5, actual_contexts=4,
                frame_states=sum(v['kind'].startswith('animation:') for c in runs for v in c['witnesses']),
                auxiliary_outputs=sum(v['kind'].startswith('auxiliary:') for c in runs for v in c['witnesses']),
                matched_viewport_pixels=sum(v['presentation']['viewport_pixels'] for c in runs for v in c['witnesses']),
                changed_visible_words=sum(v['presentation']['changed_visible_words'] for c in runs for v in c['witnesses']),
                scope='Fresh retained-data analysis closes exactly five named profile/requirement rows. Immutable creature SPR bytes and three distinct decoded frames per resource/domain join admitted source generations and actual visible output; natural projected-shadow and owned-wave routes join exact original entry/AX/order/resources and complete selected native pixels. Original ANI timing/controller recovery, per-actor lifetime, full attachments/combat/visibility, unowned reads, other maps/views/providers and current native execution remain separate. Preserve all62parent rows, original hashes, failures, historical native source phases and both pending decisions. Software creature-animation is outside this five-row promotion.',
                original_game_runs=0, new_fixture=False, new_native_code=False,
                current_native_engine_execution=False, historical_failures_retired=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts
            and not (ROOT / args.report).exists(), 'Fresh owned report required')
    result = dict(schema=1, success=False)
    try:
        with Reader(ROOT) as reader:
            before = analysis_sources(reader)
            result = analyze(reader)
            require(before == result['analysis_sources'], 'Analyzer/helper sources changed during execution')
            result['analysis_sources_stable'] = True
    except Exception as error:
        result.update(success=False, error=str(error))
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=digest)), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
