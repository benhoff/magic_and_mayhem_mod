"""EP04 collector admission and requirement-scoped progress, separate from final decisions."""
from pathlib import Path
import hashlib,json,sqlite3,struct
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require,number,commands

PREFIX='research/runtime/rendering-normal-results-20261031/'

def digest(path):
    with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()

def world_domain(observed):
    surfaces={};worlds=[]
    for op,seq,raw in commands(observed['commands']):
        if op==1:
            sid,w,h,bits,r,g,b=struct.unpack_from('<7I',raw)
            surfaces[sid]=(w,h,bits,r,g,b)
        elif op==7:surfaces.pop(struct.unpack_from('<I',raw)[0])
        elif op==19:
            sid,lo,hi=struct.unpack_from('<3I',raw)
            w,h,bits,r,g,b=surfaces[sid]
            require((bits,r,g,b)==(16,0xf800,0x7e0,31) and (w,h) in ((640,480),(800,600)),'World outside declared RGB565/size domain')
            worlds.append(dict(generation=lo+(hi<<32),width=w,height=h))
    require(worlds,'No actual World format/extent observations')
    return dict(verified=True,format='RGB565',extents=[list(p) for p in sorted({(r['width'],r['height']) for r in worlds})],worlds=worlds)

def lighting(root,acquired,artifacts):
    session=Path(acquired['session'])
    def bound(path):
        relative=path.relative_to(root).as_posix()
        require(relative in artifacts and digest(path)==artifacts[relative]['sha256'],'Unbound normal-session input: '+relative)
    manifest=session/'manifest.json';bound(manifest)
    policy=json.loads(manifest.read_text())['terrain_light_session']
    require(policy['mode']=='native' and policy['capture']=='none','Wrong actual lighting configuration')
    outcome=session/'terrain-light-session/outcome.sqlite3';bound(outcome)
    db=sqlite3.connect(outcome.as_uri()+'?mode=ro',uri=True)
    try:
        rows=db.execute('select payload from results').fetchall()
        require(len(rows)==1,'Ambiguous lighting worker result')
        result=json.loads(rows[0][0])
    finally:db.close()
    s=result['statistics']
    require(result['success'] is True and result['mode']=='native' and result['worker_exit']==0 and result['worker_exited_early'] is False and result['source_bound'] is True,'Native lighting worker failed')
    require(number(s['applied'],1)>0 and all(number(s[k])==0 for k in ('fallback','refused','rejected','error')),'Native lighting refused or fell back')
    require(all(digest(root/n)==h for n,h in policy['sources'].items()),'Changed lighting compiler closure')
    return dict(verified=True,policy=policy,result=result,record=artifacts[outcome.relative_to(root).as_posix()])

def preferences_boundary(root,acquired,artifacts):
    path=Path(acquired['session'])/'lock-capture/lifecycle.log'
    relative=path.relative_to(root).as_posix()
    require(relative in artifacts and digest(path)==artifacts[relative]['sha256'],'Unbound Preferences lifecycle')
    rows=[line.split() for line in path.read_text().splitlines() if line.startswith('resident_preferences_materialized ')]
    require(1<=len(rows)<=2 and all(len(row)==20 and int(row[4],16)==2 and 1<=int(row[5],16)<=32 for row in rows),'Missing quiescent Preferences history materialization')
    return dict(verified=True,diagnostic_policy='Identical diagnostic rows are suppressed; both original Preferences callbacks separately prove admitted dispatch through the source-bound mandatory service gate.',materializations=[dict(action=int(r[4],16),histories=int(r[5],16)) for r in rows],record=artifacts[relative])

def fullscreen(flow,observed):
    rows=flow.get('fullscreen_transitions',[])
    require(len(rows)==4 and [r['fullscreen'] for r in rows]==[True,False]*2 and all(r['accepted'] is True for r in rows),'Required F9 transitions missing')
    intervals=[]
    for begin,end in zip(rows[::2],rows[1::2]):
        a,b=int(begin['paint_before']),int(end['paint_before'])
        paints=[i for i in observed['swaps'] if a<i<=b and observed['paints'][i][0]['world']!='0']
        require(a<b and paints,'Fullscreen has no actual World presentation')
        require(any(observed['paints'][i][0]['width']==begin['screen_width'] and observed['paints'][i][0]['height']==begin['screen_height'] for i in paints),'Fullscreen did not present at actual screen extent')
        intervals.append(dict(begin=begin,end=end,presented_paints=paints))
    return dict(verified=True,intervals=intervals,scope='Actual Qt F9 entry/exit, native World swaps during fullscreen and independent full viewport comparison; no original exclusive/padded driver promise.')

