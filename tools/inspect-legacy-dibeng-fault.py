#!/usr/bin/env python3
"""Map observed Win16 exception PCs to hash-pinned DIB Engine NE instructions."""
from coverage_evidence import read_bytes, read_json, write_report, evidence_digest
import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import re

from original_x86 import Decoder

ROOT = Path(__file__).resolve().parents[1]
PIN = 'b7c0b676cd8c9960da4c53deae8de846d1251e9534ef4e4b2f05c988306dd2bd'


def ne_segment(raw, ordinal):
    def word(at, size=2):
        if at < 0 or at + size > len(raw):
            raise ValueError('NE field outside file')
        return int.from_bytes(raw[at:at+size], 'little')
    if len(raw) < 64 or raw[:2] != b'MZ':
        raise ValueError('Not an MZ module')
    ne = word(60, 4)
    if raw[ne:ne+2] != b'NE':
        raise ValueError('Not an NE module')
    count, alignment = word(ne+0x1c), word(ne+0x32)
    if type(ordinal) is not int or not 1 <= ordinal <= count or alignment > 16:
        raise ValueError('Invalid NE segment index/alignment')
    table = ne + word(ne+0x22)
    if table + count * 8 > len(raw):
        raise ValueError('NE segment table outside file')
    at = table + (ordinal-1)*8
    sector, size, flags, minimum = (word(at+i) for i in (0, 2, 4, 6))
    if not sector or flags & 8:
        raise ValueError('Require file-backed, non-iterated NE segment')
    start, size = sector << alignment, size or 65536
    if start + size > len(raw):
        raise ValueError('NE segment outside file')
    return dict(ordinal=ordinal, file_offset=start, length=size, flags=flags,
                minimum_allocation=minimum, entry_segment=word(ne+0x16),
                entry_offset=word(ne+0x14)), raw[start:start+size]


def exception_contexts(log):
    lines = log.splitlines()
    result = []
    for i, line in enumerate(lines):
        match = re.match(r'([0-9a-f]+):trace:seh:dispatch_exception eip=([0-9a-f]+)', line)
        if not match:
            continue
        prefix = match[1]+':trace:seh:dispatch_exception '
        fields = {}
        for row in lines[i+1:]:
            if not row.startswith(prefix):
                continue
            body = row[len(prefix):]
            if re.search(r'\beip=', body):
                break
            found = re.findall(r'\b(eax|ebx|cs)=([0-9a-f]+)', body)
            if any(key in fields for key, value in found):
                break
            fields.update(found)
            if {'eax', 'ebx', 'cs'} <= fields.keys():
                result.append(dict(thread=match[1], ip=int(match[2], 16),
                    ax=int(fields['eax'],16)&0xffff, bx=int(fields['ebx'],16)&0xffff,
                    cs=int(fields['cs'],16)))
                break
    return result


def continuation_diagnostic(log, admission, mapped):
    """First-chance INT31 PCs are not by themselves the terminal loader fault."""
    contexts=exception_contexts(log)
    rest=[c for c in contexts if c['ip']>0xffff and c not in [f['context'] for f in mapped]]
    observations=admission.get('readonly_loader_exception_observations',[])
    terminal=rest[-1] if rest else None
    matched=None
    if terminal:
        for i,row in enumerate(observations):
            if len(row)==6 and row[0]==88 and row[2]==terminal['ip'] and row[5]==terminal['cs']:
                if i+1<len(observations) and len(observations[i+1])==6 and observations[i+1][0]==89:
                    matched={'exception':row,'memory_query':observations[i+1]}
    services=[(thread,service) for thread,service in re.findall(
        r'(?m)^([0-9a-f]+):trace:int31:DOSVM_Int31Handler (get|set) descriptor ',log)]
    return {'descriptor_mapping_is_not_terminal_cause_proof':True,
            'observed_dpmi_dispatch':services,
            'later_32bit_exception':terminal,
            'readonly_observer_match':matched,
            'specific_compatibility_fix_validated':False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dll', type=Path, required=True)
    parser.add_argument('--log', type=Path, required=True)
    parser.add_argument('--admission-report', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True, help='New immutable SQLite research report')
    args = parser.parse_args()
    out = args.report if args.report.is_absolute() else ROOT/args.report
    if out.exists() or not out.resolve().is_relative_to(ROOT/'research/runtime'):
        parser.error('Require new immutable research/runtime report')
    raw = args.dll.read_bytes()
    if hashlib.sha256(raw).hexdigest() != PIN:
        raise ValueError('Unreviewed DIB Engine module')
    admission = read_json(args.admission_report)
    if (not admission['success'] or admission['loader_error'] != 998 or
            admission['legacy_surface_available'] or
            not {'DDRAW16.DLL', 'DIBENG.DLL'} <= set(admission['native_16bit_modules'])):
        raise ValueError('Require actual dependency-assisted initialization failure')
    log = args.log.read_text()
    loading = re.search(r"Trying native dll 'DIBENG.DLL'(.*?)Loaded module \"DIBENG.DLL\" : native", log, re.S)
    if not loading:
        raise ValueError('Missing DIB Engine segment-load diagnostics')
    modules = [(int(a), int(b, 16), int(c, 16)) for a, b, c in re.findall(
        r'Loading segment (\d+), hSeg=([0-9a-f]+), flags=([0-9a-f]+)', loading[1])]
    contexts = exception_contexts(log)
    decoder = Decoder()
    decoder.close()
    if decoder.library.cs_open(3, 2, ctypes.byref(decoder.handle)):
        raise ValueError('Cannot open Capstone x86-16 decoder')
    faults = []
    try:
        for context in contexts:
            for ordinal, handle, loaded_flags in modules:
                if context['cs'] != (handle | 1):
                    continue
                segment, code = ne_segment(raw, ordinal)
                if context['ip'] >= len(code):
                    continue
                instruction = decoder.decode(code[context['ip']:context['ip']+15], context['ip'])
                if instruction['bytes'] != 'cd31':
                    raise ValueError('Observed fault is not the reviewed INT31 boundary')
                faults.append(dict(context=context, segment=segment,
                                   observed_segment_handle=handle, loaded_flags=loaded_flags,
                                   instruction=instruction,
                                   service={11:'get_descriptor', 12:'set_descriptor'}.get(context['ax'], 'unclassified')))
    finally:
        decoder.close()
    if [f['service'] for f in faults] != ['get_descriptor', 'set_descriptor']:
        raise ValueError('Unexpected descriptor-fault sequence')
    names = ['tools/inspect-legacy-dibeng-fault.py', 'tools/original_x86.py', 'tests/test-legacy-ne-fault.py']
    report = dict(schema=1, success=True, static_only=True, executed_downloaded_code=False,
                  scope=__doc__, sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names},
                  inputs={str(p):evidence_digest(p)
                          for p in (args.dll.resolve(), args.log.resolve(), args.admission_report.resolve())},
                  dll_sha256=PIN, decoder_version=decoder.version, faults=faults, legacy_surface_available=False,
                  continuation=continuation_diagnostic(log,admission,faults),
                  compatible_host_or_specific_fix_not_validated=True,
                  specification=['https://www.delorie.com/djgpp/doc/dpmi/api/31000b.html',
                                 'https://www.delorie.com/djgpp/doc/dpmi/api/31000c.html'])
    write_report(ROOT,out,(json.dumps(report,indent=2)+'\n').encode())
    print(out)


if __name__ == '__main__':
    main()
