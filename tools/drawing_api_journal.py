"""Exact original CALL splices and a separate strict API diagnostic journal."""
from drawing_api_pairs import ApiPairs
import hashlib
import re
import struct
from drawing_census import BUILD_SHA256, splice_conflicts
from drawing_api_sites import call_mode

MAX_RECORDS = 4194304


def relative(row):
    """Byte-checked control transfers supported in the copied remainder."""
    raw = bytes.fromhex(row['bytes']); opcode = raw[0]
    kind = None; offset = None; condition = None
    if opcode in (0xe8, 0xe9) and len(raw) == 5:
        kind = 'call' if opcode == 0xe8 else 'jump'; offset = struct.unpack('<i', raw[1:])[0]
    elif opcode == 0xeb and len(raw) == 2:
        kind = 'jump'; offset = struct.unpack('<b', raw[1:])[0]
    elif 0x70 <= opcode <= 0x7f and len(raw) == 2:
        kind = 'conditional'; condition = opcode & 15; offset = struct.unpack('<b', raw[1:])[0]
    elif len(raw) == 6 and opcode == 0x0f and 0x80 <= raw[1] <= 0x8f:
        kind = 'conditional'; condition = raw[1] & 15; offset = struct.unpack('<i', raw[2:])[0]
    if kind:
        target = (row['address'] + len(raw) + offset) & 0xffffffff
        match = re.fullmatch(r'0x([0-9a-f]+)(?:\s+<[^>]+>)?', row['operand'])
        expected = 'call' if kind == 'call' else 'jmp' if kind == 'jump' else None
        if not match or int(match[1], 16) != target or (expected and row['mnemonic'] != expected) or (kind == 'conditional' and not row['mnemonic'].startswith('j')):
            raise ValueError('Relative transfer bytes/operand/mnemonic mismatch')
        return {'kind': kind, 'target': target, 'condition': condition}
    if row['mnemonic'].startswith(('j', 'loop')) or (row['mnemonic'] == 'call' and opcode != 0xff):
        raise ValueError('Unsupported copied control transfer')
    return None


def plan(site, code, occupied=()):
    """No owner inference: caller supplies an exact previously selected site."""
    if type(site) is not int or not 0 <= site <= 0xffffffff: raise ValueError('Invalid CALL site')
    first = code[site]
    if first['address'] != site: raise ValueError('CALL instruction address mismatch')
    mode = call_mode(first)
    if mode['kind'] == 'unresolved_indirect': raise ValueError('Unresolved CALL target addressing')
    signature = bytearray(); rows = []; cursor = site
    while len(signature) < 5:
        row = code[cursor]; raw = bytes.fromhex(row['bytes'])
        if row['address'] != cursor: raise ValueError('Copied instruction address mismatch')
        if not raw or len(raw) > 15: raise ValueError('Invalid instruction extent')
        transfer = relative(row)
        rows.append({'address': cursor, 'bytes': row['bytes'], 'transfer': transfer})
        signature.extend(raw); cursor += len(raw)
        if row['mnemonic'].startswith('ret') and len(signature) < 5:
            raise ValueError('Return before complete API splice')
    if len(signature) > 20: raise ValueError('API splice exceeds signature bound')
    if any(site < end and start < cursor for start, end in occupied):
        raise ValueError('API splice overlaps another original hook')
    if splice_conflicts(site, len(signature), code):
        raise ValueError('Incoming original direct flow enters overwritten interior')
    # CALL executes before the AFTER stub, remainder only afterwards. Interior
    # targets are refused above, including those originating within this splice.
    return {'site': site, 'signature': signature.hex(), 'length': len(signature),
            'call_length': len(bytes.fromhex(first['bytes'])), 'target_mode': mode,
            'call': rows[0], 'remainder': rows[1:]}


