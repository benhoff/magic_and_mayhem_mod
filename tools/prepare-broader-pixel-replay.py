#!/usr/bin/env python3
"""Derive a small UI-transform reproducer from exact independently replayed inputs."""
import copy,hashlib,json,struct,sys
from pathlib import Path
import numpy as np
from coverage_evidence import Reader,write_report
from rendering_transition_assessment import observed_epochs
from rendering_pilot import History,commands,inflate,require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
C='research/runtime/surface-loss-restore-results-20261109/bp11-loss-default640-01-comparison-01.json'

def main():
    output=Path('/dev/shm/mnm-broader-player-20261110/pixel-replay');require(not output.exists(),'Preserve previous pixel replay input');output.mkdir()
    with Reader(ROOT)as r:
        comparison=r.json(C);a=r.json(comparison['acquisition']['record'])
        require(not comparison['success']and comparison['comparison_processes_stopped']and comparison['sources_stable'],'Exact stopped negative required')
        bad=[v for v in comparison['epochs'][1]['comparison']['comparisons']if v['mismatching_pixels']]
        require(len(bad)==1 and bad[0]['paint']==1248 and bad[0]['mismatching_pixels']==4,'Review changed pixel failure')
        _,epochs,_=observed_epochs(r,a,provider_loss=True);observed=epochs[1]
        before=observed['paints'][1247][0];after=observed['paints'][1248][0]
        require((before['commands'],after['commands'])==(55887,55924),'Changed exact command window')
        corpus=Path(comparison['comparison_storage']['path']);members={v['path']:v for v in comparison['artifacts'][0]['members']}
        def retained(path):
            rel=path.relative_to(corpus).as_posix();data=path.read_bytes();item=members[rel]
            require(len(data)==item['bytes']and hashlib.sha256(data).hexdigest()==item['sha256'],'Changed retained original-reference input');return data
        reference=corpus/'epoch-1/reference';index=json.loads(retained(reference/'worlds.json'))
        require(index['success']and index['native_player_pixels_read']is False and index['original_destination_capture_read']is False,'Independent source history required')
        lookup={int(row['generation']):row for row in index['worlds']}
        class Worlds:
            last=None;value=None
            def __contains__(self,key):return key in lookup
            def __getitem__(self,key):
                if key!=self.last:
                    row=lookup[key];raw=inflate(retained(reference/row['path']));self.value=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width']);self.last=key
                return self.value
        worlds=Worlds();history=History(worlds);checkpoint=None;deltas=[];old_frame=None
        for op,seq,payload in commands(observed['commands']):
            history.apply(op,seq,payload)
            if seq==before['commands']:
                checkpoint=copy.deepcopy(history.handles);old_frame=history.render(before)
            elif checkpoint is not None:
                if op==19:
                    target=struct.unpack_from('<I',payload)[0];surface=history.handles[target];image=surface['pixels'];require(surface['bits']==16,'World source format changed')
                    payload=struct.pack('<5I',target,0,0,image.shape[1],image.shape[0])+image.astype('<u2').tobytes();op=2
                require(op!=21,'Exact short window contains a CPU read; review its separate check scope')
                deltas.append((op,payload))
            if seq==after['commands']:break
        require(checkpoint is not None and len(deltas)==37,'Incomplete selected command window')
        final=history.render(after);surface=after['surface'];words=history.handles[surface]['pixels'];types={8:'u1',16:'<u2',32:'<u4'}
        items=[]
        for identity,state in sorted(checkpoint.items()):
            pixels=state['pixels'];items.append((1,struct.pack('<7I',identity,pixels.shape[1],pixels.shape[0],state['bits'],*state['masks'])+pixels.astype(types[state['bits']]).tobytes()))
            if state['palette']is not None:items.append((4,struct.pack('<3I',identity,0,256)+state['palette'][:,:3].tobytes()))
        items.append((6,struct.pack('<I',before['surface'])));items+=deltas
        items.append((5,struct.pack('<I',surface)+words.astype(types[history.handles[surface]['bits']]).tobytes()))
        items +=[(7,struct.pack('<I',identity))for identity in sorted(history.handles)]+[(8,b'')]
        data=b'MNMCMD05'+struct.pack('<II',5,16)+b''.join(struct.pack('<III',op,i,len(body))+body for i,(op,body)in enumerate(items,1))
        require(len(data)<=64*1024**2,'Bounded UI replay exceeds64MiB')
        inputs={}
        for name,raw in [('commands.bin',data),('expected-before.rgba',old_frame.tobytes()),('expected-after.rgba',final.tobytes())]:
            (output/name).write_bytes(raw);record=P+'pixel-replay-01-'+name+'.bin';inputs[name]=dict(path=str(output/name),record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw))
        tag=history.handles[surface]['tags']
        q=dict(schema=1,kind='retained_four_pixel_UI_transform_reproducer',success=True,original_game_runs=0,
            original_reference_executions=0,parent=dict(record=C,sha256=r.digest(C)),acquisition=comparison['acquisition'],mismatch=bad[0],
            inputs=inputs,frame_size=[final.shape[1],final.shape[0]],viewport_geometry={k:after[k]for k in('width','height','left','top','image_width','image_height')},
            source_pixel=[633,479],before_rgba=old_frame[479,633].tolist(),after_rgba=final[479,633].tolist(),
            final_native_word=int(words[479,633]),world_lineage_tag=None if tag is None else int(tag[479,633]),
            original_command_window=[before['commands'],after['commands']],original_operations_retained=37,
            scope='Isolated UI-transform/presentation diagnosis only. Initial surfaces are independently reconstructed owned-input/prior-history values; World references in the37-command window become explicitly supplied independently reconstructed source images. These are component inputs, not original destination snapshots or live World validation. A new bounded component END retires every fixture surface and never substitutes for the actual failed stream END. No new game, healthy-case acceptance, historical root cause, renderer repair or original recovery is granted.',
            sources={n:r.digest(n)for n in('tools/prepare-broader-pixel-replay.py','tools/rendering_pilot.py','tools/rendering_transition_assessment.py','tools/rendering_capture.py')})
        name=P+'pixel-replay-input-01.json';sha=write_report(ROOT,name,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=sha,bytes=len(data),before=q['before_rgba'],after=q['after_rgba'],world_lineage_tag=q['world_lineage_tag'])),flush=True)
if __name__=='__main__':main()
