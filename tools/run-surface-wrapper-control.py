#!/usr/bin/env python3
"""Run one bounded GL control with a private authenticated TCP Xvfb display.

A private user namespace and disk-backed /tmp avoid host tmpfs quota failures.
Only the experiment X server sees namespace root. The temporary authorization cookie is never printed or retained.
"""
import argparse
import os
from pathlib import Path
import secrets
import socket
import struct
import subprocess
import sys
import tempfile
import time


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--log-root',type=Path,required=True)
    parser.add_argument('command',nargs=argparse.REMAINDER)
    args=parser.parse_args()
    command=args.command[1:] if args.command[:1]==['--'] else args.command
    if not command:parser.error('Require a control executable')
    args.log_root.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='display-',dir=args.log_root))
    env=dict(os.environ,QT_QPA_PLATFORM='xcb',QT_FORCE_STDERR_LOGGING='1',
             QT_ASSUME_STDERR_HAS_CONSOLE='1')
    # Check the TCP port rather than /tmp lock-file presence. Xvfb then owns
    # the selected socket; no other X server or shared authorization is changed.
    number=None
    for candidate in range(210,310):
        with socket.socket() as probe:
            try:probe.bind(('127.0.0.1',6000+candidate))
            except OSError:continue
        number=candidate;break
    if number is None:raise RuntimeError('No free private X display in bounded range')
    auth=run/'authorization'
    def field(value):return struct.pack('>H',len(value))+value
    # Xauthority FamilyWild record, with a fresh cookie scoped to this server.
    record=struct.pack('>H',65535)+field(b'')+field(str(number).encode())
    record+=field(b'MIT-MAGIC-COOKIE-1')+field(secrets.token_bytes(16))
    fd=os.open(auth,os.O_WRONLY|os.O_CREAT|os.O_EXCL,0o600)
    with os.fdopen(fd,'wb') as stream:stream.write(record)
    env.update(DISPLAY='localhost:'+str(number),XAUTHORITY=str(auth))
    temporary=run/'tmp';temporary.mkdir(mode=0o1777);temporary.chmod(0o1777)
    process=None
    try:
        with (run/'xvfb.log').open('xb') as log:
            process=subprocess.Popen(['bwrap','--die-with-parent','--unshare-user','--uid','0','--gid','0',
                '--ro-bind','/','/','--bind',str(temporary),'/tmp','--setenv','TMPDIR','/tmp',
                '--','Xvfb',':'+str(number),'-screen','0','640x480x24',
                '-nolock','-nolisten','unix','-listen','tcp','-auth',str(auth)],
                stdout=log,stderr=subprocess.STDOUT,env=env)
            deadline=time.monotonic()+30
            while time.monotonic()<deadline:
                if process.poll() is not None:raise RuntimeError('Private Xvfb failed: '+str(run/'xvfb.log'))
                try:
                    ready=subprocess.run(['xdpyinfo'],env=env,capture_output=True,timeout=2)
                    if ready.returncode==0:break
                except subprocess.TimeoutExpired:pass
                time.sleep(0.1)
            else:raise RuntimeError('Private Xvfb readiness timeout: '+str(run/'xvfb.log'))
            return subprocess.run(command,env=env,timeout=90).returncode
    finally:
        if process is not None and process.poll() is None:
            process.terminate()
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
        auth.unlink(missing_ok=True)


if __name__=='__main__':
    try:raise SystemExit(main())
    except (OSError,RuntimeError,subprocess.TimeoutExpired) as error:
        print(str(error),file=sys.stderr);raise SystemExit(2)
