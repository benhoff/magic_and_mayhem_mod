#!/usr/bin/env python3
"""Admit the explicitly amended controlled opaque-overlap mechanism scope."""
import argparse
import copy
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import tarfile

from coverage_claims import behavior_contract, fingerprint, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_acceptance import requirements, validate

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-alias-bounded-results-20261109/'
PROOF = 'research/runtime/rendering-alias-closeout-results-20261108/opaque-native-02.json'
PROOF_SHA = '1e8c49b041344976d24df9bc804bdcdfd110f070c97bae8e9f0afc8fa1d5ead6'
LEAF = 'operation.copy-alias-overlap'
BID = 'NP.alias-overlap-bounded-closeout'
SID = BID + '.retained-admission'
POLICY = dict(
    schema=1, id='ALIAS09.controlled-opaque-overlap-v1', requirement=LEAF,
    approval=dict(date='2026-11-09', response='yes', request='Approve bounded mechanism closure while keeping natural reachability explicitly open?'),
    configurations=['player-default', 'player-software'],
    acceptance_basis='controlled_original_wrapper_native_backend_and_v3_replay',
    scope=dict(format='RGB565', surface_extent=[8, 6], operations='opaque Blt/BltFast',
               aliases='same backing and original-verified Surface1 aliases normalized to one native backing ID',
               semantics='ordered per-piece snapshots, full source/destination planes and HRESULTs',
               provider='retained Wine original observations and the exact retained Mesa llvmpipe native execution'),
    proof=dict(record=PROOF, sha256=PROOF_SHA),
    profile_mapping='shared controlled mechanism; no natural profile-specific execution is inferred',
    remaining_obligation=dict(scope_decision='SC.natural-callers', gate='gate.required-boundaries', state='pending'),
    exclusions=['natural player overlap reachability and output', 'live COM alias discovery',
                'production facade whole-call frozen-source equivalence', 'keyed/masked/other-format overlap',
                'physical Windows/hardware driver equivalence', 'whole-player or original-recovery completion'])
SOURCES = ['tools/close-rendering-alias-bounded.py', 'tests/test-rendering-alias-bounded.py',
           'tools/rendering_acceptance.py', 'tools/rendering_completion.py', 'tools/rendering_execution.py',
           'tools/coverage_audit.py', 'tools/coverage_claims.py', 'tools/coverage_evidence.py',
           'tools/check-native-surface-overlap.py', 'tools/check-original-surface-overlap.py',
           'tools/check-original-surface-keys.py', 'docs/native-rendering-closure.json',
           'tests/fixtures/surfaces/original-rgb565-overlap-corpus.json',
           'tests/fixtures/surfaces/original-rgb565-overlap.json.gz']


def require(value, reason):
    if not value:
        raise ValueError(reason)


def module(name, relative):
    spec = importlib.util.spec_from_file_location(name, ROOT/relative)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def policy(plan):
    amendments = plan.get('acceptance_scope_amendments', [])
    selected = [a for a in amendments if a.get('id') == POLICY['id']]
    require(selected == [POLICY], 'Missing or widened owner-approved overlap scope')
    validate(plan)
    natural = next(d for d in plan['completion_contract']['scope_decisions'] if d['id'] == 'SC.natural-callers')
    require(natural['state'] == 'unresolved', 'Natural reachability cannot be closed by controlled proof')
    return copy.deepcopy(selected[0])


def unpack(reader, proof):
    ref = proof['retained_artifacts']
    raw = reader.read_bytes(ref['record'])
    require(len(raw) == ref['size'] <= 16*1024**2 and hashlib.sha256(raw).hexdigest() == ref['sha256'], 'Changed raw archive')
    expected = {m['path']: m for m in ref['members']}
    require(len(expected) == len(ref['members']) == 39, 'Missing/duplicate retained archive members')
    result = {}
    with tarfile.open(fileobj=io.BytesIO(raw), mode='r:gz') as archive:
        require(set(archive.getnames()) == set(expected) and len(archive.getmembers()) == 39, 'Archive inventory differs')
        for member in archive:
            require(member.isfile() and member.size == expected[member.name]['size'] and member.size <= 16*1024**2,
                    'Unsupported archive member')
            body = archive.extractfile(member).read()
            require(hashlib.sha256(body).hexdigest() == expected[member.name]['sha256'], 'Changed raw member: '+member.name)
            result[member.name] = body
    return result


