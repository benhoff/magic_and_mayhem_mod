"""Original hotspot state and independently reconstructed complete COPY outputs."""
from collections import Counter
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import numpy as np
from cursor_hotspot import ROOT,REQUIRED_CLASSES,acceptance_points as points,descriptors,bitmap_words,original_geometry
from rendering_pilot import History,commands,require,rgba
from rendering_interaction import decode
from rendering_menu_acceptance import artifact
from rendering_interaction_assessment import presented_interval,cursor_after_action


def digest(plane):return hashlib.sha256(plane.astype('<u4').tobytes()).hexdigest()


def check_copy(record,words,source,before,after):
    require(record['words']==words+[0,0,0] and record['mismatches']==0
        and [record[k] for k in ('source_width','source_height','target_width','target_height')]
            ==[source.shape[1],source.shape[0],before.shape[1],before.shape[0]]
        and record['pixels']==before.size,'COPY diagnostic identity differs')
    require(record['source_sha256']==digest(source) and record['before_sha256']==digest(before)
        and record['actual_sha256']==record['expected_sha256']==digest(after),
        'COPY differs from independent original/owned-input history')


def cursor_draws(row,descs,size,joined,owners):
    """Read the animated frame from original call arguments, never GPU pixels."""
    descriptor=descs[row['cursor_type']];hits=[]
    for v in joined:
        if v['world']!=row['frame']['generation'] or not v['atlas']:continue
        owner=owners[v['sequence']]
        sr=list(owner[24:28]);dr=list(owner[28:32])
        # Our finite edge points align their directional hotspots with the
        # image edge; selected cursors have complete40x40 original rectangles.
        if sr[0]%40 or sr[1]%40 or sr[2]-sr[0]!=40 or sr[3]-sr[1]!=40:continue
        frame=sr[0]//40+10*(sr[1]//40)
        if frame not in descriptor['frames']:continue
        g=original_geometry(row,descs,size,frame)
        if sr!=g['source'] or dr!=g['destination']:continue
        require(list(owner[42:44])==v['words'][:2] and v['words'][2:6]==sr
            and v['words'][6:8]==dr[:2] and v['words'][8]==1,
            'Original cursor atlas request differs from emitted native geometry')
        hits.append(dict(sequence=v['sequence'],frame=frame,geometry=g))
    require(hits,'Original hotspot state lacks exact original caller/atlas output')
    return hits


def readiness_states(acquired,rows,observed,name,window):
    ready=[v for v in acquired['actions'] if v['action']=='hotspot-readiness' and v['name']==name]
    require(len(ready)==1,'Missing/duplicate recorded hotspot readiness')
    ready=ready[0];lo=int(window['released']['host_us']);hi=ready['host_ns']//1000
    require(lo<hi<=int(window['settled']['host_us']),'Readiness clock outside held pointer interval')
    entries=ready['observations'][-3:]
    require(len(entries)==3 and len({v['generation'] for v in entries})==3,'Three distinct displayed readiness states required')
    selected=[]
    for entry in entries:
        require(0<entry['ordinal']<=len(rows),'Unbound readiness original row')
        row=rows[entry['ordinal']-1];paint_id=int(entry['paint'])
        require(row['kind']==1 and row['frame'] and row['frame']['generation']==entry['generation']
            and row['cursor'] and row['cursor']['poll']==entry['poll'] and row['cursor_type']==entry['cursor_class']
            and row['saved_cursor']==entry['saved'] and window['released']['frame_input_row']<row['ordinal']
            <=window['settled']['frame_input_row'] and paint_id in observed['swaps'],
            'Readiness source/poll/save/paint identity differs')
        paint,body=observed['paints'][paint_id]
        require(int(paint['world'])==entry['generation'] and lo<int(paint['host_us'])<=hi,
            'Readiness paint is from a later physical pointer action')
        selected.append((row,paint,body))
    return selected


