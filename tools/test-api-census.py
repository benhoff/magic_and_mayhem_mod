#!/usr/bin/env python3
"""Isolated PE32 import counter ABI/overflow and final decoder tests."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
from wine_import_census import ROOT,catalog,decode,wire
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=v for p,v in declaration['sources'].items()):raise ValueError('Census source declaration drift')
    parent=ROOT/'working/tests/api-census';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    spec=importlib.util.spec_from_file_location('census_build',ROOT/'tools/build-api-census.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    dll=module.build(out/'build',True);shutil.copy2(dll,out/dll.name);shutil.copy2(dll.parent/'selftest.exe',out/'selftest.exe')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'prefix'),WINEDEBUG='-all')
    with (out/'wineboot.log').open('x') as log:subprocess.run(['wineboot','-u'],env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
    try:
        with (out/'wine.log').open('x') as log:subprocess.run(['wine',str(out/'selftest.exe')],env=env,cwd=out,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
    finally:subprocess.run(['wineserver','-k'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10)
    data=bytearray(wire.SIZE);data[:64]=wire.initial_header();struct.pack_into('<5I',data,16,wire.STATE_DETACHED,wire.ROW_COUNT,0,wire.ROW_COUNT,wire.ROW_STRIDE)
    for i,(rva,module_name,symbol) in enumerate(catalog()):
        at=64+i*wire.ROW_STRIDE;struct.pack_into('<4I',data,at,rva,wire.TARGET_KIND_WINE,3 if i==0 else 0,2 if i==0 else 0)
        for offset,label in [(16,b'kernelbase.dll'),(80,module_name.encode()),(112,symbol.encode())]:data[at+offset:at+offset+len(label)]=label
    path=out/'decoder.bin';path.write_bytes(data);decoded=decode(path,True)
    assert decoded['exercised_direct_wine_imports']==1 and decoded['direct_wine_calls']==(2<<32)+3
    refused=0
    for offset,value in [(8,2),(16,1),(20,0),(24,1),(28,1),(32,128),(36,1),(64+4,99),(64+240,1)]:
        changed=bytearray(data);struct.pack_into('<I',changed,offset,value);path.write_bytes(changed)
        try:decode(path,True)
        except ValueError:refused+=1
        else:raise AssertionError('Malformed census accepted')
    path.write_bytes(data)
    try:decode(path,False)
    except ValueError:refused+=1
    else:raise AssertionError('Active process accepted')
    report=dict(schema=1,success=True,origin='synthetic_pe_counter_and_decoder',claims=declaration['claims'],sources=declaration['sources'],sources_stable=all(sha(ROOT/p)==v for p,v in declaration['sources'].items()),checks=['stdcall_arguments_return_last_error','cdecl_arguments_return','fastcall_register_stack_arguments_return','arithmetic_flags_ecx_edx_forwarding','low_overflow_counter','10000_repeat_calls','quiescent_decoder_u64','malformed_or_active_refused'],refusals=refused,dll_sha256=sha(dll))
    assert report['sources_stable'] and refused==10
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Import census checks passed',out/'report.json')
if __name__=='__main__':main()
