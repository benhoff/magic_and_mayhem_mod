"""Stopped-capture picking and live-rebuild acceptance; no original input calls."""
from rendering_picking_resize import targets, ready, physical_point
from rendering_pilot import require

def assess(reader, acquired, compared, observed):
    from pathlib import Path
    import importlib.util
    from campaign_input_observation import decode as admission_decode
    from rendering_interaction import decode
    from rendering_menu_acceptance import artifact
    from resident_capture import records
    root=Path(__file__).resolve().parents[1]
    def module(name):
        spec=importlib.util.spec_from_file_location('pick_'+name,root/'tools'/(name+'.py'))
        m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
    require(acquired['success'] and acquired['sources_stable']
            and acquired['original_game_runs']==1 and acquired['private_wine_stopped']
            and acquired['prefix_pool']['returned'] and not acquired.get('forced_private_prefix_stop'),
            'Incomplete nominal picking execution')
    config=acquired['configuration']
    require(config['actual_size_schedule']==[[800,600],[640,480],[800,600]]
            and config['source_only_targets'] and config['live_original_display_rebuilds']
            and config['skip_intros'] is False and config['terrain_lighting']=='native'
            and config['compiled_test_driver_invoked'] is False
            and ('--software-rendering' in acquired['command']) is config['software_flag'],
            'Picking configuration differs')
    require(compared['comparison']['success'] and compared['comparison']['mismatching_pixels']==0
            and compared['original_reference']['continuous_from_owned_initial_history']
            and not compared['original_reference']['native_player_pixels_read']
            and not compared['original_reference']['original_destination_capture_read'], 'Missing independent complete output')
    retirement=acquired['retirement'];graphics=retirement['graphics_retirement']
    require(retirement['state']=='completed' and retirement['forwarded']==0
            and graphics['process_exit'] and graphics['resources_released']
            and not graphics['surviving_owners'] and all(n==0 for n in graphics['counts'].values()),
            'Original Quit did not retire all graphics owners')
    decoded=decode(artifact(reader,acquired,'frame-input.bin'));rows=decoded['rows']
    admissions=admission_decode(artifact(reader,acquired,'input-admission.bin'))['rows']
    windows=acquired['proof']['picking_windows'];expected=[]
    for phase,complete in [('high-initial',True),('low',True),('high-returned',False)]:
        for mode in ('sharp','integer'):
            for view in range(4 if complete else 1):
                expected.extend(phase+'-'+mode+'-view-'+str(view)+'-'+kind for kind in ('object','cell'))
            expected.extend(phase+'-'+mode+'-panned-'+kind for kind in ('object','cell'))
    require([w['name'] for w in windows]==expected and len(windows)==48,'Incomplete picking action matrix')
    targets_by_generation={}
    for w in windows:
        t=w['target'];targets_by_generation.setdefault(t['source_generation'],[]).append(w)
    contexts={};chunks=0;source=Path(acquired['temporary_root'])/'worlds.bin'
    producer=module('inspect-canvas-producers');camera=module('close-rendering-camera-matrix')
    for generation,chunk,stream,request in records(source):
        _,source_rows=producer.decode(stream,complete=False)
        context=camera.context_many_cells(source_rows)
        if chunk:
            chunks+=1
            require(context is None,'Source chunk carries a World camera')
            continue
        contexts[generation]=context
        for w in targets_by_generation.get(generation,[]):
            t=w['target'];state=rows[t['frame_input_row']-1]
            require(state['kind']==1 and state['frame']['generation']==generation,'Unbound target source')
            body,foot=targets(source_rows,state['history'],state['position'])
            actual=body if w['kind']=='object' else foot
            require(all(t[k]==v for k,v in actual.items()),'Target differs from unique captured body/foot')
    life=module('close-rendering-world-exit').imported_lifetimes(observed['commands'])
    require(len(contexts)==len(life['imports'])==compared['original_reference']['worlds']
            and chunks==compared['original_reference']['source_chunks'],'Source/World/import cardinality differs')
    shown=[observed['paints'][i][0] for i in observed['swaps'] if int(observed['paints'][i][0]['world'])>0]
    require(len(shown)==compared['observations']['world_swaps'],'Unaccounted World swap')
    by_paint={(int(p['world']),int(p['paint'])):p for p in shown}
    pointers=[a for a in acquired['actions'] if a['action']=='pointer']
    facts=[];grounds={};previous_end=0
    for w in windows:
        begin,released,settled=(w[k] for k in ('begin','released','settled'))
        require(previous_end<=int(begin['host_us'])<=int(released['host_us'])<int(settled['host_us']),
                'Overlapping/reordered picking windows')
        previous_end=int(settled['host_us'])
        baseline=rows[w['baseline_row']-1]
        require(baseline['ordinal']<=begin['frame_input_row']<=released['frame_input_row']<settled['frame_input_row'],
                'Picking baseline/action identities differ')
        for marker in (begin,released,settled):
            row=rows[marker['frame_input_row']-1]
            require(row['kind']==1 and row['frame']['generation']==marker['frame_generation']
                    and row['tick']==marker['original_frame_tick'] and marker['actual_size']==w['size'],
                    'Picking marker is not an exact original source frame')
        chosen=[p for p in pointers if int(begin['host_us'])<=p['host_ns']//1000<=int(released['host_us'])]
        require(len(chosen)==1 and chosen[0]['logical']==w['target']['point'],'Missing unique prescribed physical motion')
        require(len(w['witnesses'])==3 and len({x['generation'] for x in w['witnesses']})==3,'Three source generations required')
        for fact in w['witnesses']:
            p=by_paint.get((fact['generation'],fact['paint']))
            require(p is not None and p['scaling']==w['scaling']
                    and [p['frame_width'],p['frame_height']]==w['size'], 'Picking witness was not actually displayed in its mode/size')
            require(int(released['host_us'])<int(p['host_us'])<=int(settled['host_us'])+500000,'Picking paint outside action')
            actual=ready(rows[:fact['frame_row']],admissions,p,w['target'],baseline,w['kind'])
            require(actual==fact,'Original object/cell/return/causal witness differs')
            physical_point(chosen[0],p,w['target']['point'])
            context=contexts[fact['generation']]
            require(context and context['view']==fact['orientation'] and context['creature_markers']>0,
                    'Original picking orientation does not match displayed source')
            if w['kind']=='cell':
                phase=next(s for s in ('high-initial','low','high-returned') if w['name'].startswith(s+'-'))
                grounds.setdefault(phase,fact['ground'])
                require(fact['ground']==grounds[phase],'Stationary source-derived foot changed picked cell within its World')
        facts.append(dict(name=w['name'],size=w['size'],scaling=w['scaling'],kind=w['kind'],witnesses=w['witnesses']))
    rebuilds=acquired['proof']['display_rebuilds']
    require(len(rebuilds)==2 and [(r['before'],r['after']) for r in rebuilds]
            ==[([800,600],[640,480]),([640,480],[800,600])],'Both original display rebuilds required')
    for rebuild in rebuilds:
        a,b=rebuild['original_before'],rebuild['original_after'];p=rebuild['confirmed_paint']
        require(a['frame']['generation']<b['frame']['generation'] and a['thread']==b['thread']==decoded['owner']
                and [p['frame_width'],p['frame_height']]==rebuild['after']
                and b['high_resolution']==int(rebuild['after'][0]==800),'Rebuild lacks current source/owner/size identity')
        require(rebuild['same_original_process'] and rebuild['same_world'] is False
                and rebuild['results']['screen']==26 and rebuild['preferences']['screen']==10
                and rebuild['returned']['screen']==3 and rebuild['selected_map']['map']==1
                and rebuild['preferences']['preferences'][2]==int(rebuild['before'][0]==640),
                'Frame change lacks the original Results/Main/Preferences/recreated-map route')
        old,new=(life['imports'][r['frame']['generation']-1] for r in (a,b))
        require((old['surface'],old['created_sequence'])!=(new['surface'],new['created_sequence']),
                'Frame-size rebuild reused the old live surface incarnation')
    for phase in ('high-initial','low'):
        for mode in ('sharp','integer'):
            selected=[w for w in windows if w['name'].startswith(phase+'-'+mode) and w['kind']=='object']
            views=[w['witnesses'][-1]['orientation'] for w in selected[:4]]
            require(set(views)==set(range(4)),'Picking did not cover every orientation')
            starts=[m for m in acquired['camera_actions'] if m['action']==phase+'-'+mode+'-pan' and m['phase']=='begin']
            require(len(starts)==1,'Missing unique physical pan action')
            start=starts[0]
            first=contexts[start['frame_generation']]
            last=contexts[selected[-1]['witnesses'][-1]['generation']]
            require(first and first['view']==last['view'] and first['center']!=last['center'],
                    'Picking pan did not change the source camera in its existing orientation')
    return dict(verified=True,windows=len(facts),object_windows=24,cell_windows=24,
                displayed_source_witnesses=sum(len(x['witnesses']) for x in facts),
                stable_picked_cells_by_world=grounds,live_display_rebuilds=2,
                rebuild_route='Quick Results -> Main Preferences -> same-map new Quick World',same_world_continuity=False,
                source_generations=len(contexts),source_chunks=chunks,windows_verified=facts,
                allocation_generations_proved=False,selection_hotspot_or_response_deadlines_proved=False)
