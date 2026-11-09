"""Strict passive native Qt SFT registration/visible-caption/retirement trace."""
import hashlib,json,re
from collections import Counter
from rendering_pilot import require
PREFIX='MNM_QTFONT01 '
SOURCES=('Sprites/heading text 800.sft','Sprites/body text 800.sft','Sprites/Tooltip Text.sft','Sprites/yellowtext.sft')
def number(value,low=0,high=2**32-1):
 require(type(value)is int and low<=value<=high,'Native font integer domain');return value
def identity(value):
 require(type(value)is str and value.isdecimal()and str(int(value))==value and 0<int(value)<=2**53,'Native font identity');return int(value)
def digest(value):require(type(value)is str and re.fullmatch('[0-9a-f]{64}',value),'Native font content digest');return value

def decode(log,*,allow_fallback=False,complete=True):
 if isinstance(log,bytes):log=log.decode('utf-8',errors='replace')
 last_ns=0
 events=[];generations={};owners={};active={};pid=None;paints=[];fallbacks=0;reuse=0;peak=0
 for line in log.splitlines():
  if not line.startswith(PREFIX):continue
  require(len(line)<=16384,'Native font event bound');row=json.loads(line[len(PREFIX):]);require(isinstance(row,dict)and row.get('schema')==1 and identity(row.get('sequence'))==len(events)+1<=4096,'Native font event schema/order')
  require(row.get('gui_thread')is True,'Native font event off GUI thread');observed=number(row.get('pid'),1);require(pid is None or pid==observed,'Native font process changed');pid=observed
  host_ns=identity(row.get('host_ns'));require(host_ns>=last_ns,'Native font event clock reversed');last_ns=host_ns
  kind=row.get('kind');require(kind in {'fallback','create','register','attach','paint','reuse','detach','unregister','destroy'},'Native font overflow/unknown event')
  if kind=='fallback':
   require(allow_fallback,'Installed native font catalog fell back');fallbacks+=1
  else:
   raw_generation=row.get('generation');require(type(raw_generation)is str and raw_generation.isdecimal()and str(int(raw_generation))==raw_generation,'Native font generation type');generation=number(int(raw_generation),0,2**53)
   if kind=='create':
    require(generation==len(generations)+1 and generation not in generations and type(row.get('root'))is str and row['root'],'Native font generation identity/root');digest(row.get('key_sha256'))
    generations[generation]=dict(id=generation,root=row['root'],key_sha256=row['key_sha256'],registrations={},owners=set(),destroyed=False)
   elif kind=='attach':
    owner=identity(row.get('owner'));require(owner not in owners and(generation in generations or allow_fallback and generation==0),'Native font owner identity')
    if generation:require(not generations[generation]['destroyed']and len(generations[generation]['registrations'])==4,'Native font owner before complete live catalog');generations[generation]['owners'].add(owner)
    owners[owner]=dict(generation=generation,live=True,widget=row.get('widget'),kind=row.get('class'))
   elif kind=='detach':
    owner=identity(row.get('owner'));require(owner in owners and owners[owner]['live']and owners[owner]['generation']==generation,'Native font unmatched/double owner drop');owners[owner]['live']=False
    if generation:generations[generation]['owners'].remove(owner)
   else:
    require(generation in generations and not generations[generation]['destroyed'],'Native font missing/retired generation');g=generations[generation]
    if kind=='register':
     role=number(row.get('role'),0,3);font_id=number(row.get('id'));require(role not in g['registrations']and font_id not in active and row.get('source')==SOURCES[role]and type(row.get('family'))is str and row['family'].startswith('MNM'),'Native font registration/source/ID differs');digest(row.get('ttf_sha256'))
     g['registrations'][role]=dict(id=font_id,family=row['family'],source=row['source'],ttf_sha256=row['ttf_sha256'],live=True);active[font_id]=(generation,role);peak=max(peak,len(active))
     if len(g['registrations'])==4:require(hashlib.sha256(b''.join(bytes.fromhex(g['registrations'][i]['ttf_sha256'])for i in range(4))).hexdigest()==g['key_sha256'],'Native font catalog content key differs')
    elif kind=='reuse':require(row.get('root')==g['root']and len(g['registrations'])==4,'Native font reused foreign/incomplete catalog');reuse+=1
    elif kind=='paint':
     owner=identity(row.get('owner'));role=number(row.get('role'),0,3);require(owner in owners and owners[owner]['live']and owners[owner]['generation']==generation and role in g['registrations'],'Native caption lacks live font owner');reg=g['registrations'][role]
     require(reg['live']and row.get('family')==row.get('resolved_family')==reg['family']and row.get('font_valid')is True and row.get('truncated')is False,'Native caption selected wrong or invalid font')
     text=row.get('text');require(type(text)is str and 0<len(text)<=256 and type(row.get('widget'))is str and type(row.get('class'))is str,'Native caption identity');number(row.get('font_size'),1,1024)
     glyphs=row.get('glyphs');require(isinstance(glyphs,list)and len(glyphs)==len(text.encode('utf-16-le'))//2 and all(type(n)is int and 0<=n<=65535 for n in glyphs),'Native caption glyph map extent')
     rect=row.get('rect');require(type(rect)is list and len(rect)==4 and all(type(n)is int and -16384<=n<=16384 for n in rect)and min(rect[2:])>0,'Native caption visible paint geometry');paints.append(row)
    elif kind=='unregister':
     font_id=number(row.get('id'));require(active.get(font_id,(None,None))[0]==generation and not g['owners']and row.get('success')is True,'Native font unregister while owned or failed');role=active.pop(font_id)[1];g['registrations'][role]['live']=False
    else:
     require(row.get('app_alive')is True and not g['owners']and len(g['registrations'])==4 and not any(v['live']for v in g['registrations'].values()),'Native font generation not retired');g['destroyed']=True
  events.append(row)
 if complete:require(events and generations and not active and not any(v['live']for v in owners.values())and all(g['destroyed']for g in generations.values()),'Native font incomplete registration/owner retirement')
 return dict(events=events,pid=pid,generations=generations,owners=owners,paints=paints,fallbacks=fallbacks,reuse=reuse,peak_registered_fonts=peak,complete=not active and not any(v['live']for v in owners.values())and bool(generations)and all(g['destroyed']for g in generations.values()),closes_acceptance=False)

def installed(acquired,raw_log):
 trace=decode(raw_log);require(trace['pid']==acquired['player_pid'],'Native font trace is not the tested player')
 expected=acquired['session']+'/game';require(all(g['root']==expected for g in trace['generations'].values()),'Native font catalog escaped the actual staged assets')
 require(trace['peak_registered_fonts']==4 and trace['reuse']>0,'Native font registration conservation/reuse missing')
 keys={g['key_sha256']for g in trace['generations'].values()};require(len(keys)==1,'Immutable installed native font catalog changed across reload')
 snapshots=[]
 for screenshot in acquired['screenshots']:
  if not screenshot['label'].startswith(('font-main','font-quick','font-original-spellbox')):continue
  lower=max([acquired.get('player_started_host_ns',0)]+[s['host_ns']for s in acquired['screenshots']if s['host_ns']<screenshot['host_ns']])
  nodes={n.get('accessible_id','').rsplit('.',1)[-1]:n for n in screenshot['nodes']if n.get('accessible_id')}
  for event in trace['paints']:
   node=nodes.get(event['widget']);text=event['text'].replace('&','')
   if(lower<int(event['host_ns'])<=screenshot['host_ns'] and screenshot['host_ns']-int(event['host_ns'])<30_000_000_000 and node and node.get('name')==text and node.get('rect')==event['rect']and event['glyphs']and all(event['glyphs'])):
    snapshots.append(dict(snapshot=screenshot['label'],snapshot_sha256=screenshot['sha256'],widget=event['widget'],text=text,role=event['role'],generation=event['generation'],owner=event['owner'],family=event['family'],rect=event['rect'],paint_sequence=event['sequence'],paint_host_ns=event['host_ns'],snapshot_host_ns=screenshot['host_ns']))
 require(snapshots and all(any(v['snapshot']==label and v['role']==role for v in snapshots)for label,role in [('font-main-initial',0),('font-main-restored',0),('font-quick-1',1),('font-quick-2',1),('font-original-spellbox-1',2),('font-original-spellbox-2',2)]),'Actual native SFT caption paints do not join both heading/body screenshots')
 return dict(verified=True,pid=trace['pid'],generation_count=len(trace['generations']),peak_registered_fonts=4,retired_registrations=4*len(trace['generations']),cache_reuses=trace['reuse'],owned_widgets=len(trace['owners']),visible_caption_paints=len(trace['paints']),caption_snapshot_joins=snapshots,source_catalogs=[dict(id=g['id'],key_sha256=g['key_sha256'],registrations=g['registrations'])for g in trace['generations'].values()],normal_retirement=trace['complete'],original_ui_pixel_equivalence=False,original_world_sft_text_claimed=False)
