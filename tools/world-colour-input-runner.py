#!/usr/bin/env python3
"""Perform one bounded input action inside the private capture's Xvfb display."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import signal
import subprocess
import time
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--input-executable',type=Path,required=True);parser.add_argument('--journal',type=Path,required=True)
    args,capture_args=parser.parse_known_args()
    if not os.environ.get('DISPLAY') or args.journal.exists():parser.error('Require private Xvfb and a fresh journal')
    parent=ROOT/'working/experiments/scene-observer';before=set(parent.glob('run-*'))
    report=dict(schema=1,success=False,private_x11=True,semantic_outcomes_assumed=False,direct_original_memory_written=False,input_executable_sha256=hashlib.sha256(args.input_executable.read_bytes()).hexdigest())
    child=subprocess.Popen(['python3',str(ROOT/'tools/capture-scene-game.py'),*capture_args],cwd=ROOT)
    try:
        deadline=time.monotonic()+180;injected=False
        while child.poll() is None and time.monotonic()<deadline:
            candidates=[p for p in parent.glob('run-*') if p not in before]
            if len(candidates)>1:raise ValueError('Ambiguous private capture')
            if candidates and not injected and (candidates[0]/'capture/startup-queue-0012.bin').is_file():
                session=candidates[0];report.update(session=str(session),input_begin_startup_queues=len(list((session/'capture').glob('startup-queue-*.bin'))))
                action=subprocess.run([str(args.input_executable)],capture_output=True,text=True,timeout=15,check=False)
                report.update(input_exit=action.returncode,input_stdout=action.stdout,input_stderr=action.stderr,input_end_startup_queues=len(list((session/'capture').glob('startup-queue-*.bin'))));injected=True
                if action.returncode:raise ValueError('Private input helper failed')
            time.sleep(.05)
        if child.poll() is None:raise TimeoutError('Finite private capture deadline')
        report.update(capture_exit=child.returncode,input_executed=injected,success=child.returncode==0 and injected)
        if not report['success']:raise ValueError('Incomplete private action/capture')
    finally:
        if child.poll() is None:
            child.send_signal(signal.SIGINT)
            try:child.wait(timeout=20)
            except subprocess.TimeoutExpired:child.kill();child.wait(timeout=10)
        with args.journal.open('x') as output:json.dump(report,output,indent=2);output.write('\n')
if __name__=='__main__':main()
