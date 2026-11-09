"""Owned accessibility bus with an explicit registry, independent of user systemd."""
import select,subprocess,time
from pathlib import Path
from rendering_pilot import require

class AccessibilityBus:
 def __init__(self,out,environment):
  self.processes=[];self.logs=[];self.environment=dict(environment)
  try:
   buslog=(Path(out)/'accessibility-bus.log').open('xb');self.logs.append(buslog)
   bus=subprocess.Popen(['dbus-daemon','--session','--nofork','--print-address=1'],stdout=subprocess.PIPE,stderr=buslog,start_new_session=True);self.processes.append(bus)
   require(select.select([bus.stdout],[],[],5)[0],'Private accessibility bus address deadline')
   address=bus.stdout.readline().decode().strip();require(address.startswith('unix:')and bus.poll()is None,'Private accessibility bus did not start');bus.stdout.close()
   registrylog=(Path(out)/'accessibility-registry.log').open('xb');self.logs.append(registrylog)
   env=dict(environment,DBUS_SESSION_BUS_ADDRESS=address,AT_SPI_BUS_ADDRESS=address)
   registry=subprocess.Popen(['/usr/lib/at-spi2-registryd'],env=env,stdout=registrylog,stderr=subprocess.STDOUT,start_new_session=True);self.processes.append(registry)
   deadline=time.monotonic()+5
   while True:
    require(registry.poll()is None,'Private accessibility registry exited')
    result=subprocess.run(['gdbus','call','--address',address,'--dest','org.freedesktop.DBus','--object-path','/org/freedesktop/DBus','--method','org.freedesktop.DBus.NameHasOwner','org.a11y.atspi.Registry'],capture_output=True,text=True,timeout=2)
    if result.returncode==0 and result.stdout.strip()=='(true,)':break
    require(time.monotonic()<deadline,'Private registry name deadline');time.sleep(.05)
   self.environment['AT_SPI_BUS_ADDRESS']=address
   self.record=dict(policy='owned-explicit-atspi-registry-v1',bus_pid=bus.pid,registry_pid=registry.pid,registry_name_owned=True)
  except BaseException:self.close();raise
 def close(self):
  for process in reversed(self.processes):
   if process.poll()is None:
    process.terminate()
    try:process.wait(timeout=5)
    except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)
  for log in self.logs:log.close()