def breakpoint_plan(site, code, occupied=()):
    """Verify the whole CALL; overwrite only its first byte with INT3.

    Incoming flow to the untouched successor is safe. Incoming flow into the
    CALL operand and overlapping original instrumentation remain refused.
    """
    if type(site) is not int or not 0 <= site <= 0xffffffff:
        raise ValueError('Invalid breakpoint CALL site')
    first = code[site]
    if first['address'] != site: raise ValueError('CALL instruction address mismatch')
    mode = call_mode(first)
    if mode['kind'] == 'unresolved_indirect': raise ValueError('Unresolved CALL target addressing')
    raw = bytes.fromhex(first['bytes']); length = len(raw)
    if not 2 <= length <= 15: raise ValueError('Invalid breakpoint CALL extent')
    if any(site < end and start < site + length for start, end in occupied):
        raise ValueError('API breakpoint CALL overlaps another original hook')
    if splice_conflicts(site, length, code):
        raise ValueError('Incoming original direct flow enters CALL operand')
    return {'site':site, 'signature':raw.hex(), 'length':length,
            'call_length':length, 'target_mode':mode, 'patch_mode':'breakpoint',
            'patch_bytes':1, 'call':{'address':site,'bytes':first['bytes'],
            'transfer':relative(first)}, 'remainder':[]}


def relocated(row, address):
    raw = bytes.fromhex(row['bytes']); transfer = row['transfer']
    if not transfer: return raw
    if transfer['kind'] == 'conditional':
        return bytes([0x0f, 0x80 + transfer['condition']]) + struct.pack('<I', (transfer['target'] - address - 6) & 0xffffffff)
    return bytes([0xe8 if transfer['kind'] == 'call' else 0xe9]) + struct.pack('<I', (transfer['target'] - address - 5) & 0xffffffff)


def trampoline(item, allocation, after_stub):
    """CALL, JMP AFTER stub, relocated remainder, JMP original continuation."""
    call = relocated(item['call'], allocation)
    rest = allocation + len(call) + 5
    data = bytearray(call + b'\xe9' + struct.pack('<I', (after_stub - rest) & 0xffffffff))
    for row in item['remainder']: data.extend(relocated(row, allocation + len(data)))
    next_pc = allocation + len(data) + 5
    data.extend(b'\xe9' + struct.pack('<I', (item['site'] + item['length'] - next_pc) & 0xffffffff))
    return bytes(data), rest


def trace(path, manifest):
    """Validate V1 exact identities and LIFO pairs; no semantic API assumptions."""
    pairs=ApiPairs(manifest);hooks=pairs.hooks
    before=path.stat();size=before.st_size
    if size<128 or size>128+MAX_RECORDS*128 or (size-128)%128:
        raise ValueError('Invalid API journal extent or missing terminal')
    with path.open('rb') as source:
        header=source.read(64);h=struct.unpack('<16I',header)
        if header[:8]!=b'MNMDAPI1' or h[2:8]!=(1,64,128,0x400000,len(hooks),MAX_RECORDS) or header[32:].hex()!=BUILD_SHA256:
            raise ValueError('API header layout/build/hook/budget mismatch')
        while source.tell()<size-64:pairs.accept(struct.unpack('<32I',source.read(128)))
        terminal=source.read(64);end=struct.unpack('<16I',terminal)
        if terminal[:8]!=b'MNMDAEND':raise ValueError('API terminal magic mismatch')
        pairs.finish(end,1)
    with path.open('rb') as source:digest=hashlib.file_digest(source,'sha256').hexdigest()
    after=path.stat()
    if (before.st_ino,before.st_size,before.st_mtime_ns)!=(after.st_ino,after.st_size,after.st_mtime_ns):
        raise ValueError('API journal changed during review')
    return {'schema':1,'scope':'Closed original CALL diagnostic journal only; no all-writer, semantic API/output/ownership/borrow lifetime or DRAW-01 closure.','journal_integrity':True,'original_sha256':BUILD_SHA256,'journal_sha256':digest,**pairs.result()}
