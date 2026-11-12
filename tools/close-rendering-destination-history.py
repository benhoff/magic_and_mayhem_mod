#!/usr/bin/env python3
"""Close finite indexed-source/history operations from accepted retained runs."""
import argparse
from collections import Counter, OrderedDict
from functools import lru_cache
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import tempfile

from coverage_evidence import Reader, write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, require
from resident_capture import records

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-destination-results-20261106/'
PACK = 'research/runtime/rendering-normal-results-20261031/cleanup-storage-closeout-20261105-01.json'
BEHAVIOR = 'NP.destination-history-closeout'
LEAVES = {'operation.destination-dependency', 'operation.sprite-indexed'}
ENTRIES = {0x595677: 0, 0x5947b2: 1, 0x59521a: 1, 0x595b47: 2, 0x59603e: 2,
           0x57de00: 3, 0x57ec90: 4, 0x57f0f0: 5, 0x57f5f0: 6, 0x5806f0: 7,
           0x597086: 8, 0x596cb8: 8, 0x596490: 9, 0x5968a4: 9, 0x57e540: 10}


def module(name):
    spec = importlib.util.spec_from_file_location('destination_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    value = importlib.util.module_from_spec(spec); spec.loader.exec_module(value); return value


def sha(raw): return hashlib.sha256(raw).hexdigest()
def signed(word): return word if word < 0x80000000 else word - 0x100000000


def fnv(raw):
    value = 2166136261
    for byte in raw: value = ((value ^ byte) * 16777619) & 0xffffffff
    return value


def crc_table():
    result = []
    for value in range(256):
        for _ in range(8): value = (value >> 1) ^ (0x82f63b78 if value & 1 else 0)
        result.append(value)
    return result


CRC = crc_table()
PAYLOAD_CRC = OrderedDict()


def crc32c(raw):
    key = sha(raw)
    if key in PAYLOAD_CRC:
        PAYLOAD_CRC.move_to_end(key); return PAYLOAD_CRC[key]
    value = 0xffffffff
    for byte in raw: value = CRC[(value ^ byte) & 255] ^ (value >> 8)
    value ^= 0xffffffff
    PAYLOAD_CRC[key] = value
    if len(PAYLOAD_CRC) > 8192: PAYLOAD_CRC.popitem(last=False)
    return value


def source_hash(fields, body):
    return fnv(struct.pack('<24I', *fields) + struct.pack('<I', crc32c(body)))


def checked_request(rows, request):
    require(len(request) >= 64 and request[:8] == b'MNMWBQ03', 'Wrong owned request envelope')
    h = struct.unpack_from('<16I', request)
    entries = [f for f, b in rows if f[2] == 11]
    require(len(entries) == 1 and not any(f[2] in (12, 13) for f, b in rows), 'Missing/unclosed source queue')
    entry = entries[0]
    rasters = [(f, b) for f, b in rows if f[1] > entry[1] and f[2] == 9]
    require(h[2:7] == (5, 64, 1, 1, len(rows)) and h[7:11] == (entry[3], entry[5], entry[6], entry[5])
            and [h[8], h[9]] in ([640, 480], [800, 600]) and h[13] == 1
            and 0 < h[14] == h[15] == len(rasters) <= 12320 and h[11] == 16 * h[14]
            and len(request) == 64 + h[11] and fnv(request[64:]) == h[12], 'Request/source queue identity differs')
    result = []
    for i, (f, body) in enumerate(rasters):
        d = struct.unpack_from('<4I', request, 64 + i * 16)
        width, height, ox = struct.unpack_from('<3I', body, 4)
        left = signed(f[8]) - signed(ox)
        ax = int(f[15] in (0, 9) and width > 0 and height > 0
                 and (left < signed(f[10]) or left + width >= signed(f[12])))
        require(d[0] == f[1] and f[3] == h[7] and f[5:7] == h[8:10]
                and ENTRIES.get(d[1]) == f[15] and d[2] == f[21] == ax
                and d[3] == source_hash(f, body), 'Raster entry/AX/order/palette/source checksum differs')
        result.append((f, body, d))
    return entry, result


@lru_cache(maxsize=1024)
def frame_runs(raw):
    require(40 <= len(raw) <= 1048576 and struct.unpack_from('<I', raw)[0] == len(raw), 'Frame extent changed')
    width, height, ox, oy = struct.unpack_from('<4I', raw, 4)
    require(0 < width <= 2048 and 0 < height <= 2048 and 40 + height * 8 <= len(raw), 'Frame dimensions changed')
    first_pixel = struct.unpack_from('<I', raw, 44)[0]
    require(40 + height * 8 <= first_pixel <= len(raw), 'Frame runs overlap header')
    runs = []
    for y in range(height):
        begin, pixel = struct.unpack_from('<2I', raw, 40 + y * 8)
        end = struct.unpack_from('<I', raw, 48 + y * 8)[0] if y + 1 < height else first_pixel
        require(40 + height * 8 <= begin <= end <= first_pixel and first_pixel <= pixel <= len(raw), 'Frame row extent changed')
        x = 0
        for index, length in enumerate(raw[begin:end]):
            require(x + length <= width, 'Opaque run exceeds row')
            if index % 2:
                require(pixel + length <= len(raw), 'Indexed row pixels truncated')
                if length: runs.append((y, x, x + length))
                pixel += length
            x += length
        require(x == width, 'Opaque runs omit row')
    return width, height, signed(ox), signed(oy), runs


def visible_witness(f, body, descriptor):
    if f[17] != 1 or f[20] != 512 or f[14] not in (0, 5): return None
    require(len(body) == f[19] + 512, 'Indexed colour-table extent changed')
    width, height, ox, oy, runs = frame_runs(body[:f[19]])
    left, top = signed(f[8]) - ox, signed(f[9]) - oy
    l, t, r, b = map(signed, f[10:14])
    require(0 <= l <= r <= f[5] and 0 <= t <= b <= f[6], 'Captured clip outside owned canvas')
    clipped = left < l or top < t or left + width > r or top + height > b
    if f[14] == 0:
        if f[15] == 0 and (left < l or left + width >= r): return None
        count = sum(max(0, min(right + left, r) - max(start + left, l))
                    for y, start, right in runs if t <= y + top < b)
        kind = 'clipped-indexed' if clipped else 'indexed'
    else:
        shadow_top = top + height // 2
        if left < l or left + width + height // 2 > r or shadow_top >= b or shadow_top + height <= t: return None
        first, last = max(0, t - shadow_top), min(height, b - shadow_top)
        count = sum(right - start for y, start, right in runs
                    if first <= y < first + 2 * ((last - first + 1) // 2) and (y - first) % 2 == 0)
        kind = 'destination-shadow'
    if not count: return None
    return dict(kind=kind, sequence=f[1], fields=list(f), descriptor=list(descriptor),
                frame_sha256=sha(body[:f[19]]), palette_sha256=sha(body[f[19]:]),
                payload_sha256=sha(body), visible_samples=count, clipped=clipped,
                original_entry=descriptor[1], source_caller=f[22])


def source_history(reader, acquired, index):
    decode = module('inspect-canvas-producers').decode
    live, epochs, chunks, global_sequence = {}, {}, 0, 0
    result = []
    with tempfile.TemporaryDirectory(prefix='mnm-destination-source-', dir='/tmp') as temporary:
        path = Path(temporary) / 'worlds.bin'; path.write_bytes(artifact(reader, acquired, 'worlds.bin'))
        for generation, chunk, raw, request in records(path):
            header, rows = decode(raw, complete=False)
            require(header[2] == 3, 'Source history is not the accepted owned V3 source policy')
            if chunk:
                chunks += 1
                require(not request and not any(f[2] in (11, 12, 13) for f, b in rows), 'World/failure in source prefix')
            else:
                require(generation == len(result) + 1, 'Missing/reordered source World')
                entry, rasters = checked_request(rows, request)
            owner = None
            for f, body in rows:
                global_sequence += 1
                if f[2] == 1:
                    require(f[3] not in live and f[5] > 0 and f[6] > 0, 'Source creation collision')
                    epochs[f[3]] = epochs.get(f[3], 0) + 1
                    live[f[3]] = dict(canvas=f[3], incarnation=epochs[f[3]], created_at=global_sequence,
                                      created_in_generation=generation, width=f[5], height=f[6])
                elif f[2] == 2:
                    require(f[3] in live, 'Unknown source release'); del live[f[3]]
                elif f[2] == 11:
                    require(f[3] in live and (live[f[3]]['width'], live[f[3]]['height']) == f[5:7], 'World source has no live creation')
                    owner = dict(live[f[3]])
            if chunk: continue
            original = index['worlds'][generation - 1]
            require(owner and int(original['generation']) == generation and original['canvas'] == entry[3]
                    and (original['width'], original['height']) == entry[5:7]
                    and original['rasters'] == len(rasters), 'Independent reference/source queue differs')
            selected = {}
            for f, body, descriptor in rasters:
                wanted = ('destination-shadow' not in selected if f[14] == 5 else
                          'indexed' not in selected or 'clipped-indexed' not in selected)
                if not wanted: continue
                witness = visible_witness(f, body, descriptor)
                if witness and witness['kind'] not in selected: selected[witness['kind']] = witness
            result.append(dict(generation=generation, source_sha256=sha(raw), request_sha256=sha(request),
                               source_records=len(rows), source_incarnation=owner,
                               rasters=len(rasters), indexed_calls=sum(f[17] == 1 for f, b, d in rasters),
                               destination_calls=sum(f[14] in (1, 2, 3, 4, 5) for f, b, d in rasters),
                               witnesses=list(selected.values())))
    require(result and chunks == int(index['source_chunks']) and len(result) == len(index['worlds']), 'Incomplete initial/intervening source history')
    return dict(worlds=result, source_chunks=chunks, source_records=global_sequence)


def output_join(acquired, compared, observed, history):
    require(acquired['success'] is compared['success'] is True and acquired['private_wine_stopped'] is True
            and acquired['retirement']['state'] == 'completed', 'Unaccepted or failed execution')
    reference, comparison = compared['original_reference'], compared['comparison']
    require(reference['continuous_from_owned_initial_history'] is True
            and reference['native_player_pixels_read'] is reference['original_destination_capture_read'] is False
            and comparison['success'] is True and comparison['mismatching_pixels'] == comparison['remaining_surfaces'] == 0,
            'Missing continuous independent comparison')
    require(sha(observed['commands']) == compared['captured_command_stream_sha256'], 'Different compared command stream')
    life = module('close-rendering-world-exit').imported_lifetimes(observed['commands'])
    worlds = history['worlds']
    require(len(compared['world_domain']['extents']) == 1, 'Ambiguous actual output size')
    size = compared['world_domain']['extents'][0]
    require(len(worlds) == len(life['imports']) == reference['worlds']
            == int(acquired['resident']['imports']) == int(acquired['resident']['completed']) == int(acquired['resident']['verified_returns'])
            and compared['world_domain']['worlds'] == [dict(generation=i, width=size[0], height=size[1]) for i in range(1, len(worlds) + 1)],
            'Source/import/reference/domain denominator differs')
    actual = [observed['paints'][p] for p in observed['swaps'] if int(observed['paints'][p][0]['world']) > 0]
    matches = [v for v in comparison['comparisons'] if v['world'] > 0]
    require(actual and len(actual) == len(matches), 'Actual World swap omitted or duplicated')
    hashes, shown = {}, []
    for (paint, body), match in zip(actual, matches):
        generation, ordinal = int(paint['world']), int(paint['paint'])
        require(0 < generation <= len(worlds) and match['world'] == generation and match['paint'] == ordinal
                and match['lease'] == int(paint['lease']) > 0, 'Paint/source/lease/order differs')
        imported = life['imports'][generation - 1]
        import_live = imported['import_sequence'] <= paint['commands'] < imported['deleted_sequence']
        owners = [dict(surface=sid, created_sequence=created, deleted_sequence=deleted)
                  for (sid, created), deleted in life['deleted_incarnations'].items()
                  if sid == paint['surface'] and created <= paint['commands'] < deleted]
        require(len(owners) <= 1 and (not import_live or len(owners) == 1)
                and (import_live or paint['commands'] >= imported['deleted_sequence'])
                and paint['presentation_owner'] == 'gui' and paint['native_gpu_frame'] is True
                and paint['gpu_read_complete'] is True and paint['pixels_captured'] is True
                and (not import_live or [paint['frame_width'], paint['frame_height']] == size), 'Not a live complete native GPU output')
        if ordinal not in hashes: hashes[ordinal] = sha(body() if callable(body) else body)
        require(match['actual_sha256'] == match['expected_sha256'] == hashes[ordinal]
                and match['pixels'] == paint['width'] * paint['height'] > 0 and match['mismatching_pixels'] == 0,
                'Complete presented pixel digest differs')
        shown.append(dict(world=generation, paint=ordinal, lease=int(paint['lease']), pixels=match['pixels'], sha256=hashes[ordinal],
                          source_import_live=import_live, presentation_owner=owners[0] if owners else None))
    represented = {v['world'] for v in shown if v['source_import_live']}
    selected = {}
    for world in worlds:
        if world['generation'] not in represented: continue
        for witness in world['witnesses']:
            selected.setdefault(witness['kind'], []).append(dict(generation=world['generation'], witness=witness))
    require(set(selected) >= {'indexed', 'clipped-indexed', 'destination-shadow'}
            and all(len({v['generation'] for v in selected[k]}) >= 3 for k in selected), 'No nonempty presented indexed/clip/destination witnesses')
    return dict(verified=True, indexed_verified=True, destination_history_verified=True,
                source_generations=len(worlds), source_chunks=history['source_chunks'],
                indexed_calls=sum(v['indexed_calls'] for v in worlds), destination_calls=sum(v['destination_calls'] for v in worlds),
                world_presentations=len(shown), live_import_presentations=sum(v['source_import_live'] for v in shown),
                post_import_presentations=sum(not v['source_import_live'] for v in shown),
                retained_after_handle_release=sum(v['presentation_owner'] is None for v in shown), retained_pixels=sum(v['pixels'] for v in shown),
                presented_worlds=shown, visible_witnesses={k: v[:3] for k, v in selected.items()},
                source_worlds=worlds, source_records=history['source_records'],
                scope='Finite owned indexed/clip/order/palette and destination-history proof on the existing Quick/Campaign storage domain. Exact natural queued descriptors and nonempty opaque/clipped/projected-shadow source runs join live source incarnations and independently matched complete native output. All World-tagged compositions retain their comparison and actual completed GPU-frame readback;3retained post-import presentation leases outlive logical command handles under the separate GpuFrame texture owner. Post-import menu compositions cannot provide current-import scene witnesses. Original reference reads require defined source canvases before every queued batch and chain every original result; no zero-base or original-destination input. No full original ABI, raw CPU caller, direct/auxiliary family, negative displacement, later-map/fault or final decision claim.')


def close_rows(plan, parent, runs):
    validate(plan); template = requirements(plan)
    require(parent['dossier_identity'] == fingerprint(template) and parent['passed_requirements'] == 38
            and [(r['configuration'], r['id']) for r in parent['requirements']] == [(r['configuration'], r['id']) for r in template], 'Accepted38row parent differs')
    expected = {(p, s, tuple(z)) for p in ('player-default', 'player-software')
                for s in ('normal-quick', 'normal-storage') for z in ([640, 480], [800, 600])}
    require(len(runs) == 8 and {(r['domain'][0], r['scenario'], tuple(r['domain'][1])) for r in runs} == expected
            and len({r['identity'] for r in runs}) == 8
            and all(r['proof']['verified'] and r['proof']['indexed_verified'] and r['proof']['destination_history_verified'] for r in runs), 'Missing/duplicate/unproved operation domain')
    result = []
    for old in parent['requirements']:
        row = dict(old)
        if row['id'] in LEAVES:
            require(row['state'] == 'pending', 'Operation row already passed')
            row.update(state='passed', admitted_execution_evidence=[dict(execution=r['pair'], actual_world_extents=[r['domain'][1]],
                       provider=r['provider'], reason=r['proof']['scope']) for r in runs if r['domain'][0] == row['configuration']])
        result.append(row)
    require(sum(r['state'] == 'passed' for r in result) == 42, 'Operation scope expanded')
    return result


def analyze(reader):
    parent_module = module('close-rendering-world-exit')
    parent, impact = parent_module.parent_plan(ROOT, reader, 'MR06.natural-movie-return-closed01', 'NP.normal-movie-return-closeout')
    storage, storage_impact = parent_module.parent_plan(ROOT, reader, 'CS05.cleanup-storage-closed01', 'NP.cleanup-storage-closeout')
    pack = reader.json(PACK)
    require(pack['success'] and storage['inputs']['closeout'] == dict(record=PACK, sha256=reader.digest(PACK)), 'Not the accepted source/history pack')
    previous = reader.json(parent['inputs']['progress']['record']); sources = dict(parent['sources']); runs = []
    for old in pack['executions']:
        print('Reviewing retained operation domain', old['domain'], old['pair']['acquisition']['record'], flush=True)
        pair = old['pair']; a, c = [reader.json(pair[k]['record']) for k in ('acquisition', 'comparison')]
        require(all(reader.digest(pair[k]['record']) == pair[k]['sha256'] for k in pair)
                and c['acquisition'] == pair['acquisition'] and old['verified'] and old['identity'] == a['session_identity'], 'Changed accepted pair')
        for name, digest in c['sources'].items():
            if Path(name).suffix in ('.cpp', '.hpp', '.c', '.h', '.S', '.cmake') or Path(name).name == 'CMakeLists.txt':
                require(reader.digest(name) == digest, 'Changed native/original reference closure: ' + name); sources[name] = digest
        record = next(v for v in c['artifacts'] if v['path'] == 'reference/worlds.json')
        index = reader.json(record['record'])
        require(reader.digest(record['record']) == record['sha256'] and index['success'] is True
                and index['reference_history'] == 'Owned CPU producer inputs and original raster results chained continuously from initial creation'
                and index['native_player_pixels_read'] is index['original_destination_capture_read'] is False, 'Dependent/reseeded reference history')
        history = source_history(reader, a, index)
        streams = [artifact(reader, a, Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
        observed = acquisition(streams, a['session_identity'])
        proof = output_join(a, c, observed, history)
        require(type(a['configuration']['software_flag']) is bool
                and old['domain'] == ['player-software' if a['configuration']['software_flag'] else 'player-default', c['world_domain']['extents'][0]],
                'Profile/actual output domain differs')
        require(module('join-rendering-normal-acceptance').resource_history(observed['commands']) == old['resources'], 'Changed accepted resource history')
        print('Verified sources/presentations', proof['source_generations'], proof['world_presentations'], 'post-import compositions', proof['post_import_presentations'], flush=True)
        runs.append(dict(domain=old['domain'], scenario=a['scenario'], identity=old['identity'], provider=old['provider'], pair=pair, proof=proof))
    rows = close_rows(reader.json('docs/native-rendering-closure.json'), previous, runs)
    reg = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text()); behavior = next(v for v in reg['behaviors'] if v['id'] == BEHAVIOR)
    sources.update({n: reader.digest(n) for n in behavior['implementation'] + behavior['tests'] + behavior['validation_dependencies']})
    require(all(reader.digest(n) == h for n, h in sources.items()), 'Current analysis/parent sources differ')
    return dict(schema=1, kind='retained_indexed_and_destination_operation_closeout', success=True, sources=sources,
                accepted_parent=dict(binding=impact['retained_input'], applicability=impact),
                source_parent=dict(binding=storage_impact['retained_input'], applicability=storage_impact),
                input_pack=dict(record=PACK, sha256=reader.digest(PACK)), executions=runs, requirements=rows,
                dossier_identity=previous['dossier_identity'], closed_leaf_ids=sorted(set(previous['closed_leaf_ids']) | LEAVES),
                passed_requirements=42, pending_requirements=114, inherited_current_rows=38, newly_passed_requirements=4,
                source_generations=sum(r['proof']['source_generations'] for r in runs),
                world_presentations=sum(r['proof']['world_presentations'] for r in runs),
                live_import_presentations=sum(r['proof']['live_import_presentations'] for r in runs),
                post_import_presentations=sum(r['proof']['post_import_presentations'] for r in runs),
                retained_after_handle_release=sum(r['proof']['retained_after_handle_release'] for r in runs),
                retained_pixels=sum(r['proof']['retained_pixels'] for r in runs),
                decisions=previous['decisions'], original_game_runs=0, new_pixel_comparisons=0,
                new_native_code=False, new_game_fixture=False, historical_failures_retired=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--report', required=True); args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh owned immutable report required')
    result = dict(schema=1, kind='retained_indexed_and_destination_operation_closeout', success=False)
    try:
        with Reader(ROOT) as reader: result = analyze(reader)
    except Exception as error: result['error'] = str(error)
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=digest)), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__': raise SystemExit(main())
