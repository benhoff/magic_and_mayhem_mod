#!/usr/bin/env python3
"""Compare actual original particle ownership and mutation under nested insertions."""
import argparse
import fcntl
import hashlib
import itertools
import json
import os
from pathlib import Path
import struct
import subprocess
from coverage_claims import behavior_contract,required_sources

ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def validate(path):
    records=cases=events=stranded=0
    with path.open('rb') as f:
        for count,pattern,priority,hint,spawn,life in itertools.product((0,1,3,1791,1792),range(4),(0,1024,1025,2000,-1),(0,1024,4096),range(3),range(4)):
            for stage in range(3):
                raw=f.read(56)
                if len(raw)!=56:raise ValueError('Incomplete pool stage header')
                h=struct.unpack('<14I',raw)
                if h[:8]!=(cases,stage,count,pattern,priority&0xffffffff,hint,spawn,life) or h[8]>1792 or h[11]>=8192 or h[12]>16384:
                    raise ValueError('Pool denominator/identity/extent')
                tokens=f.read(1792*4)
                if len(tokens)!=1792*4:raise ValueError('Truncated pool identities')
                values=struct.unpack('<1792I',tokens);active=values[:h[8]]
                if any(not v or v>h[11] for v in active) or len(set(active))!=len(active):raise ValueError('Dangling/duplicate pool generation')
                if any(values[h[8]:]):raise ValueError('Uncleared inactive pool slot')
                if len(f.read(h[12]*12))!=h[12]*12:raise ValueError('Truncated ownership events')
                if stage==2:
                    if h[8] or h[9]:raise ValueError('Clear did not retire the pool')
                    stranded+=h[10]
                events+=h[12];records+=1
            cases+=1
        if cases!=3600 or records!=10800 or f.read(1):raise ValueError('Pool matrix incomplete/trailing')
    return dict(cases=cases,stage_snapshots=records,ordered_events=events,
                observed_unadopted_allocations_before_caller_cleanup=stranded)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--session-prefix',type=Path,required=True);a=p.parse_args();out=a.output.resolve();prefix=a.session_prefix.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or not prefix.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output and an isolated experiment prefix')
    out.mkdir(parents=True);(out/'tmp').mkdir()
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());bs={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']};ids=['RS.particle-pool-ownership']
    paths=set().union(*(required_sources(bs[i]) for i in ids))|{'tools/coverage_claims.py'};sources={n:sha(ROOT/n) for n in sorted(paths)}
    report=dict(schema=1,success=False,scope=bs[ids[0]]['scope'],sources=sources,claims=[dict(behavior=i,contract_sha256=behavior_contract(bs[i],builds)) for i in ids],original_sha256=PIN,actual_original_custom_allocator=True,virtual_update_priority_inputs_owned=True,live_replacement=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))};env.update(TMPDIR=str(out/'tmp'),WINEDEBUG='-all',WINEPREFIX=str(prefix),MNM_ORIGINAL_IMAGE='Z:'+str(ROOT/'original/Arcane_Nocd/Chaos.exe').replace('/','\\'))
    def run(label,command,timeout=240,output=None):
        print(label,flush=True)
        with (out/(label+'.log')).open('x') as log:
            if output:
                with (out/output).open('xb') as target:subprocess.run([str(x) for x in command],cwd=out,env=env,stdout=target,stderr=log,check=True,timeout=timeout)
            else:subprocess.run([str(x) for x in command],cwd=out,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=timeout)
    before=False;owned_prefix=False;lock=None
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        if sha(ROOT/'original/Arcane_Nocd/Chaos.exe')!=PIN:raise ValueError('Original changed')
        imports={'GetEnvironmentVariableA':12,'CreateFileA':28,'GetFileSize':8,'ReadFile':20,'CloseHandle':4,'GetProcessHeap':0,'HeapAlloc':12,'HeapFree':12,'VirtualAlloc':16,'WriteFile':20,'ExitProcess':4,'GetModuleHandleA':4,'VirtualProtect':16,'LoadLibraryA':4,'GetProcAddress':8}
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        run('original-build',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-MD','-MF',out/'probe.d','-c',ROOT/'tests/particle-pool-reference.c','-o',out/'probe.obj'])
        for name in (out/'probe.d').read_text().replace('\\\n',' ').split()[1:]:
            file=Path(name)
            if file.is_file() and file.resolve().is_relative_to(ROOT) and file.resolve().relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared compiler dependency: '+name)
        pad=out/'pad.S';pad.write_text('.text\n.globl _original_fixture_pad\n_original_fixture_pad:\n.space 0x400000\n')
        run('original-pad',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj'])
        run('original-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/include:_original_fixture_pad','/out:'+str(out/'probe.exe'),out/'pad.obj',out/'probe.obj',out/'kernel32.lib'])
        run('native-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',ROOT/'tests/particle-pool-native.cpp','-o',out/'native'])
        lock=(prefix.parent/(prefix.name+'.lock')).open('a');fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        marker=prefix/'.original-recovery-session'
        if prefix.exists():
            if not marker.is_file() or marker.read_text()!=PIN+'\n':raise ValueError('Unowned experiment prefix')
        else:
            run('prefix',['cp','-a','--reflink=auto',a.prefix_template.resolve(),prefix]);marker.write_text(PIN+'\n')
        owned_prefix=True;report['session_prefix']=str(prefix.relative_to(ROOT));report['wine_version']=subprocess.check_output(['wine','--version'],text=True).strip()
        run('original',['wine',out/'probe.exe'])
        run('native',[out/'native'],output='native-pool.bin')
        original=out/'particle-pool.bin';native=out/'native-pool.bin'
        a_result=validate(original);b_result=validate(native)
        if a_result!=b_result or original.stat().st_size!=native.stat().st_size:raise ValueError('Pool denominator differs')
        if sha(original)!=sha(native):
            with original.open('rb') as o,native.open('rb') as n:
                at=0
                while True:
                    left=o.read(65536);right=n.read(65536)
                    if left!=right:
                        offset=next(i for i,(x,y) in enumerate(zip(left,right)) if x!=y);report['mismatch_byte']=at+offset;report['mismatch_original']=left[max(0,offset-24):offset+40].hex();report['mismatch_native']=right[max(0,offset-24):offset+40].hex();break
                    if not left:break
                    at+=len(left)
            raise ValueError('Original/native particle pool differs')
        report.update(success=True,**a_result,bytes_compared=original.stat().st_size,outputs_sha256=dict(original=sha(original),native=sha(native)),
            original_custom_heap_retired=True,all_fixture_allocations_retired_after_explicit_caller_cleanup=True,
            original_manager_instruction_bodies_unpatched=True,original_delete_entry_checked_forwarding=True,
            binaries_sha256={n:sha(out/n) for n in ['native','probe.exe']})
    except (OSError,ValueError,subprocess.SubprocessError) as error:report.update(success=False,error=str(error))
    finally:
        if owned_prefix:subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
        if lock:lock.close()
        if before:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except subprocess.SubprocessError as error:report.update(success=False,preservation_error=str(error))
        report['sources_stable']=all(sha(ROOT/n)==v for n,v in sources.items());report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
