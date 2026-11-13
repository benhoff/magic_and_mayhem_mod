#!/usr/bin/env python3
"""Select the tested scene in the actual Qt preparer child process."""
import importlib.util,os,sys
from pathlib import Path
from broader_secondary_scene_variant import variant_binding
from broader_secondary_subprocess import subprocess_binding
ROOT=Path(__file__).resolve().parents[1]

def main():
    if os.environ.get('MNM_BROADER_WORLD_BUS_OWNED')!='1':
        from rendering_pilot import require
        require(sys.argv.count('--report')==1,'One prospective report required')
        report=sys.argv[sys.argv.index('--report')+1];runtime=ROOT/'working/broader-player/world-runtime'/Path(report).stem
        runtime.mkdir(mode=0o700,parents=True,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_WORLD_BUS_OWNED='1',MNM_INTERACTION_BUS_OWNED='1',MNM_PAUSE_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon');env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    path=ROOT/'tools/run-broader-visible-world.py';sp=importlib.util.spec_from_file_location('secondary_actual_subprocess_world',path);m=importlib.util.module_from_spec(sp);sp.loader.exec_module(m)
    m.WorldBinding=subprocess_binding(variant_binding(m.WorldBinding));return m.main()
if __name__=='__main__':raise SystemExit(main())
