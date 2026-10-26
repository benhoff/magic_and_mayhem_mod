#!/usr/bin/env python3
"""Investigate computed five-route callers; preserve every unresolved boundary."""
import argparse
from collections import Counter
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import evidence_digest, read_json, write_report
from original_x86 import Decoder, PE32
from pixel_caller_values import direct_target, inspect_values

ROOT = Path(__file__).resolve().parents[1]
BEHAVIOR = 'NP.five-pixel-computed-callers'
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
spec = importlib.util.spec_from_file_location('caller_windows', ROOT/'tools/inspect-pixel-route-caller-ranges.py')
caller_windows = importlib.util.module_from_spec(spec)
spec.loader.exec_module(caller_windows)
WINDOWS = caller_windows.WINDOWS


def normalized(row):
    return dict(row, operand=row['operand'].replace('dword ptr ', 'DWORD PTR '))


def decode_graph(image, decoder, roots, limit=750000):
    """Decode from admitted roots/direct edges; indirect edges stay unresolved.

    Roots include historical discovered entries and the five helper bodies.
    Being a root does not establish runtime reachability from the PE entry.
    """
    pending, rows, failures, outside = list(roots), {}, [], set()
    while pending:
        at = pending.pop()
        if at in rows:
            continue
        if not image.text(at):
            outside.add(at)
            continue
        section = image.section(at)
        available = section['address']+section['stored']-at
        if available <= 0:
            failures.append(dict(address=at, error='non-file-backed executable address'))
            continue
        if len(rows) >= limit:
            raise ValueError('Decoded instruction budget exhausted; incomplete scan')
        try:
            row = normalized(decoder.decode(image.read(at, min(15, available)), at))
        except ValueError as error:
            failures.append(dict(address=at, error=str(error)))
            continue
        rows[at] = row
        mnemonic, target = row['mnemonic'], direct_target(row)
        if mnemonic.startswith(('ret', 'iret')) or mnemonic in ('hlt', 'ud2', 'int3', 'int', 'ljmp', 'lret'):
            continue
        if mnemonic in ('call', 'jmp') or mnemonic.startswith(('j', 'loop')):
            if target is not None:
                pending.append(target)
            if mnemonic == 'jmp' or mnemonic.startswith(('ljmp', 'lret')):
                continue
        pending.append(at+row['size'])
    ordered = sorted(rows.values(), key=lambda row: row['address'])
    overlaps = []
    for left, right in zip(ordered, ordered[1:]):
        if right['address'] < left['address']+left['size']:
            overlaps.append([left['address'], right['address']])
    return ordered, dict(roots=len(roots), instruction_count=len(rows),
                         decode_failures=failures, nontext_targets=sorted(outside),
                         overlapping_instruction_starts=overlaps,
                         decoded_rows_sha256=hashlib.sha256(json.dumps(ordered, sort_keys=True).encode()).hexdigest(),
                         roots_imply_natural_reachability=False)


def dispatches(image, decoder):
    """Byte-backed original assignments, separately from inferred value flow."""
    sites = (0x484517, 0x484521, 0x48452b, 0x484535,
             0x484543, 0x48454d, 0x484557, 0x484561, 0x48457a)
    assignments = []
    for site in sites:
        raw = image.read(site, 10)
        if raw[:2] != b'\xc7\x05':
            raise ValueError('Original CPU assignment encoding changed')
        slot, target = int.from_bytes(raw[2:6], 'little'), int.from_bytes(raw[6:10], 'little')
        assignments.append(dict(site=site, slot=slot, target=target, bytes=raw.hex(),
                                instruction=normalized(decoder.decode(raw, site))))
    table = [image.word(0x555e08+i*4) for i in range(4)]
    if table != [0x555c4e, 0x555c57, 0x555c4e, 0x555c60]:
        raise ValueError('Reviewed minimap table changed')
    cameras = []
    for case, branch in enumerate(table):
        raw = image.read(branch, 7)
        if raw[:3] != bytes.fromhex('8bcbe8'):
            raise ValueError('Reviewed minimap branch changed')
        cameras.append(dict(case=case, branch=branch, bytes=raw.hex(),
                            target=(branch+7+int.from_bytes(raw[3:], 'little', signed=True)) & 0xffffffff))
    if [v['target'] for v in cameras] != [0x5527a0, 0x552f20, 0x5527a0, 0x552b50]:
        raise ValueError('Reviewed minimap camera targets changed')
    return dict(cpu_assignments=assignments, camera_dispatch=cameras,
                five_entries_selected=False, other_dispatches_closed=False)


