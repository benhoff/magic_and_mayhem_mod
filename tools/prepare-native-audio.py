#!/usr/bin/env python3
"""Guard and redirect the pinned DirectSound ordinal import in a disposable PE."""
import hashlib
import importlib.util
from pathlib import Path
import struct

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
DESCRIPTOR=bytes.fromhex('48611d00000000000000000046661d001c501c00')
LOOKUP=bytes.fromhex('0100008000000000')

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value

def redirect(original,staged=None):
    if hashlib.sha256(original).hexdigest()!=HASH:raise ValueError('Unsupported original audio executable')
    data=original if staged is None else staged
    layout=module('audio_pe_layout','tools/prepare-native-draw.py').layout
    _,offset,descriptors=layout(data)
    matches=[at for at in descriptors if data[at:at+20]==DESCRIPTOR]
    if len(matches)!=1:raise ValueError('Expected pinned DirectSound descriptor')
    lookup,_,_,name,iat=struct.unpack('<5I',DESCRIPTOR)
    if data[offset(lookup,8):offset(lookup,8)+8]!=LOOKUP:raise ValueError('Expected DirectSound ordinal1 lookup')
    if data[offset(name,11):offset(name,11)+11]!=b'DSOUND.dll\0':raise ValueError('Expected original audio DLL name')
    thunk=offset(0x197566,6)
    if data[thunk:thunk+6]!=bytes.fromhex('ff251c505c00'):raise ValueError('Expected original audio thunk')
    result=bytearray(module('audio_native_import','tools/prepare-shadow-experiment.py').add_import(
        data,'MnmAudio.dll','AudioAnchor',b'.mnmaud'))
    _,new_offset,new_descriptors=layout(result)
    new_name=struct.unpack_from('<I',result,new_descriptors[-1]+12)[0]
    if result[new_offset(new_name,13):new_offset(new_name,13)+13]!=b'MnmAudio.dll\0':raise ValueError('Invalid native audio import name')
    target=next(at for at in new_descriptors if result[at:at+20]==DESCRIPTOR)
    struct.pack_into('<I',result,target+12,new_name)
    if struct.unpack_from('<I',result,target+16)[0]!=iat or result[thunk:thunk+6]!=data[thunk:thunk+6]:
        raise ValueError('Original audio IAT/thunk moved')
    return bytes(result)
