#!/usr/bin/env python3
"""Retained probe diagnosis only; preserve strict full-stream rejection."""
import argparse,hashlib,importlib.util,json,struct
from collections import Counter,defaultdict
from pathlib import Path
from coverage_evidence import Reader,write_report
from palette_observation import decode
from scene_actor_observation import crc32c
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/palette-cycling-results-20261108/'

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);a=p.parse_args()
 require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned result required')
 names=('tools/analyze-palette-probe.py','tools/palette_observation.py','tools/scene_actor_observation.py','tools/inspect-canvas-producers.py')
 q=dict(schema=1,kind='retained_palette_semantic_probe_review',success=False,original_game_runs=0,new_native_player_runs=0,closes_palette_rows=False,ready_for_residual_native_capture=False)
 with Reader(ROOT)as reader:
  q['sources']={n:reader.digest(n)for n in names};acquired=reader.json(a.acquisition);q['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
  def artifact(suffix):
   items=[r for r in acquired['artifacts']if r['path'].endswith(suffix)];require(len(items)==1,'Missing/ambiguous retained artifact '+suffix);r=items[0];raw=reader.read_bytes(r['record']);require(hashlib.sha256(raw).hexdigest()==r['sha256'],'Changed retained raw bytes');return raw,r
  try:
   require(acquired['original_game_runs']==1 and acquired['original_manifest_before_after'],'Probe provenance incomplete')
   raw,palette_ref=artifact('/palette-observation-0001.bin');observed=decode(raw,complete=False)
   require(not observed['complete']and observed['terminal']is None and observed['nodes'],'Expected explicit forced-stop palette prefix')
   raw,source_ref=artifact('/canvas-producers.bin');marker,marker_ref=artifact('/canvas-producers.done');header=struct.unpack_from('<16I',raw)
   require(raw[:8]==b'MNMPRO01'and header[6]==4 and len(marker)==32 and marker[:8]==b'MNMPDONE','Unexpected retained probe format')
   spec=importlib.util.spec_from_file_location('palette_probe_decoder',ROOT/'tools/inspect-canvas-producers.py');legacy=importlib.util.module_from_spec(spec);spec.loader.exec_module(legacy)
   try:legacy.decode(raw)
   except ValueError as e:q['strict_full_stream_rejection']=str(e)
   else:raise ValueError('Expected retained source-order failure disappeared')
   require(q['strict_full_stream_rejection']=='Invalid producer record bounds/sequence/operation','Different strict refusal')
   at=64;rows=[]
   while at<len(raw):
    require(len(raw)-at>=96,'Truncated wire record');f=struct.unpack_from('<24I',raw,at)
    require(f[0]==96+f[18]and f[0]<=len(raw)-at and f[2]in legacy.NAMES and not f[23]and f[2]not in(13,24,25,26,27),'Raw envelope/failure outside diagnosed source ordering')
    rows.append((at,f,raw[at+96:at+f[0]]));at+=f[0]
   m=struct.unpack('<8I',marker);require(m[2:]==(1,4,len(rows),0,sum(f[2]==10 for at,f,b in rows),len(raw)),'Original completion marker differs')
   require(sorted(f[1]for at,f,b in rows)==list(range(1,len(rows)+1)),'Lost/duplicate source sequence')
   inverted=[dict(wire_position=i+1,offset=at,producer_sequence=f[1],operation=f[2],caller=f[22])for i,(at,f,b)in enumerate(rows)if f[1]!=i+1]
   require(len(inverted)==2 and inverted[0]['producer_sequence']==inverted[1]['wire_position']and inverted[1]['producer_sequence']==inverted[0]['wire_position'],'Unexpected source order failure')
   first=next(i for i,(at,f,b)in enumerate(rows)if f[2]==11);require(all(r['wire_position']<=first for r in inverted),'Source ordering failed inside observed World prefix')
   require(all(f[1]==i+1 for i,(at,f,b)in enumerate(rows)if i>=first),'World records are not ordered')
   worlds=defaultdict(list);active=0;returns=[]
   for at,f,body in rows[first:]:
    if f[2]==11:require(active==0 and f[14]==len(returns)+1,'World entry order');active=f[14]
    if active:worlds[active].append((f,body))
    if f[2]==12:require(f[14]==active,'World return identity');returns.append(active);active=0
   require(active==0 and returns==[1,2,3,4],'Missing closed four World queues')
   bindings=[r['header']for r in observed['rows']if r['header'][2]==5 and r['header'][5]>0];wanted=defaultdict(list)
   for h in bindings:wanted[(h[5],h[14],h[25],tuple(h[16:24]),h[24],h[26],h[15],h[12])].append(h)
   matched={};cache={}
   for world,values in worlds.items():
    for f,body in values:
     if f[2]!=9 or not f[17]or f[20]<512 or f[15]in(7,9):continue
     frame=body[:f[19]];table=body[f[19]:f[19]+512]
     if frame not in cache:cache[frame]=crc32c(frame)
     key=(world,f[22],f[19],tuple(f[8:16]),f[16],f[3],cache[frame],crc32c(table))
     for h in wanted.get(key,[]):
      if h[6]<f[1]:matched[h[1]]=dict(binding=h[1],world=world,source_sequence=f[1],node=h[8],root=h[9],ordinal=h[10],table=h[11],mode=observed['nodes'][h[8]]['mode'],frame_crc32c=h[15],effective_table_crc32c=h[12])
   require(len(matched)==len(bindings),'A selected production table lacks its exact completed World source request')
   q.update(success=True,ready_for_residual_native_capture=True,palette=dict(record=palette_ref['record'],sha256=palette_ref['sha256'],nodes=len(observed['nodes']),modes=dict(Counter(n['mode']for n in observed['nodes'].values())),counts=dict(Counter(n['count']for n in observed['nodes'].values())),records=len(observed['rows']),normal_retirement=False),source=dict(record=source_ref['record'],sha256=source_ref['sha256'],records=len(rows),original_completion_marker=marker_ref,permuted_menu_records=inverted,complete_world_queues=returns,world_source_order_preserved=True,strict_whole_stream_accepted=False),binding_join=dict(checked=len(bindings),matched=len(matched),selected_modes=dict(Counter(r['mode']for r in matched.values())),examples=list(matched.values())[:8]))
   q['diagnosis']='Exactly two complete menu records (producer789/790) appear in reversed wire order before first World queue; no duplicate/missing ordinal or FAILURE. Strict decoder continues to reject this complete legacy stream. The noncontinuous emit path has no serialization; timing exposed ordering, without proving a general causal guarantee. All four later World regions remain ordered and every observed World palette selection joins its original source request. Continuous native acquisition already serializes emit under continuous_emit_lock. Reuse this passive semantic discovery; do not sort/rewrite the stream or claim complete source history, original retirement, display/cycling, or a new legacy/runtime repair.'
  except Exception as e:q['error']=str(e)
  q['sources_stable']=all(reader.digest(n)==h for n,h in q['sources'].items())
 digest=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=digest,join=q.get('binding_join'))),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
