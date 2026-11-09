"""Strict source-bound native target-loss and terminal deletion witnesses."""
import hashlib,json,shlex
from pathlib import Path
from rendering_pilot import require,counter,number

FAULT='quiescent-native-offscreen-target-loss'
ERROR='OpenGL offscreen target is unavailable'

def linked_sources(build,root,target):
    build,root=Path(build).resolve(),Path(root).resolve()
    pending=[build/'CMakeFiles'/(target+'.dir')/'link.txt'];visited=set();sources=set()
    while pending:
        link=pending.pop().resolve()
        if link in visited:continue
        visited.add(link);base=link.parents[2];objects=0
        for word in shlex.split(link.read_text()):
            path=Path(word)
            if path.suffix not in ('.o','.a'):continue
            path=(base/path).resolve()
            if not path.is_relative_to(build):continue
            if path.suffix=='.a':
                pending.append(path.parent/'CMakeFiles'/(path.stem.removeprefix('lib')+'.dir')/'link.txt');continue
            objects+=1
            for token in shlex.split(Path(str(path)+'.d').read_text().replace('\\\n',' ')):
                source=(base/Path(token)).resolve()
                if source.is_file() and source.is_relative_to(root) and not source.is_relative_to(root/'working'):
                    sources.add(source.relative_to(root).as_posix())
        require(objects>0,'Missing linked compiler dependencies: '+str(link))
    return sources

def admit_requests(cases,repair_of,maximum_requests,maximum_games):
    require(len(cases)<maximum_requests,'Surface-loss request cap exhausted')
    names={v['report']:v for v in cases};require(len(names)==len(cases),'Duplicate surface-loss request')
    charges=[v['charged_games'] for v in cases]
    require(all(type(v) is int and v in (0,1,2) for v in charges) and sum(charges)+2<=maximum_games,
            'Surface-loss charged start cap exhausted or unknown')
    def ancestors(name):
        found=set()
        while name:
            require(name in names and name not in found,'Missing/cyclic surface-loss repair ancestry')
            found.add(name);name=names[name]['repair_of']
        return found
    resolved=set()
    for row in cases:
        if row['proved'] and row['repair_of']:resolved.update(ancestors(row['repair_of']))
    repairing=ancestors(repair_of) if repair_of else set()
    require(not repair_of or sum(bool(v['repair_of']) for v in cases)<1,'Surface-loss repair case cap exhausted')
    for row in cases:
        require(row['result']['original_game_runs']<=row['charged_games'],'Observed starts exceed retained charge')
        if row['report'] in resolved:continue
        if row['report'] in repairing:require(not row['result']['success'] or row['comparison_failed'],'Repair has no retained failure')
        else:require(row['proved'],'Stop after unreviewed surface-loss acquisition/comparison')
    return dict(requests_before=len(cases),charged_original_starts_before=sum(charges),
        observed_original_starts_before=sum(v['result']['original_game_runs'] for v in cases),
        reserved_original_starts=2,repair_ancestors=sorted(repairing),resolved_ancestors=sorted(resolved))

def loss_receipt(value,expected,request_raw,observed,retired):
    require(value['kind']=='native-offscreen-target-loss' and value['observation']==observed and value['retirement']==retired,
            'Unbound native surface-loss receipts')
    request=json.loads(request_raw)
    number(request['schema'],1,1);number(request['transport_version'],3,3)
    require(value['request']==request and set(request)=={'schema','producer','transport_version','request_id','action'} and
            request['schema']==1 and type(request['producer']) is int and request['producer']==expected and
            request['transport_version']==3 and request['action']=='destroy-render-target','Wrong native loss request')
    nonce=request['request_id'];require(isinstance(nonce,str) and len(nonce)==32 and bytes.fromhex(nonce).hex()==nonce,'Invalid native loss nonce')
    number(observed['schema'],1,1);number(observed['producer'],1,2**32-1);number(observed['transport_version'],3,3)
    require(observed['schema']==1 and observed['kind']=='native_offscreen_target_loss' and observed['producer']==expected and
            observed['transport_version']==3 and observed['request_id']==nonce and observed['request_sha256']==hashlib.sha256(request_raw).hexdigest(),
            'Native provider/source identity differs')
    owner=observed['provider_owner']
    if owner=='gui-resident':require(observed['worker_thread'] is False and observed['render_owner_gui_thread'] is True,'Wrong GUI provider owner')
    elif owner=='dedicated-thread':require(observed['worker_thread'] is True,'Wrong dedicated provider owner')
    else:require(False,'Unknown native graphics owner')
    for key in ('gui_destroyed_target','surface_valid_before','graphics_failure','injected_diagnostic'):
        require(observed[key] is True,'Missing native loss observation: '+key)
    for key in ('surface_valid_after','physical_hardware_reset','old_generation_continuation'):
        require(observed[key] is False,'Native loss scope changed: '+key)
    require(observed['error']==ERROR and counter(observed['worlds_before'])>0 and number(observed['live_surfaces_before'])>0 and
            counter(observed['pending_cpu_read'])==0,'No live unborrowed original World provider')
    request_at=number(value['request_host_ns']);begin=counter(observed['begin_host_us'])*1000
    destroy=counter(observed['destroy_begin_host_us'])*1000;destroyed=counter(observed['destroy_end_host_us'])*1000
    end=counter(observed['end_host_us'])*1000;retirement=counter(retired['retirement_host_us'])*1000
    require(request_at<=begin<=destroy<=destroyed<=end<=retirement and end-begin<=5_000_000_000,'Native provider-loss clocks/bound differ')
    require(all(retired.get(k)==v for k,v in observed.items()),'Retirement changed original native fault')
    for key in ('target_restored_for_deletion','command_owner_retired','world_owner_retired','frame_leases_retired','worker_teardown_complete'):
        require(retired[key] is True,'Incomplete native loss retirement: '+key)
    require(retired['normal_completion_claimed'] is False and counter(retired['command_surfaces_after_cleanup'])==0 and
            counter(retired['renderer_surfaces_after_cleanup'])==0,'Lost provider retained storage or gained normal credit')
    return dict(begin_host_ns=str(begin),end_host_ns=str(end),retirement_host_ns=str(retirement),
                request_id=nonce,provider_owner=owner,actual_platform_target_destroyed=True,restored_for_deletion_only=True,
                policy='terminal-failure/public-fresh-generation',physical_hardware_reset=False,old_generation_continuation=False)

