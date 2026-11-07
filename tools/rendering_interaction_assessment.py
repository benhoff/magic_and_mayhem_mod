"""Strict stopped-case joins; complete output and finite original interaction."""
import hashlib
from rendering_pilot import require
from rendering_menu_acceptance import artifact
from rendering_interaction import decode
from campaign_input_observation import decode as admission_decode
from pathlib import Path
import importlib.util
import tempfile
from rendering_interaction_live import sprite_target
from resident_capture import records
ROOT=Path(__file__).resolve().parents[1]

def module(name):
    spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/(name+'.py'))
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def source_contexts(reader,a):return module('close-rendering-camera-matrix').source_contexts(reader,a)

def presented_interval(rows, paints, released, settled):
    """Join completed source identities across bounded asynchronous presentation."""
    rs=[r for r in rows[released['frame_input_row']:settled['frame_input_row']]
        if r['kind']==1 and r['frame']]
    owned={r['frame']['generation'] for r in rs}
    shown=[p for p in paints if p['world'] in owned
           and ('scaling' not in released or p['scaling']==released['scaling'])
           and int(released['host_us'])<p['host_us']<=int(settled['host_us'])+500000]
    visible={p['world'] for p in shown}
    generations=list(dict.fromkeys(r['frame']['generation'] for r in rs if r['frame']['generation'] in visible))
    require(len(generations)>=3,'Fewer than three actually presented settled source generations')
    return [r for r in rs if r['frame']['generation'] in generations[-3:]],shown

def cursor_after_action(row,baseline):
    cursor=row['cursor'];previous=baseline['cursor']
    return bool(cursor and cursor['poll']>(previous['poll'] if previous else 0)
                and (cursor['tick']-baseline['tick'])%2**32<2**31
                and (row['tick']-cursor['tick'])%2**32<2**31)

def presentation_scale_matches(paint,sharp_finished,integer_adopted):
    if paint['host_us']<=sharp_finished:return paint['scaling']==0
    if paint['host_us']>=integer_adopted:return paint['scaling']==2
    return paint['scaling'] in (0,2)

def hit_window_end(hover, following, entry):
    """Finish a hover before the following owned original selection callback."""
    settled=hover['settled'];released=following['released'];completed=following['settled']
    span=(completed['original_frame_tick']-settled['original_frame_tick'])%2**32
    require(entry['kind']==2 and settled['frame_input_row']<entry['ordinal']<=completed['frame_input_row']
            and span<2**31 and (entry['tick']-settled['original_frame_tick'])%2**32<=span
            and 0<=int(released['host_us'])-int(settled['host_us'])<=1000000
            and 0<=int(completed['host_us'])-int(settled['host_us'])<=6000000,
            'Original hit callback is outside the bounded hover window')
    return dict(frame_input_row=entry['ordinal']-1,host_us=completed['host_us'])

