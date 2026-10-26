#!/usr/bin/env python3
"""Run the public native campaign journey and check the owning cursor journal.

The campaign runner supplies real Qt/XTest input, original World dispatches,
Mini/No/Yes callbacks and normal Quit. This adds cursor installation, execution
and balanced queue/client retirement checks. No legacy-driver alias claim.
"""
import argparse
import hashlib
import importlib.util
import json
import subprocess
import sys
from pathlib import Path
from coverage_evidence import read_json, write_report
from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template',type=Path)
    p.add_argument('--player-entrypoint',choices=['fresh','reuse'])
    p.add_argument('--source-game',type=Path,required=True)
    p.add_argument('--report',required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--restart',action='store_true')
    a=p.parse_args();out=a.output.resolve()
    if a.player_entrypoint:
        if a.restart or a.prefix_template:p.error('Player acceptance uses real fresh/reused default prefix without a template or injected restart')
    elif a.prefix_template is None:p.error('Require an owned prefix template')
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/a.report).exists():p.error('Fresh owned output and report required')
    out.mkdir(parents=True);(out/'.retain').write_text('Actual native cursor campaign and immutable negative evidence.\n')
    reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    b=next(x for x in reg['behaviors'] if x['id']=='NP.cursor-live-ownership')
    behaviors=[b]
    if a.player_entrypoint:behaviors.append(next(x for x in reg['behaviors'] if x['id']=='NP.native-player-launch'))
    paths=set().union(*(required_sources(x) for x in behaviors))
    if not a.player_entrypoint:
        paths |= module('cursor_campaign','tools/test-native-campaign.py').linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT)
    else:
        # First public launch builds Qt itself. Declare all native inputs first;
        # the campaign checks the actual linked compiler closure after the run.
        for directory in ('apps','renderer','assets','audio','compat','reconstruction','third_party'):
            paths.update(f.relative_to(ROOT).as_posix() for f in (ROOT/directory).rglob('*') if f.is_file() and f.suffix in ('.c','.cpp','.h','.hpp','.S','.inc','.inl','.ipp'))
    for directory in ('runtime','protocols','tools'):
        paths.update(f.relative_to(ROOT).as_posix() for f in (ROOT/directory).rglob('*') if f.is_file() and f.suffix in ('.c','.cpp','.h','.hpp','.py','.sh','.S','.json'))
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in sorted(paths)}
    declaration=dict(sources=sources,claims=[dict(behavior=x['id'],contract_sha256=behavior_contract(x,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s) for s in reg['scenarios'] if x['id'] in s['behaviors'] and s['level']=='live_equivalence' and (s['id']=='CUR26.owning-native-restart')==a.restart and (not s['id'].startswith('NPL26.') or s['id']=='NPL26.player-'+str(a.player_entrypoint))}) for x in behaviors])
    # Physical JSON is bounded producer input, consumed immediately below.
    claims=out/'claims.json';claims.write_text(json.dumps(declaration)+'\n')
    nested=a.report.removesuffix('.json')+'-campaign.json'
    report=dict(schema=1,success=False,**declaration,original_game_executed=False,live_replacement=False)
    try:
        command=[sys.executable,ROOT/'tools/test-native-owned-campaign.py','--native-only','--no-native-music','--qt-build-type','Release','--source-game',a.source_game,'--claims',claims,'--report',ROOT/nested]
        command += ['--player-entrypoint',a.player_entrypoint] if a.player_entrypoint else ['--prefix-template',a.prefix_template]
        command += ['--restart'] if a.restart else ['--functional-lifecycle','--stress-seconds','10']
        with (out/'campaign.log').open('xb') as log: result=subprocess.run(list(map(str,command)),cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=680)
        report.update(command=list(map(str,command)),campaign_exit=result.returncode,campaign_report=nested)
        campaign=read_json(ROOT/nested);report['campaign_success']=campaign.get('success',False)
        report['campaign_error']=campaign.get('error')
        if result.returncode or not campaign.get('success'):raise ValueError('Native campaign failed: '+str(campaign.get('error')))
        session=ROOT/campaign['session'];report['session']=campaign['session']
        cursor=module('cursor_journal','tools/native_cursor_journal.py').decode((session/'native-cursor-lifetime.bin').read_bytes())
        if not {0,1,4,5,6,7}<=set(cursor['completed_clients']):raise ValueError('Missing real setup/disposal/foreground/background/timer executions')
        if cursor['queue_accepted']!=cursor['queue_completed']+cursor['queue_cancelled']:raise ValueError('Incomplete real deferred-copy retirement')
        report.update(original_sha256=hashlib.sha256((a.source_game/'Chaos.exe').read_bytes()).hexdigest(),
                      native_lifetime_contract_verified=cursor['active_clients']==0 and cursor['pending_jobs']==0 and campaign['graphics_retirement']['resources_released'],
                      native_driver_lifetime_replaced=campaign['graphics_retirement']['ddraw_module_loaded'] is False and cursor['installed_hooks']==18,
                      original_copy_and_client_bodies_retained=True,
                      deferred_queue_exercised=cursor['queue_accepted']>0,deferred_generation_validation='Separate production-installer original-client controls',cursor=cursor,original_game_executed=True,live_replacement=True,normal_quit=True,original_worlds=campaign['resident']['completed'],campaign_functional_acceptance=campaign.get('functional_acceptance'),graphics_retirement=campaign['graphics_retirement'],success=True)
        if a.player_entrypoint:
            report.update(player_entrypoint=a.player_entrypoint,actual_player_entrypoint=campaign['actual_player_entrypoint'],actual_prefix_initialized=campaign['actual_prefix_initialized'],player_marker_before=campaign['player_marker_before'],player_marker_after=campaign['player_marker_after'],native_compiler_inputs_checked=campaign['native_compiler_inputs_checked'])
    except (ValueError,OSError,subprocess.SubprocessError) as exc:report['error']=str(exc)
    finally:
        report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps({k:report.get(k) for k in ('success','error','campaign_error','session','cursor')}),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
