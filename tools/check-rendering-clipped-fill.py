#!/usr/bin/env python3
"""Bind retained clipped-fill calls and compare exact-input native GPU replay."""
from pathlib import Path
import importlib.util
import json
import struct
import sys
import os
import shlex
import subprocess
import tempfile
import argparse
import numpy as np
sys.path.insert(0, 'tools')
from coverage_evidence import Reader, write_report
from coverage_claims import behavior_contract, scenario_contract, required_sources
from rendering_menu_acceptance import artifact
from rendering_pilot import History, acquisition, commands, rgba, viewport, require, sha
from native_copy_clip_regions_scoped import decode_owner
from storage_cpu_read_binding import reply_header

root = Path(__file__).resolve().parents[1]
P = 'research/runtime/rendering-clipped-operations-results-20261107/'
OUT = 'research/runtime/rendering-clipped-fill-results-20261108/'
spec = importlib.util.spec_from_file_location('copy_contract', root/'tools/close-rendering-opaque-cropped-copy.py')
cp = importlib.util.module_from_spec(spec); spec.loader.exec_module(cp)


BID='NP.clipped-fill-operation-closeout'
LEAF='operation.fill-clipped'
COMPARISONS={'default-800-07':'06','default-640-11':'03','software-800-12':'03','software-640-14':'02'}

def bind_fills(clips,rows,owner,header):
    queries=[q for q in clips if q['phase']==3]
    actuals=[q for q in clips if q['phase']==5]
    callers=[r for r in rows if r[15]==3 and r[23] and r[3]==owner]
    require(len(queries)==len(actuals)==len(callers)==2,'Clipped fill census cardinality')
    for q,a,r in zip(queries,actuals,callers):
        require(q['ordinal']+1==a['ordinal'] and q['target_canonical']+12==r[6]
            and q['target_alias']==r[4] and q['thread']==r[3]==a['thread']
            and q['target_incarnation']>0 and q['clipper_references']>0
            and not q['source_alias'] and not q['source_canonical'] and not r[5]
            and r[1]==0x58bc5a and r[2]==r[1]-header[15]
            and header[15]<=r[1]<header[15]+header[16]
            and r[11]==0 and r[12]==0x1000400 and r[13]==0 and r[14:16]==(1,3)
            and r[10]&32 and not r[10]&128 and not any(r[28:32])
            and q['destination']==[0,0,*q['target_description'][:2]]
            and q['target_description']==list(r[19:22]) and q['flags']==0x400
            and a['flags']==r[12] and q['result']==a['result']==0
            and q['plan_pieces']==len(q['regions'])==1
            and a['fill_observation']==1 and 0<=a['fill_value']<=65535,
            'Fill query/actual/original caller binding')
        fields=('thread','target_alias','target_canonical','target_incarnation','clipper',
                'window','clipper_references','destination','target_description','rgndata_words')
        require(all(q[k]==a[k] for k in fields),'Fill snapshot changed before actual operation')
        q['actual']=a
    return queries

def fill_output(before,q,actual,upload):
    require(before.ndim==2 and before.shape==(q['target_description'][1],q['target_description'][0])
            and q['target_description'][2]==16 and actual['fill_observation']==1,
            'Fill storage/observed constant differs')
    mask=np.zeros(before.shape,dtype=bool);pieces=[]
    for l,t,r,b in q['regions']:
        l,t=max(0,l,q['destination'][0]),max(0,t,q['destination'][1])
        r,b=min(before.shape[1],r,q['destination'][2]),min(before.shape[0],b,q['destination'][3])
        if l<r and t<b:mask[t:b,l:r]=True;pieces.append([l,t,r-l,b-t])
    require(len(pieces)==1 and 0<int(mask.sum())<before.size,'Missing positive clipping/exterior')
    sid,x,y,w,h=struct.unpack_from('<5I',upload)
    require(sid>0 and [x,y,w,h]==pieces[0] and len(upload)==20+w*h*2,
            'Clipped fill UPDATE footprint/extent')
    expected=before.copy();expected[mask]=actual['fill_value']
    require(upload[20:]==expected[y:y+h,x:x+w].astype('<u2').tobytes(),
            'Actual fill upload differs from independently observed argument')
    return expected,mask

