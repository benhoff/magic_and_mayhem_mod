#!/usr/bin/env python3
"""Recheck a real UI proof and reject semantic mutations behind container hashes."""
import argparse
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import struct
from unittest.mock import patch
from coverage_evidence import Reader,write_report
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition,require
from rendering_ui_acceptance import PREFIX
import rendering_ui_proof as proof
ROOT=Path(__file__).resolve().parents[1]


def words(raw,stride,index,mutate):
    raw=bytearray(raw)
    for at in range(16,len(raw),stride):
        w=list(struct.unpack_from('<'+str(stride//4)+'I',raw,at))
        if index(w):mutate(w);struct.pack_into('<'+str(len(w))+'I',raw,at,*w)
    return bytes(raw)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--comparison',required=True);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh guard result required')
    names=['tools/test-rendering-ui-proof.py','tools/rendering_ui_proof.py','tools/rendering_ui_acceptance.py','tools/rendering_pilot.py','tools/rendering_capture.py','tools/rendering_menu_acceptance.py']
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names};result=dict(schema=1,kind='actual_ui_proof_guards',success=False,sources=sources,original_game_runs=0)
    try:
        with Reader(ROOT) as r:
            q=r.json(args.comparison);require(q['success'] and q['ui_proof']['verified'],'Accepted real comparison required')
            a=r.json(q['acquisition']['record']);observed=acquisition([artifact(r,a,Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')],a['session_identity'])
            baseline=proof.assess(r,a,q,observed);require(baseline==q['ui_proof'],'Real baseline UI proof differs')
            guards=[]
            def check(label, *, wire=None,mutator=None,case=None,compared=None,view=None):
                def read(reader,acquired,name):
                    raw=artifact(reader,acquired,name)
                    return mutator(raw) if name==wire else raw
                with patch.object(proof,'artifact',read):
                    try:proof.assess(r,case or a,compared or q,view or observed)
                    except (ValueError,KeyError,StopIteration) as e:guards.append(dict(name=label,rejected=True,reason=str(e)));return
                raise ValueError('Adverse real UI input accepted: '+label)
            def ui(label,predicate,index,value):check(label,wire='ui-acceptance.bin',mutator=lambda b:words(b,512,predicate,lambda w:w.__setitem__(index,value(w) if callable(value) else value)))
            bad=deepcopy(q);bad['comparison']['mismatching_pixels']=1;check('mismatching-output',compared=bad)
            ui('wrong-tooltip-actor-health',lambda w:w[1]==2,42,lambda w:w[42]+1)
            ui('wrong-tooltip-text',lambda w:w[1]==2,64,lambda w:(w[64]&0xffffff00)|ord('J'))
            ui('missing-visible-tooltip',lambda w:w[1]==1,20,0)
            ui('missing-background-restoration',lambda w:w[1]==1,20,1)
            ui('missing-original-pause',lambda w:w[1]==1,10,0)
            ui('advancing-paused-counter',lambda w:w[1]==1 and w[10]==1,11,lambda w:w[11]+w[0])
            ui('stopped-paused-render-sequence',lambda w:w[1]==1 and w[10]==1,7,0)
            ui('foreign-actor-lineage',lambda w:w[1]==1 and w[0]>100,37,lambda w:w[37]+4)
            for label,name,offset,value in [('active-mini-input','mini-paused-begin-render-input.bin',20,1),('pending-mini-media','mini-paused-end-native-media.bin',516,2),('foreign-mini-source','mini-paused-end-world-continuous.bin',24,0)]:
                def mutate(raw,offset=offset,value=value):
                    b=bytearray(raw);struct.pack_into('<I',b,offset,value);return bytes(b)
                check(label,wire=name,mutator=mutate)
            check('missing-original-mini-cancel',wire='events.bin',mutator=lambda b:words(b,64,lambda w:w[1]==3 and w[3]==17 and w[9]==4,lambda w:w.__setitem__(9,3)))
            check('foreign-original-mini-owner',wire='events.bin',mutator=lambda b:words(b,64,lambda w:w[1]==3 and w[3]==17 and w[9]==4,lambda w:w.__setitem__(2,w[2]+1)))
            for field,value in [('symbol',33),('down_up',False),('held_ms',0)]:
                bad=deepcopy(a);next(v for v in bad['actions'] if v['action']=='space-pause')[field]=value;check('invalid-physical-space-'+field,case=bad)
            paint=next(i for i in observed['swaps'] if int(observed['paints'][i][0]['world'])>0)
            bad=dict(observed);bad['paints']=dict(observed['paints']);metadata,body=observed['paints'][paint];bad['paints'][paint]=(dict(metadata,native_gpu_frame=False),body);check('unadmitted-native-paint',view=bad)
            require(all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items()),'Guard source drift')
            result.update(success=True,comparison=dict(record=args.comparison,sha256=r.digest(args.comparison)),baseline_verified=True,guards=guards,guard_count=len(guards),scope='Real retained case, exact baseline and semantic corruptions behind container hash admission; original files, raw captures and native pixels remain unchanged. No new game or raster execution.')
    except Exception as e:result.update(success=False,error=str(e))
    print(write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))
    return int(not result['success'])


if __name__=='__main__':raise SystemExit(main())
