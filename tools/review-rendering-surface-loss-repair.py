#!/usr/bin/env python3
"""Retain the missed primary-provider probe and a single bounded repair review."""
import argparse,hashlib,io,json,sqlite3,subprocess,tarfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_menu_acceptance import artifact
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/surface-loss-restore-results-20261109/'

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--failed',default=PREFIX+'default640-01-acquisition.json');p.add_argument('--parent-plan',default=PREFIX+'input-plan-01.json')
    p.add_argument('--report',required=True);a=p.parse_args();require(not (ROOT/a.report).exists(),'Fresh repair review required')
    with Reader(ROOT) as r:
        failed=r.json(a.failed);plan=r.json(a.parent_plan)
        require(not failed['success'] and failed['error']=='Native provider target-loss observation deadline' and
            failed['sources_stable'] and failed['player_binary_stable'] and failed['original_game_runs']==1 and
            failed['charged_original_starts']==2 and failed['original_manifest_verified_before_after'] and failed['prefix_pool']['returned'],
            'Unexpected first provider pilot/failure custody')
        require(failed['prospective_plan']==dict(record=a.parent_plan,sha256=r.digest(a.parent_plan)),'Wrong failed plan')
        require(len(failed['epochs_observed'])==1 and not any(v['action']=='physical-hard-restart' for v in failed['actions']),
            'Failure crossed a restart boundary')
        root=Path(failed['observed_session_roots'][0]);relative=root.relative_to(ROOT)
        resident=json.loads(artifact(r,failed,str(relative/'resident-world-report.json')))
        require(int(resident['completed'])>0 and 'native_render_owner' not in resident and not resident['error'],
                'Failure does not match the actual GUI-resident owner')
        paths={v['path'] for v in failed['artifacts']}
        require(str(relative/'surface-loss-request-v1.json') in paths and
            str(relative/'surface-loss-observation-v1.json') not in paths and str(relative/'surface-loss-retirement-v1.json') not in paths,
            'The retained pilot actually observed provider loss')
        prior=plan['compiler_sources']|plan['runtime_sources']|plan['sources']
        archive=tarfile.open(fileobj=io.BytesIO(r.read_bytes(plan['source_phase']['record'])),mode='r:gz')
        def original_source(name):
            if name in plan['source_phase']['members']:body=archive.extractfile(name).read()
            else:body=subprocess.check_output(['git','show',plan['source_phase']['source_revision']+':'+name],cwd=ROOT)
            require(hashlib.sha256(body).hexdigest()==prior[name],'Missing exact failed source preimage: '+name)
            return body
        host=original_source('apps/qt-shell/main.cpp');collector=original_source('tools/run-rendering-recovery-closeout.py')
        require(b'if(resident && qEnvironmentVariableIntValue("MNM_WORLD_SNAPSHOT")==1)' in host and
            '--independent-world' not in failed['command'] and b"not k.startswith(('MNM_', 'WINE'))" in collector and
            b"env['MNM_SURFACE_LOSS_TEST']='1'" in collector and b"env['MNM_WORLD_SNAPSHOT']" not in collector,
            'Provider selection differs from the exact retained source route')
        with sqlite3.connect((ROOT/plan['execution_paths']['budget']).as_uri()+'?mode=ro',uri=True) as db:
            rows=list(db.execute('SELECT report,repair_of,charged_games FROM attempts'))
        require(rows==[(a.failed,None,2)] and plan['bounds']['maximum_requests']==6 and plan['bounds']['maximum_original_games']==10,
                'Cumulative request/start custody changed')
        changes={n:dict(before=h,after=r.digest(n)) for n,h in prior.items() if r.digest(n)!=h}
        result=dict(schema=1,kind='native_surface_loss_primary_provider_repair_review',success=True,original_game_runs=0,newly_passed_requirements=0,
            failed_acquisition=dict(record=a.failed,sha256=r.digest(a.failed)),parent_plan=dict(record=a.parent_plan,sha256=r.digest(a.parent_plan)),
            failed_observed_game_starts=1,failed_charged_original_starts=2,failed_actual_count_complete=failed['actual_start_count_complete'],
            actual_gui_worlds=int(resident['completed']),actual_target_loss_observed=False,
            diagnosis='The ordinary public command uses GUI-resident rendering. Its collector deliberately clears inherited MNM variables and does not select --independent-world. The first probe existed only in NativeRenderWorker, so the actual primary provider never consumed the request. This is a diagnostic/provider-binding failure; healthy World output is not surface-loss acceptance.',
            repair='Bind the same unique request to the actual GUI-resident command provider. Destroy its real owned offscreen target on its GUI owner at a completed turn, require typed native failure, recreate only for deletion, retire all owners and physically Hard restart. Keep the ordinary backend; do not substitute the optional independent worker.',
            source_changes=changes,new_source_files=['apps/qt-shell/gui_surface_loss_probe.hpp','tests/native-gui-surface-loss-test.cpp'],
            bounds_preserved=dict(maximum_requests=6,maximum_original_games=10,maximum_repair_cases=1),
            budget_rows=rows,charged_before=2,nominal_remaining_original_starts=8,charges_refunded=0,
            confidence='high_within_recorded_source_and_owner_scope',historical_failure_preserved=True)
    sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=True,record=a.report,sha256=sha)),flush=True)
if __name__=='__main__':main()
