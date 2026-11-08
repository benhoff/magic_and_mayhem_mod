"""Actor/source/original/native joins for actual attachment and melee output."""
from collections import Counter, defaultdict
import struct
import tempfile
from pathlib import Path
import numpy as np

from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, require,History,commands
from resident_capture import records
from scene_actor_observation import decode, crc32c
from scene_effects import module

animation=module('close-rendering-animation-auxiliary')
blend,base=animation.blend,animation.base
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/attachment-combat-results-20261107/'
PARENT='research/runtime/rendering-scene-effects-results-20261107/admission-03.json'


def actor(frame,slot):
    return next((a for a in frame['actors'] if a[0]==slot),None)


def melee_windows(frames,events):
    """Only exact adjacent same-thread event/health brackets qualify."""
    result=[]
    for before,after in zip(frames,frames[1:]):
        if not before['validity']&after['validity']&2:continue
        if after['generation']!=before['generation']+1 or any(before[k]!=after[k] for k in ('thread','nonce','canvas','extent')):continue
        if before['pool_epoch']!=after['pool_epoch'] or before['view']!=after['view']:continue
        candidates=[r for r in events if before['gameplay_sequence']<r[0]<=after['gameplay_sequence'] and r[1]==2]
        if len(candidates)!=1:continue
        e=candidates[0]
        if not (e[12]<e[11] and (e[4:7]==(0,0,0) or e[8:11]==(0,0,0)) and e[2]==after['thread']):continue
        if not ((e[3]-before['tick'])&0xffffffff)<=((after['tick']-before['tick'])&0xffffffff)<5000:continue
        a,b=actor(before,e[8]),actor(after,e[8]);sa,sb=actor(before,e[4]),actor(after,e[4])
        if any(v is None for v in (a,b,sa,sb)):continue
        if not(a[1:3]==b[1:3]==e[9:11] and sa[1:3]==sb[1:3]==e[5:7]
               and (a[4],b[4],a[3],b[3])==(e[11],e[12],e[13],e[14])
               and sa[3]and sb[3]and(sa[4],sb[4])==(e[7],e[7])):continue
        result.append(dict(before=before['generation'],after=after['generation'],event=list(e)))
    return result


def regular_sources(frame,rasters,crc_cache):
    """Normal opaque calls only; temporary entry shaders earn no body credit."""
    wanted={d[11] for d in frame['draws']}
    by_key=defaultdict(list)
    for f,body,descriptor in rasters:
        if f[14]!=0 or f[15] not in (1,2,3) or f[17]!=1 or f[21]!=0 or f[19] not in wanted:continue
        key=base.sha(body[:f[19]])
        if key not in crc_cache:crc_cache[key]=crc32c(body[:f[19]])
        by_key[(f[19],crc_cache[key],f[8],f[9],f[17])].append((f,body,descriptor))
    result=defaultdict(list)
    queue_keys=Counter((d[11],d[12],d[5],d[6],d[18]) for d in frame['draws'] if d[7] in (0,31))
    for d in frame['draws']:
        # For these original normal dispatcher paths, a non-sentinel upper
        # WORD at queue+28 adjusts Y before the raster. Leave that caller open.
        if d[7] not in (0,31) or d[9]>>16!=0x8ad0 or not d[18]:continue
        key=(d[11],d[12],d[5],d[6],d[18])
        if queue_keys[key]!=1:continue
        matches=by_key.get(key,[])
        selected=split_normal_source(d,matches)
        if selected is not None:result[(d[0],d[1])].append((d,*selected))
    return result


