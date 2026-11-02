#!/usr/bin/env python3
"""Bounded journal controls; synthetic Wine process, never the original game."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import write_report
from cpu_reader_journal import decode, chains
from native_prefix_template import borrow

def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--build',required=True,type=Path);a=p.parse_args()
    sources=[str(f.relative_to(ROOT)) for f in sorted((ROOT/'runtime/render').glob('*.[ch]'))]
    sources += ['tools/test-native-cpu-reader-journal.py','tools/build-render-bridge.py','tools/build-shadow-bridge.py','tools/prepare-shadow-experiment.py','tools/cpu_reader_journal.py','runtime/shadow/win32_min.h','protocols/include/mnm/native_cpu_reader_journal_v1.h','tests/cpu-reader-journal-win32.c','tests/native-cpu-reader-journal-test.c','tests/test-cpu-reader-journal.py']
    hashes={s:hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in sources}
    parent=ROOT/'working/tests/cpu-reader-journal';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Synthetic journal controls; retained until verified handoff.\n')
    report=dict(schema=1,success=False,scope='Synthetic fixed journal controls, PE32 mapped file, exact opt-in gates and LastError. No original game, pixel equivalence or upstream caller recovery.',sources=hashes,cases=[],run_directory=str(run))
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEDEBUG='-all')
    lease=None
    try:
        for label,extra in [('optimized',[]),('sanitized',['-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            exe=a.build/('journal-'+label)
            subprocess.run(['clang','-std=c11','-O2','-Wall','-Wextra','-Werror','-pthread',*extra,str(ROOT/'tests/native-cpu-reader-journal-test.c'),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True,timeout=20);report['cases'].append(dict(name=label,success=True))
        with (run/'python.log').open('w') as log:
            subprocess.run([sys.executable,str(ROOT/'tests/test-cpu-reader-journal.py')],check=True,stdout=log,stderr=subprocess.STDOUT,timeout=45)
        dll=load('journal_build','tools/build-render-bridge.py').build(True,a.build/'selftest')
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
        obj=a.build/'journal-driver.obj';exe=a.build/'journal-driver.exe'
        subprocess.run(flags+['-c',str(ROOT/'tests/cpu-reader-journal-win32.c'),'-o',str(obj)],check=True)
        subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{exe}',str(obj),str(dll.parent/'kernel32.lib')],check=True)
        stage=load('journal_stage','tools/prepare-shadow-experiment.py');live=load('journal_live','tools/test-live-render-channel.py')
        driver=stage.add_import(exe.read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmjr')
        (run/'driver.exe').write_bytes(driver);shutil.copyfile(dll,run/dll.name)
        lease,report['prefix_pool']=borrow(ROOT/'working/fixtures/prefix-seed',run/'wineprefix',env)
        env['WINEPREFIX']=str(run/'wineprefix')
        for name,flags in [('missing-diagnostic',(1,0,1)),('missing-pilot',(1,1,0)),('missing-enable',(0,1,1)),('enabled',(1,1,1))]:
            case=run/name;case.mkdir();capture=case/'capture';capture.mkdir();channel=case/'commands.bin';frame=case/'frame.bin'
            h=bytearray(live.protocol.initial_header());struct.pack_into('<I',h,16,123)
            live.create(channel,h,live.protocol.SIZE);live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE)
            win=lambda f:'Z:'+str(f).replace('/','\\')
            child=dict(env,MNM_RENDER_OWNED_SESSION='1',MNM_RENDER_STREAM=win(frame),MNM_RENDER_COMMAND_CHANNEL=win(channel),MNM_RENDER_LOCK_CAPTURE_DIR=win(capture),MNM_JOURNAL_CONTROL_EXPECT=str(int(all(flags))))
            for key,value in zip(('MNM_NATIVE_CPU_READER_JOURNAL','MNM_ACCEPTANCE_DIAGNOSTIC','MNM_ACCEPTANCE_PILOT'),flags):child[key]=str(value)
            with (case/'wine.log').open('w') as log:
                result=subprocess.run(['wine',str(run/'driver.exe')],cwd=case,env=child,stdout=log,stderr=subprocess.STDOUT,timeout=45)
            assert result.returncode==0,(name,result.returncode)
            journal=capture/'cpu-reader-journal-v1.bin';item=dict(name=name,exit_code=result.returncode,journal_created=journal.exists(),last_error_and_result_preserved=True)
            assert journal.exists()==all(flags)
            if all(flags):
                header,rows=decode(journal.read_bytes());observed=chains(header,rows,{})
                assert observed['context_count']==1 and observed['request_count']==0 and len(rows)==2
                item.update(rows=len(rows),contexts=observed['context_count'])
            report['cases'].append(item)
        report['success']=True
    except Exception as e:
        report['error']=repr(e)
    finally:
        if lease is not None:
            subprocess.run(['wineserver','-k'],env=env,check=True,timeout=10)
            subprocess.run(['wineserver','-w'],env=env,check=True,timeout=20)
            lease.close();report['prefix_pool']['returned']=True
        report['sources_stable']=all(hashlib.sha256((ROOT/s).read_bytes()).hexdigest()==h for s,h in hashes.items());report['success'] &= report['sources_stable']
        report['artifacts']=[]
        for f in sorted(run.rglob('*')):
            if not f.is_file() or 'wineprefix' in f.relative_to(run).parts or f.is_symlink():continue
            raw=f.read_bytes();record=str(Path(a.report).with_suffix(''))+'-raw-'+hashlib.sha256(str(f.relative_to(run)).encode()).hexdigest()[:20]+'.bin'
            digest=write_report(ROOT,record,raw);report['artifacts'].append(dict(path=str(f.relative_to(run)),record=record,sha256=digest,bytes=len(raw)))
        report['build_manifest']=json.loads((a.build/'selftest/manifest.json').read_text()) if (a.build/'selftest/manifest.json').exists() else None
        digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],error=report.get('error'),record=a.report,sha256=digest)),flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
