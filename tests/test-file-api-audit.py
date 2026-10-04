#!/usr/bin/env python3
"""Synthetic PE32 and x86 text fixtures for the static file API audit."""
import importlib.util
from pathlib import Path
import struct
import tempfile

REPO = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('file_audit', REPO / 'tools/audit-file-apis.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def main():
    data = bytearray(0x600)
    data[:2] = b'MZ'
    struct.pack_into('<I', data, 0x3c, 0x80)
    data[0x80:0x84] = b'PE\0\0'
    struct.pack_into('<HH', data, 0x84, 0x14c, 1)
    struct.pack_into('<H', data, 0x94, 224)
    opt = 0x98
    struct.pack_into('<H', data, opt, 0x10b)
    struct.pack_into('<I', data, opt+28, 0x400000)
    struct.pack_into('<I', data, opt+60, 0x200)
    struct.pack_into('<2I', data, opt+104, 0x1000, 40)
    struct.pack_into('<4I', data, opt+224+8, 0x400, 0x1000, 0x400, 0x200)
    struct.pack_into('<5I', data, 0x200, 0x1040, 0, 0, 0x1080, 0x1060)
    for at in (0x240, 0x260): struct.pack_into('<3I', data, at, 0x10a0, 0x80000007, 0)
    data[0x280:0x28d] = b'KERNEL32.dll\0'
    data[0x2a2:0x2ab] = b'ReadFile\0'
    base, entries = audit.pe_imports(data)
    assert base == 0x400000 and len(entries) == 2
    assert entries[0] == {'dll': 'KERNEL32.dll', 'name': 'ReadFile', 'ordinal': None, 'iat_va': 0x401060}
    assert entries[1]['ordinal'] == 7 and entries[1]['name'] is None and entries[1]['iat_va'] == 0x401064
    # PE32 permits OriginalFirstThunk=0; names then come from FirstThunk.
    struct.pack_into('<I', data, 0x200, 0)
    assert audit.pe_imports(data)[1] == entries
    for broken in (b'not PE', bytes(len(data))):
        try: audit.pe_imports(broken)
        except ValueError: pass
        else: raise AssertionError('Invalid image accepted')
    disassembly = '''
  401000: ff 15 60 10 40 00    call DWORD PTR ds:0x401060
  401006: ff 25 60 10 40 00    jmp DWORD PTR ds:0x401060
  401020: e8 e1 ff ff ff       call 0x401006
  401030: a1 60 10 40 00       mov eax,ds:0x401060
  401035: ff d0                call eax
  401040: ff 15 60 10 40 01    call DWORD PTR ds:0x1401060
'''
    code = audit.instructions(disassembly)
    hits, callers = audit.references(code, 0x401060)
    assert [h['va'] for h in hits] == ['0x401000', '0x401006', '0x401030']
    assert [h['kind'] for h in hits] == ['direct_iat', 'direct_iat', 'iat_reference']
    assert len(callers) == 1 and callers[0]['va'] == '0x401020'
    assert callers[0]['thunk_va'] == '0x401006' and callers[0]['context']
    assert not audit.references(code, 0x401064)[0]
    data[0x300:0x30f] = b'Save\\__temp.vas\0'
    strings = audit.path_strings(data, base)
    assert strings == [{'file_offset': '0x300', 'preferred_va': '0x401100', 'value': 'Save\\__temp.vas'}]
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary) / 'fixture.exe'
        path.write_bytes(data)
        try: audit.audit(path, '0'*64, Path(temporary))
        except ValueError as e: assert 'Unsupported binary hash' in str(e)
        else: raise AssertionError('Unpinned image accepted')
    print('File API audit PE32 imports, ordinals, IAT/thunk references and hash guard passed')


if __name__ == '__main__':
    main()
