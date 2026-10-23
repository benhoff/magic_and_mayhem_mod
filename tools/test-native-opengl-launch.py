#!/usr/bin/env python3
"""Bounded actual standalone launcher proof with owned graphics and native GPU frames."""
from coverage_evidence import write_report
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
import time

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import frame_v1

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--report',type=Path)
    p.add_argument('--prefix-template',type=Path,help='Clone a marked fixture prefix held by the supervising exclusive suite lease')
    p.add_argument('--build',type=Path,default=ROOT/'working/build/render-tests')
    p.add_argument('--source-game',type=Path,help='Pinned source installation under working/; preserve the default installation')
    a=p.parse_args();decl=json.loads(a.claims.read_text())
    for path,digest in decl['sources'].items():
        assert not Path(path).is_absolute() and (ROOT/path).resolve().is_relative_to(ROOT)
        assert sha(ROOT/path)==digest,path
    parent=ROOT/'working/tests/native-opengl-launch';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    helpers=load('native_campaign_helpers','tools/test-native-campaign.py')
    env=helpers.clean_environment(out)
    env.update(MNM_RENDER_CONTINUOUS='1',MNM_RENDER_ORDERED_COPIES='1',WINEDEBUG='-all')
    report=dict(schema=1,success=False,status='running',sources=decl['sources'],claims=decl['claims'],native_draw_default_required=True,
                scope='Actual standalone launcher main-menu startup/native GPU frames with real ddraw disabled; bounded cleanup, no gameplay/FPS/movies/normal Quit/COM release proof')
    server=probe=game=None;verified=False
    try:
        closure={}
        for dep in [*(a.build/'CMakeFiles/live-render-route-probe.dir').rglob('*.o.d'),*(a.build/'CMakeFiles/mnm-renderer.dir').rglob('*.o.d')]:
            for token in shlex.split(dep.read_text().replace('\\\n',' ')):
                path=Path(token)
                if path.is_absolute() and path.resolve().is_relative_to(ROOT) and not path.resolve().is_relative_to(ROOT/'working'):
                    name=path.resolve().relative_to(ROOT).as_posix();digest=sha(path)
                    assert decl['sources'].get(name)==digest,name;closure[name]=digest
        assert closure
        report['compiled_sources']=closure
        report['native_binary_sha256']=sha(a.build/'live-render-route-probe')
        if a.prefix_template:
            from drawing_fixture_prefix import borrow
            template,_=borrow(ROOT,a.prefix_template)
            subprocess.run(['cp','-a','--reflink=auto',str(template),str(out/'wineprefix')],check=True,timeout=120)
            # Prefix relocation must finish before the bounded launch begins.
            report['fixture_prefix']=dict(template=str(template),exclusive_supervisor_lease=True,private_clone=str(out/'wineprefix'))
        server=helpers.start_native_hardware(out,env)
        # Construct bootstrap only after hardware selection has removed software
        # overrides. A stale software selector cannot be combined with Zink.
        bootstrap=dict(env,WINEDLLOVERRIDES=(env.get('WINEDLLOVERRIDES','')+';mscoree,mshtml,winemenubuilder.exe,MicrosoftEdgeUpdate.exe='))
        with (out/'prefix-bootstrap.log').open('x') as log:
            subprocess.run(['wineboot','-u' if a.prefix_template else '-i'],env=bootstrap,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
            subprocess.run(['wineserver','-w'],env=bootstrap,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
        report['hardware_driver_sha256']=sha(out/'hardware-driver.log')
        with (out/'original-before.log').open('w') as log:
            subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],stdout=log,stderr=log,check=True)
        verified=True
        frame=out/'frame.bin'
        with frame.open('xb') as f:f.write(frame_v1.initial_header());f.truncate(frame_v1.SIZE)
        with (out/'probe.log').open('w') as log:
            probe=subprocess.Popen([str(a.build.resolve()/'live-render-route-probe'),str(out)],env=env,stdout=log,stderr=log,start_new_session=True)
        def wait_for(path,seconds,process):
            deadline=time.monotonic()+seconds
            while not path.exists():
                if process.poll() is not None or time.monotonic()>deadline:raise RuntimeError('No bounded output: '+str(path))
                time.sleep(.1)
        wait_for(out/'ready.json',20,probe)
        with (out/'launch.log').open('w') as log:
            command=['python3',str(ROOT/'tools/run-opengl-game.py'),'--stream',str(frame),'--command-channel',str(out/'commands.bin'),
                     '--render-control',str(out/'commands.bin.control'),'--skip-movies','--prefix',env['WINEPREFIX']]
            if a.source_game:
                source=a.source_game.resolve()
                if not source.is_relative_to(ROOT/'working') or not source.is_dir():raise ValueError('Source game must be under working/')
                command.extend(['--source-game',str(source)])
                report['source_game']=str(source)
            game=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=log,start_new_session=True)
        deadline=time.monotonic()+120;experiment=None
        while experiment is None:
            lines=(out/'launch.log').read_text().splitlines()
            roots=[Path(line.removeprefix('Render experiment: ')) for line in lines if line.startswith('Render experiment: ')]
            if len(roots)==1:experiment=roots[0];break
            if roots or game.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Standalone staging did not produce exactly one experiment')
            time.sleep(.1)
        assert experiment.resolve().is_relative_to(ROOT/'working/experiments/opengl-render')
        report['experiment']=str(experiment)
        metadata=json.loads((experiment/'manifest.json').read_text());assert metadata['native_draw'] is True
        (out/'attach').touch()
        wait_for(out/'progress.json',120,probe)
        deadline=time.monotonic()+30
        while True:
            state=json.loads((out/'progress.json').read_text())
            if state['frames']>=20:break
            if probe.poll() is not None or game.poll() is not None or time.monotonic()>deadline:raise RuntimeError('Native standalone frames stalled')
            time.sleep(.1)
        assert state['recoveries']==0 and not state['error'] and 'NVIDIA TITAN RTX' in state['renderer']
        assert not state['ordinary_native_readbacks'] and not state['ordinary_rgba_readbacks'] and not state['viewport_uploads']
        report['native_progress']=state
        def request(ordinal,operation):
            temporary=out/'request.tmp';temporary.write_text(json.dumps(dict(id=ordinal,op=operation)));temporary.replace(out/'request.json')
            reply=out/f'reply-{ordinal:08d}.json';wait_for(reply,10,probe);return json.loads(reply.read_text())
        capture=request(1,'snapshot');assert (out/capture['image']).stat().st_size>0
        report['image_sha256']=sha(out/capture['image'])
        report['native_draw']=helpers.validate_native_draw(experiment)
        for name in ('native-draw-build.json','bridge-build.json'):
            manifest=json.loads((experiment/name).read_text())
            for path,digest in manifest['sources'].items():
                canonical=path if '/' in path else 'runtime/render/'+path
                assert report['sources'].get(canonical)==digest==sha(ROOT/canonical),canonical
            for field in ('compiled_sources','protocol_sources'):
                for path,digest in manifest.get(field,{}).items():
                    assert report['sources'].get(path)==digest==sha(ROOT/path),path
            if name=='bridge-build.json':assert manifest.get('compiled_sources')
        inputs=json.loads((experiment/'input-fingerprints.json').read_text())
        assert all(sha(experiment/'game'/path)==digest for path,digest in inputs.items())
        report['inputs']=inputs;report['inputs_stable']=True
        terminal=request(2,'finish');assert terminal['success'] and terminal['terminal_resources']==0
        probe.wait(timeout=10);assert probe.returncode==0
        report['consumer_terminal_resources']=0
        report.update(success=True,status='passed')
    except (AssertionError,ValueError,OSError,RuntimeError,subprocess.SubprocessError) as error:
        report.update(status='failed',error=str(error))
    finally:
        for process in (probe,game):helpers.stop_group(process)
        subprocess.run(['wineserver','-k'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10,check=False)
        subprocess.run(['wineserver','-w'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=15,check=True)
        helpers.stop_group(server)
        if verified:
            with (out/'original-after.log').open('w') as log:
                subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],stdout=log,stderr=log,check=True)
            report['original_manifest_verified_after']=True
        report['sources_unchanged']=all(sha(ROOT/path)==digest for path,digest in report['sources'].items())
        if not report['sources_unchanged']:report.update(success=False,status='failed',error='Source changed during proof')
        write_report(ROOT,a.report or out/'report.json',(json.dumps(report,indent=2)+'\n').encode())
    print(report['status'],report.get('error',''),flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
