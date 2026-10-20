#!/usr/bin/env python3
"""Redirect the pinned disposable game's graphics imports to MnmDraw.dll.

This is a staging operation, not proof that the native services cover gameplay.
The caller must explicitly disable Wine ddraw during a supported-scenario run.
"""
import hashlib
import importlib.util
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
SECTIONS = {
    b'.text': '23610c68f0846eb36a9ccfb8caa3b2e93525223c9fea549326eb880c50d6736b',
    b'.rdata': 'e6af6aaff1a8ec744d8553bf5ee5c1d8a7ff45c7c70aa6dd3d2057f5ffd80bd0',
    b'.data': '7877ebaf42e9abb8422ec8683dc9cd39865a5bba05fb67dcfef43da5fbcafaf3',
    b'.rsrc': 'e0ea12dcf6e9ef2547fc1dd8b3b0858923a93524bded1cc9c750b8861be8b28a',
}
DESCRIPTOR = bytes.fromhex('3c611d0000000000000000003c661d0010501c00')
LOOKUP = bytes.fromhex('24661d0010661d0000000000')
STARTUP_RELEASES = {
    0xbdb20: bytes.fromhex('8b442410508b08ff5108'),
    0xbdb65: bytes.fromhex('8b442410508b10ff5208'),
}


def layout(data):
    if data[:2] != b'MZ' or len(data) < 64:
        raise ValueError('Not a PE image')
    pe = struct.unpack_from('<I', data, 60)[0]
    if data[pe:pe+4] != b'PE\0\0' or struct.unpack_from('<H', data, pe+4)[0] != 0x14c:
        raise ValueError('Expected i386')
    opt = pe+24
    if struct.unpack_from('<H', data, opt)[0] != 0x10b:
        raise ValueError('Expected PE32')
    table = opt+struct.unpack_from('<H', data, pe+20)[0]
    count = struct.unpack_from('<H', data, pe+6)[0]
    sections = [struct.unpack_from('<8sIIII', data, table+i*40) for i in range(count)]

    def offset(rva, length):
        for _, _, base, size, raw in sections:
            if base <= rva and rva+length <= base+size and raw+rva-base+length <= len(data):
                return raw+rva-base
        raise ValueError('RVA not file-backed')

    start = struct.unpack_from('<I', data, opt+104)[0]
    descriptors = []
    for i in range(256):
        at = offset(start+i*20, 20)
        if data[at:at+20] == bytes(20):
            return sections, offset, descriptors
        descriptors.append(at)
    raise ValueError('Unbounded import directory')


def remap_enumerator(data):
    """Redirect both pinned legacy LoadLibrary string; no code byte changes."""
    _, offset, _ = layout(data)
    result = bytearray(data)
    for rva, expected in [(0x1f1a84, b'ddraw.dll\0\0\0'), (0x1e1264, b'DDRAW.DLL\0\0\0')]:
        at = offset(rva, 12)
        if data[at:at+12] != expected:
            raise ValueError('Expected pinned dynamic graphics loader string')
        result[at:at+12] = b'MnmDraw.dll\0'
    return bytes(result)


def retire_startup_probe(data, add_import):
    """Release only the pinned capability check's lost temporary COM aliases.

    Replace each ten-byte final device Release with an ABI-compatible call to
    the guarded adapter. Normal COM callers and the checker result are retained.
    """
    _, offset, _ = layout(data)
    for rva, expected in STARTUP_RELEASES.items():
        at = offset(rva, len(expected))
        if data[at:at+len(expected)] != expected:
            raise ValueError('Pinned startup probe Release bytes differ')
    result = bytearray(add_import(data, 'MnmDraw.dll', 'NativeDrawReleaseStartupProbeV1', b'.mnmret'))
    _, new_offset, descriptors = layout(result)
    iat = struct.unpack_from('<I', result, descriptors[-1]+16)[0]
    pe = struct.unpack_from('<I', result, 60)[0]
    base = struct.unpack_from('<I', result, pe+24+28)[0]
    if base != 0x400000:
        raise ValueError('Pinned startup probe image base differs')
    replacement = bytes.fromhex('ff742410ff15')+struct.pack('<I', base+iat)
    for rva in STARTUP_RELEASES:
        at = new_offset(rva, len(replacement))
        result[at:at+len(replacement)] = replacement
    return bytes(result)


