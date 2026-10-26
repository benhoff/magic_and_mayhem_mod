#!/usr/bin/env python3
"""Run one bounded terrain experiment on an authenticated private TCP Xvfb."""
import argparse,os,secrets,signal,socket,sqlite3,struct,subprocess,sys,time
from pathlib import Path

def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);p.add_argument('--inside-namespace',action='store_true',help=argparse.SUPPRESS);p.add_argument('command',nargs=argparse.REMAINDER);a=p.parse_args();cmd=a.command
 if cmd and cmd[0]=='--':cmd=cmd[1:]
 if not cmd:p.error('Require command')
 out=a.output.resolve()
 if not a.inside_namespace:
  out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Owned display diagnostics for terrain lighting evidence.\n')
  (out/'tmp').mkdir(mode=0o1777);(out/'tmp').chmod(0o1777)
  return subprocess.run(['unshare','--user','--map-root-user','--mount',sys.executable,str(Path(__file__).resolve()),'--inside-namespace','--output',str(out),'--',*cmd]).returncode
 # This mount is visible only to this task's descendants. Host /tmp and its
 # running displays/prefixes remain outside the experiment's namespace.
 subprocess.run(['mount','--make-rprivate','/'],check=True)
 subprocess.run(['mount','--bind',str(out/'tmp'),'/tmp'],check=True)
 display=None
 for number in range(450,550):
  s=socket.socket()
  try:s.bind(('127.0.0.1',6000+number));display=number;break
  except OSError:pass
  finally:s.close()
 if display is None:raise RuntimeError('No private display port available')
 cookie=secrets.token_bytes(16);fields=[b'',str(display).encode(),b'MIT-MAGIC-COOKIE-1',cookie];authority=struct.pack('>H',65535)+b''.join(struct.pack('>H',len(v))+v for v in fields);auth=out/'Xauthority';auth.write_bytes(authority);auth.chmod(0o600)
 env={**os.environ,'DISPLAY':f'localhost:{display}','XAUTHORITY':str(auth),'LIBGL_ALWAYS_SOFTWARE':'1','GALLIUM_DRIVER':'llvmpipe'}
 log=(out/'Xvfb.log').open('x');server=subprocess.Popen(['Xvfb',f':{display}','-screen','0','1280x1024x24','-nolisten','unix','-listen','tcp','-auth',str(auth),'-extension','MIT-SHM'],stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
 code=1
 try:
  deadline=time.monotonic()+15
  while time.monotonic()<deadline:
   if server.poll() is not None:raise RuntimeError('Private Xvfb exited; inspect '+str(out/'Xvfb.log'))
   try:
    s=socket.create_connection(('127.0.0.1',6000+display),timeout=.1);s.close();break
   except OSError:time.sleep(.1)
  else:raise RuntimeError('Private display startup timed out')
  code=subprocess.run(cmd,env=env).returncode
 finally:
  if server.poll() is None:os.killpg(server.pid,signal.SIGTERM);server.wait(timeout=10)
  log.close();db=sqlite3.connect(out/'display.sqlite3');db.execute('CREATE TABLE result(display INTEGER,command TEXT,exit_code INTEGER,server_exit INTEGER)');db.execute('INSERT INTO result VALUES(?,?,?,?)',(display,repr(cmd),code,server.returncode));db.commit();db.close()
 return code
if __name__=='__main__':raise SystemExit(main())
