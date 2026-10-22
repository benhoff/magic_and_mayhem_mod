#!/usr/bin/env python3
"""Observe actual CPU heap backing generations through transparent Wine calloc/free forwarding."""
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

from coverage_claims import behavior_contract, required_sources

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--executable',type=Path,default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    p.add_argument('--prefix-template',type=Path,required=True)
    p.add_argument('--session-prefix',type=Path,required=True)
    a=p.parse_args();out=a.output.resolve();prefix=a.session_prefix.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or not prefix.is_relative_to(ROOT/'working/tests'):
        p.error('Require a new retained output under working/tests')
    out.mkdir(parents=True)
    (out/'tmp').mkdir()
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bs={b['id']:b for b in r['behaviors']};builds={b['id']:b for b in r['builds']}
    ids=['RS.canvas-cpu-backing-generations']
    paths=set().union(*(required_sources(bs[i]) for i in ids))
    paths.update(['tools/coverage_claims.py','runtime/shadow/win32_min.h'])
    sources={n:sha(ROOT/n) for n in sorted(paths)}
    claims=[{'behavior':i,'contract_sha256':behavior_contract(bs[i],builds)} for i in ids]
    report={'schema':1,'success':False,'original_sha256':BUILD,'sources':sources,
            'claims':claims,'scope':__doc__,'original_instruction_bytes_patched':False,
            'scripted_hresult':False,'live_replacement':False,'physical_driver_equivalence':False}
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(TMPDIR=str(out/'tmp'),WINEDEBUG='-all',WINEPREFIX=str(prefix),
               WINEDLLOVERRIDES='ddraw=b',LIBGL_ALWAYS_SOFTWARE='1',MNM_ORIGINAL_IMAGE='Z:'+str(a.executable.resolve()).replace('/','\\'))

    def run(name,command,timeout=120):
        print(name,flush=True)
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=out,env=env,stdout=log,
                           stderr=subprocess.STDOUT,check=True,timeout=timeout)
    before=False;xserver=None;xlog=None;lock=None;owned_prefix=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        if sha(a.executable)!=BUILD:raise ValueError('Unsupported original input')
        imports={'GetEnvironmentVariableA':12,'CreateFileA':28,'GetFileSize':8,
                 'ReadFile':20,'CloseHandle':4,'GetProcessHeap':0,'HeapAlloc':12,
                 'HeapFree':12,'VirtualAlloc':16,'WriteFile':20,'ExitProcess':4}
        imports.update(GetModuleHandleA=4,VirtualProtect=16,Sleep=4,LoadLibraryA=4,GetProcAddress=8,GetLastError=0,SetLastError=4)
        definition=out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        provider_pins={'ddraw.dll':'24c8c8fbbe0bac100efa5a716ebc78506e0f42e80d2af019b641911c0847ab28','wined3d.dll':'3481e6904716421951615d8ee2ca097b55ea20eee65dea6bac1596426d950c82'}
        for name,digest in provider_pins.items():
            if sha(Path('/usr/lib/wine/i386-windows')/name)!=digest:raise ValueError('Unreviewed provider module '+name)
        report['installed_provider_sha256']=provider_pins
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        run('original-build',['clang','--target=i686-pc-windows-msvc','-O2',
            '-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx',
            '-Wall','-Wextra','-Werror','-MD','-MF',out/'probe.d','-c',
            ROOT/'tests/original-canvas-backing-reference.c','-o',out/'probe.obj'])
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
            '-extension','GLX','-nolock','-nolisten','unix','-listen','tcp','-auth',str(authority)],
            stdout=xlog,stderr=subprocess.STDOUT,env=env)
        for _ in range(100):
            if xserver.poll() is not None:raise ValueError('Isolated X server exited')
            try:
                with socket.create_connection(('127.0.0.1',6000+display),timeout=.1):break
            except OSError:time.sleep(.05)
        else:raise ValueError('Isolated X server did not become ready')
        env['DISPLAY']='127.0.0.1:'+str(display)
        run('original-execute',['wine','explorer','/desktop=original-alias,1024x768',out/'probe.exe'])
        rows=list(struct.iter_unpack('<6I',(out/'retained-canvas.bin').read_bytes()))
        if rows[-1]!=(99,99,0,0,36,0):raise ValueError('Incomplete retained-interface matrix')
        for phase in range(36):
            group=[r for r in rows if r[1]==phase and r[0]>=70 and r[0]!=99]
            if sum(r[0]==75 for r in group)!=1 or sum(r[0]==78 and r[4:]==(0,0) for r in group)!=1 or sum(r[0]==80 and r[3]==0 for r in group)!=1:
                raise ValueError('Unretired wrapper or retained credit')
            expected_lease=(phase//2)%3
            pixel_checks=[r for r in group if r[0]==74]
            if len(pixel_checks)!=(3 if expected_lease==1 else 2) or any(r[4:]!=(0,48) for r in pixel_checks):raise ValueError('Incomplete full pixel comparison')
            if any(r[3] for r in group if r[0] in (70,71,72,77,79)):raise ValueError('Unexpected COM lease failure')
        report.update(cohorts=36,later_interfaces=[3,4,7],held_lock_cohorts=12,held_dc_cohorts=12,
            actual_cpu_and_gdi_writes_after_wrapper_retirement=True,full_pixel_checks=sum(r[0]==74 for r in rows),
            initialization_samples=sum(r[0]==76 for r in rows),initial_allocation_clear_not_assumed=True,
            final_retained_com_credits_retired=True,physical_allocation_or_all_writer_closure=False)
        from original_canvas_backing import analyze
        report.update(analyze(rows,(out/'provider-allocations.bin').read_bytes()))
        report['provider_allocation_journal_sha256']=sha(out/'provider-allocations.bin')
        if sha(a.executable)!=BUILD:raise ValueError('Original input changed')
        report.update(success=True,probe_sha256=sha(out/'probe.exe'),
                      original_result_sha256=sha(out/'retained-canvas.bin'))
    except (OSError,ValueError,subprocess.SubprocessError) as e:report['error']=str(e)
    finally:
        if owned_prefix:
            subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
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
