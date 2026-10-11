"""Exact original raster global CALLs/stores; target/lifetime inference stays separate."""
import struct
from drawing_api_sites import call_mode
from drawing_census import owners

SLOTS={0x689b78,0x689b7c,0x6c4e3c,0x6903dc}


def boundaries(code, inventory):
    result=[]
    for site,row in sorted(code.items()):
        raw=bytes.fromhex(row['bytes'])
        if row['mnemonic']!='call':continue
        slot=struct.unpack_from('<I',raw,2)[0] if len(raw)==6 and raw[:2]==b'\xff\x15' else None
        callback=site==0x59d98a and raw==b'\xff\xd0'
        if slot not in SLOTS and not callback:continue
        mode=call_mode(row)
        if slot is not None and mode!={'kind':'absolute_memory','address':slot} or callback and mode!={'kind':'register','register':'eax'}:raise ValueError('CPU CALL bytes/operand disagree')
        result.append({'site':site,'return_pc':site+len(raw),'bytes':raw.hex(),'operand':row['operand'],'target_mode':mode,'selections':[{'anchor':None,'ranges':[[site,site+len(raw)]],'kind':'exact_cpu_global_call' if slot is not None else 'exact_exit_callback_call','scope':'One exact original CALL; no surrounding function/COM/target lifetime recovery.'}],'discovered_owners':owners(site,inventory),'exact_import':None,'semantic_contract':'unreviewed','remaining':'Immediate target needs before-CALL observation; static slot or inherited continuation does not establish actual callee/body/lifetime.'})
    return result


def assignments(code, inventory):
    """Only exact C7 05 absolute immediate writes, not all potential writers."""
    result=[]
    for site,row in sorted(code.items()):
        raw=bytes.fromhex(row['bytes'])
        if len(raw)!=10 or raw[:2]!=b'\xc7\x05':continue
        slot,target=struct.unpack_from('<2I',raw,2)
        if slot not in SLOTS:continue
        if row['mnemonic']!='mov' or row['operand'].replace(' ','').lower()!=f'dwordptrds:{hex(slot)},{hex(target)}':raise ValueError('CPU assignment bytes/operand disagree')
        result.append({'site':site,'bytes':raw.hex(),'slot':slot,'target':target,'discovered_owners':owners(site,inventory),'scope':'Exact absolute immediate assignment only; no executed branch, permanent binding or exclusion of indirect/alias writers.'})
    return result
