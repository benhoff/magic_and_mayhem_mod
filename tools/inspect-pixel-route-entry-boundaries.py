#!/usr/bin/env python3
"""Review original PE entry boundaries; retain computed and external unknowns."""
import argparse
from bisect import bisect_right
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('caller_windows', ROOT/'tools/inspect-pixel-route-caller-ranges.py')
caller_windows = importlib.util.module_from_spec(spec)
spec.loader.exec_module(caller_windows)
WINDOWS, BUILD = caller_windows.WINDOWS, caller_windows.BUILD


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


class PE32:
    """Bounded raw-backed PE32 reader, not a Windows loader or disassembler."""
    def __init__(self, data):
        self.data = data
        if len(data) < 64 or data[:2] != b'MZ':
            raise ValueError('Not an MZ image')
        pe = self.u32(60)
        if self.raw(pe, 4) != b'PE\0\0':
            raise ValueError('Not a PE image')
        optional, length = pe+24, self.u16(pe+20)
        self.raw(optional, length)
        if self.u16(pe+4) != 0x14c or length < 96 or self.u16(optional) != 0x10b:
            raise ValueError('Not a complete i386 PE32 header')
        count = self.u16(pe+6)
        if not 0 < count <= 96:
            raise ValueError('Unsupported section count')
        self.base = self.u32(optional+28)
        self.entry = self.u32(optional+16)
        self.headers = self.u32(optional+60)
        self.raw(0, self.headers)
        self.flags = self.u16(pe+22)
        directories = self.u32(optional+92)
        if directories > 16 or 96+directories*8 > length:
            raise ValueError('Truncated/unsupported data directories')
        self.directories = [struct.unpack('<II', self.raw(optional+96+i*8, 8)) for i in range(directories)]
        self.sections = []
        for i in range(count):
            at = optional+length+i*40
            header = self.raw(at, 40)
            virtual, rva, size, offset = struct.unpack_from('<4I', header, 8)
            self.raw(offset, size)
            if rva+size > 0x100000000 or rva < self.headers:
                raise ValueError('Invalid raw-backed section extent')
            if any(rva < s['rva']+s['size'] and s['rva'] < rva+size for s in self.sections):
                raise ValueError('Ambiguous raw-backed virtual sections')
            self.sections.append(dict(name=header[:8].rstrip(b'\0').decode('ascii', errors='replace'),
                                      rva=rva, size=size, offset=offset, virtual=virtual,
                                      executable=bool(struct.unpack_from('<I', header, 36)[0] & 0x20000000)))

    def raw(self, offset, size):
        if offset < 0 or size < 0 or offset+size > len(self.data):
            raise ValueError('Truncated raw image')
        return self.data[offset:offset+size]

    def u16(self, offset):
        return struct.unpack('<H', self.raw(offset, 2))[0]

    def u32(self, offset):
        return struct.unpack('<I', self.raw(offset, 4))[0]

    def offset(self, rva, size=1):
        if rva < 0 or size < 0 or rva+size > 0x100000000:
            raise ValueError('Wrapping RVA')
        if rva+size <= self.headers:
            return rva
        for s in self.sections:
            if s['rva'] <= rva and rva+size <= s['rva']+s['size']:
                return s['offset']+rva-s['rva']
        raise ValueError('RVA is not raw-backed')

    def read(self, rva, size):
        return self.raw(self.offset(rva, size), size)

    def word(self, rva):
        return struct.unpack('<I', self.read(rva, 4))[0]

    def string(self, rva):
        result = bytearray()
        for i in range(512):
            value = self.read(rva+i, 1)[0]
            if not value:
                return result.decode('ascii', errors='replace')
            result.append(value)
        raise ValueError('Unterminated PE string')

    def directory(self, index):
        return self.directories[index] if index < len(self.directories) else (0, 0)

    def exports(self):
        start, size = self.directory(0)
        if not start or not size:
            return []
        self.read(start, 40)
        ordinal, functions, names, eat, name_table, ordinals = struct.unpack('<6I', self.read(start+16, 24))
        if functions > 65536 or names > functions:
            raise ValueError('Unsupported export table extent')
        labels = {}
        for i in range(names):
            index = struct.unpack('<H', self.read(ordinals+i*2, 2))[0]
            if index >= functions:
                raise ValueError('Export name ordinal outside EAT')
            labels.setdefault(index, []).append(self.string(self.word(name_table+i*4)))
        rows = []
        for i in range(functions):
            rva = self.word(eat+i*4)
            if rva:
                forwarder = self.string(rva) if start <= rva < start+size else None
                rows.append(dict(ordinal=ordinal+i, names=labels.get(i, []), rva=rva,
                                 target=None if forwarder else self.base+rva, forwarder=forwarder))
        return rows

    def imports(self):
        start, size = self.directory(1)
        if not start or not size:
            return []
        rows = []
        for i in range(min(4096, size//20)):
            original, timestamp, chain, name, first = struct.unpack('<5I', self.read(start+i*20, 20))
            if not any((original, timestamp, chain, name, first)):
                return rows
            symbols = []
            for j in range(65536):
                value = self.word((original or first)+j*4)
                if not value:
                    break
                symbols.append({'ordinal': value & 0xffff} if value & 0x80000000 else {'name': self.string(value+2)})
            else:
                raise ValueError('Unterminated import thunk array')
            rows.append(dict(module=self.string(name), symbols=symbols))
        raise ValueError('Unterminated import directory')

    def tls_callbacks(self):
        start, size = self.directory(9)
        if not start or not size:
            return []
        if size < 24:
            raise ValueError('Truncated TLS directory')
        array = self.word(start+12)
        if not array:
            return []
        rows = []
        for i in range(4096):
            target = self.word(array-self.base+i*4)
            if not target:
                return rows
            rows.append(target)
        raise ValueError('Unterminated TLS callbacks')

    def relocations(self):
        start, size = self.directory(5)
        if not start or not size:
            return dict(blocks=0, highlow=0, targets=[])
        at, blocks, highlow, targets = 0, 0, 0, []
        while at < size:
            page, length = struct.unpack('<II', self.read(start+at, 8))
            if length < 8 or length % 2 or length > size-at:
                raise ValueError('Invalid relocation block')
            blocks += 1
            for i in range((length-8)//2):
                value = struct.unpack('<H', self.read(start+at+8+i*2, 2))[0]
                if value >> 12 == 3:
                    highlow += 1
                    site = page+(value & 4095)
                    try:
                        target = self.word(site)
                    except ValueError:
                        targets.append(dict(rva=site, raw_backed=False))
                    else:
                        targets.append(dict(rva=site, raw_backed=True, target=target))
            at += length
        return dict(blocks=blocks, highlow=highlow, targets=targets)


def inspect_module(data, primary, windows=WINDOWS):
    image = PE32(data)
    intervals = []
    for route, (start, end) in windows.items():
        offset = primary.offset(start-primary.base, end-start)
        for representation, low, high in (('preferred_va', start, end), ('target_rva', start-primary.base, end-primary.base),
                                           ('target_file_offset', offset, offset+end-start)):
            intervals.append((low, high, route, representation))
    intervals.sort()
    starts = [row[0] for row in intervals]
    maximum, ends = 0, []
    for row in intervals:
        maximum = max(maximum, row[1])
        ends.append(maximum)
    candidates, relative = [], []
    for section in image.sections:
        raw = image.raw(section['offset'], section['size'])
        for i in range(max(0, len(raw)-3)):
            value = struct.unpack_from('<I', raw, i)[0]
            index = bisect_right(starts, value)-1
            # File offsets and RVAs can coincide, including in the original
            # image. Retain every interpretation instead of choosing one.
            while index >= 0 and ends[index] > value:
                low, high, route, representation = intervals[index]
                if value < high:
                    candidates.append(dict(route=route, representation=representation, value=value,
                                           rva=section['rva']+i, file_offset=section['offset']+i,
                                           section=section['name'], executable_section=section['executable']))
                index -= 1
            if section['executable'] and i+5 <= len(raw) and raw[i] in (0xe8, 0xe9):
                site = image.base+section['rva']+i
                target = (site+5+struct.unpack_from('<i', raw, i+1)[0]) & 0xffffffff
                for route, (start, end) in windows.items():
                    if start <= target < end:
                        relative.append(dict(route=route, preferred_site=site, target=target,
                                             internal=bool(start <= site < end and image.base == primary.base)))
    exports, tls, imports, relocations = image.exports(), image.tls_callbacks(), image.imports(), image.relocations()
    selected = []
    boundaries = [('entry', image.base+image.entry)] if image.entry else []
    boundaries += [('export', row['target']) for row in exports if row['target'] is not None]
    boundaries += [('tls', target) for target in tls]
    for kind, target in boundaries:
        for route, (start, end) in windows.items():
            if start <= target < end:
                selected.append(dict(route=route, kind=kind, preferred_target=target))
    relocated = [dict(route=route, **row) for row in relocations['targets'] if row['raw_backed']
                 for route, (start, end) in windows.items() if start <= row['target'] < end]
    return dict(image_base=image.base, entry_rva=image.entry, characteristics=image.flags,
                exports=exports, tls_callbacks=tls, imports=imports,
                relocations={k:v for k,v in relocations.items() if k != 'targets'},
                boundary_candidates=selected, relocation_candidates=relocated,
                stored_byte_candidates=candidates, rel32_byte_candidates=relative,
                global_unreachable_proved=False,
                limits='Byte candidates require instruction/data review. Foreign preferred bases/RVAs do not establish runtime mappings. Computed addresses, callbacks constructed at runtime, system DLLs and unpacked payloads remain outside this scan.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--installed-jpeg', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_text())
    if any(sha(ROOT/p) != h for p,h in declaration['sources'].items()):
        parser.error('Prospective source changed')
    report = dict(schema=1, success=False, kind='static', sources=declaration['sources'], claims=declaration['claims'],
                  modules=[], global_unreachable_proved=False, original_game_executed=False)
    verified = False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        verified = True
        selected = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(selected) != BUILD:
            raise ValueError('Unsupported original game')
        primary = PE32(selected.read_bytes())
        paths = sorted(p for p in (ROOT/'original').rglob('*') if p.is_file() and p.suffix.lower() in ('.exe','.dll','.drv'))
        paths.append(args.installed_jpeg.resolve())
        for path in paths:
            before = sha(path)
            row = dict(path=str(path), sha256=before, selected_build=path==selected,
                       group='supplied_media' if path.is_relative_to(ROOT/'original') else 'admitted_installed_jpeg')
            try:
                row['analysis'] = inspect_module(path.read_bytes(), primary)
            except ValueError as error:
                if path in (selected,args.installed_jpeg.resolve()):
                    raise
                row['unsupported'] = str(error)
            if sha(path) != before:
                raise ValueError('Module changed during scan')
            report['modules'].append(row)
        report.update(success=True, original_sha256=BUILD,
                      windows={k:list(v) for k,v in WINDOWS.items()},
                      installed_jpeg_stable=True,
                      scope='Preferred-address/RVA/file-offset byte candidates plus PE entry/export/TLS/relocation/import boundaries in supplied media and admitted installed JPEG. Only the pinned original has matching-build function identities. No live credit, global dead-code, complete disassembly or system/unpacked-module closure.')
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
            report['original_verified_before_after'] = verified
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT/p) == h for p,h in declaration['sources'].items())
        report['success'] &= report['sources_stable']
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open('x') as stream:
            json.dump(report,stream,indent=2);stream.write('\n')
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
