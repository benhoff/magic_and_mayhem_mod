#!/usr/bin/env python3
"""Join first World source/caller contracts and complete retained native viewports."""
import argparse
from collections import Counter
import importlib.util
import io
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

from coverage_claims import fingerprint, required_sources
from coverage_evidence import Reader, write_report
from rendering_acceptance import requirements, validate
from rendering_menu_acceptance import artifact
from rendering_pilot import History, acquisition, commands, counter, require, sha, viewport
from resident_capture import records

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-first-world-results-20261107/'
PARENT = 'research/runtime/rendering-identity-results-20261107/closeout-analysis-03.json'
PACK = 'research/runtime/rendering-normal-results-20261031/loading-handoff-closeout-20261105-02.json'
BID = 'NP.first-world-composition-closeout'
LEAVES = {'journey.'+s+'.first-composed-world' for s in ('campaign-entry', 'quick-battle-entry')}
# Actual first-entry continuations only. Each byte string is checked against
# both the immutable original and the actual staged executable, not a label.
CALLERS = {
    0x5003b9: ('83c40ceb0e5753555051', {0}),
    0x5003eb: ('83c414e95504000083f8', {3}),
    0x500584: ('e9bf0200000fbf46120f', {6}),
    0x500aab: ('6689461ee9510200000f', {6}),
    0x500790: ('e9b30000008b4e0c8b56', {7}),
    0x500c4c: ('6689461ee9b00000008b', {7}),
    0x57e915: ('83c40c664866f7d81bc0', {0}),
    0x57e985: ('8b0e25ffff00008b6c24', {5}),
    0x57e9a8: ('25ffff00008b4c241c8b', {4}),
    0x57e9c7: ('25ffff00000be85f668b', {6}),
}