def inspect(image, inventory, decoder):
    if image.sha256 != BUILD or inventory['source_sha256'] != BUILD:
        raise ValueError('Unsupported executable or discovery build')
    roots = {image.entry, *(f['entry'] for f in inventory['functions']),
             *(start for start, _ in WINDOWS.values()),
             *(target for flow in inventory['flows'] for target in flow['targets']
               if isinstance(target, int))}
    # Historical switch targets can be outside every discovered function entry.
    # Include retained flow sites too, and report any decoding disagreement.
    seeds = roots | {flow['site'] for flow in inventory['flows']}
    rows, graph = decode_graph(image, decoder, seeds)
    graph['function_and_target_roots'] = len(roots)
    graph['retained_flow_site_seeds'] = len(seeds-roots)
    values = inspect_values(rows, WINDOWS, image.word, roots)
    by_address = {row['address']: row for row in rows}
    imports = {row['address']:row for row in image.imports()}
    computed = {flow['site']:flow for flow in inventory['flows'] if flow['computed']}
    observed = {row['site']:row for row in values['indirect_transfers']}
    differences = []
    for site, flow in sorted(computed.items()):
        if site not in observed:
            differences.append(dict(site=site, retained_kind=flow['kind'],
                                    decoded=by_address.get(site),
                                    retained_instruction=flow.get('instruction')))
    classes = Counter()
    for row in values['indirect_transfers']:
        operand = row['operand']
        absolute = re.fullmatch(r'DWORD PTR \[(0x[0-9a-f]+)\]', operand)
        slot = int(absolute[1],16) if absolute else None
        if slot in imports:
            row['classification'] = 'import_address_table_runtime_binding'
            row['import'] = imports[slot]
            # The initial thunk value is an import name RVA, not the OS function
            # address installed by the loader. Preserve the raw candidate as such.
            row['initial_image_value_role'] = 'import_name_or_ordinal_thunk'
        elif absolute:
            row['classification'] = 'absolute_memory_runtime_target_unresolved'
        elif '[' in operand:
            row['classification'] = 'indexed_or_receiver_memory_target_unresolved'
        else:
            row['classification'] = 'register_target_unresolved'
        imported_reads = [imports[at] for at in row['initial_image_reads'] if at in imports]
        if imported_reads:
            row['initial_import_reads'] = imported_reads
            row['initial_image_value_role'] = 'derived_from_import_name_or_ordinal_thunk'
        row['retained_computed_flow'] = row['site'] in computed
        classes[row['classification']] += 1
    graph['computed_flow_reconciliation'] = dict(
        retained_sites=len(computed), decoded_sites=len(observed),
        retained_sites_decoded_as_indirect=len(set(computed)&set(observed)),
        retained_disagreements=differences,
        decoded_sites_outside_retained_census=sorted(set(observed)-set(computed)),
        classifications=dict(sorted(classes.items())))
    for candidate in values['candidates']:
        candidate['instructions'] = [by_address[site] for site in candidate['trace']]
    direct = []
    for row in rows:
        if row['mnemonic'] != 'call' and not row['mnemonic'].startswith(('j', 'loop')):
            continue
        target = direct_target(row)
        if target is None:
            continue
        for route, (start, end) in WINDOWS.items():
            if start <= target < end and not start <= row['address'] < end:
                direct.append(dict(route=route, instruction=row, target=target,
                                   confirmed_original_caller=False))
    return dict(graph=graph, value_scan=values, decoded_direct_candidates=direct,
                dispatch_witnesses=dispatches(image, decoder),
                routes=[dict(route=route, start=start, end=end,
                             body_sha256=hashlib.sha256(image.read(start, end-start)).hexdigest(),
                             decoded_direct_candidates=sum(v['route']==route for v in direct),
                             computed_value_candidates=sum(v['route']==route for v in values['candidates']),
                             classification='unused_candidate', natural_caller_recovered=False)
                        for route, (start, end) in WINDOWS.items()],
                original_sha256=image.sha256, original_game_executed=False,
                global_unreachable_proved=False,
                limits=[
                    'Historical entries and the five explicit helper roots are discovery seeds, not naturally reachable game entries.',
                    'Local constants reset at calls, branches, known incoming targets, gaps, stores and unsupported instructions; no inter-block or interprocedural value proof.',
                    'File-backed loads describe initial-image candidates; runtime writes, aliases and concurrent/foreign writers are unresolved.',
                    'Unresolved indirect targets, undecoded/overlapping code, dynamic code, encoded addresses, external modules and other builds remain open.',
                    'No game execution, route credit, dispatch edit, denominator change, original equivalence or live replacement.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=('controls', 'image'), required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    if (ROOT/args.report).exists():
        parser.error('Refusing report overwrite')
    register = read_json(ROOT/'research/runtime/coverage/register.json', root=ROOT)
    behavior = next(b for b in register['behaviors'] if b['id'] == BEHAVIOR)
    sources = {p: evidence_digest(ROOT/p, root=ROOT) for p in sorted(required_sources(behavior))}
    report = dict(schema=1, success=False, kind='static' if args.mode=='image' else 'native_integration',
                  sources=sources, claims=[dict(behavior=BEHAVIOR, contract_sha256=behavior_contract(
                      behavior, {b['id']:b for b in register['builds']}))],
                  original_game_executed=False, global_unreachable_proved=False)
    try:
        if args.mode == 'controls':
            run = subprocess.run([sys.executable, '-B', ROOT/'tests/test-pixel-caller-values.py'],
                                 text=True, capture_output=True, timeout=60)
            report.update(returncode=run.returncode, output=run.stdout+run.stderr,
                          success=run.returncode==0)
        else:
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
            try:
                image = PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
                inventory_path = next(build['inventory'] for build in register['builds']
                                      if build['id']=='nocd-40209ca7')
                if inventory_path not in sources:
                    raise ValueError('Central inventory missing from prospective source declaration')
                inventory = read_json(ROOT/inventory_path, root=ROOT)
                decoder = Decoder()
                try:
                    report.update(inspect(image, inventory, decoder), decoder_version=decoder.version,
                                  inventory_path=inventory_path, inventory_sha256=sources[inventory_path], success=True)
                finally:
                    decoder.close()
            finally:
                subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
                report['original_manifest_verified_before_after'] = True
    except Exception as error:
        report['error'] = str(error)
    report['sources_stable'] = all(evidence_digest(ROOT/p, root=ROOT)==sha for p,sha in sources.items())
    report['success'] &= report['sources_stable']
    payload = (json.dumps(report, indent=2)+'\n').encode()
    sha = write_report(ROOT, args.report, payload)
    print(json.dumps(dict(success=report['success'], report=args.report, sha256=sha,
                          error=report.get('error'), graph=report.get('graph', {}).get('instruction_count'),
                          candidates=len(report.get('value_scan', {}).get('candidates', [])))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
