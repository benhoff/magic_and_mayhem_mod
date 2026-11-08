"""Finite public Main/setup acceptance; native UI policy and original semantics."""
import hashlib,json,struct
from pathlib import Path
from retained_artifacts import read_artifact
from rendering_pilot import require,counter
from rendering_acceptance import requirements,validate
from coverage_claims import fingerprint
LEAVES={'journey.main-setup-menus.setup-change','journey.main-setup-menus.main-menu','journey.main-setup-menus.cancel-return'}
SCREENS=[3,10,3,22,14,22,14,25,14,25,14,22,14,22,3,22,3]
CALLBACKS=[[3,3],[10,1],[3,2],[22,2],[14,0],[22,2],[14,2],[25,1],[14,2],[25,0],[14,0],[22,2],[14,0],[22,3],[3,2],[22,3],[3,4]]


def artifact(reader,acquired,path):
    return read_artifact(reader,acquired,path,label='public-menu')


def semantic_setup(raw):
    def text(offset,size):
        field=raw[offset:offset+size];require(b'\0' in field,'Unterminated original setup field');return field.split(b'\0',1)[0].decode('cp1252')
    return dict(map=struct.unpack_from('<I',raw,160)[0],map_name=text(412,128),rules=list(struct.unpack_from('<13I',raw,168)),
        players=[dict(zip(('active','portrait','colour','handicap'),struct.unpack_from('<4I',raw,220+48*i)))|dict(name=text(236+48*i,32)) for i in range(4)])


def preserved_setup(setup):
    return dict(setup,players=[dict(v) if i==0 else {k:x for k,x in v.items() if k not in ('name','portrait')} for i,v in enumerate(setup['players'])])


def host_focus(value):
    return (value['host_focused'] is True and type(value['window']) is int and type(value['host_window']) is int
            and value['window']==value['host_window']>0)


