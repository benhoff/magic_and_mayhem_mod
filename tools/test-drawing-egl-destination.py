#!/usr/bin/env python3
"""Compare owned EGL resize controls with and without destination readback."""
import argparse,json,os,subprocess,tempfile
from pathlib import Path
import numpy as np
from drawing_corpus import ROOT,sha
from drawing_egl_destination import decode,decode_rebind,decode_xcb,plane


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    if args.report.exists():parser.error('Refusing evidence overwrite')
    claims=json.loads(args.claims.read_text());sources=claims['sources']
    if any(sha((ROOT/p).read_bytes())!=h for p,h in sources.items()):raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/egl-destination-controls';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));binary=run/'fixture';library=ROOT/'working/build/drawing-egl-delivery/libmnm-drawing-egl-delivery.so'
    subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-egl-resize-forwarding-test.c'),'-lEGL','-lGL','-lX11','-o',str(binary)],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('LD_PRELOAD',None);env.update(LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4')
    results={};planes=[]
    for mode in ('control','completion','destination','rebind','release','xcb','crop'):
        directory=run/mode;directory.mkdir();command=['xvfb-run','-a','-s','-screen 0 1280x1024x24']
        if mode!='control':command+=['env','LD_PRELOAD='+str(library),'MNM_DRAW_X11_DIRECTORY='+str(directory),'MNM_DRAW_EGL_SWAP_COMPLETION=1']
        if mode in ('destination','rebind','release','xcb','crop'):command+=['MNM_DRAW_EGL_GPU_DESTINATION_WITNESS=1']
        if mode=='rebind':command+=['MNM_DRAW_EGL_REBIND_DRAWABLE=1']
        if mode=='release':command+=['MNM_DRAW_EGL_REBIND_DRAWABLE=2']
        if mode in ('xcb','crop'):command+=['MNM_DRAW_EGL_XCB_IMAGE_WITNESS=1']
        if mode=='crop':command+=['MNM_DRAW_EGL_CORRECT_STALE_UPLOAD=1']
        execution=subprocess.run([*command,str(binary)],env=env,capture_output=True,timeout=60);(directory/'output.log').write_bytes(execution.stdout+execution.stderr);results[mode]=execution
    for mode in ('destination','rebind','release','xcb','crop'):
      for metadata in sorted((run/mode).glob('egl-destination-before-swap-*.bin')):
          raw=metadata.read_bytes();row=decode(raw);row['gpu_destination']=dict(**row,path=metadata.name,sha256=sha(raw),pixels_path=metadata.with_suffix('.565').name,pixels_sha256=sha(metadata.with_suffix('.565').read_bytes()))
          destination=plane(metadata.parent,row);y,x=np.indices(destination.shape);expected=(((y%31)<<11)|((x%63)<<5)|((y+x)%31)).astype('<u2')
          planes.append(dict(mode=mode,call=row['call'],width=row['width'],height=row['height'],mismatching_words=int(np.count_nonzero(destination!=expected)),prior_read_fbo=row['prior_read_fbo'],prior_read_buffer=row['prior_read_buffer'],default_read_buffer=row['default_read_buffer'],restored=row['restored']))
    rebound={mode:[decode_rebind(p.read_bytes()) for p in sorted((run/mode).glob('egl-drawable-rebind-*.bin'))] for mode in ('rebind','release')}
    xcb=[decode_xcb(p.read_bytes()) for p in sorted((run/'xcb').glob('egl-xcb-image-*.bin'))]
    crop=[decode_xcb(p.read_bytes()) for p in sorted((run/'crop').glob('egl-xcb-image-*.bin'))]
    stable=all(sha((ROOT/p).read_bytes())==h for p,h in sources.items());control=results['control'];success=stable and not any(r.returncode or r.stdout!=control.stdout for r in results.values()) and len(control.stdout.splitlines())==7 and all(b' mismatch 0 ' in line and line.endswith(b'error0') for line in control.stdout.splitlines()) and len(planes)==35 and bool(xcb) and len(crop)==7 and not any(r['resize_crop_applied'] for r in crop) and all(len(v)==7 and [r['count'] for r in v]==[1,1,0,1,0,1,0] and all(r['policy_mode']==(2 if mode=='release' else 1) for r in v) for mode,v in rebound.items()) and all(not row['mismatching_words'] and row['prior_read_fbo'] and row['prior_read_buffer']==0x8ce0 and row['default_read_buffer']==0x404 and row['restored'] for row in planes)
    report=dict(schema=1,success=success,sources=sources,claims=claims['claims'],sources_stable=stable,original_execution=False,artifacts=str(run.relative_to(ROOT)),frames=7,destination_planes=planes,rebind_calls=rebound,xcb_images=xcb,crop_images=crop,exit_codes={k:v.returncode for k,v in results.items()},stdout_sha256={k:sha(v.stdout) for k,v in results.items()},library_sha256=sha(library.read_bytes()),binary_sha256=sha(binary.read_bytes()),original_pixels_used_as_native_inputs=False,scope='Seven owned control/completion/destination/rebind/release/XCB/crop variants across seven resize frames. Exact known-pattern destination pixels, physical output, source/default read-buffer and viewport restoration, and GL error checks. No original-game or unrestricted observation neutrality claim.')
    with args.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
    print(json.dumps(dict(success=success,frames=len(planes),exit_codes=report['exit_codes'])));return int(not success)


if __name__=='__main__':raise SystemExit(main())
