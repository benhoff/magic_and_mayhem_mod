"""Strict diagnostic Save/Load witnesses; never progression acceptance."""
import hashlib
import re
import struct
from rendering_progression import require

HEADER = b'MNMCPS01' + struct.pack('<II', 1, 64)
FIELDS = ('slot','type','owner','active','health','x','y','z','behavior','action','target','mana')
RESTORED = ('slot','type','owner','active','health','x','y','z','mana')


def save_name(name):
    require(bool(re.fullmatch(r'checkpoint-[a-z0-9-]{1,64}', name)), 'Use a unique checkpoint- name')
    return name


def decode(data):
    require(data[:16] == HEADER and (len(data)-16)%64 == 0, 'Invalid/truncated checkpoint stream')
    rows=list(struct.iter_unpack('<16I', data[16:]))
    require(rows and len(rows)<=8*257, 'Empty/exhausted checkpoint stream')
    groups=[]
    for sequence,row in enumerate(rows,1):
        require(row[0]==sequence and row[2]>0, 'Invalid checkpoint sequence/thread')
        if row[1]==1:
            require(not groups or len(groups[-1]['actors'])==groups[-1]['capacity'], 'Incomplete checkpoint pool')
            require(len(groups)<8 and not any(row[8:]) and row[4]==len(groups)+1 and row[5] in (12,13) and 0<row[6]<=256 and row[7]==1,
                    'Invalid storage boundary')
            groups.append(dict(screen=row[5],capacity=row[6],thread=row[2],tick=row[3],actors=[]))
        else:
            require(row[1]==2 and groups, 'Actor without boundary')
            group=groups[-1]
            require(row[2:4]==(group['thread'],group['tick']) and row[4]==len(group['actors']) and row[4]<group['capacity'],
                    'Invalid checkpoint actor identity')
            require(row[7] or not any(row[5:]),'Inactive checkpoint payload must be zero')
            group['actors'].append(dict(zip(FIELDS,row[4:])))
    require(len(groups[-1]['actors'])==groups[-1]['capacity'] and len({g['thread'] for g in groups})==1,
            'Incomplete pool/changed storage owner')
    return groups


def compare(saved, loaded):
    require(saved['screen']==13 and loaded['screen']==12, 'Expected original Save then Load')
    before={a['slot']:a for a in saved['actors'] if a['active']}
    after={a['slot']:a for a in loaded['actors'] if a['active']}
    differences=[];other=[]
    for slot in sorted(before.keys()|after.keys()):
        a,b=before.get(slot),after.get(slot)
        if a is None or b is None:
            differences.append(dict(slot=slot,field='active_presence',saved=a,loaded=b));continue
        for field in FIELDS:
            if a[field]!=b[field]:
                (differences if field in RESTORED else other).append(dict(slot=slot,field=field,saved=a[field],loaded=b[field]))
    return dict(verified=not differences,active_creatures=len(before),fields=list(RESTORED),
                differences=differences,other_observed_differences=other,
                scope='Synchronous original storage-return active creature slot/type/owner/HP/position/mana only; no complete world, RNG, AI, projectile, resource or native parser equivalence')


def validate_capture(flow, route, data, save_bytes):
    from campaign_storage import validate_storage_flow, validate_mini_dispatch
    validate_storage_flow(flow);validate_mini_dispatch(flow)
    require(route.get('success') is True and route.get('checkpoint_ready') is True
            and route.get('simulation_writes') is False and route.get('semantic_outcome_accepted') is False,
            'Checkpoint route did not stop before an outcome')
    seeded=route.get('seed_reference')
    event='checkpoint-seed-loaded' if seeded else 'checkpoint-ready'
    ready=[s for s in flow['states'] if s.get('event')==event]
    require(len(ready)==1,'Missing checkpoint handoff')
    if seeded:require(ready[0].get('sha256')==seeded['save_sha256'] and ready[0].get('name')==seeded['save_name'],'Seed Load identity mismatch')
    groups=decode(data)
    require([g['screen'] for g in groups]==([12,13,12] if seeded else [13,12]), 'Unexpected storage boundary order')
    groups=groups[-2:]
    original=compare(*groups)
    require(original['verified'], 'Original Save/Load creature fields differ: '+str(original['differences'][:8]))
    heroes=[a for a in groups[0]['actors'] if a['active'] and a['type']==0 and a['owner']==0]
    require(len(heroes)==1 and 60<=heroes[0]['health']<0x80000000 and heroes[0]['mana']>=36*256,
            'Checkpoint lacks a healthy player with the36mana casting budget')
    require(len([a for a in groups[0]['actors'] if a['active'] and a['type']==24 and a['owner']==1 and 0<a['health']<0x80000000])==1,'Checkpoint lacks the living Camulos')
    require(len([a for a in groups[0]['actors'] if a['active'] and a['type']==14 and a['owner']==0 and 0<a['health']<0x80000000])==1,'Checkpoint requires one living owned Zombie')
    point=route['route']['points']['CF52 PoP']
    require(all(heroes[0][k]==point[k] for k in ('x','y','z')), 'Saved wizard left resupply point')
    require(save_bytes and hashlib.sha256(save_bytes).hexdigest()==flow.get('saved_sha256'), 'Saved bytes differ from original writer witness')
    return dict(verified=True,save_sha256=flow['saved_sha256'],save_bytes=len(save_bytes),wizard=heroes[0],
                storage=original,saved_snapshot=groups[0],diagnostic_only=True,
                fresh_campaign_progression_accepted=False)


def validate_restore(flow,data,save_bytes,reference):
    require(flow.get('success') is True and flow.get('normal_quit') is True and flow.get('stage')==12
            and flow.get('native_command_fallback') is False and not flow.get('native_error')
            and flow.get('native_recoveries')==0 and flow.get('resident_world_requested') is True,
            'Fresh-process checkpoint restore did not finish normally')
    require([c.get('phase') for c in flow.get('checkpoints',[])]==['after-load'], 'Restore reused a fresh-campaign route')
    for c in flow['checkpoints']:
        require(c.get('native_active') is True and c.get('fallback') is False and not c.get('error')
                and int(c['source_generation'])>=int(c['source_base'])+3 and c['frames']>c['frame_base']+10,
                'Fresh-process restore lacks a fresh native World')
    requests=[s for s in flow['states'] if s.get('event')=='mini-request']
    require(len(requests)==1 and requests[0].get('action')==15 and requests[0].get('accepted') is True,
            'Restore cleanup did not use native Mini Quit')
    groups=decode(data)
    require(len(groups)==1 and groups[0]['screen']==12, 'Expected one fresh-process original Load')
    original=compare(reference['saved_snapshot'],groups[0])
    require(original['verified'], 'Fresh-process restored creature fields differ: '+str(original['differences'][:8]))
    require(save_bytes and hashlib.sha256(save_bytes).hexdigest()==reference['save_sha256']==flow.get('saved_sha256'),
            'Fresh-process save hash mismatch')
    return dict(verified=True,save_sha256=reference['save_sha256'],save_bytes=len(save_bytes),
                storage=original,diagnostic_only=True,fresh_campaign_progression_accepted=False)
