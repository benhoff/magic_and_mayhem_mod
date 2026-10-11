#!/usr/bin/env python3
"""Losslessly retain an independently registered original publication capture."""
import argparse
import copy
import gzip
import json
import subprocess
from pathlib import Path
from drawing_corpus import ROOT,BUILD,local,sha
from drawing_publication_corpus import admit


def retain(evidence_id,binding_id,destination,source_root=ROOT):
    from rendering_coverage import evidence_record
    destination=destination.resolve();source_root=source_root.resolve()
    if destination.exists() or not destination.is_relative_to((ROOT/'tests/fixtures').resolve()):raise ValueError('Choose a new fixtures directory')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());ev=next(e for e in register['evidence'] if e['id']==evidence_id)
    original=evidence_record(ROOT,ev,{b['id']:b for b in register['builds']});bindings=evidence_record(ROOT,next(e for e in register['evidence'] if e['id']==binding_id),{b['id']:b for b in register['builds']})
    if ev['kind']!='isolated_original' or bindings['capture_report_sha256']!=ev['sha256']:raise ValueError('Independent original/binding identity mismatch')
    observation=original['drawing_publication'];capture=Path(original['experiment']).relative_to(source_root)/'capture';prepared={}
    def pack(data,historical):
        packed=gzip.compress(data,mtime=0);path=destination/(sha(data)+'.gz');prepared[path]=packed
        return {'path':path.relative_to(ROOT).as_posix(),'encoding':'gzip','raw_sha256':sha(data),'sha256':sha(packed),'historical_path':historical}
    def read(historical,digest):
        data=local(source_root,historical).read_bytes()
        if sha(data)!=digest:raise ValueError('Original publication artifact changed: '+historical)
        return data
    stream=(capture/'canvas-producers.bin').as_posix();raw=read(stream,observation['producers']['sha256']);size=32*1024*1024
    catalog={'schema':1,'build':ev['build'],'source_sha256':BUILD,'evidence':evidence_id,'historical_report_sha256':ev['sha256'],'asset_binding_evidence':binding_id,
             'scope':observation['scope'],'observation':copy.deepcopy(observation),'input_chunks':[pack(raw[i:i+size],stream) for i in range(0,len(raw),size)],
             'trace':pack(read((capture/observation['trace']).as_posix(),observation['trace_sha256']),(capture/observation['trace']).as_posix()),'assets':[],'frames':[],
             'retention_sources':{n:sha((ROOT/n).read_bytes()) for n in ('tools/retain-drawing-publication.py','tools/drawing_publication_corpus.py','tools/drawing_corpus.py')}}
    if observation.get('wire_version',1)==2:
        path=(capture/observation['applied_actions']['path']).as_posix();catalog['actions']=pack(read(path,observation['applied_actions']['sha256']),path)
    for name,historical in sorted(bindings['asset_sources'].items()):
        entry=pack(read(historical,bindings['inputs'][historical]),historical);entry['name']=name;catalog['assets'].append(entry)
    for frame in observation['frames']:
        historical=(capture/frame['path']).as_posix();catalog['frames'].append({'identity':copy.deepcopy(frame),'oracle':pack(read(historical,frame['sha256']),historical)})
    # Validate an independent temporary mirror before publishing retained bytes.
    import tempfile,shutil
    with tempfile.TemporaryDirectory(prefix='publication-retention-') as tmp:
        mirror=Path(tmp)
        for path,data in prepared.items():
            target=mirror/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
        from drawing_publication_corpus import RETENTION
        fake=copy.deepcopy(register);fake['evidence'].append({'id':RETENTION,'kind':'static','record':str((destination/'publication.json').relative_to(ROOT)),'sha256':sha((json.dumps(catalog,indent=2)+'\n').encode()),'scope':'Lossless independent original publication retention','assertions':[{'pointer':'/schema','equals':1}]})
        for e in fake['evidence']:
            if e['id'] in (evidence_id,binding_id):
                target=mirror/e['record'];target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/e['record'],target)
        target=mirror/fake['evidence'][-1]['record'];target.parent.mkdir(parents=True,exist_ok=True);target.write_text(json.dumps(catalog,indent=2)+'\n')
        admit(mirror,catalog,fake)
    destination.mkdir()
    for path,data in prepared.items():
        with path.open('xb') as f:f.write(data)
    with (destination/'publication.json').open('x') as f:json.dump(catalog,f,indent=2);f.write('\n')
    return catalog


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--evidence',required=True);p.add_argument('--bindings',required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--source-root',type=Path,default=ROOT);a=p.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:c=retain(a.evidence,a.bindings,a.output,a.source_root)
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    print(json.dumps({'frames':len(c['frames']),'chunks':len(c['input_chunks']),'assets':len(c['assets'])}))

if __name__=='__main__':main()
