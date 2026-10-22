#!/usr/bin/env python3
"""Check supplied DirectX6 native DirectDraw admission without executing an installer or permitting builtin fallback."""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import socket
import time
import secrets
import shutil
import re

from coverage_claims import behavior_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
DIBENG='b7c0b676cd8c9960da4c53deae8de846d1251e9534ef4e4b2f05c988306dd2bd'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    p.add_argument('--prefix-template',type=Path,required=True)
    p.add_argument('--session-prefix',type=Path,required=True)
    p.add_argument('--windows-version',choices=['default','win98'],default='default')
    p.add_argument('--dibeng',type=Path,help='Optional archived Windows98 DIB Engine; accepts only the reviewed SHA-256')
    p.add_argument('--runtime-variant',choices=['win98','win95'],default='win98')
    p.add_argument('--trace-module',action='store_true',help='Retain module/Win16 selector diagnostics')
    p.add_argument('--trace-admission',action='store_true',help='Retain DPMI emulation and API relay diagnostics through the actual loader outcome')
    p.add_argument('--trace-dpmi',action='store_true',help='Retain DPMI service dispatch without the large API relay trace')
    a=p.parse_args();out=a.output.resolve();prefix=a.session_prefix.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or not prefix.is_relative_to(ROOT/'working/tests'):
        p.error('Require a new retained output under working/tests')
    out.mkdir(parents=True)
    (out/'tmp').mkdir()
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bs={b['id']:b for b in r['behaviors']};builds={b['id']:b for b in r['builds']}
    # This is an environment diagnostic, not execution of the broad game
    # contracts. Do not make unrelated native render sources its input closure.
    ids=['NP.legacy-ddraw-admission']
    paths=set().union(*(required_sources(bs[i]) for i in ids))
    paths.update(['tools/coverage_claims.py','runtime/shadow/win32_min.h'])
    sources={n:sha(ROOT/n) for n in sorted(paths)}
    claims=[{'behavior':i,'contract_sha256':behavior_contract(bs[i],builds)} for i in ids]
    report={'schema':1,'success':False,'original_sha256':BUILD,'sources':sources,
            'claims':claims,'scope':__doc__,'original_instruction_bytes_patched':False,
            'scripted_hresult':False,'live_replacement':False,'physical_driver_equivalence':False}
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(TMPDIR=str(out/'tmp'),WINEDEBUG='+loaddll,+seh',WINEPREFIX=str(prefix),
               WINEDLLOVERRIDES='ddraw=n;ddraw16=n',LIBGL_ALWAYS_SOFTWARE='1',MNM_ORIGINAL_IMAGE='Z:'+str(a.executable.resolve()).replace('/','\\'))
    if a.trace_module:env['WINEDEBUG']+=',+module'
    if a.trace_admission:env['WINEDEBUG']+=',+int31,+relay'
    elif a.trace_dpmi:env['WINEDEBUG']+=',+int31'
    report['wine_debug']=env['WINEDEBUG']

    def confined(command):
        # Keep downloaded and original Windows modules confined to this
        # experiment even when the manifest checker needs host temp files.
        return ['bwrap','--die-with-parent','--ro-bind','/','/',
                '--bind',str(out),str(out),'--bind',str(prefix),str(prefix),
                '--bind',str(out/'tmp'),'/tmp',
                '--dev','/dev','--proc','/proc','--',*command]

    def run(name,command,timeout=120):
        print(name,flush=True)
        if str(command[0])=='wine':command=confined(command)
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=out,env=env,stdout=log,
                           stderr=subprocess.STDOUT,check=True,timeout=timeout)
    before=False;xserver=None;xlog=None;lock=None;owned_prefix=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        if sha(a.executable)!=BUILD:raise ValueError('Unsupported original input')
        archives={'win98':('Win98/dx6core.exe','5f29f38abc8a9cd3ebb90812f377067d401565fdf044e82e5923f6490608839b'),
                  'win95':('Win95/Dx6eng.exe','a9daa6ec591514337ccfa3df83e363bfa29010ffccf654c0e14ff55dabf98f81')}
        relative,archive_hash=archives[a.runtime_variant]
        archive=ROOT/'original/MagicMayhem_CD/DirectX6'/relative
        if sha(archive)!=archive_hash:raise ValueError('Supplied DirectX6 archive changed')
        names=['DDRAW.DLL','DDRAW16.DLL','DDRAW.VXD','DDHELP.EXE']
        run('legacy-extract',['7z','e','-y','-o'+str(out),archive,*names])
        report['legacy_archive']=dict(path=archive.relative_to(ROOT).as_posix(),sha256=archive_hash)
        report['runtime_variant']=a.runtime_variant
        report['supplied_runtime_files']={name:sha(out/name) for name in names}
        report['runtime_windows_path_lengths']={name:len('Z:'+str(out/name)) for name in names}
        report['builtin_ddraw_fallback_allowed']=False
        if a.dibeng:
            dependency=a.dibeng.resolve()
            if sha(dependency)!=DIBENG:raise ValueError('Unreviewed DIBENG.DLL input')
            shutil.copyfile(dependency,out/'DIBENG.DLL')
            report['additional_runtime_files']={'DIBENG.DLL':dict(
                input=str(dependency.relative_to(ROOT)),sha256=DIBENG,
                size=(out/'DIBENG.DLL').stat().st_size,
                publisher_authentication_established=False)}
            env['WINEDLLOVERRIDES']+=';dibeng=n'
        report['dll_overrides']=env['WINEDLLOVERRIDES']
        imports={'GetEnvironmentVariableA' :12,'CreateFileA':28,'GetFileSize':8,
                 'ReadFile':20,'CloseHandle':4,'GetProcessHeap':0,'HeapAlloc':12,
                 'HeapFree':12,'VirtualAlloc':16,'WriteFile':20,'ExitProcess':4}
        imports.update(GetModuleFileNameA=12,GetModuleHandleA=4,VirtualProtect=16,Sleep=4,LoadLibraryA=4,GetProcAddress=8,GetLastError=0,SetLastError=4)
        definition=out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        run('original-build',['clang','--target=i686-pc-windows-msvc','-O2',
            '-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx',
            '-Wall','-Wextra','-Werror','-MD','-MF',out/'probe.d','-c',
            ROOT/'tests/legacy-ddraw-admission-reference.c','-o',out/'probe.obj'])
        closure=[]
        for name in (out/'probe.d').read_text().replace('\\\n',' ').split()[1:]:
            path=Path(name)
            if path.is_file() and path.resolve().is_relative_to(ROOT):
                name=path.resolve().relative_to(ROOT).as_posix()
                if name not in sources:raise ValueError('Undeclared compiler input: '+name)
                closure.append(name)
        report['compiled_original_sources']=closure
        pad=out/'private-image-pad.S'
        pad.write_text('.text\n.globl _original_fixture_pad\n_original_fixture_pad:\n.space 0x400000\n')
        run('original-pad',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj'])
        run('original-link',['lld-link','/machine:x86','/entry:start','/subsystem:console',
            '/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0',
            '/include:_original_fixture_pad','/out:'+str(out/'probe.exe'),out/'pad.obj',
            out/'probe.obj',out/'kernel32.lib'])
        lock=(prefix.parent/(prefix.name+'.lock')).open('a');fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        marker=prefix/'.original-recovery-session'
        if prefix.exists():
            if not marker.is_file() or marker.read_text()!=BUILD+'\n':raise ValueError('Unowned experiment prefix')
        else:
            run('prefix',['cp','-a','--reflink=auto',a.prefix_template.resolve(),prefix]);marker.write_text(BUILD+'\n')
        owned_prefix=True;report['session_prefix']=str(prefix.relative_to(ROOT))
        report['wine_version']=subprocess.check_output(['wine','--version'],text=True).strip()
        report['installed_abi_sha256']=sha(Path('/usr/include/wine/windows/ddraw.h'))
        # The host /tmp quota can prevent X's Unix socket/lock creation. Use an
        # isolated TCP-only server, with no Unix socket or lock file. Wine's
        # desktop and prefix remain private to this experiment.
        for display in range(190,290):
            with socket.socket() as candidate:
                try:candidate.bind(('127.0.0.1',6000+display))
                except OSError:continue
                break
        else:raise ValueError('No isolated X display available')
        authority=out/'Xauthority'
        fields=(b'',str(display).encode(),b'MIT-MAGIC-COOKIE-1',secrets.token_bytes(16))
        authority.write_bytes(struct.pack('>H',65535)+b''.join(struct.pack('>H',len(v))+v for v in fields))
        authority.chmod(0o600);env['XAUTHORITY']=str(authority)
        xlog=(out/'xserver.log').open('x')
        xserver=subprocess.Popen(['bwrap','--die-with-parent','--unshare-user','--uid','0','--gid','0',
            '--ro-bind','/','/','--bind',str(out/'tmp'),'/tmp','--setenv','TMPDIR','/tmp',
            '--','Xvfb',':'+str(display),'-screen','0','1024x768x24',
            '-extension','GLX',
            '-nolock','-nolisten','unix','-listen','tcp','-auth',str(authority)],
            stdout=xlog,stderr=subprocess.STDOUT,env=env)
        for _ in range(100):
            if xserver.poll() is not None:raise ValueError('Isolated X server exited')
            try:
                with socket.create_connection(('127.0.0.1',6000+display),timeout=.1):break
            except OSError:time.sleep(.05)
        else:raise ValueError('Isolated X server did not become ready')
        env['DISPLAY']='127.0.0.1:'+str(display)
        if a.windows_version!='default':
            run('version',['wine','reg','add',r'HKCU\Software\Wine\AppDefaults\probe.exe',
                '/v','Version','/t','REG_SZ','/d',a.windows_version,'/f'])
        report['application_windows_version']=a.windows_version
        run('legacy-execute',['wine',out/'probe.exe'],timeout=60)
        report['xserver_alive_after_probe']=xserver.poll() is None
        if not report['xserver_alive_after_probe']:raise ValueError('Isolated X server failed during probe')
        rows=list(struct.iter_unpack('<6I',(out/'legacy-admission.bin').read_bytes()))
        if not rows or rows[-1][0]!=99:raise ValueError('Incomplete native runtime admission')
        stage=rows[-1][4]
        loader_log=(out/'legacy-execute.log').read_text()
        probe_thread=re.search(r'(?m)^([0-9a-f]+):trace:loaddll:.*probe\.exe',loader_log)
        report['native_16bit_modules']=sorted(set(re.findall(
            r'MODULE_LoadModule16 Loaded module "([^"]+)" : native',loader_log)))
        report['loader_error']=next((r[3] for r in rows if r[0]==90 and not r[4]),None)
        report['readonly_loader_exception_observations']=[list(r) for r in rows if r[0] in {87,88,89}]
        report['observer_handles_exceptions_or_changes_protection']=False
        if probe_thread:
            report['probe_exception_codes']=sorted(set(re.findall(
                r'(?m)^'+probe_thread[1]+r':trace:seh:dispatch_exception code=([0-9a-f]+)',loader_log)))
        report.update(probe_completed=True,admission_stage=stage,legacy_surface_available=stage==5,
            loader_and_create_results=[list(r) for r in rows if r[0]>=90],
            original_game_wrappers_executed=False,original_driver_surface_equivalence_proved=False)
        module_path=out/'legacy-module-path.bin'
        if module_path.exists():
            name=module_path.read_bytes().decode('ascii');report['loaded_module_path']=name
            expected='Z:'+str(out/'DDRAW.DLL').replace('/','\\')
            if name.lower()!=expected.lower():raise ValueError('Native DLL path differs from supplied runtime')
        if stage!=5:report['remaining_boundary']='Actual supplied1998 runtime does not admit an offscreen surface on this Wine host; legacy OS/HAL outcomes remain unavailable in this environment. Inspect this run loader log for the actual stopping dependency or API; the failure is not assumed to be missing DIBENG.'
        if sha(a.executable)!=BUILD:raise ValueError('Original input changed')
        report.update(success=True,probe_sha256=sha(out/'probe.exe'),
                      original_result_sha256=sha(out/'legacy-admission.bin'))
    except (OSError,ValueError,subprocess.SubprocessError) as e:report['error']=str(e)
    finally:
        if owned_prefix:
            subprocess.run(confined(['wineserver','-k']),env=env,timeout=10,check=False)
        if xserver is not None:
            xserver.terminate()
            try:xserver.wait(timeout=10)
            except subprocess.TimeoutExpired:xserver.kill();xserver.wait()
        if xlog is not None:xlog.close()
        if lock is not None:lock.close()
        if before:
            try:
                run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
                report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as e:report.update(success=False,preservation_error=str(e))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
