#!/usr/bin/env python3
"""Run strict backing admission through the actual PE32 native facade, no game."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    parser.add_argument('--prefix',type=Path,required=True)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    if args.prefix.exists() or args.prefix.is_symlink() or args.report.exists():parser.error('Require new task-owned prefix and report paths')
    declaration=json.loads(args.claims.read_text());manifest=json.loads((args.library.parent/'manifest.json').read_text())
    sources=dict(declaration['sources'])
    for name,digest in manifest['sources'].items():
        if name in sources and sources[name]!=digest:raise RuntimeError('Compiled facade differs from declared source: '+name)
        sources[name]=digest
    def stable():return all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
    if not stable() or hashlib.sha256(args.library.read_bytes()).hexdigest()!=manifest['sha256']:raise RuntimeError('Native owner binary/source changed')
    parent=ROOT/'working/tests/native-output-owner';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='pe32-',dir=parent));(root/'.retain').write_text('Actual PE32 native backing admission and storage controls.\n')
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
        original_game_executed=False,complete_replacement=False,cases=[],artifacts=str(root.relative_to(ROOT)))
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(WINEPREFIX=str(args.prefix.resolve()),WINEDEBUG='-all',DISPLAY='',TMPDIR=str(root))
    try:
        args.prefix.mkdir(parents=True);(args.prefix/'.retain').write_text('Owned native backing probe prefix; preserve evidence.\n')
        definition=root/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(n+'@'+str(size)+'\n' for n,size in [('LoadLibraryA',4),('GetProcAddress',8),('GetEnvironmentVariableA',12),('ExitProcess',4),('VirtualQuery',12),('CreateFileA',28),('WriteFile',20),('CloseHandle',4),('VirtualProtect',16)]))
        subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(root/'kernel32.lib')],env=env,check=True)
        subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/native-owned-backing-probe.c'),'-o',str(root/'probe.obj')],env=env,check=True)
        binary=root/'probe.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(root/'probe.obj'),str(root/'kernel32.lib')],env=env,check=True)
        with (root/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot','-i'],env=dict(env,WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d'),stdout=out,stderr=subprocess.STDOUT,check=True,timeout=120)
        schedules=[('owned','1','3','1',0),('disabled','1','0','1',124),('idle','1','1','1',124),('stepping','1','2','1',124),('nonresident','1','3','0',124),('invalid','0','3','1',124),('compatibility',None,'0','0',0),('copy-prefix','1','3','1',124),('copy-span','1','3','1',124)]
        for name,strict,mode,resident,expected in schedules:
            case=root/name;case.mkdir();trial=dict(env,MNM_OWNER_DRAW_DLL='Z:'+str(args.library.resolve()).replace('/',chr(92)),MNM_NATIVE_CPU_GUARD=mode,MNM_PUBLIC_RESIDENT_WORLD=resident)
            if strict is not None:trial['MNM_NATIVE_OWNERSHIP_REQUIRED']=strict
            if name=='copy-span':trial['MNM_OWNER_BAD_COPY_SPAN']='1'
            if name=='copy-prefix':trial['MNM_OWNER_BAD_COPY_PREFIX']='1'
            if name=='compatibility':trial['MNM_OWNER_PROBE_ONLY_CREATE']='1'
            with (case/'wine.log').open('x') as out:
                result=subprocess.run(['wine',str(binary)],cwd=case,env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            if result.returncode!=expected:raise RuntimeError('Native owner case '+name+' exit '+str(result.returncode))
            if name=='owned' and (case/'backing-owner-result.bin').stat().st_size!=32:raise RuntimeError('Missing real protected backing result')
            if name!='owned' and (case/'backing-owner-result.bin').exists():raise RuntimeError('Rejected profile created an owned backing result')
            report['cases'].append(dict(id=name,exit_code=result.returncode,expected=expected))
        report.update(success=True,facade_sha256=manifest['sha256'],section_backing=True,active_lock_public_pages_protected=True,exact_restore_completed=True)
    except Exception as error:report['error']=str(error)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        report['sources_stable']=stable();report['success']=report['success'] and report['sources_stable']
        args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({key:report.get(key) for key in ('success','sources_stable','error')}))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
