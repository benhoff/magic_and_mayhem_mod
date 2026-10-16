#!/usr/bin/env python3
"""Reproduce terrain-bound Sphere/Cure campaign continuation and real Qt journeys."""
import argparse,hashlib,importlib.util,json,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MAIN=Path(subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--path-format=absolute','--git-common-dir'],text=True).strip()).parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('assets',type=Path);p.add_argument('--output',type=Path,required=True);p.add_argument('--sanitized-build',type=Path);p.add_argument('--legacy-map',type=Path,action='append',default=[]);p.add_argument('--legacy-arena',type=Path,action='append',default=[]);a=p.parse_args();a.output=a.output.resolve();a.assets=a.assets.resolve();a.output.mkdir(parents=True,exist_ok=False);(a.output/'.retain').write_text('Source-bound Sphere/map/Cure execution and window evidence.\n');(a.output/'tmp').mkdir()
    spec=importlib.util.spec_from_file_location('claims',ROOT/'tools/draft-coverage-claims.py');claims=importlib.util.module_from_spec(spec);spec.loader.exec_module(claims)
    behaviors=['NP.campaign-persistence','NP.campaign-session','NP.campaign-arena','NP.campaign-window','NP.skirmish-simulation','NP.skirmish-window','NP.skirmish-mission','NP.map-campaign','NP.map-campaign-window','NP.battle-effects','NP.cure-battle','NP.sphere-battle']
    scenarios=['NP.campaign-persistence.restart','NP.campaign-session.transaction','NP.campaign-arena.journey','NP.campaign-window.journey','world.native-skirmish','window.native-skirmish','NP.skirmish-mission.continuation','NP.map-campaign.journey','NP.map-campaign-window.journey','BATTLE01.sections','BATTLE02.cure-playable','NP.map-cure.continuation','PROJECTILE03.battle','PROJECTILE04.map-core','PROJECTILE04.map-window','PROJECTILE04.window']
    declaration=claims.declaration(ROOT,json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),behaviors,scenarios);(a.output/'claims.json').write_text(json.dumps(declaration,indent=2)+'\n')
    report=dict(schema=1,scope='Native authored Plains8x8x23 Sphere/Cure profile bound to MAP/TTD/SPR and complete owned projectile environment; arena finale remains a distinct8x8x3 open profile. Authored cost8/damage20/speed20/lifetime100 and Cure3/8 unchanged; moving hit/miss, simultaneous effects, terrain retirement, exact restart, real Qt win/reopen/advance and legacy profiles. No original full campaign/ricochet/catalog profile/physical audio/SPR/live replacement claim.',claims=declaration['claims'],sources=declaration['sources'],commands=[],success=False,legacy_checkpoints=[])
    env=dict(os.environ,TMPDIR=str(a.output/'tmp'),XDG_CONFIG_HOME=str(a.output/'qt-config'),QT_QPA_PLATFORM='xcb',QT_STYLE_OVERRIDE='Fusion',QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0',MNM_CAMPAIGN_JOURNEY_OUTPUT=str(a.output/'arena-journey'),MNM_CAMPAIGN_WINDOW_OUTPUT=str(a.output/'arena-window'),MNM_MAP_CAMPAIGN_OUTPUT=str(a.output/'map-journey'),MNM_MAP_CAMPAIGN_WINDOW_OUTPUT=str(a.output/'map-window'))
    def run(name,argv):
        argv=list(map(str,argv));log=a.output/(name+'.log')
        with log.open('w') as out:r=subprocess.run(argv,cwd=ROOT,env=env,stdout=out,stderr=subprocess.STDOUT,timeout=600)
        report['commands'].append(dict(argv=argv,exit_code=r.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)))
        if r.returncode:raise RuntimeError('Validation failed: '+str(log))
    build=ROOT/'working/build/native-map-campaign';sanitized=a.sanitized_build.resolve() if a.sanitized_build else ROOT/'working/build/native-map-cure-sanitized'
    try:
        run('manifest-before',[MAIN/'tools/original-manifest.sh','verify'])
        run('configure',['cmake','-S',ROOT/'apps/skirmish','-B',build,'-DCMAKE_BUILD_TYPE=Debug']);run('build',['cmake','--build',build,'-j4'])
        for name,args in [('map-sphere',[build/'map-sphere-test',a.assets]),('sphere',[build/'sphere-battle-test']),('map-cure',[build/'map-cure-test',a.assets]),('presentation',[build/'map-presentation-test',a.assets]),('installed-cure',[build/'cure-battle-test','installed',a.assets])]:run(name,args)
        run('sphere-window',['python3',ROOT/'tests/test-map-sphere-window.py',build/'campaign-window-test',build/'mnm-map-campaign-cli',build/'map-sphere-test',a.assets,'--output',a.output/'sphere-window'])
        run('arena-sphere-window',['python3',ROOT/'tests/test-sphere-window.py',build/'campaign-window-test',build/'mnm-campaign-cli',build/'sphere-battle-test','--output',a.output/'arena-sphere-window'])
        run('map-journey',['python3',ROOT/'tests/test-map-campaign.py',build/'mnm-map-campaign-cli',build/'map-campaign-test',a.assets])
        run('map-window',['python3',ROOT/'tests/test-map-campaign-window.py',build/'campaign-window-test',build/'mnm-map-campaign-cli',build/'map-campaign-test',a.assets])
        run('cure-map-window',['python3',ROOT/'tests/test-map-cure-window.py',build/'campaign-window-test',build/'mnm-map-campaign-cli',build/'map-cure-test',a.assets,'--output',a.output/'cure-map-window'])
        run('ctest',['ctest','--test-dir',build,'--output-on-failure'])
        run('sanitized-configure',['cmake','-S',ROOT/'apps/skirmish','-B',sanitized,'-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])
        run('sanitized-build',['cmake','--build',sanitized,'-j4','--target','map-sphere-test','sphere-battle-test','map-cure-test','cure-battle-test','battle-effects-test','skirmish-mission-test'])
        env['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1';env['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1'
        run('sanitized-contracts',['ctest','--test-dir',sanitized,'--output-on-failure','-R','native-(sphere-battle|cure-battle|battle-effects|skirmish-mission)$'])
        run('sanitized-map-sphere',[sanitized/'map-sphere-test',a.assets]);run('sanitized-map-cure',[sanitized/'map-cure-test',a.assets]);run('sanitized-installed-cure',[sanitized/'cure-battle-test','installed',a.assets])
        for kind,paths in [('map',a.legacy_map),('arena',a.legacy_arena)]:
            for i,path in enumerate(paths):
                original=path.read_bytes();target=a.output/f'legacy-{kind}-{i}.mnmc';argv=[build/('mnm-map-campaign-cli' if kind=='map' else 'mnm-campaign-cli')]
                if kind=='map':argv.append(a.assets)
                run(f'legacy-{kind}-{i}',argv+['resume',path.resolve(),target,0]);assert original==target.read_bytes();report['legacy_checkpoints'].append(dict(kind=kind,path=str(path.resolve()),sha256=hashlib.sha256(original).hexdigest(),exact_bytes=True))
        for name in ['sphere-window','map-window','map-journey','cure-map-window','arena-sphere-window']:
            path=a.output/name/'report.json';details=json.loads(path.read_text());assert details['success'];report[name]=dict(record=str(path.relative_to(ROOT)),sha256=sha(path),scope=details['scope'])
        assert all(sha(ROOT/p)==h for p,h in declaration['sources'].items()),'Declared sources changed during execution'
        report['success']=True
    except Exception as e:report['error']=str(e)
    finally:
        try:run('manifest-after',[MAIN/'tools/original-manifest.sh','verify'])
        except Exception as e:report['success']=False;report['error']=str(e)
    (a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],report=str((a.output/'report.json').relative_to(ROOT)),error=report.get('error'))));return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
