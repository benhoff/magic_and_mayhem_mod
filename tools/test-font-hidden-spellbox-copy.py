#!/usr/bin/env python3
"""Bounded pre-fix retry and current real-provider hidden Spellbox copy controls."""
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
from coverage_claims import behavior_contract,scenario_contract
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/font-dc-lifetime-results-20261108/'
PREVIOUS='research/runtime/viewport-cursor-results-20261108/'


def module(name):
    spec=importlib.util.spec_from_file_location('hidden_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True)
    p.add_argument('--prefix-template',type=Path,required=True)
    args=p.parse_args();out=args.output.resolve()
    require(not out.exists() and out.is_relative_to(ROOT/'working')
            and args.report.startswith(PREFIX) and not(ROOT/args.report).exists(),'Fresh owned controls required')
    out.mkdir(parents=True);(out/'.retain').touch()
    scratch=Path(os.environ.get('TMPDIR','/tmp'))/('mnm-hidden-spellbox-'+out.name);scratch.mkdir(exist_ok=False)
    r=dict(schema=1,kind='hidden_spellbox_copy_provider_controls',success=False,original_game_runs=0,
           commands=[],profiles={},scope='Exact original5770f5 E8 and unchanged58c4a0 wrapper with authored caller endpoints, native ownership, independently checked whole planes and strict direct COM refusal. Pre-fix DLL is retained from the failed original-game diagnostic. No whole Spellbox caller/flags or driver equivalence.')
    files=set()
    for folder in ('runtime/native-draw','runtime/render','runtime/scene','protocols/include','protocols/schemas'):
        files.update(f.relative_to(ROOT).as_posix() for f in (ROOT/folder).rglob('*')
                     if f.is_file() and f.suffix in ('.c','.h','.hpp','.inc','.json'))
    files.update(('tools/test-font-hidden-spellbox-copy.py','tools/test-hidden-spellbox-copy.py','tools/test-native-cursor-lifetime.py',
        'tools/build-native-draw.py','tests/native-hidden-spellbox-copy-probe.c',
        'tests/native-cursor-owner-probe.c','tests/graphics-lifetime-common.h','tests/original-pe32-image.h',
        'tests/hidden-spellbox-copy-policy.cpp','runtime/shadow/win32_min.h',
        'reconstruction/rendering/pixel_kernel.c','reconstruction/rendering/pixel_kernel.h'))
    r['sources']={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in sorted(files)}
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(v for v in register['behaviors'] if v['id']=='NP.font-hidden-spellbox-admission')
    scenario=next(v for v in register['scenarios'] if v['id']=='native.font-hidden-spellbox-controls')
    r['claims']=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{v['id']:v for v in register['builds']}),
                      scenarios={scenario['id']:scenario_contract(scenario)})]
    before=False
    def run(label,argv):
        with (out/(label+'.log')).open('xb') as log:
            result=subprocess.run(list(map(str,argv)),cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=180)
        r['commands'].append(dict(label=label,argv=list(map(str,argv)),exit=result.returncode))
        require(result.returncode==0,label+' failed: '+(out/(label+'.log')).read_text(errors='replace')[-2500:])
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
        builder=module('build-native-draw')
        run('policy-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',ROOT/'tests/hidden-spellbox-copy-policy.cpp','-o',scratch/'policy'])
        run('policy',[scratch/'policy']);r['policy_cases']=93
        imports={'LoadLibraryA':4,'GetProcAddress':8,'GetModuleHandleA':4,'ExitProcess':4,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'CreateThread':24,'CreateEventA':16,'SetEvent':4,'WaitForSingleObject':8,'GetCurrentThreadId':0,'GetProcessHeap':0,'HeapAlloc':12,'HeapFree':12,'VirtualAlloc':16,'VirtualProtect':16,'VirtualQuery':12,'ReadFile':20,'GetFileSize':8,'GetEnvironmentVariableA':12,'QueryPerformanceCounter':4,'QueryPerformanceFrequency':4,'GetLastError':0,'SetLastError':4}
        for dll,names in [('kernel32',imports),('gdi32',{'SetPixel':16})]:
            d=scratch/(dll+'.def');d.write_text('LIBRARY '+dll+'.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in names.items()))
            run(dll+'-imports',['llvm-dlltool','-m','i386','--kill-at','-d',d,'-l',scratch/(dll+'.lib')])
        run('probe-compile',[*builder.FLAGS,'-c',ROOT/'tests/native-hidden-spellbox-copy-probe.c','-o',scratch/'probe.obj'])
        pad=scratch/'pad.S';pad.write_text('.text\n.globl _cursor_fixture_pad\n_cursor_fixture_pad:\n.space 0x400000\n')
        run('pad',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',scratch/'pad.obj'])
        run('probe-link',['lld-link','/machine:x86','/entry:start','/base:0x400000','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),scratch/'pad.obj',scratch/'probe.obj',scratch/'kernel32.lib',scratch/'gdi32.lib','/dynamicbase:no','/include:_cursor_fixture_pad'])
        runner=module('test-native-cursor-lifetime')
        old=out/'before';old.mkdir();shutil.copyfile(out/'probe.exe',old/'probe.exe')
        with Reader(ROOT) as reader:
            from rendering_menu_acceptance import artifact
            name=PREVIOUS+'default-04-acquisition.json';a=reader.json(name)
            raw=artifact(reader,a,'game/MnmDraw.dll');(old/'MnmDraw.dll').write_bytes(raw)
            r['pre_fix_native']=dict(acquisition=dict(record=name,sha256=reader.digest(name)),sha256=hashlib.sha256(raw).hexdigest())
        # The old original void wrapper repeats INVALIDRECT after Restore.
        # Bound only the first actual probe, preserve its partial stage/log,
        # then use the existing prefix/display cleanup in execute's finally.
        original_run=subprocess.run
        def bounded(argv,*a,**kw):
            if argv and str(argv[0])=='wine' and str(argv[-1]).endswith('/probe.exe'):
                kw['timeout']=8
            return original_run(argv,*a,**kw)
        try:
            subprocess.run=bounded
            ram=scratch/'before-ram';ram.mkdir()
            runner.execute(old,args.prefix_template.resolve(),ram,True,ROOT)
            raise ValueError('Pre-fix unsafe wrapper unexpectedly completed')
        except subprocess.TimeoutExpired:
            stage=old/'plain/stage.bin'
            require(stage.read_bytes()==struct.pack('<I',1),'Pre-fix timeout did not reach the exact hidden copy')
            r['pre_fix_retry']=dict(expected_timeout=True,timeout_seconds=8,last_stage=1,
                                   normal_completion=False)
        finally: subprocess.run=original_run
        builder.build(scratch/'provider',r['sources']);shutil.copyfile(scratch/'provider/MnmDraw.dll',out/'MnmDraw.dll')
        ram=scratch/'after-ram';ram.mkdir()
        runner.execute(out,args.prefix_template.resolve(),ram,True,ROOT)
        for name in ('plain','protected','early-plain','early-protected'):
            values=struct.unpack('<32I',(out/name/'hidden-spellbox-copy.bin').read_bytes())
            hit=struct.unpack('<32I',(out/name/'native-hidden-spellbox-copy.bin').read_bytes())
            require(values[0]==1 and values[2:4]==(5,18) and values[30]==1,
                    'Missing complete planes or hidden-copy policy counter')
            require(hit[:6]==(0x484d4e4d,0x31305043,1,128,0x40209ca7,0x5770fa)
                    and hit[6:20]==(640,480,640,480,640,512,680,552,640,512,15,1,16,16)
                    and all(v==0 for v in hit[22:]),'Actual hidden-copy witness differs')
            require(all(values[i+4]==0 for i in range(5,11)),'Native pins/jobs/owners survive')
            r['profiles'][name]=dict(assertions=values[1],complete_planes=values[2],
                                    hidden_copy_hits=values[30],strict_invalid_hresult='0x88760096',remaining_owners=0)
        r.update(success=True,complete_planes=20,changed_anchor_refusals=18)
    except Exception as exc:r['error']=str(exc)
    finally:
        if before:
            subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
            r['original_manifest_verified_before_after']=True
        r['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in r['sources'].items())
        r['success'] &= r['sources_stable']
        with Reader(ROOT) as reader:
            artifacts=[]
            keep=[v for v in out.rglob('*') if v.is_file()]
            keep += [v for v in scratch.rglob('*') if v.is_file() and not any('ram' in p for p in v.relative_to(scratch).parts)
                     and v.suffix in ('.exe','.dll','.json','.log')]
            for f in sorted(keep):
                raw=f.read_bytes();n=PREFIX+Path(args.report).stem+'-'+hashlib.sha256(str(f).encode()).hexdigest()[:20]+'.bin'
                artifacts.append(dict(path=str(f),record=n,sha256=write_report(ROOT,n,raw),bytes=len(raw)))
            for n in ('runtime/native-draw/cursor_copy_leases.h','runtime/native-draw/hidden_spellbox_copy.h',
                      'tests/native-hidden-spellbox-copy-probe.c','tests/hidden-spellbox-copy-policy.cpp','tools/test-font-hidden-spellbox-copy.py'):
                raw=(ROOT/n).read_bytes();record=PREFIX+'source-'+hashlib.sha256(raw).hexdigest()+'.bin'
                artifacts.append(dict(path=n,record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw)))
            r['artifacts']=artifacts
            digest=write_report(ROOT,args.report,(json.dumps(r,indent=2)+'\n').encode())
            print(json.dumps(dict(success=r['success'],error=r.get('error'),profiles=r['profiles'],record=args.report,sha256=digest)),flush=True)
    return 0 if r['success'] else 1


if __name__=='__main__':raise SystemExit(main())
