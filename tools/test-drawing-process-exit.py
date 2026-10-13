#!/usr/bin/env python3
"""Observe owned Wine child exits through real thread and process handles."""
import argparse,hashlib,importlib.util,json,os,struct,subprocess,tempfile,time
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
from drawing_process_exit import Monitor
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--existing-fixture-prefix',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing evidence overwrite')
 prefix=a.existing_fixture_prefix.resolve();owned=(ROOT/'working/tests/drawing-cursor-source-gate').resolve()
 if not prefix.is_relative_to(owned) or prefix.name!='wineprefix' or not (prefix.parent/'execution-status.json').is_file():raise ValueError('Prefix must belong to a completed isolated gate fixture')
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(x for x in reg['behaviors'] if x['id']=='NR.drawing-quit-dispatch-observation')
 claim=dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in reg['builds']}),scenarios={})
 declaration=json.loads(a.claims.read_text());sources=declaration['sources']
 if declaration['claims']!=[claim] or set(sources)!=required_sources(b) or any(sha(ROOT/n)!=v for n,v in sources.items()):raise ValueError('Prospective fixture scope/source mismatch')
 parent=ROOT/'working/tests/drawing-process-exit';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 spec=importlib.util.spec_from_file_location('builder',ROOT/'tools/build-drawing-process-exit.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder);binary=builder.build(run/'build')
 definition=run/'child-kernel32.def';imports=dict(builder.IMPORTS,Sleep=4,GetCurrentThreadId=0,GetCurrentProcessId=0)
 definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in imports.items()))
 with (run/'child-build.log').open('x') as log:
  subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'child-kernel32.lib')],stdout=log,stderr=subprocess.STDOUT,check=True)
  subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/drawing-process-exit-child.c'),'-o',str(run/'child.obj')],stdout=log,stderr=subprocess.STDOUT,check=True)
  child=run/'child.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(child),str(run/'child.obj'),str(run/'child-kernel32.lib')],stdout=log,stderr=subprocess.STDOUT,check=True)
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_') and k!='LD_PRELOAD'};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all');cases=[]
 try:
  for code in [0,37,259]:
   case=run/('exit-'+str(code));case.mkdir();ready=case/'ready.bin';release=case/'release.flag'
   child_env=dict(env,MNM_CHILD_READY_FILE='Z:'+str(ready).replace('/','\\'),MNM_CHILD_RELEASE_FILE='Z:'+str(release).replace('/','\\'),MNM_CHILD_EXIT_CODE=str(code))
   monitor=None
   with (case/'child.log').open('x') as log:
    process=subprocess.Popen(['wine',str(child)],env=child_env,stdout=log,stderr=subprocess.STDOUT)
    try:
     deadline=time.monotonic()+15
     while not ready.exists() or ready.stat().st_size!=8:
      if process.poll() is not None or time.monotonic()>deadline:raise ValueError('Owned child did not publish live identity')
      time.sleep(.02)
     thread,pid=struct.unpack('<2I',ready.read_bytes());monitor=Monitor(binary,env,thread,case,lambda:None)
     if monitor.attached['process']!=pid:raise ValueError('Observer attached to another process')
     release.write_bytes(b'exit\n');observed=monitor.finish();launcher=process.wait(timeout=10)
     if observed['exit_code']!=code:raise ValueError('Actual owned process exit differs')
     cases.append(dict(expected_exit_code=code,observation=observed,child_launcher_return_code=launcher,journal_sha256=sha(monitor.path)))
    finally:
     if monitor:monitor.close()
     if process.poll() is None:process.terminate();process.wait(timeout=5)
  if any(sha(ROOT/n)!=v for n,v in sources.items()):raise ValueError('Fixture sources changed')
  report=dict(schema=1,success=True,sources=sources,claims=[claim],sources_stable=True,artifacts=str(run.relative_to(ROOT)),observer_binary_sha256=sha(binary),child_binary_sha256=sha(child),cases=cases,original_game_executed=False,target_memory_read=False,target_terminated=False,scope='Owned PE32 children only. Real OpenThread/GetProcessIdOfThread/OpenProcess attach while alive, signaled WaitForSingleObject and GetExitCodeProcess retain actual0/37/259 exits independently of launcher codes. Exit259 after signal is accepted as a real terminated code. No original game shutdown/quiescence/reference/parity proof.')
  with a.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
  print(json.dumps(dict(success=True,actual_exit_codes=[x['observation']['exit_code'] for x in cases])))
 finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
if __name__=='__main__':main()
