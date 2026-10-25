#!/usr/bin/env python3
"""Compare complete cursor helpers and connect them to real native surface providers."""
import argparse,hashlib,importlib.util,json,os,re,resource,shlex,shutil,struct,subprocess
from pathlib import Path
from coverage_evidence import evidence_digest,write_report
from drawing_fixture_prefix import prepare
from native_draw_retirement import require_retired
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def compare_provider(raw):
    width=250
    if len(raw)!=78*width*4:raise ValueError('Incomplete cursor provider denominator')
    for expected_index,values in enumerate(struct.iter_unpack('<'+str(width)+'I',raw)):
        row=list(values);i,gate,deferred,position,source_seed,dest_seed=row[:6]
        if i!=expected_index or (gate,deferred,position)!=(i//6,i//3%2,i%3) or row[15]:raise ValueError('Cursor provider row identity')
        source=[(source_seed+x*337+y*971)&65535 for y in range(9) for x in range(13)]
        before=[(dest_seed+x*337+y*971)&65535 for y in range(9) for x in range(13)]
        admitted=gate not in (1,2,3,4,5,6,11);copy=gate in (0,12);full=gate==12 and deferred
        expected=before[:]
        x,y=(2,3) if position==1 else (0,0)
        if copy and not full:
            for py in range(3):
                for px in range(5):expected[(py+y)*13+px+x]=source[py*13+px]
        interim=before if deferred else expected
        if full:expected[-1]=source[0]
        if row[14]!=int(full):raise ValueError('Queue-full payload preservation missing')
        if row[6:10]!=([1,1,0,1] if admitted else [0,0,0,0]) or row[10]!=int(gate not in (9,10)) or row[11]!=(128 if full else int(copy and deferred)) or any(row[12:14]):raise ValueError('Cursor wait/queue/valid-state mismatch')
        if row[16:133]!=interim or row[133:]!=expected:raise ValueError('Independent complete cursor destination mismatch at '+str(i))
    return dict(cases=78,destination_words=78*117*2,direct_and_deferred=True,queue_full_preserved=True)

def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0));p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--prefix-owner-root',type=Path,required=True);p.add_argument('--wine-prefix',type=Path,required=True);p.add_argument('--headless',action='store_true');a=p.parse_args()
    if a.headless:return subprocess.call(['xvfb-run','-a','-s','-screen 0 1024x768x24',__import__('sys').executable,'-B',__file__,*[x for x in __import__('sys').argv[1:] if x!='--headless']])
    out=a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new owned output')
    out.mkdir(parents=True);(out/'.retain').write_text('Complete cursor helpers, actual surface providers, all positive/negative artifacts.\n')
    declaration=json.loads(a.claims.read_text());sources=declaration['sources']
    stable=lambda:all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items())
    if not stable():raise ValueError('Prospective sources changed before execution')
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],original_sha256=BUILD,helper_claims=[c for c in declaration['claims'] if c['behavior'].startswith('RS.')],provider_claims=[c for c in declaration['claims'] if c['behavior']=='NP.screen-cursor-native-copy-providers'],commands=[],original_game_executed=False,live_replacement=False,physical_driver_equivalence=False,scope=__doc__+'; original local helpers use authored endpoints in arithmetic controls and actual provider wrappers/queue in bounded PE32 controls. No live installation,full screen lifetime,arbitrary concurrency or original driver parity.')
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))};env.update(WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',__GLX_VENDOR_LIBRARY_NAME='mesa',MNM_ORIGINAL_IMAGE='Z:'+str((ROOT/'original/Arcane_Nocd/Chaos.exe').resolve()).replace('/','\\'),ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    def run(label,command,extra=None,cwd=out,timeout=180):
        print(label,flush=True);log=out/(label+'.log')
        with log.open('x') as f:result=subprocess.run(list(map(str,command)),cwd=cwd,env=env|dict(extra or {}),stdout=f,stderr=subprocess.STDOUT,timeout=timeout)
        text=log.read_text(errors='replace');report['commands'].append(dict(label=label,argv=list(map(str,command)),exit=result.returncode,sha256=hashlib.sha256(log.read_bytes()).hexdigest(),log=log.relative_to(ROOT).as_posix()))
        if result.returncode:raise ValueError(label+': '+text[-2500:])
        return text
    def closure(dep):
        for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
            path=Path(token).resolve()
            if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working') and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared compiler source '+str(path))
    lock=None;prefix=None;verified=False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True;image=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if hashlib.sha256(image.read_bytes()).hexdigest()!=BUILD:raise ValueError('Original build changed')
        from original_x86 import PE32
        pe=PE32(image.read_bytes());report['original_functions']={hex(at):dict(bytes=n,sha256=hashlib.sha256(pe.read(at,n)).hexdigest()) for at,n in [(0x4a4370,11),(0x4a4380,302),(0x58d5b0,138),(0x58d660,227),(0x58c4a0,1024),(0x58acf0,154)]}
        for label,flags in [('original',['-m32','-O2','-DMNM_ORIGINAL_REFERENCE']),('sanitized',['-m32','-O1','-DMNM_ORIGINAL_REFERENCE','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('native64',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            run(label+'-build',['g++','-std=c++17','-mstackrealign','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',*flags,'-MMD','-MF',out/(label+'.d'),ROOT/'tests/screen-cursor-reference.cpp','-o',out/label]);closure(out/(label+'.d'))
            report[label]=json.loads(run(label,[out/label,*([image] if label!='native64' else [])]))
        if not report['original']==report['sanitized']==report['native64']:raise ValueError('Original/model execution totals differ')
        prefix,lock=prepare(a.prefix_owner_root.resolve(),out,a.wine_prefix.resolve(),a.wine_prefix.resolve());env['WINEPREFIX']=str(prefix);run('prefix-keepalive',['wineserver','-p'])
        spec=importlib.util.spec_from_file_location('cursor_builder',ROOT/'tools/build-native-draw.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build(out/'native-build',sources);shutil.copyfile(dll,out/'MnmDraw.dll')
        imports={name:dict(entries) for name,entries in builder.IMPORTS.items()};imports['kernel32'].update(LoadLibraryA=4,GetProcAddress=8,ReadFile=20,GetFileSize=8,CreateMutexA=12,WaitForSingleObject=8,ReleaseMutex=4);imports['gdi32'].update(SetPixel=16)
        libraries=[]
        for name,entries in imports.items():
            definition=out/(name+'.def');definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(k+'@'+str(v)+'\n' for k,v in entries.items()));library=out/(name+'.lib');run('imports-'+name,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',library]);libraries.append(library)
        pad=out/'pad.S';pad.write_text('.text\n.globl _cursor_fixture_pad\n_cursor_fixture_pad:\n.space 0x400000\n');run('pad-build',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj'])
        run('provider-build',[*builder.FLAGS,'-MD','-MF',out/'provider.d','-c',ROOT/'tests/screen-cursor-provider-probe.c','-o',out/'provider.obj']);closure(out/'provider.d')
        run('provider-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/base:0x400000','/dynamicbase:no','/include:_cursor_fixture_pad','/out:'+str(out/'provider.exe'),out/'pad.obj',out/'provider.obj',*libraries])
        report['providers']={};payloads={}
        for name,extra in [('wine',dict(WINEDLLOVERRIDES='ddraw=b')),('native',dict(WINEDLLOVERRIDES='ddraw=',MNM_FACTORY_NATIVE='1')),('guarded',dict(WINEDLLOVERRIDES='ddraw=',MNM_FACTORY_NATIVE='1',MNM_NATIVE_CPU_GUARD='3'))]:
            target=out/name;target.mkdir();retirement=target/'retirement.bin';run(name,['wine',out/'provider.exe'],extra|dict(MNM_NATIVE_DRAW_RETIREMENT='Z:'+str(retirement).replace('/','\\')),cwd=target)
            payloads[name]=(target/'cursor-provider.bin').read_bytes();report['providers'][name]=compare_provider(payloads[name])|dict(sha256=hashlib.sha256(payloads[name]).hexdigest())
            if name!='wine':report['providers'][name]['retirement']=require_retired(retirement)
        if len(set(payloads.values()))!=1:raise ValueError('Actual provider outputs differ')
        refused=[]
        for name,offset in [('destination',16*4),('identity',0),('wait-result',8*4),('queue-count',11*4),('post-drain',133*4)]:
            bad=bytearray(payloads['native']);struct.pack_into('<I',bad,offset,struct.unpack_from('<I',bad,offset)[0]^1)
            try:compare_provider(bad)
            except ValueError:refused.append(name)
            else:raise ValueError('Corrupt provider trace accepted: '+name)
        try:compare_provider(payloads['native'][:-4])
        except ValueError:refused.append('truncation')
        else:raise ValueError('Truncated provider trace accepted')
        for name,bad in [('duplicated-row',payloads['native'][:1000]*78),('reordered-rows',payloads['native'][1000:2000]+payloads['native'][:1000]+payloads['native'][2000:])]:
            try:compare_provider(bad)
            except ValueError:refused.append(name)
            else:raise ValueError('Invalid provider denominator accepted: '+name)
        report['refusal_controls']=refused
        report['provider_version']=subprocess.check_output(['wine','--version'],env=env,text=True).strip();report['success']=True
    except (OSError,ValueError,subprocess.SubprocessError) as e:report['error']=str(e)
    finally:
        if lock is not None:
            try:run('prefix-stop',['wineserver','-k']);run('prefix-retired',['wineserver','-w']);report['private_wine_stopped']=True
            except (OSError,ValueError,subprocess.SubprocessError) as e:report.update(success=False,cleanup_error=str(e))
            lock.close()
        if verified:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except (OSError,ValueError,subprocess.SubprocessError) as e:report.update(success=False,manifest_error=str(e))
        report['sources_stable']=stable();report['success']&=report['sources_stable'];write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],error=report.get('error'))),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
