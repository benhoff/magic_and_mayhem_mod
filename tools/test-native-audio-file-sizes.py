#!/usr/bin/env python3
"""Installed selected audio file-size transport and guarded PE cdecl fixture."""
import argparse,hashlib,importlib.util,json,os,shlex,shutil,struct,subprocess,tempfile,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',required=True,type=Path);p.add_argument('--sounds',type=Path,default=ROOT/'working/experiments/native-audio-input/run-h4r5z9xx/game/Sounds');a=p.parse_args();declaration=json.loads(a.claims.read_text());assert all(sha(ROOT/x)==h for x,h in declaration['sources'].items())
 parent=ROOT/'working/tests/native-audio-file-sizes';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims']);host=None;env=None;before=False
 def run(label,argv,**kw):
  with (out/(label+'.log')).open('x') as log:subprocess.run([str(x) for x in argv],check=True,stdout=log,stderr=subprocess.STDOUT,timeout=180,**kw)
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
  sounds=out/'sounds';sounds.mkdir();inputs={};queries=[]
  for source in sorted(a.sounds.iterdir()):
   if not source.is_file() or source.suffix.lower()!='.wav':continue
   inputs[str(source)]=sha(source);shutil.copy2(source,sounds/source.name);queries.append(('C:\\fixture\\Sounds\\'+source.stem+'.wav',source.stat().st_size,1))
  assert len(queries)==356
  (sounds/'Dummy.wav').write_bytes(b'');(sounds/'Malformed.wav').write_bytes(b'bad')
  for source in sorted(x for x in sounds.iterdir() if x.suffix.lower()=='.wav' and x.name not in {'Dummy.wav','Malformed.wav'})[:4]:
   queries.append(("'"+source.stem+"'.wav",source.stat().st_size,1))
  queries += [('Dummy.wav',0,1),('Malformed.wav',3,1),('missing.wav',0,0),('../escape.wav',0,0),('WrongSuffix.txt',0,0)]
  with (out/'queries.bin').open('xb') as f:
   f.write(struct.pack('<I',len(queries)))
   for name,size,success in queries:f.write(struct.pack('<512sII',name.encode('ascii'),size,success))
  build=out/'build';run('configure',['cmake','-S',ROOT/'tests/native-audio-file-sizes','-B',build,'-DCMAKE_BUILD_TYPE=Release'],cwd=ROOT);run('build',['cmake','--build',build,'--target','native-audio-file-sizes-host','-j4'],cwd=ROOT)
  dependencies={'tests/native-audio-file-sizes/CMakeLists.txt','tools/test-native-audio-file-sizes.py','tests/native-audio-file-sizes/producer.c','tools/build-audio-bridge.py','audio/CMakeLists.txt','assets/CMakeLists.txt','assets/resources/CMakeLists.txt'}
  for dep in build.rglob('*.o.d'):
   for token in shlex.split(dep.read_text().replace('\\\n',' ')):
    path=Path(token)
    if path.is_absolute():
     path=path.resolve()
     if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):dependencies.add(str(path.relative_to(ROOT)))
  assert dependencies<=declaration['sources'].keys(),'Unbound compiler inputs: '+repr(sorted(dependencies-declaration['sources'].keys()))
  run('adapter',['python3',ROOT/'tools/build-audio-bridge.py','--selftest'],cwd=ROOT);dll=ROOT/'working/build/audio-bridge-selftest/MnmAudio.dll';shutil.copy2(dll,out/'MnmAudio.dll');report['adapter_sha256']=sha(dll)
  spec=importlib.util.spec_from_file_location('builder',ROOT/'tools/build-shadow-bridge.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder);imports=dict(builder.IMPORTS,LoadLibraryA=4,GetProcAddress=8,ReadFile=20,WriteFile=20,VirtualFree=12)
  definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()));run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror'];run('producer-compile',flags+['-c',ROOT/'tests/native-audio-file-sizes/producer.c','-o',out/'producer.obj']);run('producer-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x18000000','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'producer.exe'),out/'producer.obj',out/'kernel32.lib'])
  env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='mscoree=;mshtml=',QT_QPA_PLATFORM='offscreen');channel=out/'voice.bin';stop=out/'stop';final=out/'host.json'
  with (out/'host.log').open('x') as log:
   host=subprocess.Popen([str(build/'native-audio-file-sizes-host'),str(channel),str(sounds),str(stop),str(final)],env=env,stdout=log,stderr=subprocess.STDOUT)
   end=time.monotonic()+10
   while True:
    if channel.exists() and channel.stat().st_size>=128:
     with channel.open('rb') as f:ready=struct.unpack_from('<I',f.read(128),80)[0]==1
     if ready:break
    if host.poll() is not None or time.monotonic()>end:raise RuntimeError('File-size host not ready')
    time.sleep(.02)
   env.update(MNM_AUDIO_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_FILE_SIZE_DLL='Z:'+str(out/'MnmAudio.dll').replace('/','\\'));run('producer',['wine',out/'producer.exe'],cwd=out,env=env);assert (out/'producer-success.bin').read_bytes()==struct.pack('<I',1)
   stop.write_text('stop\n');host.wait(timeout=10);assert host.returncode==0
  report['native']=json.loads(final.read_text());assert report['native']['ready_after_stop'] is False and report['native']['file_size_queries']==363 and report['native']['failures']==2 and report['native']['requests']==365
  assert all(sha(Path(path))==digest for path,digest in inputs.items())
  report.update(success=True,installed_waves=356,queries=len(queries),native_successful_queries=363,expected_host_refusals=2,client_traversal_refusal=True,zero_and_malformed_file_metadata=True,only_consumed_field_written=True,last_error_preserved_native_policy=True,patched_cdecl_call_passed=True,call_and_consumed_field_guards=True,reinstall_refused=True,inputs=inputs,input_stable=True,compiler_dependencies=sorted(dependencies))
 except Exception as error:report['error']=repr(error);raise
 finally:
  if host is not None and host.poll() is None:host.terminate();host.wait(timeout=5)
  if env is not None:
   result=subprocess.run(['wineserver','-k'],env=env,timeout=10);assert result.returncode in (0,1);subprocess.run(['wineserver','-w'],env=env,timeout=10,check=True);report['private_wine_stopped']=True
  if before:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  report['sources_stable']=all(sha(ROOT/x)==h for x,h in declaration['sources'].items());(out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 assert report['sources_stable'];print('Selected audio file-size transport passed',out/'report.json')
if __name__=='__main__':main()
