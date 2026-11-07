#!/usr/bin/env python3
"""Prove complete enabled-intro viewport output and preserve the accepted parent."""
import argparse
import hashlib
import importlib.util
import io
import json
import struct
from pathlib import Path
import sys
from PIL import Image
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from evidence_objects import configured
from rendering_acceptance import requirements,validate
from rendering_capture import capture
from rendering_intro_output import compare_paint
from rendering_menu_acceptance import artifact
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-enabled-intros-results-20261107/'
PARENT='research/runtime/rendering-identity-results-20261107/closeout-analysis-03.json'
BID='NP.enabled-intros-output'
LEAF='journey.enabled-movies.both-intros'
sha=lambda raw:hashlib.sha256(raw).hexdigest()


def module(name):
    s=importlib.util.spec_from_file_location('intro_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m


movie=module('close-rendering-movie-return')
identity=module('close-rendering-identity-gates')
retained=identity.retained


def presentation_coverage(presented,expected):
    require(set(presented)==set(expected)=={'intro0.avi','intro1.avi'},'Missing movie presentation domain')
    for name,values in presented.items():
        frames=expected[name]['frames']
        require(values and all(type(i) is int and 0<=i<len(frames) for i in values)
                and {4*i//len(frames) for i in values}=={0,1,2,3}
                and len({frames[i] for i in values})>1,'Missing changing presentation across the complete movie interval')


def post_movie_gui(row,body_size,submitted,expected):
    require(submitted==expected and row['native_gpu_frame'] is True and row['world']==row['sealed_world']=='0'
            and row['mode']==0 and row['presentation_owner']=='gui' and row['immutable_lease'] is False
            and row['transport']=='opengl' and row['gpu_read_complete'] is row['pixels_captured'] is True
            and type(row['commands']) is int and row['commands']>0 and type(row['surface']) is int and row['surface']>0
            and row['lease'].isdecimal() and int(row['lease'])>0
            and (row['frame_width'],row['frame_height']) in ((640,480),(800,600))
            and 0<row['width']<=4096 and 0<row['height']<=4096 and body_size==row['width']*row['height']*4,
            'Unbound GPU GUI handoff or GPU source before complete intro deliveries')


def viewport(rows, expected, references):
    """Every submitted image matches the independent sequence; every paint matches it."""
    ordered=[(name,i,h) for name in ('intro0.avi','intro1.avi') for i,h in enumerate(expected[name]['frames'])]
    require(sum(r['kind']=='image' and bool(r['width'] or r['height']) for r,b in rows)==len(ordered),'Incomplete submitted intro sequence')
    images={};paints={};submitted=[];presented={name:set() for name in expected};pixels=swaps=0;current=0;gui_paints=set();gui_swaps=0;gui=[]
    for row,body in rows[1:-1]:
        kind=row['kind']
        if kind=='image':
            current+=1;require(row['image']==str(current) and not body,'Image sequence/body differs')
            if row['width']==row['height']==0:
                require(row['rgba_sha256']==sha(b''),'Null image digest differs');images[current]=None;continue
            require(len(submitted)<len(ordered),'Extra movie image')
            name,index,h=ordered[len(submitted)];e=expected[name]['expected']
            require((row['width'],row['height'],row['rgba_sha256'])==(e['width'],e['height'],h),'Submitted movie image differs from independent frame')
            images[current]=(name,index);submitted.append((name,index))
        elif kind=='paint':
            require(row.get('image')==str(current),'Paint uses another submitted image')
            require(row['paint'] not in paints,'Duplicate paint identity')
            target=images.get(current)
            if row['native_gpu_frame']:
                post_movie_gui(row,len(body),len(submitted),len(ordered));target=None;gui_paints.add(row['paint'])
                gui.append({k:row[k] for k in ('paint','image','lease','commands','surface','mode','world','sealed_world','frame_width','frame_height','host_us')})
            elif target:
                require(len(body)>0,'Movie paint omitted output pixels')
                name,index=target;e=expected[name]['expected'];size=e['width']*e['height']*4
                raw=body() if callable(body) else body
                pixels+=compare_paint(row,raw,references[name][index*size:(index+1)*size],e['width'],e['height'])
            else:require(not body and row['frame_width']==row['frame_height']==0,'Unbound non-movie paint')
            paints[row['paint']]=target
        elif kind=='swap':
            require(row['paint'] in paints,'Swap lacks its actual paint')
            target=paints[row['paint']]
            if target:presented[target[0]].add(target[1]);swaps+=1
            elif row['paint'] in gui_paints:gui_swaps+=1
        else:raise ValueError('Unknown movie capture event: '+kind)
    require(len(submitted)==len(ordered) and all(len(v)>=2 for v in presented.values()) and swaps>0 and pixels>0,'Incomplete decoded/presented intro domain')
    presentation_coverage(presented,expected)
    return dict(verified=True,submitted_frames=len(submitted),movie_swaps=swaps,compared_pixels=pixels,mismatching_pixels=0,
                unique_presented_frames={n:len(v) for n,v in presented.items()},
                presented_frame_ordinals={n:sorted(v) for n,v in presented.items()},
                post_movie_gui_paints=gui,post_movie_gui_swaps=gui_swaps,post_movie_gui_pixel_credit=0,
                coalesced_or_unpresented_callbacks={n:len(expected[n]['frames'])-len(v) for n,v in presented.items()},
                scope='Every submitted RGBA image matches its ordered independent decoded reference. Every captured actual movie paint and swap is bound to that image and its whole OpenGL viewport, including letterbox and nearest sampling. Callback deliveries may coalesce; no one-swap-per-callback or physical scanout claim.')


def main_return(reader,a):
    callbacks=[[v['menu_id'],v['argument']] for v in retained.menu_events.decode(artifact(reader,a,'events.bin')) if v['event']==3]
    require(callbacks==[[3,3],[10,1],[3,2],[22,3],[3,4]],'Wrong Main/Preferences/Quick/Quit callbacks')
    require([(v['screen'],v['ack']) for v in a['states']]==[(3,0),(10,1),(3,2),(22,3),(3,4)],'Wrong ready Main return interval')
    require(a['states'][1]['preferences'][2]==int(not a['configuration']['high_resolution']),'Original High/Low context differs')
    for state in a['states']:
        raw=artifact(reader,a,state['wire_path']);words=struct.unpack_from('<40I',raw)
        require(2<=words[2]<=22 and len(raw)==words[3] and raw[:8]==(('MNMMCMD' if words[2]<10 else 'MNMMCM')+str(words[2])).encode()
                and list(words[32:40])==[int(state[k]) for k in ('sequence','generation','screen','ready','ack','status','thread','handoff')], 'Original Main return snapshot differs')
        if state['screen']==10:require(list(struct.unpack_from('<7i',raw,44724))==state['preferences'],'Original preference snapshot differs')
    shot=next(v for v in a['screenshots'] if v['label']=='main-ready-initial')
    nodes=[v for v in shot['nodes'] if v['role']=='button' and v['name']=='Quick Battle']
    require(shot['state_index']==0 and shot['host_focus']['host_focused'] and len(nodes)==1 and all(nodes[0][k] for k in ('focused','enabled','showing')),'Main focus/ready control missing')
    raw=artifact(reader,a,shot['path']);require(sha(raw)==shot['sha256'],'Changed Main screenshot')
    image=Image.open(io.BytesIO(raw)).convert('RGB');require(image.size==(1280,1024),'Wrong Main capture extent')
    x,y,w,h=nodes[0]['rect'];require(min(w,h)>0 and x>=0 and y>=0 and x+w<=1280 and y+h<=1024,'Main control outside capture')
    values=list(image.crop((x,y,x+w,y+h)).getdata());require(sum(max(p)>20 for p in values)>=len(values)*.9 and sum(min(p)>120 for p in values)>=100,'Main control not painted')
    first=next(v for v in a['actions'] if v.get('name')=='Preferences')
    require(first['host_ns']>shot['host_ns'] and first['physical']['down_up'] and first['physical']['host_focus']['host_focused'],'Main lacks subsequent physical input')
    return dict(verified=True,initial_main_sha256=shot['sha256'],callbacks=callbacks)


def case(reader, ref):
    a=retained.checked(reader,ref)
    require(a['kind']=='native_public_main_setup_menu_acquisition' and a['configuration']['skip_intros'] is False
            and a['original_game_runs']==1 and a['world_start_intended'] is False and not a['original_pixels_used_as_native_inputs'],'Unadmitted enabled movie case')
    require(all(reader.digest(n)==h for n,h in (a['sources']|a['compiler_sources']).items()),'Current player/collector source differs')
    e=reader.json(PREFIX+'independent-reference-01.json');require(e['success'] and e['original_manifest_verified_before_after'],'Independent reference incomplete')
    fixed,anchor=movie.references(reader)
    require(e['reference']==anchor and all(reader.digest(n)==h for n,h in e['sources'].items()),'Independent reference source/anchor differs')
    expected={v['name']:v for v in e['movies']};references={n:reader.read_bytes(v['reference']['record']) for n,v in expected.items()}
    require(set(expected)==set(fixed) and all(v['expected']==fixed[n] for n,v in expected.items()),'Independent reference expected data differs')
    for n,v in expected.items():
        size=v['expected']['width']*v['expected']['height']*4
        require(sha(references[n])==v['reference']['sha256']==v['expected']['rgba_sha256'] and len(references[n])==size*v['expected']['frames']
                and v['frames']==[sha(references[n][at:at+size]) for at in range(0,len(references[n]),size)],'Reference input differs')
    media=movie.completed_media(a,*movie.media_inputs(reader,a),{n:v['expected'] for n,v in expected.items()})
    streams=[v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('viewport-')]
    require(len(streams)==1,'Missing/ambiguous complete viewport capture')
    rows=capture(artifact(reader,a,streams[0]['path']))
    require(rows[0][0]['role']=='viewport' and rows[0][0]['session']==a['session_identity'],'Wrong viewport capture session')
    output=viewport(rows,expected,references)
    item=dict(reference=ref,profile='player-software' if a['configuration']['software_flag'] else 'player-default',rows=[LEAF],comparisons=[])
    lifetime=retained.retire_case(reader,item)
    original=next(v['source_sha256'] for v in reader.json('research/runtime/coverage/register.json')['builds'] if v['id']=='nocd-40209ca7')
    with configured(ROOT) as pair: provenance=identity.identify(reader,pair,item,original,set())
    lifetime['native_source_phase']='Exact new enabled-intro source closure, zero World; no whole-engine execution claim.'
    provenance['source_phase']='Exact new intro acquisition compiled/runtime sources; no other execution renewed.'
    return dict(success=True,case=ref,identity=a['session'],configuration=a['configuration'],output=output,media=media,
                main=main_return(reader,a),lifetime=lifetime,provenance=provenance,sources=a['sources'],original_source_sha256=original)


def close_rows(plan,parent,cases):
    validate(plan);template=requirements(plan)
    require(parent['passed_requirements']==sum(r['state']=='passed' for r in parent['requirements'])==74 and parent['dossier_identity']==fingerprint(template)
            and [(r['configuration'],r['id']) for r in parent['requirements']]==[(r['configuration'],r['id']) for r in template],'Changed74-row parent inventory')
    require(len(cases)==4 and len({c['identity'] for c in cases})==4 and {(c['configuration']['software_flag'],c['configuration']['high_resolution']) for c in cases}=={(s,h) for s in (False,True) for h in (False,True)}
            and all(type(c['configuration']['software_flag']) is type(c['configuration']['high_resolution']) is bool
                    and c['success'] is True and c['output']['verified'] is True and c['output']['mismatching_pixels']==0
                    and c['main']['verified'] is True and c['media']['verified'] is True for c in cases),'Missing/duplicate/failed intro domain')
    for c in cases:
        retained.check_retirement(c['lifetime']);identity.check_case(c['provenance'])
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id']==LEAF:
            require(row['state']=='pending','Both-intros already accepted')
            row.update(state='passed',admitted_execution_evidence=[dict(execution=c['case'],reason=c['output']['scope']) for c in cases if c['configuration']['software_flag'] is (row['configuration']=='player-software')])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==76,'Closeout exceeded two rows')
    return rows


def analysis_sources(reader):
    behavior=next(v for v in reader.json('research/runtime/coverage/register.json')['behaviors'] if v['id']==BID)
    return {n:reader.digest(n) for role in ('implementation','tests','validation_dependencies') for n in behavior[role]}


def check_single_proof(proof,sources):
    require(proof['success'] is True and proof['kind']=='complete_enabled_intros_output' and len(proof['cases'])==1
            and proof['original_game_runs']==1 and 'requirements' not in proof and proof['analysis_sources']==sources,
            'Changed or incomplete single-case proof')
    out=proof['cases'][0]['output']
    require(out['verified'] is True and out['mismatching_pixels']==proof['mismatching_pixels']==0
            and out['movie_swaps']==proof['movie_swaps']>0 and out['compared_pixels']==proof['compared_pixels']>0,'Unaccepted retained movie pixels')


def retained_proof(reader,ref):
    proof=retained.checked(reader,ref)
    check_single_proof(proof,analysis_sources(reader))
    c=proof['cases'][0];a=retained.checked(reader,c['case'])
    require(c['success'] is a['success'] is a['sources_stable'] is True and c['identity']==a['session'] and c['configuration']==a['configuration'] and c['sources']==a['sources']
            and c['lifetime']['reference']==c['provenance']['reference']==c['case']
            and c['provenance']['session']==a['session'] and c['provenance']['policy']['configuration']==a['configuration']
            and all(reader.digest(n)==h for n,h in c['sources'].items()),'Changed retained execution/source phase')
    expected,_=movie.references(reader)
    require(c['media']==movie.completed_media(a,*movie.media_inputs(reader,a),expected),'Changed retained media completion')
    require(c['main']==main_return(reader,a),'Changed retained Main return')
    retained.check_retirement(c['lifetime']);identity.check_case(c['provenance'])
    return c


def main():
    p=argparse.ArgumentParser(description=__doc__);group=p.add_mutually_exclusive_group(required=True);group.add_argument('--case',action='append');group.add_argument('--case-proof',action='append');p.add_argument('--report',required=True);p.add_argument('--pilot',action='store_true');args=p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(),'Fresh immutable owned result required')
    result=dict(schema=1,success=False)
    try:
        with Reader(ROOT) as r:
            behavior=next(v for v in r.json('research/runtime/coverage/register.json')['behaviors'] if v['id']==BID)
            sources=analysis_sources(r)
            inputs=[dict(record=n,sha256=r.digest(n)) for n in (args.case or args.case_proof)]
            cases=[(case if args.case else retained_proof)(r,ref) for ref in inputs]
            result=dict(schema=1,success=True,kind='complete_enabled_intros_output',cases=cases,analysis_sources=sources,
                        original_game_runs=len(cases),inputs=inputs,reused_current_case_proofs=bool(args.case_proof),movie_swaps=sum(c['output']['movie_swaps'] for c in cases),compared_pixels=sum(c['output']['compared_pixels'] for c in cases),mismatching_pixels=0)
            if not args.pilot:
                parent=r.json(PARENT);rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
                result.update(parent=dict(record=PARENT,sha256=r.digest(PARENT)),requirements=rows,decisions=parent['decisions'],dossier_identity=parent['dossier_identity'],
                              closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),passed_requirements=76,pending_requirements=80,newly_passed_requirements=2,
                              sources=dict(parent['sources'],**{n:h for n,h in sources.items() if n not in parent['sources']}),
                              scope=behavior['scope'],historical_failures_retired=False)
            require(sources=={n:r.digest(n) for n in sources},'Analyzer changed during execution')
    except Exception as error:result.update(success=False,error=str(error))
    h=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=h,movie_swaps=result.get('movie_swaps'),compared_pixels=result.get('compared_pixels'))),flush=True)
    return int(not result['success'])


if __name__=='__main__':raise SystemExit(main())
