#!/usr/bin/env python3
"""Join actual EP04 collector pairs against the unchanged 156 acceptance rows."""
import argparse,json,struct,hashlib
from collections import Counter
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_normal_acceptance import progress,PREFIX,admit,receipt,reviewed_driver_sources
from rendering_pilot import require,commands,acquisition,number,counter
from rendering_acceptance import requirements,validate
from coverage_claims import fingerprint
ROOT=Path(__file__).resolve().parents[1]

SUSTAINED_LEAVES={
    'journey.active-world.sustained-world':'Bounded contiguous original source/render progress, complete independently compared presentations and conserved command resources with zero native owners at original Quit.',
    'journey.quick-battle-entry.setup-confirm':'Original offered setup/map confirmations and two actual World visits.',
    'journey.shutdown.menu-exit':'Original Main Quit and complete native resource retirement.'}



def sustained_admit(reader,pair,configuration='player-default'):
    """Keep frozen collector admission unchanged; validate its current format4 here.

    The target/output/ownership guards mirror existing normal admission. The
    explicit codec4 checks bind retained source bytes and complete generation
    counts already consumed by the independent original-history comparison.
    """
    require(configuration in ('player-default','player-software'),'Unknown target configuration')
    require(set(pair)=={'acquisition','comparison'},'A scoped execution needs both collectors')
    a,c=[receipt(reader,pair[k]) for k in ('acquisition','comparison')]
    require(a['kind']=='public_native_normal_acquisition' and c['kind']=='public_native_normal_comparison' and c['acquisition']==pair['acquisition'],'Mixed EP04 collectors')
    require(a['scenario'] in ('normal-quick','normal-storage'),'Unknown normal scenario')
    require(all(v['success'] is True and not v.get('error') and v['sources_stable'] is True and v['original_manifest_verified_before_after'] is True and v['original_pixels_used_as_native_inputs'] is False for v in (a,c)),'Incomplete execution or comparison')
    cfg=a['configuration']
    require(cfg.get('diagnostic_only',False) is False and 'injected_fault' not in cfg,'Diagnostic execution is ineligible for nominal acceptance')
    require(cfg['software_flag'] is (configuration=='player-software') and cfg['skip_intros'] is False and cfg['terrain_lighting']=='native' and cfg.get('source_chunks') is True and cfg['native_lighting_capture'] is False and cfg['viewport_scaling']=='integer-sharp','Execution differs from the requested target')
    require(a['exit_code']==0 and a['private_wine_stopped'] is True and a['retirement']['state']=='completed' and a['retirement']['forwarded']==0,'Normal original exit or strict ownership missing')
    retired=a['retirement']['graphics_retirement']
    require(retired['process_exit'] is True and retired['resources_released'] is True and not retired['surviving_owners'] and all(v==0 for v in retired['counts'].values()),'Surviving graphics owner')
    resident=a['resident']
    if configuration=='player-software':
        require(type(resident['renderer']) is str and resident['renderer'].lower().startswith(('llvmpipe','softpipe')),'Actual Mesa software provider is unproved')
    require(resident['normal_quit_retired'] is True and resident['pending'] is False and not resident['error'] and int(resident['source_copy_rect_refusals'])==0 and resident['renderer'],'World lifetime, provider or refused caller unresolved')
    require(resident.get('source_chunks_enabled') is True and resident.get('stage_profile',{}).get('capture_complete') is True and type(resident['stage_profile']['capture']['format']) is int and resident['stage_profile']['capture']['format'] in (3,4),'Incomplete current-protocol source history')
    require(c['lighting']['verified'] is True and c['original_menu_actions']['verified'] is True and c['asset_inputs_stable'] is True,'Missing actual lighting/actions/assets')
    domain=c['world_domain']
    require(domain['verified'] is True and domain['format']=='RGB565' and domain['extents'] and all(p in ([640,480],[800,600]) for p in domain['extents']),'Unproved World domain')
    capture=resident['stage_profile']['capture']
    if capture['format']==4:
        require(capture.get('codec')=='zstd-prefix-sha256-v1' and capture.get('complete') is True
                and capture.get('finished') is True and capture.get('truncated') is False
                and not capture.get('error') and capture.get('queue_bytes')==0,
                'Incomplete lossless source capture')
        require(number(capture.get('accepted_worlds'),1,8192)==number(capture.get('written_worlds'),1,8192)==int(resident['completed'])
                and number(capture.get('accepted_source_chunks'),0,8192)==number(capture.get('written_source_chunks'),0,8192)==int(resident['source_chunks']),
                'Lossless source capture lost a generation/chunk')
        world_files=[v for v in a['artifacts'] if v['path'].endswith('/worlds.bin')]
        require(len(world_files)==1,'Ambiguous lossless source capture')
        artifact=world_files[0];body=reader.read_bytes(artifact['record'])
        require(body[:8]==b'MNMPROF4' and len(body)==capture['file_bytes']==artifact['bytes']
                and hashlib.sha256(body).hexdigest()==artifact['sha256'],
                'Lossless source format/bytes differ from compared capture')
    ref=c['original_reference']
    require(ref['source_chunks']==int(resident['source_chunks']),'Independent history omitted source chunks')
    require(ref['continuous_from_owned_initial_history'] is True and ref['native_player_pixels_read'] is False and ref['original_destination_capture_read'] is False,'Dependent oracle')
    compared=c['comparison'];cohort=compared['comparisons']
    require(compared['success'] is True and number(compared['world_presentations'],1)==number(c['observations']['world_swaps'],1) and compared['mismatching_pixels']==0 and compared['remaining_surfaces']==0,'Incomplete presented output/history')
    require(cohort and compared['pixels_compared']==sum(number(r['pixels'],1) for r in cohort) and compared['world_presentations']==sum(int(r['world'])>0 for r in cohort) and all(r['mismatching_pixels']==0 and r['actual_sha256']==r['expected_sha256'] for r in cohort),'Inconsistent independent cohort')
    if a['scenario']=='normal-quick':
        require(c['input_acknowledgements']['verified'] is True and c['fullscreen']['verified'] is True,'Actual Qt input or fullscreen output missing')
    else:
        before=number(a['flow']['preferences_cancel_original_dialogue'],0,2);draft=number(a['flow']['preferences_cancel_draft_dialogue'],0,2)
        require(draft!=before,'Cancellation did not exercise an actual changed draft')
        require(c['preferences_boundary']['verified'] is True and a['flow']['preferences_cancel_verified'] is True and c['mini_inputs']['verified'] is True and c['mini_dispatch']['verified'] is True and c['original_menu_actions']['owned_save_persisted'] is True,'Storage/cancel actions incomplete')
    effective=a['sources']
    if c.get('driver_source_applicability'):
        effective=reviewed_driver_sources(reader,c['driver_source_applicability'],pair['acquisition'],a,c['driver_scope_facts'])
    sources={}
    for phase in (effective,c['sources']):
        for n,h in phase.items():
            require(n not in sources or sources[n]==h,'Conflicting execution source phases');sources[n]=h
    return dict(scenario=a['scenario'],sources=sources,provider=resident['renderer'],world_extents=domain['extents'],pair=pair,world_presentations=compared['world_presentations'],pixels=compared['pixels_compared'])


