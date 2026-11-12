#!/usr/bin/env python3
"""Close four finite acceptance rows from retained keyed and palette output."""
import argparse,hashlib,importlib.util,json,struct,tempfile
from pathlib import Path
from functools import lru_cache
import numpy as np
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_menu_acceptance import artifact
from rendering_pilot import History,acquisition,commands,require
from resident_capture import records
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-four-row-results-20261107/'
LEAVES={'operation.copy-keyed','operation.palette-remap'}
def module(name):
 s=importlib.util.spec_from_file_location('keypal_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
copy=module('close-rendering-opaque-cropped-copy');blend=copy.base;base=blend.base
sha=base.sha

def keyed_destination(source,before,w,d):
 require(source.ndim==before.ndim==2 and len(w)==10 and w[8]==1 and w[0]!=w[1],'Not a disjoint keyed copy')
 l,t,r,b,x,y=w[2:8];h,width=source.shape;dh,dw=before.shape
 require(0<=l<r<=width and 0<=t<b<=h and x>=0 and y>=0 and x+r-l<=dw and y+b-t<=dh,'Keyed copy leaves owned storage')
 raw=source[t:b,l:r];mask=raw!=w[9];expected=before.copy();region=expected[y:y+b-t,x:x+r-l];prior=before[y:y+b-t,x:x+r-l];region[mask]=raw[mask]
 changed=mask&(region!=prior);require(np.any(mask) and np.any(~mask) and np.any(changed),'Trivial key/changed destination')
 require(d['words']==list(w)+[0,0,0] and (d['source_width'],d['source_height'],d['target_width'],d['target_height'])==(width,h,dw,dh) and d['pixels']==dw*dh and d['mismatches']==0 and d['source_sha256']==copy.words_hash(source) and d['before_sha256']==copy.words_hash(before) and d['actual_sha256']==d['expected_sha256']==copy.words_hash(expected),'Independent complete keyed destination differs')
 require(np.array_equal(region[~mask],prior[~mask]),'Skipped key changed prior destination')
 return expected,changed,dict(changed_words=int(np.count_nonzero(changed)),skipped_words=int(np.count_nonzero(~mask)),skipped_nonzero_words=int(np.count_nonzero(prior[~mask])),before_skipped_sha256=copy.words_hash(prior[~mask]),after_skipped_sha256=copy.words_hash(region[~mask]))

def keyed_output(reader,case,a,c,observed):
 header,rows,routes=copy.routes(reader,a);bound,refused=copy.command_join(observed['commands'],rows)
 diagrows=[json.loads(v) for v in artifact(reader,a,'copies.jsonl').splitlines()];diags={v['sequence']:v for v in diagrows};require(len(diags)==len(diagrows),'Duplicate actual copy diagnostic')
 model=History(blend.OriginalWorlds(reader,c));trace={};created={};marks={};proofs={};found=[];paints={};accepted={v['paint']:v for v in c['comparison']['comparisons']}
 live={v['paint'] for v in case['proof']['presented_worlds'] if v['source_import_live']}
 for ordinal in observed['swaps']:
  p,body=observed['paints'][ordinal]
  if int(p['world'])>0 and ordinal in live:paints.setdefault(p['commands'],[]).append((p,body))
 for op,seq,p in commands(observed['commands']):
  if op==1:
   w=struct.unpack_from('<7I',p);trace[w[0]]=np.full((w[2],w[1]),-1,dtype='int64');created[w[0]]=seq
  elif op in (2,20):
   sid,x,y,width,height=struct.unpack_from('<5I',p);target=trace[sid][y:y+height,x:x+width]
   if op==2:target[:]=-1
   else:
    n=width*height*(model.handles[sid]['bits']//8);mask=np.frombuffer(p[20+n:],dtype='u1').reshape(height,width).astype(bool);target[mask]=-1
  elif op==3:
   w=struct.unpack('<10I',p);source,target=model.handles[w[0]],model.handles[w[1]];copy.trace_copy(trace,model,w)
   if w[8]==1 and w[0]!=w[1] and model.generation>0 and seq in diags and all(v['bits']==16 and v['masks']==(0xf800,0x7e0,31) for v in (source,target)) and not any(v['generation']==model.generation for v in found):
    try:after,changed,extra=keyed_destination(source['pixels'],target['pixels'],w,diags[seq])
    except ValueError as e:
     if str(e)!='Trivial key/changed destination':raise
    else:
     row=bound[seq];require(row[13]==0 and row[14] and row[15]==1 and row[10]&64 and row[22]==w[9] and header[15]<=row[1]<header[15]+header[16],'Key/source/caller authority differs')
     proof=dict(kind='keyed',generation=model.generation,sequence=seq,words=list(w),caller=list(row),source_created=created[w[0]],target_created=created[w[1]],source_shape=list(source['pixels'].shape),target_shape=list(target['pixels'].shape),source_sha256=copy.words_hash(source['pixels']),before_sha256=copy.words_hash(target['pixels']),actual_sha256=copy.words_hash(after),**extra)
     proofs[seq]=proof;l,t,r,b,x,y=w[2:8];region=trace[w[1]][y:y+b-t,x:x+r-l];coords=np.arange(source['pixels'].size,dtype='int64').reshape(source['pixels'].shape)[t:b,l:r];visible=changed&(after[y:y+b-t,x:x+r-l]!=0);region[visible]=(seq*copy.STRIDE+coords)[visible]
  elif op==7:
   sid=struct.unpack('<I',p)[0];trace.pop(sid);created.pop(sid)
  elif op==11:
   x,y=struct.unpack('<2I',p);trace[x],trace[y]=trace[y],trace[x]
  elif op==19:
   for values in trace.values():values[:]=-1
   marks.clear()
  model.apply(op,seq,p)
  if op==6:
   sid=struct.unpack('<I',p)[0];marks['presented']=trace[sid].copy()
   if model.sealed is not None:marks['sealed']=trace[sid].copy()
  for paint,body in paints.get(seq,[]):copy.presentation_sample(paint,body,accepted,model,marks,proofs,found)
  if len(found)==3:break
 check_keyed(found)
 branch=routes['branches']['keyed'];keyed_boundary(branch,refused)
 return dict(verified=True,witnesses=found,route_boundary=branch,all_routes=routes['routes'],bound_copy_commands=len(bound),historical_refused_calls=refused,scope='Valid actual keyed calls in this closed supported interval are CPU-prepared and disjoint. The exact known640storage cursor destination outside640x480 retains INVALIDRECT/no emitted work and earns no success credit. Natural GPU-keyed reachability elsewhere is unproved; supplied GPU policy controls are separate, not positive application counts. No current native DLL, cursor-interaction or original driver/ABI claim.')

def keyed_boundary(branch,refused):
 bad=[r for r in refused if r[12]&(1 if r[11] else 0x8000)]
 require(branch['calls']>=3 and branch['failed_calls']==len(bad)<=1 and branch['prepared_gpu_calls']==0 and branch['routes'].get('cpu_prepared',0)+len(bad)==branch['calls'] and branch['routes'].get('unprepared',0)==len(bad),'Declared finite keyed route boundary differs')
 for r in bad:
  require(r[1]==0x58cbb2 and r[11]==1 and r[12]==17 and r[13]==0x88760096 and r[14:16]==[0,0] and r[16:24]==[400,280,16,640,480,16,31,0] and r[24:32]==[0,0,40,40,640,512,680,552] and not any(r[40:44]),'Unexpected keyed failure cannot be excluded')

def check_keyed(rows):
 require(len(rows)==3 and len({v['generation'] for v in rows})==3,'Missing three advancing, visible actual keyed destinations')
 for v in rows:
  c=v['copy'];w=c['words'];r=c['caller']
  require(v['kind']==c['kind']=='keyed' and c['generation']==v['generation']>0 and c['sequence']<=v['command_sequence'] and r[40]<=c['sequence']<=r[41] and r[13]==0 and r[14] and r[15]==1 and r[10]&64 and r[22]==w[9] and w[8]==1 and w[0]!=w[1] and c['changed_words']>0 and c['skipped_words']>0 and c['before_skipped_sha256']==c['after_skipped_sha256'] and v['actual_rgba']==v['expected_rgba'] and any(v['actual_rgba'][:3]) and v['lease']>0,'Incomplete keyed source/result/preservation/presentation')

@lru_cache(maxsize=256)
def indexed_frame(frame):
 width,height,ox,oy,runs=base.frame_runs(frame);image=np.full((height,width),-1,dtype='int16');cursor={y:struct.unpack_from('<I',frame,44+y*8)[0] for y in range(height)}
 for y,a,b in runs:
  at=cursor[y];image[y,a:b]=np.frombuffer(frame[at:at+b-a],dtype='u1');cursor[y]+=b-a
 return image,ox,oy

def palette_choices(rows,request,original):
 _,rasters=base.checked_request(rows,request);size=(original.shape[1],original.shape[0]);writers=blend.last_writers(rows,rasters,size);groups={}
 for f,body,d in rasters:
  if f[14]!=0 or f[17]!=1 or f[20]!=512 or d[2]:continue
  frame=body[:f[19]];table=body[f[19]:];colours=np.frombuffer(table,dtype='<u2');require(len(colours)==256,'Missing actual selected colour table')
  if len(np.unique(colours))<2:continue
  indices,ox,oy=indexed_frame(frame);left,top=base.signed(f[8])-ox,base.signed(f[9])-oy
  yy,xx=np.nonzero(writers==f[1]);valid=(xx<size[0]-200)&(yy<size[1]-50)&(xx>=left)&(yy>=top)&(xx<left+indices.shape[1])&(yy<top+indices.shape[0]);yy,xx=yy[valid],xx[valid]
  if not len(xx):continue
  ix=indices[yy-top,xx-left];valid=ix>=0;yy,xx,ix=yy[valid],xx[valid],ix[valid];valid=(original[yy,xx]==colours[ix])&(colours[ix]!=0);yy,xx,ix=yy[valid],xx[valid],ix[valid]
  if not len(xx):continue
  _,positions=np.unique(ix,return_index=True);framehash=sha(frame);tablehash=sha(table);shade=base.signed(f[16])
  for at in positions[:64]:
   index=int(ix[at]);x,y=int(xx[at]),int(yy[at]);word=int(colours[index]);values=groups.setdefault((framehash,index),{})
   values.setdefault((shade,tablehash,word),dict(world_pixel=y*size[0]+x,destination=[x,y],original_word=word,sequence=f[1],fields=list(f),descriptor=list(d),payload_sha256=sha(body),source=dict(word=word,palette_index=index,source=[x-left,y-top],frame_sha256=framehash,table_sha256=tablehash,shade=shade,table_hex=table.hex())))
   chosen=[]
   for value in values.values():
    if any(value['source']['shade']==v['source']['shade'] or value['source']['table_sha256']==v['source']['table_sha256'] or value['original_word']==v['original_word'] for v in chosen):continue
    chosen.append(value)
    if len(chosen)==3:return {'palette-'+str(i):[v] for i,v in enumerate(chosen)}
 raise ValueError('No three differing selected shade tables for one visible frame/index')

def palette_output(reader,case,a,c,observed):
 decoder=module('inspect-canvas-producers');originals=blend.OriginalWorlds(reader,c);wanted={};packets=[]
 with tempfile.TemporaryDirectory(prefix='keypal-source-',dir='/tmp') as td:
  path=Path(td)/'worlds.bin';path.write_bytes(artifact(reader,a,'worlds.bin'))
  for generation,chunk,raw,request in records(path):
   if chunk or generation<2:continue
   _,rows=decoder.decode(raw,complete=False);wanted[generation]=palette_choices(rows,request,originals[generation]);packets.append(dict(generation=generation,source_sha256=sha(raw),request_sha256=sha(request)))
   if generation==4:break
 require(set(wanted)=={2,3,4},'Missing three actual source-generation palette contexts')
 samples=blend.viewport_samples(case,observed,originals,wanted);check_palettes(samples)
 return dict(verified=True,witnesses=samples,source_packets=packets,scope='Effective owned source tables selected by actual signed shade/request identities; same frame/index maps to three distinct nonzero WORDs and actual independent native composition in each generation. No upstream builder/platform/chain ordinal, cycling, arbitrary asset or current native source promotion.')

def check_palettes(samples):
 require(len(samples)==9 and {v['generation'] for v in samples}=={2,3,4},'Missing selected-table output')
 for g in (2,3,4):
  rows=[v for v in samples if v['generation']==g];sources=[v['source']['source'] for v in rows]
  require(len(rows)==3 and len({v['frame_sha256'] for v in sources})==len({v['palette_index'] for v in sources})==1 and all(len({v[k] for v in sources})==3 for k in ['shade','table_sha256','word']),'No actual same-frame/index remapping variation')
  for row in rows:
   original=row['source'];s=original['source'];table=bytes.fromhex(s['table_hex']);require(sha(table)==s['table_sha256'] and struct.unpack_from('<H',table,2*s['palette_index'])[0]==s['word']==original['original_word'] and original['descriptor'][0]==original['sequence']==original['fields'][1] and base.signed(original['fields'][16])==s['shade'] and row['actual_rgba']==row['original_rgba']==blend.rgba(s['word']) and row['lease']>0 and row['lineage_world_pixel']==original['world_pixel'],'Selected effective table/request/actual output differs')

def close_rows(plan,parent,runs):
 validate(plan);require(parent['passed_requirements']==56 and parent['dossier_identity']==fingerprint(requirements(plan)),'Accepted56-row inventory differs')
 expected={(p,s,tuple(z)) for p in ('player-default','player-software') for s in ('normal-quick','normal-storage') for z in ([640,480],[800,600])}
 require(len(runs)==8 and {(r['domain'][0],r['scenario'],tuple(r['domain'][1])) for r in runs}==expected and len({r['identity'] for r in runs})==8,'Missing/duplicate required context')
 require(all(r['keyed']['verified'] and r['palette']['verified'] for r in runs),'Incomplete actual operation output')
 for r in runs:check_keyed(r['keyed']['witnesses']);check_palettes(r['palette']['witnesses'])
 rows=[]
 for old in parent['requirements']:
  row=dict(old)
  if row['id'] in LEAVES:
   require(row['state']=='pending','Already accepted selected leaf');row.update(state='passed',admitted_execution_evidence=[dict(execution=r['pair'],actual_world_extents=[r['domain'][1]],provider=r['provider'],reason=r['keyed' if row['id']=='operation.copy-keyed' else 'palette']['scope']) for r in runs if r['domain'][0]==row['configuration']])
  rows.append(row)
 require(sum(v['state']=='passed' for v in rows)==60,'Closeout exceeded four rows')
 return rows

def analyze(reader):
 parentname='research/runtime/rendering-functional-ten-results-20261107/closeout-analysis-01.json';parent=reader.json(parentname);cp=reader.json('research/runtime/rendering-copy-results-20261106/closeout-analysis-01.json');bd=reader.json(cp['input_parent']['record']);dh=reader.json(bd['input_parent']['record']);runs=[]
 require(reader.digest(cp['input_parent']['record'])==cp['input_parent']['sha256'] and reader.digest(bd['input_parent']['record'])==bd['input_parent']['sha256'] and dh['success'],'Changed retained composition parent')
 for case in dh['executions']:
  require(any(v['identity']==case['identity'] and v['pair']==case['pair'] for v in cp['executions']),'Unaccepted retained context')
  a,c=[reader.json(case['pair'][k]['record']) for k in ('acquisition','comparison')];require(a['success'] and c['success'] and c['comparison']['mismatching_pixels']==0 and c['original_reference']['continuous_from_owned_initial_history'] and not c['original_reference']['native_player_pixels_read'] and not c['original_reference']['original_destination_capture_read'] and all(reader.digest(case['pair'][k]['record'])==case['pair'][k]['sha256'] for k in case['pair']),'Changed/dependent retained proof')
  observed=acquisition([artifact(reader,a,Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')],a['session_identity']);require(sha(observed['commands'])==c['captured_command_stream_sha256'],'Different actual command stream')
  keyed=keyed_output(reader,case,a,c,observed);accepted_case=next(v for v in cp['executions'] if v['identity']==case['identity']);require(keyed['historical_refused_calls']==accepted_case['proof']['refused_calls'],'Historical refusals changed');palette=palette_output(reader,case,a,c,observed);runs.append(dict(domain=case['domain'],scenario=case['scenario'],identity=case['identity'],pair=case['pair'],provider=case['provider'],keyed=keyed,palette=palette));print('Verified',case['domain'],case['scenario'],flush=True)
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());paths={n for b in reg['behaviors'] if b['id'] in ['NP.keyed-copy-closeout','NP.palette-remap-closeout'] for role in ('implementation','tests','validation_dependencies') for n in b[role]};sources={n:reader.digest(n) for n in sorted(paths)}
 rows=close_rows(reader.json('docs/native-rendering-closure.json'),parent,runs);gpu_name='research/runtime/cpu-copy-results-20261026/gpu-controls02.json';gpu=reader.json(gpu_name);require(gpu['success'] and gpu['sources_stable'] and len(gpu['cases'])==23 and all(v['success'] and v['final_native_surfaces']==0 for v in gpu['cases']),'Incomplete retained GPU policy controls')
 combined=dict(parent['sources']);combined.update({n:h for n,h in sources.items() if n not in combined})
 return dict(schema=1,kind='retained_keyed_palette_four_row_closeout',success=True,sources=combined,analysis_sources=sources,scope='New retained-data analysis closes four finite keyed-copy/selected-palette rows across eight actual contexts. Preserve56parent rows and every historical engine/source failure. Native execution/comparison remain pinned to recorded versions; no native full-hash waiver, current whole-player/driver/ABI/unknown GPU caller/upstream palette builder/cycling/final decision claim.',parent=dict(record=parentname,sha256=reader.digest(parentname)),executions=runs,supplied_gpu_controls=dict(record=gpu_name,sha256=reader.digest(gpu_name),cases=23,source_phase='historical supplied controls; no new or natural application execution'),requirements=rows,dossier_identity=parent['dossier_identity'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|LEAVES),decisions=parent['decisions'],passed_requirements=60,pending_requirements=96,newly_passed_requirements=4,actual_contexts=8,keyed_witnesses=24,palette_witnesses=72,original_game_runs=0,new_fixture=False,new_native_code=False,historical_failures_retired=False)

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'Fresh owned report required');q=dict(schema=1,success=False)
 try:
  with Reader(ROOT) as r:q=analyze(r)
 except Exception as e:q.update(success=False,error=str(e))
 h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=h)),flush=True);return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