def assess(acquired,callbacks):
    require(acquired['success'] is True and acquired['kind']=='native_public_main_setup_menu_acquisition'
            and acquired['sources_stable'] is True and acquired['original_manifest_verified_before_after'] is True
            and acquired['original_game_runs']==1 and acquired['world_start_intended'] is False
            and acquired['original_pixels_used_as_native_inputs'] is False and acquired['exit_code']==0
            and acquired['private_wine_stopped'] is True and acquired['prefix_pool']['returned'] is True
            and acquired.get('forced_private_prefix_stop',False) is False,
            'Incomplete nominal public-menu execution')
    cfg=acquired['configuration']
    require(type(cfg['software_flag']) is type(cfg['high_resolution']) is bool and cfg['skip_intros'] is False
            and cfg['terrain_lighting']=='native' and cfg['native_ui_layout']=='800x600-logical','Unsupported native menu policy')
    states=acquired['states'];require([v['screen'] for v in states]==SCREENS and [v['ack'] for v in states]==list(range(17)),
            'Missing exact original menu transitions')
    require(len({v['thread'] for v in states})==1 and states[0]['thread']>0
            and all(v['ready'] is True and v['status'] in (0,1) and v['handoff']==0 and v['sequence']>0 and v['sequence']%2==0 for v in states)
            and all(b['generation']>a['generation'] and b['host_ns']>a['host_ns'] for a,b in zip(states,states[1:])),
            'Original menu owner/generation/readiness differs')
    require(callbacks==CALLBACKS,'Original public-menu callbacks differ')
    require(states[1]['preferences'][2]==int(not cfg['high_resolution'])==acquired['proof']['preferences'][2],
            'High/Low context lacks actual original selector')
    initial,reopened,opened,cancelled,confirmed,returned=[states[i]['setup'] for i in (4,6,7,8,10,12)]
    require(preserved_setup(initial)==preserved_setup(reopened) and opened==cancelled and preserved_setup(initial)==preserved_setup(returned),'Cancelled draft applied or outer Cancel did not restore defaults')
    draft=acquired['proof']['cancelled_draft'];changes=acquired['proof']['applied_changes']
    require(len(draft)==len(changes)==3 and [v['index'] for v in draft]==[0,8,13]
            and [v['index'] for v in changes]==[0,8,13] and all(v['before']!=v['after'] for v in draft+changes),
            'Cancel/change did not exercise an actual different draft')
    expected=list(initial['rules']);expected[0]=int(changes[0]['after']);expected[8]=int(changes[1]['after'])
    require(opened['rules']==expected and opened['players'][0]['handicap']==int(changes[2]['after'])
            and opened['rules'][0]!=initial['rules'][0] and opened['rules'][8]!=initial['rules'][8]
            and opened['players'][0]['handicap']!=initial['players'][0]['handicap'],'Original settings did not change as requested')
    require(confirmed['map']==acquired['proof']['selected_map']['map']!=opened['map']
            and confirmed['map_name']==acquired['proof']['selected_map']['name'] and confirmed['rules']==opened['rules']
            and confirmed['players']==opened['players'] and acquired['proof']['map_cancel']['map']!=opened['map'],
            'Map selection/cancel/confirmation semantics differ')
    captures={v['label']:v for v in acquired['screenshots']}
    main=[]
    for label,index in [('main-ready-initial',0),('main-ready-after-preferences',2),('main-ready-returned',14),('main-ready-final',16)]:
        view=captures[label];targets=[v for v in view['nodes'] if v['role']=='button' and (v.get('accessible_id','').endswith('.mainMenuAction2') or v['name']=='Quick Battle')]
        require(view['state_index']==index and host_focus(view['host_focus']) and len(targets)==1
                and targets[0]['focused'] is True and targets[0]['enabled'] is True and targets[0]['showing'] is True
                and min(targets[0]['rect'][2:])>0 and view['host_ns']>=states[index]['host_ns'],'Ready Main visibility/focus missing')
        main.append(dict(label=label,state_index=index,sha256=view['sha256'],target=targets[0]))
    cancel_focus=[]
    for label,index,suffix in [('quick-after-cancel',5,'.quickBattleAction0'),('setup-cancel-preserved',6,'.singlePlayerPortrait0'),('map-cancel-preserved',8,'.singlePlayerPortrait0'),('setup-reentered',12,'.singlePlayerPortrait0')]:
        view=captures[label];targets=[v for v in view['nodes'] if v['role']=='button' and v.get('accessible_id','').endswith(suffix)]
        require(view['state_index']==index and host_focus(view['host_focus']) and len(targets)==1
                and targets[0]['focused'] is True and targets[0]['enabled'] is True and targets[0]['showing'] is True,'Cancel return focus missing')
        cancel_focus.append(dict(label=label,state_index=index,sha256=view['sha256'],target=targets[0]))
    physical=[v for v in acquired['actions'] if v['action'] in ('main-focused-space','returned-main-focused-space')]
    require(len(physical)==2 and all(v['physical']['symbol']==0x20 and v['physical']['down_up'] is True
            and host_focus(v['physical']['host_focus']) for v in physical),'Focused Main physical input effect missing')
    for key in ('setup-draft-cancel','map-cancel','setup-return','setup-cancel-again','quick-cancel','quick-return-again'):
        found=[v for v in acquired['actions'] if v['action']==key]
        require(len(found)==1 and found[0]['physical']['symbol']==0xff1b and found[0]['physical']['down_up'] is True
                and host_focus(found[0]['physical']['host_focus']),'Cancel lacked actual native physical input')
    retired=acquired['retirement'];world=acquired['resident'];graphics=retired['graphics_retirement']
    require(retired['state']=='completed' and retired['forwarded']==retired['worlds']==0
            and graphics['process_exit'] is True and graphics['resources_released'] is True and not graphics['surviving_owners']
            and all(v==0 for v in graphics['counts'].values()),'Menu ownership did not retire normally')
    require(int(world['completed'])==int(world['imports'])==int(world['verified_returns'])==0 and world['pending'] is False
            and world['normal_quit_retired'] is True and not world['error'],'Cancel started or retained World work')
    return dict(verified=True,original_callbacks=callbacks,original_thread=states[0]['thread'],main_ready=main,cancel_ready=cancel_focus,
        changed_rule_indices=[0,8],changed_handicap_player=0,changed_map=confirmed['map'],original_resolution_selector=states[1]['preferences'][2],
        original_quit=True,unintended_worlds=0,native_ui_policy='Native Qt layout, ready focus and actual keyboard outcomes; no original-menu pixel equality is claimed.')


