"""Same-owner original UI semantics joined to actual independently matched swaps."""
import hashlib
import importlib.util
from pathlib import Path
import struct
from rendering_ui_acceptance import decode,decode_keys
from rendering_interaction import decode as frames_decode
from rendering_menu_acceptance import artifact
from rendering_pilot import require


def complete_witness(raw):
    """Keep final Defeat/Realm World returns separate from sealed output.

    The live controller stops consulting the stream before ordinary Quit.
    Campaign teardown then returns twice without producing another World.
    Retain both exact records; neither receives frame/state/tooltip credit.
    """
    require(len(raw)>=16 and (len(raw)-16)%512==0,'Incomplete stopped UI witness')
    words=list(struct.iter_unpack('<128I',raw[16:]));split=len(words)
    for i,w in enumerate(words):
        if w[1]==1 and w[5]!=w[59]+(w[60]<<32):split=i;break
    decoded=decode(raw[:16+split*512]);terminal=words[split:]
    require(decoded['wire_version']==3 and len(terminal)==2 and [w[16] for w in terminal]==[6,4],
            'Missing/unknown original Defeat/Realm teardown transfers')
    last=words[split-1];identity=last[56:64];generation=identity[3]+(identity[4]<<32);previous=last[3];transfers=[]
    for i,w in enumerate(terminal,1):
        require(w[0]==split+i and w[1]==1 and not w[2] and w[4]==decoded['owner']
                and w[5]==generation+i and w[7]==generation and w[56:64]==identity
                and (w[3]-previous)%2**32<2**31 and w[50]&~63==0 and w[10]==0
                and w[12]==0 and w[15]==1,'Foreign/malformed unsealed terminal World return')
        previous=w[3];transfers.append(dict(ordinal=w[0],thread=w[4],world_returns=w[5],last_sealed_generation=generation,top=w[16],tick=w[3],counter=w[11],raw_sha256=hashlib.sha256(raw[16+(split+i-1)*512:16+(split+i)*512]).hexdigest(),output_credit=False))
    decoded['terminal_transfers']=transfers
    return decoded


def sealed_worlds(frames):
    rows=[r for r in frames['rows'] if r['kind']==1 and r['frame'] and r['world_return']==r['frame']['generation']]
    result={r['frame']['generation']:r for r in rows}
    require(len(result)==len(rows),'Duplicate sealed World frame witness')
    return result


def restored_after_motion(rows,frame_rows,lo,hi,owner,shown):
    motions=[r for r in frame_rows if r['kind']==3 and r['thread']==owner and r['mask']&32
             and r['mouse'][:2]==[40,100] and lo<r['world_return']<=hi]
    require(motions,'Tooltip close has no actual original away-motion callback')
    motion=motions[0]
    restored=[r for r in rows if r['frame']['generation']>motion['world_return']]
    require(restored and all(r['tooltip']['visible']==0 for r in restored)
            and len({r['frame']['generation'] for r in restored if r['frame']['generation'] in shown})>=3,
            'Tooltip restoration lacks three shown original post-motion generations')
    return motion,restored,[r for r in rows if r not in restored]


