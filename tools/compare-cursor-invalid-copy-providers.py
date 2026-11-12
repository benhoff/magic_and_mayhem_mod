#!/usr/bin/env python3
"""Compare two supplied invalid fast-copy rectangles in real offscreen providers."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
from coverage_evidence import Reader,write_report
from rendering_operations_history import sha
from native_prefix_template import borrow

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True)
    a=p.parse_args()
    if (ROOT/a.report).exists():p.error('New immutable report required')
    out=ROOT/'working/tests'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False)
    (out/'.retain').write_text('Owned provider comparison; preserve all failures and raw results.\n')
    names=['tools/compare-cursor-invalid-copy-providers.py','tests/cursor-invalid-copy-provider-probe.c',
           'tests/graphics-lifetime-common.h','runtime/shadow/win32_min.h','tools/build-shadow-bridge.py',
           'tools/build-native-draw.py','tools/native_prefix_template.py','tools/experiment_environment.py']
    result=dict(schema=1,success=False,sources={n:sha((ROOT/n).read_bytes()) for n in names},
                commands=[],providers={},original_game_executed=False,
                scope='Supplied RGB565 offscreen/system-memory DirectDraw1 surfaces with no clipper, WAIT/WAIT+source-key flags, one valid control and two exact historical invalid rectangles. Real Wine builtin DirectDraw and current guarded native facade; full source/destination word checks. No original cursor caller, historical coordinate origin, primary-surface/physical-driver or acceptance claim.')
    lease=None;environment=None;before_verified=False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True,stdout=subprocess.DEVNULL);before_verified=True
        with Reader(ROOT) as reader:
            acquired=reader.json(a.acquisition)
            if not acquired['success'] or acquired['private_wine_stopped'] is not True:raise ValueError('Stopped successful acquisition required')
            result['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
            session=Path(acquired['session']);dll=session/'game/MnmDraw.dll'
            expected=acquired['native_admission']['native_only']['binaries']['MnmDraw.dll']
            if sha(dll.read_bytes())!=expected:raise ValueError('Native provider bytes differ')
            shutil.copyfile(dll,out/'MnmDraw.dll');result['native_provider_sha256']=expected
            manifest=json.loads((session/'native-draw-build.json').read_text())
            for name,digest in manifest['sources'].items():
                if sha((ROOT/name).read_bytes())!=digest:raise ValueError('Changed native provider source: '+name)
                result['sources'][name]=digest
        env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in ('DISPLAY','XAUTHORITY')}
        env.update(WINEDEBUG='-all',TMPDIR=str(out),WINEDLLOVERRIDES='ddraw=b')
        lease,result['prefix']=borrow(ROOT/'working/fixtures/prefix-seed',out/'wineprefix',env)
        environment=dict(env,WINEPREFIX=str(out/'wineprefix'))
        def run(command,label,extra=None,cwd=out):
            path=out/(label+'.log')
            with path.open('xb') as log:
                done=subprocess.run([str(v) for v in command],cwd=cwd,env=extra or environment,stdout=log,stderr=subprocess.STDOUT,timeout=120)
            result['commands'].append(dict(label=label,command=[str(v) for v in command],returncode=done.returncode))
            if done.returncode:raise ValueError('Provider command failed: '+label)
        spec=importlib.util.spec_from_file_location('provider_flags',ROOT/'tools/build-native-draw.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        imports={'GetModuleHandleA':4,'GetEnvironmentVariableA':12,'LoadLibraryA':4,'GetProcAddress':8,
                 'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
        run(['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'],'imports')
        run([*builder.FLAGS,'-c',ROOT/'tests/cursor-invalid-copy-provider-probe.c','-o',out/'probe.obj'],'compile')
        run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',out/'kernel32.lib'],'link')
        result['probe_sha256']=sha((out/'probe.exe').read_bytes())
        result['wine_version']=subprocess.check_output(['wine','--version'],text=True).strip()
        for name,native in [('wine',False),('native',True)]:
            directory=out/name;directory.mkdir()
            case=dict(environment,MNM_FACTORY_NATIVE=str(int(native)),MNM_NATIVE_CPU_GUARD='3')
            run(['xvfb-run','-a','-s','-screen 0 1280x1024x16','wine',out/'probe.exe'],name,case,directory)
            subprocess.run(['wineserver','-k'],env=case,check=True,timeout=10)
            subprocess.run(['wineserver','-w'],env=case,check=True,timeout=15)
            raw=(directory/'invalid-copy-provider.bin').read_bytes()
            if len(raw)!=288:raise ValueError('Provider result extent')
            rows=[list(struct.unpack_from('<24I',raw,i*96)) for i in range(3)]
            result['providers'][name]=dict(rows=rows,sha256=sha(raw),
                                          expected_returns=[r[1] for r in rows]==[0,0x88760096,0x88760096],
                                          full_pixel_checks=all(r[13:15]==[1,1] for r in rows),
                                          words_checked=sum(r[15] for r in rows))
        left,right=result['providers']['wine']['rows'],result['providers']['native']['rows']
        result['contexts_match']=all(a[:21]==b[:21] and not any(a[22:]) and not any(b[22:]) and (a[21],b[21])==(0,1) for a,b in zip(left,right))
        result['success']=result['contexts_match'] and all(v['expected_returns'] and v['full_pixel_checks'] for v in result['providers'].values())
    except Exception as error:result['error']=str(error)
    finally:
        if environment is not None:
            subprocess.run(['wineserver','-k'],env=environment,check=False,timeout=10)
            subprocess.run(['wineserver','-w'],env=environment,check=False,timeout=15)
        if lease is not None:
            try:lease.close();result['prefix_returned']=True
            except Exception as error:result.update(success=False,prefix_return_error=str(error))
        if before_verified:
            verified=subprocess.run([ROOT/'tools/original-manifest.sh','verify'],stdout=subprocess.DEVNULL)
            result['original_manifest_verified_before_after']=verified.returncode==0
            result['success'] &= result['original_manifest_verified_before_after']
        result['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in result['sources'].items())
        result['success'] &= result['sources_stable']
        result['artifacts']=[]
        for file in sorted(out.rglob('*')):
            if not file.is_file() or 'wineprefix' in file.relative_to(out).parts:continue
            raw=file.read_bytes();record=str(Path(a.report).with_suffix(''))+'-'+sha(file.relative_to(out).as_posix().encode())[:20]+'.bin'
            result['artifacts'].append(dict(path=file.relative_to(out).as_posix(),record=record,sha256=write_report(ROOT,record,raw)))
        write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ['success','error','contexts_match','prefix_returned']}),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
