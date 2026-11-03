#!/usr/bin/env python3
"""Bounded actual PE32 stale-reader refusals and existing active-lease controls."""
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
from coverage_evidence import Reader,write_report
from native_prefix_template import borrow

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-progression-results-20261102/'


def module(name):
    spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/name)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template',type=Path,required=True)
    p.add_argument('--report',required=True)
    a=p.parse_args()
    if not a.report.startswith(PREFIX) or '..' in Path(a.report).parts or (ROOT/a.report).exists():p.error('New immutable owned report required')
    out=ROOT/'working/tests'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False)
    (out/'.retain').write_text('Bounded CPU-reader controls; retain results, unique inputs and SQLite report.\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4')
    paths={str(q.relative_to(ROOT)) for folder in ('runtime','reconstruction','protocols')
           for q in (ROOT/folder).rglob('*') if q.is_file() and q.suffix in ('.c','.h','.cpp','.hpp','.inc','.S','.py','.json')}
    paths.update(str(q.relative_to(ROOT)) for q in (ROOT/'tools').glob('*.py'))
    paths.update(('tests/native-cpu-reader-refusal-probe.c','tests/native-draw-resource-probe.c',
                  'tests/native-active-reader-probe.c','tests/native-cpu-store-test.c'))
    sources={n:sha(ROOT/n) for n in sorted(paths)}
    result=dict(schema=1,kind='native_cpu_reader_refusal_controls',success=False,sources=sources,
                original_game_executed=False,player_accepted=False,cases=[],scope=__doc__)
    lease=None

    def run(label,command,*,cwd=out,extra=None,expected=0,timeout=180):
        with (out/(label+'.log')).open('xb') as log:
            c=subprocess.run([str(v) for v in command],cwd=cwd,env=env|dict(extra or {}),stdout=log,stderr=subprocess.STDOUT,timeout=timeout)
        if c.returncode!=expected:raise ValueError(f'{label}: {c.returncode}, expected {expected}')

    try:
        lease,result['prefix_pool']=borrow(a.prefix_template,out/'wineprefix',env)
        env['WINEPREFIX']=str(out/'wineprefix')
        builder=module('build-native-draw.py');dll=builder.build(out/'native-build',sources)
        imports={name:dict(values) for name,values in builder.IMPORTS.items()}
        imports['kernel32'].update(LoadLibraryA=4,GetProcAddress=8)
        imports['user32']['GetDesktopWindow']=0;imports['gdi32']['GetDIBColorTable']=16
        libraries=[]
        for name,values in imports.items():
            definition=out/(name+'.def');definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in values.items()))
            library=out/(name+'.lib');run(name,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',library]);libraries.append(library)
        run('compile',[*builder.FLAGS,'-c',ROOT/'tests/native-cpu-reader-refusal-probe.c','-o',out/'probe.obj'])
        run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',*libraries])
        result['binaries']={str(q.relative_to(ROOT)):sha(q) for q in (dll,out/'probe.exe')}
        for mode,registered,returned in [('n',0,0),('0',1,0),('1',1,1)]:
            dest=out/('case-'+mode);dest.mkdir();shutil.copyfile(dll,dest/'MnmDraw.dll')
            run('case-'+mode,['wine',out/'probe.exe'],cwd=dest,extra={'MNM_READER_RESULT':mode},expected=124)
            legacy=struct.unpack('<11I',(dest/'native-cpu-history-refusal.bin').read_bytes())
            context=struct.unpack('<24I',(dest/'native-cpu-reader-authority.bin').read_bytes())
            assert legacy[:2]==(0x484d4e4d,2) and legacy[3:8]==(2,0,5,6,1)
            assert context[:5]==(0x4e4d4e4d,1,96,registered,returned)
            assert context[5]==legacy[2] and context[6:13]==(13,9,32,16,1,0,0)
            assert context[17:24]==(0,0,0,2,0,5,6)
            result['cases'].append(dict(mode=mode,exit_code=124,legacy=list(legacy),authority=list(context)))
        run('stop-probe-server',['wineserver','-k'])
        run('wait-probe-server',['wineserver','-w'])
        declaration=out/'active-claims.json';declaration.write_text(json.dumps(dict(sources=sources,claims=[])))
        run('active-readers',[sys.executable,ROOT/'tools/test-native-active-reader.py','--claims',declaration,'--wine-prefix',out/'wineprefix'],timeout=600)
        active_path=Path((out/'active-readers.log').read_text().splitlines()[-1])
        result['active_readers']=json.loads(active_path.read_text())
        assert result['active_readers']['success'] and len(result['active_readers']['cases'])==6
        result['success']=True
    except (OSError,ValueError,AssertionError,RuntimeError,subprocess.SubprocessError) as error:result['error']=type(error).__name__+': '+str(error)
    finally:
        if lease:
            subprocess.run(['wineserver','-k'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=15)
            subprocess.run(['wineserver','-w'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=15)
            lease.close();result['prefix_pool']['returned']=True
        result['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());result['success'] &= result['sources_stable']
        # Bounded physical logs are retained input interchange; the citable
        # complete report is written directly to the paired SQLite writer.
        result['logs']={q.name:q.read_text(errors='replace') for q in sorted(out.glob('*.log')) if q.stat().st_size<=1024**2}
        h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
        with Reader(ROOT) as reader:assert reader.json(a.report)==result
        print(json.dumps(dict(success=result['success'],record=a.report,sha256=h,error=result.get('error'))),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
