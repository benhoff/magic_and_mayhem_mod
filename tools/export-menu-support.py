#!/usr/bin/env python3
"""Export hash-pinned menu disassembly, data references and optional pseudocode."""
import argparse
import csv
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {'main_menu': (0x4a6a50, 0x4a7970), 'quick_battle': (0x4a7970, 0x4a8430),
          'mini_menu': (0x4b2100, 0x4b2730), 'screen_controller': (0x557000, 0x557900),
          'screen_tick': (0x5595d0,0x559850)}
TABLES = {'main': (0x5c63e4, 14), 'quick': (0x5c641c, 14)}
CALLBACKS = {'main': (0x4a75c0, 0x4a7844), 'quick': (0x4a83a0, 0x4a83fc)}

def module(name, file):
    spec = importlib.util.spec_from_file_location(name, REPO / file)
    result = importlib.util.module_from_spec(spec); spec.loader.exec_module(result)
    return result

def image_offset(data, va, length):
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    opt = pe + 24
    base = struct.unpack_from('<I', data, opt + 28)[0]
    table = opt + struct.unpack_from('<H', data, pe + 20)[0]
    for i in range(struct.unpack_from('<H', data, pe + 6)[0]):
        _, rva, size, raw = struct.unpack_from('<4I', data, table + i*40 + 8)
        if base+rva <= va and va+length <= base+rva+size:
            offset = raw+va-base-rva
            if offset+length <= len(data): return offset
    raise ValueError(f'Address range is not file-backed: {va:x}+{length:x}')

def pinned_image(path):
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != HASH:
        raise ValueError('Unsupported executable hash; refusing menu addresses')
    return data

def export(executable, output, project=None, ghidra=None):
    data = pinned_image(executable)
    audit = module('menu_file_audit', 'tools/audit-file-apis.py')
    base, _ = audit.pe_imports(data)
    code = audit.instructions(subprocess.check_output(['objdump','-d','-Mintel',str(executable)],text=True))
    for name, (start, end) in RANGES.items():
        (output / (name+'.asm')).write_text('\n'.join(row[3] for row in code if start <= row[0] < end)+'\n')
    tables = {}
    for name, (va, count) in TABLES.items():
        tables[name] = {'va': hex(va), 'slots': [hex(v) for v in struct.unpack_from('<'+str(count)+'I',data,image_offset(data,va,count*4))]}
    anchors = []
    for value in audit.path_strings(data, base):
        if any(key in value['value'] for key in ('MainScreen','QuickBattleMainMenu','MiniMenu')):
            # Reuse instruction matching only; these are string references, not IAT calls.
            hits, _ = audit.references(code, int(value['preferred_va'],16))
            anchors.append({'string':value,'references':[{k:v for k,v in hit.items() if k!='kind'} for hit in hits]})
    (output/'anchors.json').write_text(json.dumps({'strings':anchors,'vtables':tables},indent=2)+'\n')
    for name,(start,end) in CALLBACKS.items():
        at=image_offset(data,start,end-start)
        (output/(name+'-callback.bin')).write_bytes(data[at:at+end-start])
    metadata = {'source':str(executable.resolve()),'source_sha256':HASH,'image_base':hex(base),
                'scope':'Static inferred contracts only; no live observation or replacement',
                'ranges':RANGES,'callbacks':CALLBACKS,'ghidra_export':False}
    if project:
        headless=module('menu_decompile','tools/decompile-game.py').find_headless(ghidra)
        if not headless: raise ValueError('Ghidra not found')
        command=[str(headless),str(project.resolve()),'MagicMayhem','-process','Chaos.exe','-readOnly','-noanalysis',
                 '-scriptPath',str(REPO/'tools/ghidra'),'-postScript','ExportMenuObservation.java',str(output),'-max-cpu','2']
        env=dict(os.environ,XDG_CONFIG_HOME=str(output/'config'),XDG_CACHE_HOME=str(output/'cache'),
                 GHIDRA_HEADLESS_JAVA_OPTIONS=f'-Dapplication.cachedir={output}/cache -Dapplication.tempdir={output}/tmp')
        with (output/'ghidra.log').open('w') as log:
            subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
        with (output/'functions.tsv').open() as file: rows=list(csv.DictReader(file,delimiter='\t'))
        required={'004a7110','004a75c0','004a7e40','004a83a0','00557040'}
        if not required <= {row['entry'] for row in rows if row['status']=='ok'} or any(row['status']!='ok' for row in rows):
            raise ValueError('Incomplete selected menu decompilation')
        metadata.update(ghidra_export=True,command=command,function_count=len(rows))
    metadata['input_unchanged']=hashlib.sha256(executable.read_bytes()).hexdigest()==HASH
    if not metadata['input_unchanged']: raise ValueError('Executable changed during export')
    names=list(RANGES)
    artifacts=[output/(name+'.asm') for name in names]+[output/'anchors.json']+[output/(name+'-callback.bin') for name in CALLBACKS]
    if project: artifacts += list((output/'c').glob('*.c'))+[output/'functions.tsv',output/'references.tsv']
    metadata['artifacts']={str(p.relative_to(output)):hashlib.sha256(p.read_bytes()).hexdigest() for p in artifacts}
    (output/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
    return metadata

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable',type=Path,default=REPO/'working/game-nocd/Chaos.exe')
    parser.add_argument('--project',type=Path,help='Existing analyzed MagicMayhem project directory; opened read-only')
    parser.add_argument('--ghidra',type=Path)
    args=parser.parse_args()
    parent=REPO/'working/decompiled';parent.mkdir(parents=True,exist_ok=True)
    output=Path(tempfile.mkdtemp(prefix='menu-support-',dir=parent))
    print(f'Evidence directory: {output}',flush=True)
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try: export(args.executable,output,args.project,args.ghidra)
    finally: subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)

if __name__=='__main__': main()
