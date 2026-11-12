#!/usr/bin/env python3
"""Recover post-run metadata after retention refusal; never launch another game.

The original collector's volatile final report was lost during its raw-artifact
import. Preserve that refusal and bind the retained actual run to its startup
admission, pre-run executable, unchanged accepted full-source preimage, terminal
receipts and closed capture identity. This is explicit metadata recovery, not a
new execution or a historical hash refresh.
"""
import copy,hashlib,importlib.util,json,os,sqlite3,struct,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
from campaign_storage import validate_storage_flow
from native_only_contract import outcome
from experiment_environment import _idle
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/whole-player-validation-results-20261110/'
N='research/runtime/rendering-normal-results-20261031/'
TARGET=N+'wp10-software-storage-01-acquisition.json'
PARENT=N+'wp10-default-storage-01-acquisition.json'
OUT=ROOT/'working/pilot/wp10-software-storage-01-acquisition'
OWNER=ROOT/'working/tests/native-output-owner/run-lxz0u77a'
SESSION=ROOT/'working/experiments/menu-observer/run-w7_0tcfu'


def digest(path):
    with Path(path).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()


def main():
    require(not(ROOT/TARGET).exists(),'Never overwrite an acquisition')
    failure=(ROOT/'working/whole-player/software-storage-01-launch.log').read_bytes()
    require(b'Shared evidence write would breach reserve'in failure,'Exact retention refusal required')
    with Reader(ROOT)as reader:
        before=reader.json(PARENT);source_ref=dict(record=PARENT,sha256=reader.digest(PARENT))
        require(before['success']and before['runtime_sources_stable']and before['compiler_closure_stable'],
                'Accepted earlier complete source preimage required')
        sources=before['sources']
        require(all(reader.digest(n)==h for n,h in sources.items()),'A source changed since the pre-run accepted snapshot')
        build_record=TARGET.removesuffix('.json')+'-player-build.bin'
        binary_record=TARGET.removesuffix('.json')+'-player-binary.bin'
        player=reader.read_bytes(binary_record)
        require(hashlib.sha256(player).hexdigest()==before['player_build']['binary']['sha256']==digest(ROOT/'working/build/qt-shell/mnm-qt-shell'),
                'Actual software pre-run player differs from accepted binary')
        require(reader.digest(build_record)==digest(OUT/'player-build.log'),'Pre-run software build log changed')
        player_build=copy.deepcopy(before['player_build'])
        player_build['log']=dict(record=build_record,sha256=reader.digest(build_record))
        player_build['binary']=dict(path='working/build/qt-shell/mnm-qt-shell',record=binary_record,sha256=reader.digest(binary_record))
        spec=importlib.util.spec_from_file_location('recovery_compiler',ROOT/'tools/test-native-campaign.py');campaign=importlib.util.module_from_spec(spec);spec.loader.exec_module(campaign)
        require(set(campaign.linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT))==set(player_build['compiler_sources']),
                'Software compiler dependency graph changed')
        admission=json.loads((SESSION/'native-output-admission.json').read_bytes())
        require(admission==json.loads(next((OWNER/'sessions').glob('*.json')).read_bytes())
                and admission['session']==str(SESSION),'Actual startup owner/session differs')
        require(all(reader.digest(n)==h for n,h in admission['native_only']['sources'].items()),
                'Per-run startup native/compiler admission changed')
        require(all(sources.get(n)==h for n,h in admission['native_only']['sources'].items()if n in sources),
                'Actual startup and accepted preimage source phases disagree')
        flow=json.loads((OUT/'flow.json').read_bytes());validate_storage_flow(flow)
        resident=json.loads((SESSION/'resident-world-report.json').read_bytes());retired=outcome(SESSION)
        shell=json.loads((SESSION/'native-only-session.json').read_bytes())
        require(retired['state']=='completed'and shell['normal_quit']is True and shell['launcher_exit']==0,
                'Actual normal software process/graphics retirement missing')
        launch=(OUT/'public-launch.log').read_text(errors='replace')
        require('Menu launcher exited with status 0.'in launch and 'Original process exited with status 0'in launch,
                'Actual launcher/original exit is unproved')
        _idle(OUT/'wineprefix','wineprefix')
        with sqlite3.connect((OUT/'environment-wineprefix.sqlite3').as_uri()+'?mode=ro',uri=True)as db:
            records={name:(h,body)for name,h,body in db.execute('SELECT name,sha256,payload FROM records')}
        require({'lease','final','returned'}<=set(records)and all(hashlib.sha256(body).hexdigest()==h for name,(h,body)in records.items()if name!='returned'),
                'Returned original prefix receipt is incomplete')
        lease=json.loads(records['lease'][1]);final=json.loads(records['final'][1])
        require(records['returned'][0]==lease['token'] and records['returned'][1]==b'pool-owned; restore before historical consumption',
                'Returned receipt belongs to another original lease')
        require(lease['target']==str(OUT/'wineprefix')and not(OUT/'wineprefix').exists(),
                'Software original prefix was not returned')
        pool=copy.deepcopy(before['prefix_pool']);pool.update(target=str(OUT/'wineprefix'),target_owner_sha256=final['.mnm-directplay-dx6.json'][3],
            returned=True,receipt=str(OUT/'environment-wineprefix.sqlite3'))
        ids=[]
        for path in sorted((OUT/'capture').glob('*.bin')):
            with path.open('rb')as f:
                require(f.read(8)==b'MNMACE01','Actual capture envelope differs')
                m,n=struct.unpack('<II',f.read(8));row=json.loads(f.read(m));require(row['kind']=='open'and n==0,'Capture open identity missing');ids.append(row['session'])
        require(len(ids)>=2 and len(set(ids))==1,'Software capture roles disagree')
        install={str(f.relative_to(ROOT/'working/game-nocd')):dict(sha256=digest(f),bytes=f.stat().st_size)
                 for f in sorted((ROOT/'working/game-nocd').rglob('*'))if f.is_file()}
        require(install==before['installation_inputs'],'Immutable shared installation changed')
        for label in ('original-before','original-after'):
            require((OUT/(label+'.log')).read_text().strip()=='Verified 2927 original files against research/original-files.tsv',
                    'Actual pre/post original verification missing')
        manifest_record=TARGET.removesuffix('.json')+'-launch-manifest.bin'
        manifest=copy.deepcopy(before['launch_manifest']);manifest.update(record=manifest_record,sha256=reader.digest(manifest_record))
        require(reader.digest(manifest_record)==before['launch_manifest']['sha256'],'Actual launch input manifest changed')
        configuration=copy.deepcopy(before['configuration']);configuration.update(id='software-probe',software_flag=True,configuration_home=str(OUT/'config'))
        recovery_sources={n:reader.digest(n)for n in ('tools/recover-current-player-acquisition.py','tools/experiment_environment.py','tools/native_only_contract.py','tools/campaign_storage.py')}
        provenance=dict(schema=1,kind='software_storage_retention_metadata_recovery',success=True,new_original_game_runs=0,
            retained_original_start=4,actual_run_completed=True,original_volatile_final_report_lost=True,
            full_source_preimage=source_ref,source_stability_method='All exact native/runtime/collector/compiler hashes in the accepted earlier full pre-run snapshot remain unchanged. The actual software startup admission independently pins its compiled/native sources and the pre-run software executable is retained under its own hash. Terminal raw run/capture/lease receipts are checked. This retrospectively reconstructs metadata; it does not claim the failed importer wrote its volatile stability flags.',
            native_startup_source_count=len(admission['native_only']['sources']),player_binary=player_build['binary'],capture_identity=ids[0],
            failure_log=dict(record=P+'software-storage-retention-failure-log.bin',sha256=write_report(ROOT,P+'software-storage-retention-failure-log.bin',failure)),
            sources=recovery_sources,historical_data_relocated=False,new_owned_pair=True)
        provenance_ref=dict(record=P+'software-storage-metadata-recovery-01.json',sha256=write_report(ROOT,P+'software-storage-metadata-recovery-01.json',(json.dumps(provenance,indent=2)+'\n').encode()))
        all_sources=dict(sources);all_sources.update(recovery_sources)
        report=dict(schema=1,kind='public_native_normal_acquisition',success=False,player_accepted=False,scenario='normal-storage',
            sources=all_sources,source_commit=before['source_commit'],session_identity=ids[0],configuration=configuration,
            original_pixels_used_as_native_inputs=False,original_reference_executed=False,
            scope='Actual completed current software storage run; final metadata explicitly recovered after post-run retention refusal. No new game execution, source waiver or manufactured execution result. Independent full native/original-history comparison remains required.',
            player_build=player_build,installation_inputs=install,launch_manifest=manifest,prefix_pool=pool,
            command=[str(ROOT/'run-native.sh'),'--prefix',str(OUT/'wineprefix'),'--','--live-menu-test',str(OUT/'flow.json'),'--software-rendering'],
            exit_code=0,private_wine_stopped=True,flow=flow,owner=str(OWNER),session=str(SESSION),retirement=retired,native_admission=admission,resident=resident,
            original_manifest_verified_before_after=True,sources_stable=True,compiler_closure_stable=True,player_binary_stable=True,
            native_runtime_sources=sorted(native_runtime_source_paths(ROOT)),current_player_plan=before['current_player_plan'],current_player_build=before['current_player_build'],
            current_player_scenario='storage',current_player_profile='player-software',runtime_sources_stable=True,
            metadata_recovered_after_retention_refusal=True,metadata_recovery=provenance_ref)
    write_report(ROOT,P+'software-storage-pre-retention-recovered-01.json',(json.dumps(report,indent=2)+'\n').encode())
    candidates=[f for f in OUT.rglob('*')if f.is_file()and'wineprefix'not in f.relative_to(OUT).parts and f.name!='Xauthority']
    candidates += [f for f in OWNER.rglob('*')if f.is_file()and f.name!='Xauthority']
    candidates += [f for f in SESSION.rglob('*')if f.is_file()and('game'not in f.relative_to(SESSION).parts or f.suffix.lower()in('.dll','.cfg','.sav','.ini')or(f.name.startswith('native-')and f.suffix=='.bin'))]
    report['artifacts']=[]
    for f in sorted(set(candidates)):
        limit=2*1024**3 if f==OUT/'worlds.bin'else 1024**3 if f.parent==OUT/'capture'else 512*1024**2
        if f.is_symlink()or f.stat().st_size>limit:continue
        relative=str(f.relative_to(ROOT));name=TARGET.removesuffix('.json')+'-raw-'+hashlib.sha256(relative.encode()).hexdigest()[:20]+'.bin'
        body=f.read_bytes();sha=write_report(ROOT,name,body)
        report['artifacts'].append(dict(path=relative,record=name,sha256=sha,bytes=len(body)))
    report.update(artifact_count=len(report['artifacts']),experiments=[str(SESSION)],temporary_root=str(OUT))
    require(all(digest(ROOT/n)==h for n,h in all_sources.items()),'Source changed during retention recovery')
    report['success']=True
    sha=write_report(ROOT,TARGET,(json.dumps(report,indent=2)+'\n').encode())
    with sqlite3.connect(ROOT/'working/whole-player/requests.sqlite3')as db:
        require(db.execute('SELECT state FROM requests WHERE report=?',(TARGET,)).fetchone()==('requested',),'Never reset the original start budget')
        db.execute('UPDATE requests SET state=?,result_sha256=? WHERE report=?',('acquired',sha,TARGET))
    from evidence_workspace import register_completed
    register_completed(ROOT,[OUT,SESSION],dict(record=TARGET,sha256=sha))
    print(json.dumps(dict(success=True,new_original_game_runs=0,record=TARGET,sha256=sha,metadata_recovery=provenance_ref)),flush=True)
    return 0

if __name__=='__main__':raise SystemExit(main())
