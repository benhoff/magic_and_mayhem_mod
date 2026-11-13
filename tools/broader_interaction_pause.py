"""Public Space collection control bound to the actual observed World pause byte."""
import ast,inspect,time
from pathlib import Path
from rendering_pilot import require
from rendering_ui_acceptance import decode,decode_keys
from rendering_menu_acceptance import artifact
POLICY='public-Space-World-pause-byte-interaction-v1'

def live_ui(session):
    path=Path(session)/'ui-acceptance.bin'
    if not path.exists():return []
    raw=path.read_bytes()
    if len(raw)<16:return []
    raw=raw[:16+(len(raw)-16)//512*512];q=decode(raw,allow_pending=True)
    require(q['wire_version']==3,'Actual World pause byte requires UI witness version3')
    return q['rows']

def world_rows(rows):
    return [r for r in rows if r['kind']==1 and r['frame']and r['mode']in(0,1)]

def checked_pause_state(rows):
    selected=world_rows(rows);require(selected,'Missing actual World pause observation');row=selected[-1];wizard=row['wizard']
    require(wizard and wizard['slot']==wizard['type']==wizard['owner']==0 and wizard['active']and 0<wizard['health']<2**31,
            'Public pause collection lost the living wizard')
    return row

def set_pause(journey,wanted,label):
    require(type(wanted)is int and wanted in(0,1),'Invalid public pause target')
    before=checked_pause_state(live_ui(journey.session));acted=before['mode']!=wanted
    if acted:journey.held_key(label,0x20,.22,.6)
    deadline=time.monotonic()+5
    while time.monotonic()<deadline:
        after=checked_pause_state(live_ui(journey.session))
        require(after['thread']==before['thread']and after['frame']['nonce']==before['frame']['nonce'],
                'Public pause session identity changed')
        if after['mode']==wanted:
            journey.pause_transitions.append(dict(policy=POLICY,action=label,wanted=wanted,physical_Space_sent=acted,
                before_ui_row=before['ordinal'],after_ui_row=after['ordinal'],host_ns=time.monotonic_ns()))
            return after
        time.sleep(.05)
    raise ValueError('Original World pause byte did not adopt the public Space action')

def interaction_schedule(journey):
    journey.set_scaling('sharp');set_pause(journey,1,'interaction-pause')
    for mode in('sharp','integer'):
        if mode=='integer':journey.set_scaling(mode)
        require(checked_pause_state(live_ui(journey.session))['mode']==1,'Original World resumed during paused interaction collection')
        for n in range(2):journey.held_key(mode+'-center-'+str(n),ord('w'),.22,.6)
        for view in range(4):
            journey.pick_pair(mode+'-view-'+str(view));journey.held_key(mode+'-rotate-'+str(view),ord('.'),.22,.65)
        journey.held_key(mode+'-pan',0xff52,.22,.8);journey.pick_pair(mode+'-panned')
    resumed=set_pause(journey,0,'interaction-resume');deadline=time.monotonic()+5
    while time.monotonic()<deadline:
        rows=[r for r in world_rows(live_ui(journey.session))if r['ordinal']>=resumed['ordinal']and r['mode']==0]
        if len({r['counter']for r in rows})>=8:return
        require(journey.child.poll()is None,'Player exited before resumed simulation counters');time.sleep(.05)
    raise ValueError('Eight actual resumed simulation counters missing')

def selected_interaction_proof_source(source):
    tree=ast.parse(source);removed=initial=terminal=0
    old_if=ast.dump(ast.parse("if mode+'-resume' in windows:\n required.append(mode+'-resume');interval(mode+'-resume')").body[0],include_attributes=False)
    class Select(ast.NodeTransformer):
        def visit_If(self,node):
            nonlocal removed
            if ast.dump(node,include_attributes=False)==old_if:removed+=1;return None
            return self.generic_visit(node)
        def visit_Assign(self,node):
            nonlocal initial
            if len(node.targets)==1 and isinstance(node.targets[0],ast.Name)and node.targets[0].id=='required'and isinstance(node.value,ast.List)and not node.value.elts:
                initial+=1;return [node,*ast.parse("if 'interaction-pause' in windows:\n required.append('interaction-pause');interval('interaction-pause')").body]
            return self.generic_visit(node)
        def visit_Expr(self,node):
            nonlocal terminal
            if ast.dump(node,include_attributes=False)==ast.dump(ast.parse("required.append('open-results')").body[0],include_attributes=False):
                terminal+=1;return [*ast.parse("required.append('interaction-resume');interval('interaction-resume')").body,node]
            return self.generic_visit(node)
    tree=Select().visit(tree);require((removed,initial,terminal)==(1,1,1),'Original interaction schedule predicate changed')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'

def verify_pause(reader,acquired):
    require(acquired['success']and acquired['sources_stable'],'Incomplete public pause collection')
    rows=decode(artifact(reader,acquired,'ui-acceptance.bin'))
    require(rows['wire_version']==3,'Actual pause byte witness required');rows=rows['rows'];transitions=acquired['proof']['public_pause_collection']
    require(len(transitions)==2 and [(v['action'],v['wanted'])for v in transitions]==[('interaction-pause',1),('interaction-resume',0)],
            'Wrong public pause collection boundary sequence')
    key_trace=decode_keys(artifact(reader,acquired,'ui-acceptance.bin.keys'));key_proofs=[]
    for transition in transitions:
        require(transition['policy']==POLICY and type(transition['physical_Space_sent'])is bool,'Changed pause control policy')
        require(type(transition['before_ui_row'])is int and type(transition['after_ui_row'])is int
            and 1<=transition['before_ui_row']<=transition['after_ui_row']<=len(rows),'Invalid actual pause row indices')
        before=rows[transition['before_ui_row']-1];after=rows[transition['after_ui_row']-1]
        require(before['kind']==after['kind']==1 and before['frame']['nonce']==after['frame']['nonce']
            and before['thread']==after['thread']and after['mode']==transition['wanted']
            and transition['physical_Space_sent']==(before['mode']!=after['mode']),'False actual pause transition')
        actions=[v for v in acquired['actions']if v['action']==transition['action']]
        if transition['physical_Space_sent']:
            require(len(actions)==1 and actions[0]['symbol']==0x20 and actions[0]['keycode']>0
                and actions[0]['held_ms']==220 and actions[0]['down_up']and actions[0]['host_focus']['host_focused'],
                'Missing focused physical Space action')
            edges=[v for v in key_trace['rows']if v['code']==32 and (v['tick']-before['tick'])%2**32<2**31
                and (after['tick']-v['tick'])%2**32<2**31]
            require(len(edges)==2 and [v['down']for v in edges]==[True,False]
                and all(v['thread']==before['thread']==key_trace['owner']and v['owned']and v['input_sequence']>0 for v in edges),
                'Original Space down/up ownership not witnessed between pause bytes')
            key_proofs.append(dict(action=transition['action'],original_edges=edges))
        else:require(not actions and before['mode']==after['mode'],'Unexpected physical action for an already paused World')
    pause,resume=transitions;paused=[r for r in world_rows(rows)if pause['after_ui_row']<=r['ordinal']<=resume['before_ui_row']]
    require(pause['after_ui_row']<=resume['before_ui_row']<resume['after_ui_row'],'Reordered public pause collection boundaries')
    require(paused and all(r['frame']['nonce']==acquired['resident']['nonce']==paused[0]['frame']['nonce']
        and r['thread']==paused[0]['thread']for r in world_rows(rows)),'Mixed actual pause/source session')
    require(len(paused)>=3 and all(r['mode']==1 for r in paused)and len({r['counter']for r in paused})==1,
            'Simulation counter advanced during paused interaction capture')
    require(all(r['wizard']and r['wizard']['slot']==r['wizard']['type']==r['wizard']['owner']==0
        and r['wizard']['active']and 0<r['wizard']['health']<2**31 for r in paused),'Dead/changed wizard in paused collection')
    target_rows=[v for v in acquired['actions']if v['action']in('source-sprite-target','source-sprite-focus')]
    lo,hi=paused[0]['frame']['generation'],paused[-1]['frame']['generation']
    require(len(target_rows)==12 and all(lo<=r['source_generation']<=hi for r in target_rows if r['action']=='source-sprite-target'),
            'Interaction targets outside witnessed actual pause interval')
    resumed=[r for r in world_rows(rows)if r['ordinal']>=resume['after_ui_row']and r['mode']==0]
    require(len({r['counter']for r in resumed})>=8,'Eight actual resumed simulation counters missing')
    return dict(verified=True,policy=POLICY,actual_World_pause_offset='0xfd5a',input_mode_is_distinct=True,
        pause_transitions=transitions,original_Space_key_proofs=key_proofs,paused_source_range=[lo,hi],paused_Worlds=len(paused),
        paused_simulation_counter=paused[0]['counter'],resumed_simulation_counters=sorted({r['counter']for r in resumed}),
        original_memory_writes=False,gameplay_balance_changed=False,normal_retirement_still_required=True)
