"""Build-pinned drawing call census; addresses never imply function ownership."""
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD_SHA256 = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
INSTRUCTION = re.compile(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s+)+)\s*(\S+)\s*(.*?)\s*$')


def instructions(assembly):
    rows = {}
    for line in assembly.splitlines():
        match = INSTRUCTION.match(line)
        if match:
            address = int(match[1], 16)
            row = {'address': address, 'bytes': ''.join(match[2].split()),
                   'mnemonic': match[3], 'operand': match[4]}
            if address in rows and rows[address] != row:
                raise ValueError('Ambiguous disassembly address')
            rows[address] = row
    return rows


def direct_call(row):
    raw = bytes.fromhex(row['bytes'])
    if row['mnemonic'] != 'call' or len(raw) != 5 or raw[0] != 0xe8:
        return None
    target = (row['address'] + 5 + struct.unpack('<i', raw[1:])[0]) & 0xffffffff
    operand = re.fullmatch(r'0x([0-9a-f]+)(?:\s+<[^>]+>)?', row['operand'])
    if not operand or int(operand[1], 16) != target:
        raise ValueError('CALL operand disagrees with exact relative bytes')
    return target


def splice_conflicts(entry, length, code):
    """Reject incoming direct control flow into the middle of overwritten bytes."""
    conflicts = []
    for row in code.values():
        if row['mnemonic'].startswith(('j', 'loop')) or row['mnemonic'] == 'call':
            target = re.fullmatch(r'0x([0-9a-f]+)(?:\s+<[^>]+>)?', row['operand'])
            if target and entry < int(target[1], 16) < entry + length:
                conflicts.append({'site': row['address'], 'target': int(target[1], 16)})
    return conflicts


def owners(address, inventory):
    return sorted(f['entry'] for f in inventory['functions']
                  if any(start <= address < end for start, end in f['ranges']))


def calls_for(rows, entries, inventory):
    result = []
    for address, row in sorted(rows.items()):
        target = direct_call(row)
        if target in entries:
            result.append({'call_va': address, 'return_pc': address + 5,
                           'entry': target, 'bytes': row['bytes'],
                           'discovered_owners': owners(address, inventory)})
    return result


def validate_callers(record, build, known_entries):
    if record.get('schema') != 1 or record.get('source_sha256') != build:
        raise ValueError('Supplemental caller census build/schema mismatch')
    if record.get('image_base') != 0x400000 or record.get('input_unchanged') is not True:
        raise ValueError('Supplemental caller census lacks pinned unchanged input')
    result = {}
    for call in record['callers']:
        address, target, pc = call['call_va'], call['entry'], call['return_pc']
        if any(type(v) is not int or not 0 <= v <= 0xffffffff for v in (address, target, pc)):
            raise ValueError('Invalid supplemental CALL identity')
        if target not in known_entries or pc != address + 5:
            raise ValueError('Supplemental CALL does not name an exact registered entry')
        row = {'address': address, 'bytes': call['bytes'], 'mnemonic': 'call', 'operand': hex(target)}
        if direct_call(row) != target:
            raise ValueError('Supplemental caller lacks exact direct CALL bytes')
        if pc in result:
            raise ValueError('Duplicate supplemental return PC')
        result[pc] = {'entry': target, 'call_va': address, 'discovered_owners': call['discovered_owners']}
    return result


def export(executable, catalog, inventory_path):
    raw = executable.read_bytes()
    if hashlib.sha256(raw).hexdigest() != BUILD_SHA256:
        raise ValueError('Unsupported original executable; no export')
    inventory = json.loads(inventory_path.read_text())
    if inventory['source_sha256'] != BUILD_SHA256:
        raise ValueError('Discovery census build mismatch')
    assembly = subprocess.check_output(['objdump', '-d', '-Mintel', str(executable)], text=True)
    rows = instructions(assembly)
    entries = {int(s['identity'], 0) if isinstance(s['identity'], str) else s['identity']
               for route in catalog['routes'] for s in route['selectors'] if s['kind'] == 'entry'}
    calls = calls_for(rows, entries, inventory)
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256:
        raise ValueError('Original input changed during export')
    return {'schema': 1, 'source_sha256': BUILD_SHA256, 'image_base': inventory['image_base'],
            'scope': 'Exact direct CALL sites to registered drawing entries across pinned objdump output. '
                     'Ownership uses containing discovered ranges only. No runtime module-base, indirect-target, '
                     'function-completeness or scenario-closure assertion.',
            'input_unchanged': True, 'entries': sorted(entries), 'callers': calls,
            'sources': {'tools/drawing_census.py': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                        str(inventory_path.relative_to(ROOT)): hashlib.sha256(inventory_path.read_bytes()).hexdigest()}}


