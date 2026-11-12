#!/usr/bin/env python3
"""Recheck retained zero-mismatch output after the typed paint-lookup correction."""
import argparse,copy,hashlib,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require,acquisition
from rendering_menu_acceptance import artifact
from required_boundary_observation import PREFIX
from required_boundary_source_review import review
ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('acquisition','comparison','report'):p.add_argument('--'+n,required=True)
    a=p.parse_args();require(a.report.startswith(PREFIX) and not(ROOT/a.report).exists(),'Fresh retained-data result required')
    result=dict(schema=1,kind='public_native_interaction_comparison',success=False,original_game_runs=0,
        new_native_executions=0,new_original_replays=0,new_pixel_comparisons=0)
    with Reader(ROOT)as reader:
        acquired=reader.json(a.acquisition);old=reader.json(a.comparison)
        aq_ref=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
        result.update(acquisition=aq_ref,parent_comparison=dict(record=a.comparison,sha256=reader.digest(a.comparison)))
        sources=old['sources']|{n:reader.digest(n)for n in ('tools/reanalyze-required-boundary-session.py','tools/required_boundary_source_review.py')}
        current,exceptions=review(reader,sources);result['sources']=current
        try:
            require(old['acquisition']==aq_ref and acquired['success'] and acquired['sources_stable']
                    and old['success'] is False and old['error']=='Uncompared fullscreen paint'
                    and old['sources_stable'] and old['comparison_processes_stopped']
                    and old['original_manifest_verified_before_after'],
                    'Only the exact retained final-assessment refusal is eligible')
            require(old['comparison']['success'] and old['comparison']['mismatching_pixels']==0
                    and old['comparison']['remaining_surfaces']==0,'Pixel failure cannot be relabeled a host-checker correction')
            ref=old['original_reference']
            require(ref['continuous_from_owned_initial_history'] and not ref['native_player_pixels_read']
                    and not ref['original_destination_capture_read'],'Dependent/incomplete original reference')
            plan=reader.json(acquired['prospective_plan'])
            for binary in [acquired['player_binary'],*plan['oracle']['binaries']]:
                require(reader.digest(binary['record'])==binary['sha256']==hashlib.sha256((ROOT/binary['path']).read_bytes()).hexdigest(),'Tested binary changed')
            review(reader,acquired['sources'])
            streams=[artifact(reader,acquired,Path(v['path']).name)for v in acquired['artifacts']
                     if '/capture/' in v['path'] and v['path'].endswith('.bin')]
            observed=acquisition(streams,acquired['session_identity'])
            require(observed['observations']==old['observations'] and hashlib.sha256(observed['commands']).hexdigest()==old['captured_command_stream_sha256'],
                    'Captured command/output membership changed')
            total=0
            for row in old['comparison']['comparisons']:
                require(row['mismatching_pixels']==0 and row['actual_sha256']==row['expected_sha256']
                        and row['paint'] in observed['swaps'],'Incomplete prior pixel result')
                _,body=observed['paints'][row['paint']];raw=body()if callable(body)else body
                require(hashlib.sha256(raw).hexdigest()==row['actual_sha256'],'Captured output hash changed')
                total+=row['pixels']
            require(total==old['comparison']['pixels_compared'] and old['comparison']['world_presentations']==observed['observations']['world_swaps'],
                    'Pixel result dropped a presented frame')
            assets=Path(acquired['session'])/'game'
            require(old['asset_inputs'] and all(hashlib.sha256((assets/n).read_bytes()).hexdigest()==h==acquired['installation_inputs'][n]['sha256'] for n,h in old['asset_inputs'].items()),
                    'Original replay asset inputs changed')
            spec=importlib.util.spec_from_file_location('corrected_fullscreen',ROOT/'tools/check-required-boundary-session.py')
            checker=importlib.util.module_from_spec(spec);spec.loader.exec_module(checker)
            result['proof']=checker.assess(reader,acquired,old,observed)
            for key in ('comparison','original_reference','world_domain','observations','lighting','asset_inputs','captured_command_stream_sha256'):
                result[key]=copy.deepcopy(old[key])
            result.update(success=True,asset_inputs_stable=True,comparison_processes_stopped=True,
                original_manifest_verified_before_after=old['original_manifest_verified_before_after'],
                original_pixels_used_as_native_inputs=False,reused_pixel_result=True,
                reverified_capture_hashes=len(old['comparison']['comparisons']),source_applicability=exceptions,
                preserved_source_phases=dict(acquisition=acquired['sources'],pixel_comparison=old['sources']),
                scope='Retained-data reanalysis of the exact paint-ID type refusal. Reverify all captured displayed-byte hashes and replay assets, reuse the original zero-mismatch independent comparison, and execute the corrected metadata/geometry/retirement assessment. No new original/game/native execution or pixel replay; old failure and source hashes stay immutable.')
        except Exception as error:result['error']=str(error)
        finally:
            result['sources_stable']=all(reader.digest(n)==h for n,h in current.items())
            result['success'] &= result['sources_stable']
            h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],sha256=h,error=result.get('error'))),flush=True)
    return not result['success']
if __name__=='__main__':raise SystemExit(main())
