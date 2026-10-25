#!/usr/bin/env python3
"""Reconcile scoped original rendering dependencies; never infer whole-engine closure.

Reads the current authored closure/decision maps and retained binary inventory.
Fresh byte/decoder checks establish static provenance, not original execution,
native equivalence, integration, or natural reachability.
"""
import argparse
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import re
import subprocess
from coverage_claims import behavior_contract
from coverage_evidence import evidence_digest, read_json, write_report
from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
MAP = 'docs/native-rendering-closure.json'
DECISIONS = 'docs/native-rendering-dependencies.json'
IDS = ('RS.screen-cursor-background-invalidation', 'RS.screen-cursor-background-restore')
STK = 'STK1.original-final-20261024'


def require(value, message):
    if not value:
        raise ValueError(message)


def closed_flow(image, decoder, start, end):
    pending, rows, edges = [start], {}, []
    while pending:
        at = pending.pop()
        if at in rows:
            continue
        require(start <= at < end, 'Local flow escaped its declared body')
        row = decoder.decode(image.read(at, min(15, end-at)), at)
        rows[at] = row
        mnemonic, operand = row['mnemonic'], row['operand']
        if mnemonic.startswith('ret'):
            continue
        targets = []
        if mnemonic.startswith('j'):
            require(operand.startswith('0x'), 'Unknown local computed jump')
            targets.append(int(operand, 16))
            if mnemonic != 'jmp':
                targets.append(at+row['size'])
        else:
            targets.append(at+row['size'])
        for target in targets:
            edges.append([at, target])
            pending.append(target)
    occupied = {a for row in rows.values() for a in range(row['address'], row['address']+row['size'])}
    require(len(occupied) == sum(row['size'] for row in rows.values()), 'Overlapping local decoding')
    require(occupied == set(range(start, end)), 'Unassigned local bytes')
    return dict(entry=start, end=end, sha256=hashlib.sha256(image.read(start, end-start)).hexdigest(),
                instructions=sorted(rows.values(), key=lambda row: row['address']), edges=sorted(edges),
                closed_local_cfg=True, external_services_recovered=False)


def validate_call(image, decoder, call):
    row = decoder.decode(image.read(call['site'], 15), call['site'])
    require(row['mnemonic'] in ('call', 'jmp'), 'Retained call candidate is not CALL/JMP')
    if row['operand'].startswith('0x'):
        actual = int(row['operand'], 16)
        expected = [t for t in call['targets'] if isinstance(t, int)]
        require(actual in expected, 'Retained direct target differs from decoded bytes')
        row['confirmed_static_target'] = actual
    else:
        row['runtime_target_unresolved'] = True
    row['retained_kind'] = call['kind']
    row['retained_targets'] = call['targets']
    row['computed'] = call['computed']
    # Ghidra may label a tail jump as a call reference. Preserve the real opcode.
    row['tail_jump'] = row['mnemonic'] == 'jmp'
    return row


def map_checks(plan, decisions, register, suite):
    groups = {g['id']: g for g in plan['contracts']}
    behaviors = {b['id']: b for b in register['behaviors']}
    require(len(groups) == len(plan['contracts']), 'Duplicate contract group')
    require({j['id']: j['requirements'] for j in plan['journeys']} == suite['requirements'],
            'Journey denominator changed')
    require([o['id'] for o in plan['operations']] == suite['operation_requirements'],
            'Operation denominator changed')
    require({g['contract'] for g in decisions['groups']} == set(groups), 'Missing source-role decision')
    for group in groups.values():
        require(set(group['behaviors']) <= set(behaviors), 'Unknown behavior in contract group')
    for row in decisions['groups']:
        for path in row['source_witnesses']:
            require((ROOT/path).is_file(), 'Missing source witness: '+path)
    for journey in plan['journeys']:
        require(set(journey['contracts']+journey.get('conditional_contracts', [])) <= set(groups),
                'Unknown journey dependency')
    return behaviors


