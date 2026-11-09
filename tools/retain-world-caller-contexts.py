#!/usr/bin/env python3
"""Retain pinned outer wrapper bytes for stack/consumer recovery; static only."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from coverage_evidence import Reader,write_report
from original_x86 import Decoder,PE32

ROOT=Path(__file__).resolve().parents[1]
MAIN=(ROOT/subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--git-common-dir'],text=True).strip()).resolve().parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    target=ROOT/a.report
    if target.exists()or not target.resolve().is_relative_to(ROOT/'research/runtime'):p.error('Require a fresh research report')
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in
        ('tools/retain-world-caller-contexts.py','tools/original_x86.py','tools/original-manifest.sh')}
    result=dict(schema=1,kind='world_outer_context_static_inputs',success=False,original_sha256=HASH,sources=sources,
        original_game_runs=0,scope='Pinned complete wrapper bodies and instruction records for outer stack/consumer recovery. Linear decoding supplies inputs; it establishes no natural reachability, stack offset, caller-state or pixel equivalence.')
    decoder=None
    try:
        subprocess.run([MAIN/'tools/original-manifest.sh','verify'],check=True)
        raw=(MAIN/'original/Arcane_Nocd/Chaos.exe').read_bytes()
        if hashlib.sha256(raw).hexdigest()!=HASH:raise ValueError('Unsupported original executable')
        image=PE32(raw);register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        build=next(x for x in register['builds']if x['id']=='nocd-40209ca7')
        with Reader(ROOT)as rd:inventory=rd.json(build['inventory']);result['inventory']=dict(path=build['inventory'],sha256=rd.digest(build['inventory']))
        decoder=Decoder();functions=[]
        for entry in (0x54ab00,0x54aaa0,0x546540,0x54ab60,0x57e8e0):
            f=next(x for x in inventory['functions']if x['entry']==entry)
            body=b''.join(image.read(start,end-start)for start,end in f['ranges'])
            if hashlib.sha256(body).hexdigest()!=f['sha256']:raise ValueError('Inventory wrapper body mismatch')
            rows=[]
            for start,end in f['ranges']:
                at=start
                while at<end:
                    row=decoder.decode(image.read(at,min(15,end-at)),at)
                    if row is None or at+row['size']>end:raise ValueError('Incomplete wrapper decoding')
                    rows.append(row);at+=row['size']
            functions.append(dict(entry=entry,ranges=f['ranges'],sha256=f['sha256'],instructions=rows))
        result.update(functions=functions,decoder_version=decoder.version,source_preimages={},success=True)
        name=target.with_name(target.stem+'-source.bin')
        write_report(ROOT,name,Path(__file__).read_bytes());result['source_preimages']['tools/retain-world-caller-contexts.py']=str(name.relative_to(ROOT))
    except (ValueError,OSError,subprocess.SubprocessError)as error:result['error']=str(error)
    finally:
        if decoder:decoder.close()
        check=subprocess.run([MAIN/'tools/original-manifest.sh','verify'])
        result['original_manifest_after']=check.returncode==0
        result['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
        result['success']=result['success']and result['sources_stable']and result['original_manifest_after']
        write_report(ROOT,target,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result[k]for k in('success','error')if k in result}))
    return 0 if result['success']else 1


if __name__=='__main__':raise SystemExit(main())
