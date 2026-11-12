#!/usr/bin/env python3
"""Resolve owned-interface overlap requiredness in exact retained player scopes."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import tempfile

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from native_copy_routes import decode
from native_copy_clip_regions_scoped import decode_owner
from native_player_overlap import CONTRACT, CONTRACT_SHA, LAYOUT, contract, require, relation, summarize, verify_phase
from retained_artifacts import read_artifact

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-natural-overlap-results-20261109/'
BID = 'NP.native-player-overlap-boundary'
SID = BID+'.retained-domains'
PARENT = 'research/runtime/rendering-alias-bounded-results-20261109/admission-01.json'
INVENTORIES = (
    'research/runtime/rendering-copy-results-20261106/candidate-census-01.json',
    'research/runtime/rendering-copy-natural-results-20261108/acceptance-01.json',
    'research/runtime/rendering-clipped-operations-results-20261107/clipped-copy-binding-02.json',
)


def acquisition_refs(reader):
    first, stretch, clipped = [reader.json(p) for p in INVENTORIES]
    require(all(p['success'] is True for p in (first, stretch, clipped)), 'Unaccepted input inventory')
    refs = [dict(x['pair']['acquisition'], family='quick-campaign', domain=x['domain']) for x in first['executions']]
    refs += [dict(x['acquisition'], family='grimoire',
                  domain=['player-software' if x['software'] else 'player-default', x['size']])
             for x in stretch['cases']]
    selected = {}
    def find(value):
        if isinstance(value, dict):
            if isinstance(value.get('record'), str) and value['record'].endswith('-acquisition.json'):
                selected[value['record']] = dict(record=value['record'], sha256=value['sha256'], family='clipped')
            for item in value.values(): find(item)
        elif isinstance(value, list):
            for item in value: find(item)
    find(clipped)
    refs += list(selected.values())
    require(len(refs) == len({r['record'] for r in refs}) == 16
            and Counter(r['family'] for r in refs) == {'quick-campaign':8, 'grimoire':4, 'clipped':4},
            'Retained journey selection changed')
    return refs


def canonical_check(reader, acquired, rows):
    raw = read_artifact(reader, acquired, 'native-copy-clip-regions-v1.bin', max_bytes=36*1024**2)
    owner = struct.unpack_from('<I', raw, 44)[0]
    observed, scope = decode_owner(raw, rows, owner)
    checked = 0
    for item in observed:
        if not item['source_canonical']:
            continue
        require(item['source_incarnation'] > 0 and item['target_incarnation'] > 0,
                'Canonical observation lacks owned lifetime identities')
        for side in ('source', 'target'):
            require(item[side+'_alias']-item[side+'_canonical'] in (0,12,24,36,48),
                    'Actual provider interface offset contradicts PE32 source layout')
        same = item['source_canonical'] == item['target_canonical']
        require((relation(item['target_alias'], item['source_alias']) == 'same_owned_state') == same,
                'Independent provider canonical identity contradicts interface bounds')
        checked += 1
    require(checked > 0, 'No actual canonical-provider crosscheck')
    return dict(checked_copy_observations=checked, scope=scope,
                arbitrary_other_thread_backing_claimed=False)


def analyze(reader):
    provider = contract(reader)
    phases = {p['acquisition']['record']: p for p in provider['phases']}
    cases, total, sessions = [], Counter(), set()
    refs = acquisition_refs(reader)
    require(set(phases) == {r['record'] for r in refs}, 'Provider/caller domain inventory differs')
    for ref in refs:
        name = ref['record']
        require(reader.digest(name) == ref['sha256'] == phases[name]['acquisition']['sha256'],
                'Acquisition payload changed')
        acquired = reader.json(name)
        require(acquired['success'] is True, 'Failed capture cannot close requiredness')
        session = acquired['session_identity']
        require(isinstance(session, str) and session and session not in sessions,
                'Missing or duplicated actual player session')
        sessions.add(session)
        domain = ref.get('domain') or ['player-software' if acquired['configuration']['software_flag'] else 'player-default',
                                       acquired['configuration']['actual_size']]
        phase = verify_phase(reader, acquired, phases[name])
        raw = read_artifact(reader, acquired, 'copy-routes-v2.bin', max_bytes=12*1024**2)
        with tempfile.TemporaryDirectory(prefix='native-overlap-route-') as directory:
            path = Path(directory)/'routes.bin'; path.write_bytes(raw)
            header, rows = decode(path)
        result = summarize(rows)
        require(result['counts']['successful_source_copies'] > 0, 'Empty successful COPY scope')
        extra = canonical_check(reader, acquired, rows) if ref['family'] == 'clipped' else None
        total.update(result['counts'])
        cases.append(dict(acquisition={k:ref[k] for k in ('record', 'sha256')},
                          family=ref['family'], domain=domain, session_identity=session, provider=phase, result=result,
                          source_census_sha256=hashlib.sha256(raw).hexdigest(),
                          post_retirement_callbacks_excluded=header[21],
                          provider_canonical_crosscheck=extra))
    require(total['calls'] == 54906 and total['source_copies'] == 54294
            and total['failed_source_copies'] == 4, 'Reviewed finite COPY inventory changed')
    resolved = all(c['result']['bounded_exclusion'] for c in cases)
    return dict(success=True, cases=cases, counts=dict(total),
                resolution='not_required_in_recorded_successful_active_COPY' if resolved else 'natural_alias_candidate_requires_output_proof',
                bounded_requiredness_resolved=resolved,
                natural_overlap_witness=any(c['result']['natural_overlap_witness'] for c in cases),
                independent_natural_overlap_output_proved=False, global_unreachability_proved=False,
                source_contract=dict(record=CONTRACT, sha256=CONTRACT_SHA), layout=LAYOUT,
                source_semantics_review=dict(
                    reviewed=True, phase='exact captured native provider sources only',
                    facts=[
                        'All public NsObject interfaces occupy the first60bytes of one284-byte NsState; same-state interface deltas are0/12/24/36/48.',
                        'Mode2 cursor installation witnesses provider lifetime mode2. Guarded Blt/BltFast validate both registered interface identities before state access; GPU/CPU plan services independently resolve the same five interfaces.',
                        'ns_allocate gives each state private anonymous storage; both virtual views belong to that state. ns_flip permutes complete backing tuples under the same provider guard. Borrowed Lock/DC states refuse COPY.',
                        'Native-only DLL/source/admission hashes and normal retirement are checked per case. No original DirectDraw provider, external borrowed storage or changed native source phase is admitted.',
                        'Successful active COPY inputs are distinct owned states. Four failed source calls and every post-RenderStop callback remain explicit exclusions; no global absence or positive natural invocation is inferred.']))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    parser.add_argument('--parent', default=PARENT)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts
            and not (ROOT/args.report).exists(), 'Fresh owned report required')
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
        behavior = next(b for b in register['behaviors'] if b['id'] == BID)
        scenario = next(s for s in register['scenarios'] if s['id'] == SID)
        sources = {n:reader.digest(n) for n in sorted(required_sources(behavior))}
        result = analyze(reader)
        require(result['bounded_requiredness_resolved'], 'A natural alias candidate needs independent output proof; exclusion refused')
        parent = reader.json(args.parent)
        require(parent['success'] and len(parent['requirements']) == 156, 'Invalid acceptance parent')
        value = dict(parent)
        value.update(schema=1, kind='native_player_overlap_requiredness_boundary', success=True,
                     parent=dict(record=args.parent, sha256=reader.digest(args.parent)),
                     sources=parent['sources'] | {n:h for n,h in sources.items() if n not in parent['sources']},
                     analysis_sources=sources, sources_stable=sources == {n:reader.digest(n) for n in sources},
                     natural_overlap_boundary=result, newly_passed_requirements=0,
                     original_game_runs=0, new_native_execution=False,
                     current_whole_native_execution=False, preserved_parent_rows=156,
                     claims=[dict(behavior=BID, contract_sha256=behavior_contract(behavior, {}),
                                  scenarios={SID:scenario_contract(scenario)})],
                     scope='Precise requiredness exclusion for successful active COPY in16retained native-only Quick/Campaign/Grimoire/clipped player cases. Actual captured provider ABI and allocation/lifetime rules plus54290successful raw interface pairs exclude shared owned state. Four failed calls, terminal callbacks, unobserved contexts, foreign/physical drivers, current combined execution and positive natural overlap/output remain outside. All156acceptance rows and both decisions are unchanged.')
        require(value['sources_stable'], 'Analysis sources changed')
    digest = write_report(ROOT, args.report, (json.dumps(value, indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, sha256=digest, counts=result['counts'],
                          bounded_requiredness_resolved=result['bounded_requiredness_resolved'],
                          newly_passed_requirements=0)))


if __name__ == '__main__':
    main()