def cursor_readiness_paint(observed,row,recorded,body,hits,lo,hi):
    """Bind the first complete swapped paint inside the original readiness hold.

    A World import can be painted before its subsequent cursor COPY. The
    collector's recorded paint remains evidence of that partial presentation;
    an independently captured later swap can prove the completed cursor only
    within the same World, scaling and original readiness clock interval.
    Selection uses command order and clocks, never pixel agreement.
    """
    before=[v for v in hits if v['sequence']<=recorded['commands']]
    chosen=recorded
    if not before:
        candidates=[(paint,raw)for index in observed['swaps']for paint,raw in[observed['paints'][index]]
            if int(paint['world'])==row['frame']['generation']
            and paint['scaling']==recorded['scaling']
            and max(lo,int(recorded['host_us']))<int(paint['host_us'])<=hi
            and any(v['sequence']<=paint['commands']for v in hits)]
        require(candidates,'No complete cursor paint inside the original readiness interval')
        chosen,body=min(candidates,key=lambda v:(int(v[0]['host_us']),int(v[0]['paint'])))
        before=[v for v in hits if v['sequence']<=chosen['commands']]
    require(before,'Original cursor draw did not precede the actual readiness paint')
    return chosen,body,before,dict(recorded_paint=int(recorded['paint']),
        selected_paint=int(chosen['paint']),recorded_commands=recorded['commands'],
        selected_commands=chosen['commands'],readiness_host_us=hi,
        selected_host_us=int(chosen['host_us']),same_original_readiness_interval=True,
        recorded_partial_paint_retained=chosen is not recorded)


def cursor_viewport(raw,paint,placement,atlas,key):
    """Every visible non-key atlas sample, with no native pixel choosing input."""
    x0,y0,x1,y1=placement['destination'];sx,sy,_,_=placement['source']
    xs=np.minimum(((np.arange(paint['image_width'])+.5)*paint['frame_width']/paint['image_width']).astype(int),paint['frame_width']-1)
    ys=np.minimum(((np.arange(paint['image_height'])+.5)*paint['frame_height']/paint['image_height']).astype(int),paint['frame_height']-1)
    xx=np.flatnonzero((xs>=x0)&(xs<x1));yy=np.flatnonzero((ys>=y0)&(ys<y1))
    require(len(xx)>0 and len(yy)>0,'Cursor footprint is outside displayed output')
    words=atlas[sy+ys[yy,None]-y0,sx+xs[None,xx]-x0];mask=words!=key
    require(np.count_nonzero(mask)>0,'Cursor has no independent opaque stencil')
    actual=np.frombuffer(raw,dtype='u1').reshape(paint['height'],paint['width'],4)
    actual=actual[paint['top']+yy[:,None],paint['left']+xx[None,:]][mask]
    expected=rgba(words,16,(0xf800,0x7e0,31),None)[mask]
    require(np.array_equal(actual,expected),'Displayed nonzero-hotspot cursor differs from original atlas stencil')
    return dict(viewport_pixels=len(actual),actual_sha256=hashlib.sha256(actual.tobytes()).hexdigest(),
        expected_sha256=hashlib.sha256(expected.tobytes()).hexdigest())


def require_copy_observation(sequence,checked,interval):
    require(sequence in checked or not(interval[0]<=sequence<=interval[1]),
        'Missing actual COPY output inside the required cursor/World interval')


def bind_original_copy(v,owner):
    from rendering_operations_history import signed
    w=v['words'];sr=list(map(signed,owner[24:28]));dr=list(map(signed,owner[28:32]))
    keyed=bool(owner[12]&(1 if owner[11] else 0x8000))
    require(owner[15] in (1,2) and not owner[13]&0x80000000 and owner[14]==1
        and not owner[10]&128 and (not keyed or owner[10]&64)
        and w[:2]==list(owner[42:44]) and w[8:10]==[int(keyed),owner[22] if keyed else 0]
        and v['source_extent']==list(owner[16:18]) and v['target_extent']==list(owner[19:21])
        and sr[2]-sr[0]==dr[2]-dr[0] and sr[3]-sr[1]==dr[3]-dr[1]
        and sr[0]<=w[2]<w[4]<=sr[2] and sr[1]<=w[3]<w[5]<=sr[3]
        and w[6]-w[2]==dr[0]-sr[0] and w[7]-w[3]==dr[1]-sr[1],
        'Original COPY caller/resource/key/rectangle differs from the native command')


