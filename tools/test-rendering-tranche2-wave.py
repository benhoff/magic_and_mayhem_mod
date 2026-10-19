#!/usr/bin/env python3
"""Compare complete original, native CPU and native GPU wave fixture canvases."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--reference-report',type=Path,required=True)
    p.add_argument('--build',type=Path,required=True)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();a.build=a.build.resolve();out=a.output.resolve();out.mkdir(exist_ok=False,parents=True)
    declared=json.loads(a.claims.read_text());sources=declared['sources']
    if any(sha(ROOT/n)!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    report=dict(schema=1,success=False,sources=sources,claims=declared['claims'],
        scope='Selected original5806f0 cold/warm initializer and independently generated offsets,768 full opaque/split-run clipping/phase canvases compared byte for byte with CPU and GPU. No original destination seeding, physical driver equivalence or full dispatcher coverage.',
        original_pixels_used_as_native_inputs=False,live_replacement=False)
    inputs={}
    def pin(path):inputs[str(path.resolve())]=sha(path)
    def run(name,command):
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=ROOT,env=dict(os.environ,LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4'),stdout=log,stderr=subprocess.STDOUT,timeout=180,check=True)
        return json.loads((out/(name+'.log')).read_text())
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        pin(a.reference_report);reference=json.loads(a.reference_report.read_text())
        if not reference['success'] or not reference['sources_stable']:raise ValueError('Original reference did not pass')
        if any(sha(ROOT/n)!=h for n,h in reference['sources'].items()):raise ValueError('Original fixture sources changed')
        original=Path(reference['wave_pixels']['path']);pin(original)
        if sha(original)!=reference['wave_pixels']['sha256']:raise ValueError('Original pixels changed')
        wave=next(r for r in reference['results'] if r['case']=='wave-initializer')
        if wave['effective_pixel_cases']!=768 or original.stat().st_size!=768*64*32*2:raise ValueError('Incomplete original wave matrix')
        for name in ('rendering-tranche2-test','rendering-tranche2-gpu-test'):pin(a.build/name)
        cpu=run('cpu',[a.build/'rendering-tranche2-test','--wave-pixels',out/'cpu.565'])
        gpu=run('gpu',['xvfb-run','-a',a.build/'rendering-tranche2-gpu-test','--wave-pixels',out/'gpu.565'])
        if not cpu['success'] or not gpu['success'] or gpu['remaining_surfaces']:raise ValueError('Native fixture failed')
        old=original.read_bytes();actual=(out/'cpu.565').read_bytes();rendered=(out/'gpu.565').read_bytes()
        report.update(cases=768,words_per_backend=len(old)//2,cpu_equal=actual==old,gpu_equal=rendered==old,
            table_equal=cpu['wave_table']==wave['initialized_table'],cpu=cpu,gpu=gpu,
            output_hashes={'original':sha(original),'cpu':sha(out/'cpu.565'),'gpu':sha(out/'gpu.565')})
        report['success']=report['cpu_equal'] and report['gpu_equal'] and report['table_equal']
        if not report['success']:report['error']='Original/CPU/GPU complete words differ'
    except Exception as e:report['error']=str(e)
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
        report.update(original_manifest_verified_before_after=True,inputs=inputs,
            inputs_stable=all(sha(Path(n))==h for n,h in inputs.items()),sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()))
        report['success'] &= report['inputs_stable'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps({k:report.get(k) for k in ('success','error','cases','cpu_equal','gpu_equal','table_equal')}))
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
