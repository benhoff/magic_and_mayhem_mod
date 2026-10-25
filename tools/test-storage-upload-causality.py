#!/usr/bin/env python3
"""Run bounded owned public-SHM source controls; retain SQLite evidence."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys

import numpy as np

from coverage_evidence import write_report
from storage_upload_causality import admit_namespace, bind, decode, difference, image, rgb565, samples

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--isolation-record',type=Path,required=True)
    a=p.parse_args()
    out=a.output.absolute()
    if out.exists() or a.report.exists() or not out.is_relative_to(ROOT/'working'):
        p.error('Require new owned output/report paths')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain public SHM forwarding/pixel controls and failures.\n')
    declaration=json.loads(a.claims.read_bytes());sources=declaration['sources']
    stable=lambda:all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
    if not stable():raise ValueError('Prospective source changed')
    result=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],commands=[],
                original_execution=False,original_pixels_used_as_native_inputs=False,
                scope='Owned SHM and EGL controls with exact raw sampled image planes and forwarding output. '
                      'No original-game, private allocator or whole-renderer equivalence claim.')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.pop('LD_PRELOAD',None)
    env.update(LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4',TMPDIR=str(ROOT/'working/tmp'))
    def run(label,command,extra=None):
        r=subprocess.run(list(map(str,command)),cwd=ROOT,env=dict(env,**(extra or {})),capture_output=True,timeout=120)
        data=r.stdout+b'\n'+r.stderr;(out/(label+'.log')).write_bytes(data)
        result['commands'].append(dict(label=label,returncode=r.returncode,argv=list(map(str,command)),diagnostic=data.decode(errors='replace')))
        if r.returncode:raise ValueError(label+' failed: '+str(r.returncode))
        return r.stdout
    try:
        result['isolation']=admit_namespace(ROOT,a.isolation_record)
        spec=importlib.util.spec_from_file_location('shm_build',ROOT/'tools/build-storage-upload-observer.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        library,helper,manifest=module.build()
        result['build']=manifest
        run('decoder-guards',[sys.executable,'-B',ROOT/'tests/test-storage-upload-causality.py'])
        mock=out/'libforwarding.so'; caller=out/'forwarding'
        run('forwarding-backend-build',['gcc','-std=c11','-O2','-shared','-fPIC','-DBACKEND',
            '-Wall','-Wextra','-Werror',ROOT/'tests/shm-source-forwarding-test.c','-o',mock])
        run('forwarding-caller-build',['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',
            ROOT/'tests/shm-source-forwarding-test.c',mock,'-Wl,-rpath,'+str(out),'-o',caller])
        control=run('forwarding-control',[caller])
        refused=out/'forwarding-refusals';refused.mkdir()
        observed=run('forwarding-observed',['env','LD_PRELOAD='+str(helper),
            'MNM_DRAW_X11_DIRECTORY='+str(refused),caller])
        negatives=[decode(p.read_bytes()) for p in sorted(refused.glob('*.bin'))]
        if (observed!=control or len(negatives)!=2 or any(r['status']!=2 for r in negatives) or
            [(r['cookie'],r['errno_before'],r['errno_after']) for r in negatives]!=
                [(0x12345678,101,119),(0x89abcdef,102,120)]):
            raise ValueError('Unknown-segment refusal changed original forwarding/cookie/errno')
        result['forwarding_refusals']=negatives
        for label,name,libs in [('shm','drawing-egl-xcb-crop-test.c',['-lxcb','-lxcb-shm']),
                                ('egl','drawing-egl-resize-forwarding-test.c',['-lEGL','-lGL','-lX11'])]:
            binary=out/label
            run(label+'-build',['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',ROOT/'tests'/name,*libs,'-o',binary])
            control=run(label+'-control',['xvfb-run','-a','-s','-screen 0 1280x1024x24',binary])
            directory=out/(label+'-samples');directory.mkdir()
            extra=dict(LD_PRELOAD=str(helper if label=='shm' else library),MNM_DRAW_X11_DIRECTORY=str(directory))
            if label=='egl':extra.update(MNM_DRAW_EGL_SWAP_COMPLETION='1',MNM_DRAW_EGL_GPU_DESTINATION_WITNESS='1',MNM_DRAW_EGL_XCB_IMAGE_WITNESS='1')
            # Preload only the fixture, never Xvfb or its launcher.
            observed=run(label+'-observed',['xvfb-run','-a','-s','-screen 0 1280x1024x24','env',*[k+'='+v for k,v in extra.items()],binary])
            if control!=observed:raise ValueError(label+' forwarding output changed')
            kept=samples(directory)
            rows=[]
            for sample in kept:
                w,h=sample['width'],sample['height'];y,x=np.indices((h,w))
                expected=(((y%31)<<11)|((x%63)<<5)|((y+x)%31)).astype('<u2')
                plane=image(sample);a0=sample['args'];visible=plane[a0[4]:a0[4]+h,a0[3]:a0[3]+w]
                row={k:v for k,v in sample.items() if k!='data'}
                row['known_pattern_mismatches']=difference(visible,rgb565(expected));rows.append(row)
                if label=='egl':
                    from drawing_egl_destination import decode_xcb
                    packets=[decode_xcb(p.read_bytes()) for p in directory.glob('egl-xcb-image-*.bin')]
                    packet=bind(sample,dict(pid=sample['pid'],thread=sample['thread'],width=w,height=h,xcb_images=packets))
                    row['forwarded_cookie_bound']=packet['cookie_sequence']==sample['cookie']
            if label=='shm':
                output=json.loads(control)
                if len(rows)!=6 or rows[0]['known_pattern_mismatches']!=output['stale_uncorrected'] or not output['stale_uncorrected'] or any(r['known_pattern_mismatches'] for r in rows[1:]):
                    raise ValueError('Owned stale/fresh SHM image model differs')
            elif len(rows)!=7 or any(r['known_pattern_mismatches'] for r in rows):
                raise ValueError('Owned EGL source/physical pattern differs')
            result[label]=dict(output=control.decode(),samples=rows)
        result['success']=True
    except Exception as error:
        result['error']=repr(error)
    finally:
        if result.get('success'):
            try:admit_namespace(ROOT,a.isolation_record)
            except Exception as error:result.update(success=False,error=repr(error))
        result['sources_stable']=stable();result['success'] &= result['sources_stable']
        result['artifacts']=out.relative_to(ROOT).as_posix()
        write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ('success','error','sources_stable')}))
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
