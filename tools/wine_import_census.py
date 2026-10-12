"""Admission and final decoding for the bounded original import call census."""
import hashlib
import struct
import sys
import json
import re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import api_census_v1 as wire

def catalog():
    source=(ROOT/'runtime/api-census/nocd-imports.h').read_text()
    values=re.findall(r'\{0x([0-9a-f]+),("(?:[^"\\]|\\.)*"),("(?:[^"\\]|\\.)*")\}',source)
    if len(values)!=wire.ROW_COUNT:raise ValueError('Import catalog drift')
    return [(int(rva,16),json.loads(module),json.loads(symbol)) for rva,module,symbol in values]

def authorize(root,metadata,environment):
    environment.pop('MNM_WINE_IMPORT_CENSUS_FILE',None)
    enabled=metadata.get('wine_import_census',False)
    if type(enabled) is not bool:raise ValueError('Invalid import census policy')
    if not enabled:return
    path=root/'wine-import-census.bin';dll=root/'game/MnmApiCensus.dll'
    if metadata.get('wine_import_census_file')!=str(path) or path.exists() or path.is_symlink():raise ValueError('Stale or redirected import census')
    if dll.is_symlink() or hashlib.sha256(dll.read_bytes()).hexdigest()!=metadata.get('wine_import_census_dll_sha256'):raise ValueError('Import census DLL drift')
    with path.open('xb') as output:output.write(wire.initial_header());output.truncate(wire.SIZE)
    environment['MNM_WINE_IMPORT_CENSUS_FILE']='Z:'+str(path).replace('/','\\')

def decode(path,process_stopped):
    if not process_stopped:raise ValueError('Import census requires stopped observed process')
    if path.is_symlink():raise ValueError('Redirected import census')
    data=path.read_bytes()
    if not wire.valid_header(data,len(data)):raise ValueError('Invalid import census header')
    state,installed,refusal,count,stride=struct.unpack_from('<5I',data,16)
    if state!=wire.STATE_DETACHED or refusal or installed!=wire.ROW_COUNT or count!=wire.ROW_COUNT or stride!=wire.ROW_STRIDE:raise ValueError('Incomplete import census installation/detach')
    if any(data[36:64]):raise ValueError('Nonzero reserved census header')
    rows=[];rvas=set();expected=catalog()
    def text(row,at,size):
        value=row[at:at+size];end=value.find(0)
        if end<0 or any(value[end:]) or any(c<32 or c>126 for c in value[:end]):raise ValueError('Malformed import census label')
        return value[:end].decode('ascii')
    kinds={wire.TARGET_KIND_UNKNOWN:'unknown',wire.TARGET_KIND_WINE:'wine',wire.TARGET_KIND_NATIVE_SHIM:'native_shim',wire.TARGET_KIND_LEGACY_CODEC:'legacy_codec'}
    for i in range(count):
        row=data[64+i*stride:64+(i+1)*stride];rva,kind,low,high=struct.unpack_from('<4I',row)
        if rva in rvas or kind not in kinds or any(row[240:]):raise ValueError('Malformed import census row')
        rvas.add(rva)
        rows.append(dict(iat_rva=rva,target_kind=kinds[kind],calls=(high<<32)|low,target_module=text(row,16,64),import_module=text(row,80,32),symbol=text(row,112,128)))
        if (rva,rows[-1]['import_module'],rows[-1]['symbol'])!=expected[i]:raise ValueError('Original import identity drift')
    exercised=[row for row in rows if row['calls']]
    if any(row['target_kind']=='unknown' for row in exercised):raise ValueError('Exercised import has unknown target classification')
    wine=[row for row in exercised if row['target_kind']=='wine']
    return dict(schema=1,scope='Final original-executable190-import boundary only; excludes dynamic/COM/native-shim forwarding/transitive/prefix calls. Windows-target modules are hosted by Wine; not a whole Wine API count.',process_stopped=True,available_imports=count,exercised_imports=len(exercised),exercised_direct_wine_imports=len(wine),direct_wine_calls=sum(row['calls'] for row in wine),native_shim_imports=sum(row['target_kind']=='native_shim' for row in exercised),legacy_codec_imports=sum(row['target_kind']=='legacy_codec' for row in exercised),rows=rows,raw_sha256=hashlib.sha256(data).hexdigest())
