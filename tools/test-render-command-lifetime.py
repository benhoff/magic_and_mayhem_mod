#!/usr/bin/env python3
"""Validate V3 byte rollover, real >4GiB queue/decoder traffic and legacy guards."""
import argparse,hashlib,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--qt-build',type=Path,required=True);p.add_argument('--claims',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 if a.output.exists():p.error('Refusing evidence overwrite')
 declaration=json.loads(a.claims.read_text()) if a.claims else {'claims':[],'sources':{}}
 paths=set(declaration['sources'])|{'tools/test-render-command-lifetime.py','tools/build-render-bridge.py','tools/build-shadow-bridge.py','tests/render-command-rollover-test.c','tests/render-command-queue-test.c','tests/render-command-writer-test.c','tests/render-command-ring-test.c','tests/live-render-channel-test.cpp','protocols/tests/test_contracts.py','protocols/generate.py','runtime/shadow/win32_min.h','runtime/native-draw/initial_history.h'}
 for directory in ['runtime/render','renderer','assets','protocols/include','protocols/schemas','protocols/python','apps/qt-shell']:
  paths.update(str(f.relative_to(ROOT)) for f in (ROOT/directory).rglob('*') if f.is_file() and (f.suffix in ('.cpp','.hpp','.h','.c','.S','.py','.json','.cmake') or f.name=='CMakeLists.txt'))
 sources={n:sha(ROOT/n) for n in sorted(paths)}
 if any(sources[n]!=h for n,h in declaration['sources'].items()):raise ValueError('Prospective lifetime source changed')
 parent=ROOT/'working/tests/render-command-lifetime';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text(str(a.output)+'\n');print(run,flush=True)
 result=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],executions=[],original_execution=False,scope='Actual bounded private queue and mapped V3 producer/reader pass >2^32 bytes; Qt V3 adapter/decoder/consumer passes >2^32 bytes and independent full color output. Native/sanitized boundary/refusal and V1/V2 compatibility controls. PE32 production/selftest compile only; separate live game evidence required.')
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='8',ASAN_OPTIONS='detect_leaks=0')
 def execute(label,command,timeout=180):
  done=subprocess.run([str(v) for v in command],cwd=ROOT,env=env,capture_output=True,text=True,timeout=timeout);log=run/(label+'.log');log.write_text(done.stdout+done.stderr);result['executions'].append(dict(label=label,command=[str(v) for v in command],exit_code=done.returncode,log=str(log.relative_to(ROOT)),log_sha256=sha(log)));done.check_returncode();return done.stdout
 try:
  execute('protocols',['python3','-B','protocols/tests/test_contracts.py'])
  execute('compile-ring',['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-pedantic','tests/render-command-ring-test.c','-o',run/'ring'])
  result['cross_process']=json.loads(execute('cross-process', [run/'ring']))
  for label,flags in [('native',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   execute('compile-'+label,['cc','-std=c11','-Wall','-Wextra','-Werror',*flags,'tests/render-command-rollover-test.c','-o',run/label])
   rows=[json.loads(v) for v in execute(label,[run/label,*(['--long'] if label=='native' else [])]).splitlines()];assert all(v['success'] for v in rows);result[label]=rows
  assert result['native'][-1]['queue_and_ring_actual_bytes']>2**32
  build=a.qt_build.resolve()
  if not build.is_relative_to(ROOT/'working'):raise ValueError('Use a private renderer build under working')
  execute('qt-configure',['cmake','-G','Ninja','-S',ROOT/'renderer','-B',build,'-DBUILD_TESTING=ON','-DCMAKE_BUILD_TYPE=RelWithDebInfo'],timeout=600)
  execute('qt-build',['cmake','--build',build,'--target','live-render-channel-test','--parallel','4'],timeout=600)
  binary=build/'live-render-channel-test';binary_hash=sha(binary)
  result['qt']=json.loads(execute('qt',['xvfb-run','-a','-s','-screen 0 1280x1024x24',binary]))
  assert result['qt']['v3_lifetime_bytes']>2**32 and result['qt']['ordinary_readbacks']==result['qt']['viewport_uploads']==0
  assert sha(binary)==binary_hash;result['qt_binary_sha256']=binary_hash
  execute('pe32-build',['python3','tools/build-render-bridge.py','--selftest'])
  result['pe32_build']=json.loads((ROOT/'working/build/render-selftest/manifest.json').read_text())
  missing=[n for n,h in result['pe32_build']['compiled_sources'].items() if sources.get(n)!=h]
  result['undeclared_compiler_sources']=missing
  if missing:raise ValueError('Undeclared compiler sources: '+repr(missing))
  result['changed_sources']=[n for n,h in sources.items() if sha(ROOT/n)!=h];result['sources_stable']=not result['changed_sources']
  if not result['sources_stable']:raise ValueError('Sources changed: '+repr(result['changed_sources']))
  result['success']=True
 except Exception as error:result['error']=str(error)
 finally:
  a.output.parent.mkdir(parents=True,exist_ok=True)
  with a.output.open('x') as out:json.dump(result,out,indent=2);out.write('\n')
  print(a.output,flush=True)
 if not result['success']:raise SystemExit(1)
if __name__=='__main__':main()
