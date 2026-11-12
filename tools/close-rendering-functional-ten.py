#!/usr/bin/env python3
"""Check eight retained partial-write domains and two current public returns."""
import argparse,hashlib,importlib.util,io,json,struct,sys
from pathlib import Path
import numpy as np
from PIL import Image
from coverage_evidence import Reader,write_report
from rendering_pilot import History,acquisition,commands,require
from rendering_menu_acceptance import artifact
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-functional-ten-results-20261107/'

def module(name):
 s=importlib.util.spec_from_file_location('functional_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
copy=module('close-rendering-opaque-cropped-copy')

def partial_destination(source,before,words,diagnostic):
 after,changed=copy.checked_destination(source,before,words,diagnostic)
 l,t,r,b,x,y=words[2:8];outside=np.ones(before.shape,dtype=bool);outside[y:y+b-t,x:x+r-l]=False
 preserved=int(np.count_nonzero(outside));nonzero=int(np.count_nonzero(before[outside]))
 require(changed>0 and preserved>0 and nonzero>0,'Trivial/full-destination write is not a partial-write witness')
 require(np.array_equal(before[outside],after[outside]),'Partial write changed exterior WORDs')
 return after,dict(changed_words=changed,preserved_words=preserved,preserved_nonzero_words=nonzero,before_exterior_sha256=copy.words_hash(before[outside]),after_exterior_sha256=copy.words_hash(after[outside]))

def partial_case(reader,case):
 a,c=[reader.json(case['pair'][k]['record']) for k in ('acquisition','comparison')]
 require(a['success'] is c['success'] is True and all(reader.digest(case['pair'][k]['record'])==case['pair'][k]['sha256'] for k in case['pair']),'Changed/incomplete retained execution')
 require(c['comparison']['success'] and c['comparison']['mismatching_pixels']==0 and c['original_reference']['continuous_from_owned_initial_history'] and not c['original_reference']['native_player_pixels_read'] and not c['original_reference']['original_destination_capture_read'],'Missing independent original comparison')
 streams=[artifact(reader,a,Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')];observed=acquisition(streams,a['session_identity'])
 require(hashlib.sha256(observed['commands']).hexdigest()==c['captured_command_stream_sha256'],'Changed compared commands')
 header,rows,routes=copy.routes(reader,a);bound,refused=copy.command_join(observed['commands'],rows)
 records=[json.loads(v) for v in artifact(reader,a,'copies.jsonl').splitlines()];diagnostic={v['sequence']:v for v in records};require(len(diagnostic)==len(records),'Duplicate diagnostic')
 model=History(copy.base.OriginalWorlds(reader,c));created={};witnesses=[]
 for op,seq,p in commands(observed['commands']):
  if op==1:created[struct.unpack_from('<I',p)[0]]=seq
  if op==3 and model.generation>0 and seq in diagnostic:
   w=struct.unpack('<10I',p);source,target=model.handles[w[0]],model.handles[w[1]];area=(w[4]-w[2])*(w[5]-w[3]);size=target['pixels'].size
   if (w[8:]==(0,0) and w[0]!=w[1] and 0<area<size and all(v['bits']==16 and v['masks']==(0xf800,0x7e0,31) for v in (source,target)) and model.generation not in {v['generation'] for v in witnesses}):
    try:after,proof=partial_destination(source['pixels'],target['pixels'],w,diagnostic[seq])
    except ValueError as e:
     if str(e)=='Trivial/full-destination write is not a partial-write witness':model.apply(op,seq,p);continue
     raise
    row=bound[seq];require(row[13]==0 and row[14] and row[15] in (1,2) and header[15]<=row[1]<header[15]+header[16],'Unsuccessful/unowned actual copy caller')
    witnesses.append(dict(generation=model.generation,sequence=seq,words=list(w),source_created=created[w[0]],target_created=created[w[1]],source_shape=list(source['pixels'].shape),target_shape=list(target['pixels'].shape),caller=list(row),route=row[15],source_sha256=copy.words_hash(source['pixels']),before_sha256=copy.words_hash(target['pixels']),actual_sha256=diagnostic[seq]['actual_sha256'],expected_sha256=copy.words_hash(after),**proof))
  model.apply(op,seq,p)
  if op==7:created.pop(struct.unpack('<I',p)[0])
  if len(witnesses)==3:break
 require(len(witnesses)==3 and len({v['generation'] for v in witnesses})==3,'Missing three nontrivial partial-write generations: '+str([(v['generation'],v['sequence']) for v in witnesses]))
 return dict(verified=True,domain=case['domain'],scenario=case['scenario'],identity=case['identity'],pair=case['pair'],provider=case['provider'],witnesses=witnesses,bound_copy_commands=len(bound),historical_refused_calls=refused,whole_player_sources_current=False,scope='Three successful actual caller-bound RGB565 partial destinations per retained normal domain. Independent full source/before/target hashes prove changed interiors and preserved nonzero exterior WORDs. Existing continuous viewport comparisons remain at their recorded engine source phase; no refreshed whole-GUI execution or arbitrary raw CPU/BYTE/padding/driver proof.')

def assess_return(a,callbacks):
 require(a['success'] and a['sources_stable'] and a['original_game_runs']==1 and a['exit_code']==0 and a['private_wine_stopped'] and a['prefix_pool']['returned'] and not a.get('forced_private_prefix_stop',False),'Incomplete public return')
 require(a['configuration']['actual_size']==[800,600] and a['configuration']['skip_intros'] and not a['configuration']['compiled_test_driver_invoked'] and ('--software-rendering' in a['command']) is a['configuration']['software_flag'],'Wrong return domain/driver')
 require(a['proof']['entry'] in ('public_quit_battle','public_escape_results') and a['proof']['natural_victory_defeat_proved'] is False,'Invented natural outcome')
 states=a['states'];screens=[v['screen'] for v in states]
 require(screens in ([3,10,3,22,14,25,14,7,17,26,22,3,22,3],[3,10,3,22,14,25,14,7,26,22,3,22,3]),'Wrong actual original menu sequence')
 require(len({v['thread'] for v in states})==1 and states[0]['thread']>0 and all(v['ready'] and v['status'] in (0,1) and v['handoff']==(1 if v['screen']==7 else 2 if v['screen']==26 else 0) for v in states),'Invalid original state/owner')
 require(all(b['generation']>x['generation'] and b['ack']==x['ack']+1 for x,b in zip(states,states[1:])),'Request ACK/generation gap')
 require(callbacks[:8]==[[3,3],[10,1],[3,2],[22,2],[14,2],[25,0],[14,1],[7,12]] and callbacks[-5:]==[[26,2],[22,3],[3,2],[22,3],[3,4]] and (17 not in screens or callbacks[-6]==[17,7]),'Wrong original Results/input/Quit effects')
 require(a['proof']['world_ready']['world_returns']>=10 and a['proof']['world_ready']['thread']==states[0]['thread'],'No returned original World work')
 resident=a['resident'];n=int(resident['imports']);require(n>=10 and int(resident['completed'])==int(resident['verified_returns'])==n and not resident['pending'] and resident['normal_quit_retired'],'Unbalanced World delivery')
 retirement=a['retirement'];g=retirement['graphics_retirement'];require(retirement['state']=='completed' and retirement['forwarded']==0 and retirement['worlds']==n and g['process_exit'] and g['resources_released'] and not g['surviving_owners'] and all(v==0 for v in g['counts'].values()),'Pending/retired graphics work survived')
 require(a['original_manifest_verified_before_after'],'Original inputs unverified')
 if a['configuration']['software_flag']:require(resident['renderer'].lower().startswith(('llvmpipe','softpipe')),'Wrong software provider')
 return dict(verified=True,imports=n,original_callbacks=callbacks,normal_quit=True,zero_surviving_graphics_owners=True,natural_victory_defeat_proved=False,world_pixel_equivalence_proved=False)

def results_case(reader,name):
 a=reader.json(name);require(all(reader.digest(n)==h for n,h in a['sources'].items()),'Return source changed')
 admission=json.loads(artifact(reader,a,'native-output-admission.json'))['native_only'];require(admission['policy']=='native-only-rendering-v1' and all(reader.digest(n)==h for n,h in admission['sources'].items()),'Changed native DLL source/admission closure')
 decoder=module('test-menu-observer');callbacks=[[v['menu_id'],v['argument']] for v in decoder.decode(artifact(reader,a,'events.bin')) if v['event']==3]
 proof=assess_return(a,callbacks)
 for state in a['states']:
  raw=artifact(reader,a,state['wire_path']);w=struct.unpack_from('<40I',raw);require(w[32:40]==tuple(int(state[k]) for k in ['sequence','generation','screen','ready','ack','status','thread','handoff']),'Wrong raw original menu snapshot')
 shots={v['label']:v for v in a['screenshots']}
 for label in ['results-ready','quick-returned','main-returned','main-before-quit']:
  shot=shots[label];raw=artifact(reader,a,shot['path']);require(hashlib.sha256(raw).hexdigest()==shot['sha256'],'Changed return screenshot');image=Image.open(io.BytesIO(raw));require(image.size==(1280,1024),'Unexpected host extent');image.verify()
 for label in ['main-returned','main-before-quit']:
  shot=shots[label];require(shot['host_focus']['host_focused'],'Main host lacks focus');nodes=[v for v in shot['nodes'] if v['role']=='button' and v['name']=='Quick Battle'];require(len(nodes)==1 and all(nodes[0][k] for k in ['focused','enabled','showing']),'Returned Main control not ready/focused')
  x,y,w,h=nodes[0]['rect'];pixels=list(Image.open(io.BytesIO(artifact(reader,a,shot['path']))).convert('RGB').crop((x,y,x+w,y+h)).getdata());require(min(w,h)>0 and x>=0 and y>=0 and x+w<=1280 and y+h<=1024 and sum(max(v)>20 for v in pixels)>=len(pixels)*.9 and sum(min(v)>120 for v in pixels)>=100,'Returned Main control not painted')
 require(any(v.get('object')=='quickResultAction2' and v['physical']['down_up'] and v['physical']['host_focus']['host_focused'] for v in a['actions']),'No physical Results return')
 require(any(v.get('action')=='main-focused-space' and v['physical']['down_up'] and v['physical']['host_focus']['host_focused'] for v in a['actions']),'No focused post-return input')
 return dict(record=name,sha256=reader.digest(name),configuration=a['configuration'],identity=a['session'],proof=proof,sources=a['sources']|admission['sources'])

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--partial-only',action='store_true');p.add_argument('--report',required=True);args=p.parse_args();require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh report required')
 q=dict(schema=1,kind='functional_ten_partial_analysis',success=False)
 try:
  with Reader(ROOT) as r:
   if not args.partial_only:q=full_closeout(r)
   else:
    parent=r.json('research/runtime/rendering-copy-results-20261106/closeout-analysis-01.json');cases=[partial_case(r,c) for c in parent['executions']]
    q.update(success=True,cases=cases,retained_cases=len(cases),sources=analysis_sources(r),preserved_words=sum(w['preserved_words'] for c in cases for w in c['witnesses']),witnesses=sum(len(c['witnesses']) for c in cases),scope='Retained-data partial-destination analysis, not a fresh original game or current whole-player claim.')
 except Exception as e:q.update(success=False,error=str(e))
 h=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=args.report,sha256=h,witnesses=q.get('witnesses'))),flush=True);return 0 if q['success'] else 1