def receipt(reader,reference):
    require(set(reference)=={'record','sha256'} and reference['record'].startswith(PREFIX) and '..' not in Path(reference['record']).parts,'Unsafe EP04 receipt')
    require(reader.digest(reference['record'])==reference['sha256'],'EP04 payload changed')
    return reader.json(reference['record'])

DRIVER_SOURCES={'tools/menu-storage-input.py','tools/campaign_storage.py'}

def reviewed_driver_sources(reader,reference,acquisition_reference,acquired,facts):
    review=receipt(reader,reference)
    require(review['kind']=='scoped_rendering_driver_branch_review' and review['success'] is True and review['native_binary_changes_allowed'] is False,'Invalid auxiliary-driver review')
    require(acquisition_reference in review['eligible_acquisitions'],'Driver review does not bind this acquisition')
    require(all(reader.digest(n)==h for n,h in review['sources'].items()),'Changed driver review sources')
    changes=review['changes'];require(set(changes)==DRIVER_SOURCES,'Driver review cannot waive another source')
    if acquired['scenario']=='normal-storage':
        require(facts.get('world_extents')==[[800,600]] and facts.get('presented_world_image_extents')==[[800,600]],'Low-only branch exclusion is not proved for these inputs')
    else:require(acquired['scenario']=='normal-quick','Unsupported driver reuse scenario')
    sources=dict(acquired['sources'])
    for name,change in changes.items():
        require(sources[name]==change['before'] and reader.digest(name)==change['after'],'Driver source pair differs')
        require(reader.digest(change['before_record'])==change['before'],'Lost original driver bytes')
        sources[name]=change['after']
    sources.update(review['sources'])
    return sources

def admit(reader,pair):
    require(set(pair)=={'acquisition','comparison'},'A scoped execution needs both collectors')
    a,c=[receipt(reader,pair[k]) for k in ('acquisition','comparison')]
    require(a['kind']=='public_native_normal_acquisition' and c['kind']=='public_native_normal_comparison' and c['acquisition']==pair['acquisition'],'Mixed EP04 collectors')
    require(a['scenario'] in ('normal-quick','normal-storage'),'Unknown normal scenario')
    require(all(v['success'] is True and v['sources_stable'] is True and v['original_manifest_verified_before_after'] is True and v['original_pixels_used_as_native_inputs'] is False for v in (a,c)),'Incomplete execution or comparison')
    cfg=a['configuration']
    require(cfg.get('diagnostic_only',False) is False and 'injected_fault' not in cfg,'Diagnostic execution is ineligible for nominal acceptance')
    require(cfg['software_flag'] is False and cfg['skip_intros'] is False and cfg['terrain_lighting']=='native' and cfg.get('source_chunks') is True and cfg['native_lighting_capture'] is False and cfg['viewport_scaling']=='integer-sharp','Execution differs from the default target')
    require(a['exit_code']==0 and a['private_wine_stopped'] is True and a['retirement']['state']=='completed' and a['retirement']['forwarded']==0,'Normal original exit or strict ownership missing')
    retired=a['retirement']['graphics_retirement']
    require(retired['process_exit'] is True and retired['resources_released'] is True and not retired['surviving_owners'] and all(v==0 for v in retired['counts'].values()),'Surviving graphics owner')
    resident=a['resident']
    require(resident['normal_quit_retired'] is True and resident['pending'] is False and not resident['error'] and int(resident['source_copy_rect_refusals'])==0 and resident['renderer'],'World lifetime, provider or refused caller unresolved')
    require(resident.get('source_chunks_enabled') is True and resident.get('stage_profile',{}).get('capture_complete') is True and resident['stage_profile']['capture']['format']==3,'Incomplete current-protocol source history')
    require(c['lighting']['verified'] is True and c['original_menu_actions']['verified'] is True and c['asset_inputs_stable'] is True,'Missing actual lighting/actions/assets')
    domain=c['world_domain']
    require(domain['verified'] is True and domain['format']=='RGB565' and domain['extents'] and all(p in ([640,480],[800,600]) for p in domain['extents']),'Unproved World domain')
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

