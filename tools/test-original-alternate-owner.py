#!/usr/bin/env python3
"""Authored lifetime/failure controls around the original bind and integer copy."""
import argparse,hashlib,importlib.util,json,os,shlex,subprocess,sys
from pathlib import Path
from original_x86 import PE32
from original_alternate_owner import decode,accept
from coverage_evidence import write_report,evidence_digest
from coverage_claims import required_sources,behavior_contract,scenario_contract
from drawing_fixture_prefix import prepare
ROOT=Path(__file__).resolve().parents[1];ID='NP.original-alternate-allocation-repair'
def load(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);p.add_argument('--prefix-template',type=Path,required=True);a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require a fresh owned output')
 out.mkdir(parents=True);(out/'.retain').write_text('Original alternate allocation repair controls,including every failed attempt.\n')
 j=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(x for x in j['behaviors'] if x['id']==ID)
 scenario=next(x for x in j['scenarios'] if x['id']=='OA1.controls')
 sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(required_sources(b))}
 r=dict(schema=1,success=False,scope=__doc__,original_allocation_recovered=False,provider_lifecycle_authored=True,sources=sources,claims=[dict(behavior=ID,contract_sha256=behavior_contract(b,{x['id']:x for x in j['builds']}),scenarios={scenario['id']:scenario_contract(scenario)})],cases=[],commands=[])
 prefix=lock=None
 def run(label,command,env=None,cwd=out,expected=0):
  log=out/(label+'.log')
  with log.open('x') as f:q=subprocess.run(list(map(str,command)),cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=180)
  r['commands'].append(dict(label=label,argv=list(map(str,command)),exit=q.returncode,expected=expected,log=log.relative_to(ROOT).as_posix()))
  if q.returncode!=expected:raise ValueError(label+': exit '+str(q.returncode)+' expected '+str(expected)+' '+log.read_text(errors='replace')[-3000:])
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
  builder=load('owner_builder','tools/build-original-alternate-owner.py');dll=builder.build(out/'dll',selftest=True);production=builder.build(out/'production')
  image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes());r['original_sha256']=image.sha256
  arrays='static const struct {unsigned address,length;unsigned char bytes[10];} fixture_hooks[]={\n'+''.join('{'+hex(x)+','+str(n)+',{'+','.join(hex(v) for v in image.read(x,n))+'}},\n' for x,n in builder.HOOKS)+'};\n'
  for name,x,n in [('fixture_bind',0x57dc20,0x18),('fixture_copy',0x594508,0x54)]:arrays+='static const unsigned char '+name+'[]={'+','.join(hex(v) for v in image.read(x,n))+'};\n'
  (out/'alternate_owner_fixture.h').write_text(arrays)
  imports=dict(builder.IMPORTS,LoadLibraryA=4,GetProcAddress=8,GetCommandLineA=0,VirtualQuery=12,CreateThread=24,WaitForSingleObject=8);definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(n+'@'+str(v)+'\n' for n,v in imports.items()))
  run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I'+str(out)]
  run('fixture-build',[*flags,'-MD','-MF',out/'fixture.d','-c',ROOT/'tests/original-alternate-owner.c','-o',out/'fixture.obj'])
  for n in shlex.split((out/'fixture.d').read_text().replace('\\\n',' ').split(':',1)[1]):
   q=Path(n).resolve()
   if q.is_relative_to(ROOT) and not q.is_relative_to(ROOT/'working') and q.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared fixture input '+n)
  (out/'pad.S').write_text('.text\n.globl _owner_fixture_pad\n_owner_fixture_pad:\n.space 0x310000\n')
  run('fixture-pad',['clang','--target=i686-pc-windows-msvc','-c',out/'pad.S','-o',out/'pad.obj'])
  run('fixture-link',['lld-link','/machine:x86','/base:0x400000','/dynamicbase:no','/include:_owner_fixture_pad','/entry:mainCRTStartup','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'fixture.exe'),out/'fixture.obj',out/'pad.obj',out/'kernel32.lib'])
  prefix,lock=prepare(ROOT,out,a.prefix_template.resolve(),private=True)
  env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all')
  for name,exitcode,code in [('positive',0,0),('unlogged',0,None),('saturated-log',0,0),('bad-entry',0,None),('preexisting-owner',0,None),('foreign-thread',171,1),('guard',172,2),('foreign',173,3),('allocation-failure',176,6)]:
   case=out/name;case.mkdir();(case/'MnmAlt.dll').write_bytes(dll.read_bytes());settings=dict(env,MNM_ORIGINAL_ALTERNATE_LOG='Z:'+str(case/'alternate-owner.bin').replace('/','\\'))
   if name=='unlogged':settings.pop('MNM_ORIGINAL_ALTERNATE_LOG')
   if name=='allocation-failure':settings['MNM_ALT_OWNER_TEST_ALLOC_FAIL']='1'
   run(name,['wine',out/'fixture.exe',name],env=settings,cwd=case,expected=exitcode)
   path=case/'alternate-owner.bin';v=decode(path) if path.exists() else None
   if name in ('positive','saturated-log'):
    accept(v);assert v['allocations']==5 and v['copies']==10 and v['padding_resets']==1
   elif code is None:assert v is None
   else:
    assert v['error']==code and not v['active_base']
    assert v['active_source']==(0x123456 if name=='foreign' else 0)
   r['cases'].append(dict(name=name,exit=exitcode,expected_error=code,record=v))
  r.update(success=True,production_dll_sha256=hashlib.sha256(production.read_bytes()).hexdigest())
 except (OSError,ValueError,AssertionError,subprocess.SubprocessError) as e:r['error']=str(e)
 finally:
  if prefix:
   subprocess.run(['wineserver','-k'],env=dict(os.environ,WINEPREFIX=str(prefix),WINEDEBUG='-all'),check=False)
   subprocess.run(['wineserver','-w'],env=dict(os.environ,WINEPREFIX=str(prefix),WINEDEBUG='-all'),check=True,timeout=15)
  if lock:lock.close()
  try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);r['original_manifest_verified_before_after']=True
  except (OSError,ValueError,subprocess.SubprocessError) as e:r.update(success=False,manifest_error=str(e))
  r['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items());r['success'] &= r['sources_stable']
  # Preserve exact attempt-source bytes as immutable SQLite objects.
  r['source_artifacts']={}
  for n,h in sources.items():
   q=ROOT/n
   if q.is_file() and q.suffix in ['.c','.h','.S','.py'] and hashlib.sha256(q.read_bytes()).hexdigest()==h:
    key='research/runtime/original-alternate-allocation-results-20261026/source-'+h+'.bin';write_report(ROOT,key,q.read_bytes());r['source_artifacts'][n]=key
  write_report(ROOT,a.report,(json.dumps(r,indent=2)+'\n').encode());print(json.dumps(dict(success=r['success'],error=r.get('error'),report=a.report,cases=len(r['cases']))),flush=True)
 return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
