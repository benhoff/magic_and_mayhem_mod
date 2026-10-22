#!/usr/bin/env python3
"""Entry-aligned discovery across every remaining rendering/ownership obligation.

Potential indirect tables are retained as candidates, never silently closed.
No graph reachability or absence of a constant is an ownership proof.
"""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
from collections import Counter, deque
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

from original_x86 import Decoder, PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
TABLES = (0x501184, 0x501210, 0x50129c)


def immediate(value):
    try: return int(value, 0) & 0xffffffff
    except ValueError: return None


def discover(image, inventory):
    decoder = Decoder()
    seeds = {f['entry'] for f in inventory['functions']}
    seeds.add(image.entry)
    # Previously executed constructors include unused default variants that
    # the old function inventory does not own. Keep those explicit roots.
    seeds.update((0x545dc0,0x546230,0x546590,0x548560,0x548650,0x548ac0))
    tables = {table: [image.word(table + 4*i) for i in range(35)] for table in TABLES}
    for values in tables.values(): seeds.update(values)
    # Function pointers in the particle vtable region are additional explicit
    # roots. Their class boundaries are recovered from assigning instructions.
    for address in range(0x5c7380, 0x5c7600, 4):
        target = image.word(address)
        if image.text(target): seeds.add(target)
    pointer_candidates = []
    # Virtual tables, callback tables and exception metadata can hold code
    # addresses without a direct CALL. These are potential roots, not evidence
    # that every matching DWORD is a function pointer or reachable execution.
    for section in image.sections:
        if section['executable']: continue
        for at in range(section['address'], section['address']+section['stored']-3, 4):
            target=image.word(at)
            if image.text(target):
                pointer_candidates.append(dict(site=at,target=target,kind='data_dword'))
    pending = deque(sorted(seeds | {p['target'] for p in pointer_candidates})); queued = set(pending)
    rows = {}; failures = []; indirect = []; candidates = {}; direct_calls = []
    while pending:
        address = pending.popleft()
        while address not in rows and image.text(address):
            section = image.section(address)
            size = min(15, section['address'] + section['stored'] - address)
            if size <= 0:
                failures.append(dict(address=address, reason='Executable zero-filled range'))
                break
            try: row = decoder.decode(image.read(address, size), address)
            except ValueError as error:
                failures.append(dict(address=address, reason=str(error))); break
            rows[address] = row
            if len(rows) > 1000000: raise ValueError('Discovery instruction budget exceeded')
            mnemonic = row['mnemonic']; operand = row['operand']
            if mnemonic in {'mov','push'}:
                value=immediate(operand.rsplit(', ',1)[-1])
                if value is not None and image.text(value):
                    pointer_candidates.append(dict(site=address,target=value,kind='code_immediate'))
                    if value not in queued:pending.append(value);queued.add(value)
            next_address = address + row['size']
            branch = mnemonic.startswith('j') or mnemonic.startswith('loop')
            call = mnemonic == 'call'
            if branch or call:
                target = immediate(operand)
                if target is not None:
                    if call: direct_calls.append(dict(site=address, target=target, return_pc=next_address))
                    if image.text(target) and target not in queued:
                        pending.append(target); queued.add(target)
                else:
                    indirect.append(row)
                    # This discovers candidate table entries. Exact index/range
                    # provenance remains an explicit follow-up obligation.
                    match = re.fullmatch(r'dword ptr \[[a-z]+\*4 \+ (0x[0-9a-f]+)\]', operand)
                    if branch and match:
                        table = int(match[1], 16); values = []
                        for i in range(256):
                            try: value = image.word(table + i*4)
                            except ValueError: break
                            if not image.text(value): break
                            values.append(value)
                            if value not in queued:
                                pending.append(value); queued.add(value)
                        candidates[table] = dict(site=address, targets=values,
                            index_bounds_proved=table in TABLES,
                            exact_known_count=35 if table in TABLES else None)
                if mnemonic == 'jmp': break
            if mnemonic.startswith(('ret', 'iret')) or mnemonic in {'int3', 'hlt', 'ud2'}: break
            address = next_address
    version = decoder.version; decoder.close()
    return rows, dict(decoder_version=version, seed_entries=sorted(seeds),
        potential_code_pointer_roots=pointer_candidates,
        decode_failures=failures, direct_calls=direct_calls, indirect_edges=indirect,
        candidate_jump_tables=[dict(address=k, **v) for k, v in sorted(candidates.items())],
        exact_world_tables=[dict(address=k, targets=v) for k,v in tables.items()])


