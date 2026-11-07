#!/usr/bin/env python3
"""Close bounded CPU write/readback rows from actual retained native replies."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import numpy as np
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import History,commands,require
from rendering_capture import capture
from rendering_menu_acceptance import artifact
from cpu_reader_journal import wire_reads

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-operations-results-20261107/'
PARENT='research/runtime/rendering-identity-results-20261107/closeout-analysis-03.json'
COPY='research/runtime/rendering-copy-results-20261106/closeout-analysis-01.json'
BID='NP.rendering-operations-closeout'
LEAF='operation.cpu-write-readback'
spec=importlib.util.spec_from_file_location('operations_blend',ROOT/'tools/close-rendering-blend-direct.py')
blend=importlib.util.module_from_spec(spec);spec.loader.exec_module(blend)
sha=lambda b:hashlib.sha256(b).hexdigest()
words_hash=lambda a:sha(a.astype('<u2').tobytes())


def checked(reader,ref):
 require(reader.digest(ref['record'])==ref['sha256'],'Changed retained input reference')
 return reader.json(ref['record'])


def expected_write(before,patch,mask,x,y):
 require(before.ndim==patch.ndim==mask.ndim==2 and patch.shape==mask.shape
         and x>=0 and y>=0 and x+patch.shape[1]<=before.shape[1]
         and y+patch.shape[0]<=before.shape[0], 'CPU writer leaves owned storage')
 expected=before.copy();region=expected[y:y+patch.shape[0],x:x+patch.shape[1]]
 prior=region.copy();region[mask]=patch[mask]
 changed=mask&(prior!=patch)
 written=np.zeros(before.shape,dtype=bool);written[y:y+patch.shape[0],x:x+patch.shape[1]]=mask
 require(np.array_equal(expected[~written],before[~written]),'CPU writer changed preserved words')
 changed_all=np.zeros(before.shape,dtype=bool);changed_all[y:y+patch.shape[0],x:x+patch.shape[1]]=changed
 equal=~changed_all
 return expected,changed,dict(changed_words=int(changed.sum()),physical_unwritten_words=int((~written).sum()),
  equal_history_words=int(equal.sum()),equal_nonzero_history_words=int(np.count_nonzero(before[equal])),before_sha256=words_hash(before),
  after_sha256=words_hash(expected),equal_before_sha256=words_hash(before[equal]),equal_after_sha256=words_hash(expected[equal]))


def reply_pixels(expected,actual,header):
 require(len(actual)==64+expected.size*2 and actual[:64]==header,'Wrong actual CPU reply header/extent')
 require(actual[64:]==expected.astype('<u2').tobytes(),'Actual GPU reply differs from independent current history')
 return dict(bytes=len(actual),words=expected.size,sha256=sha(actual),pixels_sha256=sha(actual[64:]),verified=True)


def check_witness(w):
 require(w['write_sequence']<w['sink_sequence'] and w['earlier_reply_ticket']>0
  and w['source_created']<w['write_sequence'] and w['observed_created']<w['sink_sequence']
  and w['changed_words']>0 and w['equal_history_words']>0 and w['equal_nonzero_history_words']>0
  and w['equal_before_sha256']==w['equal_after_sha256'] and w['surviving_written_words']>0
  and w['actual_source_sha256']==w['expected_source_sha256'] and w['full_gpu_source_verified']
  and w['full_gpu_destination_verified'] and w['copy_caller'][13]==0,
  'Incomplete CPU upload/actual GPU source and destination witness')


def case(reader,seed):
 a,c=[checked(reader,seed['pair'][k]) for k in ('acquisition','comparison')]
 require(a['success'] and c['success'] and not a['original_pixels_used_as_native_inputs']
  and a['configuration']['software_flag'] is (seed['domain'][0]=='player-software')
  and a['configuration']['terrain_lighting']=='native'
  and c['original_reference']['continuous_from_owned_initial_history']
  and not c['original_reference']['native_player_pixels_read']
  and not c['original_reference']['original_destination_capture_read']
  and c['comparison']['success'] and c['comparison']['mismatching_pixels']==0,
  'Incomplete native-only independent accepted storage domain')
 ad=json.loads(artifact(reader,a,'native-output-admission.json'))
 require(ad['native_only']['policy']=='native-only-rendering-v1','Wrong native-only source policy')
 streams=[v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('commands') and v['path'].endswith('.bin')]
 require(len(streams)==1,'Ambiguous command/reply capture')
 raw=reader.read_bytes(streams[0]['record'])
 requests,counts,digest=wire_reads(raw,a['session_identity'],artifact(reader,a,'native-cpu-read-counts.bin'),artifact(reader,a,'render-commands.bin.cpu-reply'))
 require(digest==c['captured_command_stream_sha256'] and len(requests)==counts['completed'],'Different command/reply denominator')
 rows=capture(raw);replies={v['sequence']:body for v,body in rows if v['kind']=='cpu-read'}
 stream=b''.join(body for v,body in rows if v['kind']=='fragment')
 prior={v['sequence']:v for v in c['comparison']['cpu_reads']}
 require(len(prior)==len(requests)==len(replies),'Accepted reply comparisons omitted')
 copy=blend.module('close-rendering-opaque-cropped-copy')
 header,routes,route_proof=copy.routes(reader,a);bound,refused=copy.command_join(stream,routes)
 diagnostics=[json.loads(line) for line in artifact(reader,a,'copies.jsonl').splitlines()]
 diags={d['sequence']:d for d in diagnostics};require(len(diags)==len(diagnostics),'Duplicate actual GPU copy diagnostic')
 model=History(blend.OriginalWorlds(reader,c));tags={};created={};proofs={};found=[];reads=[];updates=0;masked=0
 from storage_cpu_read_binding import reply_header
 for op,seq,p in commands(stream):
  sink=None
  if op==1:
   w=struct.unpack_from('<7I',p);tags[w[0]]=np.zeros((w[2],w[1]),dtype='uint32');created[w[0]]=seq
  elif op in (2,20):
   sid,x,y,width,height=struct.unpack_from('<5I',p);s=model.handles[sid];n=width*height*(s['bits']//8)
   patch=model.image(p[20:20+n],width,height,s['bits']);mask=np.ones(patch.shape,dtype=bool) if op==2 else np.frombuffer(p[20+n:],dtype='u1').reshape(patch.shape).astype(bool)
   require(len(p)==20+n+(width*height if op==20 else 0),'Changed CPU writer argument extent')
   before=s['pixels'][y:y+height,x:x+width];require(before.shape==patch.shape,'Writer patch outside live surface')
   changed=mask&(before!=patch);region=tags[sid][y:y+height,x:x+width];region[changed]=0
   if reads and s['bits']==16 and np.any(changed) and len(found)<3 and len(proofs)<4096:
    expected,delta,proof=expected_write(s['pixels'],patch,mask,x,y)
    if proof['equal_history_words'] and proof['equal_nonzero_history_words']:
     region[delta]=seq;proofs[seq]=dict(proof,write_sequence=seq,earlier_reply_ticket=reads[-1]['ticket'],source_created=created[sid],writer_surface=sid,upload_kind='masked-update' if op==20 else 'full-update',expected_after=expected)
   updates+=1;masked+=op==20
  elif op==3:
   w=struct.unpack('<10I',p);l,t,r,b,x,y=w[2:8];values=tags[w[0]][t:b,l:r].copy();source=model.handles[w[0]];target=model.handles[w[1]];pixels=source['pixels'][t:b,l:r]
   mask=np.ones(pixels.shape,dtype=bool) if not w[8] else pixels!=w[9]
   if len(found)<3 and np.any(tags[w[0]]) and source['bits']==target['bits']==16:
    d=diags[seq];caller=bound[seq]
    require(d['words']==list(w)+[0,0,0] and d['mismatches']==0 and caller[13]==0
     and (d['source_width'],d['source_height'],d['target_width'],d['target_height'])==(source['pixels'].shape[1],source['pixels'].shape[0],target['pixels'].shape[1],target['pixels'].shape[0])
     and d['source_sha256']==copy.words_hash(source['pixels']) and d['before_sha256']==copy.words_hash(target['pixels']),
     'Actual GPU source/prior plane differs from independent CPU transport history')
    sink=dict(tags=tags[w[0]].copy(),nonzero=source['pixels']!=0,source_sha256=d['source_sha256'],expected_source_sha256=copy.words_hash(source['pixels']),surface=w[0],created=created[w[0]],diagnostic=d,caller=list(caller))
   tags[w[1]][y:y+b-t,x:x+r-l][mask]=values[mask]
  elif op==7:
   sid=struct.unpack('<I',p)[0];tags.pop(sid);created.pop(sid)
  elif op==11:
   left,right=struct.unpack('<2I',p);tags[left],tags[right]=tags[right],tags[left]
  elif op==19:tags[struct.unpack_from('<I',p)[0]][:]=0
  model.apply(op,seq,p)
  if seq in proofs:
   q=proofs[seq];require(np.array_equal(q.pop('expected_after'),model.handles[q['writer_surface']]['pixels']),'Transport model disagrees with independent CPU upload')
  if sink:
   require(copy.words_hash(model.handles[w[1]]['pixels'])==sink['diagnostic']['actual_sha256']==sink['diagnostic']['expected_sha256'],'Actual complete GPU copy destination differs')
   for write in np.unique(sink['tags']):
    if not write or int(write) not in proofs or len(found)>=3 or any(v['write_sequence']==int(write) or v['sink_sequence']==seq or v['actual_source_sha256']==sink['source_sha256'] for v in found):continue
    at=sink['tags']==write
    if not np.any(sink['nonzero'][at]):continue
    witness=dict(proofs[int(write)],sink_sequence=seq,observed_surface=sink['surface'],observed_created=sink['created'],surviving_written_words=int(at.sum()),actual_source_sha256=sink['source_sha256'],expected_source_sha256=sink['expected_source_sha256'],full_gpu_source_verified=True,full_gpu_destination_verified=True,copy_caller=sink['caller'])
    check_witness(witness);found.append(witness)
  if op==21:
   w=struct.unpack('<10I',p);sid,ticket=w[:2];actual=replies[seq]
   result=reply_pixels(model.handles[sid]['pixels'],actual,reply_header(list(w)))
   model.reads.pop(seq)
   require(prior[seq]==dict(sequence=seq,bytes=len(actual),sha256=sha(actual)),'Frozen actual reply hash differs')
   reads.append(dict(result,sequence=seq,ticket=ticket,surface=sid,created=created[sid],world_generation=w[6]|w[7]<<32))
 require(not model.handles and len(reads)==len(requests) and len(found)==3,'Missing complete CPU operation domain: reads='+str(len(reads))+' candidates='+str(len(proofs))+' witnesses='+str(len(found)))
 result=dict(pair=seed['pair'],domain=seed['domain'],scenario=seed['scenario'],identity=a['session_identity'],provider=seed['provider'],verified=True,reads=reads,wire_counts=counts,updates=updates,masked_updates=masked,witnesses=found,refused_copy_calls=refused,
  reader_scope='All actual DC/panel read replies and actual owned CPU upload/GPU copy transport are checked separately. No same-physical-buffer read-modify-read chain, arbitrary escaped raw-reader, upstream original producer arithmetic, font/DC object lifetime or whole original ABI claim.')
 check_case(result)
 return result


def check_case(c):
 require(c['verified'] and c['scenario']=='normal-storage' and c['updates']>c['masked_updates']>0
  and len(c['reads'])>=3 and len({r['ticket'] for r in c['reads']})==len(c['reads'])
  and all(r['verified'] and r['created']<r['sequence'] and r['world_generation']>0 for r in c['reads'])
  and len({r['sha256'] for r in c['reads']})>=3 and len(c['witnesses'])==3,
  'Incomplete actual typed CPU transport domain')
 require(c['wire_counts']['completed']==c['wire_counts']['last_completed_ticket']==len(c['reads'])
  and sum(c['wire_counts']['kinds'].values())==len(c['reads'])
  and [v['ticket'] for v in c['reads']]==list(range(1,len(c['reads'])+1)), 'CPU reply counter/ticket coverage differs')
 require(len({w['actual_source_sha256'] for w in c['witnesses']})==3
  and all(w['upload_kind'] in ('full-update','masked-update') for w in c['witnesses']), 'Repeated GPU source or unknown CPU writer')
 for w in c['witnesses']:check_witness(w)
 require(len({w['write_sequence'] for w in c['witnesses']})==len({w['sink_sequence'] for w in c['witnesses']})==3,'Repeated writer/GPU output cannot supply coverage')


def close_rows(plan,parent,cases):
 validate(plan);require(parent['success'] and parent['dossier_identity']==fingerprint(requirements(plan)) and len(parent['requirements'])==156,'Changed accepted inventory')
 require(len(cases)==4 and {(c['domain'][0],tuple(c['domain'][1])) for c in cases}=={(p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600))} and len({c['identity'] for c in cases})==4,'Missing/duplicate actual storage domain')
 for c in cases:check_case(c)
 result=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id']==LEAF:
   require(row['state']=='pending','CPU operation already admitted')
   row.update(state='passed',admitted_execution_evidence=[dict(execution=c['pair'],actual_world_extents=[c['domain'][1]],reason='Every typed GPU-to-CPU reply matches independently reconstructed current history and terminal counters/reply; actual CPU uploads retain nonzero independently validated history words and reach independently checked complete actual GPU COPY source and destination planes. Existing native-only identity/lifetime cases; arbitrary raw readers and original producer/ABI/font lifetime remain separate.') for c in cases if c['domain'][0]==row['configuration']])
  result.append(row)
 require(sum(r['state']=='passed' for r in result)==parent['passed_requirements']+2,'Promotion exceeds two typed CPU operation rows')
 return result


def sources(reader):
 b=next(b for b in reader.json('research/runtime/coverage/register.json')['behaviors'] if b['id']==BID)
 return {n:reader.digest(n) for role in ('implementation','tests','validation_dependencies') for n in b[role]}


def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args()
 require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'New immutable owned report required')
 q=dict(schema=1,success=False)
 try:
  with Reader(ROOT) as r:
   before=sources(r);parent=r.json(PARENT);seed=r.json(COPY)
   cases=[]
   for c in seed['executions']:
    if c['scenario']!='normal-storage':continue
    cases.append(case(r,c));print('Verified CPU operations',c['domain'],len(cases[-1]['reads']),flush=True)
   rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
   require(before==sources(r),'Analysis sources changed')
   q=dict(schema=1,kind='retained_typed_cpu_operation_closeout',success=True,scope='Close only typed CPU write/readback for both profiles at actual640/800 from four accepted storage cases. Full actual replies use independent ordered history; actual CPU uploads retain nonzero prior-history words and changed words, and reach complete actual GPU COPY planes. No new game, original comparison, native code/build, fixture, arbitrary raw-reader/original producer/whole ABI/font lifetime or final decision claim.',sources=parent['sources']|{n:h for n,h in before.items() if n not in parent['sources']},analysis_sources=before,analysis_sources_stable=True,parent=dict(record=PARENT,sha256=r.digest(PARENT)),cases=cases,requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),passed_requirements=parent['passed_requirements']+2,pending_requirements=154-parent['passed_requirements'],newly_passed_requirements=2,actual_cases=4,actual_cpu_reads=sum(len(c['reads']) for c in cases),writer_witnesses=12,current_native_engine_execution=False,historical_failures_retired=False,original_game_runs=0,new_fixture=False,new_native_code=False)
 except Exception as e:q.update(success=False,error=str(e))
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=args.report,sha256=h)),flush=True)
 return int(not q['success'])


if __name__=='__main__':raise SystemExit(main())
