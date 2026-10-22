#!/usr/bin/env python3
"""Pinned original data and constant-address alternate-buffer writer candidates."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse,hashlib,json,re,struct,subprocess
from pathlib import Path
from drawing_census import ROOT,BUILD_SHA256,instructions,owners

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    if a.output.exists():p.error('Refusing overwrite')
    exe=ROOT/'original/Arcane_Nocd/Chaos.exe'
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        raw=evidence_bytes(exe);assert hashlib.sha256(raw).hexdigest()==BUILD_SHA256
        pe=struct.unpack_from('<I',raw,60)[0];size=struct.unpack_from('<H',raw,pe+20)[0];sections=[]
        for i in range(struct.unpack_from('<H',raw,pe+6)[0]):
            at=pe+24+size+40*i;virtual,rva,stored,offset=struct.unpack_from('<4I',raw,at+8)
            sections.append((rva,stored,offset,raw[at:at+8].split(b'\0')[0].decode()))
        def data(va,n):
            for rva,stored,offset,_ in sections:
                at=va-0x400000-rva
                if 0<=at<=stored-n:return raw[offset+at:offset+at+n]
            raise ValueError('Data not file backed')
        cycle=list(struct.unpack('<250I',data(0x5eb940,1000)))
        constants={hex(a):{'bytes':data(a,4).hex(),'float':struct.unpack('<f',data(a,4))[0]} for a in (0x5e1434,0x5e1438,0x5c5388,0x5c7490,0x5c5bb0)}
        references=[]
        for rva,stored,offset,name in sections:
            segment=raw[offset:offset+stored];start=0
            while True:
                at=segment.find(struct.pack('<I',0x6e1f68),start)
                if at<0:break
                references.append({'section':name,'address':0x400000+rva+at});start=at+1
        asm=subprocess.check_output(['objdump','-d','-Mintel','--insn-width=16',str(exe)],text=True)
        rows=instructions(asm);inventory=json.loads(evidence_text(ROOT/'research/runtime/coverage/binary-nocd.json'))
        candidates=[]
        for row in rows.values():
            args=row['operand'].split(',',1)
            if len(args)!=2 or row['mnemonic'] not in {'mov','add','sub','and','or','xor'}:continue
            target=args[0]
            if 'PTR' not in target:continue
            values=[int(x,16) for x in re.findall(r'0x([0-9a-f]+)',target)]
            if any(0x6d1f68<=v<=0x6e1f68 for v in values):
                candidates.append(dict(row,discovered_owners=owners(row['address'],inventory),
                    unresolved_index_or_indirect='[' in target and any(r in target for r in ('eax','ebx','ecx','edx','esi','edi','ebp'))))
        result={'schema':1,'success':True,'static_only':True,'original_sha256':BUILD_SHA256,
                'rng_next_index_table':cycle,'colour_plane_constants':constants,
                'alternate_address_byte_occurrences':references,'nearby_absolute_write_candidates':candidates,
                'indirect_writers_resolved':False,'live_alternate_route_proved':False,
                'sources':{n:hashlib.sha256(evidence_bytes(ROOT/n)).hexdigest() for n in
                    ['tools/inspect-original-ownership.py','tools/drawing_census.py','research/runtime/coverage/binary-nocd.json']}}
        a.output.write_text(json.dumps(result,indent=2)+'\n')
    finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    print(a.output)
if __name__=='__main__':main()
