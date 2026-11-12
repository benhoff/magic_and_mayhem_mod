#!/usr/bin/env python3
"""Attribute natural opaque/cropped copy destinations and presented pixels."""
import argparse
from collections import Counter
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import numpy as np
from coverage_evidence import Reader,write_report
from rendering_pilot import History,acquisition,commands,require,rgba
from rendering_menu_acceptance import artifact
from native_copy_routes import decode as decode_routes,validate as validate_routes
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-copy-results-20261106/'
BID='NP.opaque-cropped-copy-closeout'
STRIDE=2048*2048
LEAVES={'operation.copy-opaque','operation.image-cropping'}
SCOPE=('Finite valid opaque whole-source RGB565 copies in eight retained Quick/Campaign cases and cropped RGB565 copies in the four Campaign/storage cases, both explicit profiles and actual640/800. '
       'Closed original application route/authority intervals, live native source/destination incarnations, independent complete copy destinations and actual viewport lineage. '
       'Preserve46parent rows, refused caller failures and both decisions. No keyed/alias/attached-clipper/stretch route, original driver/ABI, arbitrary input/fault/provider or final completion claim.')


def module(name):
    spec=importlib.util.spec_from_file_location('copy_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


base=module('close-rendering-blend-direct')
sha=base.sha


def routes(reader,acquired):
    with tempfile.TemporaryDirectory(prefix='copy-routes-',dir='/tmp') as directory:
        experiment=Path(directory);capture=experiment/'lock-capture';capture.mkdir()
        for name in ('copy-routes-v2.bin','gpu-copy-census-v1.bin'):
            (capture/name).write_bytes(artifact(reader,acquired,name))
        result=validate_routes(experiment);header,rows=decode_routes(capture/'copy-routes-v2.bin')
    return header,rows,result


def command_join(raw,rows):
    copies={seq:struct.unpack('<10I',p) for op,seq,p in commands(raw) if op==3}
    bound={};refused=[]
    for row in rows:
        if row[13]&0x80000000:
            require(not row[40] and not row[41] and row[15]==0,'Refused application call emitted copy work')
            refused.append(list(row));continue
        first,last,source,target=row[40:44]
        if not first:continue
        for seq in range(first,last+1):
            if seq not in copies:continue
            w=copies[seq]
            require(seq not in bound and row[15] in (1,2) and (w[0],w[1])==(source,target)
                    and bool(w[8])==bool(row[12]&(1 if row[11] else 0x8000))
                    and not row[10]&(32|128),'Copy interval/handle/key/clip/alias binding differs')
            bound[seq]=row
    require(set(bound)==set(copies),'An actual copy has no unique successful application caller')
    return bound,refused


def words_hash(plane):return sha(plane.astype('<u4').tobytes())


def checked_destination(source,before,w,diagnostic):
    require(source.ndim==before.ndim==2 and len(w)==10 and w[8:]==(0,0) and w[0]!=w[1],'Selected copy is not ordinary opaque')
    l,t,r,b,x,y=w[2:8];height,width=source.shape;dh,dw=before.shape
    require(0<=l<r<=width and 0<=t<b<=height and 0<=x and 0<=y and x+r-l<=dw and y+b-t<=dh,'Copy geometry leaves owned storage')
    expected=before.copy();expected[y:y+b-t,x:x+r-l]=source[t:b,l:r]
    require(diagnostic['words']==list(w)+[0,0,0] and diagnostic['source_width']==width and diagnostic['source_height']==height
            and diagnostic['target_width']==dw and diagnostic['target_height']==dh and diagnostic['pixels']==dw*dh
            and diagnostic['mismatches']==0 and diagnostic['source_sha256']==words_hash(source)
            and diagnostic['before_sha256']==words_hash(before)
            and diagnostic['actual_sha256']==diagnostic['expected_sha256']==words_hash(expected),'Independent source/before/complete actual copy destination differs')
    return expected,int(np.count_nonzero(expected!=before))


def selected_kind(source,target,w):
    if w[8] or w[0]==w[1] or any(s['bits']!=16 or s['masks']!=(0xf800,0x7e0,31) for s in (source,target)):return None
    h,width=source['pixels'].shape
    return 'whole' if list(w[2:6])==[0,0,width,h] else 'crop'


def trace_copy(trace,model,w):
    l,t,r,b,x,y=w[2:8];source=model.handles[w[0]]['pixels'][t:b,l:r]
    mask=np.ones(source.shape,dtype=bool) if not w[8] else source!=w[9]
    prior=trace[w[0]][t:b,l:r].copy();target=trace[w[1]][y:y+b-t,x:x+r-l];target[mask]=prior[mask]


def presentation_sample(paint,body,parent,model,marks,proofs,found):
    ordinal=int(paint['paint']);generation=int(paint['world'])
    if ordinal not in parent:return
    accepted=parent[ordinal]
    require(accepted['world']==generation and accepted['lease']==int(paint['lease'])>0 and accepted['mismatching_pixels']==0
            and accepted['actual_sha256']==accepted['expected_sha256'],'Different accepted full composition occurrence')
    if paint['mode']==0:
        if model.display is None or model.display[0]!=paint['surface'] or model.presented_generation!=generation:return
        tags=marks.get('presented')
    else:
        if model.sealed is None or model.sealed['generation']!=int(paint['sealed_world']) or model.sealed['generation']!=generation:return
        tags=marks.get('sealed')
    if tags is None or tags.shape!=(paint['frame_height'],paint['frame_width']):return
    require(paint['scaling']==2 and paint['image_width']%paint['frame_width']==0 and paint['image_height']%paint['frame_height']==0,'Non-Integer sample geometry')
    scale=paint['image_width']//paint['frame_width'];require(scale>0 and scale==paint['image_height']//paint['frame_height'],'Sample scale differs')
    raw=None
    for seq in np.unique(tags//STRIDE):
        if seq<0 or int(seq) not in proofs:continue
        proof=proofs[int(seq)];kind=proof['kind']
        if proof['generation']!=generation or any(v['kind']==kind and v['generation']==generation for v in found):continue
        if len([v for v in found if v['kind']==kind])>=3:continue
        at=int(np.flatnonzero(tags//STRIDE==seq)[0]);y,x=divmod(at,paint['frame_width']);vx,vy=paint['left']+x*scale+scale//2,paint['top']+y*scale+scale//2
        pixels=model.presented if paint['mode']==0 else rgba(model.sealed['pixels'],model.sealed['bits'],model.sealed['masks'],model.sealed['palette'])
        expected=list(map(int,pixels[y,x]));require(any(expected[:3]),'Selected visible copy pixel is empty')
        if raw is None:
            raw=body() if callable(body) else body
            require(sha(raw)==accepted['actual_sha256'],'Actual presented body differs from accepted comparison')
        offset=4*(vy*paint['width']+vx);actual=list(raw[offset:offset+4]);require(actual==expected,'Actual copy viewport pixel differs')
        found.append(dict(kind=kind,generation=generation,paint=ordinal,lease=int(paint['lease']),command_sequence=int(paint['commands']),
                          viewport_pixel=[vx,vy],frame_pixel=[x,y],expected_rgba=expected,actual_rgba=actual,copy=proof,
                          copy_source_pixel=list(reversed(divmod(int(tags.flat[at])%STRIDE,proof['source_shape'][1]))),full_viewport_sha256=accepted['actual_sha256'],full_viewport_pixels=accepted['pixels']))


def output_proof(reader,case,acquired,compared,observed,header,rows,kinds=('whole','crop')):
    bound,refused=command_join(observed['commands'],rows)
    records=[json.loads(line) for line in artifact(reader,acquired,'copies.jsonl').splitlines()]
    diagnostic={v['sequence']:v for v in records}
    require(len(diagnostic)==len(records) and all(v['mismatches']==0 and v['actual_sha256']==v['expected_sha256'] for v in records),'Duplicate/failed original copy diagnostics')
    model=History(base.OriginalWorlds(reader,compared));trace={};created={};marks={};proofs={};found=[];authority={}
    paints={};accepted={v['paint']:v for v in compared['comparison']['comparisons']}
    live_paints={v['paint'] for v in case['proof']['presented_worlds'] if v['source_import_live']}
    for ordinal in observed['swaps']:
        p,body=observed['paints'][ordinal]
        if int(p['world'])>0 and ordinal in live_paints:paints.setdefault(p['commands'],[]).append((p,body))
    for op,seq,p in commands(observed['commands']):
        if op==1:
            w=struct.unpack_from('<7I',p);trace[w[0]]=np.full((w[2],w[1]),-1,dtype='int64');created[w[0]]=seq
        elif op in (2,20):
            sid,x,y,width,height=struct.unpack_from('<5I',p)
            target=trace[sid][y:y+height,x:x+width]
            if op==2:target[:]=-1
            else:
                n=width*height*(model.handles[sid]['bits']//8);mask=np.frombuffer(p[20+n:],dtype='u1').reshape(height,width).astype(bool);target[mask]=-1
        elif op==3:
            w=struct.unpack('<10I',p);source,target=model.handles[w[0]],model.handles[w[1]];kind=selected_kind(source,target,w)
            trace_copy(trace,model,w)
            if kind in kinds and model.generation>0 and seq in diagnostic and (len([v for v in found if v['kind']==kind])<3 or str(bound[seq][15]) not in authority):
                after,changed=checked_destination(source['pixels'],target['pixels'],w,diagnostic[seq])
                if changed:
                    row=bound[seq];require(header[15]<=row[1]<header[15]+header[16],'Selected caller lies outside original module')
                    require(row[15] in (1,2) and row[14] and row[13]==0,'Selected caller did not succeed')
                    proof=dict(kind=kind,generation=model.generation,sequence=seq,words=list(w),caller=list(row),
                               caller_rva=row[2],source_created=created[w[0]],target_created=created[w[1]],source_gpu_authority=bool(row[10]&4),route=row[15],
                               source_shape=list(source['pixels'].shape),target_shape=list(target['pixels'].shape),changed_pixels=changed,
                               preserved_exterior_pixels=int(after.size-(w[4]-w[2])*(w[5]-w[3])),
                               source_sha256=words_hash(source['pixels']),before_sha256=words_hash(target['pixels']),actual_sha256=diagnostic[seq]['actual_sha256'])
                    authority.setdefault(str(row[15]),proof);proofs[seq]=proof;l,t,r,b,x,y=w[2:8];region=trace[w[1]][y:y+b-t,x:x+r-l];nonempty=after[y:y+b-t,x:x+r-l]!=0;coords=np.arange(source['pixels'].size,dtype='int64').reshape(source['pixels'].shape)[t:b,l:r];region[nonempty]=(seq*STRIDE+coords)[nonempty]
        elif op==7:
            sid=struct.unpack('<I',p)[0];trace.pop(sid);created.pop(sid)
        elif op==11:
            a,b=struct.unpack('<2I',p);trace[a],trace[b]=trace[b],trace[a]
        elif op==19:
            for values in trace.values():values[:]=-1
            marks.clear()
        model.apply(op,seq,p)
        if op==6:
            sid=struct.unpack('<I',p)[0];marks['presented']=trace[sid].copy()
            if model.sealed is not None:marks['sealed']=trace[sid].copy()
        for paint,body in paints.get(seq,[]):presentation_sample(paint,body,accepted,model,marks,proofs,found)
        if all(len([v for v in found if v['kind']==k])==3 for k in kinds) and set(authority)=={'1','2'}:break
    check_witnesses(found,kinds)
    require(set(authority)=={'1','2'},'Missing actual CPU/GPU source authority destination proof')
    return dict(verified=True,witnesses=found,authority_witnesses=authority,bound_copy_commands=len(bound),refused_calls=refused,
                source_worlds=len(model.worlds),actual_source_world_generation=model.generation,
                retained_complete_compositions=case['proof']['world_presentations'],retained_complete_pixels=case['proof']['retained_pixels'])


def check_witnesses(rows,kinds=('whole','crop')):
    require(len(rows)==3*len(kinds) and {v['kind'] for v in rows}==set(kinds),'Missing whole/cropped output witnesses: '+str(Counter(v['kind'] for v in rows)))
    for kind in kinds:
        selected=[v for v in rows if v['kind']==kind]
        require(len(selected)==3 and len({v['generation'] for v in selected})==3 and all(v['generation']>0 for v in selected),'Missing distinct actual source generations')
        for v in selected:
            c=v['copy'];w=c['words'];h,width=c['source_shape']
            require(c['kind']==kind and c['generation']==v['generation'] and c['sequence']<=v['command_sequence']
                    and c['changed_pixels']>0 and c['caller'][13]==0 and c['caller'][40]<=c['sequence']<=c['caller'][41]
                    and w[8:]==[0,0] and w[0]!=w[1] and v['actual_rgba']==v['expected_rgba']
                    and any(v['actual_rgba'][:3]) and v['lease']>0,'Invalid actual caller/copy/presentation witness')
            require((w[2:6]==[0,0,width,h])==(kind=='whole'),'Whole/cropped source geometry differs')
            x,y=v['copy_source_pixel'];require(w[2]<=x<w[4] and w[3]<=y<w[5],'Visible copy source coordinate leaves selected source rectangle')


def close_rows(plan,parent,runs):
    validate(plan);template=requirements(plan)
    require(parent['dossier_identity']==fingerprint(template) and parent['passed_requirements']==46
            and [(r['configuration'],r['id']) for r in parent['requirements']]==[(r['configuration'],r['id']) for r in template],'Accepted46-row parent differs')
    expected={(p,s,tuple(z)) for p in ('player-default','player-software') for s in ('normal-quick','normal-storage') for z in ([640,480],[800,600])}
    require(len(runs)==8 and {(r['domain'][0],r['scenario'],tuple(r['domain'][1])) for r in runs}==expected and len({r['identity'] for r in runs})==8,'Missing/duplicate caller/profile/size domain')
    for r in runs:require(r['proof']['verified'] and set(r['proof']['authority_witnesses'])=={'1','2'},'Unproved caller/source authority domain');check_witnesses(r['proof']['witnesses'],('whole','crop') if r['scenario']=='normal-storage' else ('whole',))
    result=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id'] in LEAVES:
            require(row['state']=='pending','Selected copy row already passed')
            row.update(state='passed',admitted_execution_evidence=[dict(execution=r['pair'],actual_world_extents=[r['domain'][1]],provider=r['provider'],reason=SCOPE) for r in runs if r['domain'][0]==row['configuration']])
        result.append(row)
    require(sum(v['state']=='passed' for v in result)==50,'Copy closeout widened')
    return result


def analyze(reader):
    binding,impact=module('close-rendering-world-exit').parent_plan(ROOT,reader,'BD06.blend-direct-closed01','NP.blend-direct-closeout')
    ref=binding['inputs']['closeout-analysis-02.json'];parent=reader.json(ref['record'])
    require(reader.digest(ref['record'])==ref['sha256'] and parent['success'] and all(reader.digest(n)==h for n,h in binding['sources'].items()),'Changed current parent')
    older=reader.json(parent['input_parent']['record']);runs=[]
    for case in older['executions']:
        pair=case['pair'];a,c=[reader.json(pair[k]['record']) for k in ('acquisition','comparison')]
        require(all(reader.digest(pair[k]['record'])==pair[k]['sha256'] for k in pair) and c['acquisition']==pair['acquisition'] and a['success'] is c['success'] is True,'Changed actual caller execution')
        original=c['original_reference'];require(original['continuous_from_owned_initial_history'] is True and original['native_player_pixels_read'] is original['original_destination_capture_read'] is False and c['comparison']['success'] is True and c['comparison']['mismatching_pixels']==0,'Dependent or failed accepted original history')
        streams=[artifact(reader,a,Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
        observed=acquisition(streams,a['session_identity']);require(sha(observed['commands'])==c['captured_command_stream_sha256'],'Changed compared native commands')
        header,rows,routing=routes(reader,a);proof=output_proof(reader,case,a,c,observed,header,rows,('whole','crop') if case['scenario']=='normal-storage' else ('whole',))
        runs.append(dict(domain=case['domain'],scenario=case['scenario'],identity=case['identity'],provider=case['provider'],pair=pair,routing=routing,proof=proof))
        print('Verified caller-bound whole/cropped native output',case['domain'],case['scenario'],[(v['kind'],v['generation']) for v in proof['witnesses']],flush=True)
    rows=close_rows(reader.json('docs/native-rendering-closure.json'),parent,runs)
    reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(v for v in reg['behaviors'] if v['id']==BID);sources=dict(binding['sources'])
    sources.update({n:reader.digest(n) for n in b['implementation']+b['tests']+b['validation_dependencies']})
    require(all(reader.digest(n)==h for n,h in sources.items()),'Changed current native or analysis source closure')
    return dict(schema=1,kind='retained_caller_bound_opaque_cropped_copy_closeout',success=True,scope=SCOPE,sources=sources,
                accepted_parent=dict(binding=impact['retained_input'],applicability=impact),input_parent=ref,executions=runs,requirements=rows,
                dossier_identity=parent['dossier_identity'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|LEAVES),decisions=parent['decisions'],
                passed_requirements=50,pending_requirements=106,inherited_current_rows=46,newly_passed_requirements=4,
                new_retained_viewport_samples=sum(len(v['proof']['witnesses']) for v in runs),
                new_complete_copy_destinations=sum(len(v['proof']['witnesses']) for v in runs),
                retained_complete_compositions=sum(v['proof']['retained_complete_compositions'] for v in runs),retained_complete_pixels=sum(v['proof']['retained_complete_pixels'] for v in runs),
                original_game_runs=0,new_original_reference_runs=0,new_native_code=False,new_game_fixture=False,historical_failures_retired=False)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(),'Fresh owned immutable report required')
    result=dict(schema=1,kind='retained_caller_bound_opaque_cropped_copy_closeout',success=False)
    try:
        with Reader(ROOT) as reader:result=analyze(reader)
    except Exception as error:result['error']=str(error)
    digest=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=digest)),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