def split_normal_source(draw,matches):
    """Admit one normal source or its exact two original roof-clip halves.

    Both halves remain covered by the complete source/original/native proof.
    Pick the half with the largest sprite intersection only after identifying
    the two distinct producer callers and their complementary clip rectangles.
    """
    expected=(draw[8]&0xffff)-0x10000 if draw[8]&0x8000 else draw[8]&0xffff
    if any(f[16]!=(expected&0xffffffff) for f,b,d in matches):return None
    if len(matches)==1:return matches[0]
    if len(matches)!=2:return None
    ordered=sorted(matches,key=lambda row:row[0][11]);a,b=[row[0] for row in ordered]
    if {a[22],b[22]}!={0x500512,0x500a4e}:return None
    if not(a[3:10]==b[3:10] and a[10]==b[10]==0 and a[12]==b[12]==a[5]
           and a[11]==0 and a[13]==b[11] and b[13]==a[6]):return None
    def area(row):
        f,body,descriptor=row
        w,h,ox,oy=struct.unpack_from('<4i',body,4)
        left,top=f[8]-ox,f[9]-oy
        return max(0,min(left+w,f[12])-max(left,f[10]))*max(0,min(top+h,f[13])-max(top,f[11]))
    return max(ordered,key=area)


def unique_body(frame,joined,slot):
    a=actor(frame,slot)
    if a is None or not a[12]&1:return None
    if sum(v[16]==a[16] for v in frame['actors'])!=1:return None
    rows=joined.get((slot,0),[])
    return rows[0] if len(rows)==1 else None


def attachment_pairs(frame,joined):
    pairs=[]
    for a in frame['actors']:
        parent=unique_body(frame,joined,a[0])
        if parent is None:continue
        for role in range(1,12):
            if role==2 or not a[12]&(1<<role):continue  # Overlay caller not recovered here.
            gate=a[184+role]
            if not gate or role not in (5,) and not a[4]:continue
            point=7;half=0;branch='first-point'
            if role==1:
                if a[14]==3:
                    if a[13]==2:point=9;branch='second-point'
                    else:half=None;branch='first-point-half-child-height'
                elif a[14]!=1:continue
            for child in joined.get((a[0],role),[]):
                if parent[1][1]==child[1][1]:continue
                yhalf=child[0][14]>>1 if half is None else half
                expected=((parent[0][5]-a[30]+a[28+point]+a[28+11*role+2])&0xffffffff,
                          (parent[0][6]-a[31]+a[28+point+1]+a[28+11*role+3]+yhalf)&0xffffffff)
                if child[0][5:7]==expected:
                    pairs.append(dict(slot=a[0],role=role,branch=branch,gate=gate,actor=a,
                                      parent=parent,child=child,expected=expected))
    uses=Counter((p['child'][0][2],p['child'][1][1]) for p in pairs)
    return [p for p in pairs if uses[(p['child'][0][2],p['child'][1][1])]==1]


def source_points(joined,writers,original):
    raw,f,body,descriptor=joined
    points=animation.visible_points(f,writers,original)
    result=[]
    for point in points:
        y,x=divmod(point,f[5]);word=blend.source_word(f,body,x,y)['word']
        require(int(original[y,x])==word,'Actor source WORD differs from independent original')
        if not word:continue
        result.append(dict(world_pixel=point,original_word=word))
    if len(result)<8:return None
    if len(result)>64:
        result=[result[int(i)]for i in np.linspace(0,len(result)-1,64,dtype='int64')]
    return dict(fields=list(f),descriptor=list(descriptor),payload_sha256=base.sha(body),
                raw_queue=list(raw),samples=result)


def native_points(proof,compared,observed,originals,states):
    wanted={}
    for state in states:
        wanted.setdefault(state['generation'],{})
        for label,source in state['sources'].items():wanted[state['generation']][label]=source['samples']
    rows=scene_viewport_samples(proof,observed,originals,wanted)
    require({(v['generation'],v['kind']) for v in rows}=={(g,k+'-'+str(i)) for g,values in wanted.items() for k in values for i in range(8)},
            'Selected actor/attachment points are missing from actual native output')
    for g in wanted:
        require(len({r['paint'] for r in rows if r['generation']==g})==1,'Actor composition points split across paints')
    matches={v['paint']:v for v in compared['comparison']['comparisons']}
    for row in rows:
        m=matches[row['paint']]
        require(m['world']==row['generation'] and m['lease']==row['lease'] and m['mismatching_pixels']==0,
                'Actor point lost complete native comparison binding')
    return rows