def native_case_expected(case,raw,native):
 """Independent WORD reconstruction from actual committed plan pieces."""
 words=case['words'];width,height=words[1:3];count=width*height
 require(len(raw)==48+6192+2*count and len(native)==2*count,'Native partial control extent differs')
 h=struct.unpack_from('<12I',raw);plan=struct.unpack_from('<12I',raw,48)
 require(h[:5]==(0x50435047,1,*words[:3]) and plan[:3]==(1,6192,h[6]) and 0<=h[6]<=256,'Native partial control identity/count')
 before=(np.arange(count,dtype='uint32')+0xa000).reshape(height,width)&0xffff
 source=(0x1000+np.arange(count,dtype='uint32')*73).reshape(height,width)&0xffff
 expected=before.copy();written=np.zeros(before.shape,dtype=bool)
 pieces=[struct.unpack_from('<6I',raw,48+48+24*i) for i in range(h[6])]
 if words[4]==0:
  require(h[5]==0 and h[6]==0,'Ordinary un-clipped path unexpectedly has a clip plan/error')
  l,t,r,b=words[8:12];x,y,dr,db=words[12:16];require(r-l==dr-x and b-t==db-y,'Unexpected un-clipped scaling')
  pieces=[(l,t,r-l,b-t,x,y)]
 for l,t,piece_width,piece_height,x,y in pieces:
  r,b=l+piece_width,t+piece_height
  require(0<=l<r<=width and 0<=t<b<=height and x>=0 and y>=0 and x+r-l<=width and y+b-t<=height,'Committed native prefix leaves owned pixels')
  values=np.full((b-t,r-l),0x3c5a,dtype='uint32') if words[7] else source[t:b,l:r]
  mask=values!=int(source.flat[5]) if words[5] else np.ones(values.shape,dtype=bool)
  region=expected[y:y+b-t,x:x+r-l];region[mask]=values[mask];written[y:y+b-t,x:x+r-l]|=mask
 actual=np.frombuffer(native,dtype='<u2').astype('uint32').reshape(height,width)
 require(np.array_equal(expected,actual) and raw[48+6192:]==native,'Actual native WORDs differ from independent committed-prefix oracle')
 require(np.array_equal(actual[~written],before[~written]),'Native partial copy changed exterior/skipped WORDs')
 if 'late-source-failure' in case['name']:require(h[5]==0x88760096 and h[6]==1 and np.any(written) and np.any(~written),'Missing valid-prefix error policy')
 if 'first-source-failure' in case['name']:require(h[5]==0x88760096 and h[6]==0 and np.array_equal(before,actual),'First-piece refusal changed destination')
 if 'empty' in case['name']:require(h[5]==0 and h[6]==0 and np.array_equal(before,actual),'Empty clip changed destination')
 return dict(verified=True,name=case['name'],result=h[5],pieces=h[6],effective_pieces=len(pieces),written_words=int(np.count_nonzero(written)),preserved_words=int(np.count_nonzero(~written)),actual_sha256=hashlib.sha256(native).hexdigest())


