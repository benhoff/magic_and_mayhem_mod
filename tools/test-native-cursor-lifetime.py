#!/usr/bin/env python3
"""Execute actual PE32 provider pins, alias drains and writer/concurrency controls."""
import argparse,hashlib,importlib.util,json,os,secrets,socket,struct,subprocess,sys,time
from pathlib import Path
from coverage_evidence import write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def execute(out,template,ram,owner=False,original_root=None):
    authority=out/'Xauthority';fields=(b'',b'410',b'MIT-MAGIC-COOKIE-1',secrets.token_bytes(16))
    for number in range(410,490):
        with socket.socket() as s:
            try:s.bind(('127.0.0.1',6000+number))
            except OSError:continue
            break
    else:raise ValueError('No private display')
    fields=(fields[0],str(number).encode(),fields[2],fields[3])
    authority.write_bytes(struct.pack('>H',65535)+b''.join(struct.pack('>H',len(v))+v for v in fields));authority.chmod(0o600)
    prefix=ram/'wineprefix';subprocess.run(['cp','-a','--reflink=auto',template,prefix],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(DISPLAY='127.0.0.1:'+str(number),XAUTHORITY=str(authority),WINEPREFIX=str(prefix),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=;winemenubuilder.exe=d',MNM_NATIVE_CURSOR_OWNERSHIP='1',LIBGL_ALWAYS_SOFTWARE='1',TMPDIR='/tmp')
    if owner:env.update(MNM_NATIVE_CURSOR_OWNERSHIP='2',MNM_FACTORY_NATIVE='1',MNM_ORIGINAL_IMAGE='Z:'+str(original_root/'original/Arcane_Nocd/Chaos.exe').replace('/','\\'))
    log=(out/'xserver.log').open('xb');server=subprocess.Popen(['Xvfb',':'+str(number),'-screen','0','1024x768x24','-nolock','-nolisten','unix','-listen','tcp','-auth',str(authority)],stdout=log,stderr=subprocess.STDOUT)
    def run(name,args,cwd,extra=None,expected=0):
        with (out/(name+'.log')).open('xb') as f:
            p=subprocess.run(list(map(str,args)),cwd=cwd,env=env|dict(extra or {}),stdout=f,stderr=subprocess.STDOUT,timeout=120)
        if p.returncode!=expected:raise ValueError(name+' exited '+str(p.returncode)+' expected '+str(expected))
    try:
        for i in range(100):
            if server.poll() is not None:raise ValueError('Xvfb exited')
            try:
                with socket.create_connection(('127.0.0.1',6000+number),timeout=.1):break
            except OSError:time.sleep(.05)
        run('wineboot',['wineboot','-u'],out)
        for name,extra in [('plain',{}),('protected',{'MNM_NATIVE_CPU_GUARD':'3'})]+([('early-plain',{'MNM_CURSOR_EARLY_OUTER':'1'}),('early-protected',{'MNM_CURSOR_EARLY_OUTER':'1','MNM_NATIVE_CPU_GUARD':'3'})] if owner else []):
            dest=out/name;dest.mkdir()
            if owner:extra=extra|dict(MNM_NATIVE_CURSOR_LOG='Z:'+str(dest/'owner-journal.bin').replace('/','\\'))
            run(name,['wine',out/'probe.exe'],dest,extra)
        if owner:
            for index in range(18):
                dest=out/('bad-anchor-'+str(index));dest.mkdir();run('bad-anchor-'+str(index),['wine',out/'probe.exe'],dest,dict(MNM_CURSOR_BAD_ANCHOR=str(index)),123)
    finally:
        subprocess.run(['wineserver','-k'],env=env,timeout=15,check=False);subprocess.run(['wineserver','-w'],env=env,timeout=15,check=False)
        server.terminate()
        try:server.wait(timeout=10)
        except subprocess.TimeoutExpired:server.kill();server.wait()
        log.close()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report');p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--owner-probe',action='store_true');p.add_argument('--original-root',type=Path,default=Path('/home/hoff/swdev/magic_and_mayhem_mod'));p.add_argument('--execute',action='store_true');p.add_argument('--ram',type=Path)
    a=p.parse_args();out=a.output.resolve();template=a.prefix_template.resolve()
    if a.execute:execute(out,template,a.ram.resolve(),a.owner_probe,a.original_root.resolve());return 0
    if not a.report or out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/a.report).exists():p.error('Use new owned output and report paths')
    out.mkdir(parents=True);(out/'.retain').write_text('PE32 native cursor lifetime binaries, inputs, outputs and negative evidence.\n')
    files=[f for sub in ['runtime/native-draw','runtime/render','runtime/scene','protocols/include','protocols/schemas'] for f in (ROOT/sub).rglob('*') if f.is_file() and f.suffix in ('.c','.h','.hpp','.inc','.json')]
    files += [ROOT/n for n in ['tests/native-cursor-lifetime-probe.c','tests/native-cursor-owner-probe.c','tests/graphics-lifetime-common.h','tests/original-pe32-image.h','tools/test-native-cursor-lifetime.py','tools/build-native-draw.py','tools/build-render-bridge.py','tools/build-shadow-bridge.py','tools/build-scene-observer.py','tools/test-native-cursor-live.py','tools/native_cursor_journal.py','tools/native_cursor_launch.py','tools/run-opengl-game.py','tools/prepare-menu-observer.py','tools/menu-game-runner.py','tools/render-game-runner.py','tools/coverage_evidence.py','tools/coverage_claims.py','runtime/shadow/win32_min.h','reconstruction/rendering/pixel_kernel.c','reconstruction/rendering/pixel_kernel.h','protocols/generate.py']]
    sources={f.relative_to(ROOT).as_posix():sha(f) for f in sorted(set(files))}
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']==('NP.cursor-live-ownership' if a.owner_probe else 'NP.cursor-provider-lifetime'))
    sources.update({n:sha(ROOT/n) for n in sorted(required_sources(b))})
    assert required_sources(b)<=sources.keys()
    report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={s['id']:scenario_contract(s) for s in r['scenarios'] if b['id'] in s['behaviors'] and s['level']=='headless'})],original_game_executed=False,live_replacement=False,commands=[],scope=b['scope'])
    registries={};original_verified=False
    def run(name,args):
        with (out/(name+'.log')).open('xb') as f:result=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=180)
        report['commands'].append(dict(name=name,argv=list(map(str,args)),exit=result.returncode))
        if result.returncode:raise ValueError(name+' failed: '+(out/(name+'.log')).read_text(errors='replace')[-3000:])
    try:
        if a.owner_probe:
            subprocess.run([a.original_root/'tools/original-manifest.sh','verify'],check=True,stdout=subprocess.DEVNULL);original_verified=True
            image=a.original_root/'original/Arcane_Nocd/Chaos.exe'
            assert sha(image)=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
            report['original_sha256']=sha(image)
        registries={n:sha(template/n) for n in ['system.reg','user.reg','userdef.reg']}
        # Only the named stopped template is consumed; no other Wine prefix is changed.
        for process in Path('/proc').iterdir():
            if not process.name.isdigit():continue
            try:env=(process/'environ').read_bytes().split(b'\0')
            except (OSError,PermissionError):continue
            if ('WINEPREFIX='+str(template)).encode() in env:raise ValueError('Template has an active consumer')
        spec=importlib.util.spec_from_file_location('native_cursor_builder',ROOT/'tools/build-native-draw.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
        with (out/'provider-build.log').open('xb') as log:
            dll=m.build(out/'provider-build',sources)
        import shutil
        shutil.copyfile(dll,out/'MnmDraw.dll')
        imports={'LoadLibraryA':4,'GetProcAddress':8,'GetModuleHandleA':4,'ExitProcess':4,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'CreateThread':24,'CreateEventA':16,'SetEvent':4,'WaitForSingleObject':8,'GetCurrentThreadId':0,'GetProcessHeap':0,'HeapAlloc':12,'HeapFree':12,'VirtualAlloc':16,'VirtualProtect':16,'VirtualQuery':12,'ReadFile':20,'GetFileSize':8,'GetEnvironmentVariableA':12}
        d=out/'kernel32.def';d.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',d,'-l',out/'kernel32.lib'])
        d=out/'gdi32.def';d.write_text('LIBRARY GDI32.dll\nEXPORTS\nSetPixel@16\n');run('gdi-imports',['llvm-dlltool','-m','i386','--kill-at','-d',d,'-l',out/'gdi32.lib'])
        run('compile',[*m.FLAGS,'-MD','-MF',out/'probe.d','-c',ROOT/('tests/native-cursor-owner-probe.c' if a.owner_probe else 'tests/native-cursor-lifetime-probe.c'),'-o',out/'probe.obj'])
        objects=[out/'probe.obj']
        if a.owner_probe:
            pad=out/'pad.S';pad.write_text('.text\n.globl _cursor_fixture_pad\n_cursor_fixture_pad:\n.space 0x400000\n')
            run('pad',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj']);objects=[out/'pad.obj',out/'probe.obj']
        run('link',['lld-link','/machine:x86','/entry:start','/base:'+('0x400000' if a.owner_probe else '0x18000000'),'/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),*objects,out/'kernel32.lib',out/'gdi32.lib',*(['/dynamicbase:no','/include:_cursor_fixture_pad'] if a.owner_probe else [])])
        ram=out/'ram';ram.mkdir()
        run('execute',['unshare','--user','--map-root-user','--mount','bash','-ec','mount -t tmpfs -o size=4G,mode=0700 tmpfs "$1"; mount -t tmpfs -o size=128M,mode=1777 tmpfs /tmp; shift; exec "$@"','cursor-lifetime',ram,sys.executable,ROOT/'tools/test-native-cursor-lifetime.py','--execute','--output',out,'--prefix-template',template,'--ram',ram,*(['--owner-probe','--original-root',a.original_root] if a.owner_probe else [])])
        report['profiles']={}
        for name in ['plain','protected']+(['early-plain','early-protected'] if a.owner_probe else []):
            if a.owner_probe:
                raw=(out/name/'cursor-owner.bin').read_bytes();values=struct.unpack('<32I',raw)
                if values[0]!=1 or values[1]<1800000 or values[2:4]!=(6,18):raise ValueError('Incomplete owning original-client execution: '+str(values[:4]))
                state=values[4:]
                if not(state[1] and state[2]) or any(state[k] for k in [5,6,7,8,9,10]) or state[12]!=state[13]+state[14]:raise ValueError('Owning native queue did not retire completely')
                journal=(out/name/'owner-journal.bin').read_bytes();header=struct.unpack('<16I',journal[:64]);rows=[struct.unpack('<16I',journal[i:i+64]) for i in range(64,len(journal),64)]
                if len(journal)%64 or header[:7]!=(0x434d4e4d,0x31574f52,1,64,0x40209ca7,18,32768) or not rows or rows[-1][1]!=10 or any(rows[-1][i] for i in [3,4]):raise ValueError('Missing complete final ownership journal')
                slots={row[3] for row in rows if row[1]==7}
                if not {1,2,3,4,5,6,7}<=slots:raise ValueError('Missing original foreground/background/timer/selection/disposal executions')
                report['profiles'][name]=dict(success=True,assertions=values[1],complete_planes=values[2],installed_hooks=18,queue_accepted=state[12],queue_completed=state[13],queue_cancelled=state[14],queue_full=state[15],busy_retained=state[17],late_clients_refused=state[19],final_status=list(state),journal_rows=len(rows),journal_sha256=sha(out/name/'owner-journal.bin'),remaining_surfaces=0,remaining_devices=0,remaining_dcs=0)
            else:
                values=struct.unpack('<8I',(out/name/'provider-lifetime.bin').read_bytes())
                if values[0]!=1 or values[1]<190000 or values[2:]!=(1616,26,1600,16,8,512):raise ValueError('Incomplete execution denominator: '+str(values))
                report['profiles'][name]=dict(success=True,assertions=values[1],complete_planes=values[2],cases=values[3],concurrent_copies=values[4],alias_pairs=values[5],writer_cases=values[6],pin_capacity=values[7],remaining_surfaces=0,remaining_devices=0,remaining_dcs=0)
        if a.owner_probe:report.update(changed_byte_anchor_refusals=18,original_functions_executed=True,production_installer_executed=True)
        report['success']=True
    except (ValueError,OSError,subprocess.SubprocessError) as exc:report['error']=str(exc)
    finally:
        if original_verified:
            try:subprocess.run([a.original_root/'tools/original-manifest.sh','verify'],check=True,stdout=subprocess.DEVNULL);report['original_manifest_before_after']=True
            except (OSError,subprocess.SubprocessError) as exc:report.update(success=False,manifest_error=str(exc))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['template_registries_unchanged']=all(sha(template/n)==h for n,h in registries.items());report['success'] &= report['sources_stable'] and report['template_registries_unchanged']
        report['logs']={str(f.relative_to(ROOT)):dict(sha256=sha(f),bytes=f.stat().st_size) for f in out.rglob('*.log')}
        report['binaries']={str(f.relative_to(ROOT)):sha(f) for f in out.glob('*') if f.suffix in ('.exe','.dll')}
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],error=report.get('error'),profiles=report.get('profiles'))),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