def compare_raw(rows, members, proof, native):
    """Expectations come from original observations; native receives only pre-state/arguments."""
    inputs = json.loads(members['execution/inputs.json'])
    backend = json.loads(members['execution/backend.json'])
    require(len(rows) == len(inputs) == len(backend['cases']) == backend['operations'] == 1536,
            'Incomplete controlled domain')
    require(backend['success'] and backend['policy_tests'] == 20
            and backend['ordinary_uploads'] == backend['ordinary_readbacks'] == 0, 'Backend policy/output missing')
    directions = dict(left=0, right=0, up=0, down=0)
    aliases = partial = successes = 0
    for original, supplied, actual in zip(rows, inputs, backend['cases'], strict=True):
        c = original['input']
        flags = 0 if c['no_wait'] else 0x10 if c['fast'] else 0x1000000
        expected_input = dict(id=c['id'], shared=True, source_pixels=original['source_before'],
                              destination_pixels=original['destination_before'], source=c['source'],
                              destination=c['destination'], fast=c['fast'], flags=flags, held=0, clip=c['phase'])
        require(supplied == expected_input, 'Native input contains wrong state, geometry or expected outputs')
        require(actual['id'] == c['id'] and actual['hresult'] == original['hresult']
                and actual['source_after'] == original['source_after']
                and actual['destination_after'] == original['destination_after'], 'Complete native result/plane mismatch')
        require(native.o.cpu(original) == (original['hresult'], original['destination_after']), 'Independent original model mismatch')
        aliases += c['mode']
        partial += bool(original['hresult']) and original['destination_before'] != original['destination_after']
        if not original['hresult']:
            successes += 1
            sl, st, sr, sb = c['source']; dl, dt, dr, db = c['destination']
            if max(sl, dl) < min(sr, dr) and max(st, dt) < min(sb, db):
                directions['left'] += dl < sl; directions['right'] += dl > sl
                directions['up'] += dt < st; directions['down'] += dt > st
    require(aliases == 768 and partial == 16 and successes == 632 and all(directions.values()), 'Alias/direction/error domain missing')
    require(len(proof['chunks']) == 6, 'Missing replay chunks')
    for offset, chunk in zip(range(0, len(rows), 256), proof['chunks'], strict=True):
        subset = rows[offset:offset+256]; stem = 'execution/stream-'+str(offset)
        command = members[stem+'.cmd']; output = members[stem+'.pixels']
        actual = json.loads(members[stem+'.log'])
        require(command == native.stream(subset), 'Changed replay inputs or diagnostic stream')
        require(actual == chunk and actual['checks'] == actual['result_checks'] == actual['surface_copies'] == 256
                and actual['surface_stats']['surfaces'] == 0 and actual['rendered'], 'Incomplete replay/retirement')
        require(hashlib.sha256(command).hexdigest() == actual['command_sha256']
                and hashlib.sha256(output).hexdigest() == actual['output_sha256']
                and output == native.pixels(subset[-1]['destination_after']), 'Changed replay output')
    names = ('bad-result', 'bad-pixels', 'source-key-self', 'fast-source-key-self', 'unknown-flag', 'legacy-self')
    for name in names:
        negative = json.loads(members['execution/'+name+'.log'])
        require(negative.get('rendered') is False and negative.get('error')
                and 'execution/'+name+'.pixels' not in members, 'Negative stream published output')
    return dict(verified=True, cases=1536, successful_cases=successes, alias_cases=aliases,
                partial_error_cases=partial, overlap_directions=directions, policy_controls=20,
                rejected_streams=6, backend_hresult_checks=1536, replay_hresult_checks=1536,
                backend_complete_planes=3072, replay_complete_planes=1536, mismatches=0,
                natural_player_call_proved=False, actual_player_domains=0, tested_surface_extent=[8, 6])


def read_proof(reader):
    require(reader.digest(PROOF) == PROOF_SHA, 'Controlled proof identity changed')
    proof = reader.json(PROOF)
    require(proof['success'] and proof['sources_stable'] and proof['cases'] == 1536
            and proof['original_sha256'] == '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
            and proof['new_original_execution'] is False and proof['profile_acceptance'] is False
            and proof['natural_game_call_proved'] is False, 'Wrong/incomplete controlled proof')
    native = module('alias_bounded_original_comparator', 'tools/check-native-surface-overlap.py')
    rows, _ = native.o.load(native.o.DEFAULT)
    result = compare_raw(rows, unpack(reader, proof), proof, native)
    result.update(evidence=dict(record=PROOF, sha256=PROOF_SHA), execution_sources=proof['sources'],
                  raw_source_drift={n: dict(recorded=h, current=reader.digest(n))
                                    for n,h in proof['sources'].items() if reader.digest(n) != h},
                  tested_revision=proof['source_revision'], original_game_runs=0,
                  current_whole_native_execution=False, new_native_execution=False)
    return result


def definition_review(plan, parent):
    expected = requirements(plan)
    current = fingerprint(expected)
    review = parent.get('definition_review')
    if parent['dossier_identity'] == current:
        return copy.deepcopy(review)
    baseline = copy.deepcopy(expected)
    annotations = []
    for row in baseline:
        if row['id'] == LEAF:
            after = row['candidate_evidence']
            require(after.count('ALIAS08.caller-leads01') == 1,
                    'Exact existing candidate annotation required')
            before = [value for value in after if value != 'ALIAS08.caller-leads01']
            row['candidate_evidence'] = before
            annotations.append(dict(configuration=row['configuration'], id=LEAF,
                                    before=before, after=after))
    require(len(annotations) == 2 and fingerprint(baseline) == parent['dossier_identity'],
            'Unreviewed base definition')
    if review is not None:
        require(review.get('preserved_dossier_identity') == parent['dossier_identity']
                and review.get('current_definition_sha256') == current
                and review.get('all_acceptance_conditions_and_owners_unchanged') is True,
                'Changed inherited definition review')
        return copy.deepcopy(review)
    return dict(preserved_dossier_identity=parent['dossier_identity'],
                current_definition_sha256=current,
                all_acceptance_conditions_and_owners_unchanged=True,
                only_added_candidate_annotations=annotations,
                method='Reconstructed the exact frozen requirement fingerprint by removing only '
                       'the existing ALIAS08.caller-leads01 candidate from both overlap rows. '
                       'No condition, owner, row or decision changed; parent report remains immutable.')


