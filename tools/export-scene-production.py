#!/usr/bin/env python3
"""Export pinned original scene producer boundaries and helpers without patching."""
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES={
    'scene_outer':(0x4fd280,0x4fd77a),
    'scene_build':(0x4fcf50,0x4fd263),
    'creature_draw_prepare':(0x51e260,0x51e307),
    'creature_post_draw':(0x52ab80,0x52abb6),
    'camera_creature_adjust':(0x508a10,0x508af1),
    'rng':(0x54e200,0x54e22b),
    'creature_position_history':(0x51fa60,0x51fa9e),
    'scene_history_call':(0x4fa4db,0x4fa4f4),
}
ANCHORS={0x4fd280:'5355568bf1',0x4fd315:'8b46258bce',
         0x4fd5ad:'33db8b9629020000899e3102000089962d020000',0x4fd5c1:'a00c145e0084c0',0x4fd352:'e8f9fbffff',
         0x4fd554:'e8472d0000',0x4fa4ef:'e86c550200'}

def export():
    source=ROOT/'working/game-nocd/Chaos.exe';data=source.read_bytes()
    if hashlib.sha256(data).hexdigest()!=HASH:raise ValueError('Unknown scene producer executable')
    spec=importlib.util.spec_from_file_location('scene_offset',ROOT/'tools/export-menu-support.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    for va,value in ANCHORS.items():
        offset=module.image_offset(data,va,len(value)//2)
        if data[offset:offset+len(value)//2].hex()!=value:raise ValueError(f'Scene anchor mismatch {va:x}')
    disassembly=subprocess.check_output(['objdump','-d','-Mintel',str(source)],text=True)
    instructions=[];references=[]
    for line in disassembly.splitlines():
        match=re.match(r'\s*([0-9a-f]+):\s',line)
        if not match:continue
        va=int(match[1],16);instructions.append((va,line))
        if re.search(r'\bcall\s+0x4fd280\b',line):references.append(line)
    parent=ROOT/'working/decompiled';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='scene-production-',dir=parent));artifacts={}
    for name,(a,b) in RANGES.items():
        p=out/(name+'.asm');p.write_text('\n'.join(line for va,line in instructions if a<=va<b)+'\n')
        artifacts[p.name]=hashlib.sha256(p.read_bytes()).hexdigest()
    manifest=dict(source_sha256=HASH,ranges={n:[a,b] for n,(a,b) in RANGES.items()},
                  anchors={hex(a):v for a,v in ANCHORS.items()},direct_scene_calls=references,
                  artifacts=artifacts,scope='Static assembly only; opaque indirect flow/helper roles remain partial. No execution, patch or renderer/gameplay equivalence.')
    if hashlib.sha256(source.read_bytes()).hexdigest()!=HASH:raise ValueError('Scene executable changed')
    (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');return out

if __name__=='__main__':print(export())