def validate_artifacts(reader,acquired,callbacks):
    proof=assess(acquired,callbacks)
    for state in acquired['states']:
        raw=artifact(reader,acquired,state['wire_path']);w=struct.unpack_from('<40I',raw)
        require(raw[:8]==(('MNMMCMD' if w[2]<10 else 'MNMMCM')+str(w[2])).encode() and 2<=w[2]<=22 and len(raw)==w[3]
                and list(w[32:40])==[state[k] if k!='ready' else int(state[k]) for k in ('sequence','generation','screen','ready','ack','status','thread','handoff')],
                'Original immutable wire snapshot differs')
        if state['screen'] in (14,25):require(semantic_setup(raw)==state['setup'],'Original setup payload differs')
        if state['screen']==10:require(list(struct.unpack_from('<7i',raw,44724))==state['preferences'],'Original Preferences payload differs')
    from PIL import Image
    import io
    visible=[]
    main={v['label']:v for v in proof['main_ready']}
    for shot in acquired['screenshots']:
        raw=artifact(reader,acquired,shot['path']);require(hashlib.sha256(raw).hexdigest()==shot['sha256'] and raw[:8]==b'\x89PNG\r\n\x1a\n','Unbound native UI screenshot')
        image=Image.open(io.BytesIO(raw));require(image.size==(1280,1024),'Unexpected isolated host capture');image.verify()
        if shot['label'] in main:
            x,y,w,h=main[shot['label']]['target']['rect']
            require(x>=0 and y>=0 and x+w<=1280 and y+h<=1024,'Main focus rectangle escapes capture')
            pixels=list(Image.open(io.BytesIO(raw)).convert('RGB').crop((x,y,x+w,y+h)).getdata())
            painted=sum(max(p)>20 for p in pixels);light=sum(min(p)>120 for p in pixels)
            if painted>=len(pixels)*.9 and light>=100:
                visible.append(dict(label=shot['label'],sha256=shot['sha256'],rect=[x,y,w,h],painted_pixels=painted,light_pixels=light))
    require(visible,'No retained painted ready Main control; accessibility alone is not visual proof')
    proof['visible_main_presentations']=visible
    proof['paint_scope']='At least one retained ready Main control is visibly painted per context. Other snapshots can precede repaint; no all-transition paint simultaneity or original pixel equality is inferred.'
    return proof


def close_rows(plan,parent,runs):
    validate(plan);template=requirements(plan)
    require(parent['dossier_identity']==fingerprint(template) and parent['passed_requirements']==28
            and len(parent['requirements'])==156 and sum(v['state']=='passed' for v in parent['requirements'])==28
            and {v['id'] for v in parent['requirements'] if v['state']=='passed'}==set(parent['closed_leaf_ids'])
            and [(v['configuration'],v['id']) for v in parent['requirements']]==[(v['configuration'],v['id']) for v in template],
            'Changed accepted Main/setup parent inventory')
    expected={(s,h) for s in (False,True) for h in (False,True)}
    require(len(runs)==4 and {(v['configuration']['software_flag'],v['configuration']['high_resolution']) for v in runs}==expected
            and len({v['identity'] for v in runs})==4 and all(v['proof']['verified'] is True for v in runs),'Missing/duplicate menu domain')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id'] in LEAVES:
            require(row['state']=='pending','Menu leaf already closed')
            row.update(state='passed',admitted_execution_evidence=[dict(reason='Original setup changes/cancel and ready Main focus/physical input with zero unintended World and normal original Quit.',execution=v['acquisition'],native_ui_layout=[800,600],original_resolution_selector=v['proof']['original_resolution_selector'],scope='High/Low original preference contexts; no World is entered. Earlier accepted actual World-size proof is preserved in the parent.') for v in runs if v['configuration']['software_flag'] is (row['configuration']=='player-software')])
        rows.append(row)
    require(sum(v['state']=='passed' for v in rows)==34,'Main/setup closeout scope changed');return rows
