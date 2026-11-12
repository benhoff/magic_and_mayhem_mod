#!/usr/bin/env python3
"""Exact-address history cache: isolated policy and actual PE32 authority controls."""
import argparse,hashlib,importlib.util,io,json,os,shutil,struct,subprocess,sys,tarfile
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);p.add_argument('--prefix-template',type=Path,required=True);a=p.parse_args()
 out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests') or (ROOT/a.report).exists():p.error('Use new owned output/report paths')
 out.mkdir(parents=True)
 inputs=[p for p in (ROOT/'runtime/native-draw').glob('*.[ch]')]+[ROOT/n for n in ['runtime/shadow/win32_min.h','reconstruction/rendering/pixel_kernel.c','reconstruction/rendering/pixel_kernel.h','tests/native-history-lookup-test.c','tests/native-direct-store-test.c','tests/native-history-probe.c','tests/native-draw-resource-probe.c','tools/test-native-history-lookup.py','tools/build-native-draw.py','protocols/include/mnm/native_gpu_copy_plan_v1.h','protocols/include/mnm/native_pixel_guard_v1.h']]
 sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}
 r=dict(success=False,sources=sources,commands=[],scope='Cache namespace resolution only. Real PE32 current/interior/stale/retired/Flip history and terminal reader refusals; no original-driver or live replacement claim.')
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))};prefix=Path('/tmp/history-lookup-prefix')
 env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=;winemenubuilder.exe=d',LIBGL_ALWAYS_SOFTWARE='1')
 def run(label,cmd,extra=None,cwd=None,expected=0):
  result=subprocess.run(list(map(str,cmd)),cwd=cwd or out,env=env|dict(extra or {}),capture_output=True,timeout=180)
  (out/(label+'.log')).write_bytes(result.stdout+result.stderr);r['commands'].append(dict(label=label,command=list(map(str,cmd)),exit_code=result.returncode,expected=expected))
  if result.returncode!=expected:raise RuntimeError(label+': '+str(result.returncode)+' '+result.stderr.decode(errors='replace')[-2500:])
 try:
  for name in ['native-history-lookup-test','native-direct-store-test']:
   for mode,flags in [('native',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
    target=out/(name+'-'+mode);run(name+'-'+mode+'-build',['cc','-std=c11','-Wall','-Wextra','-Werror',*flags,ROOT/'tests'/(name+'.c'),'-o',target]);run(name+'-'+mode,[target],{'ASAN_OPTIONS':'detect_leaks=1'})
  spec=importlib.util.spec_from_file_location('history_builder',ROOT/'tools/build-native-draw.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
  dll=builder.build(out/'provider');r['provider_sha256']=hashlib.sha256(dll.read_bytes()).hexdigest();r['compiled_sources']=json.loads((dll.parent/'manifest.json').read_bytes())['sources']
  libraries=[]
  imports={'kernel32':{'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4,'GetEnvironmentVariableA':12},'user32':{'GetDesktopWindow':0},'gdi32':{'CreateDIBSection':24,'SelectObject':8,'DeleteObject':4,'GetDIBColorTable':16}}
  for name,symbols in imports.items():
   definition=out/(name+'.def');definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in symbols.items()));library=out/(name+'.lib');run(name,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',library]);libraries.append(library)
  run('probe-build',[*builder.FLAGS,'-c',ROOT/'tests/native-history-probe.c','-o',out/'probe.obj']);run('probe-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',*libraries]);shutil.copyfile(dll,out/'MnmDraw.dll')
  # The seed is copied into this execution's private /tmp; no seed mutation.
  shutil.copytree(a.prefix_template,prefix,symlinks=True)
  for profile in ['compatibility','serialized']:
   for name,mode,expected in [('current',None,0),('read-refusal','1',124),('dc-refusal','2',124),('copy-refusal','3',124)]:
    case=out/(profile+'-'+name);case.mkdir();run(profile+'-'+name,['xvfb-run','-a','-n','530','wine',out/'probe.exe'],({'MNM_HISTORY_REFUSE':mode} if mode else {})|({'MNM_NATIVE_CURSOR_OWNERSHIP':'1'} if profile=='serialized' else {}),case,expected)
    if expected:
     words=struct.unpack('<11I',(case/'native-cpu-history-refusal.bin').read_bytes());assert words[:2]==(0x484d4e4d,2) and words[3:8]==(2,0,5,6,1)
    else:r['real_provider_checks']=struct.unpack('<I',(case/'history-success.bin').read_bytes())[0]
  r['success']=True
 except Exception as exc:r['error']=str(exc)
 finally:
  if prefix.exists():
   subprocess.run(['wineserver','-k'],env=env,check=False,timeout=15);subprocess.run(['wineserver','-w'],env=env,check=False,timeout=15)
  r['sources_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items());r['success'] &= r['sources_stable']
  raw=io.BytesIO()
  with tarfile.open(fileobj=raw,mode='w:gz') as tar:
   for f in sorted(out.rglob('*')):
    if f.is_file():tar.add(f,arcname=str(f.relative_to(out)),recursive=False)
  write_report(ROOT,a.report+'.tar.gz',raw.getvalue());write_report(ROOT,a.report,json.dumps(r,indent=2).encode()+b'\n');print(json.dumps({k:r.get(k) for k in ['success','error','real_provider_checks','provider_sha256','sources_stable']}),flush=True)
 return int(not r['success'])
if __name__=='__main__':raise SystemExit(main())
