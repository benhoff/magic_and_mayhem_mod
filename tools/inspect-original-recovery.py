#!/usr/bin/env python3
"""Retain direct alternate-canvas references and known kind8 construction sites.

No absence-of-direct-writers result is a proof of absent indirect writers or
unreachable code. Original runtime pointers never become native input.
"""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

from drawing_census import BUILD_SHA256, ROOT, instructions, owners, direct_call

CLASSES={0x5c73f0:0x545dc0,0x5c7420:0x546590,0x5c7408:0x546230,
         0x5c7494:0x548560,0x5c74b0:0x548650,0x5c74cc:0x548ac0}


def reference_kind(row,address):
    literal=r'(?<![0-9a-f])0x'+format(address,'x')+r'(?![0-9a-f])'
    if not re.search(literal,row['operand']):return None
    args=row['operand'].split(',',1)
    if row['mnemonic']=='mov' and len(args)==2:
        if re.search(literal,args[0]) and ('PTR' in args[0] or 'ds:' in args[0]):return 'direct_store'
        if re.search(literal,args[1]) and ('PTR' in args[1] or 'ds:' in args[1]):return 'direct_load'
    return 'other_reference'


def inspect(executable):
    raw=evidence_bytes(executable)
    if hashlib.sha256(raw).hexdigest()!=BUILD_SHA256:raise ValueError('Unsupported original executable')
    pe=struct.unpack_from('<I',raw,60)[0];count=struct.unpack_from('<H',raw,pe+6)[0]
    optional=struct.unpack_from('<H',raw,pe+20)[0];sections=[]
    for i in range(count):
        at=pe+24+optional+40*i
        virtual,rva,stored,offset=struct.unpack_from('<4I',raw,at+8)
        sections.append({'name':raw[at:at+8].split(b'\0')[0].decode('ascii'),
                         'virtual_size':virtual,'rva':rva,'stored':stored,'offset':offset})
    def at(va,n):
        for s in sections:
            delta=va-0x400000-s['rva']
            if 0<=delta and delta+n<=s['stored']:
                return raw[s['offset']+delta:s['offset']+delta+n]
        raise ValueError('Original address is not file backed')
    inventory=json.loads(evidence_text(ROOT/'research/runtime/coverage/binary-nocd.json'))
    asm=subprocess.check_output(['objdump','-d','-Mintel','--insn-width=16',str(executable)],text=True)
    code=instructions(asm);alternate=[];classes=[]
    for row in code.values():
        kind=reference_kind(row,0x6e1f68)
        if kind:alternate.append(dict(row,reference_kind=kind,discovered_owners=owners(row['address'],inventory)))
    for table,constructor in CLASSES.items():
        window=subprocess.check_output(['objdump','-d','-Mintel','--insn-width=16',
            '--start-address='+hex(constructor),'--stop-address='+hex(constructor+0x200),
            str(executable)],text=True)
        rows=instructions(window);body=[]
        for row in rows.values():
            body.append(row)
            if row['mnemonic']=='ret':break
        if not body or body[-1]['mnemonic']!='ret':raise ValueError('Constructor lacks bounded return')
        classes.append({'class':table,'selected_constructor':constructor,
            'vtable_first_five_slots':list(struct.unpack('<5I',at(table,20))),
            'vtable_assignments':[dict(row,discovered_owners=owners(row['address'],inventory))
                for row in code.values() if reference_kind(row,table)],
            'direct_constructor_callers':[row for row in code.values() if direct_call(row)==constructor],
            'constructor_body':body,'static_constructor_sha256':hashlib.sha256(
                at(constructor,body[-1]['address']+len(bytes.fromhex(body[-1]['bytes']))-constructor)).hexdigest()})
    section=next(s for s in sections if s['rva']<=0x2e1f68<s['rva']+s['virtual_size'])
    delta=0x2e1f68-section['rva']
    initial=at(0x6e1f68,4) if delta+4<=section['stored'] else bytes(4)
    return {'schema':1,'success':True,'static_only':True,'source_executable_sha256':BUILD_SHA256,
        'alternate_canvas':{'address':0x6e1f68,'section':section,'initial_dword':int.from_bytes(initial,'little'),
            'references':alternate,'direct_loads':sum(x['reference_kind']=='direct_load' for x in alternate),
            'direct_stores':sum(x['reference_kind']=='direct_store' for x in alternate),
            'indirect_writers_resolved':False,'unreachable_proved':False},
        'known_classes':classes,'unknown_classes_retained':True,
        'scope':__doc__,'live_replacement':False}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    if a.output.exists():p.error('Refusing evidence overwrite')
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        result=inspect(a.executable)
        result['sources']={n:hashlib.sha256(evidence_bytes(ROOT/n)).hexdigest() for n in
            ['tools/inspect-original-recovery.py','tools/drawing_census.py','research/runtime/coverage/binary-nocd.json']}
        with a.output.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
    finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    print(a.output)


if __name__=='__main__':main()