def retained_loss(reader,acquisition,root,producer):
    from rendering_transition_assessment import artifact
    base=Path(root)
    value=acquisition['proof']['epochs'][0]['fault']
    require(json.loads(artifact(reader,acquisition,'fault-injection.json'))==value,'Unbound loss controller handoff')
    return loss_receipt(value,producer,artifact(reader,acquisition,base/'surface-loss-request-v1.json'),
        json.loads(artifact(reader,acquisition,base/'surface-loss-observation-v1.json')),
        json.loads(artifact(reader,acquisition,base/'surface-loss-retirement-v1.json')))

def source_phase_review(reader,name,comparison,phase=None):
    """Keep exact accepted low scopes; never waive native or unknown drift."""
    changed={n:dict(recorded=h,current=reader.digest(n)) for n,h in comparison['sources'].items() if reader.digest(n)!=h}
    if not changed:return {}
    require(phase and phase['success'] is True and phase['kind']=='native_surface_loss_prospective_plan' and
        phase['original_game_runs']==0 and phase['bounds']['maximum_requests']==6 and phase['bounds']['maximum_original_games']==10,
        'Execution source drift requires an exact bounded controller continuation')
    review=phase['controller_continuation']
    require(review['success'] is True and review['kind']=='native_surface_loss_extent_continuation' and review['original_game_runs']==0 and
        review['unchanged_compiled_player_runtime_oracle'] is True and review['existing_low_controller_ast_preserved'] is True and
        review['existing_receipt_budget_and_admission_conditions_ast_preserved'] is True and
        review['existing_assessment_ast_preserved_except_provider_terminal_header'] is True and
        review['raw_low_proofs_reassessed_unchanged'] is True and review['current_low_cli_execution_claimed'] is False and
        review['charges_refunded']==0,'Unproved or widened controller continuation')
    accepted=[v for v in review['retained_low_comparisons'] if v['record']==name]
    require(len(accepted)==1 and accepted[0]['sha256']==reader.digest(name) and accepted[0]['acquisition']==comparison['acquisition'] and
        comparison['success'] and comparison['sources_stable'] and comparison['proof']['verified'] and
        comparison['proof']['domain']['size']==[640,480] and comparison['comparison']['mismatching_pixels']==0,
        'Comparison is not one of the exact accepted low phases')
    pairs=review['source_changes']
    require(set(pairs)=={'tools/rendering_surface_loss_live.py','tools/rendering_surface_loss.py','tests/test-rendering-surface-loss.py',
        'tools/close-rendering-surface-loss.py','tools/rendering_transition_assessment.py'},'Unexpected controller/analysis delta domain')
    for n,pair in changed.items():
        require(n in pairs and pairs[n]['before']==pair['recorded'] and pairs[n]['after']==pair['current'] and
            reader.digest(pairs[n]['before_record'])==pair['recorded'],'Unreviewed execution source/preimage drift: '+n)
    return dict(accepted_comparison=accepted[0],preserved_recorded_sources=changed,current_low_cli_execution_claimed=False,
        scope='Accepted original low execution and pixel scope preserved under its exact source phase; current size dispatch delegates its unchanged handler. No current low recollection or native/runtime/oracle waiver.')
