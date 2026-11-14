"""Owned NVIDIA rendering displays with no physical output or VT switching."""
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import struct
import time

from native_render_driver import provision_owned_monitor, require_nvidia

ROOT = Path(__file__).resolve().parents[1]
_LOCKS = []



def authority_bytes(cookie):
    if len(cookie) != 16:
        raise ValueError('Private X11 cookie must be 16 bytes')
    fields = (b'', b'', b'MIT-MAGIC-COOKIE-1', cookie)
    return struct.pack('>H', 65535)+b''.join(struct.pack('>H', len(v))+v for v in fields)


def gpu_environment(source):
    env = dict(source)
    for name in ('LIBGL_ALWAYS_SOFTWARE', '__GLX_VENDOR_LIBRARY_NAME',
                 '__EGL_VENDOR_LIBRARY_FILENAMES', 'WAYLAND_DISPLAY', 'QT_XCB_GL_INTEGRATION'):
        env.pop(name, None)
    env.update(MESA_LOADER_DRIVER_OVERRIDE='zink', GALLIUM_DRIVER='zink')
    return env


def start_nvidia(directory):
    directory = Path(directory)
    bus_id = subprocess.check_output(['nvidia-smi', '--query-gpu=pci.bus_id',
        '--format=csv,noheader'], text=True).strip().splitlines()[0]
    match = re.fullmatch(r'[0-9A-Fa-f]{8}:([0-9A-Fa-f]{2}):([0-9A-Fa-f]{2})\.([0-7])', bus_id)
    if not match:
        raise ValueError('Unknown GPU bus identity')
    bus = 'PCI:'+':'.join(str(int(v, 16)) for v in match.groups())
    leases = ROOT/'working/broader-player/gpu-display-leases'
    leases.mkdir(exist_ok=True)
    selected = None
    for number in range(170, 190):
        lock = (leases/(str(number)+'.lock')).open('a+b')
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            lock.close()
            continue
        if Path('/tmp/.X11-unix/X'+str(number)).exists() or Path('/tmp/.X'+str(number)+'-lock').exists():
            lock.close()
            continue
        selected = number
        _LOCKS.append(lock)
        break
    if selected is None:
        raise ValueError('No private hardware display available')
    cookie = os.urandom(16)
    authority = directory/'Xauthority'
    with authority.open('xb') as f:
        os.chmod(authority, 0o600)
        f.write(authority_bytes(cookie))
    conf = directory/'private-gpu.conf'
    conf.write_text('Section "ServerFlags"\n Option "AutoAddDevices" "False"\n Option "AutoEnableDevices" "False"\n Option "DontVTSwitch" "True"\nEndSection\nSection "Device"\n Identifier "broader-owned-gpu"\n Driver "nvidia"\n BusID "'+bus+'"\n Option "UseDisplayDevice" "none"\n Option "AllowEmptyInitialConfiguration" "True"\n Option "ProbeAllGpus" "False"\nEndSection\nSection "Screen"\n Identifier "broader-owned-screen"\n Device "broader-owned-gpu"\n DefaultDepth 24\n SubSection "Display"\n  Depth 24\n  Virtual 1280 1024\n EndSubSection\nEndSection\nSection "ServerLayout"\n Identifier "broader-owned-layout"\n Screen "broader-owned-screen"\nEndSection\n')
    command = ['/usr/lib/Xorg', ':'+str(selected), '-seat', 'broader-player-test',
        '-config', str(conf), '-auth', str(authority), '-logfile', str(directory/'private-gpu-xorg.log'),
        '-nolisten', 'tcp', '-noreset', '-novtswitch', '-sharevts']
    with (directory/'private-gpu-start.log').open('xb') as log:
        server = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
    try:
        socket = Path('/tmp/.X11-unix/X'+str(selected))
        deadline = time.monotonic()+30
        while not socket.exists():
            if server.poll() is not None or time.monotonic() > deadline:
                raise ValueError('Owned NVIDIA server failed initialization')
            time.sleep(.1)
        env = gpu_environment(os.environ)
        env.update(DISPLAY=':'+str(selected), XAUTHORITY=str(authority))
        monitor = provision_owned_monitor(directory, env)
        run = subprocess.run(['glxinfo', '-B'], env=env, capture_output=True, timeout=20)
        (directory/'hardware-driver.log').write_bytes(run.stdout+run.stderr)
        if run.returncode:
            raise ValueError('Owned NVIDIA GLX probe failed')
        drivers = [line.split(':', 1)[1].strip() for line in run.stdout.decode().splitlines()
                   if line.startswith('OpenGL renderer string:')]
        require_nvidia(drivers)
        st = socket.stat()
        receipt = dict(schema=1, policy='broader-owned-nvidia-x11-v1', owner_pid=os.getpid(),
            server_pid=server.pid, display=':'+str(selected), socket_identity=[st.st_dev, st.st_ino],
            authority=str(authority), authority_sha256=hashlib.sha256(authority.read_bytes()).hexdigest(),
            physical_output_connected=False, physical_device_reset=False, gpu_bus=bus_id,
            monitor=monitor, observed_glx_drivers=drivers, command=command)
        (directory/'owner.json').write_text(json.dumps(receipt, indent=2)+'\n')
        return server, receipt, cookie
    except BaseException:
        if server.poll() is None:
            os.killpg(server.pid, 15)
        server.wait(timeout=10)
        raise
