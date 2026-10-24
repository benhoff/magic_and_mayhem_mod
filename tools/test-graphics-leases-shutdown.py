#!/usr/bin/env python3
"""Bounded cross-interface leases and complete original graphics shutdown."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import resource
import secrets
import shlex
import shutil
import socket
import struct
import subprocess
import time

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import evidence_digest, read_json, write_report
from drawing_fixture_prefix import prepare
from native_draw_retirement import require_retired

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS=['NP.surface-alias-lease-lifecycle','NP.graphics-session-provider-retirement']


def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--report',required=True)
    p.add_argument('--prefix-owner-root',type=Path,required=True)
    p.add_argument('--wine-prefix',type=Path,required=True)
    a=p.parse_args();out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        p.error('Require fresh working/tests output')
    out.mkdir(parents=True);(out/'.retain').write_text('Graphics alias/lease/session shutdown positive and negative evidence.\n')
    register=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
    bs={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    paths=set().union(*(required_sources(bs[i]) for i in IDS))
    # Recursively pin every local quoted include before invoking the compiler.
    todo=list(paths)
    while todo:
        name=todo.pop();path=ROOT/name
        if path.suffix not in {'.c','.h','.cpp','.hpp','.inc','.S'}:continue
        for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
            child=(path.parent/include).resolve()
            if child.is_relative_to(ROOT) and child.is_file():
                key=child.relative_to(ROOT).as_posix()
                if key not in paths:paths.add(key);todo.append(key)
    sources={name:evidence_digest(ROOT/name,root=ROOT) for name in sorted(paths)}
    scenario=next(s for s in register['scenarios'] if s['id']=='GLS1.provider-lifetime-headless')
    report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=i,contract_sha256=behavior_contract(bs[i],builds),scenarios={scenario['id']:scenario_contract(scenario)}) for i in IDS],
        original_sha256=BUILD,commands=[],original_game_executed=False,live_replacement=False,
        physical_driver_equivalence=False,performance_acceptance=False,
        scope='Identical PE32 caller comparison against actual Wine DirectDraw and native facade: Surface1/2/3/4 alias/Lock/DC matrix plus repeated complete unchanged original58acf0/58b3e0 shutdown under actual COM providers. Caller-triggered failure cleanup is explicit, not recovered failure-path reachability.')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4',
               __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',__GLX_VENDOR_LIBRARY_NAME='mesa',
               MNM_ORIGINAL_IMAGE='Z:'+str((ROOT/'original/Arcane_Nocd/Chaos.exe').resolve()).replace('/','\\'))
    env.pop('MESA_LOADER_DRIVER_OVERRIDE',None)
    lock=None;prefix=None;xserver=None;xlog=None

    def run(label,command,extra=None,cwd=out,timeout=180):
        print(label,flush=True);log=out/(label+'.log')
        with log.open('x') as f:
            result=subprocess.run(list(map(str,command)),cwd=cwd,env=env|dict(extra or {}),stdout=f,stderr=subprocess.STDOUT,timeout=timeout)
        text=log.read_text(errors='replace')
        report['commands'].append(dict(label=label,argv=list(map(str,command)),exit=result.returncode,output=text,
            log=log.relative_to(ROOT).as_posix(),sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
        if result.returncode:raise ValueError(label+': exit '+str(result.returncode)+' '+text[-3000:])
        return text

    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        if hashlib.sha256((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()).hexdigest()!=BUILD:raise ValueError('Pinned image changed')
        owner=a.prefix_owner_root.resolve();prefix,lock=prepare(owner,out,a.wine_prefix.resolve(),a.wine_prefix.resolve())
        def common(path):return (path/subprocess.check_output(['git','-C',str(path),'rev-parse','--git-common-dir'],text=True).strip()).resolve()
        if common(owner)!=common(ROOT):raise ValueError('Fixture prefix must share this Git checkout')
        env['WINEPREFIX']=str(prefix)
        report['prefix']=dict(path=str(prefix),owner=str(owner),exclusive_lock=True,idle_before_use=True)
        for display in range(190,290):
            with socket.socket() as candidate:
                try:candidate.bind(('127.0.0.1',6000+display))
                except OSError:continue
                break
        else:raise ValueError('No private X display available')
        authority=out/'Xauthority';fields=(b'',str(display).encode(),b'MIT-MAGIC-COOKIE-1',secrets.token_bytes(16))
        authority.write_bytes(struct.pack('>H',65535)+b''.join(struct.pack('>H',len(v))+v for v in fields));authority.chmod(0o600)
        env['XAUTHORITY']=str(authority);env['DISPLAY']='127.0.0.1:'+str(display)
        xlog=(out/'xserver.log').open('x')
        xserver=subprocess.Popen(['Xvfb',':'+str(display),'-screen','0','1024x768x24','-nolisten','unix','-listen','tcp','-auth',str(authority)],env=env,stdout=xlog,stderr=subprocess.STDOUT)
        for _ in range(100):
            if xserver.poll() is not None:raise ValueError('Private X server exited')
            try:
                with socket.create_connection(('127.0.0.1',6000+display),timeout=.1):break
            except OSError:time.sleep(.05)
        else:raise ValueError('Private X server not ready')
        report['display']=dict(address=env['DISPLAY'],private_tcp_only=True,authenticated=True)
        run('prefix-keepalive',['wineserver','-p'])
        spec=importlib.util.spec_from_file_location('graphics_builder',ROOT/'tools/build-native-draw.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build(out/'native-build',sources);shutil.copyfile(dll,out/'MnmDraw.dll')
        report['native_build']=json.loads((out/'native-build/manifest.json').read_text())
        imports={k:dict(v) for k,v in builder.IMPORTS.items()}
        imports['kernel32'].update(LoadLibraryA=4,GetProcAddress=8,ReadFile=20,GetFileSize=8,Sleep=4)
        imports['gdi32'].update(SetPixel=16)
        libraries=[]
        for name,entries in imports.items():
            definition=out/(name+'.def');definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(k+'@'+str(v)+'\n' for k,v in entries.items()))
            library=out/(name+'.lib');run('imports-'+name,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',library]);libraries.append(library)
        pad=out/'private-image-pad.S';pad.write_text('.text\n.globl _graphics_fixture_pad\n_graphics_fixture_pad:\n.space 0x400000\n')
        run('pad-build',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj'])
        for label,source in [('leases','tests/graphics-alias-lease-probe.c'),('flips','tests/graphics-alias-flip-probe.c'),('shutdown','tests/graphics-session-shutdown-probe.c'),('failures','tests/graphics-allocation-failure-probe.c')]:
            run(label+'-build',[*builder.FLAGS,'-MD','-MF',out/(label+'.d'),'-c',ROOT/source,'-o',out/(label+'.obj')])
            closure=[]
            for token in shlex.split((out/(label+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(token).resolve()
                if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):closure.append(path.relative_to(ROOT).as_posix())
            if set(closure)-set(sources):raise ValueError('Undeclared compiler input: '+repr(sorted(set(closure)-set(sources))))
            report.setdefault('compiled_sources',{})[label]=closure
            extra=['/base:0x400000','/dynamicbase:no','/include:_graphics_fixture_pad',str(out/'pad.obj')] if label=='shutdown' else []
            run(label+'-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0',
                '/out:'+str(out/(label+'.exe')),*extra,out/(label+'.obj'),*libraries])
            endpoints=[('wine',dict(WINEDLLOVERRIDES='ddraw=b')),('native',dict(WINEDLLOVERRIDES='ddraw=',MNM_FACTORY_NATIVE='1')),('guarded',dict(WINEDLLOVERRIDES='ddraw=',MNM_FACTORY_NATIVE='1',MNM_NATIVE_CPU_GUARD='3'))]
            if label=='failures':endpoints=[('native',dict(WINEDLLOVERRIDES='ddraw=',MNM_FACTORY_NATIVE='1')),('guarded',dict(WINEDLLOVERRIDES='ddraw=',MNM_FACTORY_NATIVE='1',MNM_NATIVE_CPU_GUARD='3'))]
            for endpoint,settings in endpoints:
                target=out/(label+'-'+endpoint);target.mkdir()
                retirement=target/'retirement.bin'
                if xserver.poll() is not None:raise ValueError('Private X server died before caller')
                command=['wine','explorer','/desktop=graphics-lifetime,1024x768',out/(label+'.exe')] if label=='flips' else ['wine',out/(label+'.exe')]
                run(label+'-'+endpoint,command,settings|dict(MNM_NATIVE_DRAW_RETIREMENT='Z:'+str(retirement).replace('/','\\')),cwd=target)
                if xserver.poll() is not None:raise ValueError('Private X server died during caller')
                if endpoint!='wine':report.setdefault('retirement',{})[label+'-'+endpoint]=require_retired(retirement)
                if label=='failures':
                    raw=(target/'allocation-failures.bin').read_bytes();expected=19 if endpoint=='guarded' else 10
                    if len(raw)!=expected*32:raise ValueError('Incomplete allocation failure matrix')
                    report.setdefault('allocation_failures',{})[endpoint]=dict(cases=expected,sha256=hashlib.sha256(raw).hexdigest(),rows=list(struct.iter_unpack('<8I',raw)))
            if label=='failures':continue
            name={'leases':'alias-leases.bin','flips':'alias-flips.bin','shutdown':'graphics-shutdown.bin'}[label]
            left=(out/(label+'-wine')/name).read_bytes();right=(out/(label+'-native')/name).read_bytes()
            count,words={'leases':(128,601),'flips':(64,16),'shutdown':(192,375)}[label]
            if len(left)!=count*words*4 or len(right)!=len(left):raise ValueError(label+' incomplete denominator')
            av,bv=[struct.unpack('<'+str(len(v)//4)+'I',v) for v in (left,right)]
            differences=[dict(word=i,case=i//words,field=i%words,wine=x,native=y) for i,(x,y) in enumerate(zip(av,bv)) if x!=y]
            policy=[]
            if label=='shutdown':
                policy=[d for d in differences if d['field']==23 and bv[d['case']*words+5]==1 and d['native']==0x8007000e and d['wine']!=0]
                differences=[d for d in differences if d not in policy]
            report[label]=dict(cases=count,words=len(av),wine_sha256=hashlib.sha256(left).hexdigest(),native_sha256=hashlib.sha256(right).hexdigest(),differences=differences)
            if policy:report[label]['intentional_dimension_refusal_hresult_policy']=policy
            if differences:raise ValueError(label+' caller/pixel mismatch: '+repr(differences[:10]))
            guarded=(out/(label+'-guarded')/name).read_bytes()
            if len(guarded)!=len(right):raise ValueError('Incomplete guarded '+label)
            gv=struct.unpack('<'+str(len(guarded)//4)+'I',guarded)
            guard_diff=[dict(word=i,case=i//words,field=i%words,plain=x,guarded=y) for i,(x,y) in enumerate(zip(bv,gv)) if x!=y]
            allowed=[d for d in guard_diff if label=='leases' and d['field']==9 and bv[d['case']*words+3]==0 and d['plain']==0 and d['guarded']==0x887601ae]
            if guard_diff!=allowed:raise ValueError('Unexpected guarded policy difference: '+repr(guard_diff[:10]))
            if label=='leases' and len(allowed)!=64:raise ValueError('Guarded admission denominator')
            report[label]['guarded']=dict(sha256=hashlib.sha256(guarded).hexdigest(),intentional_getdc_during_lock_refusals=allowed)

        report['provider']=dict(wine_version=subprocess.check_output(['wine','--version'],text=True).strip(),
            installed_modules={name:hashlib.sha256((Path('/usr/lib/wine/i386-windows')/name).read_bytes()).hexdigest() for name in ['ddraw.dll','wined3d.dll']})
        report['success']=True
    except (OSError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if lock is not None:
            try:
                run('prefix-stop',['wineserver','-k']);run('prefix-retired',['wineserver','-w'])
            except (OSError,ValueError,subprocess.SubprocessError) as error:report.update(success=False,prefix_error=str(error))
            lock.close()
        if xserver is not None:
            xserver.terminate()
            try:xserver.wait(timeout=10)
            except subprocess.TimeoutExpired:xserver.kill();xserver.wait()
        if xlog is not None:xlog.close()
        try:
            run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after']=any(c['label']=='manifest-before' and c['exit']==0 for c in report['commands'])
        except (OSError,ValueError,subprocess.SubprocessError) as error:report.update(success=False,manifest_error=str(error))
        report['sources_stable']=all(evidence_digest(ROOT/name,root=ROOT)==h for name,h in sources.items());report['success'] &= report['sources_stable']
        sha=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=a.report,sha256=sha,error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
