#!/usr/bin/env python3
"""Export pinned original Multiplayer references and selected read-only pseudocode."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project', type=Path, help='Existing read-only MagicMayhem Ghidra project directory')
    parser.add_argument('--ghidra', type=Path, help='Explicit existing Ghidra installation or headless launcher')
    parser.add_argument('--entry', action='append', type=lambda s: int(s, 0), default=[])
    parser.add_argument('--table', action='append', type=lambda s: int(s, 0), default=[], help='Candidate 14-DWORD screen vtable')
    parser.add_argument('--text', action='append', type=lambda s: int(s, 0), default=[], help='Candidate bounded original string')
    parser.add_argument('--config', action='append', type=Path, default=[], help='Read-only installed game config path relative to game-nocd')
    args = parser.parse_args()
    if args.entry and not args.project:
        parser.error('--entry requires --project')
    if any(not 0x400000 <= address < 0x5a0000 for address in args.entry):
        parser.error('Entry is outside the pinned executable code interval')
    parent = ROOT / 'working/decompiled'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='multiplayer-controls-', dir=parent))
    print(out, flush=True)
    subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
    try:
        exporter = module('multiplayer_image', 'tools/export-menu-support.py')
        audit = module('multiplayer_paths', 'tools/audit-file-apis.py')
        exe = ROOT / 'original/Arcane_Nocd/Chaos.exe'; data = exporter.pinned_image(exe)
        disassembly = subprocess.check_output(['objdump', '-d', '-Mintel', exe], text=True)
        (out / 'original.asm').write_text(disassembly)
        rows = [(int(m[1], 16), line) for line in disassembly.splitlines()
                if (m := re.match(r'\s*([0-9a-f]+):\s', line))]
        receiver = {hex(a): [line for _, line in rows if hex(a) in line] for a in (0x6a5018, 0x6de750, 0x658970)}
        strings = [s for s in audit.path_strings(data, 0x400000) if any(k in s['value'].lower() for k in ('multiplayer','multiplayergameselect'))]
        references = {s['preferred_va']: [line for _, line in rows if hex(int(s['preferred_va'], 16)) in line]
                      for s in strings}
        tables = {hex(a): [hex(v) for v in struct.unpack_from('<14I', data, exporter.image_offset(data, a, 56))]
                  for a in args.table}
        texts = {}
        for a in args.text:
            offset = exporter.image_offset(data, a, 1); raw = data[offset:offset+1024]
            if b'\0' not in raw: raise ValueError('Unterminated candidate string')
            texts[hex(a)] = raw.split(b'\0', 1)[0].decode('cp1252')
        (out / 'references.json').write_text(json.dumps(dict(receiver=receiver, strings=strings, references=references,
                                                           tables=tables, texts=texts), indent=2) + '\n')
        anchors = {hex(a): data[exporter.image_offset(data, a, 16):exporter.image_offset(data, a, 16)+16].hex()
                   for a in args.entry}
        command = None
        if args.entry:
            headless = module('multiplayer_decompiler', 'tools/decompile-game.py').find_headless(args.ghidra)
            if not headless: raise ValueError('Ghidra unavailable')
            command = [str(headless), str(args.project.resolve()), 'MagicMayhem', '-process', 'Chaos.exe',
                       '-readOnly', '-noanalysis', '-scriptPath', str(ROOT / 'tools/ghidra'), '-postScript',
                       'ExportPersistenceProgression.java', str(out / 'c'),
                       *[f'{a:x}' for a in sorted(set(args.entry))], '-max-cpu', '2']
            env = dict(os.environ, XDG_CONFIG_HOME=str(out / 'config'), XDG_CACHE_HOME=str(out / 'cache'),
                       GHIDRA_HEADLESS_JAVA_OPTIONS=f'-Dapplication.cachedir={out}/cache -Dapplication.tempdir={out}/tmp')
            with (out / 'ghidra.log').open('w') as log:
                subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=600)
            if any(not (out / 'c' / f'{a:08x}.c').exists() for a in args.entry):
                raise ValueError('Incomplete selected Multiplayer export')
        unchanged = hashlib.sha256(exe.read_bytes()).hexdigest() == exporter.HASH
        if not unchanged: raise ValueError('Original input changed during export')
        installed_inputs = {}
        if args.config: (out / 'inputs').mkdir()
        for i, relative in enumerate(args.config):
            if relative.is_absolute() or '..' in relative.parts: raise ValueError('Config path escapes installed game')
            source = ROOT / 'working/game-nocd' / relative; raw = source.read_bytes()
            (out / 'inputs' / f'{i}.cfg').write_bytes(raw)
            installed_inputs[relative.as_posix()] = hashlib.sha256(raw).hexdigest()
            if hashlib.sha256(source.read_bytes()).hexdigest() != installed_inputs[relative.as_posix()]:
                raise ValueError('Installed input changed during capture')
        artifacts = [out / 'original.asm', out / 'references.json', *list((out / 'c').glob('*')), *list((out / 'inputs').glob('*'))]
        sources = ['tools/export-multiplayer-menu.py', 'tools/export-menu-support.py', 'tools/audit-file-apis.py']
        if command: sources += ['tools/decompile-game.py', 'tools/ghidra/ExportPersistenceProgression.java']
        report = dict(success=True, source_sha256=exporter.HASH, input_unchanged=True, entries=args.entry,
                      anchors=anchors, command=command, scope='Static receiver/path references and selected read-only pseudocode; no original execution, live menu admission, native presentation or replacement.',
                      tables=tables, texts=texts,
                      installed_inputs=installed_inputs,
                      sources={p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sources},
                      artifacts={str(p.relative_to(out)): hashlib.sha256(p.read_bytes()).hexdigest() for p in artifacts if p.is_file()})
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    finally:
        subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)


if __name__ == '__main__':
    main()
