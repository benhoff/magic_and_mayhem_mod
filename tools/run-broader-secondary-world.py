#!/usr/bin/env python3
"""Run the existing World launcher with a prospectively selected scene variant."""
import importlib.util
import os
from pathlib import Path
import sys
from broader_secondary_scene_variant import variant_binding
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]


def main():
    if os.environ.get('MNM_BROADER_WORLD_BUS_OWNED') != '1':
        require(sys.argv.count('--report') == 1, 'One prospective report required')
        report = sys.argv[sys.argv.index('--report') + 1]
        runtime = ROOT / 'working/broader-player/world-runtime' / Path(report).stem
        runtime.mkdir(mode=0o700, parents=True, exist_ok=False)
        env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_BROADER_WORLD_BUS_OWNED='1',
                   MNM_INTERACTION_BUS_OWNED='1', MNM_PAUSE_BUS_OWNED='1',
                   ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable, __file__, *sys.argv[1:]], env)
    path = ROOT / 'tools/run-broader-visible-world.py'
    spec = importlib.util.spec_from_file_location('secondary_variant_world_launcher', path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    module.WorldBinding = variant_binding(module.WorldBinding)
    return module.main()


if __name__ == '__main__': raise SystemExit(main())
