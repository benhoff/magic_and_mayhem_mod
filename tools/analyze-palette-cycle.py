#!/usr/bin/env python3
"""Join directly observed World palette refresh to ordered visible native output."""
import argparse,json,struct,tempfile
from pathlib import Path
from collections import defaultdict
import numpy as np
from coverage_evidence import Reader,write_report
from rendering_pilot import acquisition,require
from rendering_menu_acceptance import artifact
from resident_capture import records
from palette_observation import decode,decode_profiles
from scene_actor_observation import crc32c
from scene_actor_output import base,blend,animation,scoped_artifacts
from scene_effects import module
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/palette-cycling-results-20261108/'
keypal=module('close-rendering-keyed-palette')

def analyze(reader,acquisition_name,comparison_name,numerical_name):
 a,c,n=[reader.json(v)for v in(acquisition_name,comparison_name,numerical_name)]
 require(a['success']and c['success']and c['palette_cycle_route']['verified']and n['success'],'Incomplete actual palette acquisition/comparisons')
 aref=dict(record=acquisition_name,sha256=reader.digest(acquisition_name));require(c['acquisition']==n['acquisition']==aref,'Palette phases use different acquisitions')
 require(n['floating_context_observed']and n['profile_inputs_complete']and n['comparison']['profile_dependent_nodes_skipped']==0 and not n['comparison']['authored_floating_context']and n['comparison']['observed_refreshes']>0,'Missing independently reconstructed actual refresh/floating/platform inputs')
 from palette_source_phase import execution_sources
 effective,review=execution_sources(reader,a,c.get('palette_reader_source_review'),aref)
 for phase in(effective,c['sources'],n['sources']):require(all(reader.digest(p)==h for p,h in phase.items()),'Current palette phase source drift')
 a,excluded=scoped_artifacts(a)
 observed=acquisition([reader.read_bytes(v['record'])for v in a['artifacts']if '/capture/'in v['path']and v['path'].endswith('.bin')],a['session_identity'])
 originals=blend.OriginalWorlds(reader,c);history=base.source_history(reader,a,originals.index);proof=base.output_join(a,c,observed,history)
 pal=decode(artifact(reader,a,'palette-observation-0001.bin'));profiles=decode_profiles(artifact(reader,a,'palette-profiles-0001.bin'),pal)
 require(pal['version']==3 and pal['full_world_prefix']==4,'Direct refresh plus complete first-four selection observation required')
 selection_rows=[r['header']for r in pal['rows']if r['header'][2]==5 and r['header'][5]>0]
 require(selection_rows and {h[5]for h in selection_rows}<={1,2,3,4},'Incomplete low-local-tag full selection domain')
 binding_at=0
 represented={v['world']for v in proof['presented_worlds']if v['source_import_live']};require(len(represented)>=3,'Required refreshed Worlds not presented')
 frame_crc={};table_crc={};decoder=module('inspect-canvas-producers').decode;wanted={};joined_count=0;refreshes={};packets=[]
 with tempfile.TemporaryDirectory(prefix='mnm-palette-source-',dir='/dev/shm')as td:
  path=Path(td)/'worlds.bin';path.write_bytes(artifact(reader,a,'worlds.bin'))
  for g,chunk,raw,request in records(path):
   if chunk:continue
   accepted=proof['source_worlds'][g-1];require(accepted['generation']==g and accepted['source_sha256']==base.sha(raw)and accepted['request_sha256']==base.sha(request),'Changed palette World source')
   _,rows=decoder(raw,complete=False);entry,rasters=base.checked_request(rows,request)
   source_bindings={}
   # Continuous packets reset their local queue/sequence. Pair complete ordered
   # lists directly; global generation comes from the owned source envelope.
   expected=[(f,body,d)for f,body,d in rasters if f[17]and f[20]>=512 and f[15]not in(0,7,9,10)]
   for f,body,descriptor in expected:
    require(binding_at<len(selection_rows),'Missing original palette selection')
    h=selection_rows[binding_at];binding_at+=1
    require(h[5]==entry[14]and h[6]<f[1]and (f[19],f[22],f[16],f[3],tuple(f[8:16]))==(h[25],h[14],h[24],h[26],tuple(h[16:24])),'Ordered palette/local-source metadata differs')
    frame=body[:f[19]];table=body[f[19]:f[19]+512]
    if frame not in frame_crc:frame_crc[frame]=crc32c(frame)
    if table not in table_crc:table_crc[table]=crc32c(table)
    require((frame_crc[frame],table_crc[table])==(h[15],h[12]),'Ordered palette/source bytes differ')
    source_bindings[f[1]]=h
   joined_count+=len(source_bindings)
   if g not in represented or g<2 or len(wanted)>=3:continue
   size=tuple(c['world_domain']['extents'][0]);writers,_=animation.writer_map(rows,rasters,size);original=originals[g];choices={}
   for f,body,descriptor in rasters:
    h=source_bindings.get(f[1])
    if h is None or f[14]!=0 or f[20]!=512 or not f[17]:continue
    node=pal['nodes'][h[8]];mode=node['mode']
    if mode not in(1,9)or mode in choices:continue
    revisions=[r for r in node['refreshes']if r['revision']==h[7]and r['result']and r['return_record']<h[1]]
    if len(revisions)!=1:continue
    refresh=revisions[0]
    if refresh['before']==refresh['after']:continue
    stride=node['stride'];offset=12+h[11]*256*stride
    def words(data):return np.frombuffer(data[offset:offset+256*stride],dtype='<u2'if stride==2 else'<u4')
    before,after=words(refresh['before']),words(refresh['after']);table=np.frombuffer(body[f[19]:f[19]+512],dtype='<u2');require(np.array_equal(table,after),'Delivered refreshed palette differs from actual version')
    encoded=body[:f[19]];indices,ox,oy=keypal.indexed_frame(encoded);left,top=base.signed(f[8])-ox,base.signed(f[9])-oy
    yy,xx=np.nonzero(writers==f[1]);valid=(xx<size[0]-200)&(yy<size[1]-50)&(xx>=left)&(yy>=top)&(xx<left+indices.shape[1])&(yy<top+indices.shape[0]);yy,xx=yy[valid],xx[valid]
    if not len(xx):continue
    ix=indices[yy-top,xx-left];safe=np.maximum(ix,0);valid=(ix>=0)&(after[safe]!=before[safe])&(after[safe]!=0)&(original[yy,xx]==after[safe]);yy,xx,ix=yy[valid],xx[valid],ix[valid]
    if not len(xx):continue
    samples=[]
    for k in range(len(xx)):
     index=int(ix[k]);x,y=int(xx[k]),int(yy[k]);samples.append(dict(world_pixel=y*size[0]+x,original_word=int(after[index]),sequence=f[1],fields=list(f),descriptor=list(descriptor),payload_sha256=base.sha(body),source=dict(palette_index=index,encoded_frame_sha256=base.sha(encoded),shade=base.signed(f[16]),node=h[8],root=h[9],ordinal=h[10],revision=h[7],table=h[11],table_crc32c=h[12],binding_record=h[1],initial_unpresented_word=int(before[index]),refreshed_word=int(after[index]),refresh_entry=refresh['entry_record'],refresh_return=refresh['return_record'])))
    choices[mode]=samples;refreshes[str(node['id'])]=dict(mode=mode,revision=h[7],birth=node['birth'],entry=refresh['entry_record'],returned=refresh['return_record'],retirement=node.get('retirement'),input_sha256=base.sha(refresh['input']),before_sha256=base.sha(refresh['before']),after_sha256=base.sha(refresh['after']),caller=refresh['caller'],queue=refresh['queue'])
   if not choices:continue
   mode=9 if 9 in choices else 1
   wanted[g]={'visible-refreshed-palette':choices[mode]};packets.append(dict(generation=g,source_sha256=base.sha(raw),request_sha256=base.sha(request)))
 require(binding_at==len(selection_rows),'Extra original palette selection outside complete owned Worlds')
 require(len(wanted)==3,'Missing three advancing visible refreshed output generations')
 samples=blend.viewport_samples(dict(proof=proof),observed,originals,wanted)
 require(len(samples)==3 and {(v['generation'],v['kind'])for v in samples}=={(g,'visible-refreshed-palette')for g in wanted},'Refreshed source pixels lack actual native viewport delivery')
 accepted={v['paint']:v for v in c['comparison']['comparisons']}
 for row in samples:
  match=accepted[row['paint']];s=row['source']['source'];require(match['world']==row['generation']and match['lease']==row['lease']and match['mismatching_pixels']==0 and row['actual_rgba']==row['original_rgba']==blend.rgba(s['refreshed_word'])and s['initial_unpresented_word']!=s['refreshed_word'],'Refresh actual native colour/comparison binding differs')
 for node_id,refresh in refreshes.items():
  node=pal['nodes'][int(node_id)];require(not node['live']and node['retirement']>refresh['returned'],'Selected refreshed allocation did not normally retire')
 return dict(schema=1,kind='actual_world_palette_refresh_output_analysis',success=True,identity=a['session_identity'],software=a['configuration']['software_flag'],actual_extent=c['world_domain']['extents'][0],
  pair=dict(acquisition=aref,comparison=dict(record=comparison_name,sha256=reader.digest(comparison_name))),numerical=dict(record=numerical_name,sha256=reader.digest(numerical_name)),
  refreshed_nodes=refreshes,native=samples,checked_all_world_bindings=joined_count,source_packets=packets,actual_profile_queries=len(profiles['rows']),
  complete_output=dict(worlds=proof['source_generations'],presentations=proof['world_presentations'],mismatching_pixels=0),excluded_foreign_artifacts=excluded,palette_reader_source_review=review,
  original_game_runs=0,new_native_player_runs=0,closes_palette_rows=False,scope='Finite actual World-entry refresh of existing WORD/DWORD blue nodes: independently reconstructed initial and refreshed production inputs/results, exact current version selection, ordered owned delivery, changed fixed-index/shade production colour from an actually visible refreshed node in three advancing complete native Worlds; both WORD/DWORD producer formats reconstructed, with selected source versions joined over the complete session, and normal original/native retirement. Initial unpresented constructor colours are a comparison baseline, not a prior native frame. No arbitrary periodic cycling, exceptional arithmetic, platform implementation or final player decision claim.')

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for k in('acquisition','comparison','numerical','report'):p.add_argument('--'+k,required=True)
 a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned palette analysis required');q=dict(schema=1,success=False)
 names=('tools/analyze-palette-cycle.py','tools/palette_source_phase.py','tools/palette_observation.py','tools/scene_actor_output.py','tools/scene_actor_observation.py','tools/close-rendering-keyed-palette.py','tools/close-rendering-animation-auxiliary.py','tools/close-rendering-blend-direct.py','tools/close-rendering-destination-history.py','tools/resident_capture.py','tools/rendering_pilot.py')
 with Reader(ROOT)as r:
  sources={n:r.digest(n)for n in names}
  try:q=analyze(r,a.acquisition,a.comparison,a.numerical)
  except Exception as e:q['error']=str(e)
  q['sources']=sources;q['sources_stable']=all(r.digest(n)==h for n,h in sources.items());q['success']&=q['sources_stable']
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h)));return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