def replay_stream(before,upload):
    sid=struct.unpack_from('<I',upload)[0];h,w=before.shape
    payloads=[(1,struct.pack('<7I',sid,w,h,16,0xf800,0x7e0,31)+before.astype('<u2').tobytes()),
              (2,upload),(6,struct.pack('<I',sid)),(7,struct.pack('<I',sid)),(8,b'')]
    return b'MNMCMD05'+struct.pack('<II',5,16)+b''.join(struct.pack('<III',op,i,len(p))+p for i,(op,p) in enumerate(payloads,1))

def checked_gpu_output(actual,expected,before,mask):
    require(len(actual)==expected.size*2,'Incomplete native GPU replay plane')
    plane=np.frombuffer(actual,dtype='<u2').reshape(expected.shape)
    require(np.array_equal(plane,expected) and np.array_equal(plane[~mask],before[~mask]),
            'Complete native GPU clipped-fill output/exterior differs')
    return dict(pixels=expected.size,changed_words=int(np.count_nonzero(plane!=before)),
                exterior_words=int((~mask).sum()),nonzero_exterior_words=int(np.count_nonzero(before[~mask])),
                actual_sha256=sha(actual),expected_sha256=sha(expected.astype('<u2').tobytes()),mismatches=0)

def check_case(case):
    require(case['success'] and case['observed_fills']==2 and case['no_op_fills']==1
        and case['region_scope']['owner_records_complete'] and not case['region_scope']['global_census_accepted'],
        'Incomplete owner-scoped clipped-fill case')
    positive=[w for w in case['all_fills'] if w['changed_words']>0]
    require(len(positive)==1 and positive[0]==case['witness'],'Repeated/no-op/inconsistent fill witness')
    w=case['witness'];r=w['caller'];q=w['query'];n=w['native_replay'];proof=n['proof']
    require(w['created']<w['sequence']<w['deleted'] and w['lower_fence']<w['sequence']<w['upper_fence']
        and w['prestate_fully_known'] and w['prior_gpu_reply_sequence']<w['sequence']
        and r[13]==q['result']==q['actual']['result']==0
        and r[6]==q['target_canonical']+12 and q['target_alias']==r[4]
        and n['success'] and n['current_native_execution']
        and not n['expected_pixels_supplied_to_native'] and not n['original_pixels_used_as_native_inputs']
        and not n['historical_missing_paint_recovered'] and proof['mismatches']==0
        and proof['pixels']==480000 and proof['changed_words']==w['changed_words']>0
        and proof['exterior_words']==w['exterior_words']>0
        and proof['nonzero_exterior_words']==w['nonzero_exterior_words']
        and proof['actual_sha256']==proof['expected_sha256']==w['expected_sha256'],
        'Incomplete natural input/current GPU output binding')
    if case['domain'][1]==[640,480]:
        require(len(case['presentations'])==1 and w['nonzero_exterior_words']>0
                and case['presentations'][0]['sequence']==w['sequence'], 'Missing historical Low presented fill')

def native_replay(reader,binary,before,upload,expected,mask,stem):
    encoded=replay_stream(before,upload)
    refs={}
    def retain(label,body):
        name=OUT+RUN+'-'+stem+'-'+label+'.bin';refs[label]=dict(record=name,sha256=write_report(root,name,body),bytes=len(body))
    retain('native-prior-input',before.astype('<u2').tobytes());retain('actual-upload',upload)
    retain('derived-native-stream',encoded)
    with tempfile.TemporaryDirectory(prefix='clipped-fill-replay-',dir=root/'working') as directory:
        directory=Path(directory);input_path=directory/'commands.bin';output=directory/'actual.565'
        input_path.write_bytes(encoded)
        env=dict(os.environ,LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='2',QT_QPA_PLATFORM='xcb')
        command=['xvfb-run','-a','-s','-screen 0 1024x768x24',str(binary),str(input_path),'--output',str(output)]
        result=subprocess.run(command,env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=60)
        retain('native-stdout',result.stdout);retain('native-stderr',result.stderr)
        require(result.returncode==0,'Native replay refused: '+result.stderr.decode(errors='replace')[-2000:])
        runtime=json.loads(result.stdout);require(runtime['rendered'] and runtime['commands']==5
            and runtime['presentations']==1 and runtime['checks']==0,'Unexpected replay route/expected-plane input')
        body=output.read_bytes();proof=checked_gpu_output(body,expected,before,mask)
        retain('actual-native-gpu-output',body)
    return dict(success=True,proof=proof,artifacts=refs,runtime=runtime,command=command,
                current_native_execution=True,historical_missing_paint_recovered=False,
                expected_pixels_supplied_to_native=False,original_pixels_used_as_native_inputs=False,
                boundary='Derived exact recorded native prior/UPDATE inputs. Current GPU upload/readback, not an assertion of an unobserved historical intermediate presentation or whole original renderer equivalence.')