def assess(reader, acquired, compared, observed):
    config = acquired['configuration']; size = config['actual_size']
    require(size == ([800, 600] if config['high_resolution'] else [640, 480])
            and type(config['software_flag']) is bool and config['skip_intros'] is False
            and ('--software-rendering' in acquired['command']) is config['software_flag']
            and config.get('viewport_scaling') == 'sharp-and-integer'
            and config['compiled_test_driver_invoked'] is False and config['terrain_lighting'] == 'native', 'Wrong camera domain/policy')
    require(acquired['success'] is True and acquired['original_manifest_verified_before_after'] is True
            and acquired['original_game_runs'] == 1
            and acquired['private_wine_stopped'] is True and acquired['prefix_pool']['returned'] is True
            and not acquired.get('forced_private_prefix_stop', False), 'Incomplete nominal camera acquisition')
    require(compared['comparison']['success'] is True and compared['comparison']['mismatching_pixels'] == 0
            and compared['original_reference']['continuous_from_owned_initial_history'] is True
            and compared['original_reference']['native_player_pixels_read'] is False
            and compared['original_reference']['original_destination_capture_read'] is False
            and hashlib.sha256(observed['commands']).hexdigest() == compared['captured_command_stream_sha256'],
            'Uncompared/dependent camera history')
    contexts, chunks = source_contexts(reader, acquired)
    life = module('close-rendering-world-exit').imported_lifetimes(observed['commands'])
    worlds = compared['world_domain']['worlds']
    require(len(worlds) == len(contexts) == len(life['imports']) == compared['original_reference']['worlds']
            and chunks == compared['original_reference']['source_chunks']
            and all(r == dict(generation=i, width=size[0], height=size[1]) for i, r in enumerate(worlds, 1)), 'Mixed camera source generation/domain')
    expected = [r for r in compared['comparison']['comparisons'] if r['world'] > 0]
    shown = [observed['paints'][p] for p in observed['swaps'] if int(observed['paints'][p][0]['world']) > 0]
    require(shown and len(shown) == len(expected), 'Camera output omits/duplicates actual swaps')
    paints, hashes = [], {}
    sharp_finished=int(next(m['host_us'] for m in acquired['camera_actions'] if m['action']=='sharp-panned-away-2' and m['phase']=='settled'))
    scale_changes=[v for v in acquired['actions'] if v['action']=='scaling' and v['mode']=='integer']
    require(len(scale_changes)==1,'Public Integer adoption missing/duplicated')
    integer_adopted=scale_changes[0]['host_ns']//1000
    require(sharp_finished<integer_adopted,'Public scaling phase order differs')
    for (p, body), match in zip(shown, expected):
        g, paint = int(p['world']), int(p['paint'])
        require(g in contexts and match['world'] == g and match['paint'] == paint
                and match['lease'] == int(p['lease']) > 0, 'Camera source/paint/lease mismatch')
        imported = life['imports'][g-1]
        require(imported['import_sequence'] <= p['commands'] < imported['deleted_sequence']
                and p['native_gpu_frame'] is p['gpu_read_complete'] is p['pixels_captured'] is True
                and p['presentation_owner'] == 'gui' and [p['frame_width'], p['frame_height']] == size,
                'Camera output lacks a live native GPU source')
        require(p['scaling'] in (0,2), 'Unexpected presentation policy')
        require(presentation_scale_matches(dict(host_us=int(p['host_us']),scaling=p['scaling']),sharp_finished,integer_adopted),
                'Actual viewport policy differs from the public scaling phase')
        if paint not in hashes:
            hashes[paint] = hashlib.sha256(body() if callable(body) else body).hexdigest()
        require(match['actual_sha256'] == match['expected_sha256'] == hashes[paint]
                and match['pixels'] == p['width'] * p['height'] and match['mismatching_pixels'] == 0, 'Camera viewport pixels changed')
        paints.append(dict(world=g, paint=paint, lease=int(p['lease']), host_us=int(p['host_us']),
                           pixels=match['pixels'], rgba_sha256=hashes[paint], scaling=p['scaling'], camera=contexts[g]['camera']))
    return interaction_proof(reader,acquired,compared,observed,contexts,chunks,paints)