def progress(plan,reader,pairs,negatives=()):
    spec=validate(plan);runs=[admit(reader,pair) for pair in pairs]
    require(runs and len({fingerprint(r['pair']) for r in runs})==len(runs),'Duplicate execution pair')
    require(len({(r['scenario'],encoded_extents(r)) for r in runs})==len(runs),'Duplicate scenario/domain; select one reviewed source phase')
    sources={}
    for run in runs:
        for n,h in run['sources'].items():
            require(n not in sources or sources[n]==h,'Different current source closures');sources[n]=h
    require(all(reader.digest(n)==h for n,h in sources.items()),'EP04 execution is historical for changed sources')
    # Each allowlisted leaf has a collector-specific check above. Broader scene,
    # media, input, conditional-scope and fault requirements remain explicit.
    obligations={
      'normal-quick':{
        'journey.quick-battle-entry.setup-confirm':'Original setup/map callbacks acknowledge the offered ordinal and the actual World entry.',
        'journey.shutdown.menu-exit':'Actual original Main Quit callback and normally stopped private Wine generation.',
      },
      'normal-storage':{
        'journey.save-load.save':'Original Save acknowledgement, ordered native history materialization and exact persisted save bytes.',
        'journey.save-load.load':'Actual Mini and Main Load callbacks, fresh source/resource generations and both observed resolution rebuilds.',
        'journey.save-load.restored-world':'Every captured restored viewport is independently compared from complete initial history; original simulation remains a declared supplier.',
        'journey.pause-preferences.cancel':'A changed dialogue draft is cancelled, reopened original values/profile are unchanged, and the actual original Cancel callbacks return.',
      },
    }
    passed={};storage_sizes={tuple(p) for r in runs if r['scenario']=='normal-storage' for p in r['world_extents']}
    for run in runs:
        for key,reason in obligations[run['scenario']].items():
            passed.setdefault(key,[]).append(dict(reason=reason,execution=run['pair'],actual_world_extents=run['world_extents'],provider=run['provider']))
    negative_runs=[]
    for reference in negatives:
        failed=receipt(reader,reference)
        require(failed['kind']=='public_native_normal_acquisition' and failed['success'] is False and failed['original_manifest_verified_before_after'] is True,'Not a retained normal execution failure')
        changes=[n for n,h in failed['sources'].items() if reader.digest(n)!=h]
        flow_records=[r for r in failed.get('artifacts',[]) if r['path'].endswith('/flow.json')]
        native_error=failed['error']
        if flow_records:
            require(len(flow_records)==1 and reader.digest(flow_records[0]['record'])==flow_records[0]['sha256'],'Changed negative action receipt')
            native_error=reader.json(flow_records[0]['record']).get('error') or native_error
        negative_runs.append(dict(reference=reference,scenario=failed['scenario'],configuration=failed['configuration'],error=native_error,acquisition_error=failed['error'],experiments=failed['experiments'],resolution='unresolved',changed_sources=changes,blocks_final_decisions=['supported-player'],scope='A passing finite leaf does not retire this failure. Root cause and current applicability remain explicit EP04/EP09 work; changed recording helpers do not establish an engine repair.'))
    require(len({r['reference']['record'] for r in negative_runs})==len(negative_runs),'Duplicate failure receipt')
    rows=[]
    for row in requirements(plan):
        proof=passed.get(row['id']) if row['configuration']=='player-default' else None
        if row['id'].startswith('journey.save-load.') or row['id']=='journey.pause-preferences.cancel':
            if storage_sizes!={(640,480),(800,600)}:proof=None
        rows.append(dict(row,state='passed' if proof else 'pending',admitted_execution_evidence=proof or []))
    require(len(rows)==156 and all(not r['admitted_execution_evidence'] for r in rows if r['state']=='pending'),'Requirement denominator changed')
    return dict(schema=1,kind='native_rendering_acceptance_progress',success=True,dossier_identity=fingerprint(dict(spec=spec,requirements=requirements(plan))),sources=sources,executions=runs,negative_executions=negative_runs,requirements=rows,scope_conditions=[dict(id=d['id'],state='unchanged_from_reviewed_scope',definition=d) for d in plan['required_gap_register']['scope_decisions']],passed_requirements=sum(r['state']=='passed' for r in rows),pending_requirements=sum(r['state']=='pending' for r in rows),decisions={'supported-player':'pending','original-recovery':'pending'},scope='EP04 requirement-specific evidence for the named finite default sessions. Storage and cancellation require actual World proof at both configured sizes. Pending rows, software configuration, remaining scope conditions and original recovery are not waived. No percentage of implementation or final acceptance is inferred.')

def encoded_extents(run):return tuple(tuple(p) for p in run['world_extents'])