def expected_original_sections(data, staged):
    """Expected original section bytes after the three guarded native edits.

    Resolve the named cleanup thunk from the actual composed import table, so
    later menu/audio/observer imports cannot change the permitted code target.
    """
    _, offset, _ = layout(data)
    _, staged_offset, descriptors = layout(staged)
    matches=[]
    for descriptor in descriptors:
        lookup, _, _, name, iat = struct.unpack_from('<5I',staged,descriptor)
        at=staged_offset(name,1)
        if staged[at:staged.index(b'\0',at)] != b'MnmDraw.dll':continue
        for index in range(256):
            symbol=struct.unpack_from('<I',staged,staged_offset(lookup+4*index,4))[0]
            if not symbol:break
            if symbol & 0x80000000:continue
            at=staged_offset(symbol,2)+2
            if staged[at:staged.index(b'\0',at)] == b'NativeDrawReleaseStartupProbeV1':matches.append(iat+index*4)
        else:raise ValueError('Unbounded native graphics import lookup')
    if len(matches)!=1:raise ValueError('Startup retirement import missing or ambiguous')
    expected=bytearray(remap_enumerator(data))
    for rva,original in STARTUP_RELEASES.items():
        at=offset(rva,len(original))
        if data[at:at+len(original)]!=original:raise ValueError('Pinned startup Release bytes differ')
        expected[at:at+10]=bytes.fromhex('ff742410ff15')+struct.pack('<I',0x400000+matches[0])
    return bytes(expected)


def redirect(data, enumeration=False):
    """Guard all original section bytes, then preserve old lookup/IAT addresses."""
    sections, offset, descriptors = layout(data)
    for name, expected in SECTIONS.items():
        matches = [(raw, size) for n, _, _, size, raw in sections if n.rstrip(b'\0') == name]
        if len(matches) != 1:
            raise ValueError('Missing or duplicate original section')
        raw, size = matches[0]
        if hashlib.sha256(data[raw:raw+size]).hexdigest() != expected:
            raise ValueError('Pinned original section bytes differ: '+name.decode())
    if len(descriptors) < 4 or data[descriptors[3]:descriptors[3]+20] != DESCRIPTOR:
        raise ValueError('Expected original DirectDraw descriptor')
    lookup, _, _, name, iat = struct.unpack('<5I', DESCRIPTOR)
    if data[offset(lookup, 12):offset(lookup, 12)+12] != LOOKUP:
        raise ValueError('Expected original graphics lookup')
    if data[offset(name, 10):offset(name, 10)+10].lower() != b'ddraw.dll\0':
        raise ValueError('Expected original graphics DLL')
    spec = importlib.util.spec_from_file_location('native_draw_import', ROOT/'tools/prepare-shadow-experiment.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    result = bytearray(module.add_import(data, 'MnmDraw.dll', 'NativeDrawAnchor', b'.mnmdraw'))
    _, new_offset, new_descriptors = layout(result)
    anchor = new_descriptors[-1]
    new_name = struct.unpack_from('<I', result, anchor+12)[0]
    if result[new_offset(new_name, 12):new_offset(new_name, 12)+12] != b'MnmDraw.dll\0':
        raise ValueError('Bad native import name')
    struct.pack_into('<I', result, new_descriptors[3]+12, new_name)
    if struct.unpack_from('<I', result, new_descriptors[3]+16)[0] != iat:
        raise ValueError('Original graphics IAT moved')
    return retire_startup_probe(remap_enumerator(bytes(result)), module.add_import) if enumeration else bytes(result)
