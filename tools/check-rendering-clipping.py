#!/usr/bin/env python3
"""Retain bounded clipped COPY subproofs without promoting incomplete rows."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import numpy as np
from coverage_evidence import Reader,write_report
from evidence_objects import configured
from rendering_capture import capture
from rendering_menu_acceptance import artifact
from rendering_pilot import commands,require
from native_copy_routes import decode,validate as validate_routes
from native_gpu_copy_diagnostics import check_pixels
import native_copy_clip_regions as clip_regions
from native_copy_clip_regions_scoped import decode_owner

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-clipped-operations-results-20261107/'
ACQUISITION=PREFIX+'default-800-06-acquisition.json'


def module(name):
    spec=importlib.util.spec_from_file_location('clip_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


identity=module('close-rendering-identity-gates')
retained=module('close-rendering-retained-rows')


terminal_state_v2=retained.terminal_state


def row_ready(proof):
    """Reduced output alone cannot establish the original clip-region contract."""
    return all(proof.get(k) is True for k in ('primary_identity_verified','normal_lifetime_verified',
        'positive_complete_pixel_witnesses','actual_clip_region_verified',
        'required_domains_verified','caller_clip_plan_verified'))


def bind_regions(observations, rows, native_copies):
    """Ordered provider inputs -> independent plan -> actual caller/C1 output."""
    queries=[o for o in observations if o['phase'] in (1,2)]
    callers=[r for r in rows if r[15] in (1,2) and r[23]]
    require(len(queries)==len(callers),'Clip query/caller cardinality differs')
    bound={}
    for q,r in zip(queries,callers):
        require(q['phase']==r[15] and q['thread']==r[3]
                and (q['target_alias'],q['source_alias'])==r[4:6]
                # Pinned PE32 NsObject is three DWORDs. Observer words5/6
                # are alias[0] bases; ns_query(IUnknown) returns alias[1].
                # Verify launch-local bases and actual intercepted identities.
                and (q['target_canonical']+12,q['source_canonical']+12)==r[6:8]
                and q['target_alias']-q['target_canonical'] in (0,12,24,36,48)
                and q['source_alias']-q['source_canonical'] in (0,12,24,36,48)
                and q['destination']==list(map(clip_regions.signed,r[28:32]))
                and q['source']==list(map(clip_regions.signed,r[24:28]))
                and q['target_description']==list(r[19:22]) and q['source_description']==list(r[16:19])
                and r[11]==0 and q['flags']==r[12]&~0x1000000
                and q['clipper'] and q['window'] and q['clipper_references']>0
                and q['target_incarnation']>0 and q['source_incarnation']>0,
                'Clip query differs from ordered original caller identity/arguments')
        pieces,result=clip_regions.plan_regions(q)
        require(q['plan_pieces']==len(pieces) and q['result']==result==r[13],
                'Independent clip input plan/result differs from provider/caller')
        require(not r[10]&128,'Natural alias requires a separate scratch-plan binding')
        emitted=[native_copies[s]['words'] for s in range(r[40],r[41]+1) if s in native_copies] if r[40] else []
        expected=[tuple(r[42:44])+tuple(p)+ (int(q['keyed']),q['key']) for p in pieces]
        require(emitted==expected,'Actual emitted C1 copies differ from independent RGNDATA plan')
        if q['phase']==1:
            actual=[o for o in observations if o['ordinal']>q['ordinal'] and o['phase'] in (1,2,4)]
            require(actual and actual[0]['phase']==4,'CPU clip query lacks immediate actual operation')
            actual=actual[0]
            fields=('thread','target_alias','source_alias','target_canonical','source_canonical',
                    'target_incarnation','source_incarnation','clipper','window','clipper_references',
                    'result','destination','source','target_description','source_description',
                    'keyed','key','rgndata_words')
            require(all(actual[k]==q[k] for k in fields) and actual['flags']&~0x1000000==q['flags'],
                    'CPU actual clip input/result differs from query')
        bound[r[0]]=dict(observation=q,independent_pieces=pieces,result=result,emitted_commands=len(emitted),
                        canonical_IUnknown=list(r[6:8]),provider_alias0_bases=[q['target_canonical'],q['source_canonical']])
    return bound


def inspect_case(reader,name=ACQUISITION,require_regions=False):
    a=reader.json(name);ref=dict(record=name,sha256=reader.digest(name))
    if require_regions:
        plan=reader.json(a['prospective_plan'])
        require(a['kind']=='public_native_clipping_acquisition'
                and all(reader.digest(n)==h for n,h in plan['clip_oracle_sources'].items())
                and all(a['sources'].get(n)==h==reader.digest(n) for n,h in plan['runtime_sources'].items()),
                'Clip oracle/provider source phase differs from frozen acquisition')
    profile='player-software' if a['configuration']['software_flag'] else 'player-default'
    item=dict(reference=ref,profile=profile,rows=[],comparisons=[])
    with configured(ROOT) as pair:
        identified=identity.identify(reader,pair,item,
            '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168',set())
    names=('native-only-session.json','native-output-admission.json','original-process-result.json',
           'render-input.bin','render-commands.bin','render-commands.bin.control',
           'native-cursor-lifetime.bin','native-draw-retirement.bin')
    terminal_raw={n:artifact(reader,a,n) for n in names}
    callbacks=[[v['menu_id'],v['argument']] for v in retained.menu_events.decode(artifact(reader,a,'events.bin')) if v['event']==3]
    terminal_proof=terminal_state_v2(a,*(json.loads(terminal_raw[n]) for n in names[:3]),
                                   *(terminal_raw[n] for n in names[3:]),callbacks)
    terminal_proof['source_terminal']=retained.world_exit.terminal_source(artifact(reader,a,'world-continuous.bin'),
        artifact(reader,a,'world-terminal-0001.bin'),dict(nonce=int(a['resident']['nonce']),imports=range(terminal_proof['source_imports'])),
        terminal_proof['original_thread'])
    terminal=dict(item,kind=a['kind'],proof=terminal_proof,input_wire_version=2,coordinate_space='desktop')
    stream=next(v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('commands'))
    records=capture(reader.read_bytes(stream['record']))
    require(records[0][0]['session']==a['session_identity'],'Mixed command session')
    raw=b''.join(body for h,body in records if h['kind']=='fragment')
    life=retained.world_exit.imported_lifetimes(raw)
    require(8<=len(life['imports'])==int(a['resident']['imports'])<=512
            and records[-2][0]['kind']=='ended' and records[-2][0]['commands']==life['command_end'],
            'Incomplete current pilot command/import lifetime')
    terminal_proof['command_lifetimes']=dict(nonce=life['nonce'],imports=len(life['imports']),deleted_import_incarnations=len(life['imports']))
    retained.check_retirement(terminal)
    diagnostics=json.loads(artifact(reader,a,'copy-diagnostics/report.json'))
    require(diagnostics['ended'] is True and diagnostics['error']==''
            and diagnostics['original_pixels_used_as_native_inputs'] is False
            and (diagnostics['success'] is True or diagnostics['truncated'] is True),
            'Diagnostic execution failed beyond its explicitly retained output bound')
    require(diagnostics['selection'] in ('all-copies','distinct-copy-inputs'),'Unreviewed clipping diagnostic selection')
    copies=[json.loads(line) for line in artifact(reader,a,'copy-diagnostics/copies.jsonl').decode().splitlines()]
    require(len(copies)==diagnostics['copies_checked'] and len({v['sequence'] for v in copies})==len(copies)
            and all(v['mismatches']==0 and v['actual_sha256']==v['expected_sha256'] for v in copies),
            'Incomplete/duplicate/mismatching retained diagnostic records')
    checked={v['sequence']:v for v in copies}
    if diagnostics['selection']=='distinct-copy-inputs':
        require(diagnostics['success'] and not diagnostics['truncated']
                and diagnostics['copies_seen']==len(copies)+sum(diagnostics[k] for k in
                    ('copies_skipped_duplicate','copies_skipped_class_budget','copies_skipped_byte_budget'))
                and diagnostics['selected_input_signatures']==diagnostics['copies_enqueued']==len(copies)
                and diagnostics['retained_pixel_cases']==len(copies)<=32
                and diagnostics['input_limit_per_class']==8
                and diagnostics['selected_plane_bytes']==sum((d['source_width']*d['source_height']+3*d['target_width']*d['target_height'])*4 for d in copies)
                <=diagnostics['retained_plane_byte_limit']==256*1024**2
                and all('files' in d for d in copies), 'Incomplete bounded distinct-input diagnostic')
    with tempfile.TemporaryDirectory(prefix='clipping-review-',dir='/tmp') as tmp:
        tmp=Path(tmp);(tmp/'lock-capture').mkdir()
        source_domain=None
        if require_regions:
            from resident_capture import records as source_records
            decoder=module('inspect-canvas-producers')
            source_path=tmp/'worlds.bin';source_path.write_bytes(artifact(reader,a,'worlds.bin'))
            source_worlds=[];chunks=0
            for generation,chunk,surface,request in source_records(source_path):
                chunks+=bool(chunk)
                if chunk:continue
                descriptors=[f for f,body in decoder.decode(surface,complete=False)[1] if f[2]==11]
                require(len(descriptors)==1 and list(descriptors[0][5:7])==a['configuration']['actual_size'],
                        'Actual captured World descriptor differs from selected size')
                source_worlds.append(generation)
            require(source_worlds==list(range(1,len(life['imports'])+1))
                    and a['resident']['stage_profile']['capture_complete'] is True,
                    'Incomplete captured source World domain')
            source_domain=dict(worlds=len(source_worlds),source_chunks=chunks,actual_size=a['configuration']['actual_size'],verified=True)
        for filename in ('copy-routes-v2.bin','gpu-copy-census-v1.bin'):
            (tmp/'lock-capture'/filename).write_bytes(artifact(reader,a,filename))
        routing=validate_routes(tmp);header,rows=decode(tmp/'lock-capture/copy-routes-v2.bin')
        live={};lives={};native_copies={};world=0;worlds={}
        for op,seq,payload in commands(raw):
            if op==1:
                f=struct.unpack_from('<7I',payload);require(f[0] not in live,'Handle collision')
                live[f[0]]=dict(created=seq,deleted=None,description=list(f));lives[f[0]]=live[f[0]]
            elif op==3:
                w=struct.unpack('<10I',payload)
                require(w[0] in live and w[1] in live,'Copy uses retired native storage')
                native_copies[seq]=dict(words=w,source=live[w[0]],target=live[w[1]],world=world)
            elif op==19:world=struct.unpack_from('<5I',payload)[1];worlds[world]=seq
            elif op==7:live.pop(struct.unpack('<I',payload)[0])['deleted']=seq
        require(not live and len(worlds)==len(life['imports']),'Native storage/import lifetime incomplete')
        from rendering_normal_acceptance import world_domain
        domain=world_domain(dict(commands=raw))
        require(domain['extents']==[a['configuration']['actual_size']], 'Actual World size differs from declared domain')
        region_bindings={};region_scope=None
        if require_regions:
            owner=terminal_proof['original_thread']
            observations,region_scope=decode_owner(artifact(reader,a,'native-copy-clip-regions-v1.bin'),rows,owner)
            region_bindings=bind_regions(observations,[r for r in rows if r[3]==owner],native_copies)
        candidates=[];witnesses=[];excluded_raw=[]
        for row in rows:
            if require_regions and row[3]!=terminal_proof['original_thread']:continue
            if row[15] not in (1,2) or not row[23] or row[13] or not row[40]:continue
            pieces=[checked[s] for s in range(row[40],row[41]+1) if s in checked]
            if len(pieces)!=row[41]-row[40]+1:continue
            area=(row[26]-row[24])*(row[27]-row[25])
            actual=sum((v['words'][4]-v['words'][2])*(v['words'][5]-v['words'][3]) for v in pieces)
            if not 0<actual<area:continue
            candidates.append(dict(caller_row=row[0],route=row[15],first=row[40],last=row[41],
                                   requested_source_words=area,emitted_source_words=actual))
            if len(pieces)!=1 or 'files' not in pieces[0]:continue
            d=pieces[0];seq=d['sequence'];bound=native_copies[seq];w=bound['words']
            if row[11] or row[10]&128 or w[8:]!=(0,0) or row[18]!=16 or row[21]!=16:
                excluded_raw.append(dict(row=row[0],sequence=seq,reason='Outside selected opaque nonalias Blt RGB565 witness scope'));continue
            if any(v['caller'][15]==row[15] for v in witnesses):continue
            if row[15]==2 and (not bound['world'] or life['imports'][bound['world']-1]['surface']!=w[0]):
                excluded_raw.append(dict(row=row[0],sequence=seq,reason='Not the current imported World source'));continue
            require(header[15]<=row[1]<header[15]+header[16] and row[2]==row[1]-header[15]
                    and row[10]&32 and not row[10]&128 and row[11]==0 and row[13]==0
                    and row[14]==1 and row[42:44]==w[:2] and list(w)+[0,0,0]==d['words']
                    and w[8:]==(0,0) and row[18]==row[21]==16,
                    'Retained clipped output lacks actual original caller/resource/key/result binding')
            require(bound['source']['description'][1:4]==list(row[16:19])
                    and bound['target']['description'][1:4]==list(row[19:22])
                    and tuple(bound['source']['description'][3:7])==tuple(bound['target']['description'][3:7])==(16,0xf800,0x7e0,31)
                    and [d['source_width'],d['source_height']]==list(row[16:18])
                    and [d['target_width'],d['target_height']]==list(row[19:21]),
                    'Caller, C1 incarnation and captured complete-plane dimensions/format differ')
            sr=list(map(clip_regions.signed,row[24:28]));dr=list(map(clip_regions.signed,row[28:32]))
            require(sr[2]-sr[0]==dr[2]-dr[0] and sr[3]-sr[1]==dr[3]-dr[1]
                    and sr[0]<=w[2]<w[4]<=sr[2] and sr[1]<=w[3]<w[5]<=sr[3]
                    and w[6]-w[2]==dr[0]-sr[0] and w[7]-w[3]==dr[1]-sr[1],
                    'Reduced COPY does not preserve original caller translation')
            for kind,filename in d['files'].items():
                require(Path(filename).name==filename,'Unsafe retained pixel path')
                (tmp/filename).write_bytes(artifact(reader,a,'copy-diagnostics/'+filename))
            pixels=check_pixels(tmp,d)
            if diagnostics['selection']=='distinct-copy-inputs':
                require(d['source_create_sequence']==bound['source']['created']
                        and d['target_create_sequence']==bound['target']['created'], 'Distinct input reused a native handle lifetime')
            imported=None
            if row[15]==2:
                require(bound['world']>0 and d['source_world_ancestry'] is True,'GPU witness lacks actual World input')
                imported=life['imports'][bound['world']-1]
                following=next((v['import_sequence'] for v in life['imports'] if v['generation']==bound['world']+1),life['command_end'])
                require(imported['surface']==w[0] and imported['created_sequence']==bound['source']['created']
                        and imported['import_sequence']<seq<min(imported['deleted_sequence'],following),
                        'GPU clipping source is not the current live imported World incarnation')
            before=np.frombuffer((tmp/d['files']['before']).read_bytes(),dtype='<u4').reshape(d['target_height'],d['target_width'])
            after=np.frombuffer((tmp/d['files']['actual']).read_bytes(),dtype='<u4').reshape(before.shape)
            source_words=np.frombuffer((tmp/d['files']['source']).read_bytes(),dtype='<u4')
            require(all(int(v.max())<=65535 for v in (before,after,source_words)),
                    'Recorded RGB565 planes contain non-WORD values')
            exterior=np.ones(before.shape,dtype=bool);exterior[w[7]:w[7]+w[5]-w[3],w[6]:w[6]+w[4]-w[2]]=False
            changed=int(np.count_nonzero(before!=after));preserved=int(np.count_nonzero(exterior))
            if changed==0 or preserved==0:
                excluded_raw.append(dict(row=row[0],sequence=seq,reason='No changed WORDs or no exterior'));continue
            require(np.array_equal(before[exterior],after[exterior])
                    and all(v['created']<seq<v['deleted'] for v in (bound['source'],bound['target'])),
                    'Clipped witness is vacuous, overwrites exterior or crosses retirement')
            witnesses.append(dict(caller=list(row),sequence=seq,world_generation=bound['world'],
                command=list(w),source_lifetime=bound['source'],target_lifetime=bound['target'],
                full_destination_pixels=pixels,changed_words=changed,preserved_exterior_words=preserved,
                nonzero_exterior_words=int(np.count_nonzero(before[exterior])),physical_store_trace=False,
                full_input_and_actual_output_verified=True,emitted_destination_rect=[w[6],w[7],w[6]+w[4]-w[2],w[7]+w[5]-w[3]],
                actual_clip_region_snapshot_retained=bool(region_bindings),
                clip_plan=region_bindings.get(row[0]),imported_world=imported,diagnostic=d))
    require(len(witnesses)>=2 and {v['caller'][15] for v in witnesses}=={1,2},'Expected retained CPU/GPU subproof missing')
    proof=dict(primary_identity_verified=True,normal_lifetime_verified=True,positive_complete_pixel_witnesses=True,
        actual_clip_region_verified=bool(region_bindings),caller_clip_plan_verified=bool(region_bindings),required_domains_verified=False)
    return dict(acquisition=ref,profile=profile,actual_size=a['configuration']['actual_size'],identity=identified,
        terminal=terminal,command_lifetimes=dict(imports=len(life['imports']),command_end=life['command_end'],
            captured_commands_sha256=hashlib.sha256(raw).hexdigest()),routing=routing,
        diagnostic=diagnostics,complete_reduced_intervals=len(candidates),raw_pixel_witnesses=witnesses,
        excluded_raw_candidates=excluded_raw,
        world_domain=domain,source_domain=source_domain,region_bound_callers=len(region_bindings),region_scope=region_scope,
        proof=proof,row_ready=row_ready(proof),source_capture=a['resident']['stage_profile']['capture'],
        missing=(['Only this profile/World-size domain is verified; all four are required for row promotion.',
                  'Global all-COPY diagnostic is separate from selected complete output; no whole-run, viewport or physical-driver proof.'] if require_regions else
                ['Actual operation-bound GetClipList/RGNDATA or equivalent independently observed clip-region proof; reduced emitted rectangles alone do not verify clip planning.',
                 'Required software and640domains are absent; only default800was run.',
                 'Whole all-COPY diagnostic failed its existing512MiB queue bound; retain all3006checked and212uncompared copies without whole-run output credit.',
                 'Resident source corpus creation failed because a precreated file symlink conflicts with QFile NewOnly; no original World/source or viewport-equivalence credit.']))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True)
    p.add_argument('--acquisition',default=ACQUISITION);p.add_argument('--require-regions',action='store_true');args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh owned clipping review required')
    with Reader(ROOT) as reader:
        result=dict(schema=1,kind='public_native_clipping_comparison',success=False,analysis_completed=False,
            complete_output_verified=False,positive_clipped_calls=0,acceptance_rows_closed=0,scope=__doc__)
        names={'tools/check-rendering-clipping.py','tools/run-rendering-clipping.py','tools/prepare-rendering-clipping.py',
               'tests/test-rendering-clipping.py','tools/close-rendering-identity-gates.py','tools/close-rendering-retained-rows.py',
               'tools/close-rendering-world-exit.py','tools/coverage_evidence.py','tools/evidence_objects.py',
               'tools/rendering_capture.py','tools/rendering_menu_acceptance.py','tools/rendering_pilot.py',
               'tools/native_copy_routes.py','tools/native_gpu_copy_diagnostics.py','tools/native_copy_clip_regions.py',
               'tools/native_copy_clip_regions_scoped.py','tools/resident_capture.py',
               'tools/inspect-canvas-producers.py','tools/rendering_normal_acceptance.py'}
        result['analysis_sources']={n:reader.digest(n) for n in sorted(names)}
        result['acquisition']=dict(record=args.acquisition,sha256=reader.digest(args.acquisition))
        try:
            case=inspect_case(reader,args.acquisition,args.require_regions);result.update(analysis_completed=True,acquisition=case['acquisition'],case=case,
                positive_clipped_calls=case['complete_reduced_intervals'],refused_acceptance=True,
                stop_on_unaccepted_pilot=True,original_game_runs=0,acquisition_original_game_runs=1,
                new_native_execution=False)
            require(not case['row_ready'],'Single domain clipping proof was promoted')
            if args.require_regions:
                result.update(success=True,complete_output_verified=True,native_clip_regions_verified=True,
                    refused_acceptance=False,stop_on_unaccepted_pilot=False,
                    captured_command_stream_sha256=case['command_lifetimes']['captured_commands_sha256'],
                    output_scope='Selected meaningful complete CPU/GPU planes plus exterior; global C4 truncation is retained without whole-run credit')
        except Exception as error:result['error']=str(error)
        result['analysis_sources_stable']=all(reader.digest(n)==h for n,h in result['analysis_sources'].items())
    h=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(record=args.report,sha256=h,analysis_completed=result['analysis_completed'],
        acceptance_rows_closed=0,positive_clipped_calls=result['positive_clipped_calls'],error=result.get('error'))),flush=True)
    return 0 if result['analysis_completed'] else 1


if __name__=='__main__':raise SystemExit(main())
