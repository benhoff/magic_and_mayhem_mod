#!/usr/bin/env python3
"""Read-only pinned installed PE32 public resource map imports."""
import argparse,importlib.util,json
from pathlib import Path
from drawing_api_session import sha
ROOT=Path(__file__).resolve().parents[1]
SELECTED={'wined3d_texture_get_resource':(0x6d6dc,0x219320),'wined3d_resource_map':(0x6d608,0x204490),'wined3d_resource_unmap':(0x6d618,0x204bc0)}


def inspect(ddraw,wined3d):
    spec=importlib.util.spec_from_file_location('resource_pe',ROOT/'tools/inspect-drawing-wine-imports.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
    if {'ddraw':sha(ddraw),'wined3d':sha(wined3d)}!=m.PINS:raise ValueError('Installed module hash drift')
    imports={r['name']:r for r in m.Pe32(ddraw).imports() if r['library'].lower()=='wined3d.dll'};exports={r['name']:r for r in m.Pe32(wined3d).exports()};rows=[]
    for name,(iat,target) in SELECTED.items():
        if name not in imports or name not in exports or imports[name]['iat_rva']!=iat or exports[name]['rva']!=target or exports[name]['forwarded']:raise ValueError('Pinned resource import/export drift')
        rows.append(dict(name=name,iat_rva=iat,export_rva=target))
    return dict(schema=1,success=True,module_sha256=m.PINS,imports=rows,source_abi_lead='Wine11.16publicwined3d.h cdecl declarations;runtimefixtureseparate',private_layout_read=False)


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    r=inspect(Path('/usr/lib/wine/i386-windows/ddraw.dll').read_bytes(),Path('/usr/lib/wine/i386-windows/wined3d.dll').read_bytes());r['sources']={n:sha((ROOT/n).read_bytes()) for n in ['tools/inspect-drawing-wine-resource-imports.py','tools/inspect-drawing-wine-imports.py']};a.report.write_text(json.dumps(r,indent=2)+'\n')