def module(name):
    spec = importlib.util.spec_from_file_location('first_'+name.replace('-', '_'), ROOT/'tools'/(name+'.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


base = module('close-rendering-destination-history')
originals = module('close-rendering-blend-direct').OriginalWorlds


def checked(reader, ref):
    require(reader.digest(ref['record']) == ref['sha256'], 'Changed accepted reference')
    return reader.json(ref['record'])


def pe_bytes(raw, address, size):
    require(raw[:2] == b'MZ' and len(raw) >= 64, 'Missing PE image')
    at = struct.unpack_from('<I', raw, 60)[0]
    require(raw[at:at+4] == b'PE\0\0' and struct.unpack_from('<H', raw, at+4)[0] == 0x14c,
            'Not the admitted PE32 image')
    count, optional = struct.unpack_from('<H', raw, at+6)[0], struct.unpack_from('<H', raw, at+20)[0]
    require(struct.unpack_from('<H', raw, at+24)[0] == 0x10b, 'Not PE32')
    image_base = struct.unpack_from('<I', raw, at+52)[0]
    for i in range(count):
        _, va, n, offset = struct.unpack_from('<4I', raw, at+24+optional+i*40+8)
        delta = address-image_base-va
        if 0 <= delta and delta+size <= n:
            require(offset+delta+size <= len(raw), 'Truncated PE region')
            return raw[offset+delta:offset+delta+size]
    raise ValueError('Unmapped original code region')


def object_bytes(reader, digest, size=None):
    """Read hash-verified immutable objects without acquiring the writer lock."""
    for store in reader.resolver.stores:
        row = store.db.execute('SELECT size FROM objects WHERE sha256=?', (digest,)).fetchone()
        if row is None:
            continue
        require(size is None or size == row[0], 'Retained object size differs')
        out = io.BytesIO()
        store.copy(digest, row[0], out)
        return out.getvalue()
    raise ValueError('Missing retained binary payload')


def caller_state(rows, request, original, staged):
    entry, rasters = base.checked_request(rows, request)
    active = [(f, b) for f, b in rows if f[1] > entry[1]]
    require(entry[14] == 1 and active and all(f[2] in (3, 4, 9, 10, 25) for f, b in active),
            'First queue includes an unreviewed writer/side-effect family')
    # Original traversal/consumer code remains the supplier. Its entry is
    # observed at runtime; staging must preserve this window and jump tables.
    before = pe_bytes(original, 0x5002a0, 0x1090)
    require(before == pe_bytes(staged, 0x5002a0, len(before)), 'Staging changed original consumer body/tables')
    calls = Counter()
    returns = Counter()
    for f, body, d in rasters:
        require(f[22] in CALLERS and f[15] in CALLERS[f[22]][1], 'Unreviewed first-World caller/backend')
        signature = bytes.fromhex(CALLERS[f[22]][0])
        require(pe_bytes(original, f[22], 10) == pe_bytes(staged, f[22], 10) == signature,
                'Original/staged caller continuation changed')
        require(f[7] == entry[5] and 0 <= f[10] <= f[12] <= entry[5]
                and 0 <= f[11] <= f[13] <= entry[6], 'Undefined pitch/effective caller clip')
        calls[(f[22], f[15])] += 1
        returns[f[21]] += 1
    require(calls and {0, 3, 6, 7} <= {backend for caller, backend in calls},
            'First queue lacks its expected terrain/sprite/auxiliary work')
    return dict(verified=True, queue_entry=list(entry), rasters=len(rasters),
                source_records=len(rows), source_request_sha256=sha(request),
                consumer_window=dict(address=0x5002a0, bytes=len(before), sha256=sha(before),
                                     staged_bytes_identical=True),
                callers=[dict(address=caller, backend=backend, calls=n,
                              continuation=CALLERS[caller][0]) for (caller, backend), n in sorted(calls.items())],
                low16_returns={str(k): v for k, v in sorted(returns.items())},
                required_boundary='Retained original traversal, ordering, row metadata and clip restoration; exact admitted raster continuations, effective source/clip/pitch/order and consumed low16 AX. Original reference executes every precise raster and verifies AX. Original lazy wave-table preparation remains active. Native surfaces own raster output; no original destination is available during the guarded batch.',
                whole_original_preparation_recovered=False,
                raw_row_snapshot_captured=False)


def first_output(acquired, compared, observed, worlds, size):
    require(compared['success'] and compared['acquisition']['sha256']
            and compared['original_reference']['continuous_from_owned_initial_history']
            and not compared['original_reference']['native_player_pixels_read']
            and not compared['original_reference']['original_destination_capture_read'], 'Dependent or incomplete reference')
    require(sha(observed['commands']) == compared['captured_command_stream_sha256'], 'Different command history')
    ordinal = next(p for p in observed['swaps'] if counter(observed['paints'][p][0]['world']))
    paint, body = observed['paints'][ordinal]
    require(counter(paint['world']) == 1 and paint['presentation_owner'] == 'gui'
            and paint['native_gpu_frame'] and paint['gpu_read_complete'] and paint['pixels_captured']
            and not paint['immutable_lease'] and paint['transport'] == 'opengl'
            and [paint['frame_width'], paint['frame_height']] == size, 'Wrong first native World owner/generation/domain')
    require(compared['first_presented_world']['paint'] == ordinal
            and compared['first_presented_world']['generation'] == 1, 'First swapped identity differs')
    matches = [v for v in compared['comparison']['comparisons'] if v['paint'] == ordinal]
    require(len(matches) == 1, 'Ambiguous first comparison')
    h = History(worlds)
    imported = []
    for op, seq, payload in commands(observed['commands']):
        h.apply(op, seq, payload)
        if op == 19:
            sid, low, high, _, _ = struct.unpack('<5I', payload)
            imported.append(dict(generation=low+(high<<32), sequence=seq, surface=sid))
        if seq == paint['commands']:
            require(imported and imported[0]['generation'] == 1, 'Missing first import')
            expected = viewport(h.render(observed['exports'][paint['reference_binding']]), paint)
            actual = body() if callable(body) else body
            require(actual == expected and matches[0]['actual_sha256'] == matches[0]['expected_sha256'] == sha(actual)
                    and matches[0]['pixels'] == paint['width']*paint['height']
                    and matches[0]['world'] == 1 and matches[0]['mismatching_pixels'] == 0,
                    'Complete first viewport differs from independent history')
            return dict(verified=True, paint=ordinal, generation=1, command_sequence=seq,
                        lease=counter(paint['lease']), frame_size=size,
                        viewport_size=[paint['width'], paint['height']], pixels=len(actual)//4,
                        actual_sha256=sha(actual), expected_sha256=sha(expected), mismatching_pixels=0,
                        imports=imported, original_world_planes_read=sorted(worlds.keys()))
    raise ValueError('First presentation command boundary missing')


def close_rows(plan, parent, cases):
    validate(plan)
    template = requirements(plan)
    require(parent['success'] and parent['passed_requirements'] == 74
            and len(parent['requirements']) == 156
            and sum(v['state']=='passed' for v in parent['requirements']) == 74
            and parent['decisions'] == {'supported-player':'pending','original-recovery':'pending'}
            and parent['dossier_identity'] == fingerprint(template)
            and [(v['configuration'], v['id']) for v in parent['requirements']] == [(v['configuration'], v['id']) for v in template],
            'Changed accepted 74-row parent')
    expected = {(p, s, tuple(z)) for p in ('player-default', 'player-software')
                for s in ('normal-quick', 'normal-storage') for z in ([640,480], [800,600])}
    require(len(cases) == 8 and {(c['domain'][0], c['scenario'], tuple(c['domain'][1])) for c in cases} == expected
            and len({c['identity'] for c in cases}) == 8
            and all(c['caller']['verified'] and c['caller']['rasters'] > 0
                    and c['output']['verified'] and c['output']['generation'] == 1
                    and c['output']['frame_size'] == c['domain'][1]
                    and c['output']['mismatching_pixels'] == 0 and c['output']['pixels'] > 0
                    and c['output']['actual_sha256'] == c['output']['expected_sha256']
                    and c['admitted_identity']
                    and c['admitted_normal_lifetime'] for c in cases), 'Missing/duplicate/unproved first-World domain')
    rows = []
    for old in parent['requirements']:
        row = dict(old)
        if row['id'] in LEAVES:
            require(row['state'] == 'pending', 'First-composition row already closed')
            scenario = 'normal-storage' if '.campaign-entry.' in row['id'] else 'normal-quick'
            row.update(state='passed', admitted_execution_evidence=[dict(execution=c['pair'],
                actual_world_extents=[c['domain'][1]], provider=c['provider'],
                reason='Complete first swapped viewport freshly reconstructed from independent original raster/history results, exact first owned request and consumed caller AX/clip/order, retained original preparation supplier and admitted identity/normal retirement. Frozen map/provider/source phase; no whole preparation recovery or current native execution claim.')
                for c in cases if c['domain'][0] == row['configuration'] and c['scenario'] == scenario])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows) == 78, 'Closeout exceeds four entry rows')
    return rows


