#!/usr/bin/env python3
"""Reproduce the bounded installed-map native campaign under a storage guard."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MAIN=Path(subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--path-format=absolute','--git-common-dir'],text=True).strip()).parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('asset_root',type=Path);parser.add_argument('--window',action='store_true');parser.add_argument('--cure',action='store_true');parser.add_argument('--legacy-cli',type=Path);args=parser.parse_args()
    if args.cure:args.window=True
    parent=ROOT/'working/tests/map-campaign';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Installed crop campaign execution evidence.\n')
    spec=importlib.util.spec_from_file_location('claims',ROOT/'tools/draft-coverage-claims.py');claims=importlib.util.module_from_spec(spec);spec.loader.exec_module(claims)
    behaviors=['NP.map-campaign','NP.skirmish-mission','NP.campaign-arena']
    scenarios=['NP.map-campaign.journey','NP.skirmish-mission.continuation','NP.campaign-arena.journey']
    if args.window:
        behaviors.extend(['NP.map-campaign-window','NP.campaign-window']);scenarios.extend(['NP.map-campaign-window.journey','NP.campaign-window.journey'])
    if args.cure:
        behaviors.extend(['NP.campaign-persistence','NP.campaign-session','NP.skirmish-simulation','NP.skirmish-window','NP.battle-effects','NP.cure-battle'])
        scenarios.extend(['NP.campaign-persistence.restart','NP.campaign-session.transaction','world.native-skirmish','window.native-skirmish','BATTLE01.sections','BATTLE02.cure-playable','NP.map-cure.continuation'])
    declaration=claims.declaration(ROOT,json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),behaviors,scenarios)
    (run/'claims.json').write_text(json.dumps(declaration,indent=2)+'\n');build=ROOT/'working/build/native-map-campaign'
    commands=[['cmake','-S',str(ROOT/'apps/skirmish'),'-B',str(build),'-DCMAKE_BUILD_TYPE=Debug'],['cmake','--build',str(build),'-j4'],['ctest','--test-dir',str(build),'--output-on-failure','-E','native-(campaign|skirmish)-window'],['python3',str(ROOT/'tests/test-map-campaign.py'),str(build/'mnm-map-campaign-cli'),str(build/'map-campaign-test'),str(args.asset_root.resolve())]]
    if args.window:
        commands[2]=commands[2][:-2]
        commands.append(['python3',str(ROOT/'tests/test-map-campaign-window.py'),str(build/'campaign-window-test'),str(build/'mnm-map-campaign-cli'),str(build/'map-campaign-test'),str(args.asset_root.resolve())])
    result={'schema':1,'scope':'Native authored Plains 8x8 sealed crop campaign, owned resource-bound restart/movement/casting and progression to the arena; original campaign and full-map equivalence excluded.','claims':declaration['claims'],'sources':declaration['sources'],'commands':[],'success':False}
    env=os.environ.copy();env['XDG_CONFIG_HOME']=str(run/'qt-config');env['QT_QPA_PLATFORM']='xcb';env['QT_STYLE_OVERRIDE']='Fusion';env['MNM_MAP_CAMPAIGN_OUTPUT']=str(run/'journey');env['MNM_MAP_CAMPAIGN_WINDOW_OUTPUT']=str(run/'window');env['MNM_CAMPAIGN_JOURNEY_OUTPUT']=str(run/'arena');env['MNM_CAMPAIGN_WINDOW_OUTPUT']=str(run/'arena-window')
    def execute(command,name):
        log=run/(name+'.log')
        with log.open('w') as output:r=subprocess.run(command,cwd=ROOT,env=env,stdout=output,stderr=subprocess.STDOUT)
        result['commands'].append({'argv':command,'exit_code':r.returncode,'log':str(log.relative_to(ROOT)),'sha256':sha(log)})
        if r.returncode:raise RuntimeError('Validation failed: '+str(log))
    try:
        execute(['sh',str(MAIN/'tools/original-manifest.sh'),'verify'],'manifest-before')
        for i in ([0,1,3,4,2] if args.window else [0,1,3,2]):
            execute(commands[i],'command-'+str(i))
            if args.window and i==1:execute([str(build/'map-presentation-test'),str(args.asset_root.resolve())],'presentation')
        if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise RuntimeError('Declared sources changed during execution')
        journey=run/'journey/report.json';details=json.loads(journey.read_text());assert details['success']
        result['journey']={'report':str(journey.relative_to(ROOT)),'sha256':sha(journey),'asset_sha256':details['checks']['assets'],'refused_resources':details['refused_resources'],'refused_checkpoints':details['checks']['refused_checkpoints']}
        if args.window:
            window=run/'window/report.json';details=json.loads(window.read_text());assert details['success']
            result['window']={'report':str(window.relative_to(ROOT)),'sha256':sha(window),'processes':details['processes'],'refused_loads':details['refused_loads'],'final_transition':details['final_transition']}
        if args.cure:
            execute([str(build/'map-cure-test'),str(args.asset_root.resolve())],'map-cure-contract')
            execute([str(build/'cure-battle-test'),'installed',str(args.asset_root.resolve())],'installed-cure')
            execute(['python3',str(ROOT/'tests/test-map-cure-window.py'),str(build/'campaign-window-test'),str(build/'mnm-map-campaign-cli'),str(build/'map-cure-test'),str(args.asset_root.resolve()),'--output',str(run/'cure-window')],'map-cure-window')
            cure_window=run/'cure-window/report.json';assert json.loads(cure_window.read_text())['success']
            result['cure_window']={'report':str(cure_window.relative_to(ROOT)),'sha256':sha(cure_window),'processes':5}
            sanitized=ROOT/'working/build/native-map-cure-sanitized'
            execute(['cmake','-S',str(ROOT/'apps/skirmish'),'-B',str(sanitized),'-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'],'sanitized-configure')
            execute(['cmake','--build',str(sanitized),'-j4','--target','map-cure-test','cure-battle-test','battle-effects-test','skirmish-mission-test'],'sanitized-build')
            env['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1';env['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1'
            execute(['ctest','--test-dir',str(sanitized),'--output-on-failure','-R','native-(battle-effects|cure-battle|skirmish-mission)$'],'sanitized-contracts')
            execute([str(sanitized/'map-cure-test'),str(args.asset_root.resolve())],'sanitized-map-cure')
            execute([str(sanitized/'cure-battle-test'),'installed',str(args.asset_root.resolve())],'sanitized-installed-cure')
            if args.legacy_cli:
                result['legacy_cli_sha256']=sha(args.legacy_cli)
                for command in ['create','start']:
                    for label,cli in [('old',args.legacy_cli),('new',build/'mnm-map-campaign-cli')]:
                        argv=[str(cli),str(args.asset_root.resolve()),command]
                        if command=='start':argv.append(str(run/(label+'-create.mnmc')))
                        execute(argv+[str(run/(label+'-'+command+'.mnmc'))],label+'-'+command)
                    assert (run/('old-'+command+'.mnmc')).read_bytes()==(run/('new-'+command+'.mnmc')).read_bytes()
                result['legacy_checkpoint_bytes_equal']=True
        if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise RuntimeError('Declared sources changed during execution')
        result['success']=True
    except Exception as e:result['error']=str(e)
    finally:
        try:execute(['sh',str(MAIN/'tools/original-manifest.sh'),'verify'],'manifest-after')
        except Exception as e:result['success']=False;result['error']=str(e)
    (run/'report.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'success':result['success'],'report':str((run/'report.json').relative_to(ROOT)),'error':result.get('error')}));return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
