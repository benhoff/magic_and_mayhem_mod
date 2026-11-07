#!/usr/bin/env python3
"""Verify natural full-fill arguments, actual command intervals and GPU planes."""
import argparse
from bisect import bisect_left
from collections import defaultdict
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import numpy as np
from coverage_evidence import Reader, write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_capture import capture
from rendering_menu_acceptance import artifact
from rendering_pilot import History, commands, require
from resident_capture import records

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-fill-operations-results-20261107/'
BID='NP.full-fill-operation-closeout'
LEAF='operation.fill-full'
COPY='research/runtime/rendering-copy-results-20261106/closeout-analysis-01.json'
spec=importlib.util.spec_from_file_location('fill_copy',ROOT/'tools/close-rendering-opaque-cropped-copy.py')
cp=importlib.util.module_from_spec(spec);spec.loader.exec_module(cp)
decoder=cp.module('inspect-canvas-producers')


def checked(reader,ref):
 require(reader.digest(ref['record'])==ref['sha256'],'Changed immutable case input')
 return reader.json(ref['record'])


def source_fills(reader,a):
 fills=[];policy=0;last=0
 with tempfile.TemporaryDirectory(prefix='full-fill-source-',dir='/tmp') as td:
  p=Path(td)/'worlds.bin';p.write_bytes(artifact(reader,a,'worlds.bin'))
  for g,chunk,raw,request in records(p):
   last=max(last,g if not chunk else 0)
   for f,body in decoder.decode(raw,complete=False)[1]:
    if f[2]!=5:continue
    if f[22]==0x4e495a31:policy+=1;continue
    fills.append(dict(generation=g,chunk=chunk,source_sha256=cp.sha(raw),fields=list(f)))
 require(last>0 and fills and policy>0,'Missing actual source fill chronology')
 return fills,policy,last


def source_caller(source,row,module):
 f=source['fields'];full=f[10:14]==[0,0,*f[5:7]]
 require(f[2]==5 and f[22]!=0x4e495a31 and module[0]<=f[22]<sum(module)
  and f[5:7]==list(row[19:21]) and 0<=f[14]<=65535
  and f[21]==row[13]==0 and row[14:16]==(1,3)
  and row[11]==0 and row[12] in (0x400,0x1000400)
  and row[21]==16 and not row[10]&(32|128) and not any(row[40:44])
  and row[1]==(0x58bc5a if full else 0x58bb0d), 'Source/caller fill identity differs')
 return full


def constant_update(payload,width,height,value):
 sid,x,y,w,h=struct.unpack_from('<5I',payload)
 require((x,y,w,h)==(0,0,width,height) and len(payload)==20+width*height*2,
         'Full fill update geometry differs')
 expected=np.full((height,width),value,dtype='uint32')
 require(payload[20:]==expected.astype('<u2').tobytes(),'Full fill differs from original argument color')
 return sid,expected


def check_witness(w):
 require(source_caller(w['source'],tuple(w['caller']),w['original_module']), 'Selected fill is partial')
 require(w['generation']>0 and w['source']['fields'][22]!=0x4e495a31
  and w['world_surface']>0 and w['world_surface']==w['sink_world_surface']
  and w['generation']==w['sink_generation']
  and w['world_created']<w['update_sequence']<w['sink_sequence']<w['world_deleted']
  and w['source']['fields'][14]==w['value']>0
  and w['created']<w['update_sequence']<w['sink_sequence']<w['deleted']
  and w['lower_fence']<w['update_sequence']<w['upper_fence']
  and w['changed_words']>0 and w['surviving_fill_words']>0
  and w['native_update_matches_argument'] and w['complete_gpu_source_verified']
  and w['complete_gpu_destination_verified'] and w['caller'][13]==0
  and w['copy_caller'][13]==0 and w['physical_identity_observations']>0
  and w['actual_source_sha256']==w['expected_source_sha256'], 'Incomplete natural full-fill output witness')


def check_case(c):
 require(c['verified'] and c['scenario']=='normal-storage' and c['source_fill_calls']>3
  and c['source_fill_calls']==c['joined_source_caller_calls']
  and c['policy_initializations_excluded']>0 and len(c['unpaired_tail_fills'])==2
  and len(c['witnesses'])==3,'Incomplete retained full-fill domain')
 require(len({v['update_sequence'] for v in c['witnesses']})==3
  and len({v['sink_sequence'] for v in c['witnesses']})==3
  and len({v['actual_source_sha256'] for v in c['witnesses']})==3,
  'Repeated full fill or GPU output cannot supply coverage')
 for w in c['witnesses']:check_witness(w)


