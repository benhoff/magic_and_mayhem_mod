#!/usr/bin/env python3
"""Run the reviewed World adapter with the explicit inverse-ANI cell policy."""
import argparse
import importlib.util
import os
from pathlib import Path
import sys
from broader_picking_animation_launch import binding_class
ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser();p.add_argument('--plan',required=True);p.add_argument('--report',required=True)
    args=p.parse_args()
    if os.environ.get('MNM_BROADER_WORLD_BUS_OWNED')!='1':
        runtime=ROOT/'working/broader-player/world-runtime'/Path(args.report).stem
        runtime.mkdir(mode=0o700,parents=True,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_BROADER_WORLD_BUS_OWNED='1',
                 MNM_INTERACTION_BUS_OWNED='1',MNM_PAUSE_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    path=ROOT/'tools/run-broader-reviewed-world.py'
    spec=importlib.util.spec_from_file_location('animation_reviewed_world',path)
    core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
    parent=core.binding_class
    core.binding_class=lambda base,controller:binding_class(parent(base,controller))
    return core.main()

if __name__=='__main__':raise SystemExit(main())
