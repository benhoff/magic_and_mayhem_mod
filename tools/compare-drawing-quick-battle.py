#!/usr/bin/env python3
"""Replay source-owned Quick Battle inputs and compare every observed EGL frame."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import numpy as np
from drawing_api_session import BUILD,sha
from drawing_scene_corpus import raster_inputs,asset_name
from drawing_egl_physical import analyze_egl_physical
from drawing_producer_points import inspect_points
ROOT=Path(__file__).resolve().parents[1]


def frame_bindings(original, points, producer_count, destinations=None, records=(), returns=None):
    contexts={(c['pid'],c['call']):c for c in original['egl_worker_context_observation']['calls']}
    packets={p['worker']:p for p in original['wine_blit_packet_observation']['bindings']}
    sources={s['submission']:s for s in original['wine_source_map_observation']['samples']}
    workers={w['call']:w for w in original.get('wine_blit_worker_observation',{}).get('calls',[])}
    apis={c['entry']:c for c in original.get('observation',{}).get('calls',[])}
    keyed={}
    for r,payload in records:
        if r[2]==6 and r[14]!=0xffffffff:keyed.setdefault(r[4],set()).add(r[14])
    results=[];refusals=[];previous=None
    for frame in original['egl_physical_observation']['frames']:
        ctx=contexts.get((frame['pid'],frame['entry']));reason=None
        if not ctx or not ctx['stable_active_context']:reason='unbound-worker-context'
        value=ctx['points'][0]['context'] if ctx else {};packet=packets.get(value.get('worker'));sample=sources.get(value.get('submission'));worker=workers.get(value.get('worker'))
        if not reason and (not packet or not sample or packet['command']!=value['command'] or packet['submission']!=sample['submission']):reason='unbound-packet-source'
        point=points.get(sample['api_entry']) if sample else None
        located=point is not None and point[2]&23==19 and not point[5] and point[3]<=producer_count
        transfer=None;refresh=False;fill=None
        if not reason and not located:
            # A NULL source may describe a pending primary colour fill. Bind
            # the exact following producer command, rather than treating a
            # resource without worker blits as unchanged (Map writes exist).
            dest=(destinations or {}).get(sample['api_entry']);end=(returns or {}).get(sample['api_entry']);api=apis.get(sample['api_entry'],{})
            r=records[end[3]][0] if end and end[3]<len(records) else None
            if dest and dest[2]&23==19 and not dest[5] and dest[7:9]==(800,600) and api.get('method')==5 and api.get('primary')==1 and api.get('flags',0)&0x400 and api.get('hresult')==0 and r and r[2]==5 and r[3]==dest[6] and r[10:14]==(0,0,800,600):fill=r[14]
            else:reason='source-recorder-not-live-or-unlocated'
        if not reason and sample['status']!=0:reason='source-map-refusal'
        if not reason and located and point[7:10]!=(800,600,800):
            # The final worker is an internal primary-buffer copy. Locate the
            # actual outer source transfer by packet/API identity and descriptor.
            roots=[w for w in workers.values() if w['call']<worker['call'] and
                w['destination']==worker['source'] and w['destination_generation']==worker['source_generation'] and
                sources[packets[w['call']]['submission']]['api_entry']==sample['api_entry'] and
                sources[packets[w['call']]['submission']]['descriptor'][6:8]==list(point[7:9])]
            if len(roots)!=1:reason='unbound-primary-rectangle-source'
            else:
                root=roots[0];keys=keyed.get(point[6],set());key=None
                if root['flags']==0x41008000 and len(keys)==1:key=next(iter(keys))
                elif root['flags']!=0x41000000:reason='unsupported-copy-effect'
                if not reason and (root['effects']!=[0]*6 or root['filter']!=1 or root['source_subresource'] or root['destination_subresource'] or
                    root['destination_box']!=worker['source_box'] or worker['source_box']!=worker['destination_box'] or worker['flags']!=0):reason='unsupported-primary-rectangle-transfer'
                if not reason:transfer=dict(source_box=root['source_box'][:4],destination_box=root['destination_box'][:4],key=key,source_worker=root['call'])
        if reason:refusals.append(dict(ordinal=frame['ordinal'],reason=reason));previous=None;continue
        result=dict(ordinal=frame['ordinal'],api_entry=sample['api_entry'],worker=packet['worker'],command=packet['command'],submission=packet['submission'],
            sequence=point[3] if located else None,canvas=point[6] if located else None,width=point[7] if located else None,height=point[8] if located else None,
            transfer=transfer,refresh=refresh,fill=fill,primary_resource=(worker['source'],worker['source_generation']) if worker else None,oracle_path=frame['path'],oracle_sha256=frame['sha256'])
        results.append(result)
        previous=dict(worker=worker['call'],resource=(worker['source'],worker['source_generation'])) if worker else None
    return results,refusals


def compose_native(previous, source, frame):
    if frame.get('fill') is not None:return struct.pack('<H',frame['fill'])*(800*600)
    if frame['refresh']:
        if previous is None:raise ValueError('Refresh lacks native history')
        return previous
    width,height=frame['width'],frame['height']
    plane=np.frombuffer(source,dtype='<u2').reshape(height,width)
    transfer=frame['transfer']
    if transfer is None:
        if (width,height)!=(800,600):raise ValueError('Full native source extent')
        return source
    if previous is None:raise ValueError('Partial update lacks native history')
    x,y,right,bottom=transfer['destination_box'];sx,sy,sright,sbottom=transfer['source_box']
    if not (0<=x<right<=800 and 0<=y<bottom<=600 and 0<=sx<sright<=width and 0<=sy<sbottom<=height and right-x==sright-sx and bottom-y==sbottom-sy):raise ValueError('Unsupported native rectangle')
    output=np.frombuffer(previous,dtype='<u2').reshape(600,800).copy();src=plane[sy:sbottom,sx:sright];dst=output[y:bottom,x:right]
    np.copyto(dst,src,where=True if transfer['key'] is None else src!=transfer['key'])
    return output.tobytes()


def background_bindings(original,points,frames):
    workers={w['call']:w for w in original['wine_blit_worker_observation']['calls']}
    packets={p['worker']:p for p in original['wine_blit_packet_observation']['bindings']}
    maps={s['submission']:s for s in original['wine_source_map_observation']['samples']}
    for f in frames:
        if not f['transfer'] or f['transfer']['key'] is not None:continue
        root=workers[f['transfer']['source_worker']]
        saves=[w for w in workers.values() if w['call']<root['call'] and
            (w['destination'],w['destination_generation'])==(root['source'],root['source_generation'])]
        if not saves:raise ValueError('Background source has no defining transfer')
        save=max(saves,key=lambda w:w['call']);sample=maps[packets[save['call']]['submission']];point=points[sample['api_entry']]
        if save['flags']!=0x41000000 or save['effects']!=[0]*6 or save['filter']!=1 or save['destination_box']!=[0,0,f['width'],f['height'],0,1] or point[2]&23!=19 or point[5] or point[7:9]!=(800,600):raise ValueError('Unsupported background definition')
        source=(save['source'],save['source_generation'])
        if source==tuple(f['primary_resource']):
            prior=[v for v in frames if v['worker']<save['call'] and v['primary_resource']==f['primary_resource']]
            if not prior:raise ValueError('Background requires prior native primary history')
            prior=max(prior,key=lambda v:v['worker'])
            if any((w['destination'],w['destination_generation'])==source for w in workers.values() if prior['worker']<w['call']<save['call']):raise ValueError('Unreplayed primary write before background save')
            binding=dict(worker=save['call'],from_presentation=prior['ordinal'])
        else:
            if point[9]!=800 or sample['descriptor'][6:8]!=[800,600]:raise ValueError('Background canvas is not a complete source')
            binding=dict(worker=save['call'],sequence=point[3],canvas=point[6],width=800,height=600)
        binding['source_box']=save['source_box'][:4];f['background']=binding
    return frames


def compare_plane(native,original):
    words=np.frombuffer(native,dtype='<u2');rgba=np.frombuffer(original,dtype=np.uint8).reshape(-1,4)
    if words.size!=800*600 or rgba.shape!=(800*600,4):raise ValueError('Whole plane extent')
    r=(words>>11)&31;g=(words>>5)&63;b=words&31
    predicted=np.stack(((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2)),axis=1).astype(np.int16)
    diff=np.max(np.abs(predicted-rgba[:,:3].astype(np.int16)),axis=1);bad=np.flatnonzero(diff>1)
    return dict(pixels=words.size,mismatching_pixels=int(bad.size),max_channel_difference=int(diff.max()),channel_tolerance=1,alpha_compared=False,native_sha256=sha(native),original_sha256=sha(original),examples=[dict(x=int(i%800),y=int(i//800),native_rgb=predicted[i].tolist(),original_rgb=rgba[i,:3].tolist()) for i in bad[:8]])


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('original','native','claims','report'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_bytes());sources=dict(claims['sources'])
    for directory in ('renderer','assets','compat/legacy','protocols/include','reconstruction/rendering'):
        for f in (ROOT/directory).rglob('*'):
            if f.is_file() and (f.suffix in ('.cpp','.hpp','.h','.c','.S','.cmake') or f.name=='CMakeLists.txt'):sources[str(f.relative_to(ROOT))]=sha(f.read_bytes())
    def stable():
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective replay sources changed')
    stable();subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True);raw=a.original.read_bytes();original=json.loads(raw)
    if original.get('success') is not True or original.get('source_sha256')!=BUILD or not all(original.get(n) is True for n in ('original_manifest_verified_before_after','normal_original_quit_verified','original_sections_unchanged')):raise ValueError('Completed original provenance required')
    capture=Path(original['experiment'])/'capture';game=Path(original['experiment'])/'game'
    peer=Path(original['experiment'])/'egl-physical-report.json'
    if sha(peer.read_bytes())!=original['egl_physical_peer_report_sha256'] or analyze_egl_physical(capture,peer)!=original['egl_physical_observation']:raise ValueError('Original presentation artifacts changed')
    observed=inspect_points(capture,session=original.get('staging',{}).get('session_producers',False))
    if observed!=original['producer_observation']:raise ValueError('Original producer bindings changed')
    point_raw=(capture/'api-producer-points.bin').read_bytes();rows=list(struct.iter_unpack('<48I',point_raw[64:-192]));points={r[2]:r[32:48] for r in rows if r[1]==1};destinations={r[2]:r[16:32] for r in rows if r[1]==1};returns={r[2]:r[16:32] for r in rows if r[1]==3}
    producer=(capture/'canvas-producers.bin').read_bytes()
    spec=importlib.util.spec_from_file_location('producer',ROOT/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);_,records=m.decode(producer,complete=not original.get('staging',{}).get('session_producers'))
    frames,refusals=frame_bindings(original,points,len(records),destinations,records,returns)
    background_bindings(original,points,frames)
    if not frames:raise ValueError('No source-bound native frame inputs')
    parent=ROOT/'working/tests/draw02-quick-battle';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True);(run/'.retain').write_text('DRAW-02 original to native whole-frame comparison; retain failures and inputs.\n')
    owned=run/'owned-inputs';owned.mkdir();binary_sha=sha(a.native.read_bytes());last=max(f['sequence'] for f in frames if f['sequence'] is not None)
    end=64
    for r,payload in records[:last]:end+=struct.unpack_from('<I',producer,end)[0]
    native_source=raster_inputs(producer[:end]);(owned/'producers.bin').write_bytes(native_source)
    requested={b[:-1].decode('ascii') for r,b in records[:last] if r[2] in (7,20)}
    existing={f.relative_to(game).as_posix().casefold():f for f in game.rglob('*') if f.is_file()}
    selected={n:existing[asset_name(n).casefold()] for n in requested}
    for name,path in existing.items():
        if name.endswith('.spr'):selected[name]=path
    assets={};input_hashes={}
    for name,path in sorted(selected.items()):
        target=owned/asset_name(name).casefold();target.parent.mkdir(parents=True,exist_ok=True);data=path.read_bytes();target.write_bytes(data);assets[name]=sha(data);input_hashes[str(path.relative_to(ROOT))]=sha(data)
    if sum(f.stat().st_size for f in owned.rglob('*') if f.is_file())>600*1024*1024:raise ValueError('Owned input budget')
    unique={(f['sequence'],f['canvas']):f for f in frames if f['fill'] is None and not f.get('background')}
    for f in frames:
        bg=f.get('background',{})
        if 'sequence' in bg:unique[bg['sequence'],bg['canvas']]=bg
    bounds=[];queues=0;cursor=0
    for (seq,canvas),f in sorted(unique.items()):
        for r,payload in records[cursor:seq]:
            if r[2]==12:queues=r[14]
        cursor=seq;bounds.append(dict(sequence=seq,queues=queues,canvas=canvas,width=f['width'],height=f['height']))
    declaration=dict(schema=1,session='1',assets=assets,segments=[dict(segment=1,stage='scene',producer=dict(path='producers.bin',sha256=sha(native_source)),frames=bounds)])
    (owned/'drawing-session.json').write_text(json.dumps(declaration,indent=2)+'\n')
    result=dict(schema=1,success=False,sources=sources,claims=claims['claims'],original_report=dict(path=str(a.original.resolve().relative_to(ROOT)),sha256=sha(raw)),artifacts=str(run.relative_to(ROOT)),original_game_executed=False,native_parity_performed=True,original_pixels_used_as_native_inputs=False,native_binary_sha256=binary_sha,observed_presentations=len(original['egl_physical_observation']['frames']),source_bound_presentations=len(frames),refusals=refusals,inputs=input_hashes,scope='Whole observed EGL Quick Battle frames from native source snapshots and recorded rectangle composition over retained native output. Source snapshots do not publish a native session frame or prove native scanout. All missing bindings and mismatches retained; RGB comparison allows one output-channel unit for RGB565 display expansion. No full14-journey/21-operation admission, hardware scanout or original caller-result parity.')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1');cmd=['xvfb-run','-a',str(a.native.resolve()),str(owned),str(run/'native'),'--source-snapshots']
        with (run/'native.log').open('x') as log:execution=subprocess.run(cmd,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=600)
        native_report=run/'native/session-report.json'
        result['native']=json.loads(native_report.read_bytes()) if native_report.exists() else None
        result['native_exit_code']=execution.returncode
        comparisons=[]
        if execution.returncode==0:
            native=result['native']
            if not native['success'] or native['remaining_surfaces'] or native['original_oracles_read'] or len(native['frames'])!=len(bounds):raise ValueError('Native completion/cohort mismatch')
            byseq={(f['sequence'],f['canvas']):f for f in native['frames']};previous=None;history={}
            composed=run/'composed';composed.mkdir()
            for frame in frames:
                oracle=(capture/frame['oracle_path']).read_bytes()
                if sha(oracle)!=frame['oracle_sha256']:raise ValueError('Original frame identity changed')
                source=None
                bg=frame.get('background')
                if bg:
                    if 'from_presentation' in bg:background=history[bg['from_presentation']]
                    else:
                        n=byseq[bg['sequence'],bg['canvas']];background=(run/'native'/n['path']).read_bytes()
                        if sha(background)!=n['sha256']:raise ValueError('Background native identity changed')
                    sx,sy,right,bottom=bg['source_box']
                    if not (0<=sx<right<=800 and 0<=sy<bottom<=600 and right-sx==frame['width'] and bottom-sy==frame['height']):raise ValueError('Background source rectangle changed')
                    source=np.frombuffer(background,dtype='<u2').reshape(600,800)[sy:bottom,sx:right].copy().tobytes()
                elif frame['fill'] is None:
                    n=byseq[frame['sequence'],frame['canvas']];source=(run/'native'/n['path']).read_bytes()
                    if sha(source)!=n['sha256'] or any(n[k]!=frame[k] for k in ('canvas','width','height')):raise ValueError('Native source identity changed')
                previous=compose_native(previous,source,frame)
                history[frame['ordinal']]=previous
                path=composed/f"frame-{frame['ordinal']:06d}.565";path.write_bytes(previous)
                comparisons.append(dict(ordinal=frame['ordinal'],sequence=frame['sequence'],canvas=frame['canvas'],transfer=frame['transfer'],background=frame.get('background'),fill=frame['fill'],refresh=frame['refresh'],native_path=str(path.relative_to(run)),**compare_plane(previous,oracle)))
        result.update(comparisons=comparisons,compared_presentations=len(comparisons),mismatched_presentations=sum(c['mismatching_pixels']>0 for c in comparisons),mismatching_pixels=sum(c['mismatching_pixels'] for c in comparisons),pixels_compared=sum(c['pixels'] for c in comparisons),all_observed_presentations_compared=len(comparisons)==result['observed_presentations'],native_input_bytes=sum(f.stat().st_size for f in owned.rglob('*') if f.is_file()))
        result['success']=execution.returncode==0 and not refusals and not result['mismatching_pixels'] and result['all_observed_presentations_compared']
        if sha(a.native.read_bytes())!=binary_sha or any(sha((ROOT/n).read_bytes())!=h for n,h in input_hashes.items()):raise ValueError('Binary or original asset inputs changed')
        stable();result['sources_stable']=True
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
        with a.report.open('x') as out:json.dump(result,out,indent=2);out.write('\n')
    print(json.dumps({k:result.get(k) for k in ('success','observed_presentations','source_bound_presentations','compared_presentations','mismatched_presentations','native_exit_code')}))


if __name__=='__main__':main()
