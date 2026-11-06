#!/usr/bin/env python3
"""Close finite normal World exit from retained, independently compared captures."""
import argparse,hashlib,importlib.util,json,struct,subprocess,sys
from collections import Counter
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require,commands,acquisition,counter

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-world-exit-results-20261106/'
PACK='research/runtime/rendering-normal-results-20261031/cleanup-storage-closeout-20261105-01.json'
LEAF='journey.shutdown.world-exit'


def normal_module():
    spec=importlib.util.spec_from_file_location('world_exit_normal',ROOT/'tools/join-rendering-normal-acceptance.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module


def parent_plan(root,reader,evidence,behavior):
    result=subprocess.run([sys.executable,str(root/'tools/evidence-applicability.py'),'plan','--root',str(root),
        '--evidence',evidence,'--behavior',behavior,'--require-applicable'],cwd=root,capture_output=True,text=True)
    require(result.returncode==0,'Accepted parent impacted: '+result.stderr)
    plan=json.loads(result.stdout);require(plan['current_scope_preserved'] is True and not plan['source_actions'],'Parent needs targeted revalidation')
    ref=plan['retained_input'];binding=reader.json(ref['record'])
    require(reader.digest(ref['record'])==ref['sha256'] and binding['success'] is True,'Accepted parent identity differs')
    return binding,plan


def imported_lifetimes(raw):
    """Bind each World import to its actual surface incarnation and DELETE."""
    live={};imports=[];deleted={};end=None;nonce=None
    for op,seq,payload in commands(raw):
        if op==1:
            sid=struct.unpack_from('<I',payload)[0];require(sid not in live,'Duplicate live surface')
            live[sid]=seq
        elif op==7:
            sid=struct.unpack('<I',payload)[0];require(sid in live,'Unknown surface deletion')
            deleted[(sid,live.pop(sid))]=seq
        elif op==19:
            require(len(payload)==20,'Malformed source import')
            sid,lo,hi,nlo,nhi=struct.unpack('<5I',payload);generation=lo+(hi<<32);current_nonce=nlo+(nhi<<32)
            require(sid in live and generation==len(imports)+1 and current_nonce>0,'Import lacks owned surface/generation/nonce')
            require(nonce is None or nonce==current_nonce,'Mixed source-session nonce');nonce=current_nonce
            imports.append(dict(generation=generation,surface=sid,created_sequence=live[sid],import_sequence=seq))
        elif op==8:
            require(not live and not payload,'Source owners survive command END');end=seq
    require(imports and end is not None,'Missing final World or command END')
    for row in imports:
        deletion=deleted.get((row['surface'],row['created_sequence']))
        require(deletion is not None and row['import_sequence']<deletion<end,'Imported source incarnation survives or is deleted out of order')
        row['deleted_sequence']=deletion
    return dict(verified=True,nonce=nonce,imports=imports,command_end=end,deleted_incarnations=deleted)


def terminal_source(channel,terminal,imports,original_thread):
    """Normal Stop cancels the next WRITING slot; it is not FINISHED success."""
    require(len(channel)==35869312 and channel[:8]==b'MNMCON06','Wrong retained source channel')
    h=struct.unpack_from('<32I',channel)
    require(h[2:6]==(6,128,len(channel),3) and h[6]+(h[7]<<32)==imports['nonce']
            and h[8:14]==(1,1,1,0,0,0) and not any(h[14:]),'Wrong source roles/nonce or unreviewed terminal policy')
    require(len(terminal)==48 and terminal[:8]==b'MNMCTERM','Wrong source terminal record')
    t=struct.unpack('<12I',terminal);n=len(imports['imports'])
    require(t[2]==1 and t[3]>0 and t[4]+(t[5]<<32)==n+1 and t[6]==1
            and t[7]>0 and 64<=t[8]<=16*1024**2 and t[9]>0 and t[10]==original_thread and t[11]==0,
            'Unexplained source closure, generation, slot owner or original thread')
    step=(len(channel)-128)//2;slots=[]
    for i in range(2):
        state,reserved,lo,hi=struct.unpack_from('<4I',channel,128+i*step)
        require(reserved==0,'Source slot reserved ownership changed')
        slots.append(dict(index=i,state=state,generation=lo+(hi<<32)))
    require(sorted((v['state'],v['generation']) for v in slots)==[(0,n),(1,n+1)],
            'A native-owned source slot survives or the next original writer differs')
    return dict(verified=True,nonce=imports['nonce'],cancelled=True,finished=False,
        next_unsubmitted_generation=n+1,next_slot_state='WRITING',terminal_reason=1,
        slots=slots,native_owned_slots=0,
        close_caller_address=t[3],original_thread=t[10],source_prefix_records=t[7],source_prefix_bytes=t[8],
        scope='Known canvas_producers_close emits reason1 before cancelling the original-owned next WRITING source slot. The completed imported generations are already returned. No pending native job or successful FINISHED assertion is inferred from cancellation; acknowledged control Stop and normal retirement are checked separately.')


def exit_state(a,c,observed,lifetimes,cleanup,terminal):
    require(a['success'] is c['success'] is True and a['private_wine_stopped'] is True
            and a['retirement']['state']=='completed' and a['retirement']['forwarded']==0,'Not a complete normal execution')
    flow=a['flow'];states=[v for v in flow['states'] if 'event' not in v]
    require(flow['success'] is True and len({v['thread'] for v in states})==1 and states[0]['thread']>0
            and all(b['ack']>a['ack'] for a,b in zip(states,states[1:])),'Original state owner/ACK order differs')
    thread=states[0]['thread'];menu=c['original_menu_actions']
    require(menu['verified'] is True and cleanup['verified'] is True and cleanup['producer_stopped'] is True
            and cleanup['normal_quit'] is True and cleanup['pending_jobs']==cleanup['graphics_owners']==0,'Incomplete producer/job/owner retirement')
    if a['scenario']=='normal-quick':
        require(menu['original_callbacks']==[[3,2],[22,2],[14,2],[25,0],[14,1],[7,12],[26,1],[26,2],[22,3],[3,4]]
                and flow['world_visits']==2 and [v['screen'] for v in states[-4:]]==[26,26,22,3],
                'Missing normal Results/Quick/Main exit chain')
        require(flow['native_fallback'] is False and not flow['native_error'] and flow['recoveries']==0,'World failed before Quick exit')
    else:
        require(a['scenario']=='normal-storage' and flow['normal_quit'] is True and flow['resident_world_requested'] is True
                and flow['native_command_fallback'] is False and not flow['native_error'] and flow['native_recoveries']==0
                and states[-1]['screen']==3 and states[-1]['reloaded'] is True,'Missing restored Campaign World/Main exit')
        require(menu['owned_save_persisted'] is True and c['mini_dispatch']['verified'] is True,'Unproved storage/menu route')
    resident=a['resident'];n=len(lifetimes['imports'])
    require(n==int(resident['imports'])==int(resident['completed'])==int(resident['verified_returns'])
            ==len(c['world_domain']['worlds'])==c['original_reference']['worlds']
            and resident['pending'] is False and resident['normal_quit_retired'] is True and not resident['error'],
            'World imports/returns/history do not drain')
    require(terminal['verified'] is True and terminal['original_thread']==thread
            and terminal['next_unsubmitted_generation']==n+1,'Source closure does not belong to this final World')
    comparison=c['comparison'];require(comparison['success'] is True and comparison['mismatching_pixels']==0
            and comparison['remaining_surfaces']==0 and c['original_reference']['continuous_from_owned_initial_history'] is True
            and c['original_reference']['native_player_pixels_read'] is False
            and c['original_reference']['original_destination_capture_read'] is False,'Dependent/incomplete World pixel history')
    shown=[observed['paints'][p][0] for p in observed['swaps'] if int(observed['paints'][p][0]['world'])>0]
    matches=[v for v in comparison['comparisons'] if v['world']>0]
    require(shown and len(matches)==len(shown) and Counter(v['paint'] for v in matches)==Counter(int(v['paint']) for v in shown),
            'Missing/repeated actual World swap occurrence')
    last=shown[-1];generation=int(last['world']);seq=last['commands']
    require(0<generation<=n and last['presentation_owner']=='gui' and last['native_gpu_frame'] is True
            and last['gpu_read_complete'] is True and last['pixels_captured'] is True and int(last['lease'])>0,
            'No completed last native World presentation')
    own=lifetimes['imports'][generation-1]
    require(own['import_sequence']<=seq<own['deleted_sequence']<lifetimes['command_end'],'Presented World source was already deleted')
    surface_deletions=[v for (sid,created),v in lifetimes['deleted_incarnations'].items() if sid==last['surface'] and created<=seq<v]
    require(len(surface_deletions)==1 and seq<surface_deletions[0]<lifetimes['command_end'],'Presentation owner did not retire')
    selected=[v for v in matches if v['paint']==int(last['paint'])]
    require(selected and all(v['world']==generation and v['pixels']==last['width']*last['height']>0
            and v['mismatching_pixels']==0 and v['actual_sha256']==v['expected_sha256'] for v in selected),
            'Final swapped World lacks complete independent output')
    require(observed['command_rows'][-2][0]['kind']=='ended'
            and observed['command_rows'][-2][0]['commands']==lifetimes['command_end'],'Retired command sequence differs')
    return dict(verified=True,original_thread=thread,exit_states=states[-4:],source_imports=n,
        last_import=lifetimes['imports'][-1],last_swapped_world=last,last_compared_occurrences=selected,
        presentation_deleted_sequence=surface_deletions[0],command_end=lifetimes['command_end'],
        import_incarnations=lifetimes['imports'],terminal_source=terminal,cleanup=cleanup,
        last_import_need_not_be_swapped=True,scope='Normal terminal World exit through the observed Quick/Results or restored Campaign/Main route; complete accepted output, every imported surface incarnation deleted, next original-owned source write cancelled by known close, acknowledged producer Stop and zero jobs/graphics owners at normal Quit. Coalesced source generations need not each swap. No natural later-map/fault/restart or intermediate retirement of every external API object is claimed.')


def close_rows(plan,parent,runs):
    validate(plan);template=requirements(plan)
    require(parent['dossier_identity']==fingerprint(template) and len(parent['requirements'])==156
            and parent['passed_requirements']==sum(v['state']=='passed' for v in parent['requirements'])==34
            and {v['id'] for v in parent['requirements'] if v['state']=='passed'}==set(parent['closed_leaf_ids'])
            and [(v['configuration'],v['id']) for v in parent['requirements']]==[(v['configuration'],v['id']) for v in template],
            'Accepted34row parent inventory changed')
    expected={(p,s,z) for p in ('player-default','player-software') for s in ('normal-quick','normal-storage') for z in ((640,480),(800,600))}
    require(len(runs)==8 and {(v['domain'][0],v['scenario'],tuple(v['domain'][1])) for v in runs}==expected
            and len({v['identity'] for v in runs})==8 and all(v['proof']['verified'] is True for v in runs),'Missing/duplicate World-exit domain')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id']==LEAF:
            require(row['state']=='pending','World exit was already passed')
            row.update(state='passed',admitted_execution_evidence=[dict(reason=v['proof']['scope'],execution=v['pair'],actual_world_extents=[v['domain'][1]],provider=v['provider']) for v in runs if v['domain'][0]==row['configuration']])
        rows.append(row)
    require(sum(v['state']=='passed' for v in rows)==36,'World-exit closeout scope changed');return rows


def analyze(reader):
    root=reader.root;normal=normal_module()
    parent,impact=parent_plan(root,reader,'MS06.main-setup-closed01','NP.main-setup-menu-closeout')
    storage,storage_impact=parent_plan(root,reader,'CS05.cleanup-storage-closed01','NP.cleanup-storage-closeout')
    progress=reader.json(parent['inputs']['progress']['record'])
    require(reader.digest(parent['inputs']['progress']['record'])==parent['inputs']['progress']['sha256'],'Parent progress hash differs')
    require(storage['inputs']['closeout']==dict(record=PACK,sha256=reader.digest(PACK)),'Retained pack is not the accepted CS05 input')
    pack=reader.json(PACK);require(pack['success'] is True and len(pack['executions'])==8,'Unaccepted normal input pack')
    runs=[];sources={n:reader.digest(n) for n in parent['sources']}
    for old in pack['executions']:
        pair=old['pair'];a=normal.receipt(reader,pair['acquisition']);c=normal.receipt(reader,pair['comparison'])
        streams=[]
        for artifact in a['artifacts']:
            if '/capture/' not in artifact['path'] or not artifact['path'].endswith('.bin'):continue
            body=reader.read_bytes(artifact['record']);require(len(body)==artifact['bytes'] and hashlib.sha256(body).hexdigest()==artifact['sha256'],'Changed retained capture');streams.append(body)
        observed=acquisition(streams,a['session_identity'])
        require(hashlib.sha256(observed['commands']).hexdigest()==c['captured_command_stream_sha256'],'Compared command source differs')
        require(normal.resource_history(observed['commands'])==old['resources'],'Accepted resource ledger changed')
        require(old['identity']==a['session_identity'] and old['verified'] is True,'Unaccepted original session')
        lifetimes=imported_lifetimes(observed['commands']);cleanup=normal.cleanup_proof(reader,a)
        require(cleanup==old['cleanup'],'Accepted terminal cleanup changed')
        diagnostics=[Path(v['path']).name for v in a['artifacts'] if Path(v['path']).name.startswith('world-terminal-')]
        require(diagnostics==['world-terminal-0001.bin'],'Unexpected source failure diagnostic in normal exit')
        terminal=terminal_source(normal.retained_artifact(reader,a,'world-continuous.bin'),normal.retained_artifact(reader,a,'world-terminal-0001.bin'),lifetimes,a['flow']['states'][0]['thread'])
        proof=exit_state(a,c,observed,lifetimes,cleanup,terminal)
        runs.append(dict(identity=a['session_identity'],domain=old['domain'],scenario=a['scenario'],pair=pair,provider=old['provider'],proof=proof))
    sources.update({n:reader.digest(n) for n in ('tools/close-rendering-world-exit.py','tests/test-rendering-world-exit-closeout.py')})
    require(all(reader.digest(n)==h for n,h in sources.items()),'Mixed current World-exit source phase')
    rows=close_rows(reader.json('docs/native-rendering-closure.json'),progress,runs)
    return dict(schema=1,kind='native_rendering_normal_world_exit_closeout',success=True,sources=sources,
        accepted_parent=dict(evidence='MS06.main-setup-closed01',binding=impact['retained_input'],applicability=impact),
        retained_input_pack=dict(record=PACK,sha256=reader.digest(PACK),applicability=storage_impact),
        dossier_identity=progress['dossier_identity'],requirements=rows,executions=runs,
        passed_requirements=36,pending_requirements=120,inherited_current_rows=34,new_world_exit_rows=2,
        closed_leaf_ids=sorted(set(progress['closed_leaf_ids'])|{LEAF}),original_game_runs=0,new_pixel_comparisons=0,
        new_native_code=False,new_game_fixture=False,source_imports=sum(v['proof']['source_imports'] for v in runs),
        cancelled_next_writers=len(runs),last_world_comparisons=sum(len(v['proof']['last_compared_occurrences']) for v in runs),
        selected_pixels=sum(m['pixels'] for v in runs for m in v['proof']['last_compared_occurrences']),
        historical_failures_retired=False,decisions=progress['decisions'],
        scope='Close2normal terminal World-exit rows across8retained Quick/storage default/software640/800domains, preserving34accepted rows. Bind actual original exit callbacks, last independently compared swapped World, every imported source incarnation/command END, known cancelled next WRITING slot, acknowledged producer Stop, returned jobs and zero native owners at normal Quit. No new game, pixel comparison, native code or fixture. Other120rows, natural later-map, intermediate every-API owner, faults/providers, both final decisions and known Save admission/reliability remain separate.')


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'Fresh owned result required')
    result=dict(schema=1,kind='native_rendering_normal_world_exit_closeout',success=False,original_game_runs=0)
    try:
        with Reader(ROOT) as reader:result=analyze(reader)
    except Exception as error:result['error']=str(error)
    sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=a.report,sha256=sha,passed=result.get('passed_requirements'))),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