def interaction_proof(reader,a,c,observed,contexts,chunks,paints):
    d=decode(artifact(reader,a,'frame-input.bin'));rows=d['rows']
    admission=admission_decode(artifact(reader,a,'input-admission.bin'))
    marks=a['camera_actions']
    require(d['nonce']==a['resident']['nonce'],'Frame witness belongs to another source session')
    module('rendering_gameplay_acceptance').bind_markers(marks,admission)
    require(d['owner']==admission['owner_thread'] and all(s['thread']==d['owner'] for s in a['states']), 'Frame/menu/input original owners differ')
    windows={};previous=0
    require(len(marks)%3==0,'Incomplete action schedule')
    for i in range(0,len(marks),3):
        group=marks[i:i+3];name=group[0]['action']
        require(name not in windows and [r['phase'] for r in group]==['begin','released','settled'] and all(r['action']==name and r['accepted'] for r in group),'Action schedule identity differs')
        times=[int(r['host_us']) for r in group]
        require(previous<=times[0]<=times[1]<times[2],'Action clocks differ');previous=times[-1]
        for m in group:
            ordinal=m['frame_input_row'];require(0<ordinal<=len(rows),'Unbound frame/input marker')
            r=rows[ordinal-1]
            require(r['kind']==1 and r['frame'] and r['frame']['generation']==m['frame_generation'] and r['tick']==m['original_frame_tick'],'Frame/input marker identity differs')
        windows[name]=dict(zip(('begin','released','settled'),group))
    transitions={}
    def selection_transition(name):
        if name in transitions:return transitions[name]
        w=windows[name];lo=w['begin']['frame_input_row'];hi=w['settled']['frame_input_row']
        entries={r['call']:r for r in rows[lo:hi] if r['kind']==2};changed=[]
        for end in (r for r in rows[lo:hi] if r['kind']==3):
            begin=entries.get(end['call'])
            if not begin or not begin['hit'] or not end['hit']:continue
            h,k=begin['hit'],end['hit']
            if h['flag68']==k['flag68']:continue
            require(begin['frame'] and end['frame'] and begin['mask']&1
                    and h['slot']==k['slot']==0 and h['type']==k['type']==0 and h['owner']==k['owner']==0
                    and h['pool']==k['pool'] and h['active'] and k['active']
                    and h['flag68'] in (0,1) and k['flag68']==1-h['flag68'],'Changed selection has wrong original identity')
            changed.append(dict(call=end['call'],entry=begin['ordinal'],returned=end['ordinal'],before=h['flag68'],after=k['flag68'],pool=h['pool'],mouse=begin['mouse'][:2],frame=begin['frame']))
        require(len(changed)==1,'Selection needs exactly one original callback transition')
        transitions[name]=changed[0];return changed[0]
    def interval(name):
        w=windows[name];upper=w['settled']
        following=(name.removesuffix('-hover-2')+'-toggle-2' if name.endswith('-hover-2') else
                   name.removesuffix('-hover')+'-toggle-1' if name.endswith('-hover') else None)
        if following:
            transition=selection_transition(following)
            upper=hit_window_end(w,windows[following],rows[transition['entry']-1])
        tail,shown=presented_interval(rows,paints,w['released'],upper)
        for r in tail:
            g=r['frame']['generation'];ctx=contexts[g]['camera']
            require(ctx is not None and ctx['view']==r['camera'][0] and ctx['creature_markers']>0,'Original state and source-frame camera disagree')
            require(cursor_after_action(r,rows[w['released']['frame_input_row']-1]),'Cursor poll does not follow the actual source action boundary')
            require(r['tracked'] and r['tracked']['slot']==0 and r['tracked']['type']==0 and r['tracked']['owner']==0 and r['tracked']['active'] and 0<r['tracked']['health']<2**31,'Wizard actor role changed')
        return tail,shown
    target_actions=[r for r in a['actions'] if r['action'] in ('source-sprite-target','source-sprite-focus')]
    require(len(target_actions)==12,'Source-only target/focus schedule differs')
    targets={}
    for target in target_actions:
        state=rows[target['frame_input_row']-1]
        require(state['frame']['generation']==target['source_generation'] and state['history'][:2]==target['anchor'],'Target/source witness identity differs')
        targets.setdefault(target['source_generation'],[]).append(target)
    checked_targets=[]
    decoder=module('inspect-canvas-producers')
    with tempfile.TemporaryDirectory(prefix='interaction-target-',dir=ROOT/'working') as temp:
        path=Path(temp)/'worlds.bin';path.write_bytes(artifact(reader,a,'worlds.bin'))
        for g,chunk,stream,request in records(path):
            if chunk or g not in targets:continue
            _,source_rows=decoder.decode(stream,complete=False)
            for target in targets[g]:
                state=rows[target['frame_input_row']-1];expected=sprite_target(source_rows,state['history'])
                require(all(target[k]==v for k,v in expected.items()),'Target was not the declared source-only opaque sprite point')
                checked_targets.append(dict(source_generation=g,**expected))
    require(len(checked_targets)==len(target_actions),'Unbound source target')
    cursor_bindings=[]
    def pointer_proof(name,tail):
        w=windows[name]
        points=[v for v in a['actions'] if v['action']=='pointer' and int(w['begin']['host_us'])*1000<=v['host_ns']<=int(w['released']['host_us'])*1000+1000]
        require(len(points)==1,'Action lacks a unique physical pointer')
        pointer=points[0];p=observed['paints'][int(pointer['paint'])][0]
        x,y=(pointer['screen'][i]-pointer['viewport'][i] for i in (0,1))
        expected=[((x-p['left'])*2+1)*p['frame_width']//(2*p['image_width']),((y-p['top'])*2+1)*p['frame_height']//(2*p['image_height'])]
        require(expected==pointer['logical'],'Physical display pixel differs from requested source texel')
        if name.endswith('-hover') or name.endswith('-hover-2'):
            following=name.removesuffix('-hover-2')+'-toggle-2' if name.endswith('-hover-2') else name.removesuffix('-hover')+'-toggle-1'
            require(selection_transition(following)['mouse']==expected,'Original selection callback mouse differs from displayed source pixel')
        for r in tail:
            require(r['cursor']['point']==expected,'Actual completed original cursor point differs from displayed source pixel')
            require(r['saved_cursor'] and r['saved_cursor'][0] and all(0<=expected[i]-r['saved_cursor'][i+1]<40 for i in (0,1)),'Cursor background/hotspot envelope does not follow the pointer')
        require(len({r['cursor']['poll'] for r in tail})>=3,'Cursor proof reuses a single stale poll')
        cursor_bindings.append(dict(action=name,point=expected,source_frames=[r['frame']['generation'] for r in tail],polls=[r['cursor']['poll'] for r in tail],
            observed_poll_to_world_return_ms=[(r['tick']-r['cursor']['tick'])%2**32 for r in tail],
            saved_origins=[r['saved_cursor'][1:] for r in tail]))
        return expected
    pairs=[];views=[];pans=[]
    required=[]
    for mode,scaling in [('sharp',0),('integer',2)]:
        if mode+'-resume' in windows:
            required.append(mode+'-resume');interval(mode+'-resume')
        for n in range(2):required.append(mode+'-center-'+str(n))
        for v,expected_view in enumerate((0,3,2,1)):
            prefix=mode+'-view-'+str(v)
            for suffix in ('-hover','-toggle-1','-away-1','-hover-2','-toggle-2','-away-2'):required.append(prefix+suffix)
            required.append(mode+'-rotate-'+str(v))
            tail,shown=interval(prefix+'-hover')
            pointer_proof(prefix+'-hover',tail)
            require(all(r['camera'][0]==expected_view and r['hit'] and r['hit']['slot']==0 and r['hit']['type']==0 and r['hit']['owner']==0 for r in tail),'Wrong view/original actor hit')
            require(all(p['scaling']==scaling for p in shown),'Wrong actual scaling during hover')
            views.append(dict(mode=mode,view=expected_view,frames=[r['frame']['generation'] for r in tail],hit=tail[-1]['hit'],history=tail[-1]['history']))
        required.append(mode+'-pan')
        for suffix in ('-hover','-toggle-1','-away-1','-hover-2','-toggle-2','-away-2'):required.append(mode+'-panned'+suffix)
        before,_=interval(mode+'-rotate-3');after,_=interval(mode+'-pan')
        require(before[-1]['camera'][0]==after[-1]['camera'][0]==0 and before[-1]['camera'][1:]!=after[-1]['camera'][1:],'Camera did not pan after completing four views')
        require(contexts[before[-1]['frame']['generation']]['camera']['center']!=contexts[after[-1]['frame']['generation']]['camera']['center'],'Source minimap center did not pan')
        pans.append(dict(mode=mode,before=before[-1]['camera'],after=after[-1]['camera'],source_frames=[before[-1]['frame']['generation'],after[-1]['frame']['generation']]))
        for label in ['view-'+str(v) for v in range(4)]+['panned']:
            name=mode+'-'+label;toggles=[]
            for suffix in ('-hover','-hover-2'):
                tail,_=interval(name+suffix)
                require(all(r['mode'] in (0,1) and r['hit'] and r['hit']['slot']==0 and r['hit']['type']==0 and r['hit']['owner']==0 for r in tail),'Hover did not hit the living wizard in an observed mode')
                if suffix=='-hover-2' or label=='panned':pointer_proof(name+suffix,tail)
            for index in (1,2):
                action=name+'-toggle-'+str(index);toggle=selection_transition(action)
                tail,shown=interval(name+'-away-'+str(index))
                pointer_proof(name+'-away-'+str(index),tail)
                require(all(r['tracked']['pool']==toggle['pool'] and r['tracked']['flag68']==toggle['after'] for r in tail),'Selection did not survive moving pointer away')
                require(all(p['scaling']==scaling for p in shown),'Wrong actual selection scaling')
                toggles.append(toggle)
            require(toggles[0]['after']==toggles[1]['before'] and toggles[1]['after']==toggles[0]['before'],'Original selection toggle pair does not restore initial state')
            pairs.append(dict(mode=mode,case=label,toggles=toggles))
    required.append('open-results')
    require(list(windows)==required,'Bounded schedule omitted or added actions')
    cleanup=module('join-rendering-normal-acceptance').cleanup_proof(reader,a)
    require(cleanup['verified'] and cleanup['normal_quit'] and cleanup['source_imports']==len(contexts),'Original Quit/source retirement incomplete')
    require(a['proof']['map']['map']==1 and a['proof']['map']['map_name']=='Celtic Forest One (small)' and a['proof']['preferences'][2]==int(not a['configuration']['high_resolution']),'Original map/resolution differs')
    require(len({r['world_counter'] for r in admission['rows'] if r['mode']==0})>=8,'Original simulation counter did not advance')
    return dict(verified=True,domain=[a['configuration']['software_flag'],a['configuration']['actual_size']],source_generations=len(contexts),source_chunks=chunks,world_presentations=len(paints),retained_pixels=sum(p['pixels'] for p in paints),views=views,pans=pans,selection_pairs=pairs,cursor_bindings=cursor_bindings,source_targets=checked_targets,cursor_samples=sum(bool(r['cursor']) for r in rows),frame_input_rows=len(rows),original_owner=d['owner'],source_nonce=d['nonce'],cleanup=cleanup,allocation_generations_proved=False,whole_game_picking_proved=False)
