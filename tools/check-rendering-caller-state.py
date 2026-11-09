#!/usr/bin/env python3
"""Compare actual caller-state publication and complete independent viewport history."""
import argparse
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import struct
import subprocess
import tarfile
import zlib
import numpy as np
from comparison_process import Processes
from coverage_evidence import Reader,write_report
from drawing_caller_state import decode,decode_bindings,decode_tail
from rendering_caller_state_join import profile_operations,join_native,retained_census,validate_model_results,caller_inventory,join_retained_sources
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition,compare,inflate,require
from resident_capture import records

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/caller-state-installed-results-20261108/'
COMPONENT='research/runtime/caller-state-results-20261108/'

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True)
    p.add_argument('--requirements',default=COMPONENT+'world-requirements-09.json')
    p.add_argument('--model-build',default=COMPONENT+'model-replay-build-03.json')
    p.add_argument('--history-build',default=COMPONENT+'history-service-build-05.json')
    p.add_argument('--claims',type=Path,help='Prospective source/behavior/scenario declaration, required for nominal comparisons')
    p.add_argument('--diagnostic-unresolved-parents',action='store_true',help='Supplied fast-tail parent for offline pipeline diagnostics only; always withhold success/acceptance')
    args=p.parse_args()
    require(args.diagnostic_unresolved_parents or args.claims is not None,'Nominal comparison requires prospective claims')
    require(args.report.startswith(PREFIX)and not(ROOT/args.report).exists(),'Fresh owned comparison required')
    out=ROOT/'working/caller-state-comparisons'/Path(args.report).stem;out.mkdir(parents=True,exist_ok=False)
    (out/'.retain').touch();processes=Processes();before=False;temporary=set()
    result=dict(schema=1,kind='public_native_caller_state_comparison',success=False,
        original_pixels_used_as_native_inputs=False,commands=[],
        scope='Actual primitive publication against separately original-compared models, exact source identity and independent complete viewport history. This component alone does not promote the entire row or original semantic milestones.')
    if args.diagnostic_unresolved_parents:
        result.update(kind='public_native_caller_state_diagnostic_comparison',newly_passed_requirements=0,
            scope='Offline pipeline diagnostic with explicitly supplied unresolved fast-tail parents. Raw observations remain unchanged. Independent pixels and model results may be inspected; no caller-state or operation acceptance is permitted.')
    with Reader(ROOT)as reader:
        a=reader.json(args.acquisition);result['acquisition']=dict(record=args.acquisition,sha256=reader.digest(args.acquisition))
        plan=reader.json(a['prospective_plan']['record']);result['prospective_plan']=a['prospective_plan']
        build=reader.json(args.model_build);requirements=reader.json(args.requirements)
        names={'tools/check-rendering-caller-state.py','tools/rendering_caller_state_join.py',
            'tools/drawing_caller_state.py','tools/resident_capture.py','tools/inspect-canvas-producers.py',
            'tools/rendering_pilot.py','tools/rendering_capture.py','tools/rendering_normal_acceptance.py',
            'tools/rendering_menu_acceptance.py','tools/retained_artifacts.py','tools/comparison_process.py',
            'tests/caller-state-history.cpp'}
        sources=a['sources']|a.get('native_admission',{}).get('native_only',{}).get('sources',{})|plan['oracle']['native_sources']|build['sources']|requirements['sources']
        sources.update({n:reader.digest(n)for n in names});result['sources']=sources
        if args.claims is not None:
            declaration=json.loads(args.claims.read_text())
            require(all(n not in sources or sources[n]==h for n,h in declaration['sources'].items()),'Claims overwrite an actual execution source phase')
            require(all(reader.digest(n)==h for n,h in declaration['sources'].items()),'Prospective comparison source changed')
            sources.update(declaration['sources']);result['claims']=declaration['claims']
        # Pin unique analysis source bytes before any oracle/model execution.
        # Acquisition sources already have exact prospective snapshots; plain
        # unchanged Git sources retain their immutable tree identity.
        result['source_commit']=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
        result['source_preimages']={};members=[];buffer=io.BytesIO()
        with tarfile.open(fileobj=buffer,mode='w:gz',compresslevel=1)as archive:
            for name,digest in sources.items():
                old=plan.get('source_preimages',{}).get(name)
                if old and old['sha256']==digest:
                    require(reader.digest(old['record'])==digest,'Lost prospective execution source')
                    result['source_preimages'][name]=old;continue
                git=subprocess.run(['git','show',result['source_commit']+':'+name],cwd=ROOT,capture_output=True)
                if not git.returncode and hashlib.sha256(git.stdout).hexdigest()==digest:continue
                raw=(ROOT/name).read_bytes();require(hashlib.sha256(raw).hexdigest()==digest,'Analysis source changed before execution snapshot')
                info=tarfile.TarInfo(name);info.size=len(raw);archive.addfile(info,io.BytesIO(raw))
                members.append(dict(path=name,sha256=digest,bytes=len(raw)))
        record=args.report.removesuffix('.json')+'-analysis-sources.bin'
        result['analysis_source_bundle']=dict(record=record,sha256=write_report(ROOT,record,buffer.getvalue()),members=members)
        def execute(label,argv):
            result['commands'].append(dict(label=label,argv=list(map(str,argv))))
            with(out/(label+'.log')).open('xb')as log:processes.run(list(map(str,argv)),cwd=ROOT,stdout=log,timeout=600)
        try:
            require(a['success']and a['sources_stable']and a['private_wine_stopped']
                and a['original_manifest_verified_before_after']and a['original_game_runs']==1
                and a['prefix_pool']['returned']and a['exit_code']==0
                and not a.get('forced_private_prefix_stop',False),'Incomplete normal acquisition')
            drift={n:dict(recorded=h,current=reader.digest(n))for n,h in sources.items()if reader.digest(n)!=h}
            if args.diagnostic_unresolved_parents:
                require(set(drift)<={'runtime/scene/drawing_caller_state.h'},'Unreviewed diagnostic source drift')
                for n,values in drift.items():
                    snapshot=plan['source_preimages'][n]
                    require(reader.digest(snapshot['record'])==snapshot['sha256']==values['recorded'],'Lost exact diagnostic execution source')
                result['diagnostic_execution_source_drift']=drift
            else:require(not drift,'Changed execution/model/oracle source phase')
            require(all(a['sources'].get(n)==h for n,h in plan['runtime_sources'].items()),'Unfrozen runtime acquisition')
            requirement_drift={n for n,h in requirements['sources'].items()if reader.digest(n)!=h}
            require(build['success']and requirements['success']and (not requirement_drift or args.diagnostic_unresolved_parents and requirement_drift<=set(drift)),'Stale model build or caller inventory')
            result['requirements']=dict(record=args.requirements,sha256=reader.digest(args.requirements))
            result['caller_inventory']=caller_inventory(requirements)
            result['model_build']=dict(record=args.model_build,sha256=reader.digest(args.model_build))
            model=Path(build['physical_binary'])
            require(hashlib.sha256(model.read_bytes()).hexdigest()==build['binary']['sha256'],'Model executable changed')
            history_build_name=args.history_build
            history_build=reader.json(history_build_name)
            history_binary=history_build['binaries']['mnm-caller-state-history']
            require(history_build['success']and reader.digest(history_binary['path'])==history_binary['sha256']
                ==hashlib.sha256((ROOT/'working/build/qt-shell/mnm-caller-state-history').read_bytes()).hexdigest(),
                'History checker executable changed')
            result['history_build']=dict(record=history_build_name,sha256=reader.digest(history_build_name))
            for b in[a['player_binary'],*plan['oracle']['binaries']]:
                require(reader.digest(b['record'])==b['sha256']==hashlib.sha256((ROOT/b['path']).read_bytes()).hexdigest(),'Changed pinned executable')
            streams=[artifact(reader,a,Path(v['path']).name)for v in a['artifacts']if'/capture/'in v['path']and v['path'].endswith('.bin')]
            observed=acquisition(streams,a['session_identity']);del streams
            result['observations']=observed['observations'];result['captured_command_stream_sha256']=hashlib.sha256(observed['commands']).hexdigest()
            from rendering_normal_acceptance import world_domain,lighting
            result['world_domain']=world_domain(observed)
            result['lighting']=lighting(ROOT,a,{v['path']:v for v in a['artifacts']})
            spec=importlib.util.spec_from_file_location('caller_cps',ROOT/'tools/inspect-canvas-producers.py')
            cps=importlib.util.module_from_spec(spec);spec.loader.exec_module(cps)
            state_raw=artifact(reader,a,'caller-state-0000.bin');state=decode(state_raw)
            require(state['version']==5,'Actual V5 source-completion identity required')
            bindings=decode_bindings(artifact(reader,a,'caller-bindings-0000.bin'),state)
            tail=decode_tail(artifact(reader,a,'caller-source-tail-0000.bin'),cps.decode)
            source=Path(a['temporary_root'])/'worlds.bin'
            require(hashlib.sha256(source.read_bytes()).hexdigest()==hashlib.sha256(artifact(reader,a,'worlds.bin')).hexdigest(),'Physical source capture changed')
            blocks=list(records(source));worlds_count=sum(not c for g,c,s,q in blocks);chunks=sum(bool(c)for g,c,s,q in blocks)
            require(tail['generation']==worlds_count+1 and tail['source']==chunks+1,'Final source prefix identity differs')
            if tail['records']:blocks.append((tail['generation'],tail['source'],tail['stream'],b''))
            operations=profile_operations(blocks,cps.decode)
            model_rows=state['rows']
            if args.diagnostic_unresolved_parents:
                supplied=[];model_rows=[]
                for row in state['rows']:
                    if row['kind']==1 and row['tag']==20 and row['caller']==0x57e9c7 and row['enclosing_caller']not in(0x500566,0x500a8c,0x500e94):
                        supplied.append(dict(id=row['id'],raw_parent=row['enclosing_caller'],supplied_parent=0x500566,observed=False))
                        row=dict(row,enclosing_caller=0x500566)
                    model_rows.append(row)
                require(supplied,'No unresolved parent for this diagnostic')
                result['supplied_diagnostic_contexts']=supplied
            joined=join_native(model_rows,operations,requirements,{r['id']:r for r in bindings['rows']},tail['nonce'])
            result['retained_census']=retained_census(state['rows'])
            result['retained_source_join']=join_retained_sources(state['rows'],operations,joined['used'],result['retained_census'],tail['nonce'])
            # Re-encode verified source bytes only. This derived transport appends
            # the final quiescent prefix; it supplies no destination pixels.
            profile=out/'complete-source.bin'
            with profile.open('xb')as f:
                f.write(b'MNMPROF3')
                for g,c,s,q in blocks:
                    body=s+q;packed=struct.pack('>I',len(body))+zlib.compress(body,1)
                    f.write(struct.pack('<QQIIII',g,c,len(s),len(q),len(packed),0));f.write(packed)
            temporary.add(profile)
            result['complete_source']=dict(sha256=hashlib.sha256(profile.read_bytes()).hexdigest(),blocks=len(blocks),worlds=worlds_count,source_chunks=chunks,final_tail_records=tail['records'],native_destination_inputs=False)
            assets=Path(a['session'])/'game';extensions={'.spr','.jpg','.jpeg','.bmp','.pcx','.sft','.fp','.ttd','.wav'}
            asset_inputs={str(f.relative_to(assets)):hashlib.sha256(f.read_bytes()).hexdigest()for f in sorted(assets.rglob('*'))if f.is_file()and f.suffix.lower()in extensions}
            require(asset_inputs and all(a['installation_inputs'][n]['sha256']==h for n,h in asset_inputs.items()),'Changed reference assets')
            result['asset_inputs']=asset_inputs
            execute('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
            pe=ROOT/'original/Arcane_Nocd/Chaos.exe'
            require(hashlib.sha256(pe.read_bytes()).hexdigest()=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Original build differs')
            reference=out/'reference'
            execute('original-history',[ROOT/'working/build/qt-shell/mnm-acceptance-world-history',profile,assets,ROOT/'working/original-reference',pe,reference])
            index=json.loads((reference/'worlds.json').read_text())
            require(index['success']and index['native_player_pixels_read']is False and index['original_destination_capture_read']is False,'Dependent pixel oracle')
            worlds={}
            for row in index['worlds']:
                g=int(row['generation']);path=reference/row['path'];require(g==len(worlds)+1 and path.parent==reference,'Original World order/path')
                raw=inflate(path.read_bytes());require(len(raw)==row['width']*row['height']*2,'Incomplete original plane')
                worlds[g]=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width']);temporary.add(path)
            result['original_reference']=dict(worlds=len(worlds),source_chunks=int(index.get('source_chunks','0')),draws=index['draws'],native_player_pixels_read=False,original_destination_capture_read=False)
            result['comparison']=compare(observed,worlds)
            require(result['comparison']['success']and result['comparison']['mismatching_pixels']==0,'Independent complete viewport mismatch')
            cases=out/'joined-cases.bin'
            raw_rows={struct.unpack_from('<I',state_raw,at)[0]:state_raw[at:at+848]for at in range(64,len(state_raw)-848,848)}
            with cases.open('xb')as f:
                f.write(b'MNMCSJ01'+struct.pack('<II',1,len(joined['joined'])))
                for j in joined['joined']:
                    g,s,seq=j['key'];r=j['row'];o=j['obligations']
                    f.write(struct.pack('<QQ6I',g,s,seq,r['id'],o['eax'],o['ecx'],o['edx'],o['eflags']));f.write(raw_rows[r['id']])
            execute('state-history',[ROOT/'working/build/qt-shell/mnm-caller-state-history',profile,assets,reference,cases,model,pe,out/'state-history'])
            summary=json.loads((out/'state-history/summary.json').read_text())
            result['caller_state']=validate_model_results((out/'state-history/model-results.bin').read_bytes(),joined['joined'])
            require(summary['success']and summary['cases']==len(joined['joined'])and not summary['native_destination_inputs'],'Incomplete state model history')
            result['caller_state'].update(noops=joined['noops'],history=summary,observed=state['completed'],normal_retirement=True,
                conditional_callee_cases=sum(j['obligations']['conditional_callees']for j in joined['joined']),
                semantic_original_comparison_required_separately=True)
            require(all(hashlib.sha256((assets/n).read_bytes()).hexdigest()==h for n,h in asset_inputs.items()),'Reference assets changed during analysis')
            result['success']=True
        except Exception as error:result['error']=str(error)
        finally:
            if before:
                try:execute('original-after',[ROOT/'tools/original-manifest.sh','verify']);result['original_manifest_verified_before_after']=True
                except Exception as error:result.update(success=False,original_verification_error=str(error))
            result['sources_stable']=all(reader.digest(n)==h for n,h in sources.items());result['success'] &= result['sources_stable']and not args.diagnostic_unresolved_parents
            # Raw acquisitions remain separately immutable. Exclude only
            # reproducible transport and original planes, never diagnostics.
            members=[];bundle=out/'outputs.tar.gz'
            with tarfile.open(bundle,'w:gz',compresslevel=1)as archive:
                for f in sorted(out.rglob('*')):
                    if not f.is_file()or f in temporary or f==bundle:continue
                    raw=f.read_bytes();name=f.relative_to(out).as_posix()
                    members.append(dict(path=name,bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest()));archive.add(f,arcname=name,recursive=False)
            record=args.report.removesuffix('.json')+'-outputs.bin';raw=bundle.read_bytes()
            result['artifacts']=[dict(path=bundle.name,record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw),encoding='tar-gzip',members=members)]
            result['comparison_processes_stopped']=processes.stopped();result['success'] &= result['comparison_processes_stopped']
            digest=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
            print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=digest,caller_state=result.get('caller_state',{}).get('cases'))),flush=True)
    return int(not result['success'])

if __name__=='__main__':raise SystemExit(main())
