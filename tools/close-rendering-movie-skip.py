#!/usr/bin/env python3
"""Close public enabled-intro Escape using exact live acknowledgements and return."""
import argparse,hashlib,importlib.util,io,json,struct,sys
from pathlib import Path
from PIL import Image
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require
from rendering_menu_acceptance import artifact
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'protocols/python'))
from mnm_protocols import media_v1 as wire
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-movie-skip-results-20261106/'
BID='NP.movie-skip-closeout'
LEAF='journey.enabled-movies.movie-skip'
SCOPE='Bounded enabled native intro Escape during accepted playback, exact cancellation acknowledgement, painted/focused Main and real original menu/input callbacks, then normal zero-World retirement. Both explicit profiles and original High/Low contexts; unchanged accepted player/compiler inputs. Preserve50parent rows, all failures and both decisions. No whole movie GPU/original audiovisual/driver/ABI/fault/provider or final-completion claim.'


def module(name):
 s=importlib.util.spec_from_file_location('skip_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m


def snapshot(raw,metadata=None):
 require(wire.valid_header(raw,len(raw)) and len(raw)==wire.SIZE,'Invalid media snapshot')
 def word(name):return struct.unpack_from('<I',raw,getattr(wire,name+'_OFFSET'))[0]
 fields={'request_sequence':'REQUEST_SEQUENCE','response_sequence':'RESPONSE_SEQUENCE','request':'REQUEST_ID','operation':'OPERATION','flags':'FLAGS','response':'RESPONSE_ID','status':'RESPONSE_STATUS','accepted':'ACCEPTED_ID','cancelled':'CANCELLED_REQUEST_ID','heartbeat':'HEARTBEAT'}
 result={k:word(n) for k,n in fields.items()}
 require(result['request_sequence']>0 and result['response_sequence']>0 and not result['request_sequence']%2 and not result['response_sequence']%2 and result['heartbeat']>0,'Unstable/inactive media snapshot')
 n=word('PATH_LENGTH');require(0<n<=wire.MAX_PATH_LENGTH,'Invalid movie asset path');result['asset']=raw[wire.PATH_OFFSET:wire.PATH_OFFSET+n].decode('ascii').replace('\\','/').lower()
 if metadata is not None:require(all(metadata[k]==v for k,v in result.items() if k!='asset') and hashlib.sha256(raw).hexdigest()==metadata['sha256'],'Media snapshot metadata/digest differs')
 return result


def assess(a,media,final,policy,expected,callbacks):
 require(a['success'] is True and a['sources_stable'] is True and a['original_game_runs']==1 and a['exit_code']==0
         and a['original_manifest_verified_before_after'] is True and a['private_wine_stopped'] is True and not a.get('forced_private_prefix_stop',False)
         and a['prefix_pool']['returned'] is True,'Incomplete nominal public execution')
 cfg=a['configuration'];target=cfg['selected_skip_index'];require(target==int(not cfg['high_resolution']) and cfg['skip_intros'] is False
         and ('--software-rendering' in a['command']) is cfg['software_flag'],'Wrong skip/profile/input context')
 require(all(policy.get(k) is True for k in ('native_audio_factory','native_media','native_media_startup_probe','wine_directsound_disabled')),'Legacy/silent media policy')
 if cfg['software_flag']:require(a['resident']['renderer'].lower().startswith(('llvmpipe','softpipe')),'Software provider is not observed')
 require(media[0]['request']==1 and media[0]['operation']==4 and media[0]['status']==3 and media[0]['native_pipeline_available'] is True,'Missing native media availability')
 require([v['request'] for v in media]==list(range(1,len(media)+1)) and len(media) in (2,3),'Duplicate/extra/out-of-order media request')
 skips=a['proof']['movie_skips'];require(skips and len({v['request'] for v in skips})==len(skips) and any(v['index']==target for v in skips),'Required actual Escape missing')
 cancelled={v['request'] for v in media[1:] if v['status']==4};require(cancelled=={v['request'] for v in skips},'Cancellation has no physical Escape or vice versa')
 for row in media[1:]:
  index=row['request']-2;require(index in (0,1) and row['operation']==1,'Unknown native intro')
  ref=expected['intro'+str(index)+'.avi'];name='FMV/Intro'+str(index)+'.avi';path=str(Path(a['session'])/'game'/name)
  require(row['path']==row['asset']==path and a['installation_inputs'][name]['sha256']==ref['input_sha256'],'Unbound intro/session/asset')
  require(row['accepted'] is True and row['error']=='' and row['result']==row['status'] and row['video_presentation_requested'] is True
          and row['caller_paused'] is False and row['video_frames']==row['presented_video_frames']>0
          and (row['width'],row['height'])==(ref['width'],ref['height']),'Movie was not accepted visible playback')
  require(row['audible_output_requested'] is row['audio_device_available'] is True and row['audio_frames']>0 and row['audio_bytes']==4*row['audio_frames']
          and row['initial_silent_output'] is row['waiting_for_audio_output'] is False and all(row[k]==0 for k in ('output_losses','output_recoveries','default_output_changes')),'Media output loss/silence is not a nominal skip')
  if row['status']==4:require(0<row['video_frames']<ref['frames'],'Skip did not interrupt playback')
  else:require(target==1 and index==0 and row['status']==3 and row['video_frames']==ref['frames'] and row['video_rgba_sha256']==ref['rgba_sha256'],'Unexpected EOF or incomplete first intro')
 for skip in skips:
  request=skip['request'];before,after=skip['decoded_before'],skip['decoded_after'];physical=skip['physical'];key=physical['physical']
  require(skip['index']==request-2 and before['request']==before['response']==before['accepted']==request and before['operation']==1
          and before['status']==2 and before['flags']==before['cancelled']==0 and before['asset'].endswith('/intro'+str(skip['index'])+'.avi'),'Escape did not target accepted visible intro')
  require(key['symbol']==0xff1b and key['down_up'] is True and key['host_focus']['host_focused'] is True
          and skip['before']['host_ns']<physical['host_ns']<skip['after']['host_ns'],'Missing ordered physical Escape/focus')
  require(after['cancelled']==0 and after['heartbeat']>=before['heartbeat'] and after['request']>=request
          and (after['request']>request or after['response']==after['accepted']==request and after['status']==4
               and after['response_sequence']>before['response_sequence']),'Escape lacks live cancellation acknowledgement')
 require(final['request']==final['response']==final['accepted']==media[-1]['request'] and final['status']==media[-1]['status']==4
         and final['flags']==final['cancelled']==0,'Final accepted movie did not retire as cancelled')
 states=a['states'];require(len(states)==5 and [(v['screen'],v['ack']) for v in states]==[(3,0),(10,1),(3,2),(22,3),(3,4)]
         and len({v['thread'] for v in states})==1 and states[0]['thread']>0 and all(v['ready'] is True and v['status'] in (0,1) and v['handoff']==0 for v in states)
         and all(b['generation']>x['generation'] for x,b in zip(states,states[1:])),'Wrong original return/input callbacks or owner')
 require(callbacks==[[3,3],[10,1],[3,2],[22,3],[3,4]],'Original menu effects differ')
 require(states[1]['preferences']==a['proof']['preferences'] and states[1]['preferences'][2]==int(not cfg['high_resolution']),'Original High/Low context differs')
 graphics=a['retirement']['graphics_retirement'];world=a['resident']
 require(a['retirement']['state']=='completed' and a['retirement']['forwarded']==a['retirement']['worlds']==0 and graphics['resources_released'] is graphics['process_exit'] is True
         and not graphics['surviving_owners'] and all(v==0 for v in graphics['counts'].values())
         and all(int(world[k])==0 for k in ('completed','imports','verified_returns')) and world['pending'] is False and world['normal_quit_retired'] is True,'Movie/Main/World/graphics owners survived normal Quit')
 return dict(verified=True,cancelled_requests=sorted(cancelled),selected_skip_index=target,decoded_frames=sum(v['video_frames'] for v in media[1:]),
             original_callbacks=callbacks,original_thread=states[0]['thread'],normal_quit=True,unintended_worlds=0,movie_gpu_output_proved=False,scope=SCOPE)


def validate_artifacts(reader,a,expected):
 media=json.loads(artifact(reader,a,'native-media-report.json'));policy=json.loads(artifact(reader,a,'native-audio-launch-policy.json'))
 for skip in a['proof']['movie_skips']:
  for label in ('before','after'):skip['decoded_'+label]=snapshot(artifact(reader,a,skip[label]['path']),skip[label])
 for state in a['states']:
  raw=artifact(reader,a,state['wire_path']);w=struct.unpack_from('<40I',raw)
  require(raw[:8]==(('MNMMCMD' if w[2]<10 else 'MNMMCM')+str(w[2])).encode() and 2<=w[2]<=22 and len(raw)==w[3]
          and list(w[32:40])==[int(state[k]) for k in ('sequence','generation','screen','ready','ack','status','thread','handoff')],'Original menu snapshot changed')
  if state['screen']==10:require(list(struct.unpack_from('<7i',raw,44724))==state['preferences'],'Original Preferences payload differs')
 shots={v['label']:v for v in a['screenshots']};shot=shots['main-ready-initial'];require(shot['state_index']==0 and shot['host_focus']['host_focused'] is True,'Main return focus missing')
 nodes=[v for v in shot['nodes'] if v['role']=='button' and v['name']=='Quick Battle'];require(len(nodes)==1 and all(nodes[0][k] is True for k in ('focused','enabled','showing')),'Ready native Main control absent')
 for capture in shots.values():
  raw=artifact(reader,a,capture['path']);require(hashlib.sha256(raw).hexdigest()==capture['sha256'],'Unbound screenshot');image=Image.open(io.BytesIO(raw));require(image.size==(1280,1024),'Unexpected display capture');image.verify()
 x,y,w,h=nodes[0]['rect'];image=Image.open(io.BytesIO(artifact(reader,a,shot['path']))).convert('RGB');pixels=list(image.crop((x,y,x+w,y+h)).getdata())
 require(min(w,h)>0 and x>=0 and y>=0 and x+w<=1280 and y+h<=1024 and sum(max(v)>20 for v in pixels)>=len(pixels)*.9 and sum(min(v)>120 for v in pixels)>=100,'Main return is not visibly painted')
 require(all(any(v['sha256']==skip['screenshot'] and v['label']=='movie-before-'+str(skip['request']) for v in shots.values()) for skip in a['proof']['movie_skips']),'Skip lacks its bound pre-Escape capture')
 for name in ('main-focused-space','preferences-cancel','quick-cancel'):
  found=[v for v in a['actions'] if v.get('action')==name];require(len(found)==1 and found[0]['physical']['down_up'] is True and found[0]['physical']['host_focus']['host_focused'] is True,'Post-skip physical input missing')
 preferences=[v for v in a['actions'] if v.get('action')=='click' and v.get('name')=='Preferences'];require(len(preferences)==1 and preferences[0]['host_ns']>shot['host_ns'],'Initial Main did not accept real input after return')
 decoder=module('test-menu-observer');callbacks=[[v['menu_id'],v['argument']] for v in decoder.decode(artifact(reader,a,'events.bin')) if v['event']==3]
 proof=assess(a,media,snapshot(artifact(reader,a,'native-media.bin')),policy,expected,callbacks);proof['painted_initial_main']=dict(sha256=shot['sha256'],target=nodes[0]);return proof


def close_rows(plan,parent,runs):
 validate(plan);template=requirements(plan);require(parent['passed_requirements']==50 and parent['dossier_identity']==fingerprint(template)
     and [(v['configuration'],v['id']) for v in parent['requirements']]==[(v['configuration'],v['id']) for v in template],'Accepted50-row parent differs')
 require(len(runs)==4 and {(v['configuration']['software_flag'],v['configuration']['high_resolution']) for v in runs}=={(s,h) for s in (False,True) for h in (False,True)}
         and len({v['identity'] for v in runs})==4 and all(v['proof']['verified'] for v in runs),'Incomplete/duplicate public skip domain')
 rows=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id']==LEAF:
   require(row['state']=='pending','Skip row already passed');row.update(state='passed',admitted_execution_evidence=[dict(execution=v['case'],reason=SCOPE,native_ui_layout=[800,600],original_resolution_selector=int(not v['configuration']['high_resolution'])) for v in runs if v['configuration']['software_flag'] is (row['configuration']=='player-software')])
  rows.append(row)
 require(sum(v['state']=='passed' for v in rows)==52,'Movie-skip scope widened');return rows


def analyze(reader):
 parent,impact=module('close-rendering-world-exit').parent_plan(ROOT,reader,'CP06.opaque-cropped-closed01','NP.opaque-cropped-copy-closeout')
 oldref=parent['inputs']['closeout-analysis-01.json'];require(reader.digest(oldref['record'])==oldref['sha256'],'Parent input changed');old=reader.json(oldref['record'])
 expected,reference=module('close-rendering-movie-return').references(reader);selection=reader.json(PREFIX+'case-selection-01.json');require(selection['success'],'No complete positive skip selection')
 sources=dict(parent['sources']);runs=[]
 for case in selection['cases']:
  require(reader.digest(case['record'])==case['sha256'],'Case changed');a=reader.json(case['record'])
  require(all(reader.digest(n)==h for n,h in (a['sources']|a['compiler_sources']).items()),'Changed producer/native compiler source')
  proof=validate_artifacts(reader,a,expected);sources.update(a['sources']);sources.update(a['compiler_sources']);runs.append(dict(case=case,identity=a['session'],configuration=a['configuration'],proof=proof))
 b=next(v for v in json.loads((ROOT/'research/runtime/coverage/register.json').read_text())['behaviors'] if v['id']==BID)
 sources.update({n:reader.digest(n) for n in b['implementation']+b['tests']+b['validation_dependencies']});require(all(reader.digest(n)==h for n,h in sources.items()),'Changed source closure')
 return dict(schema=1,kind='native_public_movie_skip_closeout',success=True,scope=SCOPE,sources=sources,accepted_parent=dict(binding=impact['retained_input'],applicability=impact),input_parent=oldref,
             case_selection=dict(record=PREFIX+'case-selection-01.json',sha256=reader.digest(PREFIX+'case-selection-01.json')),expected_movies=expected,reference=reference,executions=runs,
             requirements=close_rows(reader.json('docs/native-rendering-closure.json'),old,runs),dossier_identity=old['dossier_identity'],closed_leaf_ids=sorted(set(old['closed_leaf_ids'])|{LEAF}),decisions=old['decisions'],
             passed_requirements=52,pending_requirements=104,inherited_current_rows=50,newly_passed_requirements=2,original_game_runs=4,worlds_started=0,new_game_fixture=False,new_native_code=False,
             skipped_requests=sum(len(v['proof']['cancelled_requests']) for v in runs),movie_gpu_output_proved=False,historical_failures_retired=False)


def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh immutable report required')
 q=dict(schema=1,kind='native_public_movie_skip_closeout',success=False)
 try:
  with Reader(ROOT) as r:q=analyze(r)
 except Exception as e:q['error']=str(e)
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h)),flush=True);return 0 if q['success'] else 1


if __name__=='__main__':raise SystemExit(main())
