#!/usr/bin/env python3
"""Hash-pinned static audit of file-related PE imports and x86 references."""
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
BINARIES = {
    'clean': ('working/game-clean/Chaos.exe', '124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214'),
    'nocd': ('working/game-nocd/Chaos.exe', '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'),
    'jpeg': ('working/game-clean/jpeg.dll', 'a8c2480789bb45ab6ff2d1779e0e7d454b35290f89b20fa3c999a9c0c263328b'),
}
GROUPS = {
    'file_stream': 'CreateFileA CreateFileW ReadFile WriteFile SetFilePointer SetFilePointerEx GetFileSize GetFileSizeEx CloseHandle FlushFileBuffers SetEndOfFile GetFileType GetStdHandle SetStdHandle SetHandleCount',
    'mutation': 'DeleteFileA DeleteFileW MoveFileA MoveFileW MoveFileExA CopyFileA CopyFileW CreateDirectoryA CreateDirectoryW RemoveDirectoryA SetFileAttributesA SetFileAttributesW SetFileTime',
    'enumeration_metadata': 'FindFirstFileA FindNextFileA FindClose GetFileAttributesA GetFileAttributesW GetFileTime CompareFileTime',
    'profile': 'GetPrivateProfileStringA GetPrivateProfileIntA GetPrivateProfileSectionA WritePrivateProfileStringA WritePrivateProfileSectionA',
    'path_drive': 'GetCurrentDirectoryA SetCurrentDirectoryA GetFullPathNameA GetModuleFileNameA GetDriveTypeA GetDiskFreeSpaceA GetTempPathA GetTempFileNameA SearchPathA',
    'mapped_io': 'CreateFileMappingA CreateFileMappingW MapViewOfFile UnmapViewOfFile FlushViewOfFile',
    'media_file': 'mmioOpenA mmioClose mmioRead mmioWrite mmioSeek mmioDescend mmioAscend PlaySoundA sndPlaySoundA mciSendCommandA',
    'version': 'GetFileVersionInfoA GetFileVersionInfoSizeA VerQueryValueA',
    'module_resource': 'LoadLibraryA LoadLibraryW FreeLibrary GetProcAddress GetModuleHandleA FindResourceA FindResourceExA LoadResource LockResource SizeofResource FreeResource LoadIconA LoadCursorA CoCreateInstance',
    'path_encoding': 'GetACP GetOEMCP GetCPInfo WideCharToMultiByte MultiByteToWideChar',
}
GROUPS = {group: names.split() for group, names in GROUPS.items()}
CLASSIFICATION = {name: group for group, names in GROUPS.items() for name in names}


