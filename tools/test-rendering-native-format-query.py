#!/usr/bin/env python3
"""Execute unchanged original format-query bytes against production facade storage.

Normal successful GetPixelFormat only. No original allocator/window/provider
failure path, game reachability, live World555, physical driver or acceptance.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import resource
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import Reader, write_report
from original_x86 import PE32

ROOT = Path(__file__).resolve().parents[1]
ID = 'NP.original-native-format-query'
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def main():
    resource.setrlimit(resource.RLIMIT_CORE, (0,0))
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--report', required=True)
    p.add_argument('--prefix-template', type=Path, required=True)
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working') or (ROOT/a.report).exists():
        p.error('Require fresh private working output and report')
    out.mkdir(parents=True)
    with Reader(ROOT) as reader:
        reg=reader.json('research/runtime/coverage/register.json');behavior=next(b for b in reg['behaviors'] if b['id']==ID)
        paths=required_sources(behavior)
        for directory in ['runtime/native-draw','runtime/shadow','reconstruction/rendering','protocols/include']:
            paths.update(f.relative_to(ROOT).as_posix() for f in (ROOT/directory).rglob('*') if f.is_file() and f.suffix in {'.c','.h','.hpp','.cpp','.inc','.inl','.S'})
        sources={n:reader.digest(n) for n in paths}
    report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=ID,contract_sha256=behavior_contract(behavior,{b['id']:b for b in reg['builds']}))],
        original_code_executed=False,original_game_executed=False,native_equivalence=False,complete_replacement=False,
        physical_driver_equivalence=False,scope=__doc__,commands=[])
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(WINEPREFIX=str(out/'prefix'),WINEDEBUG='-all')
    def run(label,argv,timeout=120):
        result=subprocess.run(list(map(str,argv)),cwd=out,env=env,capture_output=True,timeout=timeout)
        name=str(Path(a.report).with_suffix(''))+'-'+label+'.log'
        digest=write_report(ROOT,name,result.stdout+b'\n'+result.stderr)
        report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=result.returncode,record=name,sha256=digest))
        if result.returncode:raise ValueError(label+' failed: '+result.stderr.decode(errors='replace')[-1200:])
    verified=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
        image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
        if image.sha256!=BUILD:raise ValueError('Original build mismatch')
        code=image.read(0x58d540,105)
        if hashlib.sha256(code).hexdigest()!='83bf8a415f63db64eb2eb5eeeca1d435d4f2105dac64439f4d02cd25cc3a7cc9':raise ValueError('Query extent/signature differs')
        (out/'original_format_query.h').write_text('static const unsigned char original_format_query[]={'+','.join(str(v) for v in code)+'};\n')
        spec=importlib.util.spec_from_file_location('query_facade',ROOT/'tools/build-native-draw.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build(out/'facade',sources)
        from native_prefix_template import clone
        report['prefix_template']=clone(a.prefix_template,out/'prefix',env)
        imports={'LoadLibraryA':4,'GetProcAddress':8,'GetEnvironmentVariableA':12,'ExitProcess':4,
            'VirtualProtect':16,'FlushInstructionCache':12,'GetCurrentProcess':0,'CreateFileA':28,'WriteFile':20,'CloseHandle':4}
        (out/'kernel32.def').write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(n+'@'+str(v)+'\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',out/'kernel32.def','-l',out/'kernel32.lib'])
        (out/'pad.S').write_text('.text\n.globl _query_pad\n_query_pad:\n.space 0x300000\n')
        run('pad',['clang','--target=i686-pc-windows-msvc','-c',out/'pad.S','-o',out/'pad.obj'])
        run('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I'+str(out),'-c',ROOT/'tests/native-original-format-query.c','-o',out/'query.obj'])
        run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/base:0x400000','/dynamicbase:no','/include:_query_pad','/out:'+str(out/'query.exe'),out/'query.obj',out/'pad.obj',out/'kernel32.lib'])
        env['MNM_QUERY_LIBRARY']='Z:'+str(dll).replace('/',chr(92))
        run('query',['xvfb-run','-a','wine',out/'query.exe'])
        rows=list(struct.iter_unpack('<12I',(out/'query-results.bin').read_bytes()))
        if len(rows)!=4 or [r[1] for r in rows]!=[0,0,0,1] or any(r[6:10]!=(1,1,0,0) for r in rows):raise ValueError('Query/retirement result differs')
        report.update(success=True,original_code_executed=True,original_sha256=BUILD,
            original_query_sha256=hashlib.sha256(code).hexdigest(),original_query_unmodified=True,
            rows=[dict(kind=r[0],selector=r[1],masks=list(r[2:5]),bits=r[5],neighbor_bytes_preserved=bool(r[6]),query_bytes_unchanged=bool(r[7]),remaining_surfaces=r[8],remaining_devices=r[9]) for r in rows])
    except Exception as error:report['error']=str(error)
    finally:
        if (out/'prefix').exists():
            for command in [['wineserver','-k'],['wineserver','-w']]:subprocess.run(command,env=env,capture_output=True,timeout=30)
        if verified:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except Exception as error:report.update(success=False,manifest_error=str(error))
        report['retained_artifacts']=[]
        for name in ['query.exe','query.obj','original_format_query.h','query-results.bin','facade/MnmDraw.dll','facade/manifest.json']:
            path=out/name
            if path.exists():
                record=str(Path(a.report).with_suffix(''))+'-raw-'+name.replace('/','-')+'.bin'
                report['retained_artifacts'].append(dict(path=name,record=record,sha256=write_report(ROOT,record,path.read_bytes())))
        with Reader(ROOT) as reader:report['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],sha256=digest,error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