def trace(path, manifest, caller_census, inventory):
    """Decode closed observation metadata; integrity never supplies census closure."""
    hooks = {r['tag']: r for r in manifest['hooks']}
    if len(hooks) != len(manifest['hooks']) or set(hooks) != set(range(len(hooks))):
        raise ValueError('Invalid hook tag census')
    if inventory['source_sha256'] != BUILD_SHA256 or manifest['original_sha256'] != BUILD_SHA256:
        raise ValueError('Trace metadata build mismatch')
    callers = validate_callers(caller_census, BUILD_SHA256, {r['entry'] for r in hooks.values()})
    route_by_entry = {r['entry']: r['route'] for r in hooks.values()}
    owner_cache = {}
    def caller_owners(pc):
        if pc not in owner_cache: owner_cache[pc] = owners(pc - 1, inventory)
        return owner_cache[pc]
    counts = Counter(); sites = Counter(); returns = Counter(); result_counts = Counter()
    open_calls = {}; last_call = sequence = 0; unassigned = set(); unknown = set()
    before = path.stat()
    size = before.st_size
    if size < 128 or size > 64 + 4194304 * 128 + 64 or (size - 128) % 128:
        raise ValueError('Missing terminal marker or invalid bounded trace extent')
    with path.open('rb') as source:
        header = source.read(64); words = struct.unpack('<16I', header)
        if header[:8] != b'MNMDCN01' or words[2:5] != (1, 64, 128) or words[5] != 0x400000:
            raise ValueError('Unreviewed trace layout or observed module base')
        if header[32:64].hex() != BUILD_SHA256 or manifest['original_sha256'] != BUILD_SHA256:
            raise ValueError('Drawing trace build mismatch')
        if words[6] != len(hooks) or words[7] != 4194304:
            raise ValueError('Drawing trace hook/budget census mismatch')
        while source.tell() < size - 64:
            row = struct.unpack('<32I', source.read(128)); sequence += 1
            seq, event, cid, tag, phase, thread, entry, caller, sp = row[:9]
            if seq != sequence or tag not in hooks or entry != hooks[tag]['entry'] or not 0 <= phase <= 14:
                raise ValueError('Trace sequence, phase or callee identity mismatch')
            if any(row[29:]) or not cid or not thread or not sp:
                raise ValueError('Trace reserved/identity fields invalid')
            hook = hooks[tag]
            if phase == 0: unassigned.add(cid)
            key = (phase, entry, caller)
            if event in (1, 3):
                if cid != last_call + 1 or (event == 3) != bool(hook['site']):
                    raise ValueError('Call identity/site contract mismatch')
                last_call = cid
                if event == 1:
                    open_calls[cid] = (tag, phase, thread, entry, caller, sp)
                    counts[key] += 1
                else: sites[key] += 1
            elif event == 2:
                if hook['site'] or open_calls.pop(cid, None) != (tag, phase, thread, entry, caller, sp):
                    raise ValueError('Return has no exact thread/stack/callee/phase entry')
                returns[key] += 1; result_counts[(phase, entry, row[11])] += 1
            else: raise ValueError('Unknown trace event')
            if event == 3 and caller != 0:
                raise ValueError('Interior site must not invent a return caller')
            if event == 1 and not caller_owners(caller): unknown.add(caller)
        terminal = source.read(64); end = struct.unpack('<16I', terminal)
        if terminal[:8] != b'MNMDCEND' or end[2:5] != (1, sequence, last_call) or any(end[8:]):
            raise ValueError('Drawing trace terminal identity/count mismatch')
        if any(end[5:8]) or open_calls:
            raise ValueError('Drawing journal has loss, open calls or IO failure')
    def items(counter):
        result = []
        for (phase, entry, caller), hits in sorted(counter.items()):
            exact = callers.get(caller)
            result.append({'phase': phase, 'entry': entry, 'caller': caller, 'count': hits,
                           'route': route_by_entry[entry],
                           'caller_owners': caller_owners(caller) if caller else [],
                           'caller_attribution': ('interior_site_no_return_caller' if not caller else
                               'exact_direct_call' if exact and exact['entry'] == entry else
                               'observed_callee_pending_call_flow_review')})
        return result
    with path.open('rb') as source: digest = hashlib.file_digest(source, 'sha256').hexdigest()
    after = path.stat()
    if (before.st_ino, before.st_size, before.st_mtime_ns) != (after.st_ino, after.st_size, after.st_mtime_ns):
        raise ValueError('Trace changed during review')
    return {'schema': 1, 'journal_integrity': True, 'original_sha256': BUILD_SHA256,
            'observed_runtime_image_base': words[5], 'events': sequence, 'calls': last_call,
            'lost_events': 0, 'open_calls': 0, 'sha256': digest,
            'entry_counts': items(counts), 'site_counts': items(sites), 'return_counts': items(returns),
            'result_counts': [{'phase': phase, 'entry': entry, 'result': value, 'count': hits}
                              for (phase, entry, value), hits in sorted(result_counts.items())],
            'unassigned_phase_calls': len(unassigned), 'caller_pcs_without_discovered_owner': sorted(unknown),
            'remaining': ['All writer/side-effect coverage needs separate review.',
                          'Observed indirect/tail callees need exact call-flow classification.',
                          'Graphics API, borrowed access, failure/recovery and complete dispatch remain separate obligations.'],
            'draw01_complete': False}
