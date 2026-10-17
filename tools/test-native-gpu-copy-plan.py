#!/usr/bin/env python3
"""Owned facade/production bridge clipping and immutable GPU alias-copy controls."""
import argparse,fcntl,hashlib,importlib.util,json,os,shutil,struct,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def load(name):
 spec=importlib.util.spec_from_file_location(name.replace('-','_'),ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def cases():
 full=[0,0,13,9]
 rows=[]
 def add(name,src=full,dst=full,*,self=0,clip=0,keyed=0,bootstrap=0,current=0,width=13,height=9):
  rows.append(dict(name=name,words=[len(rows)+1,width,height,self,clip,keyed,bootstrap,current,*src,*dst]))
 add('disjoint');add('keyed',keyed=1);add('bootstrap',bootstrap=1)
 add('clip-single',clip=1);add('clip-multiple',clip=2);add('clip-empty',clip=3,current=1)
 add('clip-outside',[-2,-1,15,10],[-2,-1,15,10],clip=1)
 add('clip-late-source-failure',[2,0,15,9],full,clip=2)
 add('self-late-source-failure',[2,0,15,9],full,self=1,clip=2)
 add('clip-first-source-failure',[-8,0,5,9],full,clip=2)
 add('self-identity',self=1);add('self-right',[0,0,12,9],[1,0,13,9],self=1)
 add('self-left',[1,0,13,9],[0,0,12,9],self=1);add('self-down',[0,0,13,8],[0,1,13,9],self=1)
 add('self-up',[0,1,13,9],[0,0,13,8],self=1)
 add('self-clipped-cross-piece',[0,0,11,8],[2,1,13,9],self=1,clip=2)
 add('self-keyed',[0,0,12,9],[1,0,13,9],self=1,keyed=1)
 add('self-keyed-clipped',[0,0,11,8],[2,1,13,9],self=1,clip=2,keyed=1)
 add('self-alias-interface',[0,0,12,9],[1,0,13,9],self=2,clip=1)
 add('self-empty',self=1,clip=3)
 add('clip-integer-extremes',[-2147483648,0,2147483647,9],[-2147483648,0,2147483647,9],clip=1)
 add('clip-256',[0,0,512,1],[0,0,512,1],clip=4,width=512,height=1)
 add('self-clip-256',[0,0,512,1],[0,0,512,1],self=1,clip=4,width=512,height=1)
 return rows

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);p.add_argument('--output-parent',type=Path,required=True);p.add_argument('--wine-prefix',type=Path,required=True);p.add_argument('--renderer',type=Path,required=True);p.add_argument('--headless',action='store_true');a=p.parse_args()
 if a.report.exists():p.error('Refusing historical result overwrite')
 if a.headless:
  return subprocess.call(['xvfb-run','-a','-s','-screen 0 1280x1024x24',sys.executable,'-B',__file__,*[x for x in sys.argv[1:] if x!='--headless']])
 claims=json.loads(a.claims.read_text());sources=claims['sources'];assert all(sha(ROOT/n)==h for n,h in sources.items()),'Prospective sources changed'
 parent=Path(os.path.abspath(a.output_parent));prefix=Path(os.path.abspath(a.wine_prefix))
 if not parent.is_relative_to(ROOT/'working') or not prefix.is_relative_to(ROOT/'working'):raise ValueError('Use owned working paths')
 parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text(__doc__+'\n')
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINEPREFIX','WINEDLLOVERRIDES','WINEARCH'))};env.update(XDG_CACHE_HOME=str(run/'cache'),MESA_SHADER_CACHE_DIR=str(run/'shader-cache'),WINEPREFIX=str(prefix.resolve()),WINEARCH='win64',WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=',QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4',QT_SCALE_FACTOR='1')
 report=dict(schema=1,success=False,claims=claims['claims'],sources=sources,experiment=str(run.relative_to(ROOT)),scope=__doc__+'; supplied inputs, no original EXE or live replacement.',cases=[],original_pixels_used_as_native_inputs=False,live_replacement=False)
 def execute(label,command,cwd=ROOT,expected=0):
  with (run/(label+'.log')).open('xb') as out:r=subprocess.run([str(x) for x in command],cwd=cwd,env=env,stdout=out,stderr=subprocess.STDOUT,timeout=240)
  if r.returncode!=expected:raise ValueError(label+': exit '+str(r.returncode)+', inspect '+str(run/(label+'.log')))
 reservation=None;verified_before=False
 try:
  marker=prefix/'.mnm-gpu-copy-plan-owner.json'
  if prefix.exists():
   if not marker.is_file() or json.loads(marker.read_text()).get('owner')!='native-gpu-copy-plan-fixture-v1':raise ValueError('Refusing unrelated Wine prefix')
  else:
   prefix.mkdir(parents=True);marker.write_text(json.dumps(dict(owner='native-gpu-copy-plan-fixture-v1'))+'\n')
  lease=(prefix/'.mnm-gpu-copy-plan.lock').open('a')
  try:fcntl.flock(lease,fcntl.LOCK_EX|fcntl.LOCK_NB)
  except BaseException:lease.close();raise
  reservation=lease
  execute('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified_before=True
  if not (prefix/'system.reg').exists():execute('wineboot',['wineboot','-u'])
  execute('command-guards-build',['clang','-std=c11','-Wall','-Wextra','-Werror','-pedantic',ROOT/'tests/native-gpu-copy-commands-test.c','-o',run/'command-guards'])
  execute('command-guards',[run/'command-guards']);report['command_guards']=json.loads((run/'command-guards.log').read_text().splitlines()[-1])
  builder=load('build-native-draw');dll=builder.build(run/'facade',sources)
  # Compile exactly the production bridge translation unit inside the fixture.
  # Production/fixture export tables differ; all production function bodies match.
  bridge=load('build-render-bridge');execute('bridge-build',[sys.executable,ROOT/'tools/build-render-bridge.py','--selftest'])
  obj=run/'probe.obj';dep=run/'probe.d';execute('probe-compile',[*builder.FLAGS,'-MD','-MF',dep,'-c',ROOT/'tests/native-gpu-copy-plan-probe.c','-o',obj])
  closure=set()
  for token in dep.read_text().replace('\\\n',' ').split():
   path=Path(token).resolve()
   if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):closure.add(path.relative_to(ROOT).as_posix())
  if closure-set(sources):raise ValueError('Undeclared fixture inputs: '+repr(sorted(closure-set(sources))))
  definition=run/'load.def';definition.write_text('LIBRARY kernel32.dll\nEXPORTS\nLoadLibraryA@4\n');execute('load-library',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',run/'load.lib'])
  build=ROOT/'working/build/render-selftest';exe=run/'probe.exe'
  execute('probe-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(exe),obj,build/'kernel32.lib',build/'kernelbase.lib',build/'gdi32.lib',run/'load.lib'])
  legacy_obj=run/'legacy.obj';execute('legacy-compile',[*builder.FLAGS,'-c',ROOT/'tests/native-gpu-copy-probe.c','-o',legacy_obj])
  extra=[]
  for dll_name,symbol in [('gdi32','GetDIBColorTable@16'),('user32','GetDesktopWindow@0')]:
   definition=run/(dll_name+'-fixture.def');definition.write_text('LIBRARY '+dll_name+'.dll\nEXPORTS\n'+symbol+'\n');lib=run/(dll_name+'-fixture.lib');execute(dll_name+'-fixture-library',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',lib]);extra.append(lib)
  execute('legacy-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'legacy.exe'),legacy_obj,build/'kernel32.lib',build/'gdi32.lib',run/'load.lib',*extra])
  legacy=run/'legacy';legacy.mkdir();shutil.copyfile(dll,legacy/'MnmDraw.dll');execute('legacy',['wine',run/'legacy.exe'],legacy);report['legacy_gpu_copy']=dict(success=True,legacy_self_refusal=True,private_baseline_and_cpu_restore=True)
  renderer_hash=sha(a.renderer)
  for case in cases():
   name=case['name'];out=run/name;out.mkdir();shutil.copyfile(dll,out/'MnmDraw.dll');(out/'input.bin').write_bytes(struct.pack('<16I',*[x&0xffffffff for x in case['words']]))
   execute(name,['wine',exe],out)
   raw=(out/'output.bin').read_bytes();h=struct.unpack_from('<12I',raw);count=case['words'][1]*case['words'][2]
   if len(raw)!=48+6192+count*2 or h[:5]!=(0x50435047,1,*case['words'][:3]):raise ValueError(name+': output identity/extent')
   plan=struct.unpack_from('<12I',raw,48)
   if plan[:3]!=(1,6192,h[6]) or plan[3]!=h[5] or plan[4:8]!=(case['words'][1],case['words'][2],case['words'][1],case['words'][2]) or plan[8:10]!=(int(bool(case['words'][3])),case['words'][5]):raise ValueError(name+': plan metadata')
   if 'late-source-failure' in name and (h[5]!=0x88760096 or h[6]!=1):raise ValueError(name+': missing partial-failure branch')
   if name.endswith('256') and h[6]!=256:raise ValueError(name+': incomplete capacity case')
   if 'empty' in name and h[6]!=0:raise ValueError(name+': missing empty clip case')
   stream=out/'session-00000001.bin';execute(name+'-gpu',[a.renderer,stream,'--output',out/'native.bin'])
   native=(out/'native.bin').read_bytes();expected=raw[48+6192:]
   if native!=expected:raise ValueError(name+': GPU pixels differ from independent CPU facade policy')
   gpu=json.loads((run/(name+'-gpu.log')).read_text().splitlines()[-1])
   if not gpu['rendered'] or gpu['surface_stats']['surfaces']!=0 or gpu['checks'] or gpu['color_checks'] or gpu['surface_copies']:raise ValueError(name+': native cleanup')
   report['cases'].append(dict(name=name,success=True,pixels=count,result=h[5],pieces=h[6],checks=h[7],commands=h[8],stale_cpu=bool(h[9]),source_frozen=bool(plan[8] and plan[2]),gpu_sha256=sha(out/'native.bin'),command_sha256=sha(stream),cpu_storage_untouched=True,final_native_surfaces=0))
  report['compatibility']={}
  for name in ['render-command-consumer-test','render-resident-world-test']:
   binary=a.renderer.parent/name;execute(name,[binary]);report['compatibility'][name]=dict(success=True,sha256=sha(binary))
  report.update(renderer_sha256=renderer_hash,probe_sha256=sha(exe),facade_sha256=sha(dll),wine_version=subprocess.check_output(['wine','--version'],env=env,text=True).strip())
  if sha(a.renderer)!=renderer_hash:raise ValueError('Renderer binary changed')
  report['success']=True
 except (OSError,ValueError,RuntimeError,AssertionError,subprocess.SubprocessError) as e:report['error']=str(e)
 finally:
  if reservation:
   subprocess.run(['wineserver','-k'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=20)
   subprocess.run(['wineserver','-w'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=20)
   reservation.close()
  execute('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=verified_before
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success']=report['success'] and report['sources_stable']
  a.report.parent.mkdir(parents=True,exist_ok=True)
  with a.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
 print(json.dumps({k:report[k] for k in ['success','experiment','sources_stable']}));return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