def resource_history(raw):
    """Account existing command lifetimes; bytes describe logical planes, not VRAM."""
    surfaces={};palettes={};worlds=[]
    created=destroyed=made_bytes=freed_bytes=palette_created=palette_destroyed=0
    peak_surfaces=peak_bytes=peak_palettes=0
    for op,seq,payload in commands(raw):
        if op==1:
            require(len(payload)>=28,'Short resource CREATE')
            sid,w,h,bits,r,g,b=struct.unpack_from('<7I',payload)
            require(sid and sid not in surfaces and 1<=w<=2048 and 1<=h<=2048
                    and bits in (8,16,32) and len(payload)==28+w*h*(bits//8),
                    'Duplicate or malformed resource CREATE')
            surfaces[sid]=(w,h,bits,r,g,b);created+=1;made_bytes+=w*h*(bits//8)
        elif op==7:
            require(len(payload)==4,'Malformed resource DELETE')
            sid=struct.unpack('<I',payload)[0]
            require(sid in surfaces,'Unknown resource DELETE')
            w,h,bits,*_=surfaces.pop(sid);destroyed+=1;freed_bytes+=w*h*(bits//8)
        elif op==12:
            require(len(payload)==8+256*3,'Malformed palette CREATE')
            pid,generation=struct.unpack_from('<2I',payload)
            require(pid and pid not in palettes,'Duplicate palette CREATE')
            palettes[pid]=generation;palette_created+=1
        elif op==15:
            require(len(payload)==8,'Malformed palette DELETE')
            pid,generation=struct.unpack('<2I',payload)
            require(pid in palettes and palettes[pid]==generation,'Unknown palette DELETE/generation')
            del palettes[pid];palette_destroyed+=1
        require(created-destroyed==len(surfaces) and palette_created-palette_destroyed==len(palettes),
                'Resource count conservation differs')
        logical_bytes=sum(w*h*(bits//8) for w,h,bits,*_ in surfaces.values())
        require(made_bytes-freed_bytes==logical_bytes,'Resource byte conservation differs')
        peak_surfaces=max(peak_surfaces,len(surfaces));peak_bytes=max(peak_bytes,logical_bytes);peak_palettes=max(peak_palettes,len(palettes))
        if op==19:
            require(len(payload)==20,'Malformed World resource boundary')
            sid,lo,hi,_,_=struct.unpack('<5I',payload);generation=lo+(hi<<32)
            require(generation==len(worlds)+1 and sid in surfaces,'World resource generation/owner differs')
            w,h,bits,r,g,b=surfaces[sid]
            require((bits,r,g,b)==(16,0xf800,0x7e0,31) and (w,h) in ((640,480),(800,600)),
                    'World resource domain differs')
            worlds.append(dict(generation=generation,sequence=seq,width=w,height=h,
                surfaces=len(surfaces),logical_surface_bytes=logical_bytes,palettes=len(palettes),
                creates=created,deletes=destroyed,created_bytes=made_bytes,deleted_bytes=freed_bytes,
                palette_creates=palette_created,palette_deletes=palette_destroyed))
        if op==8:require(not surfaces and not palettes,'Resources survive command END')
    require(len(worlds)>=3,'Insufficient bounded source forward progress')
    return dict(verified=True,worlds=worlds,creates=created,deletes=destroyed,
        created_bytes=made_bytes,deleted_bytes=freed_bytes,palette_creates=palette_created,
        palette_deletes=palette_destroyed,peak_surfaces=peak_surfaces,
        peak_logical_surface_bytes=peak_bytes,peak_palettes=peak_palettes,
        remaining_surfaces=len(surfaces),remaining_palettes=len(palettes),
        scope='Every complete command generation accounts live surface/palette identities and logical plane bytes from actual CREATE/DELETE operations. Independent native Quit counts cover final graphics owners. No physical VRAM measurement or per-generation ownership of every external API object is inferred.')


def close_rows(plan,runs):
    validate(plan)
    expected={(c['id'],tuple(size)) for c in plan['completion_contract']['configurations'] for size in c['frame_sizes']}
    require(expected=={(c,s) for c in ('player-default','player-software') for s in ((640,480),(800,600))},'Changed sustained target domain')
    domains=set();identities=set()
    for run in runs:
        require(run['domain'] in expected and run['domain'] not in domains and run['identity'] not in identities,
                'Duplicate or unsupported sustained execution')
        require(run['verified'] is True,'Unverified sustained execution')
        domains.add(run['domain']);identities.add(run['identity'])
    require(domains==expected,'Missing actual sustained profile/size domain')
    rows=[]
    for row in requirements(plan):
        proof=[dict(reason=SUSTAINED_LEAVES[row['id']],execution=r['pair'],actual_world_extents=[list(r['domain'][1])],provider=r['provider'])
               for r in runs if r['domain'][0]==row['configuration']] if row['id'] in SUSTAINED_LEAVES else []
        rows.append(dict(row,state='passed' if proof else 'pending',admitted_execution_evidence=proof))
    require(len(rows)==156 and sum(r['state']=='passed' for r in rows)==6,'Sustained closeout scope changed')
    return rows



def presented_world_interval(observed,compared):
    ids=[p for p in observed['swaps'] if int(observed['paints'][p][0]['world'])>0]
    matches=[r for r in compared['comparisons'] if r['world']>0]
    require(len(matches)==len(ids) and Counter(r['paint'] for r in matches)==Counter(ids)
            and all(int(observed['paints'][r['paint']][0]['world'])==r['world'] for r in matches),
            'Presented output does not cover every actual World swap occurrence')
    return [observed['paints'][p][0] for p in ids]


def sustained_progress(plan,reader,pairs):
    runs=[];sources={}
    require(len(pairs)==4,'Sustained closeout requires four actual domains')
    for pair in pairs:
        a=receipt(reader,pair['acquisition']);c=receipt(reader,pair['comparison'])
        require(type(a['configuration']['software_flag']) is bool,'Missing explicit target flag')
        configuration='player-software' if a['configuration']['software_flag'] else 'player-default'
        admitted=sustained_admit(reader,pair,configuration)
        require(a['scenario']=='normal-quick' and a['flow']['world_visits']==2,'Sustained pack needs two normal World visits')
        require(len(admitted['world_extents'])==1,'Ambiguous sustained World size')
        streams=[]
        for artifact in a['artifacts']:
            if '/capture/' not in artifact['path'] or not artifact['path'].endswith('.bin'):continue
            body=reader.read_bytes(artifact['record'])
            require(hashlib.sha256(body).hexdigest()==artifact['sha256'],'Changed sustained capture artifact')
            streams.append(body)
        observed=acquisition(streams,a['session_identity'])
        require(hashlib.sha256(observed['commands']).hexdigest()==c['captured_command_stream_sha256'],
                'Resource ledger does not bind compared commands')
        resources=resource_history(observed['commands'])
        domain=[dict(generation=r['generation'],width=r['width'],height=r['height']) for r in resources['worlds']]
        require(domain==c['world_domain']['worlds'] and len(domain)==c['original_reference']['worlds']
                ==int(a['resident']['completed'])==int(a['resident']['verified_returns']),
                'Independent original history omits a resource generation')
        world_paints=presented_world_interval(observed,c['comparison'])
        shown={int(r['world']) for r in world_paints}
        require(len(shown)>=3 and shown<=set(range(1,len(domain)+1)),'Repeated paints cannot replace new source progress')
        times=[int(r['host_us']) for r in world_paints]
        require(times and max(times)>min(times),'No bounded presented World interval')
        for n,h in admitted['sources'].items():
            require(n not in sources or sources[n]==h,'Mixed sustained renderer/comparison source phases');sources[n]=h
        runs.append(dict(verified=True,identity=a['session_identity'],domain=(configuration,tuple(admitted['world_extents'][0])),
            pair=pair,provider=admitted['provider'],source_generations=len(domain),presented_generations=len(shown),
            world_presentations=len(world_paints),pixels=admitted['pixels'],resources=resources,
            normal_retirement=a['retirement'],diagnostic_presented_span_us=max(times)-min(times)))
    require(all(reader.digest(n)==h for n,h in sources.items()),'Changed sustained renderer/comparison sources')
    rows=close_rows(plan,runs)
    return dict(schema=1,kind='native_rendering_sustained_world_closeout',success=True,
        dossier_identity=fingerprint(requirements(plan)),sources=sources,executions=runs,requirements=rows,
        passed_requirements=6,pending_requirements=150,closed_leaf_ids=sorted(SUSTAINED_LEAVES),
        new_sustained_rows=2,renewed_setup_quit_rows=4,
        decisions={'supported-player':'pending','original-recovery':'pending'},historical_failures_retired=False,
        scope='Close bounded sustained World, setup-confirm and menu-exit leaves for both explicit targets at both actual sizes using existing normal collectors. Source/render progress is independent of the separate animation/simulation/visibility requirements. Six current rows in this pack; other historical packs are not silently unioned or refreshed. No whole rendering decision follows.')


CLEANUP_LEAVES={'journey.shutdown.cleanup':'Final native cursor, input, graphics, original process, command/control channel and source jobs are quiescent under normal original Quit.'}
STORAGE_LEAVES={
    'journey.save-load.save':'Original Save acknowledgement and exact owned persisted bytes.',
    'journey.save-load.load':'Original Mini/Main Load dispatch, fresh generations and both display rebuilds.',
    'journey.save-load.restored-world':'Every presented restored World occurrence matches independent complete original history.',
    'journey.pause-preferences.cancel':'An actual changed draft Cancel restores original values and profile bytes.'}
CLEANUP_SOURCES=('tools/native_cursor_journal.py','protocols/include/mnm/input_v1.h',
    'protocols/include/mnm/render_commands_v3.h','protocols/include/mnm/render_control_v1.h')


def retained_artifact(reader,a,suffix):
    files=[v for v in a['artifacts'] if v['path'].endswith('/'+suffix)]
    require(len(files)==1,'Missing or ambiguous retained cleanup artifact: '+suffix)
    v=files[0];body=reader.read_bytes(v['record'])
    require(len(body)==v['bytes'] and hashlib.sha256(body).hexdigest()==v['sha256'],
            'Retained artifact bytes differ: '+suffix)
    return body


def cleanup_state(a,shell,admission,process,input_bytes,ring_bytes,control_bytes,cursor_bytes,graphics_bytes):
    """Terminal wire owners and real journals; no process-exit-only inference."""
    from native_cursor_journal import decode
    policy='native-only-rendering-v1'
    require(shell.get('schema')==1 and shell.get('policy')==policy and shell.get('session')==a['session']
            and admission.get('session')==a['session'] and admission.get('native_only',{}).get('policy')==policy,
            'Cleanup session/policy identity differs')
    require(shell.get('state')=='completed' and shell.get('normal_quit') is True
            and shell.get('renderer_retired') is True and shell.get('fallback') is False
            and not shell.get('error') and type(shell.get('launcher_exit')) is int and shell['launcher_exit']==0
            and shell.get('restart_requested') is False and shell.get('force_stop_verified') is False,
            'Cleanup needs normal shell/renderer retirement')
    require(type(process.get('original_exit_code')) is int and process['original_exit_code']==0
            and process.get('cleanup_after_original_exit') is True and process.get('prefix')==admission.get('prefix')
            and a.get('private_wine_stopped') is True,'Original/private process cleanup incomplete')
    require(len(input_bytes)==1088 and input_bytes[:8]==b'MNMINK01','Invalid terminal input extent')
    keys=struct.unpack('<272I',input_bytes)
    require(keys[2:4]==(1,1088) and keys[4]>0 and keys[4]%2==0 and keys[5]==0
            and not any(v&0x80000000 for v in keys[16:]),'Active or held terminal input')
    require(len(ring_bytes)==1048640 and ring_bytes[:8]==b'MNMRDC03','Invalid terminal command channel')
    ring=struct.unpack_from('<16I',ring_bytes)
    require(ring[2:4]==(3,1048640) and ring[4]>0 and ring[5]>0 and ring[5]==ring[9]
            and ring[6:9]==(2,0,1) and not any(ring[10:]),'Command channel is not drained/ended')
    require(len(control_bytes)==576 and control_bytes[:8]==b'MNMRCV01','Invalid terminal control channel')
    control=struct.unpack_from('<16I',control_bytes)
    require(control[2:5]==(1,576,ring[4]) and control[5]>0 and control[5]==control[8]
            and control[6:8]==(3,0) and control[9:13]==(1,1,0,2) and not any(control[13:])
            and not any(control_bytes[64:]),'Control STOP is pending, refused or belongs to another generation')
    cursor=decode(cursor_bytes)
    require(cursor['closed'] is True and cursor['active_clients']==cursor['pending_jobs']==0
            and cursor['queue_accepted']==cursor['queue_completed']+cursor['queue_cancelled'],
            'Cursor owners/jobs survive')
    require(len(graphics_bytes)==64,'Surviving graphics inventory')
    graphics=struct.unpack('<16I',graphics_bytes)
    require(graphics[:5]==(0x444d4e4d,0x31544552,1,64,1) and not any(graphics[5:11])
            and graphics[11]>0 and graphics[12]>0 and not any(graphics[13:]),'Graphics owners survive normal Quit')
    resident=a['resident']
    require(resident['pending'] is False and resident['normal_quit_retired'] is True
            and int(resident['imports'])==int(resident['completed'])==int(resident['verified_returns'])>0,
            'Source work survives or omits a return')
    return dict(verified=True,cursor=cursor,input_active=0,held_keys=0,
        command_session=ring[4],published=ring[5],acknowledged=ring[9],command_ended=True,
        stop_requested=control[5],stop_acknowledged=control[8],producer_stopped=True,
        source_imports=int(resident['imports']),source_returns=int(resident['verified_returns']),
        pending_jobs=0,graphics_owners=0,original_exit=0,private_wine_stopped=True,
        renderer_retired=True,normal_quit=True,
        scope='Terminal native protocol owners, input, cursor lifetime journal, pre-reclamation graphics inventory and balanced source returns. Cursor refusal counts and unexercised deferred-job branches remain visible; no fault/restart or OS-wide handle inventory claim.')


def cleanup_proof(reader,a):
    values=[json.loads(retained_artifact(reader,a,n)) for n in
            ('native-only-session.json','native-output-admission.json','original-process-result.json')]
    raw=[retained_artifact(reader,a,n) for n in ('render-input.bin','render-commands.bin',
        'render-commands.bin.control','native-cursor-lifetime.bin','native-draw-retirement.bin')]
    return cleanup_state(a,*values,*raw)


def cleanup_storage_rows(plan,quick,storage):
    prior=close_rows(plan,quick);close_rows(plan,storage) # same strict four unique actual domains
    require(not ({r['identity'] for r in quick}&{r['identity'] for r in storage}),
            'Storage and Quick reuse an actual session')
    leaves={**SUSTAINED_LEAVES,**CLEANUP_LEAVES,**STORAGE_LEAVES}
    for row in prior:
        runs=storage if row['id'] in STORAGE_LEAVES else quick
        if row['id'] in CLEANUP_LEAVES:
            require(all(r.get('cleanup',{}).get('verified') is True for r in runs),'Unverified cleanup domain')
        if row['id'] in CLEANUP_LEAVES or row['id'] in STORAGE_LEAVES:
            row.update(state='passed',admitted_execution_evidence=[dict(reason=leaves[row['id']],
                execution=r['pair'],actual_world_extents=[list(r['domain'][1])],provider=r['provider'])
                for r in runs if r['domain'][0]==row['configuration']])
    require(len(prior)==156 and sum(r['state']=='passed' for r in prior)==16,'Cleanup/storage scope changed')
    return prior


def cleanup_storage_progress(plan,reader,pairs):
    require(len(pairs)==8,'Cleanup/storage closeout requires four Quick and four storage executions')
    quick=[p for p in pairs if receipt(reader,p['acquisition'])['scenario']=='normal-quick']
    storage=[p for p in pairs if receipt(reader,p['acquisition'])['scenario']=='normal-storage']
    prior=sustained_progress(plan,reader,quick);sources=dict(prior['sources']);runs=[]
    for run in prior['executions']:
        run['cleanup']=cleanup_proof(reader,receipt(reader,run['pair']['acquisition']))
    for pair in storage:
        a=receipt(reader,pair['acquisition']);c=receipt(reader,pair['comparison'])
        require(type(a['configuration']['software_flag']) is bool,'Missing explicit storage target flag')
        configuration='player-software' if a['configuration']['software_flag'] else 'player-default'
        admitted=sustained_admit(reader,pair,configuration)
        require(len(admitted['world_extents'])==1,'Ambiguous storage World size')
        streams=[retained_artifact(reader,a,v['path'].split('/capture/')[-1]) for v in a['artifacts']
                 if '/capture/' in v['path'] and v['path'].endswith('.bin')]
        observed=acquisition(streams,a['session_identity'])
        require(hashlib.sha256(observed['commands']).hexdigest()==c['captured_command_stream_sha256'],
                'Storage resource ledger differs from independent compared commands')
        resources=resource_history(observed['commands'])
        domain=[dict(generation=r['generation'],width=r['width'],height=r['height']) for r in resources['worlds']]
        require(domain==c['world_domain']['worlds'] and len(domain)==c['original_reference']['worlds']
                ==int(a['resident']['completed'])==int(a['resident']['verified_returns']),
                'Storage source/resource generation omitted')
        paints=presented_world_interval(observed,c['comparison'])
        from storage_cpu_read_binding import join_reads
        captured_commands=[v for v in a['artifacts'] if '/capture/commands-' in v['path']]
        require(len(captured_commands)==1,'Ambiguous storage CPU command capture')
        delivery=join_reads(reader.read_bytes(captured_commands[0]['record']),a['session_identity'],
            retained_artifact(reader,a,'game/native-cpu-read-counts.bin'),c['comparison']['cpu_reads'],
            retained_artifact(reader,a,'lock-capture/lifecycle.log'),retained_artifact(reader,a,'render-commands.bin.cpu-reply'))
        require(delivery==c['cpu_delivery'] and delivery['request_count']>0,
                'Storage CPU requests/replies differ from independent comparison')
        for n,h in admitted['sources'].items():
            require(n not in sources or sources[n]==h,'Mixed current collector source phases');sources[n]=h
        runs.append(dict(verified=True,identity=a['session_identity'],domain=(configuration,tuple(admitted['world_extents'][0])),
            pair=pair,provider=admitted['provider'],source_generations=len(domain),
            world_presentations=len(paints),pixels=admitted['pixels'],resources=resources,
            storage_actions=c['original_menu_actions'],preferences=c['preferences_boundary'],cpu_delivery=c['cpu_delivery'],
            cleanup=cleanup_proof(reader,a)))
    sources.update({n:reader.digest(n) for n in CLEANUP_SOURCES})
    require(all(reader.digest(n)==h for n,h in sources.items()),'Changed current cleanup/storage sources')
    rows=cleanup_storage_rows(plan,prior['executions'],runs)
    return dict(schema=1,kind='native_rendering_cleanup_storage_closeout',success=True,
        dossier_identity=fingerprint(requirements(plan)),sources=sources,
        executions=prior['executions']+runs,requirements=rows,passed_requirements=16,pending_requirements=140,
        closed_leaf_ids=sorted({**SUSTAINED_LEAVES,**CLEANUP_LEAVES,**STORAGE_LEAVES}),
        new_cleanup_rows=2,new_storage_rows=8,renewed_sustained_setup_quit_rows=6,
        decisions={'supported-player':'pending','original-recovery':'pending'},historical_failures_retired=False,
        scope='Eight named leaves across both explicit targets and both actual sizes, using existing collectors. Four compatible Quick pairs are re-evaluated and four storage pairs exercise Save, Mini/Main Load, restored output and changed-draft Cancel. Terminal input/cursor/graphics/process/channel/source owners are checked directly. Other140rows and both final decisions remain pending; historical failures and unexecuted branches retain their scope.')


LIGHTING_MENU_LEAVES={
    'journey.creatures-effects.lighting':'Selected native lighting processes and applies every observed request without fallback/refusal, binds its final reply to the session/request/input, and retires its worker; complete retained World output is independently compared.',
    'journey.campaign-entry.campaign-selection':'Original initial campaign/difficulty/region confirmation on the actual current profile and World size.',
    'journey.pause-preferences.preferences-change':'Original Preferences OK callbacks and both ordered display rebuild/rebind paths; resolution setting only.'}


def lighting_checksum(raw):
    value=2166136261
    for i,byte in enumerate(raw):
        value=((value ^ (0 if 120<=i<124 else byte))*16777619)&0xffffffff
    return value


def lighting_state(policy,result,outcome,channel):
    """Join retained worker SQLite outcome to its exact terminal wire exchange."""
    import sqlite3
    require(len(outcome)<=1024*1024 and outcome.startswith(b'SQLite format 3\0'),'Invalid retained lighting outcome')
    db=sqlite3.connect(':memory:')
    try:
        db.deserialize(outcome);db.execute('PRAGMA query_only=ON')
        rows=db.execute('SELECT payload FROM results').fetchall()
        life=db.execute('SELECT pid,start_ticks,phase FROM lifecycle').fetchall()
        require(len(rows)==1 and json.loads(rows[0][0])==result and len(life)==1
                and life[0][0]>0 and life[0][1]>0 and life[0][2]=='started','Unbound/ambiguous lighting worker outcome')
    finally:db.close()
    cap=8*1024*1024
    require(len(channel)==128+3*cap and channel[:8]==b'MNMLS001','Invalid lighting channel extent/magic')
    w=struct.unpack_from('<32I',channel)
    require(w[2:5]==(1,cap,6) and w[11:14]==(1,0,0) and w[31]==0
            and any(channel[36:44]) and channel[36:44].hex()==policy['nonce']
            and policy['mode']=='native' and policy['capture']=='none','Lighting session identity/mode/terminal phase differs')
    require(result['success'] is True and not result['error'] and result['source_bound'] is True
            and result['mode']=='native' and result['worker_exit']==0 and result['worker_exited_early'] is False,
            'Lighting worker did not complete normal retirement')
    stats=dict(zip(('calls','applied','fallback','refused','disabled','processed','rejected','compared','mismatches',
                   'max_wait_ms','max_compute_us','last_caller','last_clock','last_changed','error','records','trace_active'),w[14:31]))
    require(stats==result['statistics'] and w[8]==stats['calls']==stats['applied']==stats['processed']>0
            and all(stats[k]==0 for k in ('fallback','refused','disabled','rejected','error','compared','mismatches','records','trace_active')),
            'Lighting did not process/apply every call or entered a diagnostic/failure branch')
    require(300<=w[5]<=cap and 80<=w[6]<=cap and w[7]==0,'Lighting input/reply extent differs')
    inp=channel[128:128+w[5]];reply=channel[128+cap:128+cap+w[6]]
    u=lambda at:struct.unpack_from('<I',inp,at)[0]
    require(inp[:8]==b'MNMLIT01' and (u(8),u(12),u(16),u(20))==(1,128,len(inp),w[8])
            and inp[24:32]==channel[36:44] and (u(112),u(116),u(124))==(1,0,0)
            and u(120)==lighting_checksum(inp) and (u(104),u(108))==(stats['last_clock'],stats['last_caller']),
            'Final lighting input identity/checksum/caller differs')
    width,height,layers,count=u(32),u(36),u(40),u(44)
    require(1<=width<=128 and 1<=height<=128 and 1<=layers<=32 and count==width*height*((layers+1)//2)
            and u(76)<=65536 and u(80)<=65536 and u(84)<=65536 and u(88)<=u(84)
            and u(68)<=1 and u(72)<=1 and len(inp)==300+u(76)*28+u(84)*32+count*5,
            'Final lighting input geometry/source bounds differ')
    require(reply[:8]==b'MNMLRP01' and struct.unpack_from('<4I',reply,8)==(1,len(reply),w[8],w[9])
            and struct.unpack_from('<2I',reply,24)==(w[10],u(120)) and len(reply)==80+count*5
            and struct.unpack_from('<I',reply,32)[0]==0,'Final lighting reply identity/input binding differs')
    state=struct.unpack_from('<11I',reply,36)
    require(state[0]<8 and state[3]<=65536 and state[4]<=1 and state[5]<=1 and state[8]<=65536
            and state[9]<=1 and state[10]<=1,'Final lighting reply state bounds differ')
    return dict(verified=True,statistics=stats,sequence=w[8],nonce=policy['nonce'],terminal_phase='closed',
        input_sha256=hashlib.sha256(inp).hexdigest(),reply_sha256=hashlib.sha256(reply).hexdigest(),
        caller=stats['last_caller'],clock=stats['last_clock'],worker_exit=0,
        scope='Complete observed request/process/application counts, source-bound final wire exchange and normal worker retirement. Zero failures observed; no injected failure, changing-light scene or original lighting-call equivalence is inferred.')


def lighting_menu_rows(plan,parent,runs):
    validate(plan)
    expected={(c,tuple(z)) for c in ('player-default','player-software') for z in ((640,480),(800,600))}
    require(len(runs)==4 and {tuple((r['domain'][0],tuple(r['domain'][1]))) for r in runs}==expected
            and len({r['identity'] for r in runs})==4 and all(r['verified'] is True for r in runs),
            'Lighting/menu pack lacks distinct verified target domains')
    template=requirements(plan)
    require(parent['dossier_identity']==fingerprint(template) and len(parent['requirements'])==156
            and [(r['configuration'],r['id']) for r in parent['requirements']]==[(r['configuration'],r['id']) for r in template]
            and sum(r['state']=='passed' for r in parent['requirements'])==parent['passed_requirements']==16
            and {r['id'] for r in parent['requirements'] if r['state']=='passed'}==set(SUSTAINED_LEAVES)|set(CLEANUP_LEAVES)|set(STORAGE_LEAVES),
            'Accepted parent requirement scope differs')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id'] in LIGHTING_MENU_LEAVES:
            row.update(state='passed',admitted_execution_evidence=[dict(reason=LIGHTING_MENU_LEAVES[row['id']],
                execution=r['pair'],actual_world_extents=[list(r['domain'][1])],provider=r['provider'])
                for r in runs if r['domain'][0]==row['configuration']])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==22,'Lighting/menu scope changed')
    return rows



def lighting_menu_actions(menu,size):
    selection=menu['campaign_selection'];display=menu['display_rebuilds']
    require(selection['verified'] is True and selection['callbacks']==[[3,0],[18,65537],[18,0]]
            and type(selection['thread']) is int and selection['thread']>0,'Unproved initial campaign selection')
    require(tuple(size) in ((800,600),(640,480)),'Unknown actual display size')
    order=[[640,480],[800,600]] if tuple(size)==(800,600) else [[800,600],[640,480]]
    require(display['verified'] is True and display['dimensions']==order and len(display['callback_sequences'])==2
            and len(set(display['callback_sequences']))==2 and all(type(n) is int and n>0 for n in display['callback_sequences']),
            'Both actual original Preferences rebuilds are required')
    return selection,display


def lighting_menu_progress(plan,reader,parent_evidence):
    """Consume an applicable accepted parent; analyze retained data, not recollect."""
    import subprocess,sys
    root=reader.root
    command=[sys.executable,str(root/'tools/evidence-applicability.py'),'plan','--root',str(root),
             '--evidence',parent_evidence,'--behavior','NP.cleanup-storage-closeout','--require-applicable']
    checked=subprocess.run(command,cwd=root,capture_output=True,text=True)
    require(checked.returncode==0,'Accepted cleanup parent is impacted: '+checked.stderr)
    impact=json.loads(checked.stdout)
    require(impact['current_scope_preserved'] is True and not impact['source_actions'],'Parent needs targeted revalidation')
    binding=reader.json(impact['retained_input']['record'])
    require(reader.digest(impact['retained_input']['record'])==impact['retained_input']['sha256']
            and binding['success'] is True and binding['independent_pixel_equivalence'] is True,'Changed accepted parent binding')
    parent_reference=binding['inputs']['closeout'];parent=receipt(reader,parent_reference)
    require(parent['success'] is True and parent['decisions']=={'supported-player':'pending','original-recovery':'pending'},'Invalid parent decision scope')
    sources={n:reader.digest(n) for n in binding['sources']};runs=[]
    for previous in parent['executions']:
        if 'storage_actions' not in previous:continue
        pair=previous['pair'];a=receipt(reader,pair['acquisition']);c=receipt(reader,pair['comparison'])
        require(a['scenario']=='normal-storage' and c['acquisition']==pair['acquisition'] and c['success'] is True
                and c['original_menu_actions']['verified'] is True and c['lighting']['verified'] is True,'Invalid retained storage/lighting facts')
        cfg=a['configuration'];domain=(('player-software' if cfg['software_flag'] else 'player-default'),tuple(c['world_domain']['extents'][0]))
        require(domain==(previous['domain'][0],tuple(previous['domain'][1])) and type(cfg['software_flag']) is bool
                and cfg['terrain_lighting']=='native' and cfg['native_lighting_capture'] is False,'Lighting target differs from admitted parent')
        lighting=c['lighting'];policy=lighting['policy']
        require(all(reader.digest(n)==h for n,h in policy['sources'].items()),'Lighting runtime/compiler source changed')
        for suffix,sha in [('game/MnmLight.dll',policy['dll_sha256']),('lighting-build/mnm-terrain-light-session',policy['worker_sha256'])]:
            raw=retained_artifact(reader,a,suffix);require(hashlib.sha256(raw).hexdigest()==sha,'Lighting loaded binary differs')
        outcome=retained_artifact(reader,a,'terrain-light-session/outcome.sqlite3')
        require(hashlib.sha256(outcome).hexdigest()==lighting['record']['sha256'],'Lighting outcome differs from comparison')
        proof=lighting_state(policy,lighting['result'],outcome,retained_artifact(reader,a,'terrain-light-session/channel.bin'))
        selection,display=lighting_menu_actions(c['original_menu_actions'],domain[1])
        runs.append(dict(verified=True,identity=a['session_identity'],domain=domain,pair=pair,provider=previous['provider'],
            lighting=proof,lighting_outcome=lighting['record'],campaign_selection=selection,display_rebuilds=display))
    rows=lighting_menu_rows(plan,parent,runs)
    sources.update({n:reader.digest(n) for n in ('tools/evidence-applicability.py','tools/coverage_applicability.py','tools/coverage_evidence.py',
        'tests/test-rendering-normal-acceptance.py','protocols/include/mnm/terrain_light_session_v1.h','protocols/include/mnm/terrain_light_v1.h','tools/terrain_light_session.py')})
    return dict(schema=1,kind='native_rendering_lighting_menu_closeout',success=True,sources=sources,
        accepted_parent=dict(evidence=parent_evidence,binding=impact['retained_input'],applicability=impact),parent_closeout=parent_reference,
        dossier_identity=fingerprint(requirements(plan)),requirements=rows,executions=runs,passed_requirements=22,pending_requirements=134,
        inherited_current_rows=16,renewed_previously_accepted_selection_preferences_rows=4,new_lighting_rows=2,
        closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|set(LIGHTING_MENU_LEAVES)),
        lighting_calls=sum(r['lighting']['statistics']['calls'] for r in runs),original_game_runs=0,
        decisions={'supported-player':'pending','original-recovery':'pending'},historical_failures_retired=False,
        scope='Preserve the accepted sixteen cleanup/storage/sustained/setup/Quit rows; renew four bounded initial-campaign/resolution rows and close two selected native-lighting leaves. Both explicit targets at both actual sizes, using retained channels/outcomes and independently compared World captures. Lighting failure injection, changing-light scenes, original lighting-call equivalence, other134rows and both final decisions remain separate.')


RESUME_PRODUCERS={
 'apps/qt-shell/campaign_storage_test.cpp':'ca6dfae748bb5b0d59dedc986b985fac2b5689c5001b5829d946a66a0922a658',
 'apps/qt-shell/campaign_smoke_test.hpp':'15bb571a2d5374746c29a0adc0528e030e3374ef3937ecac72c79817adb4be26',
 'apps/qt-shell/main.cpp':'310afc28328ffaa0cb3810cb10e4b9e84b5ebcaedf93c8b4cd3171b91c01859d',
 'tools/menu-storage-input.py':'6fc4434d85949f67f62c754b7b811f48a7cc585cd86e8b44134745523b9a9f52'}
RESUME_LEAF='journey.pause-preferences.resume'


def preferences_resume_state(flow,records,compared,resources,size):
    """The frozen producer admits each input only after fresh source/output progress.

    Cancel baselines were not serialized. Reaching its stage21/23 input branch
    proves the reviewed active/+11frames/+3sources/6s predicate, not a measured
    numeric baseline. Raw physical edges and original ready-state ACKs supply
    independent effect; matching presented generations supply complete output.
    """
    require(flow['success'] is True and flow['normal_quit'] is True and flow['resident_world_requested'] is True and flow['preferences_cancel_verified'] is True
            and flow['display_rebuilds']==2 and flow['reloaded_from_main'] is True
            and flow['native_command_fallback'] is False and not flow['native_error'] and flow['native_recoveries']==0,
            'Incomplete native Preferences continuation')
    states=flow['states'];ready=[s for s in states if 'event' not in s]
    threads={s['thread'] for s in ready};require(len(threads)==1 and next(iter(threads))>0,'Changed original menu thread')
    cancels=[i for i,s in enumerate(states) if s.get('event')=='click' and s.get('action')=='preferencesCancel']
    applies=[i for i,s in enumerate(states) if s.get('event')=='click' and s.get('action')=='preferencesOk']
    require([states[i]['stage'] for i in cancels]==[21,23] and [states[i]['stage'] for i in applies]==[15,17]
            and cancels[-1]<applies[0]<applies[-1],'Missing Cancel/Apply sequence')
    checkpoints={v['phase']:v for v in flow['checkpoints']};cp=checkpoints['after-recreate-load']
    require(cp['native_active'] is True and cp['fallback'] is False and not cp['error']
            and counter(cp['source_generation'])>=counter(cp['source_base'])+3 and cp['frames']>cp['frame_base']+10,
            'Apply/reload did not import fresh native Worlds')
    proofs=[]
    for phase,start,expected_stage in [(21,cancels[0],22),(23,cancels[1],4),(9,applies[-1],10)]:
        action=states[start]
        require(action.get('injected_capture_delay_ms')==0 and action['time_ms']<=action['capture_begin_ms']<=action['capture_end_ms'],
                'Diagnostic/malformed action capture')
        end=next((i for i in range(start+1,len(states)) if 'event' not in states[i] and states[i]['screen']==17),None)
        require(end is not None and states[end]['stage']==expected_stage,'No original Mini return after resumed physical input')
        destination=states[end]
        selected=[(i,s) for i,s in enumerate(states[start+1:end],start+1) if s.get('event')=='mini-input' and s['stage']==phase]
        require(selected,'Missing post-action input marker');_,marker=selected[-1];n=counter(marker['source_generation'])
        record=records[marker['record']]
        require(record['success'] is True and record['mode']=='mini' and record['skip_click'] is True
                and record['input_active_checked'] is True and record['escape_sent'] is True
                and record['viewport_pixels']==list(size) and not record['tutorial_actions'],
                'Resume input was missing, inactive or mixed with a scene click')
        edges=record['events'];require([v['event'] for v in edges]==['escape','escape','complete']
            and [v['down'] for v in edges[:2]]==[True,False] and action['capture_end_ms']<=edges[0]['time_ms']<edges[1]['time_ms']<=edges[2]['time_ms'],
            'Physical Escape ordering differs')
        words=[v['input_words'] for v in edges];sequences=[v['4'] for v in words]
        require(words[0]['5']==1 and all(type(v) is int and v>0 and v%2==0 for v in sequences)
                and sequences==sorted(sequences),'Inactive/unstable resumed native input')
        if phase in (21,23):
            opening=next((s for s in reversed(states[:start]) if 'event' not in s),None)
            require(opening and opening['screen']==10 and opening['stage']==phase and destination['ack']==opening['ack']+1
                    and destination['generation']>opening['generation'],'Preferences Cancel original ACK/generation differs')
            baseline=None
        else:
            loads=[s for s in states[start+1:end] if 'event' not in s and s['screen']==12 and s['stage']==8]
            require(len(loads)==1 and destination['ack']==loads[0]['ack']+1 and destination['generation']>loads[0]['generation']
                    and n>=counter(cp['source_generation']),'Main Apply/Load ACK or fresh input source differs')
            baseline=counter(cp['source_base'])
        matches=[r for r in compared if n<=r['world']<=n+2]
        require({r['world'] for r in matches}=={n,n+1,n+2} and all(r['pixels']>0 and r['mismatching_pixels']==0
                and r['actual_sha256']==r['expected_sha256'] for r in matches),'Missing three fresh independently compared presented Worlds')
        conserved=[]
        for generation in (n,n+1,n+2):
            require(0<generation<=len(resources),'Resumed resource generation missing');r=resources[generation-1]
            require(r['generation']==generation and (r['width'],r['height'])==tuple(size)
                    and r['creates']-r['deletes']==r['surfaces']>0
                    and r['created_bytes']-r['deleted_bytes']==r['logical_surface_bytes']>0
                    and r['palette_creates']-r['palette_deletes']==r['palettes']>=0,'Resumed resource identity/conservation differs')
            conserved.append(r)
        proofs.append(dict(phase='cancel-'+str(phase) if phase!=9 else 'main-apply-reload',verified=True,
            input_record=marker['record'],source_marker=n,serialized_source_baseline=baseline,
            freshness_basis='Frozen actual producer requires active native presentation, >10 new frames, >=3 new source imports and >=6s dwell before this physical input. Cancel numeric baselines were not serialized.',
            original_destination=destination,physical_escape=edges,compared_presentations=matches,resources=conserved))
    return dict(verified=True,returns=proofs,presentations=sum(len(v['compared_presentations']) for v in proofs),
        pixels=sum(r['pixels'] for p in proofs for r in p['compared_presentations']),
        scope='Both Mini Cancels and supported Main resolution Apply/rebuild/Load return to fresh native output and actual original Mini via physical Escape. No direct Mini Apply, pause timing or exact wall-clock/paint simultaneity is inferred.')


def preferences_resume_rows(plan,parent,runs):
    validate(plan);expected={(c,tuple(z)) for c in ('player-default','player-software') for z in ((640,480),(800,600))}
    require(len(runs)==4 and {(r['domain'][0],tuple(r['domain'][1])) for r in runs}==expected
            and len({r['identity'] for r in runs})==4 and all(r['resume']['verified'] is True for r in runs),'Missing/duplicate resume domain')
    template=requirements(plan)
    require(parent['dossier_identity']==fingerprint(template) and parent['passed_requirements']==22
            and len(parent['requirements'])==156 and sum(v['state']=='passed' for v in parent['requirements'])==22
            and {v['id'] for v in parent['requirements'] if v['state']=='passed'}==set(SUSTAINED_LEAVES)|set(CLEANUP_LEAVES)|set(STORAGE_LEAVES)|set(LIGHTING_MENU_LEAVES)
            and [(r['configuration'],r['id']) for r in parent['requirements']]==[(r['configuration'],r['id']) for r in template],
            'Changed accepted parent inventory')
    rows=[]
    for before in parent['requirements']:
        row=dict(before)
        if row['id']==RESUME_LEAF:
            require(row['state']=='pending','Resume was already closed in this parent')
            row.update(state='passed',admitted_execution_evidence=[dict(reason='Physical original Mini admission follows fresh source/output after both Cancel and supported Main Apply/reload, with lighting/history/resource checks.',execution=r['pair'],actual_world_extents=[list(r['domain'][1])],provider=r['provider']) for r in runs if r['domain'][0]==row['configuration']])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==24,'Resume closeout scope changed');return rows


def preferences_resume_progress(plan,reader,parent_evidence):
    import subprocess,sys
    root=reader.root
    checked=subprocess.run([sys.executable,str(root/'tools/evidence-applicability.py'),'plan','--root',str(root),'--evidence',parent_evidence,
        '--behavior','NP.normal-lighting-menu-closeout','--require-applicable'],cwd=root,capture_output=True,text=True)
    require(checked.returncode==0,'Lighting/menu parent is impacted: '+checked.stderr);impact=json.loads(checked.stdout)
    require(impact['current_scope_preserved'] is True and not impact['source_actions'],'Parent needs targeted revalidation')
    reference=impact['retained_input'];binding=reader.json(reference['record']);require(reader.digest(reference['record'])==reference['sha256'] and binding['success'] is True and binding['independent_pixel_equivalence'] is True,'Invalid accepted parent')
    parent=receipt(reader,binding['inputs']['progress']);storage=receipt(reader,parent['parent_closeout']);by_identity={r['identity']:r for r in storage['executions']};runs=[]
    for previous in parent['executions']:
        pair=previous['pair'];a=receipt(reader,pair['acquisition']);c=receipt(reader,pair['comparison']);old=by_identity[previous['identity']]
        require(a['session_identity']==old['identity'] and a['scenario']=='normal-storage' and c['success'] is True
                and c['acquisition']==pair['acquisition'] and c['mini_inputs']['verified'] is True and c['mini_dispatch']['verified'] is True,
                'Missing original acknowledged storage/input context')
        for name,sha in RESUME_PRODUCERS.items():
            require(reader.digest(name)==sha and a['sources'][name]==sha,'Reviewed resume producer changed: '+name)
        records={}
        checked_inputs={v['path']:v for v in c['mini_inputs']['records']}
        for marker in a['flow']['states']:
            if marker.get('event')!='mini-input':continue
            name=marker['record'];raw=retained_artifact(reader,a,name)
            require(name in checked_inputs and hashlib.sha256(raw).hexdigest()==checked_inputs[name]['sha256'],'Unbound physical input record')
            records[name]=json.loads(raw)
        proof=preferences_resume_state(a['flow'],records,c['comparison']['comparisons'],old['resources']['worlds'],previous['domain'][1])
        require(previous['lighting']['verified'] is True and all(previous['lighting']['statistics'][k]==0 for k in ('fallback','refused','disabled','error','rejected')),'Lighting failed across resumed session')
        runs.append(dict(identity=old['identity'],domain=old['domain'],pair=pair,provider=old['provider'],resume=proof,lighting=previous['lighting']))
    rows=preferences_resume_rows(plan,parent,runs);sources={n:reader.digest(n) for n in binding['sources']}
    return dict(schema=1,kind='native_rendering_preferences_resume_closeout',success=True,sources=sources,
        accepted_parent=dict(evidence=parent_evidence,binding=reference,applicability=impact),parent_progress=binding['inputs']['progress'],
        dossier_identity=fingerprint(requirements(plan)),requirements=rows,executions=runs,passed_requirements=24,pending_requirements=132,
        inherited_current_rows=22,new_resume_rows=2,closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{RESUME_LEAF}),
        reviewed_producer_sources=RESUME_PRODUCERS,returns=sum(len(r['resume']['returns']) for r in runs),
        selected_presentations=sum(r['resume']['presentations'] for r in runs),selected_pixels=sum(r['resume']['pixels'] for r in runs),
        original_game_runs=0,new_pixel_comparisons=0,historical_failures_retired=False,decisions=parent['decisions'],
        scope='Two resume rows at both profiles/sizes; preserve22 accepted rows. Join actual physical input/original Mini ACKs and three fresh fully compared source generations per supported Cancel/Main Apply-reload return, resource conservation and accepted native lighting/history. Frozen producer establishes freshness; Cancel numeric baselines were not serialized. Direct Mini Apply, pause timing, Main focus, movie output/return, other132rows and both final decisions remain separate.')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--execution',nargs=2,action='append',default=[],metavar=('ACQUISITION','COMPARISON'))
    p.add_argument('--preferences-resume',action='store_true',help='Close supported Preferences Cancel/Main Apply-reload resume from retained evidence');
    p.add_argument('--lighting-menu',action='store_true',help='Analyze retained lighting and campaign/display facts while preserving the applicable cleanup/storage parent');p.add_argument('--parent-evidence')
    p.add_argument('--cleanup-storage',action='store_true',help='Close cleanup and four storage leaves while renewing sustained/setup/Quit');p.add_argument('--sustained-world',action='store_true',help='Close sustained World plus setup/Quit using all four existing Quick domains');p.add_argument('--negative',action='append',default=[]);p.add_argument('--report',required=True);a=p.parse_args()
    if not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('New immutable owned report required')
    if not (a.lighting_menu or a.preferences_resume) and (not a.execution or a.parent_evidence):p.error('Existing modes require executions; parent evidence is for lighting/menu')
    with Reader(ROOT) as reader:
        pairs=[{k:dict(record=n,sha256=reader.digest(n)) for k,n in zip(('acquisition','comparison'),names)} for names in a.execution]
        if a.preferences_resume:
            if a.execution or a.lighting_menu or a.cleanup_storage or a.sustained_world or a.negative:p.error('Preferences resume consumes its exact accepted parent')
            report=preferences_resume_progress(reader.json('docs/native-rendering-closure.json'),reader,a.parent_evidence or 'LM05.lighting-menu-closed01')
        elif a.lighting_menu:
            if a.execution or a.cleanup_storage or a.sustained_world or a.negative:p.error('Lighting/menu consumes its exact accepted parent and retained inputs')
            report=lighting_menu_progress(reader.json('docs/native-rendering-closure.json'),reader,a.parent_evidence or 'CS05.cleanup-storage-closed01')
        elif a.cleanup_storage:
            if a.sustained_world or a.negative:p.error('Cleanup/storage pack has its own exact scope')
            report=cleanup_storage_progress(reader.json('docs/native-rendering-closure.json'),reader,pairs)
        elif a.sustained_world:
            if a.negative:p.error('Sustained pack retains negative history separately; no failure is retired')
            report=sustained_progress(reader.json('docs/native-rendering-closure.json'),reader,pairs)
        else:report=progress(reader.json('docs/native-rendering-closure.json'),reader,pairs,[dict(record=n,sha256=reader.digest(n)) for n in a.negative])
        report['sources'].update({n:reader.digest(n) for n in ('tools/rendering_normal_acceptance.py','tools/join-rendering-normal-acceptance.py','docs/native-rendering-closure.json','tools/rendering_acceptance.py','tools/coverage_claims.py')})
        h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=True,passed=report['passed_requirements'],pending=report['pending_requirements'],record=a.report,sha256=h)))
if __name__=='__main__':main()