def case(reader,seed):
 a,c=[checked(reader,seed['pair'][k]) for k in ('acquisition','comparison')]
 require(a['success'] and c['success'] and not a['original_pixels_used_as_native_inputs']
  and c['original_reference']['continuous_from_owned_initial_history']
  and not c['original_reference']['native_player_pixels_read']
  and not c['original_reference']['original_destination_capture_read']
  and c['comparison']['success'] and c['comparison']['mismatching_pixels']==0,
  'Dependent or failed original/native case')
 require(a['configuration']['software_flag'] is (seed['domain'][0]=='player-software')
  and json.loads(artifact(reader,a,'native-output-admission.json'))['native_only']['policy']=='native-only-rendering-v1',
  'Wrong supported profile or native-only admission')
 source,policy,last_world=source_fills(reader,a)
 header,rows,routing=cp.routes(reader,a);fills=[(i,row) for i,row in enumerate(rows) if row[15]==3]
 require(len(fills)==len(source)+2 and len({row[3] for _,row in fills})==1,'Unmatched or multi-owner fill chronology')
 full=[source_caller(s,row,(header[15],header[16])) for s,(i,row) in zip(source,fills)]
 refs=[v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('commands') and v['path'].endswith('.bin')]
 require(len(refs)==1,'Ambiguous actual command history')
 raw=b''.join(body for h,body in capture(reader.read_bytes(refs[0]['record'])) if h['kind']=='fragment')
 require(cp.sha(raw)==c['captured_command_stream_sha256'],'Changed compared command history')
 bound,refused=cp.command_join(raw,rows)
 creates={};lives={};updates=[];last_import=0;last_import_generation=0
 for op,seq,p in commands(raw):
  if op==1:
   w=struct.unpack_from('<7I',p);creates[w[0]]=w;lives[w[0]]=[seq,2**32]
  elif op==7:lives[struct.unpack('<I',p)[0]][1]=seq
  elif op==19:
   w=struct.unpack('<5I',p);last_import=seq;last_import_generation=w[1]|w[2]<<32
  elif op==2:
   sid,x,y,w,h=struct.unpack_from('<5I',p)
   if creates[sid][3]!=16:continue
   values=np.frombuffer(p[20:],dtype='<u2')
   if values.size and np.all(values==values[0]):updates.append(dict(sequence=seq,sid=sid,geometry=[x,y,w,h],value=int(values[0])))
 require(last_import_generation==last_world,'Source/command terminal World differs')
 physical=defaultdict(list)
 for i,row in enumerate(rows):
  if not row[40]:continue
  for obj,sid in ((row[7],row[42]),(row[6],row[43])):
   physical[obj].append((i,row[40],sid))
 sequences=[v['sequence'] for v in updates];selected={}
 for source_index,(s,(index,row),is_full) in enumerate(zip(source,fills,full)):
  if not is_full:continue
  lo=max((v[41] for v in rows[:index] if v[41]),default=0)
  hi=next((v[40] for v in rows[index+1:] if v[40]),2**32)
  candidates=[]
  for u in updates[bisect_left(sequences,lo+1):bisect_left(sequences,hi)]:
   sid=u['sid'];start,end=lives[sid]
   matches=[v for v in physical[row[6]] if start<v[1]<end and v[2]==sid]
   if u['geometry']==[0,0,*s['fields'][5:7]] and u['value']==s['fields'][14] and matches:
    candidates.append((u,len(matches)))
  if len(candidates)!=1:continue
  u,n=candidates[0];require(u['sequence'] not in selected,'Two original fills bind one UPDATE')
  selected[u['sequence']]=dict(source=s,source_index=source_index,caller=list(row),original_module=[header[15],header[16]],lower_fence=lo,upper_fence=hi,
   created=lives[u['sid']][0],deleted=lives[u['sid']][1],physical_identity_observations=n,value=u['value'])
 diagnostics=[json.loads(line) for line in artifact(reader,a,'copies.jsonl').splitlines()]
 diags={v['sequence']:v for v in diagnostics};require(len(diags)==len(diagnostics),'Duplicate GPU plane diagnostic')
 model=History(cp.base.OriginalWorlds(reader,c));tags={};proofs={};found=[]
 for op,seq,p in commands(raw):
  sink=None
  if op==1:
   w=struct.unpack_from('<7I',p);tags[w[0]]=np.zeros((w[2],w[1]),dtype='uint32')
  elif op in (2,20):
   sid,x,y,w,h=struct.unpack_from('<5I',p);target=tags[sid][y:y+h,x:x+w]
   if op==2:target[:]=0
   else:
    n=w*h*(model.handles[sid]['bits']//8);target[np.frombuffer(p[20+n:],dtype='u1').reshape(h,w).astype(bool)]=0
   if seq in selected and model.generation>0 and model.world_id in model.handles:
    candidate=selected[seq];sid,expected=constant_update(p,*candidate['source']['fields'][5:7],candidate['value'])
    before=model.handles[sid]['pixels'];require(before.shape==expected.shape,'Full fill leaves live surface')
    changed=int(np.count_nonzero(before!=expected))
    if candidate['value'] and changed:
     tags[sid][:]=seq;proofs[seq]=dict(candidate,generation=model.generation,update_sequence=seq,
      world_surface=model.world_id,world_created=lives[model.world_id][0],world_deleted=lives[model.world_id][1],
      native_update_matches_argument=True,changed_words=changed,expected_fill_sha256=cp.words_hash(expected))
  elif op==3:
   w=struct.unpack('<10I',p);l,t,r,b,x,y=w[2:8];src=model.handles[w[0]];dst=model.handles[w[1]]
   prior=tags[w[0]][t:b,l:r].copy();pixels=src['pixels'][t:b,l:r];mask=np.ones(pixels.shape,dtype=bool) if not w[8] else pixels!=w[9]
   if len(found)<3 and np.any(tags[w[0]]) and src['bits']==dst['bits']==16 and model.world_id in model.handles:
    d=diags[seq]
    require(d['words']==list(w)+[0,0,0] and d['mismatches']==0 and bound[seq][13]==0
     and (d['source_width'],d['source_height'],d['target_width'],d['target_height'])==(src['pixels'].shape[1],src['pixels'].shape[0],dst['pixels'].shape[1],dst['pixels'].shape[0])
     and d['pixels']==dst['pixels'].size
     and d['source_sha256']==cp.words_hash(src['pixels']) and d['before_sha256']==cp.words_hash(dst['pixels']),
     'Actual GPU full-fill source/prior plane differs')
    sink=dict(tags=tags[w[0]].copy(),source_sha256=d['source_sha256'],caller=list(bound[seq]),diagnostic=d,world_surface=model.world_id,generation=model.generation)
   tags[w[1]][y:y+b-t,x:x+r-l][mask]=prior[mask]
  elif op==7:tags.pop(struct.unpack('<I',p)[0])
  elif op==11:
   left,right=struct.unpack('<2I',p);tags[left],tags[right]=tags[right],tags[left]
  elif op==19:
   for values in tags.values():values[:]=0
  model.apply(op,seq,p)
  if sink:
   d=sink['diagnostic'];require(cp.words_hash(model.handles[w[1]]['pixels'])==d['actual_sha256']==d['expected_sha256'],
    'Actual complete GPU destination differs after full-fill source copy')
   for tag in np.unique(sink['tags']):
    if not tag or int(tag) not in proofs or len(found)>=3:continue
    if proofs[int(tag)]['world_surface']!=sink['world_surface'] or proofs[int(tag)]['generation']!=sink['generation']:continue
    if any(v['update_sequence']==int(tag) or v['sink_sequence']==seq or v['actual_source_sha256']==sink['source_sha256'] for v in found):continue
    q=dict(proofs[int(tag)],sink_sequence=seq,sink_world_surface=sink['world_surface'],sink_generation=sink['generation'],surviving_fill_words=int(np.count_nonzero(sink['tags']==tag)),
     actual_source_sha256=sink['source_sha256'],expected_source_sha256=d['source_sha256'],complete_gpu_source_verified=True,
     complete_gpu_destination_verified=True,copy_caller=sink['caller'])
    check_witness(q);found.append(q)
  if len(found)==3:break
 require(len(found)==3,'Missing three meaningful natural full-fill/GPU plane witnesses')
 tails=[dict(row=list(row),source_arguments_retained=False,lower_command_fence=max((r[41] for r in rows[:i] if r[41]),default=0)) for i,row in fills[len(source):]]
 require(all(row['row'][1]==0x58bc5a and row['row'][13]==0 and row['lower_command_fence']>last_import for row in tails),'Unexpected unmatched fill tail')
 result=dict(pair=seed['pair'],domain=seed['domain'],scenario=seed['scenario'],identity=a['session_identity'],provider=seed['provider'],verified=True,
  source_fill_calls=len(source),joined_source_caller_calls=len(source),full_source_fill_calls=sum(full),policy_initializations_excluded=policy,
  last_source_world=last_world,last_command_import=last_import,unique_full_fill_update_candidates=len(selected),witnesses=found,unpaired_tail_fills=tails,
  refused_copy_calls=refused,scope='Actual bounded full fills while the same imported World surface and generation remain live through both the UPDATE and GPU COPY sink. Exact original source colors/rectangles and ordered caller/canonical-object/native UPDATE joins; meaningful argument-derived full writes reach checked complete recorded actual GPU source and destination planes. Full overwrite has no exterior. Native initialization markers, retired-World transitions and two unpaired tail fills receive no invocation/output credit. No clipped fill, caller ABI, arbitrary driver/history or new native execution.')
 check_case(result)
 return result


def close_rows(plan,parent,cases):
 validate(plan);require(parent['success'] and parent['dossier_identity']==fingerprint(requirements(plan)) and len(parent['requirements'])==156,'Changed accepted inventory')
 require(len(cases)==4 and {(c['domain'][0],tuple(c['domain'][1])) for c in cases}=={(p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600))}
  and len({c['identity'] for c in cases})==4,'Missing/duplicate full-fill domain')
 for c in cases:
  check_case(c)
  require(any(row['state']=='passed' and row['configuration']==c['domain'][0] and any(v.get('execution')==c['pair'] for v in row.get('admitted_execution_evidence',[])) for row in parent['requirements']),
   'Full-fill case missing from admitted primary cohort')
 rows=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id']==LEAF:
   require(row['state']=='pending','Full-fill row already accepted')
   row.update(state='passed',admitted_execution_evidence=[dict(execution=c['pair'],actual_world_extents=[c['domain'][1]],reason=c['scope']) for c in cases if c['domain'][0]==row['configuration']])
  rows.append(row)
 require(sum(v['state']=='passed' for v in rows)==parent['passed_requirements']+2,'Full-fill promotion exceeds two rows')
 return rows