def analyze(reader):
    parent = reader.json(PARENT)
    pack = reader.json(PACK)
    identity_cases = {c['reference']['record']: c for c in parent['cases']}
    lifetime = reader.json('research/runtime/rendering-retained-results-20261107/closeout-analysis-04.json')
    normal = {c['reference']['record'] for c in lifetime['retirements'] if c['proof']['verified']}
    cases = []
    payload = lambda h, n=None: object_bytes(reader,h,n)
    for old in pack['executions']:
        a, c = [checked(reader, old['pair'][k]) for k in ('acquisition', 'comparison')]
        identity = identity_cases[old['pair']['acquisition']['record']]
        require(identity['reference'] == old['pair']['acquisition'] and identity['profile'] == old['domain'][0]
                and old['pair']['acquisition']['record'] in normal and c['acquisition'] == old['pair']['acquisition'],
                'First case not admitted by both identity and normal-lifetime gates')
        size = old['domain'][1]
        require(c['world_domain']['extents'] == [size] and a['session_identity'] == old['identity'], 'Mixed source domain')
        require(a['scenario'] in ('normal-quick','normal-storage') and a['flow']['success']
                and c['original_menu_actions']['verified'], 'Unacknowledged entry')
        if a['scenario'] == 'normal-quick':
            require(a['flow']['selected_map_id'] == '1', 'Unreviewed Quick map')
        else:
            require(c['original_menu_actions']['campaign_selection']['callbacks'] == [[3,0],[18,65537],[18,0]],
                    'Unreviewed Campaign selection')
        native = json.loads(artifact(reader,a,'native-output-admission.json'))['native_only']['sources']
        require(fingerprint(native) == identity['native_source_sha256'], 'Native identity source closure differs')
        # These narrow native paths are unchanged from the actual compiler
        # closure. Other source drift remains historical, never waived.
        phases = {}
        for name in ('runtime/scene/canvas_producers.c', 'runtime/scene/world_raster_batch.c',
                     'runtime/scene/world_raster_callers.h'):
            require(native[name] == reader.digest(name)
                    and (name not in a['sources'] or a['sources'][name] == native[name]),
                    'Changed required caller adapter source')
            phases[name] = native[name]
        for name in ('tests/acceptance-world-reference.cpp', 'tests/acceptance-world-history.cpp'):
            require(c['sources'][name] == reader.digest(name), 'Changed independent AX/history executor source')
            phases[name] = c['sources'][name]
        original = payload(identity['installation']['original_executable_sha256'])
        staged = payload(identity['staged_executable']['sha256'], identity['staged_executable']['bytes'])
        with tempfile.TemporaryDirectory(dir='/tmp', prefix='first-world-source-') as d:
            path = Path(d)/'worlds.bin'
            path.write_bytes(artifact(reader,a,'worlds.bin'))
            chunks = []
            for generation, chunk, raw, request in records(path):
                if chunk:
                    chunks.append(dict(generation=generation, chunk=chunk, sha256=sha(raw)))
                    continue
                require(generation == 1, 'First source generation missing')
                rows = base.module('inspect-canvas-producers').decode(raw, complete=False)[1]
                caller = caller_state(rows,request,original,staged)
                source = dict(sha256=sha(raw), request_sha256=sha(request), prefix_chunks=chunks)
                break
            else:
                raise ValueError('No first World source')
        streams = [artifact(reader,a,Path(v['path']).name) for v in a['artifacts']
                   if '/capture/' in v['path'] and v['path'].endswith('.bin')]
        observed = acquisition(streams,a['session_identity'])
        worlds = originals(reader,c)
        output = first_output(a,c,observed,worlds,size)
        # The original helper checks every exact raster AX before recording
        # a result. Bind its actual successful stdout, not just the summary.
        log = artifact(reader,c,'original-history.log').decode()
        require(f"{c['original_reference']['worlds']} independently reconstructed Worlds" in log.splitlines(),
                'Missing original AX/history execution receipt')
        cases.append(dict(identity=old['identity'],domain=old['domain'],scenario=a['scenario'],pair=old['pair'],
            provider=old['provider'],source=source,caller=caller,output=output,
            map_selection={k:v for k,v in a['flow'].items() if k in ('selected_map_id','selected_map_name','selected_map_row')},
            preparation_source_phase=phases,admitted_identity=True,admitted_normal_lifetime=True))
        print('Checked complete first World',old['domain'],a['scenario'],output['pixels'],flush=True)
    rows = close_rows(reader.json('docs/native-rendering-closure.json'),parent,cases)
    return dict(schema=1,kind='retained_first_world_composition_closeout',success=True,
        scope='Four first-composed-World leaves on the eight admitted Campaign/default-region and Quick/map1 default/software640/800 executions. Fresh complete viewport reconstruction and exact first queue/caller preservation analysis; original preparation remains a permitted supplier. All native/compiler/original/viewport executions retain historical source phases, raw drift, failures and both pending decisions. No whole preparation recovery, arbitrary map/provider, current native engine execution or watchdog retirement.',
        parent=dict(record=PARENT,sha256=reader.digest(PARENT)),pack=dict(record=PACK,sha256=reader.digest(PACK)),
        cases=cases,requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
        closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|LEAVES),passed_requirements=78,pending_requirements=78,
        newly_passed_requirements=4,first_world_presentations=len(cases),
        compared_pixels=sum(c['output']['pixels'] for c in cases),mismatching_pixels=0,
        original_game_runs=0,new_native_code=False,current_native_engine_execution=False,
        historical_failures_retired=False,whole_original_preparation_recovered=False)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report',required=True)
    args = p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(), 'Fresh owned result required')
    result = dict(schema=1,success=False)
    try:
        with Reader(ROOT) as r:
            impact = subprocess.run([sys.executable, str(ROOT/'tools/evidence-applicability.py'), 'plan',
                '--evidence', 'IG07.identity-closed01', '--behavior', 'NP.accepted-identity-gate-closeout',
                '--require-applicable'], cwd=ROOT, capture_output=True, text=True)
            require(impact.returncode == 0, 'Accepted identity parent requires targeted revalidation')
            b = next(b for b in r.json('research/runtime/coverage/register.json')['behaviors'] if b['id']==BID)
            before = {n:r.digest(n) for n in sorted(required_sources(b))}
            result = analyze(r)
            require(before == {n:r.digest(n) for n in before}, 'Analysis sources changed')
            inherited = r.json(PARENT)['sources']
            result.update(analysis_sources=before,analysis_sources_stable=True,
                          sources=dict(inherited, **{n:h for n,h in before.items() if n not in inherited}),
                          parent_applicability=json.loads(impact.stdout))
    except Exception as error:
        result.update(success=False,error=str(error))
    h = write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=h)),flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
