#!/usr/bin/env python3
"""Exercise the production additive/colour entry trampolines against a callee-pop control.

Synthetic callback; proves stack, registers and x87/SSE restoration, without
original binaries or a claim about the production primitive admission handler.
"""
import argparse,hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['runtime/scene/canvas_producers.S','tests/resident-leaf-abi.c','tests/resident-leaf-abi.S','tests/world-producer-bypass-call.S','runtime/shadow/win32_min.h','tools/test-resident-leaf-abi.py','tools/build-shadow-bridge.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix',type=Path,required=True);a=p.parse_args()
 prefix=a.prefix.resolve()
 if not prefix.is_relative_to(ROOT/'working/tests') or prefix.name!='wineprefix':p.error('Require an owned test wineprefix')
 parent=ROOT/'working/tests/resident-leaf-abi';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
 report=dict(success=False,scope=__doc__,sources={s:sha(ROOT/s) for s in SOURCES},prefix=str(prefix))
 env=dict(os.environ,WINEPREFIX=str(prefix),WINEDEBUG='-all',TMPDIR=str(ROOT/'working/compiler-tmp'));Path(env['TMPDIR']).mkdir(exist_ok=True)
 try:
  definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\nExitProcess@4\nCreateFileA@28\nWriteFile@20\nCloseHandle@4\n')
  subprocess.run(['llvm-dlltool','-m','i386','-D','KERNEL32.dll','-d',str(definition),'-l',str(out/'kernel32.lib'),'--kill-at'],check=True)
  objects=[]
  for name in SOURCES[:4]:
   obj=out/(Path(name).name+'.obj');objects.append(str(obj))
   subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/name),'-o',str(obj)],env=env,check=True)
  exe=out/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(exe),*objects,str(out/'kernel32.lib')],check=True)
  result=subprocess.run(['wine',str(exe)],cwd=out,env=env,timeout=30,capture_output=True)
  report.update(exit_code=result.returncode,stderr=result.stderr.decode(errors='replace'),exe_sha256=sha(exe))
  if result.returncode:raise RuntimeError('Production leaf ABI mismatch')
  report.update(registers=8,fx_bytes=512,callee_pop_bytes=8,trampolines=2,output_sha256=sha(out/'leaf-abi.bin'),success=True)
 except (OSError,RuntimeError,subprocess.SubprocessError) as e:report['error']=str(e)
 finally:
  cleanup=subprocess.run(['wineserver','-k'],env=env,timeout=15)
  report['cleanup_exit_code']=cleanup.returncode
  if cleanup.returncode not in (0,1):report.update(success=False,cleanup_error='Private Wine cleanup failed')
  report['sources_unchanged']=all(sha(ROOT/s)==h for s,h in report['sources'].items())
  if not report['sources_unchanged']:report.update(success=False,error='Fixture source changed during execution')
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