def mini_ownership(reader,a,nonce):
    phases=[]
    for label in ('mini-paused-begin','mini-paused-end'):
        raw=artifact(reader,a,label+'-render-input.bin');require(len(raw)==1088 and raw[:8]==b'MNMINK01','Wrong Mini input wire')
        w=struct.unpack('<272I',raw)
        require(w[2:4]==(1,1088) and w[4]>0 and not w[4]&1 and w[5]==0
                and not any(v&0x80000000 for v in w[16:]),'Mini retains active native input or held keys')
        media=artifact(reader,a,label+'-native-media.bin');require(len(media)==2048 and media[:8]==b'MNMMED01','Wrong Mini media wire')
        m=struct.unpack_from('<16I',media);response=struct.unpack_from('<5I',media,512)
        require(m[2:4]==(1,2048) and not m[4]&1 and not response[4]&1
                and m[5]==response[0]==response[3]>0 and response[1] in (3,4)
                and m[12]==0,'Mini has a pending/failed/cancelled-by-producer native media request')
        source=artifact(reader,a,label+'-world-continuous.bin');h=struct.unpack_from('<32I',source)
        require(source[:8]==b'MNMCON06' and h[2:6]==(6,128,len(source),3)
                and h[6]+(h[7]<<32)==int(nonce) and h[8:12]==(1,1,0,0),'Mini has a foreign/cancelled source owner')
        stride=(len(source)-128)//2;slots=[]
        for i in range(2):
            state,reserved,lo,hi=struct.unpack_from('<4I',source,128+i*stride)
            require(reserved==0 and state in range(9),'Mini source slot ownership invalid')
            slots.append(dict(state=state,generation=lo+(hi<<32)))
        if label.endswith('end'):require(all(v['state'] in (0,1,5,8) for v in slots),'Native source work remains pending at the end of Mini dwell')
        phases.append(dict(label=label,input_sequence=w[4],input_active=0,held_keys=0,
            media_request=m[5],media_status=response[1],source_nonce=nonce,slots=slots,
            raw_sha256={name:hashlib.sha256(body).hexdigest() for name,body in [('input',raw),('media',media),('source',source)]}))
    require(phases[0]['media_request']==phases[1]['media_request'],'Mini unexpectedly changed the media request')
    return dict(verified=True,phases=phases,scope='No pending native movie/file-sound request; original selected music/voice backend policy is retained, not replaced or muted by this observer.')


def glyph_sources(a,tooltips):
    """Match actual captured glyph source packets to the exact installed SFT."""
    from resident_capture import records
    path=Path(a['session'])/'game/Sprites/Tooltip Text.sft';raw=path.read_bytes()
    require(hashlib.sha256(raw).hexdigest()==a['installation_inputs']['Sprites/Tooltip Text.sft']['sha256'],'Tooltip font differs from the actual installation')
    h=struct.unpack_from('<10I',raw);require(raw[:4]==b'SFT\0' and h[1]==len(raw) and h[2]==3 and 0<h[3]<=256,'Unsupported tooltip font')
    at=40+(768 if h[8] else 0)+h[4]*h[9]*8;base=at+h[3]*4;glyphs={}
    for i in range(h[3]):
        offset=base+struct.unpack_from('<I',raw,at+i*4)[0];size=struct.unpack_from('<I',raw,offset)[0]
        require(40<=size<=len(raw)-offset,'Unclosed font glyph')
        frame=bytearray(raw[offset:offset+size]);struct.pack_into('<I',frame,28,0)
        glyphs.setdefault(hashlib.sha256(frame).hexdigest(),set()).add(chr(i+33))
    spec=importlib.util.spec_from_file_location('ui_sources',Path(__file__).with_name('inspect-canvas-producers.py'))
    decoder=importlib.util.module_from_spec(spec);spec.loader.exec_module(decoder)
    bounds=[(max(1,t['entry']['world_returns']),t['first']['frame']['generation']) for t in tooltips]
    by_case=[{} for _ in tooltips]
    for generation,chunk,body,request in records(Path(a['temporary_root'])/'worlds.bin'):
        selected=[i for i,(lo,hi) in enumerate(bounds) if lo<=generation<=hi]
        if not selected:continue
        _,rows=decoder.decode(body,complete=False)
        for f,payload in rows:
            if f[2]!=8:continue
            possibilities=glyphs.get(hashlib.sha256(payload[:f[19]]).hexdigest())
            if not possibilities:continue
            for i in selected:by_case[i].setdefault(f[3],[]).append(dict(characters=sorted(possibilities),generation=generation,chunk=chunk,sequence=f[1],x=f[8],y=f[9],source_sha256=hashlib.sha256(payload[:f[19]]).hexdigest()))
    result=[]
    for t,groups in zip(tooltips,by_case):
        expected=''.join(c for c in t['text'] if not c.isspace());matches=[]
        for surface,rows in groups.items():
            for i in range(len(rows)-len(expected)+1):
                selection=rows[i:i+len(expected)]
                if all(c in r['characters'] for c,r in zip(expected,selection)):matches.append(dict(surface=surface,glyphs=selection))
        require(matches,'Actual tooltip text has no matching complete captured SFT glyph sequence')
        result.append(dict(text=t['text'],font_sha256=hashlib.sha256(raw).hexdigest(),visible_glyphs=len(expected),matches=matches))
    return result


