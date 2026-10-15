#!/usr/bin/env python3
"""Read-only installed PE32 public-copy import/export/prologue pin."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    spec=importlib.util.spec_from_file_location('pe',ROOT/'tools/inspect-drawing-wine-imports.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    blobs={k:Path('/usr/lib/wine/i386-windows',k+'.dll').read_bytes() for k in module.PINS}
    hashes={k:hashlib.sha256(v).hexdigest() for k,v in blobs.items()}
    if hashes!=module.PINS:raise ValueError('Installed module hash changed')
    name='wined3d_device_context_copy_sub_resource_region';imports=[r for r in module.Pe32(blobs['ddraw']).imports() if r['name']==name];pe=module.Pe32(blobs['wined3d']);exports=[r for r in pe.exports() if r['name']==name]
    if len(imports)!=1 or len(exports)!=1 or imports[0]['iat_rva']!=0x6d598 or exports[0]['rva']!=0x1df0d0 or exports[0]['forwarded']:raise ValueError('Copy boundary differs')
    prefix=blobs['wined3d'][pe.offset(exports[0]['rva']):pe.offset(exports[0]['rva'])+16]
    if prefix.hex()!='5589e557565383e4f881ec900000008b':raise ValueError('Copy prologue changed')
    paths=['tools/inspect-drawing-wine-resource-copy.py','tools/inspect-drawing-wine-imports.py']
    a.report.write_text(json.dumps(dict(schema=1,success=True,sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in paths},module_sha256=hashes,import_entry=imports[0],export_entry=exports[0],entry16_hex=prefix.hex(),public_abi_source='https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/include/wine/wined3d.h',calling_convention='cdecl',arguments=10,return_type='HRESULT',original_game_executed=False,scope='Installed PE32 module/import/export/prologue pins; public header is ABI source lead, not loaded behavior/transfer/ownership proof.'),indent=2)+'\n')


if __name__=='__main__':main()
