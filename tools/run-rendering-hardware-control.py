#!/usr/bin/env python3
"""Execute a diagnostic on a private no-monitor/no-input NVIDIA seat."""
import argparse,importlib.util,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('command',nargs=argparse.REMAINDER);a=p.parse_args()
 if not a.command or a.command[0]!='--' or len(a.command)<2:p.error('Provide -- command arguments')
 output=a.output.absolute()
 if not output.is_relative_to(ROOT/'working') or output.exists():p.error('Require a new owned working output')
 output.mkdir(parents=True)
 spec=importlib.util.spec_from_file_location('hardware_control_helpers',ROOT/'tools/test-native-campaign.py');helper=importlib.util.module_from_spec(spec);spec.loader.exec_module(helper)
 env=helper.clean_environment(output);server=None
 try:
  server=helper.start_native_hardware(output,env)
  return subprocess.run(a.command[1:],env=env,cwd=ROOT).returncode
 finally:helper.stop_group(server)
if __name__=='__main__':raise SystemExit(main())
