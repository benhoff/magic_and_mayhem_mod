"""Stopped raw-wire joins for the bounded Campaign Mini pause contract."""
import hashlib
import importlib.util
from pathlib import Path
import sqlite3
import struct
import json
import tempfile
from rendering_pilot import require
from rendering_menu_acceptance import artifact
from rendering_pause import input_state,media_state,music_state,world_state,validate_dwell
from rendering_interaction import decode
from pause_focus import key_down
ROOT=Path(__file__).resolve().parents[1]
def module(name):
 s=importlib.util.spec_from_file_location('pause_'+name,ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def read_samples(raw,frame_raw):
 db=sqlite3.connect(':memory:');db.deserialize(raw)
 require(db.execute('PRAGMA integrity_check').fetchone()==('ok',),'Damaged pause SQLite samples')
 stored=list(db.execute('SELECT * FROM samples ORDER BY ordinal'));db.close()
 frames=decode(frame_raw);rows=[]
 for ordinal,phase,host_ns,menu,inp,med,mus,world,frame,size in stored:
  require(ordinal==len(rows)+1 and len(frame)==384 and size==35869312,'Sample order/World size differs')
  w=struct.unpack('<96I',frame);mw=struct.unpack_from('<40I',menu)
  require(0<w[0]<=len(frames['rows']) and frame_raw[16+(w[0]-1)*384:16+w[0]*384]==frame
   and w[2]==1 and w[7]&256 and mw[32]>0 and not mw[32]&1 and mw[35] in (0,1)
   and mw[38]==w[5]==frames['owner'],'Unbound raw original return/menu owner')
  rows.append(dict(sample=ordinal,phase=phase,host_ns=host_ns,input=input_state(inp),media=media_state(med),music=music_state(mus),world=world_state(world),
   original_world_return=w[3],original_generation=w[59]|w[60]<<32,menu=dict(screen=mw[34],ready=bool(mw[35]),ack=mw[36],thread=mw[38],mini=list(struct.unpack_from('<8I',menu,42000)))))
 require(rows and all(b['host_ns']>a['host_ns'] for a,b in zip(rows,rows[1:])),'Empty/reordered raw sample history')
 return rows,frames

def assess(reader,a,c,observed):
 cfg=a['configuration'];size=cfg['actual_size']
 require(cfg.get('command_capture_policy')=='command-history-lossless-v1','Undeclared command capture policy')
 command_open=[]
 for entry in a['artifacts']:
  if '/capture/' not in entry['path'] or not entry['path'].endswith('.bin'):continue
  raw=artifact(reader,a,Path(entry['path']).name);m,n=struct.unpack_from('<II',raw,8);meta=json.loads(raw[16:16+m])
  if meta['role']=='commands':command_open.append(meta)
 require(len(command_open)==1 and command_open[0]['version']==4 and command_open[0]['policy']=='command-history-lossless-v1','Actual command capture is not version4')
 require(size==([800,600] if cfg['high_resolution'] else [640,480]) and type(cfg['software_flag']) is bool
  and ('--software-rendering' in a['command']) is cfg['software_flag'] and cfg['viewport_scaling']=='sharp'
  and cfg['skip_intros'] is False and cfg['compiled_test_driver_invoked'] is False and cfg['terrain_lighting']=='native','Wrong pause domain')
 require(a['success'] and a['original_manifest_verified_before_after'] and a['original_game_runs']==1 and a['private_wine_stopped']
  and a['prefix_pool']['returned'] and not a.get('forced_private_prefix_stop',False),'Incomplete nominal pause acquisition')
 require(c['comparison']['success'] and c['comparison']['mismatching_pixels']==0 and c['original_reference']['continuous_from_owned_initial_history']
  and not c['original_reference']['native_player_pixels_read'] and not c['original_reference']['original_destination_capture_read']
  and hashlib.sha256(observed['commands']).hexdigest()==c['captured_command_stream_sha256'],'Uncompared/dependent pause output')
 rows,frames=read_samples(artifact(reader,a,'pause-samples.sqlite3'),artifact(reader,a,'frame-input.bin'))
 require(frames['nonce']==a['resident']['nonce'] and all(s['thread']==frames['owner'] for s in a['states']),'Pause witnesses have different original owner/session')
 dwells=[];actions=a['actions'];declared=a['proof']['dwells'];require(len(declared)==2,'Missing two real Mini dwells')
 worlds=c['world_domain']['worlds'];require(worlds and len(worlds)==c['original_reference']['worlds'] and all(v==dict(generation=i,width=size[0],height=size[1]) for i,v in enumerate(worlds,1)),'Mixed source generation/extents')
 shown=[(observed['paints'][p][0],m) for p,m in zip([p for p in observed['swaps'] if int(observed['paints'][p][0]['world'])>0],[m for m in c['comparison']['comparisons'] if m['world']>0])]
 require(shown,'No actually presented compared Worlds')
 for number,d in enumerate(declared,1):
  selected=[r for r in rows if r['phase']=='mini-dwell-'+str(number)];proof=validate_dwell(selected)
  require(d['number']==number and d['first_sample']==selected[0]['sample'] and d['last_sample']==selected[-1]['sample']
   and all(v['menu']==dict(screen=17,ready=True,ack=4+number,thread=frames['owner'],mini=[0,0,5,2,31,5,2,0]) for v in selected),'Original Campaign Mini admission changed')
  before=next(v for v in rows if v['phase']=='world-before-mini-'+str(number))
  resumed=next(v for v in rows if v['phase']=='resumed-'+str(number));control=next(v for v in rows if v['phase']=='resumed-input-control-'+str(number))
  require(before['sample']<selected[0]['sample']<selected[-1]['sample']<resumed['sample']<control['sample']
   and before['input']['active']==1 and not before['input']['held'] and resumed['input']['active']==control['input']['active']==1
   and not resumed['input']['held'] and not control['input']['held']
   and control['input']['keys'][0x26]!=resumed['input']['keys'][0x26]
   and resumed['original_world_return']>selected[-1]['original_world_return']
   and control['original_world_return']>resumed['original_world_return']
   and resumed['original_generation']>selected[-1]['original_generation']
   and resumed['menu']['ack']==5+number,'Missing acknowledged World/input resume or positive key control')
  blocked=[v for v in actions if v['action']=='mini-blocked-gameplay-up-'+str(number)]
  require(len(blocked)==1 and selected[0]['host_ns']<blocked[0]['host_ns']<selected[-1]['host_ns'] and blocked[0]['symbol']==0xff52 and blocked[0]['held_ms']>=200,'No actual blocked gameplay key within Mini dwell')
  post=[(p,m) for p,m in shown if resumed['original_generation']<=int(p['world'])<=control['original_generation'] and resumed['host_ns']//1000-500000<=int(p['host_us'])<=control['host_ns']//1000+500000]
  require(post and all(m['mismatching_pixels']==0 and p['native_gpu_frame'] and p['gpu_read_complete'] and p['pixels_captured'] and p['presentation_owner']=='gui' and [p['frame_width'],p['frame_height']]==size for p,m in post),'Missing complete resumed native GPU output')
  proof.update(number=number,resume_ack=resumed['menu']['ack'],resumed_generation=resumed['original_generation'],control_generation=control['original_generation'],compared_resumed_paints=len(post));dwells.append(proof)
 events=module('test-menu-observer').decode(artifact(reader,a,'events.bin'));callbacks=[[r['menu_id'],r['argument']] for r in events if r['event']==3]
 require(callbacks.count([17,4])==2 and callbacks[-1]==[3,4] and all(v['thread']==frames['owner'] for v in events),'Missing original Mini Cancel callbacks or normal Quit')
 require(len([v for v in actions if v.get('object')=='miniMenuButton5'])==1 and len([v for v in actions if v['action']=='mini-escape-cancel'])==1,'Both public resume modalities required')
 focused=[v for v in actions if v['action']=='focus-mini-cancel'];escaped=[v for v in actions if v['action']=='mini-escape-cancel']
 require(len(focused)==len(escaped)==1,'Missing unique admitted Mini Escape')
 focus=focused[0]['focus_admission'];physical=escaped[0]['physical']
 require(focus['accessible_id'].endswith('.miniMenuButton5') and focus['focused'] and focus['enabled'] and focus['showing'] and focus['host_focus']['host_focused']
  and focus['confirmed_host_ns']<=focused[0]['host_ns']<=escaped[0]['host_ns'] and escaped[0]['host_ns']-focus['confirmed_host_ns']<1_000_000_000
  and physical['before_down'] is False and physical['pressed'] is True and physical['released'] is True and physical['host_focus']['host_focused']
  and [key_down(bytes.fromhex(physical['keymaps'][p]),physical['keycode']) for p in ('before','pressed','released')]==[False,True,False],'Unacknowledged physical Escape focus/key admission')
 acked=[v for v in rows if v['phase']=='escape-ack-2'];resumed=next(v for v in rows if v['phase']=='resumed-2')
 require(len(acked)==1 and escaped[0]['host_ns']<acked[0]['host_ns']<resumed['host_ns'] and acked[0]['menu']['ack']==7,'Missing independently sampled original Escape Cancel ACK')
 ingress_marks=[v for v in actions if v['action']=='qualified-mini-ingress']
 require(ingress_marks,'Missing qualified physical Mini ingress')
 with tempfile.TemporaryDirectory(prefix='pause-ingress-',dir='/tmp') as directory:
  dest=Path(directory)
  for i,mark in enumerate(ingress_marks):
   raw=artifact(reader,a,mark['record']);record=json.loads(raw)
   require(all(v['action']=='dismiss' and v['logical_point']==[400,300] and v['button']==1 for v in record['tutorial_actions']),'Unclassified gameplay tutorial input')
   (dest/('storage-input-%03d.json'%i)).write_bytes(raw)
   for o in record['tutorial_observations']:
    for kind in ('image','crop'):
     name=o[kind];require(Path(name).name==name,'Unsafe title image')
     (dest/name).write_bytes(artifact(reader,a,name))
  from campaign_storage import validate_mini_inputs
  ingress=validate_mini_inputs(dest)
 require(ingress['verified'] and len(ingress['records'])==len(ingress_marks),'Unverified original Mini input controls')
 cleanup=module('join-rendering-normal-acceptance').cleanup_proof(reader,a)
 require(cleanup['verified'],'Normal source/resource retirement missing')
 return dict(verified=True,dwells=dwells,pause_intervals=2,compared_worlds=len(worlds),compared_presentations=len(shown),cleanup=cleanup,callbacks=callbacks,
  all_simulation_animation_pause_claimed=False,music_playback_frozen_claimed=False,ingress=ingress,domain=dict(software=cfg['software_flag'],size=size))
