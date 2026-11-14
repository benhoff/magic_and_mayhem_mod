#!/usr/bin/env python3
"""Run the existing physical recovery controller on an explicitly bound provider."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys

import coverage_evidence
from broader_native_host import gpu_environment, start_nvidia
from broader_capture_storage import CaptureStorage
from coverage_evidence import Reader
from native_render_driver import require_nvidia
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kind', choices=('loss', 'transition'), required=True)
    parser.add_argument('--provider', choices=('mesa', 'nvidia'), required=True)
    parser.add_argument('--plan', required=True)
    parser.add_argument('--report', required=True)
    parser.add_argument('--low', action='store_true')
    parser.add_argument('--software', action='store_true')
    args = parser.parse_args()
    require(args.provider != 'nvidia' or not args.software, 'NVIDIA cannot supply a software-profile claim')
    if os.environ.get('MNM_BROADER_RECOVERY_BUS_OWNED') != '1':
        runtime = ROOT/'working/broader-player/recovery-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True, mode=0o700, exist_ok=False)
        env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_BROADER_RECOVERY_BUS_OWNED='1',
                   MNM_TRANSITION_BUS_OWNED='1', ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable, __file__, *sys.argv[1:]], env)
    with Reader(ROOT) as reader:
        plan = reader.json(args.plan)
        require(plan['success'] and all(reader.digest(n) == h for n, h in plan['sources'].items()),
                'Provider phase sources changed')
        matches = [c for c in plan['broader_cases'] if c['report'] == args.report]
        require(len(matches) == 1 and matches[0]['provider'] == args.provider, 'Unbound provider case')
        wrapper_sources = {n: reader.digest(n) for n in
            ('tools/run-broader-recovery.py', 'tools/broader_native_host.py', 'tools/native_render_driver.py')}
        plan_ref = dict(record=args.plan, sha256=reader.digest(args.plan))
    capture=CaptureStorage(ROOT,args.report,plan.get('capture_storage',{}).get('enabled',False))
    filename = 'run-rendering-recovery-closeout.py' if args.kind == 'loss' or args.low else 'run-rendering-transition.py'
    spec = importlib.util.spec_from_file_location('broader_provider_recovery', ROOT/'tools'/filename)
    core = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(core)
    if filename == 'run-rendering-transition.py':
        from rendering_transition_low_live import TransitionJourney as Safe
        from rendering_transition_live import TransitionJourney as Wide
        class SafeWide(Safe):
            def normal_quit(self):
                return Wide.normal_quit(self)
        core.TransitionJourney = SafeWide
    host_receipts, environments = [], []
    original_module = core.menus.module
    def module(name):
        loaded = original_module(name)
        if name == 'run-native-owned.py' and args.provider == 'nvidia':
            original_display = loaded.start_display
            def display(directory, allow_glx=False):
                if not allow_glx:
                    return original_display(directory, allow_glx=False)
                server, receipt, cookie = start_nvidia(directory)
                host_receipts.append(receipt)
                return server, receipt, cookie
            loaded.start_display = display
        return loaded
    core.menus.module = module
    original_popen = subprocess.Popen
    class Processes:
        def __getattr__(self, name):
            return getattr(subprocess, name)
        def Popen(self, command, *positional, **keywords):
            if command and str(command[0]) == str(ROOT/plan['player_binary']['path']) and args.provider == 'nvidia':
                selected = gpu_environment(keywords['env'])
                keywords['env'] = selected
                environments.append({n: selected.get(n) for n in ('DISPLAY', 'XAUTHORITY',
                    'MESA_LOADER_DRIVER_OVERRIDE', 'GALLIUM_DRIVER', 'LIBGL_ALWAYS_SOFTWARE',
                    '__GLX_VENDOR_LIBRARY_NAME', '__EGL_VENDOR_LIBRARY_FILENAMES')})
            player=bool(command)and str(command[0])==str(ROOT/plan['player_binary']['path'])
            if player:capture.prepare(keywords['env'])
            process=original_popen(command,*positional,**keywords)
            return capture.wrap(process)if player else process
    core.subprocess = Processes()
    original_connect = sqlite3.connect
    class State:
        def __getattr__(self, name):
            return getattr(sqlite3, name)
        def connect(self, path, *positional, **keywords):
            if Path(path) == ROOT/'working/transition-attempts.sqlite3':
                path = ROOT/plan['execution_paths']['budget']
                path.parent.mkdir(parents=True, exist_ok=True)
            return original_connect(path, *positional, **keywords)
    core.sqlite3 = State()
    original_write = core.write_report
    def writer(root, path, raw):
        if path == args.report:
            value = json.loads(raw)
            value['delegate_success'] = value['success']
            drivers = [e['resident']['renderer'] for e in value.get('proof', {}).get('epochs', [])]
            error = None
            try:
                if args.provider == 'nvidia':
                    require(len(drivers) == 2 and len(host_receipts) == 1 and len(environments) == 1,
                            'Missing actual NVIDIA provider generation')
                    require_nvidia(drivers)
            except ValueError as failure:
                error = str(failure)
            stable = all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest() == h for n, h in wrapper_sources.items())
            value.setdefault('sources', {}).update(wrapper_sources)
            value.update(success=value['success'] and stable and error is None,
                sources_stable=value.get('sources_stable', True) and stable,
                provider_adaptation=dict(provider=args.provider, actual_drivers=drivers,capture_storage=capture.receipt(),
                    host_receipts=host_receipts, actual_qt_environments=environments,
                    controller=filename, startup_polling='owned admissions and original seqlocks',
                    budget_namespace=plan['execution_paths']['budget'], phase=plan_ref,
                    game_actions_changed=False, response_deadlines_changed=False,
                    original_game_runs_added_by_adapter=0, physical_device_reset=False))
            if error is not None:
                value['provider_admission_error'] = error
            raw = (json.dumps(value, indent=2)+'\n').encode()
        return original_write(root, path, raw)
    core.write_report = writer
    sys.argv = [str(ROOT/'tools'/filename), '--plan', args.plan, '--report', args.report]
    if args.low:
        sys.argv.append('--low')
    if args.software:
        sys.argv.append('--software')
    if filename == 'run-rendering-recovery-closeout.py':
        return core.main(surface_loss=args.kind == 'loss')
    return core.main()


if __name__ == '__main__':
    raise SystemExit(main())
