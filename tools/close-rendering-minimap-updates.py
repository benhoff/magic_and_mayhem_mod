#!/usr/bin/env python3
"""Bind changing retained minimap inputs to independently expected visible output."""
import argparse,hashlib,importlib.util,json,struct,tempfile
from pathlib import Path
import numpy as np
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_menu_acceptance import artifact
from rendering_pilot import History,acquisition,commands,require
from resident_capture import records
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-next-two-results-20261107/'
BID='NP.minimap-update-closeout'
LEAF='journey.hud-minimap-tooltips.minimap-update'
def module(name):
 s=importlib.util.spec_from_file_location('map_update_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
copy=module('close-rendering-opaque-cropped-copy');blend=copy.base;base=blend.base
sha=base.sha

def source_context(raw):
 at=64;maps=[];entry=None;copies=[]
 while at<len(raw):
  require(at+96<=len(raw),'Truncated source operation');f=struct.unpack_from('<24I',raw,at);require(f[0]>=96 and at+f[0]<=len(raw),'Source operation extent')
  body=raw[at+96:at+f[0]]
  if f[2] in (22,24):maps.append((f,body))
  if f[2]==6:copies.append(f)
  if f[2]==11:require(entry is None,'Duplicate source queue');entry=f
  at+=f[0]
 if not maps:return None
 terrain=[(f,b) for f,b in maps if f[2]==22];overlays=[(f,struct.unpack_from('<37I',b),b[148:]) for f,b in maps if f[2]==24]
 require(len(terrain)==1 and 3<=len(overlays)<=66 and [m[0] for f,m,b in overlays]==[2]+[0]*(len(overlays)-2)+[1] and entry is not None,'Missing/reordered minimap inputs')
 t,body=terrain[0];camera,creatures=overlays[0],overlays[-1];cells=overlays[1:-1]
 ctx=dict(terrain_fields=list(t),terrain_payload_sha256=sha(body),camera=dict(fields=list(camera[0]),metadata=list(camera[1])),cells=[dict(fields=list(v[0]),metadata=list(v[1])) for v in cells],captured_cell_calls=len(cells),creatures=dict(fields=list(creatures[0]),metadata=list(creatures[1]),points=[list(struct.unpack_from('<3I',creatures[2],i)) for i in range(0,len(creatures[2]),12)]),entry=list(entry))
 display=[f for f in copies if f[4]==t[3] and creatures[0][1]<f[1]<entry[1] and f[5:7]==t[5:7] and f[8:14]==(0,0,0,0,t[5],t[6])]
 require(len(display)==1,'Missing unique pre-scene minimap display copy');ctx['display_copy']=list(display[0])
 validate_context(ctx)
 normalized=[]
 for f,m,b in overlays:
  m=list(m);m[12]=int(bool(m[12] and m[16])) if m[0]==0 else 0
  normalized.append(dict(metadata=m,points=b.hex()))
 ctx['render_input_sha256']=fingerprint(dict(terrain=list(t[3:18]),terrain_payload=sha(body),overlays=normalized))
 return ctx,body

def validate_context(c):
 t=c['terrain_fields'];d=c['display_copy'];require(d[4]==t[3] and d[5:7]==t[5:7] and d[8:14]==[0,0,0,0,t[5],t[6]] and c['creatures']['fields'][1]<d[1]<c['entry'][1],'Missing/reordered pre-scene display copy');camera,creatures=c['camera'],c['creatures'];cells=c['cells'];m=camera['metadata'];entry=c['entry'];overlays=[camera,*cells,creatures]
 require(1<=len(cells)==c['captured_cell_calls']<=64,'Missing captured cell-marker call')
 require(t[16]==m[9]==0 and camera['fields'][17]==0,'Only actual view-zero update contexts are admitted')
 require([t[1]]+[v['fields'][1] for v in overlays]+[entry[1]]==sorted({t[1],entry[1],*[v['fields'][1] for v in overlays]}),'Wrong original minimap source order')
 require(t[3]==entry[3] and all(v['fields'][3]==t[3] and v['metadata'][1:5]==m[1:5] and v['metadata'][7:12]==m[7:12] for v in overlays),'Mixed map/camera/surface context')
 require(t[14:16]==m[1:3] and t[10:12]==m[3:5] and t[8:10]==m[7:9] and t[7]==t[5] and t[19]==0xfffffffe and t[17] in (0,1) and 0<t[14]<=256 and t[14]%2==0 and 0<t[15]<=256,'Unsupported terrain geometry/history')
 require(m[10]==0 and all(v['metadata'][9]==0 and v['metadata'][10]==0 and len(v['metadata'])==37 and v['fields'][5:8]==t[5:8] for v in overlays),'Mixed minimap format or layout')
 require(camera['fields'][19:21]==m[7:9] and creatures['fields'][19:21]==m[1:3] and creatures['metadata'][17]==len(creatures['points'])>0,'Wrong finite camera/creature result')
 require(all(len(p)==3 and 0<=p[0]<m[1] and 0<=p[1]<m[2] and p[2] in (0,1) for p in creatures['points']),'Invalid owned creature inputs')
 for cell in cells:
  cellm=cell['metadata'];require(cellm[15]<9 and cellm[16] in (0,1) and cellm[12] in (0,1),'Invalid cell selection/flash')
  u=(base.signed(cellm[13])+m[1]//2-m[3])%m[1];v=(base.signed(cellm[14])+m[2]//2-m[4])%m[2];ring=bool(cellm[12] and cellm[16]);require(cell['fields'][21]==(m[8]+(u+v)//2+(2 if ring else 1))*t[7]+m[7]+u-v+(2 if ring else 0),'Wrong selected cell return offset')

def minimap_writes(c,body):
 """Source-derived write coordinates; every directly determined WORD is independently computed."""
 validate_context(c);t=c['terrain_fields'];width,height=t[5:7];gw,gh=t[14:16];cx,cy=t[10:12];rx,ry=t[8:10]
 require(len(body)==gw*gh*3 and sha(body)==c['terrain_payload_sha256'],'Terrain source bytes differ')
 data=np.frombuffer(body,dtype='u1').reshape(gh,gw,3);footprint=set();known={}
 def put(x,y,word):
  require(0<=x<width and 0<=y<height,'Minimap write outside owned World');at=y*width+x;footprint.add(at)
  if word is None:known.pop(at,None)
  else:known[at]=int(word)
 sx,sy=(cx-gw//2)%gw,(cy-gh//2)%gh
 for row in range(gh):
  for col in range(gw):
   x,y=rx-2*(row//2)+col,ry+(row+1)//2+col//2;a=data[(sy+row)%gh,(sx+col)%gw];require(a[2] in (0,1),'Invalid hidden terrain input');put(x,y,None if a[2] else int(a[0])|(int(a[1])<<8))
 camera=c['camera']['metadata'];a,b=camera[5]//2,camera[6]//2;sw,sh=camera[18]//128+1,camera[19]//64
 for i in range(4):
  dx,dy=base.signed(camera[20+2*i]),base.signed(camera[21+2*i]);require(dx in (-1,1) and dy in (-1,1),'Invalid actual camera directions');x=(a+b)//2-dx*sw*2-b;y=(a+b)//2-1-dy*sh+int(dy<0)
  for n in range(4):put(rx+x+n*dx,ry+y,0x400);put(rx+x,ry+y+n*dy,0x400)
 def project(meta,x,y):
  u=(x+gw//2-meta[3])%gw;v=(y+gh//2-meta[4])%gh;return meta[7]+u-v,meta[8]+(u+v)//2
 cell_points=[]
 for request in c['cells']:
  cell=request['metadata'];x,y=project(cell,base.signed(cell[13]),base.signed(cell[14]));ring=bool(cell[12] and cell[16]);offsets=[(-1,-1),(2,-1),(1,0),(0,0),(0,1),(1,1),(2,2),(-1,2)] if ring else [(0,0),(1,0),(0,1),(1,1)]
  for dx,dy in offsets:put(x+dx,y+dy,cell[28+cell[15]]);cell_points.append((y+dy)*width+x+dx)
 meta=c['creatures']['metadata'];offsets=[(-1,-2),(0,-2),(1,-2),(2,-1),(-2,-1),(-2,0),(0,0),(2,0),(2,1),(-2,1),(-1,2),(0,2),(1,2)]
 for px,py,hidden in c['creatures']['points']:
  if meta[11] and hidden:continue
  x,y=project(meta,px,py)
  for dx,dy in offsets:put(x+dx,y+dy,0xfc00)
 return footprint,known,cell_points

def source_witness(reader,case,c,observed):
 decoder=module('inspect-canvas-producers');wanted={};previous=None
 with tempfile.TemporaryDirectory(dir='/tmp',prefix='minimap-update-') as td:
  path=Path(td)/'worlds.bin';a=reader.json(case['pair']['acquisition']['record']);path.write_bytes(artifact(reader,a,'worlds.bin'))
  for g,chunk,raw,request in records(path):
   if chunk or g<2:continue
   if g>128:break
   ctx,body=source_context(raw);footprint,known,cell=minimap_writes(ctx,body)
   rows=decoder.decode(raw,complete=False)[1];entry,rasters=base.checked_request(rows,request)
   accepted=case['proof']['source_worlds'][g-1]
   require(ctx['entry']==list(entry) and accepted['generation']==g and accepted['source_sha256']==sha(raw) and accepted['request_sha256']==sha(request),'Different accepted source packet/request')
   # Hidden cells require auxiliary background bytes. They supply no independent
   # output claim here. Every directly determined final terrain/overlay WORD is
   # checked before the scene overwrite, including all overlaps in source order.
   if len(known)<=len(set(cell))+32:continue
   change=[]
   if previous:
    if previous['context']['render_input_sha256']==ctx['render_input_sha256']:continue
    change=[at for at in set(known)&set(previous['known']) if known[at]!=previous['known'][at]]
    if not change:continue
   item=dict(generation=g,context=ctx,source_sha256=sha(raw),request_sha256=sha(request),shape=[ctx['terrain_fields'][6],ctx['terrain_fields'][5]],footprint_words=len(footprint),hidden_background_words=len(footprint-set(known)),known_direct_words=len(known),visible=sorted(known),known=known,cell_points=cell,changed_source_words=change,previous_generation=previous['generation'] if previous else None,expected_sha256=sha(np.array([known[at] for at in sorted(known)],dtype='<u2').tobytes()))
   wanted[g]=item;previous=item
   if len(wanted)==3:break
 require(len(wanted)==3,'Missing three meaningful source minimap updates')
 return wanted,blend.OriginalWorlds(reader,c)

def rgba(words):
 words=words.astype('uint32');r=(words>>11)&31;g=(words>>5)&63;b=words&31
 return np.stack([(r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2),np.full(words.shape,255)],axis=1).astype('uint8')

def presented_maps(reader,case,a,c,observed,wanted,originals):
 history=History(originals);header,routes,_=copy.routes(reader,a);bound,refused=copy.command_join(observed['commands'],routes)
 diagnostics={v['sequence']:v for v in [json.loads(x) for x in artifact(reader,a,'copies.jsonl').splitlines()]}
 accepted={v['paint']:v for v in c['comparison']['comparisons']};paints={};trace={};marks={};proofs={};found={};created={}
 imports=[(seq,struct.unpack_from('<5I',p)) for op,seq,p in commands(observed['commands']) if op==19];intervals={}
 last=0
 for seq,w in imports:
  g=w[1]|w[2]<<32
  if g in wanted:
   candidates=[(n,struct.unpack('<10I',p)) for op,n,p in commands(observed['commands']) if last<n<seq and op==3 and struct.unpack_from('<I',p)[0]==w[0] and list(struct.unpack('<10I',p)[2:])==[0,0,wanted[g]['shape'][1],wanted[g]['shape'][0],0,0,0,0]]
   require(len(candidates)==1,'Pre-scene copy is missing/ambiguous inside exact source import interval')
   n,words=candidates[0];intervals[n]=(g,last,seq,w[0],words)
  last=seq
 for ordinal in observed['swaps']:
  p,body=observed['paints'][ordinal]
  if int(p['world'])>0:paints.setdefault(p['commands'],[]).append((p,body))
 for op,seq,p in commands(observed['commands']):
  if op==1:
   sid,width,height=struct.unpack_from('<3I',p);trace[sid]=np.full((height,width),-1,dtype='int64');created[sid]=seq
  elif op in (2,20):
   sid,x,y,width,height=struct.unpack_from('<5I',p);region=trace[sid][y:y+height,x:x+width]
   mask=np.ones(region.shape,dtype=bool) if op==2 else np.frombuffer(p[20+width*height*(history.handles[sid]['bits']//8):],dtype='u1').reshape(height,width).astype(bool);region[mask]=-1
  elif op==3:
   w=struct.unpack('<10I',p)
   if seq in intervals:
    g,prior,following,sid,words=intervals[seq];item=wanted[g];source=history.handles[sid]['pixels'];positions=item['visible'];expected=np.array([item['known'][at] for at in positions],dtype='uint32')
    require(np.array_equal(source.ravel()[positions],expected),'Pre-scene actual map writes differ from source-derived expected WORDs')
    row=bound[seq];require(row[13]==0 and row[14] and row[40]<=seq<=row[41] and (row[42],row[43])==(w[0],w[1]) and row[15] in (1,2),'Unsuccessful/unbound native display copy')
    copy.checked_destination(source,history.handles[w[1]]['pixels'],w,diagnostics[seq])
    trace[sid][:]=-1;trace[sid].ravel()[positions]=g*copy.STRIDE+np.array(positions)
    proofs[g]=dict(copy_sequence=seq,prior_import_sequence=prior,next_import_sequence=following,source_surface=sid,source_created=created[sid],target_surface=w[1],target_created=created[w[1]],caller=list(row),words=list(w),source_direct_sha256=sha(expected.astype('<u2').tobytes()),complete_copy_sha256=diagnostics[seq]['actual_sha256'])
   copy.trace_copy(trace,history,w)
  elif op==7:
   sid=struct.unpack_from('<I',p)[0];trace.pop(sid);created.pop(sid)
  elif op==11:
   aa,bb=struct.unpack_from('<2I',p);trace[aa],trace[bb]=trace[bb],trace[aa]
  elif op==19:
   sid=struct.unpack_from('<I',p)[0];trace[sid][:]=-1
   # Existing display/sealed UI retains its copied map while only the World
   # source is replaced. Preserve those tags across the later scene import.
  history.apply(op,seq,p)
  if op==6:
   sid=struct.unpack_from('<I',p)[0];marks['presented']=trace[sid].copy()
   if history.sealed is not None:marks['sealed']=trace[sid].copy()
  for paint,body in paints.get(seq,[]):
   tags=marks.get('presented' if paint['mode']==0 else 'sealed')
   if tags is None:continue
   if paint['mode']==0:
    if history.display is None or history.display[0]!=paint['surface'] or history.presented_generation!=int(paint['world']):continue
   elif history.sealed is None or history.sealed['generation']!=int(paint['sealed_world']):continue
   for g,item in wanted.items():
    if g in found or g not in proofs:continue
    yy,xx=np.nonzero(tags//copy.STRIDE==g);indices=(tags[yy,xx]%copy.STRIDE).astype('int64');unique=np.unique(indices)
    if set(map(int,unique))!=set(item['visible']):continue
    require(tags.shape==(paint['frame_height'],paint['frame_width']) and paint['scaling']==2 and paint['image_width']%paint['frame_width']==0 and paint['image_height']%paint['frame_height']==0,'Noninteger map viewport')
    scale=paint['image_width']//paint['frame_width'];require(scale>0 and scale==paint['image_height']//paint['frame_height'],'Asymmetric map viewport')
    raw=body() if callable(body) else body;match=accepted[int(paint['paint'])]
    require(sha(raw)==match['actual_sha256']==match['expected_sha256'] and match['world']==int(paint['world']) and match['lease']==int(paint['lease'])>0 and match['mismatching_pixels']==0,'Different accepted complete presentation')
    actual=np.frombuffer(raw,dtype='u1').reshape(paint['height'],paint['width'],4);expected=rgba(np.array([item['known'][int(at)] for at in indices],dtype='uint32'));collected=[]
    for dy in range(scale):
     for dx in range(scale):
      pixels=actual[paint['top']+yy*scale+dy,paint['left']+xx*scale+dx];require(np.array_equal(pixels,expected),'Actual displayed map differs from source-derived expected WORDs');collected.append(pixels.tobytes())
    found[g]=dict(generation=g,paint=int(paint['paint']),presentation_world=int(paint['world']),lease=int(paint['lease']),command_sequence=seq,viewport_pixels=len(indices)*scale*scale,mapped_source_words=list(map(int,unique)),source_visible_words=len(item['visible']),mapped_unique_words=len(unique),composition_unmapped_words=0,actual_map_sha256=sha(b''.join(collected)),expected_map_sha256=sha(expected.tobytes()*scale**2),full_viewport_sha256=match['actual_sha256'],full_viewport_pixels=match['pixels'],independently_matched=True,phase=proofs[g])
  if len(found)==3:break
 require(set(found)==set(wanted),'Missing fully visible source-bound native minimap update')
 result=[]
 for g,item in wanted.items():
  proof=found[g]
  proof['changed_visible_words']=len(item['changed_source_words']) if item['previous_generation'] else 0
  source={k:v for k,v in item.items() if k not in ('known','visible','changed_source_words')};result.append(dict(source=source,presentation=proof))
 check_updates(result);return result

def check_updates(rows):
 require(len(rows)==3 and [v['source']['generation'] for v in rows]==sorted({v['source']['generation'] for v in rows}),'Missing advancing map source updates')
 for i,v in enumerate(rows):
  s,p=v['source'],v['presentation'];validate_context(s['context'])
  require(p['generation']==s['generation'] and p['lease']>0 and p['independently_matched'] and p['actual_map_sha256']==p['expected_map_sha256'] and p['viewport_pixels']>0 and p['mapped_unique_words']==p['source_visible_words']==s['known_direct_words'] and p['composition_unmapped_words']==p['source_visible_words']-p['mapped_unique_words'] and s['known_direct_words']>32 and p['phase']['prior_import_sequence']<p['phase']['copy_sequence']<p['phase']['next_import_sequence'] and p['phase']['copy_sequence']<=p['command_sequence'] and p['phase']['source_direct_sha256']==s['expected_sha256'],'Incomplete visible map output/ownership')
  phase=p['phase'];row=phase['caller'];w=phase['words'];height,width=s['shape']
  require(row[13]==0 and row[14] and row[15] in (1,2) and row[40]<=phase['copy_sequence']<=row[41] and row[42:44]==w[:2]==[phase['source_surface'],phase['target_surface']] and w[2:]==[0,0,width,height,0,0,0,0] and phase['source_created']<phase['prior_import_sequence'] and phase['target_created']<phase['copy_sequence'] and p['presentation_world'] in (s['generation']-1,s['generation']),'Wrong copy result/phase/incarnation')
  if i:require(s['previous_generation']==rows[i-1]['source']['generation'] and s['context']['render_input_sha256']!=rows[i-1]['source']['context']['render_input_sha256'] and p['changed_visible_words']>0,'Redraw or metadata-only change is not a minimap update')

def close_rows(plan,parent,runs):
 validate(plan);require(parent['passed_requirements']==60 and parent['dossier_identity']==fingerprint(requirements(plan)),'Accepted60-row inventory differs')
 require(len(runs)==4 and {(v['domain'][0],tuple(v['domain'][1])) for v in runs}=={(p,tuple(z)) for p in ('player-default','player-software') for z in ([640,480],[800,600])} and len({v['identity'] for v in runs})==4,'Missing/duplicate profile/size context')
 for v in runs:check_updates(v['updates'])
 rows=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id']==LEAF:
   require(row['state']=='pending','Minimap update already accepted');row.update(state='passed',admitted_execution_evidence=[dict(execution=v['pair'],actual_world_extents=[v['domain'][1]],provider=v['provider'],reason='Three advancing source-bound real minimap updates, independently source-derived WORDs before the scene overwrite, all directly determined terrain/overlay WORDs and their complete actual viewport mapping; hidden auxiliary-background words remain outside this finite update claim with meaningful changed output. View zero and recorded native source phases; full markers/borders/camera/picking/visibility and other contexts remain separate.') for v in runs if v['domain'][0]==row['configuration']])
  rows.append(row)
 require(sum(v['state']=='passed' for v in rows)==62,'Closeout exceeded two acceptance rows')
 return rows

def analyze(reader):
 parentname='research/runtime/rendering-four-row-results-20261107/closeout-analysis-02.json';parent=reader.json(parentname);cp=reader.json('research/runtime/rendering-copy-results-20261106/closeout-analysis-01.json');bd=reader.json(cp['input_parent']['record']);dh=reader.json(bd['input_parent']['record']);runs=[]
 for case in dh['executions']:
  if case['scenario']!='normal-storage':continue
  a,c=[reader.json(case['pair'][k]['record']) for k in ('acquisition','comparison')];require(a['success'] and c['success'] and c['acquisition']==case['pair']['acquisition'] and a['session_identity']==case['identity'] and c['comparison']['mismatching_pixels']==0 and c['original_reference']['continuous_from_owned_initial_history'] and not c['original_reference']['native_player_pixels_read'] and not c['original_reference']['original_destination_capture_read'] and all(reader.digest(case['pair'][k]['record'])==case['pair'][k]['sha256'] for k in case['pair']),'Changed/dependent admitted original comparison')
  observed=acquisition([artifact(reader,a,Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')],a['session_identity']);require(sha(observed['commands'])==c['captured_command_stream_sha256'],'Different compared command stream')
  wanted,originals=source_witness(reader,case,c,observed);updates=presented_maps(reader,case,a,c,observed,wanted,originals);runs.append(dict(domain=case['domain'],scenario=case['scenario'],identity=case['identity'],pair=case['pair'],provider=case['provider'],updates=updates));print('Verified minimap updates',case['domain'],[(v['source']['generation'],v['presentation']['changed_visible_words']) for v in updates],flush=True)
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(v for v in reg['behaviors'] if v['id']==BID);sources={n:reader.digest(n) for role in ('implementation','tests','validation_dependencies') for n in b[role]};combined=dict(parent['sources']);combined.update({n:h for n,h in sources.items() if n not in combined});rows=close_rows(reader.json('docs/native-rendering-closure.json'),parent,runs)
 return dict(schema=1,kind='retained_minimap_update_two_row_closeout',success=True,sources=combined,analysis_sources=sources,scope='Close actual view-zero minimap updates for both profiles at both actual frame sizes from four retained Campaign/storage cases. Owned map state/source/import identities, independently computed directly determined terrain/overlay WORDs before scene drawing, uniquely interval-bound display copies and all corresponding native viewport pixels; hidden auxiliary-background restoration remains separate with meaningful changed output. Preserve60parent rows, all failures and historical native source versions. No whole markers/borders/camera/picking/simulation visibility, new game/build/fixture or final decision claim.',parent=dict(record=parentname,sha256=reader.digest(parentname)),executions=runs,requirements=rows,dossier_identity=parent['dossier_identity'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),decisions=parent['decisions'],passed_requirements=62,pending_requirements=94,newly_passed_requirements=2,actual_contexts=4,source_updates=12,matched_map_pixels=sum(v['presentation']['viewport_pixels'] for c in runs for v in c['updates']),changed_visible_words=sum(v['presentation']['changed_visible_words'] for c in runs for v in c['updates']),original_game_runs=0,new_fixture=False,new_native_code=False,historical_failures_retired=False,current_native_engine_execution=False)

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'Fresh owned report required');q=dict(schema=1,success=False)
 try:
  with Reader(ROOT) as r:
   reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(v for v in reg['behaviors'] if v['id']==BID);before={n:r.digest(n) for role in ('implementation','tests','validation_dependencies') for n in b[role]};q=analyze(r);require(before==q['analysis_sources'],'Analyzer/helper sources changed during execution');q['analysis_sources_stable']=True
 except Exception as e:q.update(success=False,error=str(e))
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h)),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