def assess(reader,a,compared,observed):
    require(compared['comparison']['success'] and compared['comparison']['mismatching_pixels']==0,
            'UI proof has no independent complete native output')
    decoded=complete_witness(artifact(reader,a,'ui-acceptance.bin'));rows=decoded['rows']
    require(decoded['wire_version']==3,'Legacy UI pilot has no admitted original World-pause byte')
    frames=frames_decode(artifact(reader,a,'frame-input.bin'))
    worlds=sealed_worlds(frames)
    ui=[r for r in rows if r['kind']==1 and r['frame']['generation']==r['world_returns']]
    key_files=[v for v in a['artifacts'] if Path(v['path']).name=='ui-acceptance.bin.keys']
    keys=decode_keys(artifact(reader,a,'ui-acceptance.bin.keys')) if key_files else None
    if keys:require(keys['owner']==decoded['owner'],'UI key polls belong to another original owner')
    require(ui and decoded['owner']==frames['owner'] and all(r['frame']['nonce']==frames['nonce'] for r in rows if r['frame']),
            'UI/source owner or nonce differs')
    matched={m['paint']:m for m in compared['comparison']['comparisons']}
    shown={}
    for paint in observed['swaps']:
        p,body=observed['paints'][paint];g=int(p['world'])
        if not g:continue
        m=matched[paint]
        require(m['world']==g and m['mismatching_pixels']==0 and m['actual_sha256']==m['expected_sha256']
                and p['native_gpu_frame'] and p['gpu_read_complete'] and p['pixels_captured'], 'Uncompared UI swap')
        shown.setdefault(g,[]).append(dict(paint=paint,commands=p['commands'],pixels=m['pixels'],sha256=m['actual_sha256']))
    pools={};addresses={}
    for r in ui:
        g=r['frame']['generation'];f=worlds[g]
        require(f['thread']==r['thread'] and f['tick']<=r['tick'] and f['world_return']==r['world_returns']==g
                and f['mode']==r['input_mode'] and r['wizard'] and f['tracked']
                and (f['tracked']['slot'],f['tracked']['type'],f['tracked']['owner'])
                    ==(r['wizard']['slot'],r['wizard']['type'],r['wizard']['owner'])
                and f['tracked']['health']==r['wizard']['health'] and f['tracked']['flag68']==r['wizard']['selected'],
                'UI original frame/actor state differs from the independent frame witness')
        pool,address=f['tracked']['pool'],r['wizard']['address']
        require(pools.setdefault(pool,address)==address and addresses.setdefault(address,pool)==pool,'UI actor pool/address lineage changed')
    def marker(name,phase):
        found=[m for m in a['camera_actions'] if m['action']==name and m['phase']==phase]
        require(len(found)==1,'Missing/duplicate UI physical boundary: '+name+'/'+phase);return found[0]
    def interval(name):
        lo,hi=marker(name,'released'),marker(name,'settled')
        selected=[r for r in ui if lo['frame_generation']<r['frame']['generation']<=hi['frame_generation']]
        require(selected and len({r['frame']['generation'] for r in selected if r['frame']['generation'] in shown})>=3,
                'UI action has fewer than three actually shown source generations: '+name)
        return selected
    tooltip_cases=[]
    for suffix in ('','-after-cast'):
        opening=interval('tooltip-open'+suffix);closing=interval('tooltip-close'+suffix)
        visible=[r for r in opening if r['tooltip']['visible']==1]
        require(len(visible)>=3,'Tooltip visible interval missing')
        lo=marker('tooltip-close'+suffix,'released')['frame_generation'];hi=marker('tooltip-close'+suffix,'settled')['frame_generation']
        motion,restored,transient=restored_after_motion(closing,frames['rows'],lo,hi,decoded['owner'],shown)
        first,last=visible[0],visible[-1]
        returns=[r for r in rows if r['kind']==3 and r['ordinal']<first['ordinal'] and r['tooltip']['pending']==1]
        require(returns,'Tooltip lacks its actual completed text preparation callback')
        returned=returns[-1];entries=[r for r in rows if r['kind']==2 and r['call']==returned['call']]
        require(len(entries)==1,'Tooltip original preparation entry absent');entry=entries[0]
        text=entry['tooltip']['text'];wizard=entry['wizard']
        require(wizard and wizard['type']==wizard['owner']==wizard['slot']==0 and wizard['active'] and wizard['health']>0
                and text==f"Health : {wizard['health']}\nMaximum Mana : {wizard['maximum_mana']}\nCurrent Mana : {wizard['mana']>>8}",
                'Known portrait tooltip text differs from the actual original actor values')
        require(entry['argument']==returned['argument']==0x657fe8 and entry['tooltip']['visible']==0
                and text and all(r['tooltip']['text']==text for r in visible)
                and all(r['tooltip']['font']==0x6df1c0 and r['tooltip']['font_raw']>0 and r['tooltip']['surface']>0
                        and r['tooltip']['width']>0 and r['tooltip']['height']>0 for r in visible),
                'Tooltip input/font/resource identity not admitted')
        tooltip_cases.append(dict(verified=True,text=text,prepared_call=entry['call'],entry=entry,returned=returned,
                    visible_source_generations=[r['frame']['generation'] for r in visible],
                    restored_source_generations=[r['frame']['generation'] for r in restored],
                    original_close_motion=motion,close_transition_generations=[r['frame']['generation'] for r in transient],first=first,last=last))
    tooltip=dict(verified=True,cases=tooltip_cases,glyph_sources=glyph_sources(a,tooltip_cases))
    paused=interval('space-pause');resumed=interval('space-resume')
    mode1=[r for r in paused if r['mode']==1];mode0=[r for r in resumed if r['mode']==0]
    require(len(mode1)>=3 and mode1[-1]['tick']-mode1[0]['tick']>=1000
            and len({r['counter'] for r in mode1})==1 and len({r['counter'] for r in mode0})>=2,
            'Original pause/resume counters lack a bounded stable/advancing interval')
    mini=[r for r in rows if r['kind']==4 and r['menu']['top']==17]
    # Separate the Cancel dwell from the later ordinary Quit visit.
    begin=next(v for v in a['actions'] if v['action']=='mini-paused-begin')['ui_row']
    end=next(v for v in a['actions'] if v['action']=='mini-paused-end')['ui_row']
    mini=[r for r in mini if begin<=r['ordinal']<=end]
    require(len(mini)>=3 and mini[-1]['tick']-mini[0]['tick']>=1000
            and len({r['counter'] for r in mini})==1 and len({r['world_returns'] for r in mini})==1,
            'Mini dwell lacks suspended original World dispatch')
    pause=dict(verified=True,paused_first=mode1[0],paused_last=mode1[-1],resumed_first=mode0[0],resumed_last=mode0[-1],
               mini_first=mini[0],mini_last=mini[-1],all_simulation_pause_recovered=False)
    spec=importlib.util.spec_from_file_location('ui_menu_events',Path(__file__).with_name('test-menu-observer.py'))
    events_module=importlib.util.module_from_spec(spec);spec.loader.exec_module(events_module)
    events=events_module.decode(artifact(reader,a,'events.bin'))
    callbacks=[[v['menu_id'],v['argument']] for v in events if v['event']==3]
    require(callbacks==[[3,3],[10,1],[3,0],[18,65537],[18,0],[17,4],[17,3],[6,21],[3,4]],
            'Unexpected original fresh Adept campaign/Cancel/Quit/Continue callbacks')
    cancelled=[v for v in events if v['event']==3 and v['menu_id']==17 and v['argument']==4]
    require(len(cancelled)==1 and cancelled[0]['thread']==decoded['owner'] and cancelled[0]['object']==0x6a5088,
            'Missing unique original campaign Mini Cancel callback')
    cancel=cancelled[0]
    entries=[v for v in events if v['event']==2 and v['menu_id']==17 and v['argument']==4 and v['sequence']<cancel['sequence']]
    resumes=[v for v in events if v['event']==20 and v['sequence']>cancel['sequence'] and v['active_screen']==0x6cbb78]
    ready=[v for v in events if v['event']==16 and v['sequence']<cancel['sequence'] and v['menu_id']==17
           and v['initialized']==1 and v['argument']==2 and v['result']==5 and v['active_screen']==0x6a5088]
    require(len(entries)==1 and resumes and ready and all(v['thread']==decoded['owner'] for v in (entries+resumes+ready)),
            'Mini ready/Cancel entry/original World resume ownership missing')
    pause['mini_original_callbacks']=dict(ready=ready[-1],entry=entries[0],returned=cancel,resumed=resumes[0])
    transitions=[(p,q) for p,q in zip(ui,ui[1:]) if p['mode']!=q['mode']]
    lo=marker('space-pause','begin')['frame_generation'];hi=marker('space-resume','settled')['frame_generation']
    transitions=[(p,q) for p,q in transitions if lo<=p['frame']['generation'] and q['frame']['generation']<=hi]
    require([(p['mode'],q['mode']) for p,q in transitions]==[(0,1),(1,0)],'Unexpected original World pause transitions')
    deliveries=[]
    for (p,q),name in zip(transitions,('space-pause','space-resume')):
        begin,released=marker(name,'begin'),marker(name,'released')
        actions=[v for v in a['actions'] if v['action']==name]
        require(len(actions)==1 and actions[0]['symbol']==32 and actions[0]['down_up'] is True
                and actions[0]['keycode']>0 and actions[0]['held_ms']==220 and actions[0]['host_focus']['host_focused']
                and begin['frame_generation']<=p['frame']['generation']<q['frame']['generation']<=released['frame_generation']+1,
                'Original pause transition is outside its unique physical Space delivery')
        deliveries.append(dict(action=actions[0],begin=begin,released=released,original_before=p,original_after=q))
    require(mode1[-1]['accepted_sequence']>mode1[0]['accepted_sequence'],'Pause stopped required render delivery')
    pause.update(physical_space_transitions=deliveries,optional_api_key_receipts=keys,
        original_key_message_handler_recovered=False,
        key_scope='Physical XTest Space press/release through current public Qt forwarding, bounded by same-owner original pause-byte changes and stable/advancing counters. An absent optional API journal supplies no API-poll or original message-handler credit.',
        mini_ownership=mini_ownership(reader,a,frames['nonce']))
    mana_values=sorted({r['wizard']['mana'] for r in ui})
    health_values=sorted({r['wizard']['health'] for r in ui})
    health_updates=[]
    for entry in (r for r in rows if r['kind']==5):
        returned=next(r for r in rows if r['kind']==6 and r['call']==entry['call'])
        if not entry['wizard'] or not entry['widget'] or entry['widget']['actor']!=entry['wizard']['address']:continue
        require(returned['widget'] and returned['wizard'] and entry['argument']==returned['argument']==entry['wizard']['health']
                and returned['widget']['cached_health']==returned['wizard']['health']==entry['argument']
                and entry['widget']['maximum']==returned['widget']['maximum']>0
                and returned['widget']['actor']==entry['widget']['actor'],'Original HUD health callback did not update the actual wizard widget')
        health_updates.append(dict(entry=entry,returned=returned))
    require(health_updates,'Original HUD health widget has no admitted state update')
    hud=dict(verified=tooltip_cases[1]['entry']['wizard']['mana']<tooltip_cases[0]['entry']['wizard']['mana']
             and tooltip_cases[1]['text']!=tooltip_cases[0]['text'] and sorted({r['wizard']['selected'] for r in ui})==[0,1],mana_values=mana_values,health_values=health_values,
             selection_states=sorted({r['wizard']['selected'] for r in ui}),health_updates=health_updates)
    return dict(verified=bool(tooltip['verified'] and pause['verified'] and hud['verified']),
                tooltip=tooltip,pause=pause,hud=hud,original_owner=decoded['owner'],source_nonce=frames['nonce'],
                source_generations=len(ui),world_presentations=sum(len(v) for v in shown.values()),
                terminal_transfers=decoded['terminal_transfers'],original_callbacks=callbacks,
                compared_pixels=sum(p['pixels'] for v in shown.values() for p in v),mismatching_pixels=0)
