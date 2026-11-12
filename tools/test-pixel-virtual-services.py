#!/usr/bin/env python3
"""Recover three original virtual calls with scripted and real-driver endpoints."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile

from coverage_evidence import read_bytes, read_json, write_report
from original_x86 import PE32

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = [(0x58ad90,0x58b263),(0x58b660,0x58b815),(0x58d3b0,0x58d4a7),
          (0x58d770,0x58d8b6),(0x59bee0,0x59bf07)]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def check_driver(raw):
    size = 48+64+1024
    if len(raw) != size*64:
        raise ValueError('Incomplete real-driver matrix: '+str(len(raw)))
    rows, seen, targets = [], set(), set()
    for at in range(0,len(raw),size):
        kind,fmt,w,h,early,result,count,rva,module,distinct,source_pitch,dest_pitch = struct.unpack_from('<12I',raw,at)
        key = (kind,fmt,w,h,early)
        if key in seen or kind not in (0,1) or fmt not in (0,1) or w not in (1,3,7,16) or h not in (1,5) or early not in (0,1):
            raise ValueError('Unexpected or repeated driver cell')
        seen.add(key)
        if module != 1 or distinct != 2 or not rva:
            raise ValueError('Missing real Surface1/2 aliases or concrete ddraw target')
        targets.add(rva)
        if count>4 or source_pitch<2*w or dest_pitch<2*w or source_pitch%2 or dest_pitch%2:
            raise ValueError('Invalid driver event count/pitch')
        events = [list(struct.unpack_from('<4I',raw,at+48+16*i)) for i in range(count)]
        want = []
        if early: want.append([0x58b80c,0])
        if not kind:
            if early: want.append([0x58b80c,1])
            want += [[0x58d48d,0],[0x58d49a,1]]
        else: want.append([0x58d8ae,0])
        if count != len(want) or [e[:2] for e in events] != want or any(e[2]!=1 for e in events):
            raise ValueError('Original driver caller/order/base-pointer mismatch')
        if result != events[-1][3] or any(e[3] != (0x88760248 if early and e[0]!=0x58b80c else 0) for e in events):
            raise ValueError('Unexpected actual Unlock HRESULT/propagation')
        words = struct.unpack_from('<512H',raw,at+112)
        before=[list(words[:128]),list(words[128:256])]
        after=[list(words[256:384]),list(words[384:])]
        expected=[list(before[0]),list(before[1])]
        if not kind: expected[1][:w]=before[0][:w]
        else:
            shift,mask,maximum,add=(10,0x3e0,31,16) if fmt else (11,0x7e0,63,32)
            for i in range(w*h):
                if (i//w)*(source_pitch//2)+i%w >= w*h:
                    continue # The original brighten traverses flat words, including padding.
                p=before[0][i]
                expected[0][i]=(min(31,((p>>shift)&31)//2+16)<<shift) | (min(maximum,((p>>6)&(mask>>6))+add)<<5) | min(31,((p&255)>>1 & 15)+16)
        if after != expected:
            raise ValueError('Independent real-driver pixel mismatch '+str(key))
        rows.append(dict(kind=kind,format=fmt,width=w,height=h,early=early,result=result,events=events,target_rva=rva,source_pitch=source_pitch,dest_pitch=dest_pitch))
    return dict(cases=len(rows),target_rvas=sorted(targets),rows=rows,
                target_dispatches=96,compared_word_slots=64*512,
                readable_logical_words=4*sum(r['width']*r['height'] for r in rows))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode',choices=['scripted','driver'],required=True)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--report',required=True)
    parser.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64')
    parser.add_argument('--prefix-path',type=Path,help='New private prefix; may be on bounded tmpfs')
    parser.add_argument('--shared-fixture-prefix',type=Path,help='Marked idle fixture prefix; exclusively leased, never a game session')
    args=parser.parse_args()
    declaration=json.loads(args.claims.read_text())
    sources=declaration['sources']
    def stable(): return all(sha(read_bytes(ROOT/p))==h for p,h in sources.items())
    if not stable(): raise ValueError('Prospective sources changed')
    if (ROOT/args.report).exists(): raise ValueError('Report already exists')
    scratch=ROOT/'working/tests/pixel-virtual-services'
    scratch.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix=args.mode+'-',dir=scratch))
    report=dict(schema=1,success=False,mode=args.mode,sources=sources,claims=declaration['claims'],
        original_sha256=BUILD,original_instruction_bytes_patched=False,live_game_executed=False,
        native_renderer_executed=False,experiment=str(run.relative_to(ROOT)),commands=[])
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',TMPDIR=str(run))
    def command(label,argv,timeout=120,expected=(0,)):
        r=subprocess.run(list(map(str,argv)),cwd=run,env=env,capture_output=True,timeout=timeout)
        (run/(label+'.log')).write_bytes(r.stdout+r.stderr)
        report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=r.returncode))
        if r.returncode not in expected: raise ValueError(label+' failed: '+(r.stdout+r.stderr)[-2000:].decode(errors='replace'))
        return r.stdout
    prefix_lock=None
    try:
        command('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        original=(ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()
        if sha(original)!=BUILD: raise ValueError('Wrong original executable')
        image=PE32(original)
        report['original_ranges']={hex(a):dict(end=b,sha256=sha(image.read(a,b-a))) for a,b in RANGES}
        report['surface2_iid']=image.read(0x5c7990,16).hex()
        if report['surface2_iid']!='85588057ec6ecf119441a82303c10e27': raise ValueError('Wrong original Surface2 IID')
        if args.mode=='scripted':
            report['variants']={}
            for name,flags in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
                exe=run/name
                command(name+'-build',['g++','-m32','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',*flags,
                    '-MMD','-MF',run/(name+'.d'),ROOT/'tests/pixel-virtual-services-reference.cpp','-o',exe])
                dependencies=shlex.split((run/(name+'.d')).read_text().replace('\\\n',' ').split(':',1)[1])
                missing={str(Path(p).resolve().relative_to(ROOT)) for p in dependencies if Path(p).resolve().is_relative_to(ROOT)}-sources.keys()
                if missing: raise ValueError('Undeclared compiled sources: '+repr(missing))
                result=json.loads(command(name,[exe,ROOT/'original/Arcane_Nocd/Chaos.exe']))
                if result!={'success':True,'cases':512,'factories':1024,'target_dispatches':768,'physical_words':4194304,'original_text_unchanged':True}:
                    raise ValueError('Incomplete scripted matrix: '+repr(result))
                report['variants'][name]=result
        else:
            prefix=args.shared_fixture_prefix.resolve() if args.shared_fixture_prefix else args.prefix_path.resolve() if args.prefix_path else run/'prefix'
            if prefix.exists() and not args.shared_fixture_prefix: raise ValueError('Private prefix already exists')
            prefix.parent.mkdir(parents=True,exist_ok=True)
            env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=b',LIBGL_ALWAYS_SOFTWARE='1')
            (run/'original.exe').write_bytes(original)
            imports={'VirtualAlloc':16,'VirtualProtect':16,'VirtualQuery':12,'GetFileSize':8,'LoadLibraryA':4,'GetProcAddress':8,'GetModuleFileNameA':12,
                     'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
            definition=run/'kernel32.def'
            definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
            command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',run/'kernel32.lib'])
            command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx',
                '-Wall','-Wextra','-Werror','-c',ROOT/'tests/pixel-virtual-services-driver.c','-o',run/'probe.obj'])
            command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/include:_original_reservation',
                '/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),run/'probe.obj',run/'kernel32.lib'])
            probe=(run/'probe.exe').read_bytes();opt=struct.unpack_from('<I',probe,60)[0]+24
            if struct.unpack_from('<I',probe,opt+16)[0]<32*1024*1024: raise ValueError('Harness overlaps original reservation')
            if args.shared_fixture_prefix:
                from drawing_fixture_prefix import prepare
                owner=Path(read_json(prefix.parent/'fixture-prefix-owner.json')['root'])
                def common(path):
                    result=subprocess.check_output(['git','-C',str(path),'rev-parse','--git-common-dir'],text=True).strip()
                    return (path/result).resolve()
                if common(owner)!=common(ROOT): raise ValueError('Fixture belongs to another checkout')
                prefix,prefix_lock=prepare(owner,run,None,prefix)
                report['fixture_prefix']=dict(path=str(prefix),exclusive_lock=True,idle_before_use=True)
                report['prefix_registry_before']={name:write_report(ROOT,Path(args.report).parent/(Path(args.report).stem+'-registry-before-'+name+'.bin'),(prefix/name).read_bytes()) for name in ['system.reg','user.reg','userdef.reg']}
            else:
                command('prefix',['cp','-a','--reflink=auto',args.prefix_template.resolve(),prefix])
            try:
                command('wineboot',['xvfb-run','-a','wineboot','-u'],180)
                command('driver',['xvfb-run','-a','wine',run/'probe.exe'],180)
                report['driver']=check_driver((run/'outputs.bin').read_bytes())
                report['environment']=dict(wine=command('wine-version',['wine','--version']).decode().strip(),
                    ddraw_sha256=sha((prefix/'drive_c/windows/syswow64/ddraw.dll').read_bytes()),override='ddraw=b',
                    cooperative_level='NORMAL',caps=0x840)
            finally:
                command('stop-private-prefix',['wineserver','-k'],expected=(0,1))
                command('private-prefix-stopped',['wineserver','-w'])
                if prefix_lock:
                    report['prefix_registry_after']={name:write_report(ROOT,Path(args.report).parent/(Path(args.report).stem+'-registry-after-'+name+'.bin'),(prefix/name).read_bytes()) for name in ['system.reg','user.reg','userdef.reg']}
        report['success']=True
    except Exception as error:
        report['error']=repr(error)
    finally:
        try: command('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
        except Exception as error: report.update(success=False,manifest_error=repr(error))
        report['sources_stable']=stable()
        report['input_unchanged']=sha((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())==BUILD
        report['original_manifest_verified_before_after']=all(any(c['label']==label and c['returncode']==0 for c in report['commands']) for label in ['manifest-before','manifest-after'])
        report['success']=report['success'] and report['sources_stable'] and report['input_unchanged'] and report['original_manifest_verified_before_after']
        artifacts={}
        parent=Path(args.report).parent/(Path(args.report).stem+'-artifacts')
        for path in sorted(run.iterdir()):
            if path.is_file() and path.name!='original.exe':
                target=parent/(path.name if path.suffix in {'.bin','.log','.json'} else path.name+'.bin')
                artifacts[str(target)]=dict(sha256=write_report(ROOT,target,path.read_bytes()),bytes=path.stat().st_size)
        report['artifacts']=artifacts
        if prefix_lock: prefix_lock.close()
        digest=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=args.report,sha256=digest,error=report.get('error'),variants=report.get('variants'),
            driver={k:v for k,v in report.get('driver',{}).items() if k!='rows'})),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
