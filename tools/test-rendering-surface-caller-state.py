#!/usr/bin/env python3
"""Compare current owned facade caller outputs and pixels with real Wine DirectDraw."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
from coverage_evidence import write_report
from drawing_fixture_prefix import prepare

ROOT=Path(__file__).resolve().parents[1]
MAIN=(ROOT/subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--git-common-dir'],text=True).strip()).resolve().parent
PREFIX='research/runtime/caller-state-results-20261108/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--name',required=True);p.add_argument('--claims',type=Path,required=True);a=p.parse_args()
 if not a.name or any(c not in 'abcdefghijklmnopqrstuvwxyz0123456789-' for c in a.name):p.error('Unique lowercase name required')
 target=PREFIX+a.name+'.json'
 if (ROOT/target).exists():p.error('Preserve immutable result')
 out=Path(tempfile.mkdtemp(prefix='mnm-surface-caller-',dir='/tmp'));sha=lambda b:hashlib.sha256(b).hexdigest()
 frozen={}
 for directory in ('runtime','reconstruction','protocols','tests','tools'):
  for path in (ROOT/directory).rglob('*'):
   if path.is_file()and path.suffix in ('.c','.h','.cpp','.hpp','.S','.py','.sh'):frozen[path.relative_to(ROOT).as_posix()]=path.read_bytes()
 declaration=json.loads(a.claims.read_text())
 if any(n not in frozen or sha(frozen[n])!=h for n,h in declaration['sources'].items()):raise ValueError('Changed prospective surface sources')
 report=dict(schema=1,kind='surface_caller_state_provider_comparison',success=False,original_game_runs=0,newly_passed_requirements=0,
  scope='Actual current PE32 native facade versus Wine builtin DirectDraw: factory HRESULT/interface identity/refcounts, RGB565 surface copy/fill/key/overlap outputs, clipper/DC/primary startup outputs, and five invalid BltFast boundary requests with complete source/destination pixels. Native LastError preservation and mode/capacity/placement policies remain separate. No original game callers, full GPR/FX comparison, hardware driver, installed GPU history or whole-row acceptance.',
  claims=declaration['claims'],temporary_directory=str(out),temporary_classification='RAM compiler objects/import libraries are temporary. Tested binaries, exact changed source preimages, provider result bytes and diagnostics are retained in SQLite before closeout; RAM remains held.',commands=[],source_preimages={})
 def retain(name,raw):
  path=PREFIX+a.name+'-'+name;write_report(ROOT,path,raw);return dict(path=path,sha256=sha(raw),bytes=len(raw))
 used=set(declaration['sources'])|{'tools/test-rendering-surface-caller-state.py','tools/build-native-draw.py','tools/drawing_fixture_prefix.py','tools/experiment_environment.py','tools/original-manifest.sh'}
 lease=None;prefix=None;xvfb=None;env=None
 def run(label,argv,extra=None,cwd=None,check=True):
  r=subprocess.run(list(map(str,argv)),cwd=cwd or out,env={**(env or os.environ),**(extra or {})},capture_output=True,timeout=180)
  report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=r.returncode,log=retain(label+'.log.bin',r.stdout+r.stderr)))
  if check and r.returncode:raise RuntimeError(label+': exit '+str(r.returncode))
  return r
 try:
  run('original-before',[MAIN/'tools/original-manifest.sh','verify'])
  for candidate in sorted((MAIN/'working/tests/drawing-fixture-prefix-suite').glob('*/wineprefix'),key=lambda x:('wow64'not in str(x),str(x))):
   # This host's wine command executes PE32 through WoW64. A marked legacy
   # win32 prefix is owned but cannot load kernel32 with this installed loader.
   if not (candidate/'system.reg').is_file()or '#arch=win64'not in (candidate/'system.reg').read_text(errors='replace'):
    continue
   try:prefix,lease=prepare(MAIN,out,None,shared=candidate);break
   except (OSError,ValueError,subprocess.SubprocessError):continue
  if lease is None:raise ValueError('No idle exclusively leased fixture prefix')
  report['prefix']=str(prefix)
  display=out/'display';handle=display.open('wb');xvfb_log=(out/'xvfb.log').open('wb')
  xvfb=subprocess.Popen(['Xvfb','-displayfd',str(handle.fileno()),'-screen','0','1024x768x24','-nolisten','tcp'],pass_fds=(handle.fileno(),),stdout=xvfb_log,stderr=xvfb_log);handle.close()
  import time
  for _ in range(100):
   if display.read_bytes().strip():break
   if xvfb.poll()is not None:raise ValueError('Xvfb stopped')
   time.sleep(.05)
  number=display.read_text().strip()
  if not number.isdecimal():raise ValueError('No private X display')
  env={k:v for k,v in os.environ.items()if not k.startswith(('MNM_','WINE','DISPLAY'))}
  env.update(DISPLAY=':'+number,WINEPREFIX=str(prefix),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',TMPDIR=str(out))
  run('wine-version',['wine','--version']);run('clang-version',['clang','--version'])
  spec=importlib.util.spec_from_file_location('surface_state_builder',ROOT/'tools/build-native-draw.py');builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
  # Preserve every changed potential dependency before compilation/execution.
  changed=subprocess.check_output(['git','ls-files','--modified','--others','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
  for name in changed:
   if name in frozen:report['source_preimages'][name]=retain('source-'+sha(name.encode())[:16]+'.bin',frozen[name])
  dll=builder.build(out/'native-build',frozen);shutil.copyfile(dll,out/'MnmDraw.dll')
  manifest=json.loads((out/'native-build/manifest.json').read_text());used.update(manifest['sources']);report['native_build']=manifest
  imports={k:dict(v)for k,v in builder.IMPORTS.items()}
  imports['kernel32'].update(LoadLibraryA=4);imports['user32'].update(GetDesktopWindow=0);imports['gdi32'].update(GetPixel=12,SetPixel=16)
  libs=[]
  for module,names in imports.items():
   definition=out/(module+'.def');definition.write_text('LIBRARY '+module+'.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n'for n,v in names.items()))
   lib=out/(module+'.lib');run('imports-'+module,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',lib]);libs.append(lib)
  probes={'factory':'tests/native-draw-factory-probe.c','surface':'tests/native-draw-surface-probe.c','services':'tests/native-draw-services-probe.c','invalid':'tests/native-draw-invalid-copy-reference.c'}
  for name,source in probes.items():
   used.add(source);run('compile-'+name,[*builder.FLAGS,'-MD','-MF',out/(name+'.d'),'-c',ROOT/source,'-o',out/(name+'.obj')])
   run('link-'+name,['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/(name+'.exe')),out/(name+'.obj'),*libs])
  import shlex
  for dep in out.glob('*.d'):
   for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
    path=Path(token).resolve()
    if path.is_relative_to(ROOT)and not path.is_relative_to(ROOT/'working'):used.add(path.relative_to(ROOT).as_posix())
  if used-frozen.keys()or any((ROOT/n).read_bytes()!=frozen[n]for n in used):raise ValueError('Unfrozen/changed compiler closure')
  report['tested_binaries']={path.name:retain(path.name+'.bin',path.read_bytes())for path in [out/'MnmDraw.dll',*[out/(n+'.exe')for n in probes]]}
  report['providers']={}
  for name in probes:
   for provider in ('original','native'):
    dest=out/name/provider;dest.mkdir(parents=True)
    extra={'WINEDLLOVERRIDES':'ddraw=b'if provider=='original'else'ddraw='}
    if provider=='native':extra.update(MNM_FACTORY_NATIVE='1',MNM_NATIVE_DRAW_STATUS='Z:'+str(dest/'status.bin').replace('/','\\'))
    run(name+'-'+provider,['wine',out/(name+'.exe')],extra,cwd=dest)
  def words(path):
   data=path.read_bytes()
   if len(data)%4:raise ValueError('Truncated WORD result')
   return struct.unpack('<'+str(len(data)//4)+'I',data)
  for name,cases,stride in [('factory',128,18),('surface',64,32+5*13*9),('services',64,32+3*13*9)]:
   left=words(out/name/'original'/(name+'-results.bin'));right=words(out/name/'native'/(name+'-results.bin'))
   if len(left)!=cases*stride or len(right)!=len(left):raise ValueError('Incomplete '+name)
   mismatches=[i for i,(x,y)in enumerate(zip(left,right))if x!=y and not(name=='factory'and i%stride==2)]
   report['providers'][name]=dict(cases=cases,compared_words=len(left)-(cases if name=='factory'else 0),mismatch_words=mismatches[:64])
   if name=='factory':
    report['factory_last_error']=dict(native_policy='preserve incoming0x51',original_values=sorted(set(left[i*18+2]for i in range(128))),equivalence=False)
    if any(right[i*18+2]!=0x51 for i in range(128)):raise ValueError('Native LastError policy')
   if mismatches:raise ValueError(name+' caller/pixel mismatch '+repr(mismatches[:16]))
  for filename,expected in [('surface-policy.bin',(15,117,512,0,4,367200,12)),('services-policy.bin',(2,351,1,0)),('startup-mode-policy.bin',(2,1,8,1,1))]:
   name='surface'if filename.startswith('surface')else'services'
   if words(out/name/'native'/filename)!=expected:raise ValueError('Native policy control '+filename)
  a_start=(out/'services/original/startup-results.bin').read_bytes();b_start=(out/'services/native/startup-results.bin').read_bytes()
  if len(a_start)!=1024 or a_start!=b_start:raise ValueError('Startup caller mismatch')
  report['startup_compared_words']=256
  invalid=(out/'invalid/original/invalid-results.bin').read_bytes();other=(out/'invalid/native/invalid-results.bin').read_bytes()
  if invalid!=other:raise ValueError('Invalid-copy provider caller/pixel mismatch')
  cursor=0;rows=[]
  for i in range(5):
   header=struct.unpack_from('<16I',invalid,cursor);cursor+=64
   if header[0]!=i or any(header[12:]):raise ValueError('Invalid-copy header')
   size=2*(header[1]*header[2]+header[3]*header[4]);before=invalid[cursor:cursor+size];cursor+=size
   result=struct.unpack_from('<I',invalid,cursor)[0];cursor+=4;after=invalid[cursor:cursor+size];cursor+=size
   if len(before)!=size or before!=after or result!=0x88760096:raise ValueError('Invalid-copy status or mutated plane')
   rows.append(dict(case=i,hresult=result,unchanged_pixel_words=size//2))
  if cursor!=len(invalid):raise ValueError('Trailing invalid-copy bytes')
  report['invalid_copies']=rows;report['success']=True
 except (ValueError,OSError,RuntimeError,subprocess.SubprocessError)as error:report['error']=str(error)
 finally:
  report['artifacts']={}
  for path in sorted(out.rglob('*')):
   if path.is_file()and path.suffix not in('.obj','.lib','.def','.d')and path.name!='display':report['artifacts'][path.relative_to(out).as_posix()]=retain('artifact-'+sha(path.relative_to(out).as_posix().encode())[:20]+'.bin',path.read_bytes())
  if lease is not None:
   for command in (['wineserver','-k'],['wineserver','-w']):
    try:run('retire-'+command[1][1:],command,check=False)
    except (OSError,subprocess.SubprocessError):report['success']=False
   from experiment_environment import _busy
   report['private_wine_stopped']=not _busy(prefix);report['success'] &= report['private_wine_stopped'];lease.close()
  if xvfb is not None:
   xvfb.terminate();xvfb.wait(timeout=10)
  try:run('original-after',[MAIN/'tools/original-manifest.sh','verify'])
  except (OSError,RuntimeError,subprocess.SubprocessError)as error:report.update(success=False,manifest_error=str(error))
  report['sources']={n:sha(frozen[n])for n in sorted(used&frozen.keys())};report['sources_stable']=not(used-frozen.keys())and all((ROOT/n).read_bytes()==frozen[n]for n in used&frozen.keys())
  report['success'] &= report['sources_stable'];write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps({k:report[k]for k in('success','error','providers','invalid_copies','private_wine_stopped')if k in report}),flush=True)
 return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
