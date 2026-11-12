#!/usr/bin/env python3
"""Independent original histories for failed and fresh public rendering epochs."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tarfile
import numpy as np
from coverage_evidence import Reader, write_report
from rendering_pilot import acquisition, compare, inflate, require
from rendering_transition_assessment import artifact,observed_epochs,assess
from comparison_process import Processes

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-transition-recovery-results-20261107/'

def archive_outputs(out,bundle):
    """Keep every logical member, using standard TAR links for equal contents."""
    members=[];contents={};saved=links=0
    files=sorted(f for f in out.rglob('*') if f.is_file() and f!=bundle)
    with tarfile.open(bundle,'w:gz',compresslevel=1) as archive:
        for f in files:
            raw=f.read_bytes();relative=str(f.relative_to(out));digest=hashlib.sha256(raw).hexdigest();key=digest,len(raw)
            members.append(dict(path=relative,bytes=len(raw),sha256=digest))
            if key in contents:
                entry=archive.gettarinfo(str(f),arcname=relative);entry.type=tarfile.LNKTYPE;entry.linkname=contents[key];entry.size=0;archive.addfile(entry);saved+=len(raw);links+=1
            else:
                contents[key]=relative;archive.add(f,arcname=relative,recursive=False)
    # Verify original member bytes through the independent standard TAR reader,
    # including all content references. Physical producer files are unchanged.
    with tarfile.open(bundle,'r:gz') as archive:
        require(len(archive.getmembers())==len(members),'Archive lost a logical member')
        verified={}
        for member in members:
            entry=archive.getmember(member['path'])
            expected=member['sha256'],member['bytes']
            if entry.islnk():
                require(entry.linkname in verified and verified[entry.linkname]==expected,'Archive content reference changed bytes')
            else:
                require(entry.isfile(),'Unexpected archive member type');source=archive.extractfile(entry);require(source is not None,'Archive member absent');raw=source.read()
                require((hashlib.sha256(raw).hexdigest(),len(raw))==expected,'Archive payload changed bytes')
            verified[member['path']]=expected
    return members,dict(logical_files=len(members),unique_contents=len(contents),hardlinked_members=links,logical_bytes=sum(v['bytes'] for v in members),duplicate_bytes=saved,all_member_bytes_verified=True,physical_producer_files_modified=False)


def main():
    p = argparse.ArgumentParser(description=__doc__); p.add_argument('--acquisition', required=True); p.add_argument('--report', required=True)
    p.add_argument('--reuse-pixels');p.add_argument('--analysis-review')
    args = p.parse_args()
    require(bool(args.reuse_pixels)==bool(args.analysis_review),'Retained pixels require exact analysis review')
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh immutable comparison required')
    out = ROOT / 'working/transition-comparisons' / Path(args.report).stem; out.mkdir(parents=True, exist_ok=False); (out / '.retain').touch()
    result = dict(schema=1, kind='public_native_transition_recovery_comparison', success=False, original_pixels_used_as_native_inputs=False)
    processes, before = Processes(), False
    with Reader(ROOT) as reader:
        a = reader.json(args.acquisition); result['acquisition'] = dict(record=args.acquisition, sha256=reader.digest(args.acquisition))
        plan_name = a['prospective_plan']['record']
        require(reader.digest(plan_name)==a['prospective_plan']['sha256'],'Prospective pause plan changed')
        plan = reader.json(plan_name)
        result['prospective_plan'] = dict(record=plan_name, sha256=reader.digest(plan_name))
        sources = a['sources'] | a.get('native_admission', {}).get('native_only', {}).get('sources', {}) | plan['oracle']['native_sources'] | plan['sources']
        sources.update({n: reader.digest(n) for n in ('tools/check-rendering-transition.py', 'tools/rendering_transition.py', 'tools/rendering_transition_assessment.py', 'tools/rendering_pause_assessment.py', 'tools/rendering_pause.py', 'tools/close-rendering-camera-matrix.py', 'tools/rendering_interaction.py',
            'tools/rendering_pilot.py', 'tools/rendering_capture.py', 'tools/comparison_process.py', 'tools/close-rendering-source-camera.py',
            'tools/close-rendering-world-exit.py', 'tools/join-rendering-normal-acceptance.py', 'tools/rendering_normal_acceptance.py',
            'tools/rendering_menu_acceptance.py', 'tools/resident_capture.py', 'tools/inspect-canvas-producers.py',
            'tools/rendering_gameplay_acceptance.py', 'tools/campaign_input_observation.py', 'tools/test-menu-observer.py')})
        result['sources'] = sources
        def execute(label, command):
            with (out / (label + '.log')).open('xb') as log: processes.run([str(v) for v in command], cwd=ROOT, stdout=log, timeout=300)
        try:
            require(a['success'] and a['sources_stable'] and a['private_wine_stopped']
                    and all(reader.digest(n) == h for n, h in sources.items()), 'Incomplete/changed camera source phase')
            require(all(a['sources'].get(n)==h for n,h in plan['runtime_sources'].items()),'Prospectively frozen runtime closure changed')
            for binary in [a['player_binary'], *plan['oracle']['binaries']]:
                require(reader.digest(binary['record']) == binary['sha256'] == hashlib.sha256((ROOT / binary['path']).read_bytes()).hexdigest(), 'Exact accepted executable differs')
            parent=None
            if args.reuse_pixels:
                parent=reader.json(args.reuse_pixels);review=reader.json(args.analysis_review)
                allowed={'tools/check-rendering-transition.py','tools/rendering_transition_assessment.py'}
                require(review['success'] and review['kind']=='retained_transition_analysis_repair' and review['original_game_runs']==0 and review['native_sources_unchanged'] is True and review['failed_comparison']==dict(record=args.reuse_pixels,sha256=reader.digest(args.reuse_pixels)) and review['acquisition']==result['acquisition'],'Unreviewed retained analysis')
                require(parent['acquisition']==result['acquisition'] and parent['sources_stable'] and parent['comparison_processes_stopped'] and parent['original_manifest_verified_before_after'] and len(parent['epochs'])==2 and all(v['comparison']['success'] and v['comparison']['mismatching_pixels']==0 for v in parent['epochs']),'Retained pixel cohort incomplete')
                require(set(parent['sources'])==set(sources) and all(h==sources[n] for n,h in parent['sources'].items() if n not in allowed),'Native/capture/oracle source changed')
                require(set(review['changes'])==allowed,'Wrong analyzer repair scope')
                for n,change in review['changes'].items():
                    require(parent['sources'][n]==change['before']==reader.digest(change['before_record']) and sources[n]==change['after'],'Analyzer source preimage/current pair differs')
                result['retained_pixel_comparison']=dict(record=args.reuse_pixels,sha256=reader.digest(args.reuse_pixels),analysis_review=dict(record=args.analysis_review,sha256=reader.digest(args.analysis_review)),new_original_reference_execution=False,new_pixel_comparison=False)
            view,epochs,result['partition']=observed_epochs(reader,a)
            if parent:require(parent['partition']==result['partition'],'Retained epoch partition differs')
            result['epochs']=[]
            execute('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
            pe=ROOT/'original/Arcane_Nocd/Chaos.exe'
            require(hashlib.sha256(pe.read_bytes()).hexdigest()=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Original build differs')
            from rendering_normal_acceptance import world_domain,lighting
            from rendering_transition import terminal_compare,terminal_domain
            for i,(epoch,observed) in enumerate(zip(a['proof']['epochs'],epochs)):
                source=Path(epoch['resident']['stage_profile']['capture']['path'])
                require(source.read_bytes()==artifact(reader,a,str(source)),'Physical source capture changed')
                assets=Path(epoch['root'])/'game'
                extensions={'.spr','.jpg','.jpeg','.bmp','.pcx','.sft','.fp','.ttd','.wav'}
                asset_inputs={str(f.relative_to(assets)):hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted(assets.rglob('*')) if f.is_file() and f.suffix.lower() in extensions}
                require(asset_inputs and all(a['installation_inputs'][n]['sha256']==h for n,h in asset_inputs.items()),'Independent reference asset differs')
                if parent:
                    item=dict(parent['epochs'][i])
                    require(item['root']==epoch['root'] and item['failed'] is (i==0) and item['asset_inputs']==asset_inputs and item['observations']==observed['observations'] and item['world_domain']==(terminal_domain(observed) if i==0 else world_domain(observed)),'Retained pixel inputs/domain differ')
                    cohort=item['comparison']['comparisons']
                    require(cohort and item['comparison']['pixels_compared']==sum(v['pixels'] for v in cohort) and len(cohort)==len(observed['swaps']) and item['comparison']['world_presentations']==observed['observations']['world_swaps'] and all(v['mismatching_pixels']==0 and v['actual_sha256']==v['expected_sha256'] for v in cohort),'Incomplete retained actual swap cohort')
                else:
                    reference=out/('epoch-'+str(i))/'reference';reference.parent.mkdir()
                    execute('original-history-'+str(i),[ROOT/'working/build/qt-shell/mnm-acceptance-world-history',source,assets,ROOT/'working/original-reference',pe,reference])
                    index=json.loads((reference/'worlds.json').read_bytes());require(index['success'] and index['native_player_pixels_read'] is False and index['original_destination_capture_read'] is False,'Dependent original oracle')
                    worlds={}
                    for row in index['worlds']:
                        generation=int(row['generation']);require(generation==len(worlds)+1 and Path(row['path']).name==row['path'],'Original World ordering')
                        raw=inflate((reference/row['path']).read_bytes());require(len(raw)==row['width']*row['height']*2,'Incomplete original plane')
                        worlds[generation]=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width'])
                    item=dict(root=epoch['root'],failed=i==0,observations=observed['observations'],world_domain=terminal_domain(observed) if i==0 else world_domain(observed),
                        original_reference=dict(worlds=len(worlds),source_chunks=int(index.get('source_chunks','0')),draws=index['draws'],native_player_pixels_read=False,original_destination_capture_read=False,continuous_from_owned_initial_history=True),asset_inputs=asset_inputs)
                    item['comparison']=terminal_compare(observed,worlds,epoch['terminal']) if i==0 else compare(observed,worlds)
                    # Even replies after the last visible failed paint are explicit,
                    # uncredited terminal tail; never silently count them as compared.
                    item['uncredited_terminal_cpu_sequences']=[int(m['sequence']) for m,_ in observed['command_rows'] if m['kind']=='cpu-read' and i==0 and int(m['sequence'])>item['comparison']['required_prefix_commands']]
                require(all(hashlib.sha256((assets/n).read_bytes()).hexdigest()==h for n,h in asset_inputs.items()),'Reference asset changed')
                fresh=dict(a,session=epoch['root'],resident=epoch['resident'],artifacts=[v for v in a['artifacts'] if v['path'].startswith(str(Path(epoch['root']).relative_to(ROOT))+'/')])
                if i==1:item['lighting']=lighting(ROOT,fresh,{v['path']:v for v in fresh['artifacts']})
                result['epochs'].append(item)
            result['proof']=assess(reader,a,result,epochs)
            result['comparison']=dict(success=all(v['comparison']['success'] for v in result['epochs']),pixels_compared=sum(v['comparison']['pixels_compared'] for v in result['epochs']),mismatching_pixels=sum(v['comparison']['mismatching_pixels'] for v in result['epochs']),world_presentations=sum(v['comparison']['world_presentations'] for v in result['epochs']))
            result['success']=result['comparison']['success'] and result['proof']['verified']
        except Exception as error:
            result['error'] = str(error)
        finally:
            if before:
                try:
                    execute('original-after', [ROOT / 'tools/original-manifest.sh', 'verify']); result['original_manifest_verified_before_after'] = True
                except Exception as error: result.update(success=False, original_verification_error=str(error))
            result['sources_stable'] = all(reader.digest(n) == h for n, h in sources.items())
            result['success'] &= result['sources_stable']
            # One immutable bounded archive preserves every exact output and
            # member hash without thousands of SQLite transactions.
            bundle = out / 'comparison-outputs.tar.gz'
            members,result['archive_content_references']=archive_outputs(out,bundle)
            raw = bundle.read_bytes(); name = args.report.removesuffix('.json') + '-outputs.bin'
            result['artifacts'] = [dict(path=bundle.name, record=name, sha256=write_report(ROOT,name,raw), bytes=len(raw), encoding='tar-gzip', members=members)]
            result['comparison_processes_stopped'] = processes.stopped()
            h = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
            print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=h,
                                  worlds=result.get('original_reference', {}).get('worlds'))), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__': raise SystemExit(main())
