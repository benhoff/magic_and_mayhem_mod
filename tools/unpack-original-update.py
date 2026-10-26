#!/usr/bin/env python3
"""Recover the supplied update payload by running its unchanged x86 extractor.

Only owned in-memory file/allocator endpoints are provided. Execution stops
before CreateProcessA: neither the updater nor its extracted program is launched.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import evidence_digest, write_report
from original_x86 import PE32

ROOT = Path(__file__).resolve().parents[1]
HASH = '90a032fef35f7ffc21fa9cf9aea991147e9c2f92a5b550b63e6768825d8c80b3'
ID = 'RS.alternate-canvas-provenance'


def unpack(raw):
    import unicorn
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
    from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_EIP

    if unicorn.__version__ != '2.1.4' or hashlib.sha256(raw).hexdigest() != HASH:
        raise ValueError('Require pinned update and Unicorn2.1.4')
    image = PE32(raw)
    if struct.unpack('<2I', raw[-8:]) != (0x4a4b4358, 0x4a00):
        raise ValueError('Unexpected compressed-overlay footer')
    u = Uc(UC_ARCH_X86, UC_MODE_32)
    u.mem_map(0, 4096)
    u.mem_write(0, struct.pack('<I', 0xffffffff))
    u.mem_map(image.base, (image.length + 4095) & ~4095)
    for s in image.sections:
        if s['stored']:
            u.mem_write(s['address'], image.read(s['address'], s['stored']))
    u.mem_map(0x10000000, 0x10000)
    u.mem_map(0x11000000, 0x1000000)
    u.mem_map(0x20000000, 0x100000)
    imports = {0x10000000 + i * 16: r['symbol'] for i, r in enumerate(image.imports())}
    for address, row in zip(imports, image.imports()):
        u.mem_write(row['address'], struct.pack('<I', address))
    output = bytearray()
    calls = []
    cursor = 0
    heap = 0x11000000
    stopped = False
    def words(at, n):
        return struct.unpack('<' + 'I' * n, u.mem_read(at, n * 4))
    def put(at, value):
        u.mem_write(at, struct.pack('<I', value & 0xffffffff))
    def text(at):
        return bytes(u.mem_read(at, 1024)).split(b'\0', 1)[0].decode('ascii')
    def hook(mu, address, size, data):
        nonlocal cursor, heap, stopped
        if address not in imports:
            raise ValueError('Unknown owned endpoint')
        name = imports[address]
        sp = mu.reg_read(UC_X86_REG_ESP)
        args = words(sp + 4, 10)
        result = 0
        count = 0
        if name == 'CreateProcessA':
            if text(args[0]) != 'C:\\owned-update.tmp' or args[1] or len(output) < 512:
                raise ValueError('Unexpected extracted-program request')
            stopped = True
            mu.emu_stop()
            return
        if name == 'GlobalAlloc':
            count = 2
            if not 0 < args[1] <= 0x400000:
                raise ValueError('Allocation bound')
            result = heap
            heap += (args[1] + 4095) & ~4095
            if heap >= 0x12000000:
                raise ValueError('Heap budget')
        elif name == 'GetModuleFileNameA':
            count = 3
            value = b'C:\\owned-wrapper.exe\0'
            if args[0] not in (0, image.base) or args[2] < len(value):
                raise ValueError('Module-name boundary')
            mu.mem_write(args[1], value)
            result = len(value) - 1
        elif name == 'GetModuleHandleA':
            count = 1
            result = image.base
        elif name == 'CreateFileA':
            count = 7
            path = text(args[0])
            if path == 'C:\\owned-wrapper.exe' and args[1] == 0x80000000 and args[4] == 3:
                result = 1
            elif path == 'C:\\owned-update.tmp' and args[1] == 0x40000000 and args[4] == 2:
                result = 2
            else:
                raise ValueError('Unexpected file request: ' + path)
        elif name == 'SetFilePointer':
            count = 4
            signed = struct.unpack('<i', struct.pack('<I', args[1]))[0]
            if args[0] != 1 or args[2] or args[3] not in (0, 2):
                raise ValueError('Unexpected seek')
            cursor = (len(raw) if args[3] == 2 else 0) + signed
            if not 0 <= cursor <= len(raw):
                raise ValueError('Seek outside input')
            result = cursor
        elif name == 'ReadFile':
            count = 5
            if args[0] != 1 or args[2] > 4096 or args[4]:
                raise ValueError('Unexpected read')
            chunk = raw[cursor:cursor + args[2]]
            cursor += len(chunk)
            if chunk:
                mu.mem_write(args[1], chunk)
            put(args[3], len(chunk))
            result = 1
        elif name == 'WriteFile':
            count = 5
            if args[0] != 2 or args[2] > 4096 or args[4] or len(output) + args[2] > 8 * 1024 * 1024:
                raise ValueError('Unexpected/budgeted output write')
            output.extend(mu.mem_read(args[1], args[2]))
            put(args[3], args[2])
            result = 1
        elif name == 'GetTempPathA':
            count = 2
            value = b'C:\\\0'
            if args[0] < len(value):
                raise ValueError('Temp-path capacity')
            mu.mem_write(args[1], value)
            result = len(value) - 1
        elif name == 'GetTempFileNameA':
            count = 4
            mu.mem_write(args[3], b'C:\\owned-update.tmp\0')
            result = 1
        elif name == 'CloseHandle':
            count = 1
            if args[0] not in (1, 2):
                raise ValueError('Unknown handle')
            result = 1
        else:
            raise ValueError('Unadmitted endpoint: ' + name)
        calls.append(dict(name=name, result=result))
        if len(calls) > 20000:
            raise ValueError('Endpoint budget')
        mu.reg_write(UC_X86_REG_EAX, result)
        mu.reg_write(UC_X86_REG_ESP, sp + 4 + count * 4)
        mu.reg_write(UC_X86_REG_EIP, words(sp, 1)[0])
    u.hook_add(UC_HOOK_CODE, hook, begin=0x10000000, end=0x1000ffff)
    sp = 0x200ff000
    u.mem_write(sp, struct.pack('<5I', 0x1000fff0, 0, 0, 0, 0))
    u.reg_write(UC_X86_REG_ESP, sp)
    u.emu_start(0x401000, 0x1000fff1, timeout=60000000, count=500000000)
    if not stopped:
        raise ValueError('Extractor did not reach pre-launch boundary within budget: ' +
                         str(dict(pc=hex(u.reg_read(UC_X86_REG_EIP)), output=len(output),
                                  input_cursor=cursor, calls=calls[-12:])))
    for s in image.sections:
        if s['executable'] and bytes(u.mem_read(s['address'], s['stored'])) != image.read(s['address'], s['stored']):
            raise ValueError('Extractor text changed')
    return bytes(output), dict(emulator='Unicorn2.1.4', endpoint_calls=calls,
                              extractor_text_unchanged=True, extracted_program_launched=False)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    a = p.parse_args()
    register = read_register()
    b = next(b for b in register['behaviors'] if b['id'] == ID)
    names = required_sources(b) | {'tools/unpack-original-update.py', 'tools/original_x86.py'}
    sources = {n: evidence_digest(ROOT / n, root=ROOT) for n in sorted(names)}
    report = dict(schema=1, success=False, static_only=True, sources=sources,
                  claims=[dict(behavior=ID, contract_sha256=behavior_contract(b, {v['id']: v for v in register['builds']}))],
                  original_owner_recovered=False, all_writers_closed=False)
    before = False
    try:
        subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        raw = (ROOT / 'original/magicmayhemuksp1.exe').read_bytes()
        payload, details = unpack(raw)
        path = a.report.removesuffix('.json') + '-payload.bin'
        sha = write_report(ROOT, path, payload)
        report.update(success=True, wrapper_sha256=HASH,
                      payload=dict(record=path, sha256=sha, bytes=len(payload)), **details)
    except Exception as error:
        report['error'] = str(error)
    finally:
        if before:
            subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
            report['original_manifest_verified_before_after'] = True
        report['sources_stable'] = all(evidence_digest(ROOT / n, root=ROOT) == h for n, h in sources.items())
        report['success'] &= report['sources_stable']
        sha = write_report(ROOT, a.report, (json.dumps(report, indent=2) + '\n').encode())
    print(json.dumps({k: report.get(k) for k in ('success', 'error', 'payload')} | dict(sha256=sha)))
    return 0 if report['success'] else 1


def read_register():
    from coverage_evidence import read_json
    return read_json(ROOT / 'research/runtime/coverage/register.json', root=ROOT)


if __name__ == '__main__':
    raise SystemExit(main())