def sources(reader):
 b=next(b for b in reader.json('research/runtime/coverage/register.json')['behaviors'] if b['id']==BID)
 return {n:reader.digest(n) for role in ('implementation','tests','validation_dependencies') for n in b[role]}


def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--parent',required=True);p.add_argument('--report',required=True);args=p.parse_args()
 require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'New immutable owned report required')
 q=dict(schema=1,success=False)
 try:
  with Reader(ROOT) as r:
   before=sources(r);parent=r.json(args.parent);cases=[]
   for seed in r.json(COPY)['executions']:
    if seed['scenario']!='normal-storage':continue
    cases.append(case(r,seed));print('Verified full fills',seed['domain'],[(w['update_sequence'],w['sink_sequence']) for w in cases[-1]['witnesses']],flush=True)
   rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases);require(before==sources(r),'Analysis sources changed')
   q=dict(schema=1,kind='retained_natural_full_fill_closeout',success=True,scope='Only full-fill for both explicit profiles at640/800 while the same imported World remains live at actual UPDATE and GPU COPY output. Twelve natural argument/caller/live-handle/complete-plane witnesses; preserve every other row, failure, historical native phase, terminal exclusion and pending decision. No new game/native execution, fixture, clipping or whole ABI claim.',analysis_sources=before,analysis_sources_stable=True,
    sources=parent['sources']|{n:h for n,h in before.items() if n not in parent['sources']},parent=dict(record=args.parent,sha256=r.digest(args.parent)),
    cases=cases,requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
    closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),passed_requirements=parent['passed_requirements']+2,pending_requirements=154-parent['passed_requirements'],newly_passed_requirements=2,
    actual_cases=4,full_fill_witnesses=12,unpaired_tail_fills=8,current_native_engine_execution=False,original_game_runs=0,new_fixture=False,new_native_code=False,historical_failures_retired=False)
 except Exception as e:q.update(success=False,error=str(e))
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=h)),flush=True)
 return int(not q['success'])


if __name__=='__main__':raise SystemExit(main())
