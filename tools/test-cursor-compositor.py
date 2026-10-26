#!/usr/bin/env python3
"""Exercise owning OpenGL cursor generations, complete pixels and shutdown races."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import resource
import secrets
import socket
import struct
import time
import subprocess
from coverage_evidence import write_report
from coverage_claims import behavior_contract,scenario_contract
ROOT=Path(__file__).resolve().parents[1]


def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',required=True)
    parser.add_argument('--user-namespace-display',action='store_true',help='Use an unprivileged user/mount namespace with a private /tmp for Xvfb when the shared tmpfs quota is full')
    args=parser.parse_args()
    out=args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/args.report).exists():
        parser.error('Require new owned output and report paths')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Native cursor pixels, bounded concurrent teardown, exact executed binaries and logs.\n')
    files=[p for p in (ROOT/'renderer').rglob('*') if p.is_file() and p.suffix in ('.cpp','.hpp','.h','.inc')]
    files += [p for p in (ROOT/'protocols/include').rglob('*') if p.is_file() and p.suffix in ('.h','.hpp')]
    files += [ROOT/n for n in ('renderer/CMakeLists.txt','tests/cursor-compositor-test.cpp','tools/test-cursor-compositor.py','tools/coverage_evidence.py','tools/coverage_claims.py')]
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    sources={p.relative_to(ROOT).as_posix():digest(p) for p in sorted(set(files))}
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='NP.cursor-owned-compositor')
    scenario=next(s for s in register['scenarios'] if s['id']=='CUR26.owned-pixels-and-teardown')
    claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{b['id']:b for b in register['builds']}),scenarios={scenario['id']:scenario_contract(scenario)})]
    report=dict(schema=1,success=False,sources=sources,claims=claims,commands=[],
                original_game_executed=False,live_replacement=False,all_source_alias_ownership_proved=False,
                scope='Native private RGB565 OpenGL canvas/atlas/save-under generations; independent whole-frame scalar oracle, metadata-only concurrent producers, cancellation and owner-thread GPU teardown. No original code execution or legacy alias admission.')
    display_env={}
    server=None
    xlog=None
    def run(label,argv,extra=None):
        print(label,flush=True)
        path=out/(label+'.log')
        env=os.environ|dict(TMPDIR=str(ROOT/'working/tmp'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='2',ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')|display_env|dict(extra or {})
        with path.open('x') as f:
            result=subprocess.run(list(map(str,argv)),cwd=ROOT,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=240)
        report['commands'].append(dict(label=label,argv=list(map(str,argv)),exit=result.returncode,log=path.relative_to(ROOT).as_posix()))
        if result.returncode:raise ValueError(label+': '+path.read_text(errors='replace')[-4000:])
        return path.read_text()
    try:
        for display in range(310,400):
            with socket.socket() as probe:
                try:probe.bind(('127.0.0.1',6000+display))
                except OSError:continue
                break
        else:raise ValueError('No private display available')
        authority=out/'Xauthority'
        fields=(b'',str(display).encode(),b'MIT-MAGIC-COOKIE-1',secrets.token_bytes(16))
        authority.write_bytes(struct.pack('>H',65535)+b''.join(struct.pack('>H',len(v))+v for v in fields));authority.chmod(0o600)
        display_env.update(DISPLAY='127.0.0.1:'+str(display),XAUTHORITY=str(authority),QT_FORCE_STDERR_LOGGING='1')
        xlog=(out/'xserver.log').open('x')
        display_tmp=out/'display-tmp';display_tmp.mkdir()
        namespace=['unshare','--user','--map-root-user','--mount','bash','-ec','mount --bind "$1" /tmp; shift; exec "$@"','cursor-display',str(display_tmp)] if args.user_namespace_display else []
        server_command=namespace+['Xvfb',':'+str(display),'-screen','0','1024x768x24','-nolisten','unix','-listen','tcp','-auth',str(authority)]+(['-nolock'] if args.user_namespace_display else [])
        report['display_command']=server_command
        server=subprocess.Popen(server_command,stdout=xlog,stderr=subprocess.STDOUT)
        for attempt in range(100):
            if server.poll() is not None:raise ValueError('Private display exited')
            try:
                with socket.create_connection(('127.0.0.1',6000+display),timeout=.1):break
            except OSError:time.sleep(.05)
        else:raise ValueError('Private display readiness timeout')
        for name,flags in [('optimized',''),('sanitized','-O1 -fsanitize=address,undefined -fno-omit-frame-pointer')]:
            build=out/name
            run(name+'-configure',['cmake','-S',ROOT/'renderer','-B',build,'-DCMAKE_BUILD_TYPE='+('Debug' if name=='sanitized' else 'RelWithDebInfo'),'-DCMAKE_CXX_FLAGS='+flags])
            run(name+'-build',['cmake','--build',build,'--target','cursor-compositor-test','-j2'])
            report[name]=json.loads(run(name,[build/'cursor-compositor-test']))
            if not report[name]['success'] or report[name]['complete_pixel_cases']!=187 or report[name]['producer_attempts']!=2400 or report[name]['teardown_races']!=8 or report[name]['accepted']!=report[name]['drained']:
                raise ValueError('Incomplete cursor execution denominator: '+str(report[name]))
            # Compile the complete native renderer, and check all repository
            # headers in dependency files against the prospective declaration.
            for dep in build.rglob('*.o.d'):
                import shlex
                for name in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                    p=Path(name).resolve()
                    if p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working') and p.relative_to(ROOT).as_posix() not in sources:
                        raise ValueError('Undeclared compiled dependency '+str(p))
        report['success']=True
    except (ValueError,OSError,subprocess.SubprocessError) as exc:
        report['error']=str(exc)
    finally:
        if server is not None:
            server.terminate()
            try:server.wait(timeout=10)
            except subprocess.TimeoutExpired:server.kill();server.wait(timeout=10)
            report['private_display_retired']=True
        if xlog is not None:xlog.close()
        report['sources_stable']=all(digest(ROOT/n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        report['logs']={p.relative_to(ROOT).as_posix():dict(bytes=p.stat().st_size,sha256=digest(p)) for p in out.glob('*.log')}
        write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],error=report.get('error'),optimized=report.get('optimized'),sanitized=report.get('sanitized'))),flush=True)
    return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
