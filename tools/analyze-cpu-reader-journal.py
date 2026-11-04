#!/usr/bin/env python3
"""Join one diagnostic reader/DC journal to all retained CPU requests/replies."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import sqlite3
import struct
import sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader,write_report
from cpu_reader_journal import decode,chains,pe_identity,resolve_callers,wire_reads
from rendering_pilot import require
from coverage_claims import required_sources,behavior_contract,scenario_contract

def expected_world_extent(acquisition, size=None):
    """Keep the historical default; explicit profiles must match admitted mode."""
    if size is None:return (640,480)
    profiles={'640x480':(1,(640,480)),'800x600':(0,(800,600))}
    require(size in profiles,'Unsupported diagnostic World size')
    initial=acquisition.get('flow',{}).get('initial_resolution')
    mode,extent=profiles[size]
    require(type(initial) is int and initial==mode,'Explicit World size differs from acquisition initial mode')
    return extent

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True)
    p.add_argument('--retained-context',help='Earlier analysis record: reuse its pinned module candidates and charged-attempt receipt without live files')
    p.add_argument('--control',action='append',help='Current immutable journal/DC control report; defaults to the original retained pair')
    p.add_argument('--loader-control',help='Current successful PE32 Wine loader control enabling the single ImageBase candidate rule')
    p.add_argument('--world-size',choices=['640x480','800x600'],help='Explicit World extent bound to the acquisition initial mode; omitted preserves the historical640x480 default')
    a=p.parse_args()
    reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behavior=next(b for b in reg['behaviors'] if b['id']=='NP.cpu-reader-dc-journal')
    scenario=next(s for s in reg['scenarios'] if s['id']=='SC.cpu-reader-dc-journal')
    names=sorted(required_sources(behavior))
    source={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names}
    result=dict(schema=1,success=False,scope=f'Single software {a.world_size or "640x480"} enabled-intro normal-storage diagnostic. Full CPU reply identity/extent and observer metadata joins; no new pixel oracle, nominal acceptance, original upstream caller recovery or replacement.',sources=source,acquisition=a.acquisition,nominal_acceptance=False,pixel_equivalence_asserted=False,missing_links=[])
    result['claims']=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{b['id']:b for b in reg['builds']}),scenarios={scenario['id']:scenario_contract(scenario)})]
    with Reader(ROOT) as d:
        acq=d.json(a.acquisition);result['acquisition_sha256']=hashlib.sha256(d.read_bytes(a.acquisition)).hexdigest()
        context=d.json(a.retained_context) if a.retained_context else None
        if context is not None:
            require(context['acquisition']==a.acquisition and context['acquisition_sha256']==result['acquisition_sha256']
                    and context['charged_attempts']==1,'Retained journal context differs')
            result['retained_context']=dict(record=a.retained_context,sha256=d.digest(a.retained_context))
        def artifact(predicate):
            items=[x for x in acq['artifacts'] if predicate(x['path'])]
            require(len(items)==1,'Missing/ambiguous diagnostic artifact');v=items[0];raw=d.read_bytes(v['record'])
            require(len(raw)==v['bytes'] and hashlib.sha256(raw).hexdigest()==v['sha256'],'Diagnostic artifact changed')
            return raw,v
        try:
            require(acq['kind']=='public_native_cpu_reader_diagnostic_acquisition' and acq['scenario']=='normal-storage'
                    and acq['configuration']['software_flag'] is True and acq['configuration']['skip_intros'] is False
                    and acq['configuration']['diagnostic_only'] is True and acq['configuration']['deadline_ms']==530000
                    and acq['configuration']['movie_budget_ms']==180000,'Wrong diagnostic route')
            require(acq['cpu_reader_attempt_charged'] is True,'Uncharged diagnostic attempt')
            expected=expected_world_extent(acq,a.world_size)
            result['world_size_policy']=dict(expected=list(expected),source='explicit acquisition initial mode' if a.world_size else 'historical640x480 default')
            if context is None:
                budget=ROOT/'working/cpu-reader-journal/attempt-budget.sqlite3'
                with sqlite3.connect(f'file:{budget}?mode=ro',uri=True) as db:
                    attempts=db.execute('SELECT report FROM attempts').fetchall()
                require(attempts==[(a.acquisition,)],'Diagnostic attempt budget differs')
            result['charged_attempts']=1
            require(acq['success'] and acq['sources_stable'] and acq['compiler_closure_stable'] and acq['player_binary_stable']
                    and acq['private_wine_stopped'] and acq['original_manifest_verified_before_after'],'Incomplete public diagnostic acquisition')
            result['controls']=[]
            controls=a.control or ['research/runtime/cpu-reader-journal-results-20261102/'+n+'.json' for n in ('controls04','dc-regression04')]
            for record in controls:
                control=d.json(record);require(control['success'],'Journal/DC controls failed')
                require(all(d.digest(n)==h for n,h in control['sources'].items()),'Journal/DC control sources changed')
                result['controls'].append(dict(record=record,sha256=d.digest(record)))
                source.update(control['sources'])
            if a.loader_control:
                control=d.json(a.loader_control)
                require(control.get('kind')=='pe32_loader_header_control' and control['success']
                        and control['sources_stable'] and control['only_image_base_changed']
                        and control['relocated_pointer_value_verified'],'Loader header control refused')
                require(all(d.digest(n)==h for n,h in control['sources'].items()),'Loader control sources changed')
                source.update(control['sources'])
                result['loader_control']=dict(record=a.loader_control,sha256=d.digest(a.loader_control),wine_version=control['wine_version'])
            source.update(acq['sources'])
            source.update(acq['native_admission']['native_only']['sources'])
            require(all(d.digest(n)==h for n,h in source.items()),'Acquisition compiled sources changed')
            config=acq['configuration'];result['configuration']={k:config[k] for k in ('software_flag','skip_intros','diagnostic_only','deadline_ms','movie_budget_ms')}
            command,command_ref=artifact(lambda n:'/capture/commands-' in n and n.endswith('.bin'))
            count,count_ref=artifact(lambda n:n.endswith('/native-cpu-read-counts.bin'))
            terminal,terminal_ref=artifact(lambda n:n.endswith('/render-commands.bin.cpu-reply'))
            journal,journal_ref=artifact(lambda n:n.endswith('/cpu-reader-journal-v1.bin'))
            requests,counts,stream_sha=wire_reads(command,acq['session_identity'],count,terminal)
            h,rows=decode(journal);observed=chains(h,rows,requests)
            require(all(x['words'][3]==h[11] for x in requests.values()),'Mixed journal/wire sessions')
            # Auxiliary CPU surfaces need not have the World canvas extent.
            # Inspect retained paint metadata only; this is no pixel comparison.
            viewport,viewport_ref=artifact(lambda n:'/capture/viewport-' in n and n.endswith('.bin'))
            require(viewport[:8]==b'MNMACE01','Diagnostic viewport envelope')
            at=8;index=0;world_extents=Counter();last=None
            while at<len(viewport):
                require(len(viewport)-at>=8,'Diagnostic viewport metadata header')
                m,n=struct.unpack_from('<II',viewport,at);at+=8
                require(0<m<=65536 and m+n<=len(viewport)-at,'Diagnostic viewport metadata extent')
                meta=json.loads(viewport[at:at+m]);at+=m+n
                require(meta['index']==index,'Diagnostic viewport metadata order');index+=1;last=meta
                if index==1:require(meta['role']=='viewport' and meta['session']==acq['session_identity'],'Diagnostic viewport session')
                if meta['kind']=='paint' and int(meta.get('world','0')):
                    require(meta['native_gpu_frame'] is True,'World metadata is not a native frame')
                    world_extents[(meta['frame_width'],meta['frame_height'])]+=1
            require(last and last['kind']=='close' and last['complete'] is True and last['error']=='','Diagnostic viewport closure')
            require(set(world_extents)=={expected},'Unexpected observed World extent')
            result['world_paint_extents']=[dict(width=w,height=z,paints=n) for (w,z),n in world_extents.items()]
            result['reader_extents']=[dict(width=w,height=z,requests=n) for (w,z),n in Counter((x['words'][4],x['words'][5]) for x in requests.values()).items()]
            result['viewport_metadata_artifact']=viewport_ref
            classes=Counter(x['reader_kind'] for x in observed['requests'])
            require([classes[i] for i in range(4)]==list(counts['kinds'].values()),'Per-request reader class/count differs')
            modules=[];module_bytes={}
            for entry in acq['artifacts']:
                if not entry['path'].lower().endswith('.dll'):continue
                raw=d.read_bytes(entry['record']);require(hashlib.sha256(raw).hexdigest()==entry['sha256'],'Pinned module differs')
                modules.append(dict(name=Path(entry['path']).name,identity=pe_identity(raw),sha256=entry['sha256'],record=entry['record']))
                module_bytes[entry['sha256']]=raw
            # This private executable is a candidate, never assumed to match a PC.
            prior={m['name']:m for m in context['pinned_modules']} if context else {}
            exe=d.read_bytes(prior['Chaos.exe']['record']) if context else (ROOT/'working/game-nocd/Chaos.exe').read_bytes()
            eh=hashlib.sha256(exe).hexdigest()
            require(eh=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Executable candidate differs')
            er=prior['Chaos.exe']['record'] if context else str(Path(a.report).with_suffix(''))+'-executable.bin'
            if context is None:write_report(ROOT,er,exe)
            modules.append(dict(name='Chaos.exe',identity=pe_identity(exe),sha256=eh,record=er))
            module_bytes[eh]=exe
            # The launcher adds verified imports to its private executable. Its
            # PE headers can therefore differ from the unmodified fixture's.
            staged=Path(acq['session'])/'game/Chaos.exe'
            if context:
                sr=prior['staged Chaos.exe']['record'];se=d.read_bytes(sr);sh=hashlib.sha256(se).hexdigest()
                require(sh==prior['staged Chaos.exe']['sha256'],'Retained staged executable differs')
            else:
                require(staged.resolve().is_relative_to((ROOT/'working/experiments/menu-observer').resolve())
                        and staged.is_file() and not staged.is_symlink(),'Staged executable candidate escaped its owned session')
                se=staged.read_bytes();sr=str(Path(a.report).with_suffix(''))+'-staged-executable.bin';sh=write_report(ROOT,sr,se)
            modules.append(dict(name='staged Chaos.exe',identity=pe_identity(se),sha256=sh,record=sr,observed_path=str(staged),mapped_code_hash_verified=False))
            module_bytes[sh]=se
            callers=resolve_callers(observed.pop('context_callers'),modules,module_bytes=module_bytes if a.loader_control else None)
            missing=observed['missing_links']
            for c in callers:
                if not c['module_resolved']:missing.append(dict(kind='caller_binary_unresolved',context=c['context'],candidates=c.get('candidate_binary_count',0)))
            observed['complete_observed_chain']=not missing
            result.update(success=True,journal=dict(record=journal_ref,records=len(rows),capacity=h[5],drops=h[8],error_bits=h[9],normal_closed=True),
                input_artifacts=[command_ref,count_ref,terminal_ref,journal_ref],command_stream_sha256=stream_sha,
                wire_requests=requests,completion_counts=counts,observed=observed,callers=callers,
                caller_groups=[dict(module=m,rva=r,reason=k,contexts=n) for (m,r,k),n in Counter((c.get('module','unresolved'),c['rva'],c['reason']) for c in callers).items()],
                pinned_modules=modules,missing_links=missing,complete_observed_chain=not missing,
                full_reply_payload_bytes=sum(x['payload_bytes'] for x in requests.values()))
        except Exception as e:
            result.update(error=str(e),missing_links=[dict(kind='diagnostic_join_refused',reason=str(e))],complete_observed_chain=False)
    result['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in source.items());result['success'] &= result['sources_stable']
    digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ('success','error','charged_attempts','complete_observed_chain','missing_links')}|dict(record=a.report,sha256=digest)),flush=True)
    return 0 if result['success'] else 1

if __name__=='__main__':raise SystemExit(main())
