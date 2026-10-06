#!/usr/bin/env python3
"""Offline recovered retry replay and native undefined-content policy tests."""
import argparse,importlib.util,json,os,shlex,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('retry',ROOT/'tools/check-original-surface-retry.py');r=importlib.util.module_from_spec(s);s.loader.exec_module(r)

def check(build,report):
    rows,m=r.load();parent=ROOT/'working/tests/native-surface-retry';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    sources=['reconstruction/rendering/surface_retry.hpp','tests/surface-retry-model.cpp','tools/check-native-surface-retry.py','tools/check-original-surface-retry.py','renderer/known_pixels.hpp','renderer/blit.cpp','renderer/blit.hpp','tests/render-restoration-test.cpp','tests/fixtures/surfaces/'+m['path'],str(r.DEFAULT.relative_to(ROOT))];fingerprints={n:r.sha(ROOT/n) for n in sources}
    model=run/'model';subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-pedantic',str(ROOT/sources[1]),'-o',str(model)],check=True)
    cases=run/'inputs.txt';cases.write_text(''.join(' '.join(str(v) for v in [*(c[k] for k in ['id','entry','fast','no_wait','key_enabled','reload_bits','source_restore','destination_restore','key_result','color']),len(c['draw_results']),*c['draw_results']])+'\n' for c in (row['input'] for row in rows)))
    result=subprocess.run([str(model),str(cases)],capture_output=True,text=True,check=True,timeout=30);(run/'native-traces.json').write_text(result.stdout);actual=json.loads(result.stdout)
    for row,out in zip(rows,actual,strict=True):r.require(out['returned'] and not out['budget_exhausted'] and {k:out[k] for k in ['id','draws_consumed','events']}==row['original'],'Portable recovered trace mismatch')
    result=subprocess.run([str(model),str(cases),'2'],capture_output=True,text=True,check=True,timeout=30);bounded=json.loads(result.stdout);exhausted=0
    for row,out in zip(rows,bounded,strict=True):
        r.require(out['draws_consumed']<=2 and out['events']==row['original']['events'][:len(out['events'])],'Bounded prefix mismatch')
        if out['budget_exhausted']:r.require(not out['returned'] and out['draws_consumed']==2,'Budget disguised as success');exhausted+=1
    r.require(exhausted>0,'Missing retry budget tests')
    for budget in ['0','65537']:
        out=subprocess.run([str(model),str(cases),budget],capture_output=True,text=True,timeout=30);r.require(out.returncode!=0,'Invalid budget accepted')
    binary=run/'validity';qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','Qt6OpenGL'],text=True));subprocess.run(['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'renderer'),str(ROOT/'tests/render-restoration-test.cpp'),str(build/'libmnm-renderer.a'),*qt,'-o',str(binary)],check=True)
    output=subprocess.run([str(binary)],env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1'),capture_output=True,text=True,timeout=60);(run/'validity.log').write_text(output.stdout+output.stderr);r.require(output.returncode==0,output.stdout+output.stderr);validity=json.loads(output.stdout)
    r.require(all(r.sha(ROOT/n)==h for n,h in fingerprints.items()),'Sources changed during execution')
    data=dict(schema=1,success=True,original_sha256=r.BUILD_HASH,sources=fingerprints,cases=len(rows),matched=len(actual),bounded_cases=len(bounded),budget_exhaustions=exhausted,invalid_budgets_refused=2,validity=validity,binary_sha256=r.sha(model),validity_binary_sha256=r.sha(binary),artifacts=str(run.relative_to(ROOT)),real_driver_validation=False,scope='Portable C++ recovered scheduling matches4480 unchanged original/scripted fault traces; bounded execution preserves original prefixes and reports exhaustion separately. Native GL undefined-content policy enforces overwritten-region validity; no original restored pixel/driver/palette/reloader, live recovery or new wire contract equivalence.')
    with report.open('x') as f:json.dump(data,f,indent=2);f.write('\n')
    print(json.dumps({k:data[k] for k in ['success','cases','budget_exhaustions','validity']}));return data
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/renderer');p.add_argument('--report',type=Path,required=True);a=p.parse_args();check(a.build.resolve(),a.report)
