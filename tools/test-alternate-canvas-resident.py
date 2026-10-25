#!/usr/bin/env python3
"""Validate native resident alternate ownership separately from original provenance."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import secrets
import shlex
import socket
import struct
import subprocess
import time
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import write_report

ROOT=Path(__file__).resolve().parents[1]
BID='NP.alternate-canvas-resident-session'
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True)
 p.add_argument('--live',action='store_true');p.add_argument('--source-game',type=Path);p.add_argument('--prefix-template',type=Path)
 p.add_argument('--difficulty',type=int,choices=(0,1),default=1,help='Existing campaign configuration: Apprentice default; Initiate tutorial remains a separate input boundary')
 a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require a new owned output')
 if a.live and (not a.source_game or not a.prefix_template):p.error('Live execution requires source-game and prefix-template')
 out.mkdir(parents=True);(out/'.retain').write_text('Alternate resident native controls/live results,inputs,failures and logs.\n')
 j=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in j['behaviors'] if b['id']==BID)
 sid='ACO3.resident-live' if a.live else 'ACO3.resident-controls';s=next(s for s in j['scenarios'] if s['id']==sid)
 paths=required_sources(b)|set(s['tests'])
 paths.update(('tests/resource-fixtures.hpp','tests/native-prepared-world-test.cpp','tests/native-retained-world-test.cpp','tests/resident-canvas-transition-test.cpp'))
 for directory in ('apps','renderer','compat','assets','audio','runtime','protocols','reconstruction'):
  paths.update(v.relative_to(ROOT).as_posix() for v in (ROOT/directory).rglob('*') if v.is_file() and (v.suffix in ('.cpp','.hpp','.c','.h','.S','.inc','.py','.json') or v.name=='CMakeLists.txt'))
 sources={n:sha(ROOT/n) for n in sorted(paths)}
 claim=dict(behavior=BID,contract_sha256=behavior_contract(b,{}),scenarios={sid:scenario_contract(s)})
 report=dict(schema=1,success=False,sources=sources,claims=[claim],commands=[],live_game_executed=False,original_ownership_recovered=False,original_pointer_publication=False)
 env=dict(os.environ,TMPDIR=str(out/'tmp'),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',__GLX_VENDOR_LIBRARY_NAME='mesa');(out/'tmp').mkdir()
 env.pop('MESA_LOADER_DRIVER_OVERRIDE',None)
 def run(name,cmd,timeout=180,allow_failure=False):
  log=out/(name+'.log')
  with log.open('x') as f:r=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=timeout)
  report['commands'].append(dict(name=name,argv=list(map(str,cmd)),exit=r.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)))
  if r.returncode and not allow_failure:raise RuntimeError(name+': '+log.read_text(errors='replace')[-1800:])
  return log.read_text()
 server=None
 try:
  if a.live:
   # The physical campaign runner is the explicit consumer of this bounded
   # interchange declaration and its own working report. Durable result here
   # is written directly into SQLite, including failed attempts.
   declaration=out/'claims.json';declaration.write_text(json.dumps(dict(claims=[claim],sources=sources)))
   # Private /tmp avoids host X11-lock quotas and confines all helper sockets.
   command=['bwrap','--ro-bind','/','/','--bind',str(ROOT),str(ROOT),'--bind',str((ROOT/'working/build/qt-shell').resolve()),str((ROOT/'working/build/qt-shell').resolve()),'--dev','/dev','--proc','/proc','--tmpfs','/tmp','--setenv','TMPDIR','/tmp','--','python3',ROOT/'tools/test-native-campaign.py','--resident-world','--independent-world','--alternate-canvas','--normal-quit','--stress-seconds','20','--skip-movies','--no-native-music','--fps-policy','diagnostic','--timeout','240','--claims',declaration,'--source-game',a.source_game.resolve(),'--prefix-template',a.prefix_template.resolve()]
   command.extend(['--difficulty',str(a.difficulty)])
   text=run('campaign',command,600,allow_failure=True)
   name=next(line[8:] for line in text.splitlines() if line.startswith('Report: '));live=json.loads(Path(name).read_text());report['campaign']=live;report['campaign_report']=name;report['live_game_executed']=bool(live.get('game_entry_verified'))
   if not live['success'] or not live.get('normal_quit_verified') or not live.get('sources_unchanged'):raise RuntimeError('Native campaign did not complete its existing functional contract: '+str(live.get('error'))[:600])
   alt=live['resident_world']['native_alternate_canvas'];count=int(live['resident_world']['completed'])
   if not alt['enabled'] or not count or int(alt['frames'])!=count or int(alt['copy_backs'])<count:raise RuntimeError('Missing live alternate dispatch/copy-back')
  else:
   build=ROOT/'working/build/qt-shell';names=['alternate-canvas-resident-test','resident-canvas-transition-test','native-prepared-world-test','native-retained-world-test']
   run('configure',['cmake','-S',ROOT/'apps/qt-shell','-B',build,'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON'])
   run('build',['cmake','--build',build,'--target',*names,'-j4'],600)
   for display in range(300,390):
    with socket.socket() as probe:
     try:probe.bind(('127.0.0.1',6000+display))
     except OSError:continue
     break
   else:raise RuntimeError('No private display')
   authority=out/'Xauthority';fields=(b'',str(display).encode(),b'MIT-MAGIC-COOKIE-1',secrets.token_bytes(16));authority.write_bytes(struct.pack('>H',65535)+b''.join(struct.pack('>H',len(v))+v for v in fields));authority.chmod(0o600)
   env.update(DISPLAY='127.0.0.1:'+str(display),XAUTHORITY=str(authority));env.pop('WAYLAND_DISPLAY',None);env.pop('MNM_NATIVE_ALTERNATE_CANVAS',None)
   with (out/'xserver.log').open('x') as log:server=subprocess.Popen(['bwrap','--ro-bind','/','/','--dev','/dev','--proc','/proc','--tmpfs','/tmp','--','Xvfb',':'+str(display),'-screen','0','1024x768x24','-nolisten','unix','-listen','tcp','-auth',str(authority)],env=env,stdout=log,stderr=subprocess.STDOUT)
   for _ in range(100):
    if server.poll() is not None:raise RuntimeError('Private display exited')
    try:
     with socket.create_connection(('127.0.0.1',6000+display),timeout=.1):break
    except OSError:time.sleep(.05)
   else:raise RuntimeError('Private display readiness')
   report['controls']={}
   for name in names:
    text=run(name,[build/'world-rolling'/name]);report['controls'][name]=json.loads(next(line for line in text.splitlines() if line.startswith('{')))
   report['executables']={name:sha(build/'world-rolling'/name) for name in names}
   control=report['controls'][names[0]]
   if control['frames']!=18 or control['prepared_replays']!=48 or control['remaining_surfaces'] or control['frame_readbacks']:raise RuntimeError('Incomplete alternate lifetime controls')
  compiled=set()
  for dependency in (ROOT/'working/build/qt-shell').resolve().rglob('*.o.d'):
   for token in shlex.split(dependency.read_text().replace('\\\n',' ')):
    path=Path(token)
    if path.is_absolute():
     path=path.resolve()
     if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):compiled.add(path.relative_to(ROOT).as_posix())
  report['undeclared_compiler_inputs']=sorted(compiled-set(sources))
  if report['undeclared_compiler_inputs']:raise RuntimeError('Undeclared compiler inputs: '+repr(report['undeclared_compiler_inputs']))
  report['compiled_sources']={name:sources[name] for name in sorted(compiled)}
  report['success']=True
 except Exception as error:report['error']=str(error)
 finally:
  if server and server.poll() is None:server.terminate();server.wait(timeout=10)
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
  digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps(dict(success=report['success'],sha256=digest,error=report.get('error'),output=str(out))))
 return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
