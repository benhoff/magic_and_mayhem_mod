#!/usr/bin/env python3
"""Use the unchanged public UI journey with source640-only physical controls."""
import argparse
import importlib.util
import os
from pathlib import Path
import sys
from types import SimpleNamespace
from rendering_pilot import require
from rendering_ui_low import CONTROL,make_journey
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(add_help=False);p.add_argument('--report',required=True);p.add_argument('--low',action='store_true');args,_=p.parse_known_args()
    require(args.low,'Source640 control requires --low')
    if os.environ.get('MNM_INTERACTION_BUS_OWNED')!='1':
        runtime=ROOT/'working/ui-runtime'/Path(args.report).stem;runtime.mkdir(parents=True,mode=0o700,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_INTERACTION_BUS_OWNED='1');env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    spec=importlib.util.spec_from_file_location('ui_high_controller',ROOT/'tools/run-rendering-ui.py');high=importlib.util.module_from_spec(spec);spec.loader.exec_module(high)
    real_spec,real_module=importlib.util.spec_from_file_location,importlib.util.module_from_spec
    def adapted_spec(name,path):
        s=real_spec(name,path)
        if name!='ui_base':return s
        original_loader=s.loader
        class Loader:
            def create_module(self,spec):return original_loader.create_module(spec)
            def exec_module(self,module):
                original_loader.exec_module(module)
                paths=module.native_runtime_source_paths
                module.native_runtime_source_paths=lambda root:paths(root)|set(CONTROL)
                launch=module.main
                def low_launch():
                    # The high controller installs its Journey before invoking
                    # base.main. Adapt only its physical source640 policies.
                    module.InteractionJourney=make_journey(module.InteractionJourney)
                    return launch()
                module.main=low_launch
        s.loader=Loader();return s
    high.importlib=SimpleNamespace(util=SimpleNamespace(spec_from_file_location=adapted_spec,module_from_spec=real_module))
    return high.main()


if __name__=='__main__':raise SystemExit(main())
