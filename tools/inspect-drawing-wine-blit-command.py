#!/usr/bin/env python3
"""Pin selected Wine blit emission/table bytes; no execution or module mutation."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('wine_imports',ROOT/'tools/inspect-drawing-wine-imports.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)


def inspect(data):
    if hashlib.sha256(data).hexdigest()!=module.PINS['wined3d']:raise ValueError('Pinned Wine module mismatch')
    pe=module.Pe32(data)
    checks={0x1de393:bytes.fromhex('e8181b0000'),0x1dfeb0:bytes.fromhex('5589e5575653'),
            0x1dff18:bytes.fromhex('c7002a000000'),0x1d1fe5:bytes.fromhex('ff148d20d42410'),
            0x1ce280:bytes.fromhex('5589e583ec68')}
    for rva,expected in checks.items():
        if pe.read(pe.offset(rva,len(expected)),len(expected))!=expected:raise ValueError('Selected Wine instruction mismatch')
    table=pe.read(pe.offset(0x24d420,51*4),51*4)
    entries=struct.unpack('<51I',table)
    if entries[42]!=0x101ce280 or any(not 0x10001000<=v<0x10220000 for v in entries):raise ValueError('Selected command dispatch table mismatch')
    return dict(schema=1,success=True,module_sha256=module.PINS['wined3d'],
                selected_instructions=[dict(rva=rva,bytes=value.hex()) for rva,value in checks.items()],
                emitter=dict(rva=0x1dfeb0,command_opcode=42,command_bytes=100),
                dispatch=dict(table_rva=0x24d420,entries=51,sha256=hashlib.sha256(table).hexdigest(),
                              selected_opcode=42,handler_rva=0x1ce280),
                original_game_executed=False,worker_identity_observed=False,source_storage_owned=False,
                egl_temporal_assignment_performed=False,
                scope='Exact selected installedWine11.16PE32instruction/table inventory. Onlyopcode42entrymapped;otherhandlers/layouts/indirectroutes remainunknown. Noexecution/semantic equivalence or source/EGLownership.')


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--module',type=Path,default=Path('/usr/lib/wine/i386-windows/wined3d.dll'));p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    value=inspect(a.module.read_bytes());names=['tools/inspect-drawing-wine-blit-command.py','tools/inspect-drawing-wine-imports.py'];value['sources']={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names};a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(value,indent=2)+'\n');print(json.dumps(value['dispatch']))


if __name__=='__main__':main()