def pe_imports(data):
    """Parse named and ordinal PE32 imports independently of objdump formatting."""
    def u16(offset): return struct.unpack_from('<H', data, offset)[0]
    def u32(offset): return struct.unpack_from('<I', data, offset)[0]
    if data[:2] != b'MZ':
        raise ValueError('Not MZ')
    pe = u32(0x3c)
    opt = pe + 24
    if data[pe:pe+4] != b'PE\0\0' or u16(pe+4) != 0x14c or u16(opt) != 0x10b:
        raise ValueError('Expected i386 PE32')
    base = u32(opt+28)
    section_table = opt + u16(pe+20)
    sections = [struct.unpack_from('<4I', data, section_table+i*40+8) for i in range(u16(pe+6))]

    def offset(rva):
        if rva < u32(opt+60) and rva < len(data): return rva
        for _, start, size, raw in sections:
            if start <= rva < start+size and raw+rva-start < len(data): return raw+rva-start
        raise ValueError(f'RVA outside file: {rva:x}')

    def string(rva):
        at = offset(rva)
        end = data.find(b'\0', at)
        if end < 0: raise ValueError('Unterminated import name')
        return data[at:end].decode('ascii')

    directory, length = struct.unpack_from('<2I', data, opt+104)
    imports = []
    if directory:
        for at in range(directory, directory+length, 20):
            lookup, timestamp, chain, name, iat = struct.unpack_from('<5I', data, offset(at))
            if not any((lookup, timestamp, chain, name, iat)): break
            dll = string(name)
            for index in range(len(data)//4):
                thunk = u32(offset((lookup or iat)+index*4))
                if not thunk: break
                imports.append({'dll': dll, 'name': None if thunk & 0x80000000 else string(thunk+2),
                                'ordinal': thunk & 0xffff if thunk & 0x80000000 else None,
                                'iat_va': base+iat+index*4})
            else: raise ValueError('Unterminated import thunk array')
        else: raise ValueError('Unterminated import directory')
    return base, imports


def instructions(disassembly):
    result = []
    for line in disassembly.splitlines():
        match = re.match(r'^\s*([0-9a-f]+):\s+(?:[0-9a-f]{2}\s+)+\s*([a-z][a-z0-9]*)\s*(.*?)\s*$', line)
        if match: result.append((int(match[1], 16), match[2], match[3], line))
    return result


def references(code, slot):
    pattern = re.compile(r'(?<![0-9a-f])0x' + format(slot, 'x') + r'(?![0-9a-f])')
    hits = []
    for index, (address, mnemonic, operand, line) in enumerate(code):
        if pattern.search(operand):
            hits.append({'va': hex(address), 'kind': 'direct_iat' if mnemonic in ('call', 'jmp') and 'PTR' in operand else 'iat_reference',
                         'instruction': line, 'context': [c[3] for c in code[max(0, index-12):index+4]]})
    thunks = {int(hit['va'], 16) for hit in hits if re.search(r'\bjmp\s', hit['instruction'])}
    callers = []
    for index, (address, mnemonic, operand, line) in enumerate(code):
        target = re.match(r'^0x([0-9a-f]+)(?:\s|$)', operand)
        if mnemonic in ('call', 'jmp') and target and int(target[1], 16) in thunks:
            callers.append({'va': hex(address), 'thunk_va': hex(int(target[1], 16)), 'instruction': line,
                            'context': [c[3] for c in code[max(0, index-12):index+4]]})
    return hits, callers


def path_strings(data, base):
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    table = pe+24+struct.unpack_from('<H', data, pe+20)[0]
    sections = [struct.unpack_from('<4I', data, table+i*40+8)
                for i in range(struct.unpack_from('<H', data, pe+6)[0])]
    result = []
    for match in re.finditer(rb'[\x20-\x7e]{4,}', data):
        value = match.group().decode('ascii')
        if not re.search(r'(?:save[\\/]|\.(?:cfg|ini|vas|wav|avi|jpg)(?:$|[\s";]))', value, re.I): continue
        address = next((base+rva+match.start()-raw for _, rva, size, raw in sections
                        if raw <= match.start() < raw+size), None)
        result.append({'file_offset': hex(match.start()), 'preferred_va': hex(address) if address else None,
                       'value': value})
    return result


def audit(path, expected, output):
    data = path.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != expected: raise ValueError(f'Unsupported binary hash: {path}')
    base, imports = pe_imports(data)
    disassembly = subprocess.run(['objdump', '-d', '-Mintel', str(path)], check=True, capture_output=True, text=True).stdout
    (output / 'disassembly.asm').write_text(disassembly)
    code = instructions(disassembly)
    if not code: raise ValueError('No instructions parsed')
    selected = []
    for entry in imports:
        name = entry['name']
        if name in CLASSIFICATION or (name and name.startswith(('mmio', 'GetPrivateProfile', 'WritePrivateProfile'))):
            hits, callers = references(code, entry['iat_va'])
            selected.append({**entry, 'iat_va': hex(entry['iat_va']), 'group': CLASSIFICATION.get(name, 'other_file'),
                             'references': hits, 'thunk_callers': callers})
    named = {entry['name'] for entry in imports}
    strings = {match.group().decode('ascii') for match in re.finditer(rb'[\x20-\x7e]{4,}', data)}
    if hashlib.sha256(path.read_bytes()).hexdigest() != digest: raise ValueError('Binary changed during audit')
    report = {'source': str(path), 'source_sha256': digest, 'image_base': hex(base), 'architecture': 'PE32 i386',
              'imports': imports, 'file_related_imports': selected,
              'watched_names_not_imported': sorted(set(CLASSIFICATION) - named),
              'watched_names_embedded_but_not_imported': sorted((set(CLASSIFICATION) & strings) - named),
              'path_like_strings': path_strings(data, base),
              'disassembly_sha256': hashlib.sha256(disassembly.encode()).hexdigest(),
              'audit_script_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
              'source_unchanged': True,
              'limitations': ['Static imports/references only; not live call coverage',
                              'Register-loaded IAT calls and deeper wrappers are not fully attributed',
                              'Dynamic lookup, COM/media internals and system DLL implementations are not audited',
                              'Imported CRT support may be unused or unrelated to gameplay assets']}
    (output / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
    return {'source_sha256': digest, 'named_imports': len(named-{None}), 'file_related_imports': len(selected),
            'groups': {g: [e['name'] for e in selected if e['group'] == g] for g in GROUPS}}


def main():
    parent = REPO / 'working/tests/file-api-audit'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Windows file audit: {root}', flush=True)
    def verify(phase):
        result = subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], capture_output=True, text=True)
        (root / f'original-{phase}.log').write_text(result.stdout+result.stderr)
        print(result.stdout, end='', flush=True)
        result.check_returncode()
    verify('before')
    try:
        summary = {}
        for label, (relative, expected) in BINARIES.items():
            output = root / label
            output.mkdir()
            summary[label] = audit(REPO / relative, expected, output)
        (root / 'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
        print({label: value['file_related_imports'] for label, value in summary.items()}, flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