def copy_history(observed,worlds,records,atlas,required_interval):
    """No diagnostic source or destination plane seeds the independent model."""
    checked={r['sequence']:r for r in records}
    require(len(checked)==len(records),'Duplicate COPY diagnostic')
    model=History(worlds);joined=[];atlas_ids=set();total=0;missing=[];required=0
    for op,seq,p in commands(observed['commands']):
        if op==3:
            w=list(struct.unpack('<10I',p));require_copy_observation(seq,checked,required_interval)
            s,t=model.handles[w[0]],model.handles[w[1]]
            source,before=s['pixels'].copy(),t['pixels'].copy()
            is_atlas=source.shape==atlas.shape and np.array_equal(source,atlas)
            if is_atlas:atlas_ids.add(w[0])
            model.apply(op,seq,p)
            if seq in checked:
                check_copy(checked.pop(seq),w,source,before,t['pixels'])
                total+=t['pixels'].size
            else:missing.append(dict(sequence=seq,world=model.generation,actual_copy_output_observed=False))
            required+=int(required_interval[0]<=seq<=required_interval[1])
            joined.append(dict(sequence=seq,world=model.generation,words=w,
                atlas=is_atlas,source_extent=list(source.shape[::-1]),target_extent=list(before.shape[::-1])))
        else:model.apply(op,seq,p)
    require(not checked and atlas_ids and not model.handles,'Incomplete independent COPY history/atlas/lifetime')
    require(required>0,'No cursor-scenario World COPY interval')
    return joined,dict(verified=True,copies=len(joined),compared_copies=len(records),pixels=total,
        required_copy_interval=list(required_interval),required_compared_copies=required,
        all_application_copies_compared=not missing,unobserved_outside_cursor_interval=missing,
        atlas_handles=sorted(atlas_ids),diagnostic_pixels_used_as_reference_inputs=False,
        scope='Complete actual COPY outputs in the cursor-scenario World interval plus every available outside observation. Raw broad-observer truncation stays negative; no complete-menu/all-operation COPY claim.')


def windows(acquired,rows,observed):
    marks=acquired['camera_actions'];result={};previous=0
    for i in range(0,len(marks),3):
        group=marks[i:i+3];require(len(group)==3,'Incomplete action interval')
        name=group[0]['action'];require(name not in result and
            [v['phase'] for v in group]==['begin','released','settled'] and
            all(v['action']==name and v['accepted'] for v in group),'Action interval identity differs')
        times=[int(v['host_us']) for v in group]
        require(previous<=times[0]<=times[1]<times[2],'Action clocks differ');previous=times[-1]
        for v in group:
            require(0<v['frame_input_row']<=len(rows),'Unbound action row')
            r=rows[v['frame_input_row']-1]
            require(r['kind']==1 and r['frame'] and r['frame']['generation']==v['frame_generation']
                and r['tick']==v['original_frame_tick'],'Action source identity differs')
        result[name]=dict(zip(('begin','released','settled'),group))
    return result