def acquire(report):
    register = read_json(ROOT/'research/runtime/coverage/register.json', root=ROOT)
    plan, decisions = read_json(ROOT/MAP, root=ROOT), read_json(ROOT/DECISIONS, root=ROOT)
    suite_path = plan['scope']['journey_catalog']
    suite = read_json(ROOT/suite_path, root=ROOT)
    behaviors = map_checks(plan, decisions, register, suite)
    inventory_path = register['builds'][0]['inventory']
    inventory = read_json(ROOT/inventory_path, root=ROOT)
    functions = {f['entry']: f for f in inventory['functions']}
    evidence = {e['id']: e for e in register['evidence']}
    screen_record_path = evidence[STK]['record']
    screen = read_json(ROOT/screen_record_path, root=ROOT)['flow']
    source_paths = {MAP, DECISIONS, suite_path, inventory_path, screen_record_path,
                    'tools/reconcile-rendering-dependencies.py', 'tools/original_x86.py',
                    'tools/coverage_claims.py', 'tools/original-manifest.sh', 'research/original-files.tsv'}
    source_paths.update(p for row in decisions['groups'] for p in row['source_witnesses'])
    source_paths.update(row['path'] for group in plan['contracts'] for row in group['code_witnesses'])
    source_paths.update(row['evidence'] for row in decisions['screen_services'] if 'evidence' in row)
    report['sources'] = {p: evidence_digest(ROOT/p, root=ROOT) for p in sorted(source_paths)}
    report['claims'] = [dict(behavior=i, contract_sha256=behavior_contract(behaviors[i],
                         {b['id']: b for b in register['builds']})) for i in IDS]
    image = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
    require(image.sha256 == BUILD, 'Unsupported original executable')
    require(plan['original_sha256'] == inventory['source_sha256'] == BUILD, 'Build identity mismatch')
    require(evidence_digest(ROOT/suite_path, root=ROOT) == plan['scope']['journey_catalog_sha256'],
            'Journey catalog hash changed')
    by_owner = defaultdict(list)
    for call in inventory['flows']:
        if call['kind'] == 'call':
            by_owner[call['owner']].append(call)
    roots = {}
    for group in plan['contracts']:
        for bid in group['behaviors']:
            for ref in behaviors[bid]['original']:
                address = ref['address']
                node = roots.setdefault(address, dict(address=address, contracts=[], behavior_scopes=[],
                                        relation=ref.get('relation', 'code'), inventory_function=address in functions))
                if group['id'] not in node['contracts']:
                    node['contracts'].append(group['id'])
                scope = dict(behavior=bid, scope=ref['scope'])
                if scope not in node['behavior_scopes']:
                    node['behavior_scopes'].append(scope)
    # Add the focused caller's external service frontier; it is absent from the
    # retained whole-image function census and must not disappear from the graph.
    for service in decisions['screen_services']:
        address = int(service['entry'], 16)
        roots.setdefault(address, dict(address=address, contracts=['C09'],
                         behavior_scopes=[dict(behavior='RS.screen-tick-control-flow',
                             scope='External service candidate only; caller scope does not cover callee.')],
                         relation='code', inventory_function=address in functions))
    by_owner[screen['entry']] = [dict(site=row['address'], kind='call',
        computed=not row['operand'].startswith('0x'),
        targets=[int(row['operand'], 16)] if row['operand'].startswith('0x') else [])
        for row in screen['calls']]
    decoder = Decoder()
    try:
        report['decoder_version'] = decoder.version
        checked_functions, calls = [], {}
        for address, node in roots.items():
            function = functions.get(address)
            if function:
                body = b''.join(image.read(start, end-start) for start, end in function['ranges'])
                require(hashlib.sha256(body).hexdigest() == function['sha256'], 'Inventory function hash differs')
                checked_functions.append(address)
            node['candidate_calls'] = []
            for call in by_owner[address]:
                site = call['site']
                if site not in calls:
                    calls[site] = validate_call(image, decoder, call)
                node['candidate_calls'].append(site)
        for row in screen['instructions']:
            require(image.read(row['address'], row['size']).hex() == row['bytes'], 'Screen bytes differ')
        imports = {row['address']: row for row in image.imports()}
        for row in calls.values():
            match = re.fullmatch(r'dword ptr \[0x([0-9a-f]+)\]', row['operand'])
            if match and int(match[1], 16) in imports:
                row['declared_import'] = imports[int(match[1], 16)]
        report['screen_local_flow'] = dict(entry=screen['entry'], end=screen['end'],
                                         sha256=screen['bytes_sha256'], calls=screen['calls'])
        invalidation = closed_flow(image, decoder, 0x4a4370, 0x4a437b)
        restore = closed_flow(image, decoder, 0x4a4380, 0x4a44ae)
        require(image.read(0x4a4370, 11).hex() == 'c705542c640000000000c3', 'Flag clear bytes differ')
        report['recovered_local_helpers'] = dict(invalidation=invalidation, restore=restore)
        # Independent negative witnesses: wrong target metadata and original byte hash.
        direct = next(c for c in inventory['flows'] if c['kind']=='call' and not c['computed']
                      and c['targets'] and isinstance(c['targets'][0], int) and c['site'] in calls)
        bad = dict(direct, targets=[direct['targets'][0]+1])
        refused = False
        try:
            validate_call(image, decoder, bad)
        except ValueError:
            refused = True
        require(refused, 'Changed direct-target metadata was accepted')
        mutated = bytearray(image.raw)
        mutated[0] ^= 1
        require(hashlib.sha256(mutated).hexdigest() != BUILD, 'Changed original image was accepted')
        report['negative_controls'] = dict(changed_direct_target_refused=True, changed_original_hash_refused=True)
    finally:
        decoder.close()
    # Declaration links are candidates for exactly their recorded scopes.
    anchors = defaultdict(list)
    for behavior in register['behaviors']:
        for ref in behavior['original']:
            anchors[ref['address']].append(behavior['id'])
    targets = sorted({row['confirmed_static_target'] for row in calls.values()
                      if 'confirmed_static_target' in row})
    report['graph'] = dict(anchors=sorted(roots.values(), key=lambda x:x['address']),
                          call_sites=sorted(calls.values(), key=lambda x:x['address']),
                          direct_target_behavior_candidates={str(a):sorted(set(anchors[a])) for a in targets},
                          unlinked_direct_targets=[a for a in targets if not anchors[a]],
                          role_decisions=decisions['groups'], screen_services=decisions['screen_services'],
                          guard_findings=decisions['guard_findings'], required_frontiers=decisions['required_frontiers'],
                          journeys=[dict(id=j['id'], requirements=j['requirements'], contracts=j['contracts'],
                                         conditional_contracts=j.get('conditional_contracts', [])) for j in plan['journeys']],
                          operations=plan['operations'])
    report['counts'] = dict(anchors=len(roots), checked_inventory_functions=len(checked_functions),
                           candidate_call_sites=len(calls), focused_screen_calls=len(screen['calls']),
                           unresolved_computed_call_sites=sum(row.get('runtime_target_unresolved', False)
                               and not row['retained_targets'] for row in calls.values()),
                           direct_targets=len(targets), unlinked_direct_targets=sum(not anchors[a] for a in targets),
                           tail_jump_candidates=sum(row['tail_jump'] for row in calls.values()),
                           journeys=len(plan['journeys']), operations=len(plan['operations']))
    report['source_roles_reviewed'] = True
    report['sources_stable'] = all(evidence_digest(ROOT/p, root=ROOT) == sha for p, sha in report['sources'].items())
    require(report['sources_stable'], 'Static-analysis sources changed during acquisition')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    target = ROOT/args.report
    require(not target.exists(), 'Refusing to overwrite a report')
    report = dict(schema=1, success=False, kind='static_rendering_dependency_reconciliation',
                  original_sha256=BUILD, original_code_executed=False, native_equivalence=False,
                  integration_promoted=False, live_replacement=False, global_unreachable_proved=False,
                  scope='Current-source role/route reconciliation and original static bytes/CFG only. '
                        'Containing-function edges and address links are candidates for their recorded scopes. '
                        'No recovered whole-game ownership, all-writer/indirect-flow closure or end-to-end acceptance.')
    verified_before = False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        verified_before = True
        acquire(report)
        report['success'] = True
    except (ValueError, OSError, KeyError, subprocess.SubprocessError) as exc:
        report['error'] = str(exc)
    finally:
        if verified_before:
            try:
                subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
                report['original_manifest_verified_before_after'] = True
            except (OSError, subprocess.SubprocessError) as exc:
                report.update(success=False, manifest_error=str(exc))
        sha = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'], sha256=sha,
                              counts=report.get('counts'), error=report.get('error'))), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
