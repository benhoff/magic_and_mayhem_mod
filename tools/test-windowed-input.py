#!/usr/bin/env python3
"""Source-bound shell build and Qt/X11 to PE32 input controls; no original game."""
import argparse
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tarfile
from coverage_evidence import write_report,evidence_digest
from native_prefix_template import clone
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ['claims','output','report','prefix-template']:p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();out=a.output.resolve();build=ROOT/'working/build/qt-shell'
    if out.exists() or a.report.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh owned output/report required')
    d=json.loads(a.claims.read_text());stable=lambda:all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in d['sources'].items())
    if not stable():p.error('Prospective sources changed')
    out.mkdir(parents=True);(out/'.retain').write_text('Retain input controls, compiled inputs and failures.\n')
    report=dict(schema=1,success=False,sources=d['sources'],claims=d['claims'],scope=__doc__,original_game_executed=False,commands=[])
    env=dict(os.environ,TMPDIR=str(out),PYTHONDONTWRITEBYTECODE='1',QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='2',WINEDEBUG='-all')
    def run(args,name,timeout=600):
        with (out/(name+'.log')).open('xb') as log:
            t=subprocess.run([str(x) for x in args],env=env,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=timeout)
        report['commands'].append(dict(command=[str(x) for x in args],exit_code=t.returncode,log=name+'.log'))
        if t.returncode:raise ValueError(name+' failed')
    try:
        run(['cmake','-S',ROOT/'apps/qt-shell','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'],'configure')
        run(['cmake','--build',build,'--target','mnm-qt-shell','qt-input-test','menu-result-controller-test','--parallel','2'],'shell-build',1200)
        report['binary_sha256']=hashlib.sha256((build/'mnm-qt-shell').read_bytes()).hexdigest()
        run(['ctest','--test-dir',build,'--output-on-failure','-R','^qt-shell-input|^qt-menu-result-controller$'],'qt-controls',90)
        run([sys.executable,'-B',ROOT/'tests/test-input-wire.py','-v'],'wire-controls',30)
        state=out/'input.bin';run(['xvfb-run','-a',build/'qt-input-test',state],'qt-fixture',30)
        spec=importlib.util.spec_from_file_location('bridge_build',ROOT/'tools/build-render-bridge.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        dll=module.build(True,ROOT/'working/build/input-selftest')
        spec=importlib.util.spec_from_file_location('bridge_stage',ROOT/'tools/prepare-shadow-experiment.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        exe=out/'selftest.exe';exe.write_bytes(module.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        (out/'MnmRender.dll').write_bytes(dll.read_bytes())
        report['prefix']=clone(a.prefix_template,out/'wineprefix',env)
        sys.path.insert(0,str(ROOT/'protocols/python'));from mnm_protocols import frame_v1,input_v2
        frame=out/'frame.bin'
        with frame.open('xb') as f:f.write(frame_v1.initial_header());f.truncate(frame_v1.SIZE)
        initial=state.read_bytes();assert initial[:8]==b'MNMINK01'
        env.update(WINEPREFIX=str(out/'wineprefix'),MNM_INPUT_SELFTEST='1',MNM_RENDER_INPUT='Z:'+str(state).replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'))
        run(['wine',exe],'pe32-client-controls',90)
        desktop=bytearray(initial);desktop[:8]=input_v2.MAGIC;struct.pack_into('<I',desktop,8,2);struct.pack_into('<I',desktop,40,1);state.write_bytes(desktop)
        run(['wine',exe],'pe32-desktop-controls',90)
        report['success']=True
    except Exception as e:report['error']=str(e)
    finally:
        if 'WINEPREFIX' in env:
            subprocess.run(['wineserver','-k'],env=env,timeout=15,check=True)
            subprocess.run(['wineserver','-w'],env=env,timeout=15,check=True)
        report['sources_stable']=stable();report['success'] &= report['sources_stable']
        archive=io.BytesIO()
        with tarfile.open(fileobj=archive,mode='w:gz') as tar:
            for f in out.rglob('*'):
                if f.is_file() and 'wineprefix' not in f.relative_to(out).parts:tar.add(f,arcname='controls/'+str(f.relative_to(out)),recursive=False)
            if (build/'mnm-qt-shell').is_file():tar.add(build/'mnm-qt-shell',arcname='working/build/qt-shell/mnm-qt-shell')
        raw=a.report.with_name(a.report.stem+'-raw.tar.gz');write_report(ROOT,raw,archive.getvalue());report['raw_record']=str(raw)
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ['success','error','binary_sha256','sources_stable']}),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
