"""Retained scene occlusion proof, separate from attachment and combat semantics."""
from collections import defaultdict
from copy import deepcopy
import importlib.util
from pathlib import Path
import struct
import tempfile

import numpy as np

from coverage_claims import fingerprint
from rendering_closeout import inventory
from rendering_menu_acceptance import artifact
from rendering_pilot import History, acquisition, commands, require
from resident_capture import records

ROOT = Path(__file__).resolve().parents[1]
PARENT = 'research/runtime/rendering-enabled-intros-results-20261107/integrated-acceptance-03.json'
CORPUS = 'research/runtime/rendering-destination-results-20261106/closeout-analysis-05.json'
PREFIX = 'research/runtime/rendering-scene-effects-results-20261107/'
LEAF = 'journey.active-world.visibility'
TARGETS = (LEAF, 'journey.creatures-effects.attachment', 'journey.creatures-effects.combat')
BID = 'NP.scene-occlusion-output'
SID = 'native.scene-occlusion-retained'
MIN_POINTS = 8
MAX_GENERATION = 128


def module(name):
    spec = importlib.util.spec_from_file_location('scene_' + name.replace('-', '_'), ROOT/'tools'/(name+'.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


animation = module('close-rendering-animation-auxiliary')
first = module('close-rendering-first-world')
blend, base = animation.blend, animation.base
sha = base.sha


def checked(reader, ref):
    require(reader.digest(ref['record']) == ref['sha256'], 'Changed retained scene input')
    return reader.json(ref['record'])


def footprint(fields, body):
    """The opaque sprite pixels inside the scene interior, before later writes."""
    return {y*fields[5]+x for y, left, right in blend.spans(fields, body)
            for x in range(left, right) if x < fields[5]-200 and y < fields[6]-50}


def candidates(rows, catalog):
    entry = next(f for f, b in rows if f[2] == 11)
    rasters = [(f, b, None) for f, b in rows if f[2] == 9 and f[1] > entry[1]]
    writers, rectangles = animation.writer_map(rows, rasters, (entry[5], entry[6]))
    by_sequence = {f[1]: (f, b) for f, b in rows}
    result = {}
    for f, body, _ in rasters:
        refs = catalog.get(sha(body[:f[19]]), [])
        if not refs or f[14] != 0:
            continue
        require(len(refs) == 1, 'Ambiguous creature frame identity')
        points = footprint(f, body)
        if len(points) < MIN_POINTS:
            continue
        # Full effective palette bytes, clip and anchor must also stay fixed.
        key = fingerprint(dict(resource=refs[0], payload=sha(body), anchor=list(f[8:10]),
                               clip=list(f[10:14]), canvas=f[3], shape=list(f[5:7])))
        visible = {p for p in points if int(writers.flat[p]) == f[1]}
        hidden = {p for p in points if int(writers.flat[p]) > f[1]
                  and by_sequence[int(writers.flat[p])][0][2] == 9
                  and by_sequence[int(writers.flat[p])][0][14] == 0}
        value = dict(fields=f, body=body, resource=refs[0], points=points, visible=visible,
                     hidden=hidden, writers=writers, by_sequence=by_sequence, rectangles=rectangles)
        # A shared frame/address is not an actor identifier. Refuse ambiguity.
        result[key] = None if key in result else value
    return result


def describe(value, request_descriptors, points, original):
    f, body = value['fields'], value['body']
    samples, winners = [], {}
    for point in sorted(points):
        y, x = divmod(point, f[5])
        sequence = int(value['writers'].flat[point])
        require(sequence >= f[1], 'Unbound scene writer')
        winner, payload = value['by_sequence'][sequence]
        require(winner[2] == 9 and winner[14] == 0, 'Blend, wave or rectangle is not opaque occlusion')
        expected = blend.source_word(winner, payload, x, y)['word']
        underlying = blend.source_word(f, body, x, y)['word']
        require(int(original[y, x]) == expected, 'Independent original differs from ordered opaque source')
        if sequence != f[1]:
            require(expected != underlying, 'An equal-colour overlap cannot witness hidden output')
        if sequence not in winners:
            winners[sequence] = dict(fields=list(winner), descriptor=list(request_descriptors[sequence]),
                                     payload_sha256=sha(payload))
        samples.append(dict(point=point, winner=sequence, underlying=underlying, original=expected,
                            visible=sequence == f[1]))
    return dict(fields=list(f), descriptor=list(request_descriptors[f[1]]), payload_sha256=sha(body),
                resource=value['resource'], winners=list(winners.values()), samples=samples,
                primitive_occlusions=value['rectangles'])


def select_transition(reader, case, acquired, compared, catalog):
    """Choose from source state and independent WORDs before opening native pixels."""
    decode = module('inspect-canvas-producers').decode
    originals = blend.OriginalWorlds(reader, compared)
    presented = {v['world'] for v in case['proof']['presented_worlds'] if v['source_import_live']}
    previous, previous_context = {}, None
    best = None
    scanned = 0
    with tempfile.TemporaryDirectory(prefix='mnm-scene-effects-', dir='/tmp') as temporary:
        path = Path(temporary)/'worlds.bin'
        path.write_bytes(artifact(reader, acquired, 'worlds.bin'))
        for generation, chunk, raw, request in records(path):
            if chunk:
                continue
            if generation > MAX_GENERATION:
                break
            scanned += 1
            if generation < 2:
                continue  # First composition has a separate loading/preparation contract.
            accepted = case['proof']['source_worlds'][generation-1]
            require(accepted['generation'] == generation and accepted['source_sha256'] == sha(raw)
                    and accepted['request_sha256'] == sha(request), 'Different accepted source generation')
            _, rows = decode(raw, complete=False)
            now = candidates(rows, catalog)
            context = dict(generation=generation, source_sha256=sha(raw), request_sha256=sha(request),
                           source_incarnation=accepted['source_incarnation'], camera=animation.camera_context(rows))
            if previous_context and generation-1 in presented and generation in presented:
                for key in sorted(set(now) & set(previous)):
                    a, b = previous[key], now[key]
                    if a is None or b is None or a['points'] != b['points']:
                        continue
                    changed = (a['visible'] & b['hidden']) | (a['hidden'] & b['visible'])
                    # A native body colour, not metadata or an equal-colour overlap,
                    # must actually be replaced by a later opaque sprite (or return).
                    contrast = set()
                    for point in changed:
                        y, x = divmod(point, a['fields'][5])
                        hidden = b if point in b['hidden'] else a
                        winner = int(hidden['writers'].flat[point])
                        wf, wb = hidden['by_sequence'][winner]
                        if blend.source_word(wf, wb, x, y)['word'] != blend.source_word(a['fields'], a['body'], x, y)['word']:
                            contrast.add(point)
                    if len(contrast) < MIN_POINTS or best and len(contrast) <= len(best['points']):
                        continue
                    require(previous_context['source_incarnation'] == context['source_incarnation']
                            and previous_context['camera'] == context['camera'], 'Camera/import changed across occlusion')
                    # Only selected transitions pay for complete request checks.
                    before_rows = previous_context['_rows']
                    states = []
                    for value, state_rows, ctx, req in ((a, before_rows, previous_context, previous_context['_request']),
                                                        (b, rows, context, request)):
                        _, rasters = base.checked_request(state_rows, req)
                        descriptors = {f[1]: d for f, body, d in rasters}
                        states.append(dict(**{k: v for k, v in ctx.items() if not k.startswith('_')},
                                           **describe(value, descriptors, contrast, originals[ctx['generation']])))
                    best = dict(key=key, points=sorted(contrast), states=states)
            context.update(_rows=rows, _request=request)
            previous, previous_context = now, context
    require(best is not None, 'No adjacent unchanged-body opaque visibility transition in retained bound')
    return best, originals, scanned


def present(case, compared, observed, transition, originals):
    history = History(originals)
    states = {s['generation']: s for s in transition['states']}
    prior = {v['paint']: v for v in case['proof']['presented_worlds']}
    matches = {v['paint']: v for v in compared['comparison']['comparisons']}
    paints = defaultdict(list)
    for ordinal in observed['swaps']:
        paint, body = observed['paints'][ordinal]
        if int(paint['world']) in states and prior[ordinal]['source_import_live']:
            paints[paint['commands']].append((paint, body))
    found = {}
    for op, sequence, payload in commands(observed['commands']):
        history.apply(op, sequence, payload)
        for paint, body in paints.get(sequence, []):
            generation = int(paint['world'])
            if generation in found:
                continue
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
                    and paint['image_height'] % paint['frame_height'] == 0, 'Uncovered visibility viewport geometry')
            scale = paint['image_width']//paint['frame_width']
            require(scale > 0 and scale == paint['image_height']//paint['frame_height'], 'Unequal viewport scale')
            yy, xx = np.nonzero(np.isin(tags, transition['points']))
            indices = tags[yy, xx].astype('int64')
            if set(map(int, np.unique(indices))) != set(transition['points']):
                continue
            raw = body() if callable(body) else body
            match = matches[int(paint['paint'])]
            require(sha(raw) == match['actual_sha256'] == match['expected_sha256'] == prior[int(paint['paint'])]['sha256']
                    and match['mismatching_pixels'] == 0 and match['world'] == generation
                    and match['lease'] == int(paint['lease']) > 0, 'Changed complete accepted visibility viewport')
            words = {s['point']: s['original'] for s in states[generation]['samples']}
            expected = np.array([blend.rgba(words[int(at)]) for at in indices], dtype='uint8')
            pixels = np.frombuffer(raw, dtype='u1').reshape(paint['height'], paint['width'], 4)
            actual = []
            for dy in range(scale):
                for dx in range(scale):
                    part = pixels[paint['top']+yy*scale+dy, paint['left']+xx*scale+dx]
                    require(np.array_equal(part, expected), 'Actual visibility output differs from original WORDs')
                    actual.append(part.tobytes())
            found[generation] = dict(generation=generation, paint=int(paint['paint']), lease=int(paint['lease']),
                                    command_sequence=sequence, mapped_points=sorted(map(int, np.unique(indices))),
                                    viewport_pixels=len(indices)*scale*scale,
                                    actual_sha256=sha(b''.join(actual)), expected_sha256=sha(expected.tobytes()*scale**2),
                                    complete_viewport_sha256=match['actual_sha256'])
        if len(found) == len(states):
            break
    require(set(found) == set(states), 'Transition has no complete paired native viewport mapping')
    return [found[g] for g in sorted(found)]


def check_transition(value):
    points, states, outputs = value['points'], value['states'], value['presentations']
    require(len(points) >= MIN_POINTS and points == sorted(set(points)) and len(states) == len(outputs) == 2,
            'Missing transition points/states/output')
    a, b = states
    require(b['generation'] == a['generation']+1 and a['camera'] == b['camera'] and a['camera']['view'] == 0
            and a['source_incarnation'] == b['source_incarnation'] and a['resource'] == b['resource']
            and a['payload_sha256'] == b['payload_sha256'] and a['fields'][3:14] == b['fields'][3:14],
            'Animation, palette, camera, resource or placement changes cannot stand in for occlusion')
    require(a['resource']['asset']['path'] in animation.ASSETS, 'Unbound scene body resource')
    for state, output in zip(states, outputs):
        f, d = state['fields'], state['descriptor']
        require(f[14] == 0 and f[1] == d[0] and base.ENTRIES[d[1]] == f[15] and d[2] == f[21]
                and state['source_incarnation']['canvas'] == f[3], 'Wrong actual scene body/entry/AX')
        require(output['generation'] == state['generation'] and output['lease'] > 0
                and output['command_sequence'] > 0 and output['mapped_points'] == points
                and output['viewport_pixels'] >= len(points) and output['actual_sha256'] == output['expected_sha256'],
                'Missing complete actual visibility pixel mapping')
        require([s['point'] for s in state['samples']] == points, 'Lost/reordered visibility samples')
        winners = {w['fields'][1]: w for w in state['winners']}
        require(len(winners) == len(state['winners']), 'Duplicate scene writer')
        for sample in state['samples']:
            winner = winners[sample['winner']]
            wf, wd = winner['fields'], winner['descriptor']
            require(wf[2] == 9 and wf[14] == 0 and wf[1] == wd[0] and wd[2] == wf[21]
                    and base.ENTRIES[wd[1]] == wf[15] and wf[3:7] == f[3:7]
                    and wf[1] >= f[1] and sample['visible'] is (wf[1] == f[1])
                    and (sample['original'] == sample['underlying']) is sample['visible'],
                    'Blend/equal colour/unbound writer cannot prove hidden output')
            for rect in state['primitive_occlusions']:
                y, x = divmod(sample['point'], f[5])
                l, t, r, bottom = rect['area']
                require(not (rect['sequence'] > wf[1] and l <= x < r and t <= y < bottom),
                        'A later rectangle overwrites the claimed opaque result')
    require(all(x['point'] == y['point'] and x['visible'] is not y['visible']
                and x['underlying'] == y['underlying'] and x['original'] != y['original']
                for x, y in zip(a['samples'], b['samples'])), 'No genuine opaque visibility change')


def close_rows(plan, parent, cases):
    inventory(plan, parent)
    expected = {(p, tuple(size)) for p in ('player-default', 'player-software') for size in ([640, 480], [800, 600])}
    require(len(cases) == 4 and {(c['domain'][0], tuple(c['domain'][1])) for c in cases} == expected
            and len({c['identity'] for c in cases}) == 4 and all(c['scenario'] == 'normal-storage' for c in cases),
            'Missing/duplicate actual visibility domain')
    for case in cases:
        check_transition(case['transition'])
    rows = deepcopy(parent['requirements'])
    for row in rows:
        if row['id'] == LEAF:
            require(row['state'] == 'pending', 'Visibility row is already admitted')
            row.update(state='passed', admitted_execution_evidence=[dict(execution=c['pair'], provider=c['provider'],
                       actual_world_extents=[c['domain'][1]], reason='Adjacent source-bound opaque occlusion changes with an unchanged named creature bitmap, palette, clip, anchor and camera; independent ordered source/original WORDs and complete selected native viewport mapping. Original fog/mask/owner production remains a permitted supplier and is not newly recovered.')
                       for c in cases if c['domain'][0] == row['configuration']])
    require(sum(r['state'] == 'passed' for r in rows) == parent['passed_requirements']+2, 'Visibility scope expanded')
    return rows


def analyze(reader):
    parent, corpus = reader.json(PARENT), reader.json(CORPUS)
    plan = reader.json('docs/native-rendering-closure.json')
    inventory(plan, parent)
    cases = []
    class Assets:
        def read(self, digest, size):
            return first.object_bytes(reader, digest, size)
    for case in corpus['executions']:
        if case['scenario'] != 'normal-storage':
            continue
        acquired, compared = (checked(reader, case['pair'][key]) for key in ('acquisition', 'comparison'))
        require(acquired['success'] and compared['success'] and compared['acquisition'] == case['pair']['acquisition']
                and acquired['session_identity'] == case['identity'] and compared['comparison']['mismatching_pixels'] == 0
                and compared['original_reference']['continuous_from_owned_initial_history']
                and not compared['original_reference']['native_player_pixels_read']
                and not compared['original_reference']['original_destination_capture_read'], 'Unaccepted/dependent scene corpus')
        catalog, assets = animation.asset_catalog(Assets(), acquired, compared)
        transition, originals, scanned = select_transition(reader, case, acquired, compared, catalog)
        observed = acquisition([artifact(reader, acquired, Path(v['path']).name) for v in acquired['artifacts']
                                if '/capture/' in v['path'] and v['path'].endswith('.bin')], acquired['session_identity'])
        require(sha(observed['commands']) == compared['captured_command_stream_sha256'], 'Changed native command history')
        transition['presentations'] = present(case, compared, observed, transition, originals)
        check_transition(transition)
        cases.append(dict(domain=case['domain'], scenario=case['scenario'], identity=case['identity'], pair=case['pair'],
                          provider=case['provider'], assets=assets, transition=transition, scanned_generations=scanned))
        print('Verified scene occlusion', case['domain'], len(transition['points']), flush=True)
    rows = close_rows(plan, parent, cases)
    return dict(schema=1, kind='retained_scene_occlusion_output', success=True,
                parent=dict(record=PARENT, sha256=reader.digest(PARENT)), corpus=dict(record=CORPUS, sha256=reader.digest(CORPUS)),
                cases=cases, requirements=rows, dossier_identity=parent['dossier_identity'], decisions=parent['decisions'],
                sources=parent['sources'], closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {LEAF}),
                passed_requirements=parent['passed_requirements']+2, pending_requirements=parent['pending_requirements']-2,
                newly_passed_requirements=2, task_rows=[r for r in rows if r['id'] in TARGETS],
                changed_world_points=sum(len(c['transition']['points']) for c in cases),
                matched_viewport_pixels=sum(p['viewport_pixels'] for c in cases for p in c['transition']['presentations']),
                original_game_runs=0, current_native_engine_execution=False, historical_failures_retired=False,
                remaining_task_rows=4,
                remaining='Actual attachment parent/child/ANI anchors and placement across views, and action/effect/damage identity joined to native output are still required. Asset names, synthetic attachment previews, unbound old combat observations and downstream raster changes cannot supply those semantics.',
                scope='Adjacent retained source occlusion changes only, at both recorded player profiles and actual640/800. Keep the creature frame/palette/anchor/clip/camera fixed; independently check every changed opaque source/original WORD and its complete selected actual viewport mapping. This is finite scene occlusion delivery; no new fog/coverage-grid/terrain-owner recovery, actor generation, attachment/combat, arbitrary provider/map/view, native execution or final decision claim.')
