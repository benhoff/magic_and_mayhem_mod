#!/usr/bin/env python3
"""Export hash-pinned Save/Load screen controls without changing game artifacts."""
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
RANGES = {'load_controls': (0x4a9b40, 0x4aab80),
          'save_controls': (0x4aab80, 0x4abf20),
          'mini_actions': (0x4b23f0, 0x4b25c0),
          'list_controls': (0x4d1060, 0x4d113b),
          'list_selected': (0x4d1df0, 0x4d1e4d),
          'edit_set_text': (0x4cd0e0, 0x4cd130)}

def main():
    subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
    try:
        spec = importlib.util.spec_from_file_location('menu_export', ROOT/'tools/export-menu-support.py')
        exporter = importlib.util.module_from_spec(spec); spec.loader.exec_module(exporter)
        executable = ROOT/'original/Arcane_Nocd/Chaos.exe'
        data = exporter.pinned_image(executable)
        def at(address, size):
            offset = exporter.image_offset(data, address, size)
            return data[offset:offset+size]
        disassembly = subprocess.check_output(['objdump', '-d', '-Mintel', executable], text=True)
        rows = [(int(m[1], 16), line) for line in disassembly.splitlines()
                if (m := re.match(r'\s*([0-9a-f]+):\s', line))]
        parent = ROOT/'working/decompiled'; parent.mkdir(parents=True, exist_ok=True)
        out = Path(tempfile.mkdtemp(prefix='save-load-menus-', dir=parent))
        tables = {hex(va): [hex(v) for v in struct.unpack('<14I', at(va, 56))]
                  for va in (0x5c64c4, 0x5c64fc)}
        for name, (start, end) in RANGES.items():
            (out/(name+'.asm')).write_text('\n'.join(line for va, line in rows if start <= va < end)+'\n')
        (out/'tables.json').write_text(json.dumps(tables, indent=2)+'\n')
        anchors = {hex(start): at(start, min(16, end-start)).hex() for start, end in RANGES.values()}
        unchanged = hashlib.sha256(executable.read_bytes()).hexdigest() == exporter.HASH
        if not unchanged: raise ValueError('Executable changed during export')
        report = {'schema': 1, 'success': True, 'source_sha256': exporter.HASH,
                  'input_unchanged': unchanged, 'scope': 'Static instructions and vtable candidates only; no live integration or replacement',
                  'ranges': RANGES, 'tables': tables, 'anchors': anchors,
                  'sources': {'tools/export-save-load-menus.py': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},
                  'artifacts': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()}}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out)
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)

if __name__ == '__main__': main()