def native_controls(reader):
 import tarfile
 name=PREFIX+'native-partial-controls-04.json';report=reader.json(name)
 require(report['success'] and report['sources_stable'] and all(reader.digest(n)==h for n,h in report['sources'].items()),'Native partial controls incomplete/changed')
 raw=reader.read_bytes(report['raw_record']);producer=module('test-native-gpu-copy-plan');cases=producer.cpu_cases();records=[]
 with tarfile.open(fileobj=io.BytesIO(raw),mode='r:gz') as archive:
  def body(path):
   f=archive.extractfile('controls/'+path);require(f is not None,'Missing retained native control '+path);return f.read()
  for case,record in zip(cases,report['cases']):
   require(case['name']==record['name'] and record['success'] and record['final_native_surfaces']==0,'Native control ordering/cleanup differs')
   require(struct.unpack('<16I',body(case['name']+'/input.bin'))==tuple(v&0xffffffff for v in case['words']),'Changed actual native control inputs')
   actual=body(case['name']+'/native.bin');require(hashlib.sha256(actual).hexdigest()==record['gpu_sha256'],'Native control hash differs')
   records.append(native_case_expected(case,body(case['name']+'/output.bin'),actual))
 require(len(records)==len(cases)==len(report['cases']) and {'clip-late-source-failure','clip-first-source-failure','clip-empty'}<={v['name'] for v in records},'Missing required native partial cases')
 return dict(verified=True,record=name,sha256=reader.digest(name),sources=report['sources'],cases=records,scope='Fresh current production facade/bridge/GL output, independently rebuilt from committed WORD plan pieces. Native prefix/error/empty policy only; no original driver or ABI equivalence.')


