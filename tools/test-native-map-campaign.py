#!/usr/bin/env python3
"""Reproduce the bounded installed-map native campaign under a storage guard."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MAIN=Path(subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--path-format=absolute','--git-common-dir'],text=True).strip()).parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('asset_root',type=Path);args=parser.parse_args()
    parent=ROOT/'working/tests/map-campaign';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Installed crop campaign execution evidence.\n')
    spec=importlib.util.spec_from_file_location('claims',ROOT/'tools/draft-coverage-claims.py');claims=importlib.util.module_from_spec(spec);spec.loader.exec_module(claims)
    behaviors=['NP.map-campaign','NP.skirmish-mission','NP.campaign-arena']
    scenarios=['NP.map-campaign.journey','NP.skirmish-mission.continuation','NP.campaign-arena.journey']
    declaration=claims.declaration(ROOT,json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),behaviors,scenarios)
    (run/'claims.json').write_text(json.dumps(declaration,indent=2)+'\n');build=ROOT/'working/build/native-map-campaign'
    commands=[['cmake','-S',str(ROOT/'apps/skirmish'),'-B',str(build),'-DCMAKE_BUILD_TYPE=Debug'],['cmake','--build',str(build),'-j4'],['ctest','--test-dir',str(build),'--output-on-failure','-E','native-(campaign|skirmish)-window'],['python3',str(ROOT/'tests/test-map-campaign.py'),str(build/'mnm-map-campaign-cli'),str(build/'map-campaign-test'),str(args.asset_root.resolve())]]
    result={'schema':1,'scope':'Native authored Plains 8x8 sealed crop campaign, owned resource-bound restart/movement/casting and progression to the arena; original campaign and full-map equivalence excluded.','claims':declaration['claims'],'sources':declaration['sources'],'commands':[],'success':False}
    env=os.environ.copy();env['XDG_CONFIG_HOME']=str(run/'qt-config');env['QT_QPA_PLATFORM']='xcb';env['QT_STYLE_OVERRIDE']='Fusion';env['MNM_MAP_CAMPAIGN_OUTPUT']=str(run/'journey');env['MNM_CAMPAIGN_JOURNEY_OUTPUT']=str(run/'arena');env['MNM_CAMPAIGN_WINDOW_OUTPUT']=str(run/'arena-window')
    def execute(command,name):
        log=run/(name+'.log')
        with log.open('w') as output:r=subprocess.run(command,cwd=ROOT,env=env,stdout=output,stderr=subprocess.STDOUT)
        result['commands'].append({'argv':command,'exit_code':r.returncode,'log':str(log.relative_to(ROOT)),'sha256':sha(log)})
        if r.returncode:raise RuntimeError('Validation failed: '+str(log))
    try:
        execute(['sh',str(MAIN/'tools/original-manifest.sh'),'verify'],'manifest-before')
        for i in [0,1,3,2]:execute(commands[i],'command-'+str(i))
        if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise RuntimeError('Declared sources changed during execution')
        journey=run/'journey/report.json';details=json.loads(journey.read_text());assert details['success']
        result['journey']={'report':str(journey.relative_to(ROOT)),'sha256':sha(journey),'asset_sha256':details['checks']['assets'],'refused_resources':details['refused_resources'],'refused_checkpoints':details['checks']['refused_checkpoints']}
        result['success']=True
    except Exception as e:result['error']=str(e)
    finally:
        try:execute(['sh',str(MAIN/'tools/original-manifest.sh'),'verify'],'manifest-after')
        except Exception as e:result['success']=False;result['error']=str(e)
    (run/'report.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'success':result['success'],'report':str((run/'report.json').relative_to(ROOT)),'error':result.get('error')}));return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
