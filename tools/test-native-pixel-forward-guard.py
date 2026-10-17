#!/usr/bin/env python3
"""Exercise the actual six-family PE32 no-forward callback before a fallback."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from drawing_api_session import sha
from pixel_forwarding_guard import FLAG,decode_refusal
ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('claims','prefix-template','report'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args()
    if args.report.exists():parser.error('Refusing report overwrite')
    declaration=json.loads(args.claims.read_bytes());sources=declaration['sources']
    stable=lambda:all(sha((ROOT/n).read_bytes())==h for n,h in sources.items())
    if not stable():raise ValueError('Prospective guard sources changed')
    parent=ROOT/'working/tests/native-pixel-forward-guard';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=')
    result=dict(schema=1,success=False,experiment=str(out),sources=sources,claims=declaration['claims'],
                original_game_executed=False,complete_rendering_replacement=False)
    def run(name,command,*,environment=env,cwd=out,expected=0):
        with (out/(name+'.log')).open('x') as log:
            p=subprocess.run([str(x) for x in command],cwd=cwd,env=environment,stdout=log,stderr=subprocess.STDOUT,timeout=120)
        if p.returncode!=expected:raise ValueError(f'{name}: expected {expected}, got {p.returncode}')
        return p.returncode
    try:
        spec=importlib.util.spec_from_file_location('guard_builder',ROOT/'tools/build-native-draw.py')
        builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build(out/'build',sources);shutil.copyfile(dll,out/'MnmDraw.dll')
        result['native_build']=json.loads((out/'build/manifest.json').read_bytes())
        imports=dict(LoadLibraryA=4,GetModuleHandleA=4,GetProcAddress=8,GetEnvironmentVariableA=12,GetLastError=0,SetLastError=4,CreateFileA=28,WriteFile=20,CloseHandle=4,ExitProcess=4)
        (out/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{size}\n' for n,size in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',out/'kernel32.def','-l',out/'kernel32.lib'])
        run('compile',[*builder.FLAGS,'-c',ROOT/'tests/native-pixel-forward-guard-probe.c','-o',out/'probe.obj'])
        run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',out/'kernel32.lib'])
        run('prefix',['cp','-a','--reflink=auto',args.prefix_template.resolve(),out/'wineprefix'])
        run('wineboot',['wineboot','-u'])
        result['cases']=[]
        cases=[('partial-default',None,'1','forward',0,None,True),('partial-zero','0','1','forward',0,None,True),
               ('native','1','1','native',0,None,False),('forward','1','1','forward',126,6,False),
               ('missing','1','1','missing',126,5,False),('duplicate','1','1','duplicate',126,4,False),
               ('route','1','1','route',126,3,False),('uninstalled','1','1','uninstalled',126,7,False),
               ('counters','1',None,'native',126,2,False),('invalid','yes','1','native',126,1,False),
               ('empty','','1','native',126,1,False)]
        for name,flag,counters,mode,expected,reason,fallback in cases:
            case=out/name;case.mkdir();extra=dict(env,MNM_GUARD_CASE=mode)
            if flag is not None:extra[FLAG]=flag
            if counters is not None:extra['MNM_NATIVE_PIXEL_COUNTERS']=counters
            code=run(name,['wine',out/'probe.exe'],environment=extra,cwd=case,expected=expected)
            if (case/'original-fallback-reached.bin').exists()!=fallback:raise ValueError('Original fallback reachability differs: '+name)
            refusal=case/'native-pixel-forward-refusal.bin'
            observed=decode_refusal(refusal.read_bytes()) if refusal.exists() else None
            if (observed['reason'] if observed else None)!=reason:raise ValueError('Refusal reason differs: '+name)
            result['cases'].append(dict(case=name,exit_code=code,original_fallback_reached=fallback,refusal=observed))
        result.update(success=True,binary_sha256=sha((out/'probe.exe').read_bytes()))
    except (ValueError,OSError,subprocess.SubprocessError) as error:result['error']=str(error)
    finally:
        if (out/'wineprefix').exists():
            subprocess.run(['wineserver','-k'],env=env,check=True,timeout=10)
            subprocess.run(['wineserver','-w'],env=env,check=True,timeout=30)
            result['private_prefix_stopped']=True
        result['sources_stable']=stable();result['success']=result['success'] and result['sources_stable']
        args.report.parent.mkdir(parents=True,exist_ok=True)
        with args.report.open('x') as output:json.dump(result,output,indent=2);output.write('\n')
    print(json.dumps(dict(success=result['success'],error=result.get('error'),cases=len(result.get('cases',[])))))
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
