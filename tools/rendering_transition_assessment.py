"""Stopped, independently decoded witnesses for one failed and one fresh epoch."""
import importlib.util,io,json,sqlite3,struct,tempfile
from pathlib import Path
from types import SimpleNamespace
from rendering_pilot import require,counter,capture
from rendering_transition import viewport_rows,epoch_admission,assert_epoch_partition,fault_receipt,capture_path,TERMINAL_KEYS
from rendering_pause_assessment import read_samples
from rendering_pause import input_state,validate_dwell
from rendering_menu_acceptance import artifact as retained_artifact
ROOT=Path(__file__).resolve().parents[1]
class SourceBytes:
 """Feed the exact verified SQLite payload to the unchanged production reader."""
 def __init__(self,raw):self.raw=raw
 def stat(self):return SimpleNamespace(st_size=len(self.raw))
 def open(self,mode):
  require(mode=='rb','Source corpus is read-only');return io.BytesIO(self.raw)
def artifact(reader,a,path):
 path=Path(path)
 return retained_artifact(reader,a,str(path.relative_to(ROOT) if path.is_absolute() else path))

def module(name):
 spec=importlib.util.spec_from_file_location('transition_'+name,ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def provider_stopped_header(h,ring):
    # A terminal cancellation may leave an active writer, a cancelled writer,
    # or an interrupted close/deadline writer. Never admit normal END or an
    # overflow/gap/invalid failure; queued tail bytes receive no visible credit.
    return (len(h)==16 and len(ring)==1048640 and ring[:8]==b'MNMRDC03' and
        h[2]==3 and h[3]==1048640 and h[4]>0 and h[8]==1 and not any(h[10:]) and
        (h[6],h[7]) in ((1,0),(3,3),(3,4)) and ((h[5]-h[9])&0xffffffff)<=1048576)

def observed_epochs(reader,a,*,provider_loss=False):
 files=[v for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
 streams=[(v,artifact(reader,a,v['path'])) for v in files]
 views=[raw for v,raw in streams if capture(raw)[0][0]['role']=='viewport']
 require(len(views)==1,'One actual native viewport capture required');view=viewport_rows(views[0],a['session_identity'])
 commands=[(v,raw) for v,raw in streams if capture(raw)[0][0]['role']=='commands'];require(len(commands)==2,'Two actual command epochs required')
 epochs=[]
 for i,e in enumerate(a['proof']['epochs']):
  root=e['root'];ring=artifact(reader,a,str(Path(root)/'render-commands.bin'));h=struct.unpack_from('<16I',ring)
  if provider_loss and i==0:
   require(provider_stopped_header(h,ring),'Wrong provider-loss stopped epoch')
  else:require(h[2]==3 and ring[:8]==b'MNMRDC03' and h[4]>0 and h[6]==(3 if i==0 else 2) and h[7]==(5 if i==0 else 0) and h[8]==1,'Wrong stopped producer epoch')
  matches=[]
  for v,raw in commands:
   binding=json.loads(artifact(reader,a,v['path']+'.producer.json'))
   if binding['channel']==str(Path(root)/'render-commands.bin'):matches.append((v,raw,binding))
  require(len(matches)==1,'Ambiguous actual command producer');v,raw,binding=matches[0]
  epochs.append(epoch_admission(raw,binding,view,a['session_identity'],root,h[4],failed=i==0,capture_file=str(ROOT/v['path'])))
 return view,epochs,assert_epoch_partition(view,epochs)

def assess(reader,a,c,epochs,*,provider_loss=False):
 cfg=a['configuration'];size=cfg['actual_size'];proof=a['proof'];declared=proof['epochs']
 require(a['success'] and a['sources_stable'] and a['player_binary_stable'] and a['exit_code']==0 and a['original_game_runs']==2 and a['private_wine_stopped'] and a['prefix_pool']['returned'] and not a.get('forced_private_prefix_stop',False),'Incomplete public recovery execution')
 expected_fault='quiescent-native-offscreen-target-loss' if provider_loss else 'drained-Mini-producer-invalid'
 if provider_loss:require(a['actual_start_count_complete'] is True and a['charged_original_starts']==2 and cfg['render_owner']=='gui-resident','Unknown/refunded starts or changed primary provider cannot supply surface-loss acceptance')
 require(size==([800,600] if cfg['high_resolution'] else [640,480]) and type(cfg['software_flag']) is bool and ('--software-rendering' in a['command']) is cfg['software_flag'] and cfg['skip_intros'] is False and cfg['terrain_lighting']=='native' and cfg['compiled_test_driver_invoked'] is False and cfg['declared_fault']==expected_fault,'Wrong recovery domain')
 require(len(declared)==2 and declared[0]['root']!=declared[1]['root'] and proof['controlled_faults']==1 and proof['physical_restart'] is True and proof['no_compiled_test_driver'] is True and proof['failed_epoch_normal_credit'] is False,'Wrong recovery class')
 roots=[e['root'] for e in declared];rows=[];frames=[];nonces=[];callbacks=[]
 for i,e in enumerate(declared):
  r,f=read_samples(artifact(reader,a,'epoch-'+str(i)+'-samples.sqlite3'),artifact(reader,a,str(Path(e['root'])/'frame-input.bin')));rows.append(r);frames.append(f)
  resident=e['resident'];require(json.loads(artifact(reader,a,str(Path(e['root'])/'resident-world-report.json')))==resident,'Unbound resident report')
  require(f['nonce']==resident['nonce'] and all(v['input']['size']==size for v in r),'Raw original owner/source dimensions differ');nonces.append(f['nonce'])
  if cfg['software_flag']:require(resident['renderer'].lower().startswith(('llvmpipe','softpipe')),'Actual software provider missing')
  require(all(v['thread']==f['owner'] for v in a['states'] if v['epoch']==i),'Different original menu owner')
  events=module('test-menu-observer').decode(artifact(reader,a,str(Path(e['root'])/'events.bin')));require(all(v['thread']==f['owner'] for v in events),'Callback thread owner mismatch');callbacks.append([[v['menu_id'],v['argument']] for v in events if v['event']==3])
  cap=resident['stage_profile']['capture'];expected=capture_path(str(Path(e['root'])/'world-continuous.bin'),str(Path(a['temporary_root'])/'worlds'))
  require(cap['path']==expected and cap['truncated'] is False and not cap['error'] and cap['queue_bytes']==0 and cap['format']==4 and cap['codec']=='zstd-prefix-sha256-v1','Wrong/incomplete epoch source corpus')
  if i==1:require(cap['complete'] is True and cap['finished'] is True,'Fresh normal source writer did not finish')
  require(cap['accepted_worlds']==cap['written_worlds']==int(resident['completed']) and cap['accepted_source_chunks']==cap['written_source_chunks']==int(resident['source_chunks']),'Epoch source generation/chunk loss')
  source=artifact(reader,a,str(Path(expected).relative_to(ROOT)));require(source[:8]==b'MNMPROF4' and len(source)==cap['file_bytes'],'Actual epoch capture bytes differ')
  # The failure report is saved before ResidentWorldSession destruction. Its
  # complete/finished flags are preserved as reported. Actual owner retirement
  # joins the writer, and every retained body/order/hash must parse independently.
  from resident_capture import records
  source_worlds=source_chunks=0
  for generation,chunk,body,request in records(SourceBytes(source)):
   if chunk:source_chunks+=1
   else:source_worlds+=1
  require(source_worlds==cap['written_worlds'] and source_chunks==cap['written_source_chunks'],'Incomplete retained source records')
  comparison=c['epochs'][i];require(comparison['comparison']['success'] and comparison['comparison']['mismatching_pixels']==0 and comparison['original_reference']['worlds']==int(resident['completed']) and comparison['original_reference']['source_chunks']==int(resident['source_chunks']),'Uncompared source/output epoch')
  domain=comparison['world_domain'];require(domain['extents']==[size] and domain['worlds']==[dict(generation=g,width=size[0],height=size[1]) for g in range(1,len(domain['worlds'])+1)],'Mixed source extents/order')
  require(0<len(domain['worlds'])<=int(resident['completed']) and (i==0 or len(domain['worlds'])==int(resident['completed'])),'Missing fresh source import or excessive terminal prefix')
  require(not comparison['original_reference']['native_player_pixels_read'] and not comparison['original_reference']['original_destination_capture_read'],'Dependent output oracle')
 require(nonces[0]!=nonces[1],'Fresh launch reused World nonce')
 selected=[v for v in rows[0] if v['phase']=='mini-dwell-1'];dwell=validate_dwell(selected)
 require(all(v['menu']==dict(screen=17,ready=True,ack=5,thread=frames[0]['owner'],mini=[0,0,5,2,31,5,2,0]) for v in selected),'Wrong actual Campaign Mini')
 blocked=[v for v in a['actions'] if v['action']=='mini-blocked-gameplay-up-1'];require(len(blocked)==1 and selected[0]['host_ns']<blocked[0]['host_ns']<selected[-1]['host_ns'] and blocked[0]['held_ms']>=200,'No physical blocked Mini input')
 boundary=next(v for v in rows[0] if v['phase']=='mini-fault-boundary');require(boundary['menu']==selected[-1]['menu'] and boundary['host_ns']>selected[-1]['host_ns'] and boundary['original_world_return']==selected[-1]['original_world_return'] and not boundary['input']['active'] and not boundary['input']['held'] and boundary['world']['pending_world_jobs']==0,'No raw quiescent Mini fault boundary')
 if provider_loss:
  from rendering_surface_loss import retained_loss
  fault=retained_loss(reader,a,roots[0],epochs[0]['binding']['producer'])
  require(fault['provider_owner']=='gui-resident','An alternate worker cannot substitute for the actual primary provider')
 else:fault=fault_receipt(declared[0]['fault'],epochs[0]['binding']['producer'])
 require(json.loads(artifact(reader,a,'fault-injection.json'))==declared[0]['fault'] and counter(fault['begin_host_ns'])>selected[-1]['host_ns'],'Fault outside admitted Mini')
 require(counter(fault['begin_host_ns'])>boundary['host_ns'],'Fault predates final Mini boundary')
 terminal=json.loads(artifact(reader,a,str(Path(roots[0])/'native-terminal-state.json')))
 require(terminal==declared[0]['terminal'] and terminal['session']==roots[0] and set(terminal['state'])==TERMINAL_KEYS and all(v is True for v in terminal['state'].values()) and terminal['error'] and counter(terminal['host_us'])*1000>=counter(fault['begin_host_ns']),'Native terminal ownership receipt mismatch')
 if provider_loss:require(terminal['error']=='OpenGL offscreen target is unavailable' and counter(terminal['host_us'])*1000>=counter(fault['retirement_host_ns']),'Native terminal predates provider retirement')
 db=sqlite3.connect(':memory:');db.deserialize(artifact(reader,a,'terminal-samples.sqlite3'));require(db.execute('PRAGMA integrity_check').fetchone()==('ok',),'Damaged terminal SQLite samples');stored=list(db.execute('SELECT * FROM terminal ORDER BY ordinal'));db.close();require(len(stored)>=8 and stored[-1][1]-stored[0][1]>=600_000_000,'Insufficient terminal stability')
 for i,(ordinal,at,inp,native,paint) in enumerate(stored):
  state=json.loads(native);input=input_state(inp);shown=json.loads(paint)
  require(ordinal==i+1 and (i==0 or stored[i-1][1]<at) and at>=counter(terminal['host_us'])*1000 and state['session']==roots[0] and state['state']=='failed' and not input['active'] and not input['held'] and json.loads(stored[0][4])==shown,'Terminal World/input/renderer advanced or remained active')
  require(shown==declared[0]['terminal_samples'][i]['last_world_paint'],'Terminal paint witness differs')
 old_shell=json.loads(artifact(reader,a,str(Path(roots[0])/'native-only-session.json')))
 require(old_shell['state']=='restarted' and all(old_shell[k] is True for k in ('restart_requested','force_stop_verified','renderer_retired')) and declared[0]['outcome']['rendering_accepted'] is False,'Failed epoch received normal credit or was not retired')
 restart=[v for v in a['actions'] if v['action']=='physical-hard-restart'];require(len(restart)==1,'Missing unique public restart chord');key=restart[0];require(key['host_ns']>stored[-1][1] and key['host_focus']['host_focused'] and key['symbols']==[0xffe3,0xffe1,ord('r')],'Wrong restart action')
 for phase,expected in [('before',False),('pressed',True),('released',False)]:
  bitmap=bytes.fromhex(key['keymaps'][phase]);require(len(bitmap)==32 and all(bool(bitmap[k//8]&(1<<(k%8)))==expected for k in key['keycodes']),'Unacknowledged physical restart chord')
 require(epochs[0]['closing']['host_us']<=epochs[1]['opening']['host_us'] and counter(epochs[1]['opening']['host_us'])*1000>=key['host_ns'],'Fresh producer predates restart')
 identities=[v['admission']['runner_identity'] for v in declared];require(identities[0]!=identities[1],'Original runner identity reused')
 for e in declared:require(json.loads(artifact(reader,a,str(Path(e['root'])/'native-output-admission.json')))==e['admission'],'Admission changed')
 before=next(v for v in rows[1] if v['phase']=='fresh-before-gameplay-control');after=next(v for v in rows[1] if v['phase']=='fresh-after-gameplay-control')
 require(before['host_ns']<after['host_ns'] and before['input']['active']==after['input']['active']==1 and not before['input']['held'] and not after['input']['held'] and before['input']['keys'][0x26]!=after['input']['keys'][0x26] and before['original_world_return']<after['original_world_return'] and before['original_generation']<after['original_generation'],'Fresh original/input progress missing')
 require(callbacks[1][-1]==[3,4] and [17,3] in callbacks[1],'Fresh ordinary Campaign/Main Quit callbacks absent')
 # Restrict the existing cleanup reader to the fresh root, avoiding basename ambiguity.
 fresh=dict(a,session=roots[1],resident=declared[1]['resident'],artifacts=[v for v in a['artifacts'] if v['path'].startswith(str(Path(roots[1]).relative_to(ROOT))+'/')]);cleanup=module('join-rendering-normal-acceptance').cleanup_proof(reader,fresh)
 require(cleanup['verified'],'Fresh ordinary owner/resource cleanup missing')
 marks=[v for v in a['actions'] if v['action']=='qualified-mini-ingress'];require(marks,'No title-qualified ordinary Mini ingress')
 with tempfile.TemporaryDirectory(prefix='transition-ingress-') as directory:
  dest=Path(directory)
  for i,mark in enumerate(marks):
   record=json.loads(artifact(reader,a,mark['record']));require(all(v['action']=='dismiss' and v['logical_point']==[400,300] and v['button']==1 for v in record['tutorial_actions']),'Unclassified tutorial control');(dest/('storage-input-%03d.json'%i)).write_text(json.dumps(record))
   for observation in record['tutorial_observations']:
    for field in ('image','crop'):
     n=observation[field];require(Path(n).name==n,'Unsafe title image');(dest/n).write_bytes(artifact(reader,a,n))
  from campaign_storage import validate_mini_inputs
  from pause_low_ingress import validate_inputs
  ingress=validate_inputs(dest,validate_mini_inputs) if size==[640,480] else validate_mini_inputs(dest)
 require(ingress['verified'],'Unverified actual Mini ingress')
 result=dict(verified=True,domain=dict(software=cfg['software_flag'],size=size),dwell=dwell,terminal_samples=len(stored),terminal_stability_ns=stored[-1][1]-stored[0][1],distinct_world_nonces=nonces,distinct_runner_identities=identities,physical_public_restart=True,old_rendering_accepted=False,fresh_positive_input=True,cleanup=cleanup,callbacks=callbacks,ingress=ingress,terminal_owner_authority='native-terminal-state/schema1',failed_source_report_complete=declared[0]['resident']['stage_profile']['capture']['complete'],failed_summary_renderer_retired=json.loads(stored[0][3])['renderer_retired'],failed_epoch_normal_completion_claimed=False,failed_uncredited_source_worlds=int(declared[0]['resident']['completed'])-len(c['epochs'][0]['world_domain']['worlds']))

 if provider_loss:result['provider_loss']=fault
 return result
