#!/usr/bin/env python3
"""Pin original API identities and the selected minimap camera switch."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SITES=(0x58bfe2,0x58c1d6,0x58c3f5,0x58c534,0x58c6f8,0x58c922,0x58cb17,0x58cc16,0x58cdd1,0x58cfa1)


def sha(data):return hashlib.sha256(data).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    declaration=json.loads(a.claims.read_text())
    if any(sha((ROOT/path).read_bytes())!=h for path,h in declaration['sources'].items()):p.error('Prospective source changed')
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        image=ROOT/'original/Arcane_Nocd/Chaos.exe';data=image.read_bytes()
        if sha(data)!=BUILD:raise ValueError('Unsupported executable')
        pe=struct.unpack_from('<I',data,60)[0];opt=pe+24
        n=struct.unpack_from('<H',data,pe+6)[0];s=struct.unpack_from('<H',data,pe+20)[0]
        sections=[struct.unpack_from('<III',data,opt+s+i*40+12) for i in range(n)]
        def read(address,size):
            for rva,length,raw in sections:
                offset=address-0x400000-rva
                if 0<=offset and offset+size<=length:return data[raw+offset:raw+offset+size]
            raise ValueError('Unbacked PE address')
        inventory=json.loads((ROOT/'research/runtime/coverage/binary-nocd.json').read_text())
        target=next(i for i in inventory['imports'] if i['iat_va']==0x5c51fc)
        if target['name']!='ClientToScreen' or target['dll'].lower()!='user32.dll':raise ValueError('Wrong imported API')
        if any(i.get('name')=='BitBlt' for i in inventory['imports']):raise ValueError('Re-review BitBlt imports')
        for site in SITES:
            if read(site,6)!=bytes.fromhex('ff15fc515c00'):raise ValueError('Unexpected retired CALL')
        table=struct.unpack('<4I',read(0x555e08,16))
        if table!=(0x555c4e,0x555c57,0x555c4e,0x555c60):raise ValueError('Camera switch changed')
        cases=[]
        for case,branch in enumerate(table):
            code=read(branch,7)
            if code[:3]!=bytes.fromhex('8bcbe8'):raise ValueError('Camera branch changed')
            callee=branch+7+struct.unpack_from('<i',code,3)[0]
            route={0x5527a0:'camera0',0x552b50:'camera1',0x552f20:'camera2'}.get(callee)
            if route is None:raise ValueError('Unreviewed camera branch')
            cases.append(dict(value=case,branch=branch,call=branch+2,target=callee,route=route))
        if read(0x555c3c,18)!=bytes.fromhex('8b86a400000083f8037720ff2485085e5500'):raise ValueError('Camera selector changed')
        report=dict(schema=1,success=True,kind='static',claims=declaration['claims'],sources=declaration['sources'],
                    original_executable_sha256=BUILD,binary_inventory_sha256=sha((ROOT/'research/runtime/coverage/binary-nocd.json').read_bytes()),
                    imported_api=target,retired_sites=list(SITES),bitblt_imports=0,
                    camera_switch=dict(selector=0x555c3c,table=0x555e08,cases=cases,
                                       code_sha256=sha(read(0x555c3c,43)),table_sha256=sha(read(0x555e08,16))),
                    unselected_body=0x5532f0,original_game_executed=False,
                    scope='Exact pinned imported API identity, ten CALL operands and four-entry minimap dispatch selection: camera0, camera2, camera0, camera1. Static scope only; no universal camera3 dead-code, indirect-flow, input orientation or pixel-equivalence claim.')
        # Account for every still-unexercised valid route in the prior campaign.
        # Byte candidates deliberately overapproximate instructions. Absence
        # cannot exclude computed addresses or callers outside this image.
        remaining={
            'pixel/rgb_add':0x545c10,'pixel/pcx_words':0x54b5f0,
            'pixel/brighten':0x58d770,'pixel/row_copy':0x58d3b0,
            'minimap/camera3':0x5532f0,'colour_rect/rectangle':0x547d80,
            **{f'raster/splice{i}':entry for indices,entry in [
                ((0,1),0x5947b2),((4,5),0x595b47),((6,7),0x59603e),
                ((8,),0x595677),((11,12),0x597086),((13,),0x596490),
                ((14,),0x5968a4),((15,),0x57e1b0),((16,),0x57ec90),
                ((17,),0x57f0f0),((18,),0x57f5f0),((19,),0x5806f0),
                ((20,),0x57e540),((22,),0x57de00)] for i in indices}}
        functions={f['entry']:f for f in inventory['functions']}
        relative={entry:[] for entry in remaining.values()}
        for i,(rva,length,raw) in enumerate(sections):
            flags=struct.unpack_from('<I',data,opt+s+i*40+36)[0]
            if not flags&0x20000000:continue
            code=data[raw:raw+length]
            for opcode in (b'\xe8',b'\xe9'):
                at=code.find(opcode)
                while at>=0:
                    if at+5<=len(code):
                        site=0x400000+rva+at
                        target=(site+5+struct.unpack_from('<i',code,at+1)[0])&0xffffffff
                        if target in relative:relative[target].append(dict(site=site,opcode=opcode.hex()))
                    at=code.find(opcode,at+1)
        reachability=[]
        for route,entry in remaining.items():
            pointer=struct.pack('<I',entry);pointers=[];at=data.find(pointer)
            while at>=0:
                for rva,length,raw in sections:
                    if raw<=at<raw+length:pointers.append(0x400000+rva+at-raw)
                at=data.find(pointer,at+1)
            reachability.append(dict(route=route,entry=entry,discovered_function=entry in functions,
                census_calls=[dict(site=f['site'],owner=f['owner']) for f in inventory['flows']
                              if f['kind']=='call' and not f['computed'] and entry in f['targets']],
                census_data_references=functions.get(entry,{}).get('data_references',[]),
                rel32_byte_candidates=relative[entry],absolute_pointer_byte_candidates=pointers,
                live_exercised=False,global_unreachable_proved=False))
        report['remaining_route_reachability']=reachability
        report['reachability_scope']='Static candidate inventory for24 valid routes absent from the prior bounded campaign. Byte hits are not automatically instructions or live callers. Zero candidates does not prove global dead code. No native comparison or live route promotion.'
        report['sources_stable']=all(sha((ROOT/path).read_bytes())==h for path,h in declaration['sources'].items())
        if not report['sources_stable'] or sha(image.read_bytes())!=BUILD:raise ValueError('Inputs changed')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)


if __name__=='__main__':main()
