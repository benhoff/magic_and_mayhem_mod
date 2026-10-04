#!/usr/bin/env python3
"""Hash-pinned ANI loader/controller disassembly and installed version-5 inventory."""
import collections
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES={'ani_loader':(0x4644d0,0x464ab4),'sequence_count':(0x464b20,0x464b27),
        'sequence_first':(0x464b30,0x464b49),'sequence_empty':(0x464ba0,0x464bc5),
        'first_sprite':(0x464bd0,0x464c08),'controller_construct':(0x464c50,0x464c78),
        'controller_start':(0x464cb0,0x464d64),'controller_direction_switch':(0x464e20,0x464ebd),
        'controller_tick':(0x464ec0,0x464fdc),'controller_initial_controls':(0x465000,0x4650a8),
        'world_animation_calls':(0x46b72c,0x46b79f)}

def sha(b):return hashlib.sha256(b).hexdigest()
def main():
    parent=REPO/'working/decompiled';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='animation-support-',dir=parent));print(f'Animation evidence: {out}',flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);print(p.stdout,end='',flush=True);p.check_returncode()
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe';binary=exe.read_bytes()
        if sha(binary)!=HASH:raise ValueError('Unsupported executable')
        asm=subprocess.check_output(['objdump','-d','-Mintel',str(exe)],text=True)
        rows=[]
        for line in asm.splitlines():
            m=re.match(r'\s*([0-9a-f]+):\s',line)
            if m:rows.append((int(m[1],16),line))
        callers={}
        for name,(start,end) in RANGES.items():
            (out/(name+'.asm')).write_text('\n'.join(line for va,line in rows if start<=va<end)+'\n')
            callers[name]=[{'va':hex(va),'context':[s for _,s in rows[max(0,i-8):i+3]]}
                           for i,(va,line) in enumerate(rows) if re.search(r'\bcall\s+0x'+f'{start:x}'+r'\b',line)]
        (out/'callers.json').write_text(json.dumps(callers,indent=2)+'\n')
        files=[];snapshots={};root=REPO/'working/game-clean'
        for path in sorted(root.rglob('*')):
            if path.suffix.lower()!='.ani' or not path.is_file():continue
            b=path.read_bytes();snapshots[path]=sha(b)
            if len(b)<44 or b[:4]!=b'ANI\0':raise ValueError(f'Invalid ANI header: {path}')
            size,count,version,opaque,offsets=struct.unpack_from('<5I',b,4)
            item={'path':str(path.relative_to(root)),'sha256':sha(b),'source_bytes':len(b),'declared_bytes':size,
                  'declared_records':count,'version':version,'opaque_header':opaque,'offset_count':offsets,
                  'sprite_name_hex':b[24:44].hex()}
            if version==5:
                if offsets<2 or 44+4*offsets>len(b):raise ValueError('Offset count invalid')
                starts=list(struct.unpack_from('<'+str(offsets)+'I',b,44));base=44+4*offsets
                if (len(b)-base)%44:raise ValueError('Partial ANI record')
                records=[struct.unpack_from('<11i',b,pos) for pos in range(base,len(b),44)]
                if size!=len(b) or starts[0]!=0 or starts[-1]!=len(records) or starts!=sorted(starts):raise ValueError('ANI extent/table differs')
                if any(a==z or records[z-1][0]!=6 for a,z in zip(starts,starts[1:])):raise ValueError('Missing terminal records')
                item.update(record_count=len(records),sequence_count=offsets-1,types=dict(collections.Counter(r[0] for r in records)),
                            header_records_match=count==len(records),empty_sequences=sum(records[a][0]==6 for a in starts[:-1]))
                if path.name.lower()=='redcap.ani' and path.parent==root/'Creatures':
                    (out/'redcap-sequences.json').write_text(json.dumps({'starts':starts,'first_sequences':[[list(r) for r in records[a:z]] for a,z in zip(starts[:8],starts[1:9])]},indent=2)+'\n')
            files.append(item)
        if sha(exe.read_bytes())!=HASH or any(sha(path.read_bytes())!=digest for path,digest in snapshots.items()):raise ValueError('Input changed')
        artifacts={p.name:sha(p.read_bytes()) for p in out.iterdir() if p.suffix in ('.asm','.json')}
        report={'executable_sha256':HASH,'ranges':RANGES,'scope':'Static selected contracts and installed inventory, no live observation',
                'input_unchanged':True,'source_sha256':sha(Path(__file__).read_bytes()),'artifacts_sha256':artifacts,
                'summary':{'files':len(files),'versions':dict(collections.Counter(f['version'] for f in files)),
                           'records':sum(f.get('record_count',0) for f in files)},'files':files}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report['summary']),flush=True)
    finally:verify('after')
if __name__=='__main__':main()
