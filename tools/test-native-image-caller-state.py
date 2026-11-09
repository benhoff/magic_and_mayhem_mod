#!/usr/bin/env python3
"""Bounded real PE32 image-copy caller-state diagnostic; never player admission."""
import argparse
from collections import Counter
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shlex
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
MAIN=(ROOT/Path(subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--git-common-dir'],text=True).strip())).resolve().parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PREFIX='research/runtime/caller-state-results-20261108/'

def sha(raw):return hashlib.sha256(raw).hexdigest()
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

def analyze(raw):
    if len(raw)<16 or struct.unpack_from('<4I',raw)!=(0x494d4e4d,0x31305343,1,164) or (len(raw)-16)%164:
        raise ValueError('Partial/wrong diagnostic wire')
    counts=Counter();examples=[]
    for index,at in enumerate(range(16,len(raw),164)):
        w=struct.unpack_from('<41I',raw,at)
        if w[0]!=index+1:raise ValueError('Missing diagnostic ordinal')
        n=index;seed=n%12;n//=12;mode=n%9;n//=9;y=n%8-3;n//=8;x=n%11-4;n//=11
        if list(w[1:6])!=[n,x&0xffffffff,y&0xffffffff,mode,seed]:raise ValueError('Repeated/missing declared fixture domain')
        if not w[6]or not w[7]or abs(w[6]-w[7])<4096:raise ValueError('Original/native destination storage is not independent')
        if w[40]&~7:raise ValueError('Unknown diagnostic check bits')
        a,b=list(w[8:24]),list(w[24:40])
        pitch=0xffffffc0 if mode==8 else 64
        if a[9]!=w[6]or b[9]!=w[7]or a[10]!=pitch or b[10]!=pitch or a[11]!=0 or b[11]!=0 or a[13]!=w[3]or b[13]!=w[3]or not a[14]or a[14]!=b[14]:
            raise ValueError('Captured arguments differ from the declared fixture input')
        # Distinct independently seeded destinations have distinct identities.
        def destination(value,base):
            offset=2048 if w[4]==8 else 512
            return ('destination-offset',value-base) if base-offset<=value<=base+4096-offset else ('literal',value)
        checks={name:a[i]==b[i] for i,name in enumerate(['eax','ecx','edx_raw','ebx','esp','ebp','esi','edi'])}
        checks['edx_destination_normalized']=destination(a[2],w[6])==destination(b[2],w[7])
        checks['defined_integer_flags']=((a[8]^b[8])&0xcd5)==0
        for i in range(6):
            checks['retired_argument_'+str(i)]=(destination(a[9+i],w[6])==destination(b[9+i],w[7])) if i==0 else a[9+i]==b[9+i]
        checks['last_error']=a[15]==b[15]
        checks.update(complete_destination=bool(w[40]&1),source_unchanged=bool(w[40]&2),fx_except_instruction_history=bool(w[40]&4))
        failed=[name for name,ok in checks.items() if not ok and name!='edx_raw']
        for name,ok in checks.items():counts[name+'_matches']+=int(ok)
        counts['cases']+=1;counts['complete_state_matches']+=not failed
        if failed and len(examples)<8:examples.append(dict(case=index,format=w[1],x=struct.unpack('<i',struct.pack('<I',w[2]))[0],y=struct.unpack('<i',struct.pack('<I',w[3]))[0],clip=w[4],seed=w[5],different=failed,original=a,native=b))
    if counts['cases']!=19008:raise ValueError('Missing planned format/position/clip/shape/pitch/precision/rounding domain')
    return dict(counts=dict(counts),examples=examples,equivalent=counts['complete_state_matches']==counts['cases'])

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--name',required=True);parser.add_argument('--regressions',action='store_true');parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args()
    if not re.fullmatch('[a-z0-9-]+',args.name):raise ValueError('Unique lowercase acquisition name required')
    from coverage_evidence import write_report,Reader
    declaration=json.loads(args.claims.read_text())
    destination=PREFIX+args.name+'.json'
    if (ROOT/destination).exists():raise ValueError('Preserve the previous immutable report')
    scratch=ROOT/'working/caller-state-20261108';scratch.mkdir(parents=True,exist_ok=True)
    (scratch/'.retain').touch(exist_ok=True)
    report=dict(schema=1,kind='native_image_caller_state_diagnostic',success=False,experiment_completed=False,
                original_game_runs=0,newly_passed_requirements=0,original_sha256=HASH,
                claims=declaration['claims'],
                scope='Private actual production MnmDraw PE32 full533a00 entry versus unchanged pinned original bytes relocated to533d00, real Wine IntersectRect, identical seeded flags/floating state and independent complete input/destination buffers. Return GPRs, RET24 argument residue, defined integer flags, LastError and selected FX bytes are diagnostic. No natural caller liveness, installed game, all bypass families, failure/provider/exception equivalence or player-row admission.')
    prospective={}
    for folder in ['runtime','reconstruction','protocols','tests']:
        for path in (ROOT/folder).rglob('*'):
            if path.is_file() and path.suffix in ('.c','.h','.hpp','.S'):
                prospective[path.relative_to(ROOT).as_posix()]=sha(path.read_bytes())
    fixed=['tools/test-native-image-caller-state.py','tools/build-native-draw.py','tools/drawing_fixture_prefix.py','tools/experiment_environment.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/original-manifest.sh','tests/native-image-caller-state.c','tests/native-image-caller-state-probe.S','tests/test-native-image-caller-state.py','tools/test-native-extended-draw.py']
    fixed=list(dict.fromkeys(fixed+list(declaration['sources'])))
    for name in fixed:
        prospective[name]=sha((ROOT/name).read_bytes())
    if any(prospective[name]!=h for name,h in declaration['sources'].items()):raise ValueError('Prospective claim source changed')
    report['source_commit']=subprocess.check_output(['git','-C',str(ROOT),'rev-parse','HEAD'],text=True).strip()
    report['temporary_classification']='Compiler objects, import libraries, depfiles and resettable build tree are temporary; exact commands/tool versions retained. Tested EXE/DLL, state wire, logs, changed source bytes and prefix registry deltas are durable SQLite evidence.'
    commands=[];lease=None;prefix=None;env=None
    with tempfile.TemporaryDirectory(prefix='mnm-caller-state-')as temporary:
        out=Path(temporary);logs={}
        def run(label,argv,*,environment=None,timeout=180,cwd=ROOT,expected=0):
            result=subprocess.run(list(map(str,argv)),cwd=cwd,env=environment,capture_output=True,timeout=timeout)
            logs[label]=result.stdout+result.stderr
            # Preserve diagnostics immediately, including a future cleanup refusal.
            record=PREFIX+args.name+'-'+label+'.log.bin'
            write_report(ROOT,record,logs[label])
            commands.append(dict(label=label,argv=list(map(str,argv)),returncode=result.returncode,expected=expected))
            if result.returncode!=expected:raise RuntimeError(label+': exit '+str(result.returncode))
            return result.stdout
        try:
            run('manifest-before',[MAIN/'tools/original-manifest.sh','verify'])
            original=MAIN/'original/Arcane_Nocd/Chaos.exe'
            binary=original.read_bytes()
            report['original_input']=dict(path=str(original),sha256=sha(binary),bytes=len(binary))
            if sha(binary)!=HASH:raise ValueError('Original executable hash mismatch')
            text=(ROOT/'runtime/native-draw/image_copy_original.h').read_text().split('{',1)[1].split('}',1)[0]
            expected=bytes(int(s,16)for s in re.findall(r'0x([0-9a-fA-F]{2})',text))
            pe=struct.unpack_from('<I',binary,60)[0];count=struct.unpack_from('<H',binary,pe+6)[0];opt=struct.unpack_from('<H',binary,pe+20)[0]
            body=None
            for i in range(count):
                entry=pe+24+opt+40*i;vsize,rva,size,offset=struct.unpack_from('<4I',binary,entry+8)
                if rva<=0x133a00 and 0x133a00+len(expected)<=rva+size:body=binary[offset+0x133a00-rva:offset+0x133a00-rva+len(expected)]
            if body!=expected:raise ValueError('Pinned image body differs from unchanged original')
            report['original_body']=dict(address=0x533a00,bytes=len(body),sha256=sha(body),unchanged=True)
            with Reader(ROOT)as reader:
                register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
                build=next(b for b in register['builds']if b['id']=='nocd-40209ca7')
                inventory=reader.json(build['inventory'])
                matches=[i for i in inventory['imports']if i.get('iat_va')==0x5c5294]
                if len(matches)!=1 or matches[0]['dll'].lower()!='user32.dll' or matches[0]['name']!='IntersectRect':raise ValueError('Original rectangle import identity differs')
                report['rectangle_provider_import']=matches[0]
            builder=load('image_state_native_builder','tools/build-native-draw.py')
            # The builder uses subprocess directly; capture its bounded log.
            run('native-build',['python3','-c',"import sys;from pathlib import Path;sys.path.insert(0,'tools');import importlib.util;s=importlib.util.spec_from_file_location('b','tools/build-native-draw.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);m.build(Path(sys.argv[1]))",out/'native'])
            run('clang-version',['clang','--version']);run('linker-version',['lld-link','--version']);run('wine-version',['wine','--version'])
            run('probe-build',[*builder.FLAGS,'-MD','-MF',out/'probe.d','-c',ROOT/'tests/native-image-caller-state.c','-o',out/'probe.obj'])
            run('assembly-build',['clang','--target=i686-pc-windows-msvc','-c',ROOT/'tests/native-image-caller-state-probe.S','-o',out/'state.obj'])
            exe=out/'caller-state.exe';dll=out/'native/MnmDraw.dll'
            imports={name:dict(entries)for name,entries in builder.IMPORTS.items()}
            imports['kernel32'].update(LoadLibraryA=4,CreateThread=24)
            imports['user32']['GetDesktopWindow']=0;imports['gdi32']['GetDIBColorTable']=16
            libraries=[]
            for library,entries in imports.items():
                definition=out/('fixture-'+library+'.def')
                definition.write_text('LIBRARY '+library+'.dll\nEXPORTS\n'+''.join(name+'@'+str(size)+'\n'for name,size in entries.items()))
                target=out/('fixture-'+library+'.lib');libraries.append(target)
                run('fixture-imports-'+library,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',target])
            link=['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/dynamicbase:no','/base:0x400000','/timestamp:0']
            run('probe-link',[*link,'/out:'+str(exe),out/'probe.obj',out/'state.obj',*libraries])
            regression_binaries=[]
            if args.regressions:
                for label,name in [('extended','tests/native-extended-draw-probe.c'),('backing','tests/native-owned-backing-probe.c')]:
                    obj=out/(label+'.obj');target=out/(label+'.exe')
                    run(label+'-build',[*builder.FLAGS,'-MD','-MF',out/(label+'.d'),'-c',ROOT/name,'-o',obj])
                    run(label+'-link',[*link,'/out:'+str(target),obj,*libraries]);regression_binaries.append(target)
            used=set(fixed)
            for dependency in out.rglob('*.d'):
                for name in shlex.split(dependency.read_text().replace('\\\n',' ').split(':',1)[1]):
                    path=Path(name).resolve()
                    if path.is_relative_to(ROOT):used.add(path.relative_to(ROOT).as_posix())
            if used-set(prospective):raise ValueError('Undeclared compiler dependency '+str(sorted(used-set(prospective))))
            report['sources']={name:prospective[name]for name in sorted(used)}
            report['source_preimages']={}
            for name,h in report['sources'].items():
                old=subprocess.run(['git','-C',str(ROOT),'show','HEAD:'+name],capture_output=True)
                if old.returncode or sha(old.stdout)!=h:
                    raw=(ROOT/name).read_bytes();record=PREFIX+args.name+'-source-'+sha(raw)[:20]+'.bin'
                    report['source_preimages'][name]=dict(record=record,sha256=write_report(ROOT,record,raw))
            report['pre_execution_binaries']={}
            for path in [exe,dll,*regression_binaries]:
                record=PREFIX+args.name+'-tested-'+path.name+'.bin'
                raw=path.read_bytes();report['pre_execution_binaries'][path.name]=dict(record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw))
            fixture=load('image_state_prefix','tools/drawing_fixture_prefix.py');attempts=[]
            candidates=list((MAIN/'working/tests/drawing-fixture-prefix-suite').glob('*/wineprefix'))
            for candidate in sorted(candidates,key=lambda p:('wow64'not in str(p),str(p))):
                try:prefix,lease=fixture.prepare(MAIN,out,None,shared=candidate);break
                except (ValueError,OSError,subprocess.TimeoutExpired)as error:attempts.append(dict(prefix=str(candidate),refusal=str(error)))
            report['prefix_admission_refusals']=attempts
            if lease is None:raise ValueError('No idle exclusively leased fixture prefix available')
            report['prefix']=str(prefix);report['prefix_registry_before']={}
            for name in ['system.reg','user.reg','userdef.reg']:
                path=prefix/name
                if path.is_file():
                    raw=path.read_bytes();record=PREFIX+args.name+'-registry-before-'+name+'.bin'
                    report['prefix_registry_before'][name]=dict(record=record,sha256=write_report(ROOT,record,raw))
            env={k:v for k,v in os.environ.items()if not k.startswith(('MNM_','WINE'))}
            env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',DISPLAY='',TMPDIR=str(out),WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d',MNM_TEST_LIBRARY='Z:'+str(dll).replace('/','\\'),MNM_STATE_OUTPUT='Z:'+str(out/'states.bin').replace('/','\\'),MNM_NATIVE_CPU_GUARD='3',MNM_PUBLIC_RESIDENT_WORLD='1',MNM_NATIVE_OWNERSHIP_REQUIRED='1')
            env['MNM_OWNER_DRAW_DLL']=env['MNM_TEST_LIBRARY']
            setup=PREFIX+args.name+'-setup.json'
            report['prospective_setup']=dict(record=setup,sha256=write_report(ROOT,setup,(json.dumps(dict(report,stage='pre-execution'),indent=2)+'\n').encode()))
            controls=run('collector-controls',['python3','-B',ROOT/'tests/test-native-image-caller-state.py'])
            report['collector_controls_passed']=True
            run('actual-pe32-fixture',['wine',exe],environment=env,timeout=120)
            raw=(out/'states.bin').read_bytes();report['comparison']=analyze(raw)
            report['experiment_completed']=True;report['success']=report['comparison']['equivalent']
            report['original_pixels_used_as_native_inputs']=False
            report['original_reference_body_verified_after_install_and_execution']=True
            report['retired_native_entry_poisoned_after_eight_bytes']=True
            report['regressions']=[]
            for path in regression_binaries:
                directory=out/'regressions'/path.stem;directory.mkdir(parents=True)
                run('regression-'+path.stem,['wine',path],environment=env,cwd=directory,timeout=120)
                report['regressions'].append(dict(name=path.stem,exit_code=0))
            if regression_binaries:
                for label,key in [('copy-span','MNM_OWNER_BAD_COPY_SPAN'),('copy-prefix','MNM_OWNER_BAD_COPY_PREFIX')]:
                    directory=out/'regressions'/label;directory.mkdir(parents=True)
                    run('regression-'+label,['wine',out/'backing.exe'],environment=dict(env,**{key:'1'}),cwd=directory,expected=124)
                    report['regressions'].append(dict(name=label,exit_code=124,expected_refusal=True))
                directory=out/'regressions/direction';directory.mkdir(parents=True)
                run('regression-direction',['wine',exe],environment=dict(env,MNM_IMAGE_DIRECTION_REFUSAL='1'),cwd=directory,expected=124)
                report['regressions'].append(dict(name='direction',exit_code=124,expected_refusal=True))
        except Exception as error:report['error']=str(error)
        finally:
            if lease is not None:
                try:
                    retirement_env=env if env is not None else dict(os.environ,WINEPREFIX=str(prefix),WINEDEBUG='-all')
                    report['retirement_command_refusals']=[]
                    for label,argv in [('wine-retirement',['wineserver','-k']),('wine-idle',['wineserver','-w'])]:
                        try:run(label,argv,environment=retirement_env,timeout=10)
                        except Exception as error:report['retirement_command_refusals'].append(dict(command=argv,error=str(error)))
                    from experiment_environment import _busy
                    report['private_wine_stopped']=not _busy(prefix)
                    if not report['private_wine_stopped']:report['success']=False
                    report['prefix_registry_after']={}
                    for name in ['system.reg','user.reg','userdef.reg']:
                        path=prefix/name
                        if path.is_file():
                            record=PREFIX+args.name+'-registry-after-'+name+'.bin'
                            report['prefix_registry_after'][name]=dict(record=record,sha256=write_report(ROOT,record,path.read_bytes()))
                finally:lease.close()
            try:run('manifest-after',[MAIN/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']='manifest-before'in logs
            except Exception as error:report.update(success=False,manifest_error=str(error))
            report['commands']=commands;report['logs']={};report['artifacts']={}
            for label,raw in logs.items():
                record=PREFIX+args.name+'-'+label+'.log.bin';report['logs'][label]=dict(record=record,sha256=write_report(ROOT,record,raw))
            for name in ['caller-state.exe','native/MnmDraw.dll','states.bin']:
                path=out/name
                if path.is_file():
                    record=PREFIX+args.name+'-'+path.name+'.bin';raw=path.read_bytes()
                    report['artifacts'][name]=dict(record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw))
            for path in (out/'regressions').rglob('*.bin')if(out/'regressions').exists()else[]:
                name=path.relative_to(out).as_posix();raw=path.read_bytes();record=PREFIX+args.name+'-'+name.replace('/','-')+'.bin'
                report['artifacts'][name]=dict(record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw))
            report.setdefault('sources',{name:prospective[name]for name in prospective if name.startswith(('tools/','tests/'))})
            report['sources_stable']=all(sha((ROOT/name).read_bytes())==h for name,h in report['sources'].items())
            report.setdefault('source_preimages',{})
            for name,h in report['sources'].items():
                old=subprocess.run(['git','-C',str(ROOT),'show','HEAD:'+name],capture_output=True)
                if old.returncode or sha(old.stdout)!=h:
                    raw=(ROOT/name).read_bytes();record=PREFIX+args.name+'-source-'+sha(raw)[:20]+'.bin'
                    report['source_preimages'][name]=dict(record=record,sha256=write_report(ROOT,record,raw))
            report['success']=report['success']and report['sources_stable']and report.get('private_wine_stopped',False)
            value=write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode())
            print(json.dumps(dict(success=report['success'],completed=report['experiment_completed'],report=destination,sha256=value,error=report.get('error'),comparison=report.get('comparison'))),flush=True)
    return 0 if report['success']else 1

if __name__=='__main__':raise SystemExit(main())