def assess(reader,a,c,observed,worlds):
    cfg=a['configuration'];size=cfg['actual_size']
    require(a['success'] and a['original_game_runs']==1 and a['private_wine_stopped']
        and a['prefix_pool']['returned'] and not a.get('forced_private_prefix_stop')
        and a['original_manifest_verified_before_after'] and a['exit_code']==0,
        'Incomplete normal original acquisition')
    require(size==([800,600] if cfg['high_resolution'] else [640,480])
        and ('--software-rendering' in a['command']) is cfg['software_flag']
        and cfg['compiled_test_driver_invoked'] is False and cfg['skip_intros'] is False
        and cfg['cursor_hotspot']['synthetic_coordinate_exercise'] is False,
        'Wrong hotspot acquisition domain')
    desc=descriptors(artifact(reader,a,'cursors.cfg'))
    atlas=bitmap_words((Path(a['session'])/'game/Bitmaps/cursors.bmp').read_bytes())
    import native_copy_routes,cursor_copy_state,native_cursor_journal
    session=Path(a['session']);directory=Path(a['temporary_root'])/'copy-diagnostics'
    # Every physical diagnostic consumed below must be an exact retained input.
    retained={v['path']:v for v in a['artifacts']}
    for f in [*directory.rglob('*'),*(session/'lock-capture').glob('*.bin')]:
        if not f.is_file():continue
        v=retained.get(str(f.relative_to(ROOT)))
        require(v is not None and hashlib.sha256(f.read_bytes()).hexdigest()==v['sha256']
            ==reader.digest(v['record']),'Unbound physical diagnostic')
    rh,routes=native_copy_routes.decode(session/'lock-capture/copy-routes-v2.bin')
    route_proof=native_copy_routes.validate(session)
    require(route_proof['failed_calls']==0 and not route_proof['routes'].get('unprepared'),
        'Failed original COPY remains a negative; no hotspot success credit')
    companion=cursor_copy_state.validate(session)
    import copy_caller_provenance
    layouts=reader.json('research/runtime/copy-caller-results-20261102/layouts01.json')['wrappers']
    for v,length in zip(layouts,[5,5,6,6,7,8,8]):v['hook_length']=length
    _,callers=copy_caller_provenance.decode((session/'lock-capture/copy-callers-v3.bin').read_bytes(),rh,routes,layouts)
    report=json.loads((directory/'report.json').read_text())
    records=[json.loads(v) for v in (directory/'copies.jsonl').read_text().splitlines()]
    require(report['ended'] and not report['error'] and report['selection']=='all-copies'
        and report['copies_checked']==len(records) and report['copies_seen']>=len(records)
        and report['diagnostic_native_readbacks']==3*len(records)
        and report['pixels_compared']==sum(v['pixels'] for v in records),
        'Invalid broad COPY observer accounting')
    first=[v['frame_generation'] for v in a['camera_actions']
        if v['action']=='sharp-hotspot-center' and v['phase']=='begin']
    require(len(first)==1,'Missing first planned physical cursor action')
    required_first=first[0]
    required_last=max(v['frame_generation'] for v in a['camera_actions']
        if v['action']=='integer-restore-frame-left' and v['phase']=='settled')
    imports=[(struct.unpack_from('<I',p,4)[0],seq) for op,seq,p in commands(observed['commands']) if op==19]
    require(imports and imports[0][0]==1 and 1<=required_first<=required_last<len(imports),'Missing source progress after cursor restoration')
    interval=(next(seq for g,seq in imports if g==required_first),next(seq-1 for g,seq in imports if g==required_last+1))
    joined,history=copy_history(observed,worlds,records,atlas,interval)
    history['required_world_generations']=[required_first,required_last]
    require(report['copies_seen']==len(joined),'Broad COPY census differs from captured command history')
    owners={}
    for r in routes:
        for seq in range(r[40],r[41]+1) if r[40] else ():
            require(seq not in owners,'Overlapping original COPY intervals');owners[seq]=r
    require({v['sequence'] for v in joined}==set(owners),'Unowned/missing COPY command')
    for v in joined:bind_original_copy(v,owners[v['sequence']])
    from natural_gpu_copy_branches import union_area
    by_sequence={v['sequence']:v for v in joined};complete_cursor_calls=0
    for owner,caller in zip(routes,callers):
        if caller[17]!=3 or not owner[40] or not(interval[0]<=owner[40]<=interval[1]):continue
        require(owner[41]<=interval[1],'Cursor call crosses the required observation interval')
        pieces=[by_sequence[s]['words'] for s in range(owner[40],owner[41]+1)]
        rects=[[w[6],w[7],w[6]+w[4]-w[2],w[7]+w[5]-w[3]] for w in pieces]
        dr=list(owner[28:32]);dw,dh=owner[19:21]
        require(0<=dr[0]<dr[2]<=dw and 0<=dr[1]<dr[3]<=dh
            and union_area(rects)==(dr[2]-dr[0])*(dr[3]-dr[1]),
            'Cursor save/draw/restore output does not cover the complete original destination')
        complete_cursor_calls+=1
    require(complete_cursor_calls>0,'No complete original cursor copy calls')
    history['complete_original_cursor_calls']=complete_cursor_calls
    d=decode(artifact(reader,a,'frame-input.bin'));rows=d['rows'];intervals=windows(a,rows,observed)
    require(d['nonce']==a['resident']['nonce'],'Wrong original source session')
    paints=[dict(world=int(p['world']),paint=int(p['paint']),host_us=int(p['host_us']),scaling=p['scaling'])
        for index in observed['swaps'] for p,body in [observed['paints'][index]] if int(p['world'])]
    cases=[];classes=Counter();nonzero=[]
    for mode,scale in [('sharp',0),('integer',2)]:
        for label,point in points(size):
            for name in [mode+'-hotspot-'+label]+([mode+'-restore-'+label] if label!='center' else []):
                w=intervals[name];selected=readiness_states(a,rows,observed,name,w)
                tail=[v[0] for v in selected];shown=[v[1] for v in selected]
                expected_point=point if '-hotspot-' in name else points(size)[0][1]
                pointers=[v for v in a['actions'] if v['action']=='pointer'
                    and int(w['begin']['host_us'])*1000<=v['host_ns']<=int(w['released']['host_us'])*1000+1000]
                require(len(pointers)==1,'Action lacks a unique physical pointer event')
                pointer=pointers[0];paint=observed['paints'][int(pointer['paint'])][0]
                xy=[pointer['screen'][i]-pointer['viewport'][i] for i in (0,1)]
                mapped=[((xy[i]-paint[k])*2+1)*paint[f]//(2*paint[e])
                    for i,k,f,e in [(0,'left','frame_width','image_width'),(1,'top','frame_height','image_height')]]
                require(mapped==pointer['logical']==expected_point,'Physical display texel differs from original cursor point')
                require(all(p['scaling']==scale for p in shown),'Incorrect displayed scaling')
                require(len({r['cursor']['poll'] for r in tail})>=3 and all(
                    r['cursor']['point']==expected_point and cursor_after_action(r,rows[w['released']['frame_input_row']-1])
                    for r in tail),'Stale/different original pointer')
                geometries=[];paint_bindings=[]
                readiness=next(v for v in a['actions']if v['action']=='hotspot-readiness'and v['name']==name)
                for r,shown_paint,shown_body in selected:
                    classes[r['cursor_type']]+=1
                    zero_classes={n for n,d in desc.items() if d['hotspot']==[0,0]}
                    require(r['cursor_type'] in (REQUIRED_CLASSES if '-hotspot-frame-' in name else zero_classes),
                        'The settled edge/default cursor class is not the required actual state')
                    hits=cursor_draws(r,desc,size,joined,owners)
                    shown_paint,shown_body,hits,binding=cursor_readiness_paint(observed,r,shown_paint,shown_body,
                        hits,int(w['released']['host_us']),readiness['host_ns']//1000)
                    paint_bindings.append(binding)
                    for v in hits:
                        owner=owners[v['sequence']];caller=callers[owner[0]-1]
                        require(owner[13]==0
                            and caller[9]==1 and caller[17]==3
                            and caller[3] in (0x18c8a0,0x18ca90),
                            'Hotspot draw lacks resolved original cursor caller')
                    g=hits[0]['geometry'];g['copy_sequences']=[v['sequence'] for v in hits]
                    g['original_requested_frames']=sorted({v['frame'] for v in hits});geometries.append(g)
                    if r['cursor_type'] in REQUIRED_CLASSES:
                        keys={owners[v['sequence']][22] for v in hits}
                        require(len(keys)==1,'Original cursor key changed inside one paint')
                        g['viewport_stencil']=cursor_viewport(shown_body() if callable(shown_body) else shown_body,
                            shown_paint,g,atlas,next(iter(keys)))
                require(len(geometries)==3 and len({g['cursor_class'] for g in geometries})==1,
                    'Need three stable original descriptor/copy/output states')
                nonzero.extend(dict(mode=mode,action=name,**g) for g in geometries if g['cursor_class'] in REQUIRED_CLASSES)
                cases.append(dict(action=name,point=expected_point,readiness_paints=paint_bindings,
                    generations=[r['frame']['generation'] for r in tail],classes=[r['cursor_type'] for r in tail]))
        require(REQUIRED_CLASSES<={v['cursor_class'] for v in nonzero if v['mode']==mode},
            'Required nonzero cursor classes were not actually observed')
    spec=importlib.util.spec_from_file_location('hotspot_cleanup',ROOT/'tools/join-rendering-normal-acceptance.py')
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
    cleanup=m.cleanup_proof(reader,a)
    require(cleanup['verified'],'Owner retirement incomplete')
    return dict(verified=True,windows=cases,classes=dict(classes),nonzero=nonzero,
        copy_history=history,broad_copy_diagnostic=report,routes=route_proof,companion={k:v for k,v in companion.items() if k!='observations'},
        cleanup=cleanup,scope='Finite actual nonzero-hotspot descriptor/state/keyed-copy and complete independent COPY/display history. Edge-to-center moves retain all restoration outputs. No original driver, arbitrary cursor/provider or lifetime/ABI claim.')
