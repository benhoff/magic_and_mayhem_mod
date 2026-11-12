#!/usr/bin/env python3
"""Admit source-bound minimap updates through actual physical pan intervals."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

from coverage_claims import behavior_contract, fingerprint, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_acceptance import requirements, validate
from rendering_interaction import decode
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, require
from resident_capture import records

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-markers-results-20261107/'
MATRIX = 'research/runtime/rendering-interaction-results-20261107/matrix-01.json'
PARENT = 'research/runtime/rendering-picking-resize-results-20261107/pan-closeout-02.json'
BID = 'NP.marker-acceptance'
SID = 'native.marker-acceptance'
LEAF = 'journey.hud-minimap-tooltips.markers'


def module(name):
    spec = importlib.util.spec_from_file_location('first_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    value = importlib.util.module_from_spec(spec); spec.loader.exec_module(value); return value


def checked(reader, ref):
    require(reader.digest(ref['record']) == ref['sha256'], 'Changed retained first-World input')
    return reader.json(ref['record'])


def caller(row, words, generation, nonce, size, context):
    """Actual post-World observation, not synthetic callback or a pixel guess."""
    actor = row['tracked']
    require(row['kind'] == 1 and row['world_return'] == generation
            and row['frame']['generation'] == generation and row['frame']['nonce'] == str(nonce)
            and row['thread'] == row['frame']['owner'] and row['mode'] == 0
            and words[20:22] == (2, 1) and row['high_resolution'] == int(size == [800, 600])
            and actor and actor['slot'] == actor['type'] == actor['owner'] == 0
            and actor['active'] == 1 and actor['health'] > 0 and actor['pool'] > 0
            and row['camera'][0] == 0 and row['position'] == row['camera'][1:4]
            and row['history'] and 0 <= row['history'][0] < size[0]
            and 0 <= row['history'][1] < size[1] and row['history'][2] > 0,
            'Incomplete first-World original caller state')
    if generation == 1:
        require(row['history'] == ([330,218,106] if size == [800,600] else [250,158,106]),
                'First caller preparation history differs')
    if context:
        require(context['view'] == row['camera'][0]
                and context['center'] == [row['camera'][1] + 2, row['camera'][2] + 2]
                and context['creature_markers'] > 0, 'Scene/source camera differs from original return')
    return dict(generation=generation, observation=row, original_result=words[18],
                last_error=words[19], object_screen=words[20], initialized=words[21],
                scope='Observed World return and living numeric wizard role; no whole-register/EFLAGS or allocation-generation recovery')


def validate_startup(c):
    require(c['verified'] and c['normal_quit'] and c['owned_initial_history']
            and not c['native_pixels_seed_oracle'] and not c['original_destination_seed_oracle']
            and c['first_labelled_world'] == 1 and c['first_scene_generation'] == 2
            and c['scene_generations'] == [2, 3, 4] and c['startup_chunks'] > 0
            and c['startup_creates'] > 0 and c['startup_jpegs'] > 0
            and c['guarded_batches'] == 4 and c['checked_rasters'] > 0
            and c['caller_generations'] == [1, 2, 3, 4]
            and c['whole_viewport_swaps'] > c['scene_swaps'] >= 3
            and c['loading_world_swaps'] > 0 and c['pre_world_swaps'] > 0
            and c['compared_pixels'] > 0 and c['mismatching_pixels'] == 0
            and c['surface_lifetime_verified'] and c['source_nonce_verified']
            and c['every_prefix_swap_bound'] and c['first_scene_has_present'],
            'Incomplete first composed Quick World admission')


def validate_marker_interval(v):
    require(v['verified'] and v['normal_quit'] and v['original_selection_acknowledged']
            and v['selection_state'] in (0,1) and v['view'] in range(4)
            and v['source_generations'] == 3 and v['whole_viewport_swaps'] >= 3
            and v['compared_pixels'] > 0 and v['mismatching_pixels'] == 0
            and v['matched_marker_pixels'] > 0 and v['all_three_roles_presented']
            and v['cell_return_checked'] and v['camera_origin_checked']
            and v['creature_dimension_refresh_checked'] and v['owned_copy_lineage']
            and v['original_outer_border_disabled'] and not v['outer_border_render_claimed'],
            'Incomplete presented markers/camera border evidence')


def validate_case(c):
    validate_startup(c)
    intervals=c['marker_intervals']
    require(len(intervals)==20 and {(v['mode'],v['case'],v['selection_state']) for v in intervals}
            == {(mode,case,state) for mode in ('sharp','integer')
                for case in ('view-0','view-1','view-2','view-3','panned') for state in (0,1)},
            'Incomplete marker views/scales/selection states')
    for v in intervals:validate_marker_interval(v)


def marker_words(rows):
    """Independent documented NoCD overlay geometry and consumed results."""
    camera=module('close-rendering-camera-matrix').context_many_cells(rows)
    require(camera and camera['creature_markers']>0,'Missing current marker context')
    overlays=[(f,struct.unpack_from('<37I',b),b[148:]) for f,b in rows if f[2]==24]
    require([m[0] for f,m,b in overlays]==[2]+[0]*(len(overlays)-2)+[1], 'Wrong marker ordering')
    width,height=overlays[0][0][5:7];known={};owners={};forms=set()
    def signed(v):return v if v<2**31 else v-2**32
    def put(x,y,word,role):
        require(0<=x<width and 0<=y<height,'Marker outside owned logical canvas')
        at=y*width+x;known[at]=word;owners[at]=role
    def project(m,x,y):
        u=(x+m[1]//2-m[3])%m[1];v=(y+m[2]//2-m[4])%m[2]
        if m[9]==1:u,v=v,m[5]-u
        elif m[9]==2:u,v=m[5]-u,m[6]-v
        elif m[9]==3:u,v=m[6]-v,u
        return m[7]+u-v,m[8]+int((u+v)/2)
    for f,m,body in overlays:
        require(m[10]==0 and f[3]==camera['surface'] and f[5:7]==(width,height),
                'Mixed marker canvas or unsupported non-RGB565 domain')
        if m[0]==2:
            view=f[17];a,b=(m[6]//2,m[5]//2) if view in (1,2) else (m[5]//2,m[6]//2)
            ox=m[7] if view==0 else f[7]-(m[5] if view==3 else m[6])-8;oy=m[8] if view==0 else 6
            require(f[19:21]==(ox,oy),'Camera origin result differs')
            corners=[(signed(m[20+i*2]),signed(m[21+i*2])) for i in range(4)]
            require(set(corners)=={(-1,-1),(-1,1),(1,-1),(1,1)},'Missing required camera corner')
            for dx,dy in corners:
                x=(a+b)//2-dx*(m[18]//128+1)*2-b;y=(a+b)//2-1-dy*(m[19]//64)+int(dy<0)
                for i in range(4):put(ox+x+i*dx,oy+y,0x400,'camera');put(ox+x,oy+y+i*dy,0x400,'camera')
        elif m[0]==0:
            x,y=project(m,signed(m[13]),signed(m[14]));ring=bool(m[12] and m[16]);forms.add('ring' if ring else 'square')
            require(f[21]==(y+(2 if ring else 1))*f[7]+x+(2 if ring else 0),'Original selected cell return offset differs')
            offsets=[(-1,-1),(2,-1),(1,0),(0,0),(0,1),(1,1),(2,2),(-1,2)] if ring else [(0,0),(1,0),(0,1),(1,1)]
            require(m[15]<9 and m[16] in (0,1),'Invalid cell emphasis/palette')
            for dx,dy in offsets:put(x+dx,y+dy,m[28+m[15]],'cell')
        else:
            require(f[19:21]==m[1:3] and len(body)==m[17]*12,'Creature dimension refresh/list differs')
            for x,y,hidden in struct.iter_unpack('<3I',body):
                require(0<=x<m[1] and 0<=y<m[2] and hidden in (0,1),'Invalid source creature marker')
                if m[11] and hidden:continue
                x,y=project(m,x,y)
                for dx,dy in [(-1,-2),(0,-2),(1,-2),(2,-1),(-2,-1),(-2,0),(0,0),(2,0),(2,1),(-2,1),(-1,2),(0,2),(1,2)]:
                    put(x+dx,y+dy,0xfc00,'creature')
    require(set(owners.values())=={'camera','cell','creature'},'Required marker role has no surviving words')
    copies=[f for f,b in rows if f[2]==6 and f[4]==camera['surface']
            and f[1]>overlays[-1][0][1] and f[8:14]==(0,0,0,0,width,height)]
    require(len(copies)==1,'Missing unique original pre-scene marker display copy')
    return dict(context=camera,known=known,owners=owners,forms=sorted(forms),shape=[height,width],
                copy_source=list(copies[0]),marker_calls=len(overlays),outer_border_disabled=True)


def presented_marker_words(observed, wanted):
    """Trace only independently computed markers through owned C1 history.

    World imports are unknown pixels, never destination snapshots. CPU uploads
    are operation inputs; their marker words must match the independent model.
    Keyed copies conservatively discard lineage. Only actual direct PRESENTs
    can supply marker evidence.
    """
    import numpy as np
    from rendering_pilot import commands
    stride=4194304;planes={};imports=[];jobs={};prior=0
    ops=list(commands(observed['commands']))
    for op,seq,p in ops:
        if op!=19:continue
        w=struct.unpack('<5I',p);g=w[1]|w[2]<<32
        if g in wanted:
            choices=[(n,struct.unpack('<10I',b)) for code,n,b in ops if prior<n<seq and code==3
                and struct.unpack_from('<I',b)[0]==w[0]
                and list(struct.unpack('<10I',b)[2:])==[0,0,wanted[g]['shape'][1],wanted[g]['shape'][0],0,0,0,0]]
            require(len(choices)==1,'Missing/ambiguous live pre-import marker COPY')
            n,words=choices[0];jobs[n]=(g,words);imports.append(g)
        prior=seq
    require(set(imports)==set(wanted),'Missing marker source import')
    paints={}
    for ordinal in observed['swaps']:
        p,body=observed['paints'][ordinal]
        if p['native_gpu_frame']:paints.setdefault(p['commands'],[]).append((p,body))
    presented=None;display=None;found={};copies={}
    for op,seq,p in ops:
        if op==1:
            sid,w,h,bits=struct.unpack_from('<4I',p)
            planes[sid]=dict(values=np.zeros((h,w),dtype='uint32'),known=np.zeros((h,w),dtype=bool),tags=np.full((h,w),-1,dtype='int64'),bits=bits,created=seq)
        elif op in (2,20):
            sid,x,y,w,h=struct.unpack_from('<5I',p);plane=planes[sid];count=w*h;unit=plane['bits']//8
            values=np.frombuffer(p[20:20+count*unit],dtype={1:'u1',2:'<u2',4:'<u4'}[unit]).reshape(h,w)
            mask=np.ones((h,w),dtype=bool) if op==2 else np.frombuffer(p[20+count*unit:],dtype='u1').reshape(h,w).astype(bool)
            plane['values'][y:y+h,x:x+w][mask]=values[mask];plane['known'][y:y+h,x:x+w][mask]=True;plane['tags'][y:y+h,x:x+w][mask]=-1
        elif op==3:
            w=struct.unpack('<10I',p);source,target=planes[w[0]],planes[w[1]];l,t,r,b,x,y=w[2:8]
            if seq in jobs:
                g,words=jobs[seq];item=wanted[g];indices=np.array(sorted(item['known']),dtype='int64')
                require(source['bits']==16 and source['values'].shape==tuple(item['shape'])
                        and np.all(source['known'].ravel()[indices])
                        and np.array_equal(source['values'].ravel()[indices],np.array([item['known'][int(i)] for i in indices])),
                        'Owned pre-scene marker words differ from independently computed source semantics')
                source['tags'].ravel()[indices]=g*stride+indices
                copies[g]=dict(sequence=seq,source_surface=w[0],target_surface=w[1],source_created=source['created'],target_created=target['created'],words=list(words))
            for field in ('values','known','tags'):
                target[field][y:y+b-t,x:x+r-l]=source[field][t:b,l:r].copy()
            if w[8] or source['bits']!=target['bits']:target['known'][y:y+b-t,x:x+r-l]=False;target['tags'][y:y+b-t,x:x+r-l]=-1
        elif op==7:planes.pop(struct.unpack('<I',p)[0])
        elif op==11:
            a,b=struct.unpack_from('<2I',p)
            for field in ('values','known','tags'):planes[a][field],planes[b][field]=planes[b][field],planes[a][field]
        elif op==19:
            sid=struct.unpack_from('<I',p)[0];planes[sid]['known'][:]=False;planes[sid]['tags'][:]=-1
        elif op==6:
            display=struct.unpack_from('<I',p)[0];presented=planes[display]['tags'].copy()
        elif op in (5,10,17):raise ValueError('Unsupported diagnostic or surface-copy path in marker trace')
        for paint,body in paints.get(seq,[]):
            if paint['mode']!=0 or presented is None:continue
            require(paint['surface']==display,'Marker PRESENT/display mismatch')
            gens=set(map(int,np.unique(presented[presented>=0]//stride))) & set(wanted)
            for g in gens-set(found):
                item=wanted[g];yy,xx=np.nonzero(presented//stride==g);indices=presented[yy,xx]%stride
                if set(map(int,indices))!=set(item['known']):continue
                raw=body() if callable(body) else body;actual=np.frombuffer(raw,dtype='u1').reshape(paint['height'],paint['width'],4)
                xs=((np.arange(paint['image_width'])+.5)*paint['frame_width']/paint['image_width']).astype(int)
                ys=((np.arange(paint['image_height'])+.5)*paint['frame_height']/paint['image_height']).astype(int)
                expected=[];values=[];roles=set()
                for y,x,index in zip(yy,xx,indices):
                    destx=np.flatnonzero(xs==x)+paint['left'];desty=np.flatnonzero(ys==y)+paint['top']
                    if not len(destx) or not len(desty):continue
                    word=item['known'][int(index)]
                    # RGB565 presentation uses bit replication, not integer scaling.
                    r5,g6,b5=(word>>11)&31,(word>>5)&63,word&31;rgb=[(r5<<3)|(r5>>2),(g6<<2)|(g6>>4),(b5<<3)|(b5>>2),255]
                    pixels=actual[desty[:,None],destx[None,:]]
                    require(np.all(pixels==np.array(rgb,dtype='u1')),'Presented marker pixel differs from independently expected WORD')
                    values.append(pixels.tobytes());expected.append(bytes(rgb)*len(destx)*len(desty));roles.add(item['owners'][int(index)])
                require(roles=={'camera','cell','creature'} and values,'Required marker role is absent from presentation')
                found[g]=dict(generation=g,paint=int(paint['paint']),presentation_world=int(paint['world']),commands=seq,
                    matched_pixels=sum(len(v)//4 for v in values),source_words=len(item['known']),roles=sorted(roles),
                    actual_sha256=hashlib.sha256(b''.join(values)).hexdigest(),expected_sha256=hashlib.sha256(b''.join(expected)).hexdigest(),copy=copies[g])
        if len(found)==len(wanted):break
    require(set(found)==set(wanted),'Missing source-bound markers in actual complete native presentation')
    return found


def marker_intervals(reader,comparison,acquired,observed,frames):
    from rendering_interaction_assessment import presented_interval
    tasks=[];wanted=set();pairs=[]
    shown=[observed['paints'][i] for i in observed['swaps'] if int(observed['paints'][i][0]['world'])>0]
    matches=[m for m in comparison['comparison']['comparisons'] if m['world']>0]
    require(len(shown)==len(matches),'Missing actual marker viewport swap')
    pairs=list(zip(shown,matches));flat=[dict(p,world=int(p['world']),host_us=int(p['host_us'])) for p,b in shown]
    for group in comparison['proof']['selection_pairs']:
        for index,toggle in enumerate(group['toggles'],1):
            action=group['mode']+'-'+group['case']+'-away-'+str(index)
            marks={v['phase']:v for v in acquired['camera_actions'] if v['action']==action}
            require(set(marks)=={'begin','released','settled'},'Missing physical post-selection marker interval')
            tail,visible=presented_interval(frames['rows'],flat,marks['released'],marks['settled'])
            require(all(v['tracked']['flag68']==toggle['after'] and v['tracked']['pool']==toggle['pool'] for v in tail),
                    'Presented marker interval lost actual original selection state')
            entry,returned=(frames['rows'][toggle[k]-1] for k in ('entry','returned'))
            require(entry['kind']==2 and returned['kind']==3 and entry['call']==returned['call']==toggle['call']
                    and entry['hit']['flag68']==toggle['before'] and returned['hit']['flag68']==toggle['after'], 'Missing original marker selection callback acknowledgement')
            gens=[v['frame']['generation'] for v in tail];wanted.update(gens)
            selected=[(p,b,m) for (p,b),m in pairs if int(marks['released']['host_us'])<int(p['host_us'])<=int(marks['settled']['host_us'])+500000]
            tasks.append((group,toggle,gens,selected))
    contexts={};decoder=module('inspect-canvas-producers')
    for g,ch,stream,request in records(Path(acquired['temporary_root'])/'worlds.bin'):
        if ch or g not in wanted:continue
        _,rows=decoder.decode(stream,complete=False);item=marker_words(rows)
        item.update(source_sha256=hashlib.sha256(stream).hexdigest());contexts[g]=item
    require(set(contexts)==wanted,'Missing required marker source generation')
    output=presented_marker_words(observed,contexts);results=[];hashes={}
    for group,toggle,gens,selected in tasks:
        view=(0,3,2,1)[int(group['case'][-1])] if group['case']!='panned' else 0
        require(all(contexts[g]['context']['view']==view for g in gens),'Wrong actual marker view')
        pixels=0
        for p,body,match in selected:
            ordinal=int(p['paint'])
            if ordinal not in hashes:hashes[ordinal]=hashlib.sha256(body() if callable(body) else body).hexdigest()
            require(hashes[ordinal]==match['actual_sha256']==match['expected_sha256'] and match['mismatching_pixels']==0
                    and match['paint']==ordinal and match['world']==int(p['world']) and match['lease']==int(p['lease']),
                    'Complete marker interval viewport differs from independent original history')
            pixels+=match['pixels']
        result=dict(verified=True,normal_quit=True,mode=group['mode'],case=group['case'],view=view,
            selection_state=toggle['after'],original_selection_acknowledged=True,selection=toggle,
            source_generations=len(gens),sources=[dict(generation=g,context=contexts[g]['context'],forms=contexts[g]['forms'],source_sha256=contexts[g]['source_sha256'],original_display_copy=contexts[g]['copy_source']) for g in gens],
            whole_viewport_swaps=len(selected),compared_pixels=pixels,mismatching_pixels=0,
            presentations=[output[g] for g in gens],matched_marker_pixels=sum(output[g]['matched_pixels'] for g in gens),
            all_three_roles_presented=True,cell_return_checked=True,camera_origin_checked=True,
            creature_dimension_refresh_checked=True,owned_copy_lineage=True,
            original_outer_border_disabled=True,outer_border_render_claimed=False)
        validate_marker_interval(result);results.append(result)
    require({form for v in contexts.values() for form in v['forms']}=={'square','ring'},'Missing actual square/ring selection-marker output')
    return results


def case(reader, ref):
    c = checked(reader, ref); a = checked(reader, c['acquisition']); proof = c['proof']
    oracle = c['original_reference']
    require(c['success'] and c['sources_stable'] and c['comparison_processes_stopped']
            and c['comparison']['success'] and c['comparison']['mismatching_pixels'] == 0
            and c['comparison']['remaining_surfaces'] == 0
            and oracle['continuous_from_owned_initial_history']
            and not oracle['native_player_pixels_read'] and not oracle['original_destination_capture_read']
            and proof['verified'] and proof['cleanup']['normal_quit']
            and a['success'] and a['original_game_runs'] == 1 and a['private_wine_stopped']
            and a['original_manifest_verified_before_after'] and not a['original_pixels_used_as_native_inputs']
            and not a.get('forced_private_prefix_stop')
            and all(reader.digest(n) == h for n, h in c['sources'].items()),
            'Unaccepted or changed startup execution/oracle phase')
    print('Verifying retained startup: ' + ref['record'], flush=True)
    streams = [artifact(reader, a, Path(v['path']).name) for v in a['artifacts']
               if '/capture/' in v['path'] and v['path'].endswith('.bin')]
    observed = acquisition(streams, a['session_identity'])
    require(hashlib.sha256(observed['commands']).hexdigest() == c['captured_command_stream_sha256'],
            'Different independently compared command history')
    lifetimes = module('close-rendering-world-exit').imported_lifetimes(observed['commands'])
    frame_raw = artifact(reader, a, 'frame-input.bin'); frames = decode(frame_raw)
    require(frames['nonce'] == str(lifetimes['nonce']) == proof['source_nonce'], 'Different source/caller nonce')
    returns = [v for v in frames['rows'] if v['kind'] == 1 and v['frame'] and v['frame']['generation'] <= 4]
    require([v['frame']['generation'] for v in returns] == [1, 2, 3, 4], 'Missing/duplicate initial World return')
    source = Path(a['temporary_root']) / 'worlds.bin'
    source_sha = hashlib.sha256(artifact(reader, a, 'worlds.bin')).hexdigest()
    require(hashlib.sha256(source.read_bytes()).hexdigest() == source_sha, 'Changed physical startup archive')
    decoder = module('inspect-canvas-producers'); camera = module('close-rendering-camera-matrix')
    request_check = module('close-rendering-destination-history').checked_request
    startup = {}; contexts = {}; batches = []; callers = []; chunks = []
    for generation, chunk, stream, request in records(source):
        _, rows = decoder.decode(stream, complete=False)
        if chunk:
            require(not batches, 'Late startup chunk in first-World prefix')
            chunks.append(dict(generation=generation,chunk=chunk,sha256=hashlib.sha256(stream).hexdigest()))
            for fields, body in rows: startup[fields[2]] = startup.get(fields[2], 0) + 1
            continue
        require(generation == len(batches) + 1 <= 4, 'Initial source generation gap')
        entry, rasters = request_check(rows, request)
        context = camera.context_many_cells(rows); contexts[generation] = context
        require((generation == 1 and context is None) or (generation > 1 and context),
                'Scene qualification changed: loading cannot stand in for first scene')
        row = returns[generation - 1]
        words = struct.unpack_from('<96I', frame_raw, 16 + (row['ordinal'] - 1) * 384)
        callers.append(caller(row, words, generation, lifetimes['nonce'], a['configuration']['actual_size'], context))
        batches.append(dict(generation=generation, rasters=len(rasters), guarded_no_access=True,
                            source_sha256=hashlib.sha256(stream).hexdigest(),
                            request_sha256=hashlib.sha256(request).hexdigest(), canvas=entry[3], context=context))
        if generation == 4: break
    require(len(batches) == 4, 'Missing initial source batches')
    native_swaps = [i for i in observed['swaps'] if observed['paints'][i][0]['native_gpu_frame']]
    shown = [(observed['paints'][i], m) for i,m in zip(native_swaps, c['comparison']['comparisons'])]
    require(len(shown) == len(native_swaps) == len(c['comparison']['comparisons']), 'Omitted native swap')
    require(all(int(p['world']) == m['world'] and int(p['paint']) == m['paint'] for (p,b),m in shown),
            'Comparison/presentation sequence differs')
    prefix = []
    for (paint, body), match in shown:
        if int(paint['world']) > 4: break
        if not paint['native_gpu_frame']: continue
        prefix.append((paint,body,match))
    require(prefix and len(prefix) <= 512 and {int(p['world']) for p,b,m in prefix} == {0,1,2,3,4},
            'Missing or oversized startup viewport prefix')
    imports = {v['generation']: v for v in lifetimes['imports'][:4]}
    from rendering_pilot import commands
    # Original canvas IDs, imported native World storage and composed display
    # handles occupy different namespaces. Check each actual incarnation.
    live={};display_lives={}
    for op,seq,payload in commands(observed['commands']):
        if op==1: live[struct.unpack_from('<I',payload)[0]]=seq
        elif op==7:
            sid=struct.unpack('<I',payload)[0]
            display_lives.setdefault(sid,[]).append((live.pop(sid),seq))
    hashes = {}; witnesses = []; pixels = 0
    for paint, body, match in prefix:
        g = int(paint['world']); index = int(paint['paint'])
        require(paint['native_gpu_frame'] and paint['gpu_read_complete'] and paint['pixels_captured']
                and paint['presentation_owner'] == 'gui' and paint['transport'] == 'opengl'
                and paint['scaling'] == 0
                and [paint['frame_width'], paint['frame_height']] == a['configuration']['actual_size']
                and match['lease'] == int(paint['lease']), 'Incomplete initial native GPU viewport')
        if g:
            life = imports[g]
            require(batches[g-1]['canvas'] == batches[0]['canvas']
                    and life['created_sequence'] < life['import_sequence'] <= paint['commands'] < life['deleted_sequence']
                    and sum(begin < paint['commands'] < end for begin,end in display_lives.get(paint['surface'],[])) == 1,
                    'First World rendered from another surface incarnation')
        if index not in hashes: hashes[index] = hashlib.sha256(body() if callable(body) else body).hexdigest()
        require(hashes[index] == match['actual_sha256'] == match['expected_sha256']
                and match['pixels'] == paint['width'] * paint['height'] and match['mismatching_pixels'] == 0,
                'Initial viewport differs from independent complete owned-history oracle')
        pixels += match['pixels']
        witnesses.append(dict(paint=index,world=g,commands=paint['commands'],lease=paint['lease'],
                              actual_sha256=hashes[index],expected_sha256=match['expected_sha256'],pixels=match['pixels']))
    # A World import alone cannot qualify a stale present of the loading screen.
    present = {}; current = 0; last = 0
    wanted = {v['commands'] for v in witnesses if v['world'] >= 2}
    for op,seq,payload in commands(observed['commands']):
        require(op not in (5,10), 'Destination diagnostic pixels in native/oracle input')
        if op == 19: current = struct.unpack_from('<5I',payload)[1]
        if op == 6: last = current
        if seq in wanted: present[seq] = last
    require(all(present[v['commands']] == v['world'] for v in witnesses if v['world'] >= 2),
            'First scene has no matching actual PRESENT')
    result = dict(verified=True, acquisition=c['acquisition'], comparison=ref, domain=proof['domain'],
        normal_quit=True, owned_initial_history=True, native_pixels_seed_oracle=False,
        original_destination_seed_oracle=False, first_labelled_world=1, first_scene_generation=2,
        scene_generations=[2,3,4], startup_chunks=len(chunks), startup_creates=startup.get(1,0),
        startup_jpegs=startup.get(7,0), startup_operations=startup, startup_chunk_witnesses=chunks,
        guarded_batches=len(batches), checked_rasters=sum(b['rasters'] for b in batches), batches=batches,
        caller_generations=[v['generation'] for v in callers], callers=callers,
        whole_viewport_swaps=len(prefix), scene_swaps=sum(v['world']>=2 for v in witnesses),
        loading_world_swaps=sum(v['world']==1 for v in witnesses), pre_world_swaps=sum(v['world']==0 for v in witnesses),
        compared_pixels=pixels, mismatching_pixels=0, surface_lifetime_verified=True,
        source_nonce_verified=True, every_prefix_swap_bound=True, first_scene_has_present=True,
        source_archive_sha256=source_sha, viewport_witnesses=witnesses,
        original_reference=oracle, source_phase='Unchanged IP07 original/native/compiler/oracle execution; fresh retained-source and viewport hash analysis')
    validate_startup(result)
    result['marker_intervals'] = marker_intervals(reader,c,a,observed,frames)
    validate_case(result)
    print('Verified marker intervals: '+str(result['domain'])+' '+str(len(result['marker_intervals'])),flush=True)
    return result


def close_rows(plan, parent, cases):
    validate(plan); defs = requirements(plan)
    require(parent['dossier_identity'] == fingerprint(defs) and len(parent['requirements']) == len(defs) == 156
            and {(v['configuration'],v['id']) for v in parent['requirements']} == {(v['configuration'],v['id']) for v in defs},
            'Frozen acceptance inventory differs')
    require(len(cases)==4 and {(v['domain'][0],tuple(v['domain'][1])) for v in cases}
            == {(s,z) for s in (False,True) for z in ((800,600),(640,480))}, 'Incomplete first-World matrix')
    for c in cases: validate_case(c)
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id']==LEAF:
            require(row['state']=='pending', 'Markers row already admitted')
            software=row['configuration']=='player-software'
            row.update(state='passed',admitted_execution_evidence=[dict(execution=dict(acquisition=c['acquisition'],comparison=c['comparison']),
                actual_world_extents=[c['domain'][1]],reason='Required camera corners, cell selection square/ring and visible creature markers independently calculated from owned original arguments and exact returns. Real original selection callbacks bind both flag68 states across four orientations plus pan at Sharp/Integer. Ordered owned CPU history, unique pre-import display copy, live resource lineage and direct complete native viewport pixels prove markers reach presentation. Outer diamond border is disabled by the original admitted caller (+6207 nonzero), not claimed rendered. Celtic Forest One, Xvfb/Mesa DPR1; no full cursor/selection restoration, arbitrary markers/maps/providers or atomic post-tick HUD claim.') for c in cases if c['domain'][0] is software])
        rows.append(row)
    require(sum(v['state']=='passed' for v in rows)==parent['passed_requirements']+2, 'First-World admission exceeded two rows')
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(), 'Fresh owned report required')
    result=dict(schema=1,kind='presented_marker_acceptance',success=False)
    with Reader(ROOT) as reader:
        try:
            reg=reader.json('research/runtime/coverage/register.json')
            b=next(v for v in reg['behaviors'] if v['id']==BID);s=next(v for v in reg['scenarios'] if v['id']==SID)
            sources={n:reader.digest(n) for n in required_sources(b)}
            result.update(scope=b['scope'],analysis_sources=sources,
                          parent=dict(record=PARENT,sha256=reader.digest(PARENT)),
                          matrix=dict(record=MATRIX,sha256=reader.digest(MATRIX)))
            matrix=reader.json(MATRIX)
            require(matrix['success'] and matrix['mismatching_pixels']==0
                    and all(reader.digest(n)==h for n,h in matrix['sources'].items()), 'Retained matrix phase changed')
            cases=[case(reader,ref) for ref in matrix['comparisons']]
            result['cases']=cases
            parent=reader.json(PARENT);rows=close_rows(reader.json('docs/native-rendering-closure.json'),parent,cases)
            require(all(reader.digest(n)==h for n,h in sources.items()), 'First-World analysis sources changed')
            result.update(success=True,scope=b['scope'],sources=parent['sources'],analysis_sources=sources,sources_stable=True,
                claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],
                parent=dict(record=PARENT,sha256=reader.digest(PARENT)),matrix=dict(record=MATRIX,sha256=reader.digest(MATRIX)),
                cases=cases,requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),passed_requirements=parent['passed_requirements']+2,
                pending_requirements=154-parent['passed_requirements'],newly_passed_requirements=2,
                compared_pixels=sum(p['compared_pixels'] for v in cases for p in v['marker_intervals']),marker_intervals=80,matched_marker_pixels=sum(p['matched_marker_pixels'] for v in cases for p in v['marker_intervals']),mismatching_pixels=0,
                original_game_runs=0,new_native_executions=0,new_original_oracle_runs=0,new_pixel_comparisons=0,
                historical_failures_retired=False,second_battle_resize_closed=False,
                inherited_current_source_drift={n:dict(recorded=h,current=reader.digest(n)) for n,h in parent['sources'].items() if (ROOT/n).is_file() and reader.digest(n)!=h})
        except Exception as error: result['error']=str(error)
        print(json.dumps(dict(record=args.report,success=result['success'],error=result.get('error'),sha256=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))))
    return 0 if result['success'] else 1


if __name__=='__main__': raise SystemExit(main())
