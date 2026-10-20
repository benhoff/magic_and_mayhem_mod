#!/usr/bin/env python3
"""Execute public-entry and selected presentation resolver bypass controls."""
import argparse,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
from drawing_api_session import sha
from drawing_source_reservations import decode
from drawing_fixture_prefix import prepare
from drawing_x11_delivery import decode_delivery
ROOT=Path(__file__).resolve().parents[1]

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--shared-fixture-prefix',type=Path);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=claims['sources']
    def stable():
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    stable();parent=ROOT/'working/tests/drawing-ownership-bypasses';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Owned executable entry/resolver bypass controls. Preserve binaries and refusals.\n')
    report=dict(schema=1,success=False,sources=sources,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),original_game_executed=False,physical_backing_owned=False,global_presentation_routes_closed=False)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('LD_PRELOAD',None);lock=None
    try:
        dll=module('pe_build','tools/build-drawing-session.py').build(True)
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
        subprocess.run(flags+['-c',str(ROOT/'tests/drawing-mutator-entry-test.c'),'-o',str(run/'fixture.obj')],check=True)
        binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(dll.parent/'packet.obj'),str(dll.parent/'kernel32.lib')],check=True)
        prefix,lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
        wine_env=dict(env,WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW_SESSION_MODE='physical',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_RESOURCE_COPY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1',MNM_DRAW_WINE_SOURCE_MAP='2',MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1',MNM_DRAW_WINE_SOURCE_VERSION='1',MNM_DRAW_WINE_GPU_COMPLETION='1',MNM_DRAW_WINE_SOURCE_RESERVATIONS='1')
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-i' if a.shared_fixture_prefix else '-u'],env=wine_env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        pe_cases=[]
        for case in ['0','1','2','3','4','5','6','7','off']:
            target=run/('entry-'+case);target.mkdir();trial=dict(wine_env,MNM_TEST_ENTRY='0' if case=='off' else case,MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
            if case=='off':trial.pop('MNM_DRAW_WINE_SOURCE_RESERVATIONS')
            with (target/'wine.log').open('x') as out:r=subprocess.run(['wine',str(binary)],env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            negative=case in ('1','2','3');expected=99 if negative else 0
            if r.returncode!=expected:raise ValueError('Entry case '+case+' exit '+str(r.returncode))
            forwards=(target/'fixture-worker-forwards.bin').read_bytes();count=0 if negative or case in ('4','5','6','7') else 1
            if forwards!=struct.pack('<I',1)*count:raise ValueError('Entry original forward count '+case)
            analysis=None
            if case!='off':
                raw=(target/'wine-source-reservations.bin').read_bytes()
                if negative:
                    last=struct.unpack_from('<32I',raw,len(raw)-128)
                    if last[1]!=8 or last[15]!=1 or last[16]!={'1':1,'2':2,'3':4}[case]:raise ValueError('Wrong pre-forward writer refusal '+case)
                else:
                    analysis=decode(raw)
                    if analysis['reservations']!=count:raise ValueError('Entry reservation count '+case)
            elif (target/'wine-source-reservations.bin').exists():raise ValueError('Default-off policy changed')
            pe_cases.append(dict(case=case,exit=r.returncode,original_worker_forwards=count,analysis=analysis))
        builder=module('unix_build','tools/build-drawing-x11-delivery.py')
        capture=module('capture_policy','tools/capture-drawing-interactive.py');capability=[]
        for tag,kwargs in [('x11',{}),('glx',dict(glx=True)),('egl',dict(egl=True))]:
            candidate=builder.build(**kwargs);data=candidate.read_bytes()
            try:receipt=capture.strict_route_library_manifest(candidate,data)
            except ValueError:
                if tag=='egl':raise
                capability.append(dict(case=tag,refused=True))
            else:
                if tag!='egl' or len(receipt)!=64:raise ValueError('Unsupported observer admitted strict policy')
                capability.append(dict(case=tag,refused=False,manifest_sha256=receipt))
        library=candidate;original_manifest=(library.parent/'manifest.json').read_bytes()
        # Wrong binary hash, stale policy source and missing receipt all refuse.
        for tag in ('binary','source','missing'):
            target=run/('capability-'+tag);target.mkdir();candidate=target/'observer.so';candidate.write_bytes(library.read_bytes())
            manifest=json.loads(original_manifest)
            if tag=='binary':manifest['binary_sha256']='0'*64
            if tag=='source':manifest['sources']['runtime/display/presentation_route_guard.h']='0'*64
            if tag!='missing':(target/'manifest.json').write_text(json.dumps(manifest))
            try:capture.strict_route_library_manifest(candidate,candidate.read_bytes())
            except (ValueError,OSError):capability.append(dict(case=tag,refused=True))
            else:raise ValueError('Unpinned observer admitted strict policy '+tag)
        host=run/'route-fixture';subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-presentation-route-test.c'),'-lEGL','-lGL','-ldl','-o',str(host)],check=True)
        for tag,identity in [('b',2),('a',1)]:
            dependency=['-L'+str(run),'-Wl,--no-as-needed','-l:provider-b.so','-Wl,-rpath,$ORIGIN'] if tag=='a' else []
            subprocess.run(['gcc','-std=c11','-O2','-fPIC','-shared','-Wall','-Wextra','-Werror','-DMNM_PROVIDER_ID='+str(identity),str(ROOT/'tests/drawing-route-next-provider.c'),*dependency,'-ldl','-o',str(run/('provider-'+tag+'.so'))],check=True)
        routes=[]
        def route(route,name,strict,expected,refusal=None):
            trial=dict(env,LD_PRELOAD=str(library))
            if strict is not None:trial['MNM_DRAW_STRICT_ROUTES']=strict
            r=subprocess.run([str(host),route,name],env=trial,capture_output=True,timeout=15)
            if r.returncode!=expected or refusal and refusal not in r.stderr:raise ValueError('Route case '+route+'/'+name+' '+str(r.returncode)+' '+repr(r.stderr))
            routes.append(dict(route=route,name=name,strict=strict,exit=r.returncode,stdout=r.stdout.decode(),stderr=r.stderr.decode()))
        for strict in (None,'1'):route('ordinary',str(run),strict,0)
        selected=['glXSwapBuffers','glXGetProcAddress','glXGetProcAddressARB','eglSwapBuffers','eglGetProcAddress','glBlitFramebuffer','glBlitFramebufferEXT']
        for name in selected:route('next',name,'1',97,b'Strict routes reject selected RTLD_NEXT lookup')
        alternates=['eglSwapBuffersWithDamageKHR','eglSwapBuffersWithDamageEXT','eglPostSubBufferNV','eglSwapBuffersRegionNOK','eglSwapBuffersRegion2NOK','glXCopySubBufferMESA','glXSwapBuffersMscOML']
        for lookup in ('next','default','handle','egl','glx','arb'):
            for name in alternates:route(lookup,name,'1',97,b'Strict routes reject')
        for invalid in ('','0','2','true'):route('next','eglSwapBuffers',invalid,97,b'Invalid MNM_DRAW_STRICT_ROUTES')
        # Real Mesa contexts and coloured backbuffers: compare baseline, default
        # observer, and strict observer for all existing selected dispatch paths.
        graphics=[]
        for name,libs,count in [('egl',['-lEGL','-lGLESv2'],4),('glx',['-lGL'],5)]:
            fixture=run/(name+'-fixture');subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/('tests/drawing-'+name+'-delivery-test.c')),*libs,'-lX11','-ldl','-o',str(fixture)],check=True)
            baseline=subprocess.run([str(fixture)],env=dict(env,LIBGL_ALWAYS_SOFTWARE='1'),capture_output=True,timeout=30)
            if baseline.returncode:raise ValueError('Graphics baseline '+name+' '+str(baseline.returncode))
            for strict in (None,'1'):
                target=run/(name+'-'+('strict' if strict else 'default'));target.mkdir();trial=dict(env,LIBGL_ALWAYS_SOFTWARE='1',LD_PRELOAD=str(library),MNM_DRAW_X11_DIRECTORY=str(target))
                if strict:trial['MNM_DRAW_STRICT_ROUTES']=strict
                r=subprocess.run([str(fixture)],env=trial,capture_output=True,timeout=30);(target/'fixture.log').write_bytes(r.stdout+r.stderr)
                if r.returncode or r.stdout!=baseline.stdout:raise ValueError('Graphics forwarding '+name+' '+str(strict)+' '+str(r.returncode)+' '+repr(r.stderr))
                decoded=decode_delivery((target/'x11-delivery.bin').read_bytes())
                if decoded['methods'].get('eglSwapBuffers' if name=='egl' else 'glXSwapBuffers')!=count:raise ValueError('Selected dispatch denominator')
                graphics.append(dict(api=name,strict=strict,swaps=count,stdout_sha256=sha(r.stdout)))
        stable();report.update(success=True,capability_cases=capability,pe_cases=pe_cases,resolver_cases=routes,graphics_cases=graphics,binary_sha256=sha(binary.read_bytes()),library_sha256=sha(library.read_bytes()),scope='All four pinned public mutator entry trampolines under owned PE32 bodies, pre-forward reservation refusal, all-site mismatch preflight, post-close/default-off forwarding; selected RTLD_NEXT and seven alternate presentation resolver refusals, ordinary caller-relative lookup preserved, real software EGL/GLX selected routes unchanged. Raw CPU stores, private Wine writes, distinct-resource physical aliases, direct alternate calls/other contexts and global presentation remain unproved.')
    except Exception as error:report['error']=repr(error)
    finally:
        if 'wine_env' in locals():subprocess.run(['wineserver','-k'],env=wine_env,check=False,timeout=10)
        if lock is not None:lock.close()
    report['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable'];a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report.get(k) for k in ('success','sources_stable','error')}));return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
