#!/usr/bin/env python3
"""Verify retained physical pans, original object picking and minimap/HUD alignment."""
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
from rendering_interaction_live import sprite_target
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, require
from resident_capture import records
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-picking-resize-results-20261107/'
MATRIX='research/runtime/rendering-interaction-results-20261107/matrix-01.json'
PARENT='research/runtime/rendering-retained-results-20261107/closeout-analysis-04.json'
BID='NP.pan-picking-hud-acceptance'
SID='native.pan-picking-hud-acceptance'
LEAF='journey.camera-motion.pan'


def module(name):
    spec=importlib.util.spec_from_file_location('pan_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def checked(r,ref):
    require(r.digest(ref['record'])==ref['sha256'],'Changed retained pan input')
    return r.json(ref['record'])


def validate_pan(p):
    require(p['verified'] and p['physical_pan'] and p['normal_quit']
            and p['before']['view']==p['after']['view']==0
            and p['before']['center']!=p['after']['center']
            and p['object_hit']['slot']==p['object_hit']['type']==p['object_hit']['owner']==0
            and p['object_hit']['active'] and p['object_hit']['health']>0
            and p['motion_callback_returned'] and p['selection_callback_returned']
            and p['source_only_target'] and p['picking_source_after_pan']
            and p['minimap_camera_matches_world'] and p['minimap_layout_unchanged']
            and p['whole_viewport_swaps']>=3 and p['compared_pixels']>0
            and p['mismatching_pixels']==0 and p['source_generations']>=3,
            'Incomplete pan/picking/HUD alignment')


def case(r,ref):
    c=checked(r,ref);a=checked(r,c['acquisition']);proof=c['proof']
    require(c['success'] and c['sources_stable'] and c['comparison_processes_stopped']
            and c['comparison']['success'] and c['comparison']['mismatching_pixels']==0
            and c['original_reference']['continuous_from_owned_initial_history']
            and not c['original_reference']['native_player_pixels_read']
            and not c['original_reference']['original_destination_capture_read']
            and proof['verified'] and proof['cleanup']['normal_quit']
            and a['success'] and a['original_game_runs']==1 and a['private_wine_stopped']
            and not a.get('forced_private_prefix_stop')
            and all(r.digest(n)==h for n,h in c['sources'].items()),'Unaccepted or changed pan source phase')
    streams=[artifact(r,a,Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
    observed=acquisition(streams,a['session_identity'])
    require(hashlib.sha256(observed['commands']).hexdigest()==c['captured_command_stream_sha256'],'Different compared command history')
    frames=decode(artifact(r,a,'frame-input.bin'));rows=frames['rows']
    by_generation={x['frame']['generation']:x for x in rows if x['kind']==1 and x['frame']}
    matches=[v for v in c['comparison']['comparisons'] if v['world']>0]
    shown=[observed['paints'][i] for i in observed['swaps'] if int(observed['paints'][i][0]['world'])>0]
    require(len(shown)==len(matches)==proof['world_presentations'],'Omitted actual World swap')
    pairs=list(zip(shown,matches));wanted=set();tasks=[]
    for mode,scale in [('sharp',0),('integer',2)]:
        pan=next(v for v in proof['pans'] if v['mode']==mode)
        def mark(action,phase):
            found=[v for v in a['camera_actions'] if v['action']==action and v['phase']==phase]
            require(len(found)==1,'Missing/duplicate original action marker');return found[0]
        begin=mark(mode+'-pan','begin');end=mark(mode+'-panned-away-2','settled')
        keys=[v for v in a['actions'] if v['action']==mode+'-pan']
        require(len(keys)==1 and keys[0]['symbol']==0xff52 and keys[0]['down_up'] and keys[0]['held_ms']>0
                and keys[0]['host_focus']['host_focused'],'No physical focused Up pan')
        selected=[(p,body,m) for (p,body),m in pairs if int(begin['host_us'])<int(p['host_us'])<=int(end['host_us'])]
        require(selected and all(p['scaling']==scale for p,b,m in selected),'Wrong actual pan scaling')
        wanted.update(int(p['world']) for p,b,m in selected);wanted.update(pan['source_frames'])
        target=next(v for v in a['actions'] if v.get('action')=='source-sprite-target' and v.get('name')==mode+'-panned')
        wanted.add(target['source_generation'])
        hit=next(v for v in proof['cursor_bindings'] if v['action']==mode+'-panned-hover')
        selection=next(v for v in proof['selection_pairs'] if v['mode']==mode and v['case']=='panned')['toggles'][0]
        tasks.append((mode,scale,pan,begin,end,selected,target,hit,selection))
    source=Path(a['temporary_root'])/'worlds.bin'
    require(hashlib.sha256(source.read_bytes()).hexdigest()==hashlib.sha256(artifact(r,a,'worlds.bin')).hexdigest(),'Changed physical source history')
    contexts={};targets={};decoder=module('inspect-canvas-producers');camera=module('close-rendering-camera-matrix')
    for g,chunk,stream,request in records(source):
        if chunk or g not in wanted:continue
        _,sr=decoder.decode(stream,complete=False);ctx=camera.context_many_cells(sr)
        require(ctx and ctx['view']==0 and ctx['creature_markers']>0,'Pan source lacks current camera/creatures')
        terrain=[f for f,b in sr if f[2]==22]
        overlays=[(f,struct.unpack_from('<37I',b)) for f,b in sr if f[2]==24]
        require(len(terrain)==1 and overlays and all(m[9]==ctx['view'] and list(m[3:5])==ctx['center'] for f,m in overlays),'HUD minimap uses another camera')
        t=terrain[0];contexts[g]=dict(ctx,minimap_layout=list(t[5:10]),source_sha256=hashlib.sha256(stream).hexdigest())
        for task in tasks:
            target=task[6]
            if target['source_generation']==g:
                state=by_generation[g];actual=sprite_target(sr,state['history'])
                require(all(target[k]==v for k,v in actual.items()),'Panned picking target not derived from original sprite')
                targets[g]=actual
    require(set(contexts)==wanted,'Missing selected source generation')
    results=[]
    for mode,scale,pan,begin,end,selected,target,hit,selection in tasks:
        before,after=(contexts[g] for g in pan['source_frames'])
        require(before['view']==after['view']==0 and before['center']!=after['center'],'Pan did not move current source camera')
        require(target['source_generation']>=pan['source_frames'][-1] and target['source_generation'] in targets,'Picking used pre-pan source')
        entry,returned=(rows[selection[k]-1] for k in ('entry','returned'))
        require(entry['kind']==2 and returned['kind']==3 and entry['call']==returned['call']==selection['call']
                and entry['mouse'][:2]==returned['mouse'][:2]==target['point']
                and entry['frame']['generation']>=target['source_generation']
                and entry['hit']['pool']==returned['hit']['pool']==selection['pool']
                and entry['hit']['flag68']==selection['before'] and returned['hit']['flag68']==selection['after'],
                'Panned target lacks its actual original selection callback')
        physical=[v for v in a['actions'] if v['action']=='pointer' and v['logical']==target['point']
                  and int(begin['host_us'])<=v['host_ns']//1000<=int(end['host_us'])]
        require(physical,'Panned source target lacks actual pointer motion')
        pixels=0;gens=[];hashes={};layout=before['minimap_layout']
        for p,body,m in selected:
            g,paint=int(p['world']),int(p['paint']);state=by_generation[g]
            require(g>=int(begin['world']) and contexts[g]['minimap_layout']==layout
                    and state['camera'][0]==contexts[g]['view'] and state['frame']['nonce']==frames['nonce'],
                    'Pan output camera/nonce or HUD anchoring differs')
            require(m['world']==g and m['paint']==paint and m['lease']==int(p['lease'])
                    and p['native_gpu_frame'] and p['gpu_read_complete'] and p['pixels_captured']
                    and p['presentation_owner']=='gui' and [p['frame_width'],p['frame_height']]==a['configuration']['actual_size'],
                    'Pan output lacks its actual complete native frame')
            if paint not in hashes:hashes[paint]=hashlib.sha256(body() if callable(body) else body).hexdigest()
            require(hashes[paint]==m['actual_sha256']==m['expected_sha256'] and m['pixels']==p['width']*p['height']
                    and m['mismatching_pixels']==0,'Pan output differs from independent complete original viewport')
            pixels+=m['pixels'];gens.append(g)
        hover_rows=[by_generation[g] for g in hit['source_frames']]
        require(len(hover_rows)>=3 and all(v['hit'] and v['hit']['slot']==v['hit']['type']==v['hit']['owner']==0
            and v['hit']['active'] and v['hit']['health']>0 and v['cursor']['point']==target['point']
            and v['frame']['generation'] in gens for v in hover_rows),'Panned original hit/pointer/display identities differ')
        callbacks=[v for v in rows if v['kind']==3 and v['mask']&112 and not v['mask']&3
            and v['mouse'][:2]==target['point'] and v['ordinal']<=entry['ordinal']
            and v['frame'] and v['frame']['generation']>=target['source_generation']]
        require(callbacks,'No panned original motion callback return')
        result=dict(verified=True,mode=mode,physical_pan=True,before=before,after=after,
            object_hit=hover_rows[-1]['hit'],motion_callback_returned=True,selection_callback_returned=True,
            motion_callback=callbacks[-1]['call'],selection_callback=selection,target=target,
            source_only_target=True,picking_source_after_pan=True,minimap_camera_matches_world=True,
            minimap_layout_unchanged=True,whole_viewport_swaps=len(selected),compared_pixels=pixels,
            source_generations=len(set(gens)),mismatching_pixels=0,normal_quit=True)
        validate_pan(result);results.append(result)
    return dict(acquisition=c['acquisition'],comparison=ref,domain=proof['domain'],pans=results,
                source_phase='Exact retained original/native/compiler/oracle bytes; no new game or native execution')


def close_rows(plan,parent,cases):
    validate(plan);defs=requirements(plan);identities={(x['configuration'],x['id']) for x in defs}
    require(parent['dossier_identity']==fingerprint(defs) and len(parent['requirements'])==len(identities)==156
            and {(x['configuration'],x['id']) for x in parent['requirements']}==identities,'Frozen requirement inventory differs')
    require(len(cases)==4 and {(v['domain'][0],tuple(v['domain'][1])) for v in cases}=={(s,z) for s in (False,True) for z in ((800,600),(640,480))},'Incomplete pan domain matrix')
    for c in cases:
        require(len(c['pans'])==2 and {p['mode'] for p in c['pans']}=={'sharp','integer'},'Incomplete pan/scaling matrix')
        for p in c['pans']:validate_pan(p)
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id']==LEAF:
            require(row['state']=='pending','Pan row already accepted')
            software=row['configuration']=='player-software'
            row.update(state='passed',admitted_execution_evidence=[dict(execution=dict(acquisition=c['acquisition'],comparison=c['comparison']),actual_world_extents=[c['domain'][1]],reason='Actual focused physical pan, changed original/source camera, post-pan source-derived object target and original motion/selection acknowledgement, coherent fixed-layout minimap camera and every contiguous actual viewport swap independently compared. Finite Xvfb/Mesa scope; no atomic HUD, cell/ray, resize or hotspot claim.') for c in cases if c['domain'][0] is software])
        rows.append(row)
    require(sum(x['state']=='passed' for x in rows)==parent['passed_requirements']+2,'Pan closeout exceeded two rows')
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh owned report required')
    result=dict(schema=1,kind='pan_picking_hud_acceptance',success=False)
    with Reader(ROOT) as r:
        try:
            matrix=r.json(MATRIX);require(matrix['success'] and matrix['mismatching_pixels']==0 and all(r.digest(n)==h for n,h in matrix['sources'].items()),'Accepted matrix source changed')
            cases=[case(r,ref) for ref in matrix['comparisons']]
            parent=r.json(PARENT);rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
            reg=r.json('research/runtime/coverage/register.json');b=next(b for b in reg['behaviors'] if b['id']==BID);s=next(s for s in reg['scenarios'] if s['id']==SID);builds={b['id']:b for b in reg['builds']}
            sources={n:r.digest(n) for n in required_sources(b)}
            result.update(success=True,scope=b['scope'],sources=parent['sources'],analysis_sources=sources,sources_stable=True,
                claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,builds),scenarios={SID:scenario_contract(s)})],
                parent=dict(record=PARENT,sha256=r.digest(PARENT)),matrix=dict(record=MATRIX,sha256=r.digest(MATRIX)),cases=cases,
                requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),
                passed_requirements=parent['passed_requirements']+2,pending_requirements=154-parent['passed_requirements'],newly_passed_requirements=2,
                original_game_runs=0,new_native_executions=0,new_original_oracle_runs=0,
                pan_windows=8,compared_pixels=sum(p['compared_pixels'] for c in cases for p in c['pans']),mismatching_pixels=0,
                historical_failures_retired=False,picking_resize_or_hotspot_closed=False,atomic_hud_snapshot_proved=False)
        except Exception as e:result['error']=str(e)
        print(json.dumps(dict(record=args.report,success=result['success'],error=result.get('error'),sha256=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))))
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
