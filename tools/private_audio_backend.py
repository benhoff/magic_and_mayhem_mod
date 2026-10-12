"""Owned PipeWire/Pulse graph for bounded native-audio endpoint tests."""
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time

class PrivateAudioBackend:
    def __init__(self, evidence):
        self.evidence=Path(evidence)
        self.runtime=Path(tempfile.mkdtemp(prefix='mnm-audio-',dir='/tmp'))
        self.runtime.chmod(0o700)
        self.server='unix:'+str(self.runtime/'pulse/native')
        self.environment={name:value for name,value in os.environ.items()
                          if not name.startswith(('MNM_','PIPEWIRE_','PULSE_'))}
        self.environment.update(XDG_RUNTIME_DIR=str(self.runtime),
            XDG_CONFIG_HOME=str(self.runtime/'config'),XDG_STATE_HOME=str(self.runtime/'state'),
            XDG_CACHE_HOME=str(self.runtime/'cache'),
            DBUS_SESSION_BUS_ADDRESS='unix:path='+str(self.runtime/'unavailable-bus'),
            PIPEWIRE_RUNTIME_DIR=str(self.runtime),PIPEWIRE_REMOTE='pipewire-0',
            PULSE_RUNTIME_PATH=str(self.runtime/'pulse'),PULSE_SERVER=self.server)
        self.children=[];self.logs=[];self.modules={}
        self.report=dict(runtime=str(self.runtime),server=self.server,
                         desktop_routing_changed=False,hardware_monitors_enabled=False)

    def _launch(self,name,*arguments):
        log=(self.evidence/(name+'.log')).open('x');self.logs.append(log)
        child=subprocess.Popen([name,*arguments],env=self.environment,stdout=log,
                               stderr=subprocess.STDOUT,start_new_session=True)
        self.children.append(child)

    def command(self,*arguments):
        result=subprocess.run(['pactl','--server',self.server,*arguments],
            env=self.environment,text=True,capture_output=True,timeout=10)
        if result.returncode:
            raise RuntimeError('Private pactl failed: '+result.stderr.strip())
        return result.stdout

    def wait(self,predicate,description,seconds=10):
        deadline=time.monotonic()+seconds
        while time.monotonic()<deadline:
            if any(child.poll() is not None for child in self.children):
                raise RuntimeError('Owned audio daemon exited')
            if predicate():return
            time.sleep(.05)
        raise TimeoutError(description)

    def start(self):
        main=self.runtime/'mnm-private.conf'
        main.write_bytes(Path('/usr/share/pipewire/pipewire.conf').read_bytes())
        pulse=self.runtime/'mnm-private-pulse.conf'
        text=Path('/usr/share/pipewire/pipewire-pulse.conf').read_text()
        begin=text.index('pulse.cmd = [');end=text.index('\n]',begin)+2
        # Disable automatic dummy sinks and restoration modules. A uniquely
        # named explicit config also excludes site pipewire.conf.d routing.
        pulse.write_text(text[:begin]+'pulse.cmd = [\n]'+text[end:])
        for path in [main,pulse]:
            (self.evidence/path.name).write_bytes(path.read_bytes())
        self._launch('pipewire','-c',str(main))
        self.wait(lambda:(self.runtime/'pipewire-0').is_socket(),'Private PipeWire socket',15)
        self._launch('pipewire-pulse','-c',str(pulse))
        # This installed profile provides routing policy without opening audio,
        # Bluetooth or camera hardware. State/config/cache belong to the fixture.
        self._launch('wireplumber','--profile','policy')
        self.wait(lambda:(self.runtime/'pulse/native').is_socket(),'Private Pulse socket',15)
        info=self.command('info')
        if 'Server String: '+self.server+'\n' not in info:
            raise RuntimeError('Audio client did not identify owned server')
        self.report['initial_sinks']=self.command('list','short','sinks')
        if self.report['initial_sinks'].strip():
            raise RuntimeError('Unexpected sink in owned initial graph')
        return self

    def add_sink(self,label):
        if label not in ('a','b','c') or label in self.modules:
            raise ValueError('Invalid or repeated private sink label')
        name='mnm_private_'+label
        module=self.command('load-module','module-null-sink','sink_name='+name,
                            'rate=48000','channels=2','format=s16le').strip()
        if not module.isdecimal():raise ValueError('Invalid private module identity')
        self.modules[label]=module
        self.wait(lambda:name in self.command('list','short','sinks'),'Private sink publication')
        self.command('set-default-sink',name)

    def remove_sink(self,label):
        module=self.modules.pop(label)
        self.command('unload-module',module)
        self.wait(lambda:'mnm_private_'+label not in self.command('list','short','sinks'),
                  'Private sink removal')

    def finish_graph(self):
        for label in list(self.modules):self.remove_sink(label)
        self.wait(lambda:not self.command('list','short','sinks').strip(),'Private graph returned to empty')
        self.report['final_sinks']=self.command('list','short','sinks')

    def close(self):
        for child in reversed(self.children):
            if child.poll() is None:
                os.killpg(child.pid,signal.SIGTERM)
                try:child.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=5)
        for log in self.logs:log.close()
        self.report['owned_daemons_stopped']=all(child.poll() is not None for child in self.children)

    def __enter__(self):
        try:return self.start()
        except BaseException:self.close();raise

    def __exit__(self,*unused):self.close()