def close_rows(plan, parent, proof):
    amendment = policy(plan)
    require(proof['verified'] and proof['cases'] == 1536 and proof['mismatches'] == 0
            and not proof['natural_player_call_proved'] and proof['actual_player_domains'] == 0,
            'Invalid controlled-only proof')
    expected = requirements(plan); keys = {(r['configuration'], r['id']) for r in expected}
    rows = parent['requirements']
    require(parent['success'] and len(rows) == len(keys) == 156
            and {(r['configuration'], r['id']) for r in rows} == keys, 'Parent inventory differs')
    require(sum(r['state'] == 'passed' for r in rows) == parent['passed_requirements']
            and sum(r['state'] == 'pending' for r in rows) == parent['pending_requirements'], 'Parent totals differ')
    definition_review(plan, parent)
    result = copy.deepcopy(rows); count = 0
    for old, row in zip(rows, result, strict=True):
        if row['id'] == LEAF:
            require(row['state'] == 'pending', 'Overlap row already admitted')
            require(row.get('admitted_execution_evidence') == [], 'Prior player execution credit cannot be relabelled')
            count += 1
            row.update(state='passed', acceptance_scope='bounded_controlled_opaque_overlap',
                       scope_amendment=dict(id=amendment['id'], sha256=fingerprint(amendment)),
                       admitted_mechanism_evidence=[proof['evidence']], tested_surface_extent=[8, 6],
                       natural_player_reachability='pending', actual_player_overlap_domains=0,
                       retained_obligation=dict(state='pending', scope_decision='SC.natural-callers',
                                                gate='gate.required-boundaries', condition=old['pending_reason']))
        elif row['id'] == 'gate.required-boundaries':
            require(row['state'] == 'pending', 'Required-boundaries gate must remain pending')
    require(count == 2 and all(n == o for n,o in zip(result, rows, strict=True) if o['id'] != LEAF),
            'Unrelated acceptance row changed')
    return result, amendment


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--parent', required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(), 'Fresh owned report required')
    with Reader(ROOT) as reader:
        sources = {n: reader.digest(n) for n in SOURCES}
        parent = reader.json(args.parent); plan = reader.json('docs/native-rendering-closure.json')
        proof = read_proof(reader); rows, amendment = close_rows(plan, parent, proof)
        register = reader.json('research/runtime/coverage/register.json')
        behavior = next(b for b in register['behaviors'] if b['id'] == BID)
        scenario = next(s for s in register['scenarios'] if s['id'] == SID)
        require(required_sources(behavior) == set(SOURCES), 'Admission source closure differs')
        require(sources == {n: reader.digest(n) for n in SOURCES}, 'Analysis sources changed')
        value = dict(schema=1, kind='owner_amended_controlled_overlap_admission', success=True,
                     sources=parent.get('sources', {}) | {n:h for n,h in sources.items() if n not in parent.get('sources', {})},
                     analysis_sources=sources, sources_stable=True, proof=proof,
                     parent=dict(record=args.parent, sha256=reader.digest(args.parent)),
                     parent_source_phases=parent.get('sources', {}), requirements=rows, decisions=parent['decisions'],
                     dossier_identity=parent['dossier_identity'], definition_review=definition_review(plan, parent),
                     scope_amendments=[amendment], acceptance_conditions_amended=True,
                     amended_contract_identity=fingerprint(dict(base_definition=current_definition(plan), amendment=amendment)),
                     passed_requirements=parent['passed_requirements']+2, pending_requirements=parent['pending_requirements']-2,
                     newly_passed_requirements=2, closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {LEAF}),
                     natural_player_reachability='pending', current_whole_native_execution=False,
                     original_recovery_promoted=False, original_game_runs=0, new_native_execution=False,
                     preserved_unrelated_parent_rows=154,
                     claims=[dict(behavior=BID, contract_sha256=behavior_contract(behavior, {}),
                                  scenarios={SID: scenario_contract(scenario)})],
                     scope='Owner-approved controlled opaque RGB565 overlap mechanism only; both profile labels share this bounded mechanism proof. Raw retained original/native outputs are freshly checked. No natural player overlap, live alias discovery, production-facade, other-format/keyed overlap, hardware, current whole-player or original-recovery completion. All other cells and both final decisions are preserved.')
    digest = write_report(ROOT, args.report, (json.dumps(value, indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, sha256=digest, passed=value['passed_requirements'], pending=value['pending_requirements'], newly_passed=2)))


def current_definition(plan):
    return fingerprint(requirements(plan))


if __name__ == '__main__':
    main()