def compiler_sources(reader,binary):
    build=next(p for p in binary.parents if (p/'CMakeCache.txt').is_file())
    cache=(build/'CMakeCache.txt').read_text()
    source_root=Path(next(v.split('=',1)[1] for v in cache.splitlines() if v.startswith('CMAKE_HOME_DIRECTORY:INTERNAL='))).resolve()
    source_root=Path(subprocess.check_output(['git','-C',str(source_root),'rev-parse','--show-toplevel'],text=True).strip()).resolve()
    names=set();generated={};count=0
    for target in ('mnm-renderer.dir','mnm-render-commands.dir'):
        deps=list((build/'renderer/CMakeFiles'/target).rglob('*.o.d'));require(deps,'Replay compiler closure missing')
        for dep in deps:
            obj=dep.with_suffix('');require(obj.is_file(),'Missing actual replay object')
            for name in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                p=Path(name).resolve()
                if p.is_relative_to(source_root):
                    relative=p.relative_to(source_root).as_posix()
                    require(p.stat().st_mtime_ns<=obj.stat().st_mtime_ns and sha(p.read_bytes())==reader.digest(relative),
                            'Replay compiled source differs: '+relative)
                    names.add(relative)
                elif p.is_relative_to(build) and p.is_file():generated[str(p.relative_to(build))]=sha(p.read_bytes())
            count+=1
    require({'renderer/commands_main.cpp','renderer/blit.cpp','renderer/command_consumer.cpp','renderer/commands.cpp'}<=names,
            'Replay compiler input coverage incomplete')
    return dict(source_root=str(source_root),build=str(build),objects=count,sources={n:reader.digest(n) for n in sorted(names)},
                generated_inputs=generated,binary_sha256=sha(binary.read_bytes()),compiler=subprocess.check_output(['c++','--version'],text=True).splitlines()[0])


