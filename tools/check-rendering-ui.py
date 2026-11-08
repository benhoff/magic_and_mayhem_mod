#!/usr/bin/env python3
"""Independently replay original raster/history and check real UI observations."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import tarfile
import tempfile
import numpy as np
from coverage_evidence import Reader,write_report
from comparison_process import Processes
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition,compare,inflate,require
from rendering_ui_acceptance import PREFIX
from rendering_ui_proof import assess
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);p.add_argument('--reuse-pixels');a=p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'Fresh immutable UI comparison required')
    out=Path(tempfile.mkdtemp(prefix='mnm-ui-'+Path(a.report).stem+'-'));(out/'.retain').touch()
    result=dict(schema=1,kind='public_native_ui_comparison',success=False,original_pixels_used_as_native_inputs=False)
    processes=Processes();before=False
    with Reader(ROOT) as r:
        acquired=r.json(a.acquisition);result['acquisition']=dict(record=a.acquisition,sha256=r.digest(a.acquisition));plan=r.json(acquired['prospective_plan'])
        sources=acquired['sources']|acquired.get('native_admission',{}).get('native_only',{}).get('sources',{})|plan['oracle']['native_sources']
        sources.update({n:r.digest(n) for n in ['tools/check-rendering-ui.py','tools/rendering_ui_proof.py','tools/rendering_pilot.py','tools/rendering_capture.py','tools/comparison_process.py','tools/rendering_normal_acceptance.py','tools/rendering_menu_acceptance.py','tools/rendering_interaction.py','tools/close-rendering-retained-rows.py','tools/close-rendering-identity-gates.py','tools/close-rendering-world-exit.py','tools/native_cursor_journal.py','tools/test-menu-observer.py','tools/evidence_objects.py']})
        spec=importlib.util.spec_from_file_location('ui_contract',ROOT/'tools/close-rendering-ui.py');contract=importlib.util.module_from_spec(spec);spec.loader.exec_module(contract)
        required,claims=contract.contracts(r);sources.update(required);result['claims']=claims
        result['sources']=sources
        def execute(label,command):
            with (out/(label+'.log')).open('xb') as log:processes.run([str(v) for v in command],cwd=ROOT,stdout=log,timeout=300)
        try:
            require(acquired['kind']=='public_native_ui_acquisition' and acquired['success'] and acquired['sources_stable']
                    and acquired['private_wine_stopped'] and all(r.digest(n)==h for n,h in sources.items()),'Incomplete/changed UI source phase')
            for b in [acquired['player_binary'],*plan['oracle']['binaries']]:
                require(r.digest(b['record'])==b['sha256']==hashlib.sha256((ROOT/b['path']).read_bytes()).hexdigest(),'Changed accepted executable')
            observed=acquisition([artifact(r,acquired,Path(v['path']).name) for v in acquired['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')],acquired['session_identity'])
            result['observations']=observed['observations'];result['captured_command_stream_sha256']=hashlib.sha256(observed['commands']).hexdigest()
            source=Path(acquired['temporary_root'])/'worlds.bin';require(source.read_bytes()==artifact(r,acquired,'worlds.bin'),'Changed physical source history')
            execute('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
            if a.reuse_pixels:
                prior=r.json(a.reuse_pixels)
                analysis_paths={'tools/check-rendering-ui.py','tools/rendering_ui_proof.py','tools/close-rendering-ui.py'}|set(next(v for v in r.json('research/runtime/coverage/register.json')['behaviors'] if v['id']==contract.BID)['tests'])
                require(prior['kind']=='public_native_ui_comparison' and prior['acquisition']==result['acquisition']
                        and prior['comparison_processes_stopped']
                        and prior['original_manifest_verified_before_after'] and prior['comparison']['success']
                        and prior['comparison']['mismatching_pixels']==0
                        and prior['captured_command_stream_sha256']==result['captured_command_stream_sha256']
                        and {n:h for n,h in prior['sources'].items() if n not in analysis_paths}=={n:h for n,h in sources.items() if n not in analysis_paths},
                        'Retained pixel proof differs from exact acquisition/oracle/native sources')
                for key in ('asset_inputs','original_reference','comparison','world_domain','lighting'):
                    result[key]=prior[key]
                result['retained_pixel_phase']=dict(record=a.reuse_pixels,sha256=r.digest(a.reuse_pixels),
                    parent_whole_source_phase_stable=prior['sources_stable'],
                    reviewed_analysis_source_changes={n:dict(before=h,after=r.digest(n)) for n,h in prior['sources'].items() if n in analysis_paths and r.digest(n)!=h},
                    protected_pixel_sources_exact=True,
                    scope='Native/runtime/compiler, captured input, raster/history oracle and pixel comparison helpers retain exact original hashes. Only the named UI analyzer/closeout/test sources may differ. Reassess retained data with current semantics; preserve the old whole-source refusal. No new original execution or historical hash refresh.')
                result['original_manifest_verified_before_after']=True
            pe=ROOT/'original/Arcane_Nocd/Chaos.exe';require(hashlib.sha256(pe.read_bytes()).hexdigest()=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Wrong original build')
            assets=Path(acquired['session'])/'game'
            asset_inputs={str(f.relative_to(assets)):hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted(assets.rglob('*')) if f.is_file() and f.suffix.lower() in {'.spr','.jpg','.jpeg','.bmp','.pcx','.sft','.fp','.ttd','.wav'}}
            require(asset_inputs and all(acquired['installation_inputs'][n]['sha256']==h for n,h in asset_inputs.items()),'Changed reference assets')
            if a.reuse_pixels:require(result['asset_inputs']==asset_inputs,'Retained reference assets changed')
            else:
                result['asset_inputs']=asset_inputs;reference=out/'reference'
                execute('original-history',[ROOT/'working/build/qt-shell/mnm-acceptance-world-history',source,assets,ROOT/'working/original-reference',pe,reference])
                index=json.loads((reference/'worlds.json').read_text());require(index['success'] and not index['native_player_pixels_read'] and not index['original_destination_capture_read'],'Dependent original oracle')
                worlds={}
                for row in index['worlds']:
                    g=int(row['generation']);require(g==len(worlds)+1 and Path(row['path']).name==row['path'],'Original World ordering')
                    raw=inflate((reference/row['path']).read_bytes());require(len(raw)==row['width']*row['height']*2,'Incomplete original plane')
                    worlds[g]=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width'])
                result['original_reference']=dict(worlds=len(worlds),source_chunks=int(index.get('source_chunks','0')),draws=index['draws'],native_player_pixels_read=False,original_destination_capture_read=False,continuous_from_owned_initial_history=True)
                result['comparison']=compare(observed,worlds)
                from rendering_normal_acceptance import world_domain,lighting
                result['world_domain']=world_domain(observed);result['lighting']=lighting(ROOT,acquired,{v['path']:v for v in acquired['artifacts']})
            result['ui_proof']=assess(r,acquired,result,observed)
            def module(name):
                spec=importlib.util.spec_from_file_location('ui_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
            retained=module('close-rendering-retained-rows');identity=module('close-rendering-identity-gates')
            profile='player-software' if acquired['configuration']['software_flag'] else 'player-default'
            item=dict(reference=result['acquisition'],profile=profile,requirements=sorted(__import__('rendering_ui_acceptance').LEAVES),comparisons=[])
            result['normal_lifetime']=retained.retire_case(r,item)
            life=module('close-rendering-world-exit').imported_lifetimes(observed['commands'])
            require(life['nonce']==int(acquired['resident']['nonce']) and len(life['imports'])==result['normal_lifetime']['proof']['source_imports'],'UI command/source lifetimes differ')
            result['command_lifetimes']=dict(nonce=life['nonce'],imports=len(life['imports']),command_end=life['command_end'],deleted_import_incarnations=len(life['imports']))
            from evidence_objects import configured
            with configured(ROOT) as pair:
                result['identity']=identity.identify(r,pair,item,'40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168',set())
            result['success']=bool(result['comparison']['success'] and result['ui_proof']['verified'])
            if not result['success']:result['error']='UI semantics incomplete; retain the independent pixel result'
        except Exception as e:result['error']=str(e)
        finally:
            if before:
                try:execute('original-after',[ROOT/'tools/original-manifest.sh','verify']);result['original_manifest_verified_before_after']=True
                except Exception as e:result.update(success=False,original_verification_error=str(e))
            result['sources_stable']=all(r.digest(n)==h for n,h in sources.items());result['success'] &= result['sources_stable']
            bundle=out/'outputs.tar.gz';files=sorted(f for f in out.rglob('*') if f.is_file());members=[]
            with tarfile.open(bundle,'w:gz',compresslevel=1) as archive:
                for f in files:
                    raw=f.read_bytes();name=str(f.relative_to(out));members.append(dict(path=name,bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest()));archive.add(f,arcname=name,recursive=False)
            raw=bundle.read_bytes();name=a.report.removesuffix('.json')+'-outputs.bin'
            result['artifacts']=[dict(record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw),encoding='tar-gzip',members=members)]
            result['comparison_processes_stopped']=processes.stopped()
            h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
            print(json.dumps(dict(success=result['success'],error=result.get('error'),record=a.report,sha256=h)),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
