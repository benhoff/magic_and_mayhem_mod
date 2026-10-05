"""Import the Qt engine Preferences store into a disposable installation only."""
import hashlib
import json
import re
from pathlib import Path
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def valid(values):
    bounds=((0,15),(-2500,0),(0,1),(0,1),(0,2),(0,2),(0,1))
    return isinstance(values,list) and len(values)==7 and all(type(v) is int and lo<=v<=hi for v,(lo,hi) in zip(values,bounds))
def load(path):
    p=Path(path)
    if not p.exists():return {'revision':'missing','values':None,'warning':None}
    try:
        with p.open('rb') as stream:raw=stream.read(65537)
    except OSError as error:return {'revision':'unreadable','values':None,'warning':str(error)}
    if len(raw)>65536:return {'revision':'oversized','values':None,'warning':'Preference store is too large'}
    revision=hashlib.sha256(raw).hexdigest()
    try:
        if len(raw)>65536:raise ValueError('Preference store is too large')
        def pairs(items):
            out={}
            for key,value in items:
                if key in out:raise ValueError('Duplicate preference store key')
                out[key]=value
            return out
        data=json.loads(raw,object_pairs_hook=pairs)
        if not isinstance(data,dict) or type(data.get('schema')) is not int or data['schema']!=1 or data.get('source_sha256')!=BUILD or not valid(data.get('values')):raise ValueError('Unsupported or invalid preference store')
        return {'revision':revision,'values':data['values'],'warning':None}
    except (ValueError,TypeError,UnicodeError) as error:
        return {'revision':revision,'values':None,'warning':str(error)}
def overlay(text,values):
    if not valid(values):raise ValueError('Invalid persisted preferences')
    settings={'sound/musicvolume':str(values[0]),'sound/sfxvolume':str(values[1]),
        'video/ishighres':'TRUE' if values[2]==0 else 'FALSE','video/cutdownanims':'TRUE' if values[3] else 'FALSE',
        'video/dialogspeed':str(values[4]),'video/maxframespersec':str((20,17,14)[values[5]]),'video/windowsize':str(values[6])}
    seen={key:0 for key in settings};section='';lines=[]
    for line in text.splitlines(keepends=True):
        heading=re.match(rb'\s*\[([^]]+)\]',line)
        if heading:section=heading[1].decode('ascii').lower()
        match=re.match(rb'(\s*([^=;\r\n]+?)\s*=\s*)([^;\r\n]*?)([ \t]*(?:;[^\r\n]*)?)(\r?\n)?$',line)
        if match:
            key=section+'/'+match[2].decode('ascii').strip().lower()
            if key in settings:
                seen[key]+=1;line=match[1]+settings[key].encode('ascii')+match[4]+(match[5] or b'')
        lines.append(line)
    if any(n!=1 for n in seen.values()):raise ValueError('Missing or ambiguous staged Preferences keys')
    return b''.join(lines)
def stage(game,path,encoder,decoder):
    snapshot=load(path)
    if snapshot['values'] is None:return snapshot
    plain=game/'CFG/prefs.cfg';packed=game/'CFG/Encrypted/prefs.cfg'
    edits=[]
    for p in (plain,packed):
        if not p.exists():continue
        raw=p.read_bytes();text=decoder.decode(raw)[1] if p==packed else raw
        changed=overlay(text,snapshot['values']);out=encoder.encode(changed,decoder)[0] if p==packed else changed
        if p==packed and decoder.decode(out)[1]!=changed:raise ValueError('Preferences CFG round trip failed')
        edits.append((p,out))
    if not edits:raise ValueError('No staged preferences found')
    for p,out in edits:p.write_bytes(out)
    return snapshot