def analyze(image, inventory):
    rows, graph = discover(image, inventory)
    assignments = {}
    for row in rows.values():
        if row['mnemonic'] != 'mov': continue
        operands = row['operand'].split(', ', 1)
        if len(operands) != 2 or 'ptr [' not in operands[0]: continue
        target = immediate(operands[1])
        if target is None or not 0x5c0000 <= target < 0x5e0000: continue
        try: first = image.word(target)
        except ValueError: continue
        if image.text(first): assignments.setdefault(target, []).append(row['address'])
    boundaries = sorted(assignments)
    classes = []
    for i, table in enumerate(boundaries):
        limit = min(table + 32*4, boundaries[i+1] if i+1 < len(boundaries) else table+32*4)
        methods = []
        for address in range(table, limit, 4):
            target = image.word(address)
            if not image.text(target): break
            methods.append(target)
        if any(0x545500 <= target < 0x54ad00 for target in methods):
            classes.append(dict(vtable=table, methods=methods, assigning_sites=sorted(assignments[table]),
                next_assigned_boundary=limit, semantic_classification_pending=True))
    imports = image.imports(); by_import = {i['address']: i for i in imports}
    import_edges = []; unresolved = []
    for row in graph['indirect_edges']:
        match = re.fullmatch(r'dword ptr \[(0x[0-9a-f]+)\]', row['operand'])
        target = int(match[1], 16) if match else None
        if target in by_import: import_edges.append(dict(**row, imported=by_import[target]))
        else: unresolved.append(row)
    writes = []; address_constructions = []
    for row in rows.values():
        operands = row['operand'].split(', ', 1)
        if len(operands) != 2: continue
        if 'ptr [' in operands[0] and row['mnemonic'] not in {'cmp','test','lea'}:
            values = [int(v, 16) for v in re.findall(r'0x([0-9a-f]+)', operands[0])]
            if any(0x600000 <= v <= 0x6e1f6b for v in values): writes.append(row)
        if row['mnemonic'] in {'mov','lea','add','sub','push'}:
            values = [int(v, 16) for v in re.findall(r'0x([0-9a-f]+)', row['operand'])]
            if any(0x6e1e68 <= v <= 0x6e2068 for v in values): address_constructions.append(row)
    seed_calls = [c for c in graph['direct_calls'] if c['target'] == 0x54e170]
    original_roots = {0x58ad90,0x58b3e0,0x58b660,0x58bc10,0x4d8ab0,0x4a5ce0,0x5002a0}
    direct_callers = [c for c in graph['direct_calls'] if c['target'] in original_roots]
    return dict(schema=1, success=True, static_only=True, original_sha256=image.sha256,
        sections=image.sections, imports=imports, decoded_instructions=len(rows),
        control_flow=graph, import_edges=import_edges, unresolved_indirect_edges=unresolved,
        kind8_classes=classes, kind8_rng_seed_callers=seed_calls,
        selected_original_callers=direct_callers,
        global_write_candidates=writes, alternate_nearby_address_constructions=address_constructions,
        all_writer_ownership_proved=False, all_indirect_edges_resolved=False,
        rows=[rows[k] for k in sorted(rows)])


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--inventory', type=Path, default=ROOT/'research/runtime/coverage/binary-nocd-all-rendering-complete-entries-20261020.json')
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'): p.error('Require new retained output under working/tests')
    out.mkdir(parents=True)
    inventory_path=a.inventory.resolve()
    inventory_name=inventory_path.relative_to(ROOT).as_posix()
    sources={n:hashlib.sha256(evidence_bytes(ROOT/n)).hexdigest() for n in
        ['tools/inspect-all-original-recovery.py','tools/original_x86.py','tests/test-original-x86.py',
         inventory_name]}
    (out/'prospective.json').write_text(json.dumps(dict(sources=sources,original_sha256=BUILD,static_only=True),indent=2)+'\n')
    before=False;report=dict(schema=1,success=False,sources=sources,static_only=True)
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
        image=PE32(evidence_bytes(ROOT/'original/Arcane_Nocd/Chaos.exe'))
        if image.sha256 != BUILD: raise ValueError('Require pinned unchanged original')
        inventory=json.loads(evidence_text(inventory_path))
        if inventory['source_sha256']!=BUILD: raise ValueError('Inventory build differs')
        report.update(analyze(image,inventory))
    except (OSError, ValueError, subprocess.SubprocessError) as error:report.update(success=False,error=str(error))
    finally:
        if before:
            try:
                subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
                report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(hashlib.sha256(evidence_bytes(ROOT/n)).hexdigest()==v for n,v in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json')
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