def inspect(reader, stem, binary, ordinal):
    a = reader.json(P+stem+'-acquisition.json')
    comparison=P+stem+'-comparison-'+COMPARISONS[stem]+'.json'
    accepted=reader.json(comparison)
    require(a['success'] and a['sources_stable'] and a['private_wine_stopped']
        and accepted['success'] and accepted['analysis_completed']
        and accepted['acquisition']==dict(record=P+stem+'-acquisition.json',sha256=reader.digest(P+stem+'-acquisition.json'))
        and accepted['case']['proof']['primary_identity_verified']
        and accepted['case']['proof']['normal_lifetime_verified']
        and accepted['case']['source_domain']['verified'], 'Unadmitted historical clipping input')
    observed = acquisition([reader.read_bytes(v['record']) for v in a['artifacts'] if '/capture/' in v['path']], a['session_identity'])
    raw = observed['commands']; header, rows, routing = cp.routes(reader, a)
    owner = next(r[3] for r in rows if r[15] == 2)
    clips, scope = decode_owner(artifact(reader, a, 'native-copy-clip-regions-v1.bin'), rows, owner)
    fills = [(i, r) for i, r in enumerate(rows) if r[15] == 3 and r[23] and r[3] == owner]
    queries = bind_fills(clips, rows, owner, header)
    updates = []; lives = {}; physical = {}; targets = {}
    for op, seq, p in commands(raw):
        if op == 1:
            w = struct.unpack_from('<7I', p); lives[w[0]] = [seq, 2**32, w]
        elif op == 7: lives[struct.unpack('<I', p)[0]][1] = seq
        elif op == 2: updates.append((seq, struct.unpack_from('<5I', p)))
    for r in rows:
        if r[40]:
            for obj, sid in ((r[7], r[42]), (r[6], r[43])):
                physical.setdefault(obj, []).append((r[40], sid))
    for (index, row), q in zip(fills, queries):
        lo = max((r[41] for r in rows[:index] if r[41]), default=0)
        hi = next((r[40] for r in rows[index+1:] if r[40]), 2**32)
        pieces = [[max(0, l, q['destination'][0]), max(0, t, q['destination'][1]), min(q['target_description'][0], rr, q['destination'][2]), min(q['target_description'][1], b, q['destination'][3])] for l,t,rr,b in q['regions']]
        pieces = [v for v in pieces if v[0]<v[2] and v[1]<v[3]]
        candidates = []
        for seq, w in updates:
            sid,x,y,width,height = w
            if not lo < seq < hi or [x,y,x+width,y+height] not in pieces: continue
            life = lives[sid]
            if life[2][1:4] != tuple(q['target_description']):continue
            identities = [v for v in physical.get(row[6], []) if v[1] == sid and life[0]<v[0]<life[1]]
            if identities:candidates.append((seq, w))
        require(len(candidates)==1, 'Ambiguous/missing clipped fill UPDATE '+str((index, lo, hi, candidates)))
        seq,w = candidates[0];targets[seq] = dict(caller=list(row), query=q, surface=w[0], lower=lo, upper=hi, created=lives[w[0]][0], deleted=lives[w[0]][1])
    class Worlds:
        def __contains__(self, g):return True
        def __getitem__(self, g):return np.zeros(model.handles[world_sid]['pixels'].shape, dtype='uint32')
    model = History(Worlds()); known = {}; snapshots = {}; results = []; selections = []; reads = {}; world_sid = 0
    captured = {h['sequence']:b for h,b in observed['command_rows'] if h['kind']=='cpu-read'}
    paints = {}
    for ordinal in observed['swaps']:
        h,b = observed['paints'][ordinal]
        if h.get('mode') == 0:paints.setdefault(h['commands'], []).append((h,b))
    for op,seq,p in commands(raw):
        if op == 1:
            w = struct.unpack_from('<7I',p);known[w[0]] = np.ones((w[2],w[1]), dtype=bool)
        elif op in (2,20):
            sid,x,y,w,h = struct.unpack_from('<5I',p)
            if seq in targets:
                q = targets[seq]['query'];before = model.handles[sid]['pixels'].copy()
                require(known[sid].all(), 'Unknown pre-fill history at '+str(seq))
                actual = q['actual']
                expected, mask = fill_output(before, q, actual, p)
                result=dict(sequence=seq,caller=targets[seq]['caller'],query=q,
                    created=targets[seq]['created'],deleted=targets[seq]['deleted'],
                    lower_fence=targets[seq]['lower'],upper_fence=targets[seq]['upper'],surface=sid,
                    changed_words=int(np.count_nonzero(expected!=before)),
                    exterior_words=int((~mask).sum()),nonzero_exterior_words=int(np.count_nonzero(before[~mask])),
                    prior_sha256=sha(before.astype('<u2').tobytes()),expected_sha256=sha(expected.astype('<u2').tobytes()),
                    observed_value=actual['fill_value'],prior_gpu_reply_sequence=reads.get(sid),
                    prestate_fully_known=True,original_driver_equivalence=False)
                if result['changed_words']:
                    result['native_replay']=native_replay(reader,binary,before,p,expected,mask,stem)
                snapshots[sid]=(expected,result)
                selections.append(result)
            mask = np.ones((h,w),dtype=bool) if op==2 else np.frombuffer(p[20+w*h*(model.handles[sid]['bits']//8):],dtype='u1').reshape(h,w).astype(bool)
            known[sid][y:y+h,x:x+w][mask] = True
        elif op==3:
            w=struct.unpack('<10I',p);l,t,r,b,x,y=w[2:8];source=known[w[0]][t:b,l:r].copy();target=known[w[1]][y:y+b-t,x:x+r-l]
            if w[8]:
                active=model.handles[w[0]]['pixels'][t:b,l:r]!=w[9];target[active]=source[active];target[~source]=False
            else:target[:]=source
        elif op==19:
            world_sid=struct.unpack_from('<I',p)[0];known[world_sid][:]=False
        elif op==7:known.pop(struct.unpack('<I',p)[0])
        model.apply(op,seq,p)
        if op==21:
            w=list(struct.unpack('<10I',p));body=captured[seq];require(body[:64]==reply_header(w), 'GPU prior reply identity')
            model.handles[w[0]]['pixels'][:]=np.frombuffer(body[64:],dtype='<u2').reshape(w[5],w[4]);known[w[0]][:]=True;reads[w[0]]=seq
        if op==6:
            sid=struct.unpack('<I',p)[0]
            if sid in snapshots:
                expected,result=snapshots.pop(sid)
                require(np.array_equal(model.handles[sid]['pixels'], expected), 'Intervening target mutation before PRESENT')
                for h,b in paints.get(seq,[]):
                    actual=b() if callable(b) else b
                    require(actual==viewport(rgba(expected,16,(0xf800,0x7e0,31),None),h), 'Complete presented clipped fill differs')
                    result.update(paint=int(h['paint']),present_sequence=seq,viewport_pixels=h['width']*h['height'],prior_gpu_reply_sequence=reads.get(sid))
                    results.append(result)
    positive=[s for s in selections if s['changed_words']>0]
    require(len(selections)==2 and len(positive)==1 and positive[0]['native_replay']['success'], 'Missing meaningful clipped-fill domain')
    case=dict(stem=stem,success=True,domain=['player-software' if a['configuration']['software_flag'] else 'player-default',a['configuration']['actual_size']],identity=a['session_identity'],acquisition=dict(record=P+stem+'-acquisition.json',sha256=reader.digest(P+stem+'-acquisition.json')),comparison=dict(record=comparison,sha256=reader.digest(comparison)),source_phase=a['sources'],observed_fills=len(selections),no_op_fills=1,witness=positive[0],all_fills=selections,presentations=[v for v in results if v['changed_words']>0],region_scope=scope)
    check_case(case)
    return case

def main():
    global RUN
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--native-replay',type=Path,required=True);parser.add_argument('--report',required=True)
    args=parser.parse_args();require(args.report.startswith(OUT) and not (root/args.report).exists(),'Fresh owned clipped-fill result required')
    RUN=Path(args.report).stem
    result=dict(schema=1,kind='retained_clipped_fill_native_replay_analysis',success=False,cases=[],original_game_runs=0,
                current_native_engine_execution=False,current_native_player_cases=0,new_game_fixture=False)
    with Reader(root) as reader:
        reg=reader.json('research/runtime/coverage/register.json');b=next(b for b in reg['behaviors'] if b['id']==BID)
        paths=required_sources(b);before={n:reader.digest(n) for n in sorted(paths)}
        result['sources']=before
        result['claims']=[dict(behavior=BID,contract_sha256=behavior_contract(b,{}),scenarios={s['id']:scenario_contract(s) for s in reg['scenarios'] if BID in s['behaviors']})]
        try:
            compiler=compiler_sources(reader,args.native_replay)
            require(set(compiler['sources'])<=paths,'Replay compiler inputs missing from contract dependencies')
            result['compiler']=compiler
            name=OUT+Path(args.report).stem+'-native-replayer.bin';result['binary']=dict(record=name,sha256=write_report(root,name,args.native_replay.read_bytes()))
            for i,stem in enumerate(('default-800-07','default-640-11','software-800-12','software-640-14')):
                case=inspect(reader,stem,args.native_replay,i);result['cases'].append(case);print('Verified',stem,case['witness']['changed_words'],flush=True)
            require({(c['domain'][0],tuple(c['domain'][1])) for c in result['cases']}=={(p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600))},'Four actual profile/World domains required')
            result.update(success=True,current_native_replay_cases=4,historical_positive_presentations=sum(len(c['presentations']) for c in result['cases']),
                          source_phases_preserved=True,scope='Four actual owner-thread clipped constant-fill caller/query/output inputs. Fresh existing native command replay validates complete GPU planes; only two Low historical meaningful paints are witnessed. High intermediate paints remain coalesced. Prior native GPU reply and intervening recorded uploads are transform inputs, not independent original prior-history proof. No physical memory-store, partial-prefix failure, arbitrary format, full original ABI/driver or whole-player claim.')
        except Exception as e:result['error']=str(e)
        result['sources_stable']=all(reader.digest(n)==h for n,h in before.items());result['success'] &= result['sources_stable']
    digest=write_report(root,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),sha256=digest)))
    return int(not result['success'])

if __name__=='__main__':raise SystemExit(main())
