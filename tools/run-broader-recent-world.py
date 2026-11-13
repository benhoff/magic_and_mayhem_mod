#!/usr/bin/env python3
"""Select the verified scene and its explicit compiled-input startup admission."""
import importlib.util,os,sys
from pathlib import Path
from broader_secondary_scene_variant import variant_binding
from broader_secondary_subprocess import subprocess_binding
from broader_derived_world_launch import derived_binding
from broader_interaction_recent_launch import recent_binding
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]

def main():
    if os.environ.get('MNM_BROADER_WORLD_BUS_OWNED')!='1':
        require(sys.argv.count('--report')==1,'One prospective report required')
        report=sys.argv[sys.argv.index('--report')+1];runtime=ROOT/'working/broader-player/world-runtime'/Path(report).stem
        runtime.mkdir(mode=0o700,parents=True,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_WORLD_BUS_OWNED='1',MNM_INTERACTION_BUS_OWNED='1',MNM_PAUSE_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon');env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    path=ROOT/'tools/run-broader-visible-world.py';sp=importlib.util.spec_from_file_location('explicit_derived_world_launcher',path);module=importlib.util.module_from_spec(sp);sp.loader.exec_module(module)
    module.WorldBinding=recent_binding(derived_binding(subprocess_binding(variant_binding(module.WorldBinding))))
    return module.main()
if __name__=='__main__':raise SystemExit(main())
