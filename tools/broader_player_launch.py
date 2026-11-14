"""Explicit host, launch and counter bindings for unmodified physical controllers."""
import hashlib
import importlib.util
import json
from pathlib import Path
import sqlite3
import subprocess
import types

from broader_capture_storage import CaptureStorage
from broader_native_host import gpu_environment, start_nvidia
from rendering_pilot import require


class LaunchBinding:
    def __init__(self, root, plan, report, provider):
        self.root, self.plan, self.report, self.provider = Path(root), plan, report, provider
        self.hosts, self.environments, self.loaded = [], [], set()
        self.popen, self.connect = subprocess.Popen, sqlite3.connect
        self.capture=CaptureStorage(self.root,self.report,self.plan.get('capture_storage',{}).get('enabled',False))

    def patch(self, module):
        path = getattr(module, '__file__', None)
        if not path or not Path(path).resolve().is_relative_to(self.root/'tools') or id(module) in self.loaded:
            return module
        self.loaded.add(id(module))
        if Path(path).name == 'run-native-owned.py' and self.provider == 'nvidia':
            original = module.start_display
            def display(directory, allow_glx=False):
                if not allow_glx:
                    return original(directory, allow_glx=False)
                server, receipt, cookie = start_nvidia(directory)
                self.hosts.append(receipt)
                return server, receipt, cookie
            module.start_display = display
        factory = vars(module).get('module')
        if callable(factory) and not isinstance(factory, types.ModuleType):
            def load(*args, **kwargs):
                return self.patch(factory(*args, **kwargs))
            module.module = load
        for item in list(vars(module).values()):
            if isinstance(item, types.ModuleType):
                self.patch(item)
        return module

    def load(self, filename):
        spec = importlib.util.spec_from_file_location('broader_'+filename.replace('-', '_'), self.root/'tools'/filename)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return self.patch(module)

    def install(self):
        def popen(command, *args, **kwargs):
            if command and str(command[0]) == str(self.root/self.plan['player_binary']['path']):
                env = dict(kwargs['env'])
                if self.provider == 'nvidia':
                    env = gpu_environment(env)
                kwargs['env'] = env
                self.capture.prepare(env)
                self.environments.append({n:env.get(n) for n in ('DISPLAY', 'XAUTHORITY',
                    'MESA_LOADER_DRIVER_OVERRIDE', 'GALLIUM_DRIVER', 'LIBGL_ALWAYS_SOFTWARE',
                    '__GLX_VENDOR_LIBRARY_NAME', '__EGL_VENDOR_LIBRARY_FILENAMES')})
            process=self.popen(command,*args,**kwargs)
            return self.capture.wrap(process)if command and str(command[0])==str(self.root/self.plan['player_binary']['path'])else process
        def connect(path, *args, **kwargs):
            replacement = self.plan['counter_bindings'].get(str(path))
            if replacement is not None:
                path = self.root/replacement
                path.parent.mkdir(parents=True, exist_ok=True)
            return self.connect(path, *args, **kwargs)
        subprocess.Popen, sqlite3.connect = popen, connect

    def restore(self):
        subprocess.Popen, sqlite3.connect = self.popen, self.connect

    def receipt(self):
        if self.provider == 'nvidia':
            require(len(self.hosts) == len(self.environments) == 1, 'Missing unique NVIDIA host/player launch')
            require(self.environments[0]['DISPLAY'] == self.hosts[0]['display']
                    and self.environments[0]['XAUTHORITY'] == self.hosts[0]['authority']
                    and self.environments[0]['LIBGL_ALWAYS_SOFTWARE'] is None
                    and self.environments[0]['GALLIUM_DRIVER'] == 'zink', 'NVIDIA launch environment differs')
        else:
            require(not self.hosts and len(self.environments) == 1, 'Missing unique software player launch')
        return dict(provider=self.provider, host_receipts=self.hosts, actual_qt_environments=self.environments,capture_storage=self.capture.receipt(),
            budget_namespace=self.plan['counter_bindings'], game_actions_changed=False,
            response_deadlines_changed=False, original_game_runs_added_by_adapter=0,
            physical_device_reset=False)
