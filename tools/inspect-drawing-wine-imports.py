#!/usr/bin/env python3
"""Pin the selected installed Wine PE32 submission imports; never patch files."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
PINS = {'ddraw': '24c8c8fbbe0bac100efa5a716ebc78506e0f42e80d2af019b641911c0847ab28',
        'wined3d': '3481e6904716421951615d8ee2ca097b55ea20eee65dea6bac1596426d950c82'}
NAMES = ('wined3d_device_context_blt', 'wined3d_swapchain_present')


class Pe32:
    def __init__(self, data):
        self.data = data
        if len(data) < 64 or data[:2] != b'MZ':
            raise ValueError('Missing DOS header')
        pe = self.word(60)
        if self.read(pe, 24)[:4] != b'PE\0\0' or self.short(pe+4) != 0x14c:
            raise ValueError('Expected PE32 i386')
        opt = pe+24
        count, size = self.short(pe+6), self.short(pe+20)
        if not 1 <= count <= 96 or size < 224 or self.short(opt) != 0x10b:
            raise ValueError('Invalid PE32 optional header')
        self.read(opt, size)
        self.directories = [struct.unpack('<II', self.read(opt+96+i*8, 8)) for i in range(2)]
        self.sections = []
        for i in range(count):
            vsize, rva, rawsize, offset = struct.unpack('<4I', self.read(opt+size+i*40+8, 16))
            self.read(offset, rawsize)
            self.sections.append((rva, rawsize, offset))

    def read(self, offset, size):
        if offset < 0 or size < 0 or offset+size > len(self.data):
            raise ValueError('PE file bounds')
        return self.data[offset:offset+size]

    def short(self, offset):
        return struct.unpack('<H', self.read(offset, 2))[0]

    def word(self, offset):
        return struct.unpack('<I', self.read(offset, 4))[0]

    def offset(self, rva, size=1):
        matches = [off+rva-start for start, extent, off in self.sections
                   if start <= rva and rva+size <= start+extent]
        if len(matches) != 1:
            raise ValueError('Ambiguous/unbacked RVA')
        return matches[0]

    def string(self, rva):
        out = bytearray()
        for i in range(512):
            ch = self.data[self.offset(rva+i)]
            if not ch:
                return out.decode('ascii')
            out.append(ch)
        raise ValueError('Unterminated PE name')

    def imports(self):
        rva, size = self.directories[1]
        rows = []
        for i in range(min(size//20, 256)):
            ilt, stamp, chain, name, iat = struct.unpack('<5I', self.read(self.offset(rva+i*20, 20), 20))
            if not any((ilt, stamp, chain, name, iat)):
                return rows
            library = self.string(name)
            for j in range(4096):
                entry = self.word(self.offset((ilt or iat)+j*4, 4))
                if not entry:
                    break
                self.offset(iat+j*4, 4)
                symbol = '#' + str(entry & 65535) if entry & 0x80000000 else self.string(entry+2)
                rows.append(dict(library=library, name=symbol, iat_rva=iat+j*4))
            else:
                raise ValueError('Import thunk budget')
        raise ValueError('Missing import terminator')

    def exports(self):
        rva, size = self.directories[0]
        values = struct.unpack('<IIHH7I', self.read(self.offset(rva, 40), 40))
        base, funcs, names, functions, strings, ordinals = values[5:]
        if funcs > 65536 or names > funcs:
            raise ValueError('Export budget')
        rows = []
        for i in range(names):
            ordinal = self.short(self.offset(ordinals+i*2, 2))
            if ordinal >= funcs:
                raise ValueError('Export ordinal')
            target = self.word(self.offset(functions+ordinal*4, 4))
            self.offset(target)
            rows.append(dict(name=self.string(self.word(self.offset(strings+i*4, 4))),
                             ordinal=base+ordinal, rva=target, forwarded=rva <= target < rva+size))
        return rows


def inspect(ddraw, wined3d):
    blobs = {'ddraw': ddraw, 'wined3d': wined3d}
    hashes = {name: hashlib.sha256(data).hexdigest() for name, data in blobs.items()}
    if hashes != PINS:
        raise ValueError('Installed Wine module pin mismatch')
    imports, exports = Pe32(ddraw).imports(), Pe32(wined3d).exports()
    selected = []
    for name in NAMES:
        source = [r for r in imports if r['name'] == name and r['library'].casefold() == 'wined3d.dll']
        target = [r for r in exports if r['name'] == name and not r['forwarded']]
        if len(source) != 1 or len(target) != 1:
            raise ValueError('Missing/ambiguous selected Wine boundary')
        selected.append(dict(name=name, iat_rva=source[0]['iat_rva'], export_rva=target[0]['rva'],
                             export_ordinal=target[0]['ordinal']))
    return dict(schema=1, success=True, module_sha256=hashes, selected=selected,
                original_game_executed=False, runtime_abi_validated=False,
                source_submission_identity_bound=False,
                scope='Exact installed Wine11.16PE32 named import/export inventory only; no mutation, execution or private object-layout claim.')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--wine-directory', type=Path, default=Path('/usr/lib/wine/i386-windows'))
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    report = inspect((a.wine_directory/'ddraw.dll').read_bytes(), (a.wine_directory/'wined3d.dll').read_bytes())
    report['sources'] = {str(Path(__file__).relative_to(ROOT)): hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
    a.report.parent.mkdir(parents=True, exist_ok=True)
    a.report.write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report['selected']))


if __name__ == '__main__':
    main()