def visible_native_points(tags,paint,wanted):
    """Select eight distinct source points per body before reading native RGBA.

    Scene writers supply candidates; final composition lineage removes points
    covered by the cursor/HUD. Every body must still have eight points in this
    same complete paint. Color coincidence cannot restore lost lineage.
    """
    fw,fh=paint['frame_width'],paint['frame_height'];iw,ih=paint['image_width'],paint['image_height']
    require(tags.shape==(fh,fw)and paint['scaling']in(0,2)and min(iw,ih,fw,fh)>0 and
            0<=paint['left']<=paint['width']-iw and 0<=paint['top']<=paint['height']-ih,
            'Unsupported actor viewport geometry')
    xs=((2*np.arange(iw,dtype='int64')+1)*fw)//(2*iw)
    ys=((2*np.arange(ih,dtype='int64')+1)*fh)//(2*ih)
    selected=[]
    for kind,choices in wanted.items():
        require(len({v['world_pixel']for v in choices})==len(choices),'Duplicate actor source point')
        found=[]
        for source in choices:
            chosen=None
            for position in np.flatnonzero(tags==source['world_pixel']):
                y,x=divmod(int(position),fw);vx=np.flatnonzero(xs==x);vy=np.flatnonzero(ys==y)
                if len(vx)and len(vy):
                    chosen=(x,y,paint['left']+int(vx[len(vx)//2]),paint['top']+int(vy[len(vy)//2]));break
            if chosen is not None:
                found.append((kind+'-'+str(len(found)),source,chosen))
                if len(found)==8:break
        if len(found)!=8:return None
        selected.extend(found)
    return selected


def scene_viewport_samples(proof,observed,originals,wanted):
    """Exact Sharp/Integer pixel-center mapping on the compared native swaps.

    Coordinate lineage and all requested source points select one complete
    paint before opening its pixels. This uses the same nearest pixel-center
    geometry independently checked across the entire viewport by the parent
    comparison; integer enlargement is a supported special case.
    """
    history=History(originals);parents={p['paint']:p for p in proof['presented_worlds']}
    observations=defaultdict(list)
    for ordinal in observed['swaps']:
        paint,body=observed['paints'][ordinal];generation=int(paint['world'])
        if generation in wanted and parents[ordinal]['source_import_live']:
            observations[paint['commands']].append((paint,body))
    require(observations,'No current-import actor output')
    result=[];completed=set();last=max(observations)
    for op,sequence,payload in commands(observed['commands']):
        history.apply(op,sequence,payload)
        for paint,body in observations.get(sequence,[]):
            generation=int(paint['world'])
            if generation in completed:continue
            sealed=history.sealed
            if sealed is None or sealed['tags']is None:continue
            if paint['mode']==0:
                if history.display[0]!=paint['surface']or sealed['surface']!=paint['surface']or sealed['generation']!=generation:continue
            elif sealed['generation']!=int(paint['sealed_world']):continue
            tags=sealed['tags'];fw,fh=paint['frame_width'],paint['frame_height']
            if tags.shape!=(fh,fw):continue
            selected=visible_native_points(tags,paint,wanted[generation])
            if selected is None:continue
            raw=body()if callable(body)else body
            require(base.sha(raw)==parents[int(paint['paint'])]['sha256'],'Actor native body differs from complete comparison')
            for kind,source,(x,y,vx,vy)in selected:
                offset=4*(vy*paint['width']+vx);actual=list(raw[offset:offset+4]);expected=blend.rgba(source['original_word'])
                require(actual==expected,'Actual Sharp/Integer actor output pixel differs from original')
                result.append(dict(kind=kind,generation=generation,paint=int(paint['paint']),lease=int(paint['lease']),
                    command_sequence=sequence,scaling=paint['scaling'],target_pixel=[x,y],viewport_pixel=[vx,vy],
                    lineage_world_pixel=int(tags[y,x]),original_rgba=expected,actual_rgba=actual,source=source))
            completed.add(generation)
        if sequence==last:break
    return result


def combat_pose_change(window,states):
    """Original melee engagement plus a changed rendered body frame at impact."""
    if not all(states):return False
    event=window['event'];labels={'actor-0','actor-'+str(event[4]if event[4]else event[8])}
    if not all(set(s['sources'])>=labels for s in states):return False
    source=str(event[4]);target=event[8]
    if not all(s['actors'][source][9:12]==[3,10,target]for s in states):return False
    return any(states[0]['sources'][label]['raw_queue'][11:13]!=states[1]['sources'][label]['raw_queue'][11:13]
               for label in labels)


def recorded_scene_repository(acquired):
    """Recognize explicit --repo and the recorded repository-local launcher."""
    command=acquired.get('command',[])
    require(command and command.count('--repo')<=1,'Missing/ambiguous recorded scene repository')
    if '--repo'in command:
        index=command.index('--repo')+1
        require(index<len(command),'Missing recorded scene repository value')
        return Path(command[index])
    launcher=Path(command[0])
    require(launcher.is_absolute()and launcher.name=='run-native.sh'and
            acquired.get('sources',{}).get('run-native.sh'),
            'Missing recorded scene repository or pinned launcher')
    return launcher.parent


def session_artifact_root(acquired,name):
    """Use the retained producer's repository, independent of this reader."""
    root=recorded_scene_repository(acquired);value=Path(acquired[name])
    require(root.is_absolute()and value.is_absolute()and
            '..'not in root.parts and '..'not in value.parts,'Invalid recorded scene root')
    relative=value.relative_to(root)
    require(relative.parts,'Scene artifact root cannot be the whole repository')
    return relative.as_posix()


def scoped_artifacts(acquired):
    """A directory census can retain another overlapping run; never join it.

    Only the named public output, native owner and original session belong to
    this case. Keep the immutable acquisition and all excluded raw artifacts.
    """
    roots=[]
    for name in('temporary_root','session','owner'):
        if acquired.get(name):roots.append(session_artifact_root(acquired,name)+'/')
    require(roots,'No exact scene session/output roots')
    keep=[];excluded=[]
    for value in acquired['artifacts']:
        (keep if any(value['path'].startswith(r)for r in roots)else excluded).append(value)
    require(len({v['path']for v in keep})==len(keep),'Duplicate exact scene artifact path')
    return dict(acquired,artifacts=keep),[v['path']for v in excluded]


def analyze_case(reader,acquired,compared):
    purpose={'normal-combat':'combat','normal-attachment':'attachment'}.get(acquired.get('scenario'))
    require(purpose is not None,'Unknown scene output scenario')
    require(acquired['success'] is compared['success'] is True and compared['scene_'+purpose+'_route']['verified'] is True,
            'Actual scene case lacks normal route/pixel admission')
    require(acquired['sources_stable'] is compared['sources_stable'] is True,'Unstable scene case sources')
    acquired,excluded_artifacts=scoped_artifacts(acquired)
    for phase in (acquired['sources'],compared['sources']):
        require(all(reader.digest(n)==h for n,h in phase.items()),'Changed current scene execution/comparison source')
    capture_files=[a for a in acquired['artifacts'] if '/capture/' in a['path'] and a['path'].endswith('.bin')]
    observed=acquisition([reader.read_bytes(a['record']) for a in capture_files],acquired['session_identity'])
    originals=blend.OriginalWorlds(reader,compared)
    history=base.source_history(reader,acquired,originals.index)
    proof=base.output_join(acquired,compared,observed,history)
    poses=decode(artifact(reader,acquired,'scene-actors-0001.bin'))
    require(poses['version']==2 and poses['nonce']==int(acquired['resident']['nonce'])
            and len(poses['frames'])==len(history['worlds']), 'Actor/source session or denominator differs')
    frames=poses['frames'];require([f['generation'] for f in frames]==list(range(1,len(frames)+1)), 'Missing actor generation')
    journal=artifact(reader,acquired,'gameplay-events.bin')
    require(journal[:16]==b'MNMGP001'+struct.pack('<II',1,64) and (len(journal)-16)%64==0,'Original gameplay journal envelope')
    events=[struct.unpack_from('<16I',journal,at) for at in range(16,len(journal),64)]
    require([r[0] for r in events]==list(range(1,len(events)+1)), 'Original gameplay ordinal gap')
    windows=melee_windows(frames,events)
    require(windows or purpose=='attachment','No exact original melee/health source-generation bracket')
    wanted_combat={g for w in windows for g in (w['before'],w['after'])}
    represented={v['world'] for v in proof['presented_worlds'] if v['source_import_live']}
    body_states={};attachments={};crc_cache={};domains=Counter()
    decoder=module('inspect-canvas-producers').decode
    with tempfile.TemporaryDirectory(prefix='mnm-actor-source-',dir='/tmp') as temporary:
        path=Path(temporary)/'worlds.bin';path.write_bytes(artifact(reader,acquired,'worlds.bin'))
        for g,chunk,raw,request in records(path):
            if chunk:continue
            frame=frames[g-1];accepted=proof['source_worlds'][g-1]
            require(accepted['generation']==g and accepted['source_sha256']==base.sha(raw)
                    and accepted['request_sha256']==base.sha(request),'Actor input changed from complete source history')
            _,rows=decoder(raw,complete=False);entry=next(f for f,b in rows if f[2]==11)
            require(frame['source_entry_sequence']==entry[1] and frame['canvas']==entry[3]
                    and tuple(frame['extent'])==entry[5:7] and frame['thread']==poses['owner']
                    and frame['validity']&1 and frame['gameplay_sequence']<=len(events),'Actor queue/owner/journal join differs')
            domains[frame['view']]+=1
            if g not in represented:continue
            if g not in wanted_combat and not any(d[1]>0 for d in frame['draws']):continue
            _,rasters=base.checked_request(rows,request);joined=regular_sources(frame,rasters,crc_cache)
            pairs=attachment_pairs(frame,joined)
            if not pairs and g not in wanted_combat:continue
            writers,_=animation.writer_map(rows,rasters,tuple(frame['extent']));original=originals[g]
            if g in wanted_combat:
                source={}
                for slot in {s for w in windows if g in (w['before'],w['after']) for s in (w['event'][4],w['event'][8])}:
                    body=unique_body(frame,joined,slot)
                    if body is not None:
                        value=source_points(body,writers,original)
                        if value:source['actor-'+str(slot)]=value
                slots={s for w in windows if g in(w['before'],w['after'])for s in(w['event'][4],w['event'][8])}
                body_states[g]=dict(generation=g,sources=source,actors={str(slot):list(actor(frame,slot))for slot in slots})
            for pair in pairs:
                key=(frame['view'],pair['role'],pair['branch'])
                if key in attachments:continue
                parent=source_points(pair['parent'],writers,original);child=source_points(pair['child'],writers,original)
                if parent and child:
                    attachments[key]=dict(generation=g,view=frame['view'],role=pair['role'],branch=pair['branch'],
                        actor=list(pair['actor']),expected=list(pair['expected']),sources={'parent':parent,'child':child})
    combat=None
    for window in windows:
        states=[body_states.get(window[k]) for k in ('before','after')]
        labels={'actor-0','actor-'+str(window['event'][4] if window['event'][4] else window['event'][8])}
        if combat_pose_change(window,states):
            for s in states:s['sources']={k:v for k,v in s['sources'].items() if k in labels}
            combat=dict(**window,states=states,native=native_points(proof,compared,observed,originals,states));break
    require(combat is not None or purpose=='attachment','Original melee bracket has no unambiguous visible paired native bodies')
    common={(role,branch) for view,role,branch in attachments if all((v,role,branch) in attachments for v in range(4))}
    attachment=None
    if common:
        role,branch=sorted(common)[0];states=[attachments[(v,role,branch)] for v in range(4)]
        attachment=dict(role=role,branch=branch,states=states,native=native_points(proof,compared,observed,originals,states))
    require(attachment is not None or purpose=='combat','Active attachment has no unambiguous four-view parent/child native output')
    return dict(success=True,purpose=purpose,actual_extent=compared['world_domain']['extents'][0],software=acquired['configuration']['software_flag'],
                source_generations=len(frames),views=dict(domains),combat=combat,attachment=attachment,
                attachment_candidates=[list(k) for k in sorted(attachments)],
                excluded_foreign_artifact_paths=excluded_artifacts,
                complete_output=dict(worlds=proof['source_generations'],presentations=proof['world_presentations'],
                    retained_pixels=proof['retained_pixels'],mismatching_pixels=0),
                actor_allocation_generations_proved=False)
