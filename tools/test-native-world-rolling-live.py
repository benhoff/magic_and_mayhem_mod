#!/usr/bin/env python3
"""Exercise the guarded original32-queue producer through ordered native rolling delivery."""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import statistics
import subprocess
import tempfile
import time
from coverage_claims import behavior_contract, required_sources, scenario_contract
ROOT = Path(__file__).resolve().parents[1]
BEHAVIOR = 'NR.world-rolling-live-handoff'
SCENARIO = 'native-world-rolling-live-handoff-20261010'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prepared',type=Path,help='Reuse a source/binary/dependency-verified native build')
    p.add_argument('--mode', choices=['complete', 'consumer-death', 'producer-death'], default='complete')
    p.add_argument('--corpus', type=Path, required=True, help='Complete original V3 batch capture for admission fixtures')
    a = p.parse_args()
    parent=ROOT/'working/tests/native-world-rolling-live';parent.mkdir(parents=True,exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    r = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    b = next(x for x in r['behaviors'] if x['id'] == BEHAVIOR)
    s = next(x for x in r['scenarios'] if x['id'] == SCENARIO)
    sources = {n: sha(ROOT/n) for n in sorted(required_sources(b))}
    claims = [dict(behavior=BEHAVIOR, contract_sha256=behavior_contract(b, {x['id']:x for x in r['builds']}), scenarios={SCENARIO:scenario_contract(s)})]
    declaration = out/'claims.json';declaration.write_text(json.dumps(dict(sources=sources,claims=claims),indent=2)+'\n')
    report = dict(success=False, mode=a.mode, sources=sources, claims=claims, scope=b['scope'], original_pixels_used_as_native_inputs=False)
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='8')
    def run(name, cmd, **kw):
        with (out/(name+'.log')).open('x') as log:
            return subprocess.run([str(x) for x in cmd], cwd=ROOT, env=env, stdout=log,stderr=subprocess.STDOUT,timeout=600,**kw)
    process = None; experiment = None; before = False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
        frozen=out/'source'
        configured={n for n in subprocess.check_output(['git','ls-files','-z'],cwd=ROOT).decode().split('\0') if n and (Path(n).suffix in ('.cpp','.hpp','.c','.h','.S','.cmake') or Path(n).name=='CMakeLists.txt')}
        for n in sorted(configured|set(sources)):
            target=frozen/n;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/n,target)
        report['frozen_sources']={n:sha(frozen/n) for n in configured|set(sources)}
        build=out/'build';dependency_root=frozen
        if a.prepared:
            prepared=a.prepared.resolve();previous=json.loads((prepared/'report.json').read_text())
            assert previous['success'] and previous['sources_stable'] and previous['frozen_sources_stable']
            build=prepared/'build';dependency_root=prepared/'source'
            assert sha(build/'mnm-world-rolling-live')==previous['native_binary_sha256']
            for n,h in previous['compiled_dependency_review']['sources'].items():
                assert sha(ROOT/n)==sha(dependency_root/n)==h,n
            report['prepared_report_sha256']=sha(prepared/'report.json')
        else:
            run('configure',['cmake','-S',frozen/'compat/legacy/world-rolling','-B',build,'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON'],check=True)
            run('build',['cmake','--build',build,'--target','mnm-world-rolling-live','world-rolling-handoff-test','-j4'],check=True)
        compiled=set()
        for dep in build.rglob('*.o.d'):
            for token in dep.read_text().replace('\\\n',' ').split():
                path=Path(token).resolve()
                if path.is_file() and path.is_relative_to(dependency_root):compiled.add(path.relative_to(dependency_root).as_posix())
        missing=sorted(compiled-set(sources));report['compiled_dependency_review']=dict(sources={n:sha(dependency_root/n) for n in sorted(compiled)},undeclared_dependencies=missing)
        assert not missing,missing
        corpus=a.corpus.resolve();inputs={str(corpus/'canvas-producers.bin'):sha(corpus/'canvas-producers.bin')}
        for path in sorted(corpus.glob('world-batch-*.request')):inputs[str(path)]=sha(path)
        run('handoff-fixtures',[build/'world-rolling-handoff-test',corpus],check=True);report['handoff_fixtures_passed']=True
        report['native_binary_sha256']=sha(build/'mnm-world-rolling-live')
        report['handoff_fixture_binary_sha256']=sha(build/'world-rolling-handoff-test')
        command=['xvfb-run','-a','-s','-screen 0 1280x1024x24','python3',ROOT/'tools/capture-scene-game.py','--canvas-producers','--world-producer-handoff','--samples','32','--world-raster-prefix','32','--world-raster-batch','--producer-oracle-mib','3072','--skip-window-screenshot','--map','2','--magic-items','0','--producer-live',build/'mnm-world-rolling-live','--claims',declaration]
        with (ROOT/'working/rolling-live-staging.lock').open('a') as gate, (out/'capture.log').open('x') as log:
            # PE32 preparation uses shared build outputs; serialize live staging.
            fcntl.flock(gate,fcntl.LOCK_EX)
            process=subprocess.Popen([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            deadline=time.monotonic()+600; injected=False;fault_queue=None
            while process.poll() is None:
                lines=(out/'capture.log').read_text().splitlines()
                roots=[Path(x) for x in lines if x.startswith(str(ROOT/'working/experiments/scene-observer/run-')) and Path(x).is_dir()]
                if roots:experiment=roots[-1]
                if a.mode!='complete' and experiment and not injected:
                    ready=experiment/'native-producers/ready'
                    # Terminate only this experiment's native consumer or private Wine server.
                    pending=[x for x in (experiment/'capture').glob('world-batch-*.request') if int(x.stem.split('-')[-1])>=2 and not x.with_suffix('.reply').exists()]
                    if ready.exists() and (experiment/'capture/world-batch-0001.reply').exists() and pending:
                        fault_queue=int(pending[-1].stem.split('-')[-1])
                        if a.mode=='consumer-death':os.kill(json.loads(ready.read_text())['pid'],signal.SIGKILL)
                        else:subprocess.run(['wineserver','-k'],env=dict(env,WINEPREFIX=str(experiment/'wineprefix')),check=True,timeout=10)
                        injected=True
                if time.monotonic()>deadline:raise RuntimeError('Rolling live deadline')
                time.sleep(.1)
        report.update(exit_code=process.returncode,experiment=str(experiment),fault_injected=injected,fault_queue=fault_queue)
        if a.mode!='complete':
            assert injected and process.returncode!=0
            text=(out/'capture.log').read_text()
            expected='Native producer viewer failed' if a.mode=='consumer-death' else 'Original game exited'
            assert expected in text, 'Missing expected peer-death refusal'
            assert not (experiment/'capture/canvas-producers.done').exists()
            subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(experiment/'wineprefix')),check=True,timeout=10)
            report.update(expected_terminal_failure=True,capture_log_sha256=sha(out/'capture.log'),private_wine_stopped=True,completed_success_claimed=False,success=True)
        else:
            assert process.returncode==0, 'Original/native capture failed; inspect '+str(out/'capture.log')
            capture=json.loads((experiment/'report.json').read_text());native=capture['native_producers']
            assert capture['success'] and capture['canvas_guard_verified'] and capture['native_canvas_writebacks']==32
            assert native['rolling_adapter'] and native['queues']==native['world_queues']==32 and native['remaining_surfaces']==0 and native['viewport_image_uploads']==0
            assert all(x['mismatches']==0 for x in capture['producer_comparison'])
            work=[x['native_work_ms'] for x in native['world_frames']]
            diagnostic=[x['checkpoint_diagnostics_ms'] for x in native['world_frames']]
            delivery=[x['request_to_reply_ms']-diagnostic[i] for i,x in enumerate(native['deliveries'])]
            limits=dict(native_warm_median_ms=statistics.median(work[1:]),native_warm_limit_ms=60,native_cold_ms=work[0],native_cold_limit_ms=500,delivery_warm_median_ms=statistics.median(delivery[1:]),delivery_warm_limit_ms=120)
            limits['passed']=limits['native_warm_median_ms']<=60 and limits['native_cold_ms']<=500 and limits['delivery_warm_median_ms']<=120
            report.update(completed=capture,performance_budget=limits)
            assert limits['passed'],'Rolling live native/delivery latency budget exceeded'
            report['success']=True
        report.update(inputs=inputs,inputs_stable=all(sha(Path(n))==h for n,h in inputs.items()))
    except Exception as e:report.update(success=False,error=str(e))
    finally:
        if process and process.poll() is None:
            os.killpg(process.pid,signal.SIGINT)
            try:process.wait(timeout=20)
            except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
        if before:run('original-after',[ROOT/'tools/original-manifest.sh','verify'],check=True)
        report.update(original_manifest_verified_before_after=before,sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()),frozen_sources_stable=all(sha(out/'source'/n)==h for n,h in report.get('frozen_sources',{}).items()))
        report['success']=report['success'] and report['sources_stable'] and report['frozen_sources_stable'] and report.get('inputs_stable',False)
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