def analysis_sources(reader):
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
 b=next(v for v in reg['behaviors'] if v['id']=='NP.partial-write-closeout')
 paths=set(b['implementation']+b['tests']+b['validation_dependencies'])|{'tools/close-rendering-blend-direct.py','tools/close-rendering-destination-history.py','tools/resident_capture.py','tools/inspect-canvas-producers.py'}
 return {n:reader.digest(n) for n in sorted(paths)}


def close_rows(plan,parent,partial,returns):
 from rendering_acceptance import requirements,validate
 from coverage_claims import fingerprint
 validate(plan);required=requirements(plan)
 require(parent['passed_requirements']==52 and parent['dossier_identity']==fingerprint(required) and [(v['configuration'],v['id']) for v in parent['requirements']]==[(v['configuration'],v['id']) for v in required],'Accepted52-row inventory differs')
 domain={(p,s,tuple(size)) for p in ('player-default','player-software') for s in ('normal-quick','normal-storage') for size in ([640,480],[800,600])}
 require(len(partial)==8 and {(v['domain'][0],v['scenario'],tuple(v['domain'][1])) for v in partial}==domain and all(v['verified'] and len(v['witnesses'])==3 for v in partial),'Incomplete eight-case partial-write domain')
 require(len(returns)==2 and {v['configuration']['software_flag'] for v in returns}=={False,True} and len({v['identity'] for v in returns})==2 and all(v['proof']['verified'] for v in returns),'Incomplete two-profile return domain')
 rows=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id']=='operation.partial-write':
   require(row['state']=='pending','Partial write already passed');row.update(state='passed',admitted_execution_evidence=[dict(execution=v['pair'],actual_world_extents=[v['domain'][1]],provider=v['provider'],reason='Caller-bound RGB565 partial destination: nontrivial changed interior and independently matched complete destination/nonzero exterior WORD preservation in three distinct retained generations. Native current prefix/error controls supplement the unchanged component; whole-player captures remain at their recorded source phase.') for v in partial if v['domain'][0]==row['configuration']])
  if row['id']=='journey.battle-results.return-to-menu':
   require(row['state']=='pending','Results return already passed');row.update(state='passed',admitted_execution_evidence=[dict(execution=dict(record=v['record'],sha256=v['sha256']),actual_world_extents=[[800,600]],reason='Current public ordinary Quick Escape/Results/setup/Main, original callbacks/ACKs, painted/focused native Main, useful post-return input, balanced World work and normal zero-owner Quit. Forfeit return only; no natural victory/defeat or World-pixel claim.') for v in returns if v['configuration']['software_flag'] is (row['configuration']=='player-software')])
  rows.append(row)
 require(sum(v['state']=='passed' for v in rows)==56,'Closeout widened beyond four bounded leaf rows')
 return rows


