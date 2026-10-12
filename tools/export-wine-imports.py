#!/usr/bin/env python3
"""Export the fixed import census for the unmodified supported NoCD executable."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def entries(data):
    if hashlib.sha256(data).hexdigest()!=HASH:raise ValueError('Unsupported original import image')
    spec=importlib.util.spec_from_file_location('import_layout',ROOT/'tools/prepare-native-draw.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    _,offset,descriptors=module.layout(data)
    def string(rva):
        start=offset(rva,1);end=data.index(0,start)
        if end-start>127:raise ValueError('Overlong import label')
        return data[start:end].decode('ascii')
    rows=[]
    for at in descriptors:
        lookup,_,_,name,iat=struct.unpack_from('<5I',data,at)
        dll=string(name)
        if len(dll)>=32:raise ValueError('Overlong module label')
        for i in range(2048):
            value=struct.unpack_from('<I',data,offset((lookup or iat)+4*i,4))[0]
            if not value:break
            rows.append(dict(rva=iat+4*i,module=dll,symbol='#'+str(value&65535) if value&0x80000000 else string(value+2)))
        else:raise ValueError('Unterminated import lookup')
    if len(rows)!=190 or len({x['rva'] for x in rows})!=190:raise ValueError('Pinned import census drift')
    return rows

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--executable',type=Path,required=True);parser.add_argument('--header',type=Path,required=True);parser.add_argument('--report',type=Path,required=True);args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        data=args.executable.read_bytes();rows=entries(data)
        header='#pragma once\n/* Generated fixed NoCD import metadata; no live target pointers. */\n#define MNM_CENSUS_IMPORT_COUNT 190u\nstruct CensusImport {unsigned rva;const char* module;const char* symbol;};\nstatic const struct CensusImport census_imports[MNM_CENSUS_IMPORT_COUNT]={\n'+''.join('    {0x%x,%s,%s},\n'%(x['rva'],json.dumps(x['module']),json.dumps(x['symbol'])) for x in rows)+'};\n'
        args.header.parent.mkdir(parents=True,exist_ok=True);args.header.write_text(header)
        assert hashlib.sha256(args.executable.read_bytes()).hexdigest()==HASH
        with args.report.open('x') as report:json.dump(dict(schema=1,scope='Static190 available import slots; not executed API count',original_sha256=HASH,rows=rows,header_sha256=hashlib.sha256(args.header.read_bytes()).hexdigest()),report,indent=2);report.write('\n')
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