def full_closeout(reader):
 from coverage_claims import fingerprint
 from rendering_acceptance import requirements
 name=PREFIX+'partial-analysis-02.json';partial=reader.json(name);require(partial['success'] and partial['retained_cases']==8 and partial['witnesses']==24 and all(reader.digest(n)==h for n,h in partial['sources'].items()),'Current partial analyzer not executed')
 native=native_controls(reader);returns=[results_case(reader,PREFIX+n) for n in ('default-results-acquisition-03.json','software-results-acquisition-01.json')]
 source='research/runtime/rendering-movie-skip-results-20261106/closeout-analysis-01.json';parent=reader.json(source)
 sources=dict(parent['sources']);sources.update({n:h for n,h in partial['sources'].items() if n not in sources})
 for v in returns:
  sources.update({n:h for n,h in v['sources'].items() if n not in sources})
 for n,h in sources.items():
  if n not in parent['sources']:require(reader.digest(n)==h,'Changed new source dependency')
 plan=reader.json('docs/native-rendering-closure.json');rows=close_rows(plan,parent,partial['cases'],returns)
 return dict(schema=1,kind='bounded_functional_ten_closeout',success=True,scope='Eight retained successful RGB565 partial-write domains plus two current public Results/setup/Main focus/normal Quit cases. Fresh component/error controls and raw original callbacks/paint/focus/retirement. Preserve52accepted historical rows, all failures and their source phases. No natural victory/defeat, generic CPU/BYTE/padding/driver, renewed whole-player/World-pixel or final decision claim.',sources=sources,
  accepted_parent=dict(record=source,sha256=reader.digest(source),whole_player_current=False),partial_input=dict(record=name,sha256=reader.digest(name)),partial_cases=partial['cases'],native_controls=native,current_returns=returns,
  source_phases=dict(inherited_and_partial_viewports='Historical original recorded native/player sources; six concurrent C++changes remain raw drift.',current_partial_component='Current production native facade/bridge/GL controls, plus unchanged-copy-component retained natural caller data.',current_public_returns='Each actual fixed-player/compiler/DLL source closure is separately current in current_returns; no replacement of historical parent hashes.'),
  requirements=rows,dossier_identity=fingerprint(requirements(plan)),closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{'operation.partial-write','journey.battle-results.return-to-menu'}),decisions=parent['decisions'],
  passed_requirements=56,pending_requirements=100,newly_passed_requirements=4,functional_cases_closed=10,retained_partial_cases=8,current_public_return_cases=2,partial_witnesses=24,preserved_words=partial['preserved_words'],current_native_control_cases=len(native['cases']),
  original_game_runs=3,successful_original_game_runs=2,negative_original_game_runs=1,zero_game_preparation_refusals=1,new_fixture=False,new_native_code=False,historical_failures_retired=False)


if __name__=='__main__':raise SystemExit(main())
